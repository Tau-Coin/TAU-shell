/*

Copyright (c) 2012, Arvid Norberg
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in
      the documentation and/or other materials provided with the distribution.
    * Neither the name of the author nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

*/

#include "handler/tau_handler.hpp"

#include "util/base64.hpp"
#include "util/escape_json.hpp" // for escape_json
#include "util/hex_util.hpp"
#include "util/json_util.hpp"
#include "util/response_buffer.hpp" // for appendf
#include "util/tau_constants.hpp"

#include <string.h> // for strcmp()
#include <stdio.h>
#include <vector>
#include <map>
#include <cstdint>

extern "C" {
#include "rpc/local_mongoose.h"
#include "util/jsmn.h"
}

#include "libTAU/session.hpp"
#include "libTAU/session_status.hpp"
#include "libTAU/hex.hpp"
#include "libTAU/performance_counters.hpp"
#include "libTAU/aux_/common_data.h"
#include "libTAU/blockchain/block.hpp"
#include "libTAU/blockchain/transaction.hpp"
#include "libTAU/communication/message.hpp"

using namespace libTAU;

namespace libTAU
{

void return_error(mg_connection* conn, char const* msg)
{
    mg_printf(conn, "HTTP/1.1 401 Invalid Request\r\n"
        "Content-Type: text/json\r\n"
        "Content-Length: %d\r\n\r\n"
        "{ \"result\": \"%s\" }", int(16 + strlen(msg)), msg);
}

void return_failure(std::vector<char>& buf, char const* msg, std::int64_t tag)
{
    buf.clear();
    appendf(buf, "{ \"result\": \"%s\", \"tag\": %" "I64d" "}", msg, tag);
}

struct method_handler
{
    char const* method_name;
    void (tau_handler::*fun)(std::vector<char>&, jsmntok_t* args, std::int64_t tag
        , char* buffer);
};

static method_handler handlers[] =
{
    {"session-stats", &tau_handler::session_stats},
    {"set-loop-time-interval", &tau_handler::set_loop_time_interval},
    {"add-new-friend", &tau_handler::add_new_friend},
    {"update-friend-info", &tau_handler::update_friend_info},
    {"get-friend-info", &tau_handler::get_friend_info},
    {"delete-friend", &tau_handler::delete_friend},
    {"add-new-message", &tau_handler::add_new_message},
    {"create-chain-id", &tau_handler::create_chain_id},
    {"create-new-community", &tau_handler::create_new_community},
    {"follow-chain", &tau_handler::follow_chain},
    {"follow-chain-mobile", &tau_handler::follow_chain_mobile},
    {"unfollow-chain", &tau_handler::unfollow_chain},
    {"unfollow-chain-mobile", &tau_handler::unfollow_chain_mobile},
    {"get-median-tx-fee", &tau_handler::get_median_tx_fee},
    {"get-block-by-number", &tau_handler::get_block_by_number},
    {"get-block-by-hash", &tau_handler::get_block_by_hash},
    {"send-data", &tau_handler::send_data},
    {"get-chain-state", &tau_handler::get_chain_state},
    {"get-account-info", &tau_handler::get_account_info},
    {"submit-note-transaction", &tau_handler::submit_note_transaction},
    {"submit-transaction", &tau_handler::submit_transaction},
    {"get-tx-state", &tau_handler::get_tx_state},
};

void tau_handler::handle_json_rpc(std::vector<char>& buf, jsmntok_t* tokens , char* buffer)
{
    // we expect a "method" in the top level
    jsmntok_t* method = find_key(tokens, buffer, "method", JSMN_STRING);
    if (method == NULL)
    {
        std::cout << "missing method in request" << std::endl;
        return_failure(buf, "missing method in request", -1);
        return;
    }

    bool handled = false;
    buffer[method->end] = 0;
    char const* m = &buffer[method->start];
    jsmntok_t* args = NULL;
    for (int i = 0; i < sizeof(handlers)/sizeof(handlers[0]); ++i)
    {
        std::cout << "==================================" << std::endl;
        std::cout << "Method Name: " <<  handlers[i].method_name << std::endl;
        std::cout << "==================================" << std::endl;
        if (strcmp(m, handlers[i].method_name)) continue;

        args = find_key(tokens, buffer, "arguments", JSMN_OBJECT);
        std::int64_t tag = find_int(tokens, buffer, "tag");
        handled = true;

        if (args) {
			buffer[args->end] = 0;
        	printf("%s: %d, %s\n", m, args->type, args ? buffer + args->start : "{}");
		}

        (this->*handlers[i].fun)(buf, args, tag, buffer);
        std::cout << "Method Over" << std::endl;
        break;
    }

    if (!handled)
        printf("Unhandled: %s: %s\n", m, args ? buffer + args->start : "{}");

}

void tau_handler::session_stats(std::vector<char>& buf, jsmntok_t* args, std::int64_t tag, char* buffer)
{
    // TODO: post session stats instead, and capture the performance counters
    m_ses.post_session_stats();
    appendf(buf, "{ \"result\": \"post session status success\"}\n");
}

//communication apis
void tau_handler::new_account_seed(std::vector<char>& buf, jsmntok_t* args, std::int64_t tag, char* buffer)
{
    jsmntok_t* s = find_key(args, buffer, "account_seed", JSMN_STRING);

    std::string account_seed_str;
    buffer[s->end] = 0;
    for(int i = s->start; i < s->end; i++){
        account_seed_str.push_back(buffer[i]);
    }

    //new account seed
    m_ses.new_account_seed(account_seed_str);

    appendf(buf, "{ \"result\": \"new account seed success\"}\n");
}

// main loop time interval
void tau_handler::set_loop_time_interval(std::vector<char>& buf, jsmntok_t* args, std::int64_t tag, char* buffer)
{
    jsmntok_t* ti = find_key(args, buffer, "time-interval", JSMN_PRIMITIVE);
    int time_interval = atoi(buffer + ti->start);
    std::cout << "Set Time Interval: " << time_interval << std::endl;
    m_ses.set_loop_time_interval(time_interval);
    appendf(buf, "{ \"result\": \"Set Time Interval\": %d \"OK\"}", time_interval);
}

//friends
void tau_handler::add_new_friend(std::vector<char>& buf, jsmntok_t* args, std::int64_t tag, char* buffer)
{
    jsmntok_t* f = find_key(args, buffer, "friend", JSMN_STRING);

    buffer[f->end] = 0;
    char const* friend_pubkey_hex = &buffer[f->start];
    char* friend_pubkey_char = new char[KEY_LEN];
    std::cout << "Friend: " << friend_pubkey_char << std::endl;
    hex_char_to_bytes_char(friend_pubkey_hex, friend_pubkey_char, KEY_HEX_LEN);
    dht::public_key friend_pubkey(friend_pubkey_char);
    m_ses.add_new_friend(friend_pubkey);

    //add in db
    std::cout << "Add sqldb" << std::endl;
    m_db->db_add_new_friend(friend_pubkey_hex);
    appendf(buf, "{ \"result\": \"Add New Friend\": %s \"OK\"}", friend_pubkey_hex);
}


void tau_handler::delete_friend(std::vector<char>& buf, jsmntok_t* args, std::int64_t tag, char* buffer)
{
    jsmntok_t* f = find_key(args, buffer, "friend", JSMN_STRING);
    buffer[f->end] = 0;
    char const* friend_pubkey_hex = &buffer[f->start];
    char* friend_pubkey_char = new char[KEY_LEN];
    hex_char_to_bytes_char(friend_pubkey_hex, friend_pubkey_char, KEY_HEX_LEN);
    dht::public_key friend_pubkey(friend_pubkey_char);

    m_ses.delete_friend(friend_pubkey);

    //delete in db
    m_db->db_delete_friend(friend_pubkey_char);
    appendf(buf, "{ \"result\": \"Delete Friend\": %s \"OK\"}", friend_pubkey_hex);
}

void tau_handler::get_friend_info(std::vector<char>&, jsmntok_t* args, std::int64_t tag, char* buffer)
{

}

void tau_handler::update_friend_info(std::vector<char>&, jsmntok_t* args, std::int64_t tag, char* buffer)
{
}

// message
void tau_handler::add_new_message(std::vector<char>&, jsmntok_t* args, std::int64_t tag, char* buffer)
{
    jsmntok_t* s = find_key(args, buffer, "sender", JSMN_STRING);
    jsmntok_t* r = find_key(args, buffer, "receiver", JSMN_STRING);
    jsmntok_t* p = find_key(args, buffer, "payload", JSMN_STRING);

    //sender
    buffer[s->end] = 0;
    char const* sender_pubkey_hex = &buffer[s->start];
    char* sender_pubkey_char = new char[KEY_LEN];
    hex_char_to_bytes_char(sender_pubkey_hex, sender_pubkey_char, KEY_HEX_LEN);
    dht::public_key sender_pubkey(sender_pubkey_char);
    //std::cout << sender_pubkey_hex << std::endl;

    //receiver
    buffer[r->end] = 0;
    char const* receiver_pubkey_hex = &buffer[r->start];
    char* receiver_pubkey_char = new char[KEY_LEN];
    hex_char_to_bytes_char(receiver_pubkey_hex, receiver_pubkey_char, KEY_HEX_LEN);
    dht::public_key receiver_pubkey(receiver_pubkey_char);
    //std::cout << receiver_pubkey_hex << std::endl;

    //payload
    aux::bytes payload;
    int index = p->start;
    while(index < p->end){
        payload.push_back(buffer[index]);
        index++;
    }
    //std::cout << payload << std::endl;

    //timestamp
    std::int64_t time_stamp = m_ses.get_session_time();

    communication::message msg(time_stamp, sender_pubkey, receiver_pubkey, payload);

    m_ses.add_new_message(msg);
    
    //insert friend info
    //m_db->db_add_new_message(msg);
}

//blockchain
void tau_handler::create_chain_id(std::vector<char>& buf, jsmntok_t* args, std::int64_t tag, char* buffer)
{
    jsmntok_t* c = find_key(args, buffer, "community_name", JSMN_STRING);
    std::vector<char> community_name;
    buffer[c->end] = 0;
    for(int i = c->start; i < c->end; i++){
        community_name.push_back(buffer[i]);
    }
    std::cout << "Name: "    << community_name.data() << std::endl;
    std::vector<char> chain_id_bytes = m_ses.create_chain_id(community_name);
    std::cout << "Chain id size: "    << chain_id_bytes.size() << std::endl;

    std::string str_chain_id = bytes_chain_id_to_string(chain_id_bytes.data(), chain_id_bytes.size());

    std::cout << "Id: "    << str_chain_id << std::endl;
    appendf(buf, "{\"chainID\": \"%s\"}", str_chain_id.c_str());
}

void tau_handler::create_new_community(std::vector<char>& buf, jsmntok_t* args, std::int64_t tag, char* buffer)
{
    jsmntok_t* c = find_key(args, buffer, "chain_id", JSMN_STRING);
    jsmntok_t* ks = find_key(args, buffer, "peers", JSMN_ARRAY);

    //chain_id
    int size = c->end - c->start;
    buffer[c->end] = 0;
    char const* chain_id_str = &buffer[c->start];
    std::vector<char> chain_id = string_chain_id_to_bytes(chain_id_str, size);

    //peer -> account
    std::cout << "Follow Chain Peers Size: " << ks->size << std::endl;
    std::map<dht::public_key, blockchain::account> accounts;
    std::set<dht::public_key> peer_list;
    for(int i = 0; i < ks->size; i++) {
        jsmntok_t* pk = find_key_in_array(args, buffer, "peer_key", i, args->size*2 + 2, JSMN_STRING);
        buffer[pk->end] = 0;
        char const* pubkey_hex = &buffer[pk->start];
        std::cout << "key: " << pubkey_hex << std::endl;
        char* pubkey_char = new char[KEY_LEN];
        hex_char_to_bytes_char(pubkey_hex, pubkey_char, KEY_HEX_LEN);
        dht::public_key pubkey(pubkey_char);
        peer_list.emplace(pubkey);
        std::int64_t time_stamp = m_ses.get_session_time();
        blockchain::account act(100000000000, 0, 1, 0);
        accounts[pubkey] = act;
    }

    std::int64_t time_stamp = m_ses.get_session_time();
    //tx, note: d30101906e6f7465207472616e73616374696f6e
    std::vector<char> payload;
    payload.resize(20);
    hex_char_to_bytes_char("d30101906e6f7465207472616e73616374696f6e", payload.data(), 40);
    auto iter_act = accounts.begin();
    blockchain::transaction tx(chain_id, blockchain::tx_version::tx_version1, time_stamp, iter_act->first, payload);

    //create new community
    m_ses.create_new_community(chain_id, accounts);

    //db follow
    m_db->db_follow_chain(chain_id_str, peer_list);

    appendf(buf, "{\"result\": \"success\"}");
}

void tau_handler::follow_chain(std::vector<char>& buf, jsmntok_t* args, std::int64_t tag, char* buffer)
{
    jsmntok_t* c = find_key(args, buffer, "chain_id", JSMN_STRING);
    jsmntok_t* ks = find_key(args, buffer, "peers", JSMN_ARRAY);
   
    //deal with peer
    std::cout << "Follow Chain Peers Size: " << ks->size << std::endl;
    std::set<dht::public_key> peer_list;
    for(int i = 0; i < ks->size; i++) {
        jsmntok_t* pk = find_key_in_array(args, buffer, "peer_key", i, args->size*2 + 2, JSMN_STRING);
        buffer[pk->end] = 0;
        char const* pubkey_hex = &buffer[pk->start];
        std::cout << "key: " << pubkey_hex << std::endl;
        char* pubkey_char = new char[KEY_LEN];
        hex_char_to_bytes_char(pubkey_hex, pubkey_char, KEY_HEX_LEN);
        dht::public_key pubkey(pubkey_char);
        peer_list.emplace(pubkey);
    }
   
    //deal with chain id
    int size = c->end - c->start; //input chars size
    buffer[c->end] = 0;
    char* chain_id_hex = &buffer[c->start];
    std::string chain_id_hex_str(chain_id_hex);
    std::vector<char> chain_id_char = hex_chain_id_to_bytes(chain_id_hex, size);

    m_ses.follow_chain(chain_id_char, peer_list);

    //save chain into db
    m_db->db_follow_chain(chain_id_hex_str, peer_list);
    appendf(buf, "{\"Follow Chain Id\": %s \"OK\"}", chain_id_hex_str.data());
}

void tau_handler::follow_chain_mobile(std::vector<char>& buf, jsmntok_t* args, std::int64_t tag, char* buffer)
{
    jsmntok_t* c = find_key(args, buffer, "chain_id", JSMN_STRING);
    jsmntok_t* ks = find_key(args, buffer, "peers", JSMN_ARRAY);

    //peer_list
    std::set<dht::public_key> peer_list;
    for(int i = 0; i < ks->size; i++) {
        jsmntok_t* pk = find_key_in_array(args, buffer, "peer_key", i, args->size*2 + 2, JSMN_STRING);
        buffer[pk->end] = 0;
        char const* pubkey_hex_char = &buffer[pk->start];
        std::cout << "key: " << pubkey_hex_char << std::endl;
        char* pubkey_char = new char[KEY_LEN];
        hex_char_to_bytes_char(pubkey_hex_char, pubkey_char, KEY_HEX_LEN);
        dht::public_key pubkey(pubkey_char);
        peer_list.emplace(pubkey);
    }

    //chain_id
    int size = c->end - c->start;
    buffer[c->end] = 0;
    char const* chain_id_str = &buffer[c->start];
    std::cout << "Follow Chain Str Size: " << size << std::endl;
    std::vector<char> chain_id = string_chain_id_to_bytes(chain_id_str, size);
    std::cout << "Follow Chain ID Size: " << chain_id.size() << std::endl;

    m_ses.follow_chain(chain_id, peer_list);

    //peer_list
    //save chain into db
    m_db->db_follow_chain(chain_id_str, peer_list);

    appendf(buf, "{\"Follow Chain Id\": %s \"OK\"}", chain_id_str);
}

void tau_handler::get_tx_state(std::vector<char>& buf, jsmntok_t* args, std::int64_t tag, char* buffer)
{
    jsmntok_t* h = find_key(args, buffer, "tx_hash", JSMN_STRING);

    //chain_id
    int size = h->end - h->start;
    buffer[h->end] = 0;
    char const* tx_hash_char = &buffer[h->start];
    std::string tx_hash(tx_hash_char);
    std::cout << "Get Tx Hash: " << tx_hash << std::endl;

    //peer_list
    //save chain into db
    int tx_state = m_db->db_get_tx_state(tx_hash);

    appendf(buf, "{\"result\": \"%s\", \"state\": %d}", "success", tx_state);
}

void tau_handler::unfollow_chain(std::vector<char>& buf, jsmntok_t* args, std::int64_t tag, char* buffer) 
{
    jsmntok_t* c = find_key(args, buffer, "chain_id", JSMN_STRING);

    //deal with chain id
    int size = c->end - c->start; //input chars size
    buffer[c->end] = 0;
    char* chain_id_hex = &buffer[c->start];
    std::string chain_id_hex_str(chain_id_hex);
    std::vector<char> chain_id_char = hex_chain_id_to_bytes(chain_id_hex, size);

    m_ses.unfollow_chain(chain_id_char);

    //save chain into db
    m_db->db_unfollow_chain(chain_id_hex_str);
    appendf(buf, "{\"Unfollow Chain Id\": %s \"OK\"}", chain_id_hex);

}

void tau_handler::unfollow_chain_mobile(std::vector<char>& buf, jsmntok_t* args, std::int64_t tag, char* buffer)
{
    jsmntok_t* c = find_key(args, buffer, "chain_id", JSMN_STRING);

    //chain_id
    int size = c->end - c->start;
    buffer[c->end] = 0;
    char const* chain_id_str = &buffer[c->start];
    std::vector<char> chain_id = string_chain_id_to_bytes(chain_id_str, size);

    m_ses.unfollow_chain(chain_id);

    //save chain into db
    m_db->db_unfollow_chain(chain_id_str);
    appendf(buf, "{\"Unfollow Chain Id\": %s \"OK\"}", chain_id_str);

}

void tau_handler::get_chain_state(std::vector<char>& buf, jsmntok_t* args, std::int64_t tag, char* buffer)
{
    jsmntok_t* c = find_key(args, buffer, "chain_id", JSMN_STRING);

    //chain_id
    int size = c->end - c->start;
    buffer[c->end] = 0;
    char const* chain_id_str = &buffer[c->start];
    std::vector<char> chain_id = string_chain_id_to_bytes(chain_id_str, size);

    m_ses.request_chain_state(chain_id);
   
    int block_number[3] = {};
    std::string block_hash[3] = {};

    //get block number and hash
    m_db->db_get_chain_state(std::string(chain_id_str), block_number, block_hash);

    appendf(buf, "{\"headBlockNumber\": %d, \"consensusBlockNumber\": %d, \"tailBlockNumber\": %d, \"headBlockHash\": \"%s\", \"consensusBlockHash\": \"%s\", \"tailBlockHash\": \"%s\"}", block_number[0], block_number[1], block_number[2], block_hash[0].c_str(), block_hash[1].c_str(), block_hash[2].c_str());
}

//TODO:DEBUG
void tau_handler::submit_note_transaction(std::vector<char>& buf, jsmntok_t* args, std::int64_t tag, char* buffer)
{
    jsmntok_t* c = find_key(args, buffer, "chain_id", JSMN_STRING);
    jsmntok_t* s = find_key(args, buffer, "sender", JSMN_STRING);
    jsmntok_t* f = find_key(args, buffer, "fee", JSMN_PRIMITIVE);
    jsmntok_t* p = find_key(args, buffer, "payload", JSMN_STRING);

    //chain_id
    int size = c->end - c->start;
    buffer[c->end] = 0;
    char const* chain_id_str = &buffer[c->start];
    std::vector<char> chain_id = string_chain_id_to_bytes(chain_id_str, size);

    //sender
    buffer[s->end] = 0;
    char const* sender_pubkey_hex_char = &buffer[s->start];
    char* sender_pubkey_char = new char[KEY_LEN];
    hex_char_to_bytes_char(sender_pubkey_hex_char, sender_pubkey_char, KEY_HEX_LEN);
    dht::public_key sender_pubkey(sender_pubkey_char);

    std::cout << sender_pubkey_hex_char << std::endl;

	//fee
    std::int64_t fee = atoi(buffer + f->start);
	if(fee < 0)
		fee = m_ses.get_median_tx_free(chain_id);

    //payload
    /*
    aux::bytes payload;
    int index = p->start;
    while(index < p->end){
        payload.push_back(buffer[index]);
        index++;
    }
    */
    std::vector<char> payload;
    payload.resize(20);
    hex_char_to_bytes_char("d30101906e6f7465207472616e73616374696f6e", payload.data(), 40);

    //timestamp
    std::int64_t time_stamp = m_ses.get_session_time();
    blockchain::transaction tx(chain_id, blockchain::tx_version::tx_version1, time_stamp, sender_pubkey, payload);
	tx.sign(m_pubkey, m_seckey);
	//construct and sign
    bool result = m_ses.submit_transaction(tx);
	std::string tx_hash = aux::to_hex(tx.sha256());

    if(result)
    {
        //insert friend info
        appendf(buf, "{\"result\": \"%s\", \"nonce\": %d, \"txHash\":\"%s\"}", "success", 0, tx_hash.c_str());
    } else {
        appendf(buf, "{\"result\": \"%s\", \"nonce\": %d, \"txHash\":\"%s\"}", "fail", 0, tx_hash.c_str());
    }
}

//TODO:DEBUG
void tau_handler::submit_transaction(std::vector<char>& buf, jsmntok_t* args, std::int64_t tag, char* buffer)
{
    jsmntok_t* c = find_key(args, buffer, "chain_id", JSMN_STRING);
    jsmntok_t* s = find_key(args, buffer, "sender", JSMN_STRING);
    jsmntok_t* r = find_key(args, buffer, "receiver", JSMN_STRING);
    jsmntok_t* a = find_key(args, buffer, "amount", JSMN_PRIMITIVE);
    jsmntok_t* f = find_key(args, buffer, "fee", JSMN_PRIMITIVE);
    jsmntok_t* p = find_key(args, buffer, "payload", JSMN_STRING);

    //chain_id
    int size = c->end - c->start;
    buffer[c->end] = 0;
    char const* chain_id_str = &buffer[c->start];
    std::vector<char> chain_id = string_chain_id_to_bytes(chain_id_str, size);

    //sender
    buffer[s->end] = 0;
    char const* sender_pubkey_hex_char = &buffer[s->start];
    char* sender_pubkey_char = new char[KEY_LEN];
    hex_char_to_bytes_char(sender_pubkey_hex_char, sender_pubkey_char, KEY_HEX_LEN);
    dht::public_key sender_pubkey(sender_pubkey_char);

    std::cout << sender_pubkey_hex_char << std::endl;

    //receiver
    buffer[r->end] = 0;
    char const* receiver_pubkey_hex_char = &buffer[r->start];
    char* receiver_pubkey_char = new char[KEY_LEN];
    hex_char_to_bytes_char(receiver_pubkey_hex_char, receiver_pubkey_char, KEY_HEX_LEN);
    dht::public_key receiver_pubkey(receiver_pubkey_char);

	//nonce
    blockchain::account act = m_ses.get_account_info(chain_id, sender_pubkey);
    if(act.empty()){
        appendf(buf, "{ \"result\": \"%s\"}", "fail");
        return ;
    }
    std::int64_t nonce = act.nonce() + 1;

    //amount
    int amount = atoi(buffer + a->start);

	//fee
    std::int64_t fee = atoi(buffer + f->start);
	if(fee < 0)
		fee = m_ses.get_median_tx_free(chain_id);

    //payload
    /*
    aux::bytes payload;
    int index = p->start;
    while(index < p->end){
        payload.push_back(buffer[index]);
        index++;
    }
    */
    std::vector<char> payload;
    payload.resize(22);
    hex_char_to_bytes_char("d5010292776972696e67207472616e73616374696f6e", payload.data(), 44);

    //timestamp
    std::int64_t time_stamp = m_ses.get_session_time();
    blockchain::transaction tx(chain_id, blockchain::tx_version::tx_version1, time_stamp, sender_pubkey, receiver_pubkey, nonce, amount, fee, payload);
	tx.sign(m_pubkey, m_seckey);
	std::string tx_hash = aux::to_hex(tx.sha256());

	//construct and sign
    bool result = m_ses.submit_transaction(tx);
    if(result)
    {
        //insert friend info
        appendf(buf, "{\"result\": \"%s\", \"nonce\": %d, \"txHash\":\"%s\"}", "success", nonce, tx_hash.c_str());
    } else {
        appendf(buf, "{\"result\": \"%s\", \"nonce\": %d, \"txHash\":\"%s\"}", "fail", nonce, tx_hash.c_str());
    }
}

void tau_handler::get_account_info(std::vector<char>& buf, jsmntok_t* args, std::int64_t tag, char* buffer)
{
    jsmntok_t* c = find_key(args, buffer, "chain_id", JSMN_STRING);
    jsmntok_t* k = find_key(args, buffer, "pubkey", JSMN_STRING);

    //chain_id
    int size = c->end - c->start;
    buffer[c->end] = 0;
    char const* chain_id_str = &buffer[c->start];
    std::vector<char> chain_id = string_chain_id_to_bytes(chain_id_str, size);

    //pubkey
    buffer[k->end] = 0;
    char const* pubkey_hex_char = &buffer[k->start];
    char* pubkey_char = new char[KEY_LEN];
    hex_char_to_bytes_char(pubkey_hex_char, pubkey_char, KEY_HEX_LEN);
    dht::public_key pubkey(pubkey_char);

    std::cout << pubkey_hex_char << std::endl;

    m_ses.request_chain_state(chain_id);
   
    int block_number[3] = {};
    std::string block_hash[3] = {};

    //get block number and hash
    m_db->db_get_chain_state(std::string(chain_id_str), block_number, block_hash);

    std::int64_t balance = 0;
    std::int64_t nonce = 0;
    std::int64_t effective_power = 0;
    std::int64_t state_number = -1;
    
    blockchain::account act = m_ses.get_account_info(chain_id, pubkey);
    if(!act.empty()){
        //balance
        balance = act.balance();
        nonce = act.nonce();
        effective_power = act.effective_power();
        state_number = act.block_number();
    }
    
    appendf(buf, "{\"ChainID\": \"%s\", \"Pubkey\": \"%s\","
            "\"balance\": %ld, \"nonce\": %ld, \"power\": %ld, \"block_number\": %ld, "
            "\"headBlockNumber\": %d, \"consensusBlockNumber\": %d, \"tailBlockNumber\": %d,"
            "\"headBlockHash\": \"%s\", \"consensusBlockHash\": \"%s\", \"tailBlockHash\": \"%s\"}",
            chain_id_str, pubkey_hex_char, 
            balance, nonce, effective_power, state_number,
            block_number[0], block_number[1], block_number[2],
            block_hash[0].c_str(), block_hash[1].c_str(), block_hash[2].c_str());
}

//TODO:Debug
void tau_handler::get_top_tip_block(std::vector<char>& buf, jsmntok_t* args, std::int64_t tag, char* buffer)
{
    jsmntok_t* c = find_key(args, buffer, "chain_id", JSMN_STRING);
    jsmntok_t* n = find_key(args, buffer, "number", JSMN_PRIMITIVE);

    //chain_id
    int size = c->end - c->start;
    buffer[c->end] = 0;
    char const* chain_id_str = &buffer[c->start];
    std::vector<char> chain_id = string_chain_id_to_bytes(chain_id_str, size);

    //number
    int number = atoi(buffer + n->start);

    std::cout << number << std::endl;

    std::vector<blockchain::block> blocks = m_ses.get_top_tip_block(chain_id, number);

    for(int i = 0; i< blocks.size(); i++) {
        appendf(buf, blocks[i].to_string().c_str());
    }
}

void tau_handler::get_median_tx_fee(std::vector<char>& buf, jsmntok_t* args, std::int64_t tag, char* buffer)
{
    jsmntok_t* c = find_key(args, buffer, "chain_id", JSMN_STRING);

    //chain_id
    int size = c->end - c->start;
    buffer[c->end] = 0;
    char const* chain_id_str = &buffer[c->start];
    std::vector<char> chain_id = string_chain_id_to_bytes(chain_id_str, size);

    std::int64_t fee = m_ses.get_median_tx_free(chain_id);

    appendf(buf, "{\"Get Chain Id\": %s, Fee: %d \"OK\"}", chain_id_str, fee);

}

void tau_handler::get_block_by_number(std::vector<char>& buf, jsmntok_t* args, std::int64_t tag, char* buffer)
{
    jsmntok_t* c = find_key(args, buffer, "chain_id", JSMN_STRING);
    jsmntok_t* n = find_key(args, buffer, "number", JSMN_PRIMITIVE);

    //chain_id
    int size = c->end - c->start;
    buffer[c->end] = 0;
    char const* chain_id_str = &buffer[c->start];
    std::vector<char> chain_id = string_chain_id_to_bytes(chain_id_str, size);

    //number
    int number = atoi(buffer + n->start);

    std::cout << number << std::endl;

    blockchain::block block = m_ses.get_block_by_number(chain_id, number);

    appendf(buf, block.to_string().c_str());
}

void tau_handler::get_block_by_hash(std::vector<char>& buf, jsmntok_t* args, std::int64_t tag, char* buffer)
{
    jsmntok_t* c = find_key(args, buffer, "chain_id", JSMN_STRING);
    jsmntok_t* h = find_key(args, buffer, "block_hash", JSMN_STRING);

    //chain_id
    int size = c->end - c->start;
    buffer[c->end] = 0;
    char const* chain_id_str = &buffer[c->start];
    std::vector<char> chain_id = string_chain_id_to_bytes(chain_id_str, size);

    //blockhash
    size = h->start - h->end + 1;
    std::cout << size << std::endl;
    buffer[h->end] = 0;
    char const* block_hash_hex = &buffer[c->start];
    std::vector<char> block_hash;
    block_hash.resize(size/2);
    hex_char_to_bytes_char(block_hash_hex, block_hash.data(), size);

    sha256_hash hash(block_hash);

    std::cout << block_hash.data() << std::endl;

    blockchain::block block = m_ses.get_block_by_hash(chain_id, hash);

    appendf(buf, block.to_string().c_str());
}

//send
void tau_handler::send_data(std::vector<char>& buf, jsmntok_t* args, std::int64_t tag, char* buffer)
{
    jsmntok_t* f = find_key(args, buffer, "receiver", JSMN_STRING);
    jsmntok_t* p = find_key(args, buffer, "payload", JSMN_STRING);
    jsmntok_t* a = find_key(args, buffer, "alpha", JSMN_PRIMITIVE);
    jsmntok_t* b = find_key(args, buffer, "beta", JSMN_PRIMITIVE);
    jsmntok_t* i = find_key(args, buffer, "invoke_limit", JSMN_PRIMITIVE);
    jsmntok_t* h = find_key(args, buffer, "hit_limit", JSMN_PRIMITIVE);

    //receiver
    buffer[f->end] = 0;
    char const* receiver_pubkey_hex_char = &buffer[f->start];
    char* receiver_pubkey_char = new char[KEY_LEN];
    //std::cout << "Receiver: " << receiver_pubkey_hex_char << std::endl;
    hex_char_to_bytes_char(receiver_pubkey_hex_char, receiver_pubkey_char, KEY_HEX_LEN);
    dht::public_key receiver_pubkey(receiver_pubkey_char);

    //entry
    buffer[p->end] = 0;
    char const* payload = &buffer[p->start];
    //std::cout << "Payload: " << payload << std::endl;
    entry e(payload);

    //alpha
    std::int8_t alpha = atoi(buffer + a->start);
    //std::cout << alpha << std::endl;

    //beta
    std::int8_t beta = atoi(buffer + b->start);
    //std::cout << beta << std::endl;

    //invoke_limit
    std::int8_t invoke_limit = atoi(buffer + i->start);
    //std::cout << invoke_limit << std::endl;

    //hit_limit
    std::int8_t hit_limit = atoi(buffer + i->start);

    auto now = std::chrono::system_clock::now(); 
    auto now_c = std::chrono::system_clock::to_time_t(now); 
    std::cout << std::put_time(std::localtime(&now_c), "%c") << " Send-data-Payload: " << payload << std::endl;

    m_ses.send(receiver_pubkey, e, alpha, beta, invoke_limit, hit_limit);
}

tau_handler::tau_handler(session& s, tau_shell_sql* sqldb, auth_interface const* auth, dht::public_key& pubkey, dht::secret_key& seckey)
    : m_ses(s)
    , m_db(sqldb)
    , m_auth(auth)
	, m_pubkey(pubkey)
	, m_seckey(seckey)
{

}

tau_handler::~tau_handler() {}

bool tau_handler::handle_http(mg_connection* conn, mg_request_info const* request_info)
{
    std::cout << "==============Incoming HTTP ==============" << std::endl;
    // we only provide access to paths under /web and /upload
    if (strcmp(request_info->uri, "/rpc"))
        return false;

    permissions_interface const* perms = parse_http_auth(conn, m_auth);
    if (perms == NULL)
    {    
        mg_printf(conn, "HTTP/1.1 401 Unauthorized\r\n"
            "WWW-Authenticate: Basic realm=\"BitTorrent\"\r\n"
            "Content-Length: 0\r\n\r\n");
        return true;
    } 

    char const* cl = mg_get_header(conn, "content-length");
    std::vector<char> post_body;
    if (cl != NULL)
    {
        int content_length = atoi(cl);
        if (content_length > 0 && content_length < 10 * 1024 * 1024)
        {
            post_body.resize(content_length + 1);
            mg_read(conn, &post_body[0], post_body.size());
            // null terminate
            post_body[content_length] = 0;
        }
    }

    printf("REQUEST: %s%s%s\n", request_info->uri
        , request_info->query_string ? "?" : ""
        , request_info->query_string ? request_info->query_string : "");

    std::vector<char> response;
    if (post_body.empty())
    {
        return_error(conn, "request with no POST body");
        return true;
    }
    jsmntok_t tokens[256];
    jsmn_parser p;
    jsmn_init(&p);

    int r = jsmn_parse(&p, &post_body[0], tokens, sizeof(tokens)/sizeof(tokens[0]));
    if (r == JSMN_ERROR_INVAL)
    {
        return_error(conn, "request not JSON");
        return true;
    }
    else if (r == JSMN_ERROR_NOMEM)
    {
        return_error(conn, "request too big");
        return true;
    }
    else if (r == JSMN_ERROR_PART)
    {
        return_error(conn, "request truncated");
        return true;
    }
    else if (r != JSMN_SUCCESS)
    {
        return_error(conn, "invalid request");
        return true;
    }

    handle_json_rpc(response, tokens, &post_body[0]);

    // we need a null terminator
    response.push_back('\0');
    // subtract one from content-length
    // to not count null terminator
    mg_printf(conn, "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/json\r\n"
        "Content-Length: %d\r\n\r\n", int(response.size()) - 1);
    mg_write(conn, &response[0], response.size());
    printf("%s\n", &response[0]);
    return true;
}

}
