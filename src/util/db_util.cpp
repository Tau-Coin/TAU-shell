#include <fstream>
#include <iostream>
#include <cstring>
#include <string>
#include <vector>
#include <stdio.h>

extern "C" {
#include "util/jsmn.h"
}

#include "util/hex_util.hpp"
#include "util/db_util.hpp"
#include "util/json_util.hpp"

#include "libTAU/hex.hpp"
#include "libTAU/blockchain/constants.hpp"

namespace libTAU {

	tau_shell_sql::tau_shell_sql(std::string db_storage_file_name)
	{
		m_db_path = db_storage_file_name;
	    int sqlrc = sqlite3_open_v2(m_db_path.c_str(), &m_db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_SHAREDCACHE, NULL);
  		if (sqlrc != SQLITE_OK) {
       		//报错退出-打开数据库失败
        	fprintf(stderr, "failed to open sqlite db");
        	exit(1);
    	} 
	}

	tau_shell_sql::~tau_shell_sql()
	{
	    int sqlrc = sqlite3_close_v2(m_db);
  		if (sqlrc != SQLITE_OK) {
       		//报错退出-打开数据库失败
        	fprintf(stderr, "failed to close sqlite db");
    	} 
	}

	void tau_shell_sql::sqlite_db_initial()
	{
		// read db info from config.json
		std::vector<char> sql_info;
		FILE *fp = fopen("./config/1.json", "r");
		if(fp != NULL) {
			while(!feof(fp)) {
				sql_info.push_back(fgetc(fp));
			}
		}

		int i = sql_info.size() - 1;
		while(sql_info[i] != '}') {
			sql_info.pop_back();
			i--;
		}
		sql_info[sql_info.size()]='\0';

		// parse into map(string, string), 1st is table name, 2nd is create sql
		jsmntok_t tokens[2048];
    	jsmn_parser p;
    	jsmn_init(&p);

    	int r = jsmn_parse(&p, &sql_info[0], tokens, sizeof(tokens)/sizeof(tokens[0]));
    	if (r == JSMN_ERROR_INVAL)
    	{   
        	std::cout << "request not JSON" << std::endl;
    	}   
    	else if (r == JSMN_ERROR_NOMEM)
    	{   
        	std::cout << "request too big" << std::endl;
    	}   
    	else if (r == JSMN_ERROR_PART)
    	{   
        	std::cout << "request truncated" << std::endl;
    	}   
    	else if (r != JSMN_SUCCESS)
    	{   
        	std::cout << "invalid request" << std::endl;
    	}

		// delete 
		for(int i = 0; i< 2048; i++) {
			sql_info[tokens[i].end] = 0;
		}

		// database -> entities		
		jsmntok_t* database = find_key(tokens, &sql_info[0], "database", JSMN_OBJECT);
		if(!database) {
			std::cout << "no key: database in config file" << std::endl;
		}

		jsmntok_t* tables = find_key(database, &sql_info[0], "entities", JSMN_ARRAY);
		if(!tables) {
			std::cout << "no key: entities in config file" << std::endl;
		}

		// entities -> tables and create
		for(int i = 0; i < tables->size; i++) {
			jsmntok_t* table_name = find_key_in_array(tokens, &sql_info[0], "tableName", i , 2048, JSMN_STRING);
			if(!table_name) {
				std::cout << "no key: tableName in config file" << std::endl;
			}
			int tn_size = table_name->end - table_name->start + 1;
			char *tn = new char[tn_size];
			memcpy(tn, &sql_info[table_name->start], tn_size);

			jsmntok_t* sql_table = find_key_in_array(tokens, &sql_info[0], "createSql", i , 2048, JSMN_STRING);
			if(!sql_table) {
				std::cout << "no key: createSql in config file" << std::endl;
			}

			int sql_size = sql_table->end - sql_table->start + 1;
			char *sql_temp = new char[sql_size];
			memcpy(sql_temp, &sql_info[sql_table->start], sql_size);

			// process sql into sql standard
			char* sql = sql_process(sql_temp, sql_size, tn, tn_size);
			// create db table
			char *err_msg = nullptr;
            int ok = sqlite3_exec(m_db, sql, nullptr, nullptr, &err_msg);
            if (ok != SQLITE_OK) {
                sqlite3_free(err_msg);
				std::cout << "sql create table exec error" << std::endl;
            }

		}
			
	}

	char* tau_shell_sql::sql_process(char *s, int s_s, char *t, int s_t)
	{
		//relpace ` into "
		int index = -1;
		for(int i = 0; i < s_s ; i++) {
			if(s[i]=='`') {
				s[i] = '"';
				if(-1 == index) {
					index = i;
				}
			}
		}
		int new_size = s_s + s_t - 14;
		char *c = new char[new_size]; //quit ${TABLE_NAME}
		//1st "
		memcpy(c, &s[0], index + 1);
		//2nd table name
		memcpy(c + index + 1, &t[0], s_t - 1);
		//3rd
		memcpy(c + index + s_t, &s[index + 14], s_s - index - 14);

		return c;
	}

	bool tau_shell_sql::db_add_new_friend(std::string f)
	{
        std::string sql = "INSERT INTO Friends VALUES(";
		std::string friend_info = "\"" + f + "\", "+ 
								  "\"" + f + "\", "+
								 "-1, -1, -1, -1, -1)";
		sql += friend_info;
		std::cout << sql << std::endl;

		char *err_msg = nullptr;
        int ok = sqlite3_exec(m_db, sql.data(), nullptr, nullptr, &err_msg);
        if (ok != SQLITE_OK) {
            sqlite3_free(err_msg);
			std::cout << "sql insert new friend error" << std::endl;
        	return false;
		}

        return true;
	}

	bool tau_shell_sql::db_delete_friend(char* f)
	{
		return true;
	}

	bool tau_shell_sql::db_update_friend_last_seen(const dht::public_key& pubkey, std::int64_t time)
	{
		//pubkey
		std::string peer = aux::toHex(pubkey.bytes);
		std::cout << peer << std::endl;

		//time
		std::stringstream ss_time;
		ss_time << time;
		std::cout << ss_time.str() << std::endl;

        std::string sql = "UPDATE Friends SET lastSeenTime = ";
		sql += ss_time.str() + " WHERE friendPK = \"" +
			   peer + "\"";
		std::cout << sql << std::endl;

		char *err_msg = nullptr;
        int ok = sqlite3_exec(m_db, sql.data(), nullptr, nullptr, &err_msg);
        if (ok != SQLITE_OK) {
            sqlite3_free(err_msg);
			std::cout << "sql update friend last seen error" << std::endl;
        	return false;
		}
		
		return true;
	}

	bool tau_shell_sql::db_update_friend_last_comm(const dht::public_key& pubkey, std::int64_t time)
	{
		//pubkey
		std::string peer = aux::toHex(pubkey.bytes);
		std::cout << peer << std::endl;

		//time
		std::stringstream ss_time;
		ss_time << time;
		std::cout << ss_time.str() << std::endl;

        std::string sql = "UPDATE Friends SET lastCommTime = ";
		sql += ss_time.str() + " WHERE friendPK = \"" +
			   peer + "\"";
		std::cout << sql << std::endl;

		char *err_msg = nullptr;
        int ok = sqlite3_exec(m_db, sql.data(), nullptr, nullptr, &err_msg);
        if (ok != SQLITE_OK) {
            sqlite3_free(err_msg);
			std::cout << "sql update friend last seen error" << std::endl;
        	return false;
		}
		
		return true;
	}

	bool tau_shell_sql::db_insert_friend_info()
	{
		return true;
	}

	bool tau_shell_sql::db_update_friend_info()
	{
		return true;
	}

	bool tau_shell_sql::db_add_new_message(const communication::message& msg, int status)
	{
		//hash
	 	std::string hash = aux::to_hex(msg.sha1());
		std::cout << hash << std::endl;

        std::cout << "========New=====Msg========Alert=========" << std::endl;
		//timestamp
		std::int64_t ts = msg.timestamp();
		std::stringstream time_stamp;
		time_stamp << ts;
		std::cout << "Time In Msg: " << time_stamp.str() << std::endl;

		//sender
 		dht::public_key sender_pubkey = msg.sender();
		std::string sender = aux::toHex(sender_pubkey.bytes);
		std::cout << "Sender In Msg: " << sender << std::endl;

		//receiver
 		dht::public_key receiver_pubkey = msg.receiver();
		std::string receiver = aux::toHex(receiver_pubkey.bytes);
		std::cout << "Receiver In Msg: " << receiver << std::endl;

		//payload
		std::string payload;
		aux::bytes p = msg.payload();
		payload.insert(payload.end(), p.begin(), p.end());
		std::cout << "Payload In Msg: " << payload << std::endl;
        std::cout << "=========================================" << std::endl;

		/*
	      insert into ChatMessages		
		*/
        std::string sql = "INSERT INTO ChatMessages VALUES(";
		sql += "\"" + hash + "\", \"" + 
			   sender + "\", \"" +
			   receiver + "\", " +
			   time_stamp.str() + ", 0, \"" +
			   hash + "\", 0, \"" + 
			   payload + "\")";
			
		std::cout << sql << std::endl;

		char *err_msg = nullptr;
        int ok = sqlite3_exec(m_db, sql.data(), nullptr, nullptr, &err_msg);
        if (ok != SQLITE_OK) {
            sqlite3_free(err_msg);
			std::cout << "sql insert new msg error" << std::endl;
        	return false;
		}
		
		/*
		TODO: update friend last comm time
		*/
	
		//status
		std::stringstream ss_status;
		ss_status << status;
		std::cout << ss_status.str() << std::endl;

		/*
	      insert into ChatMsgLOgs		
		*/
        sql = "INSERT INTO ChatMsgLogs VALUES(";
		sql += "\"" + hash + "\", " + 
			   ss_status.str() + ", " +
			   time_stamp.str() + ")";
			
		std::cout << sql << std::endl;

        ok = sqlite3_exec(m_db, sql.data(), nullptr, nullptr, &err_msg);
        if (ok != SQLITE_OK) {
            sqlite3_free(err_msg);
			std::cout << "sql insert new msg chatmsglogs error" << std::endl;
        	return false;
		}
		
		return true;
	}

	bool tau_shell_sql::db_update_message_status(const sha1_hash& msg_hash, std::int64_t time, int status) {

		//hash
	 	std::string hash = aux::to_hex(msg_hash);
        //cout confirm root
        if(2==status) {
            auto now = std::chrono::system_clock::now(); 
            auto now_c = std::chrono::system_clock::to_time_t(now); 
            std::cout << std::put_time(std::localtime(&now_c), "%c") << " Confirm-msg-hash: " << hash << std::endl;
        }

        /*
		//time
		std::stringstream ss_time;
		ss_time << time;
		std::cout << ss_time.str() << std::endl;

		//status
		std::stringstream ss_status;
		ss_status << status;
		std::cout << ss_status.str() << std::endl;

	    //update ChatMsgLOgs		
        std::string sql = "UPDATE ChatMsgLogs SET status = ";
		sql += ss_status.str() + ", timestamp = " +
			   ss_time.str() + " WHERE hash = \"" +
			   hash + "\"";
			
		std::cout << sql << std::endl;

		char *err_msg = nullptr;
        int ok = sqlite3_exec(m_db, sql.data(), nullptr, nullptr, &err_msg);
        if (ok != SQLITE_OK) {
            sqlite3_free(err_msg);
			std::cout << "sql update new msg chatmsglogs error" << std::endl;
        	return false;
		}
	    */	
		return true;
	}

	bool tau_shell_sql::db_follow_chain(const std::string& chain_id, std::set<dht::public_key> peers)
	{

        std::string sql = "INSERT INTO Communities(chainID, communityName, headBlock, tailBlock, consensusBlock) VALUES( \"" + chain_id + "\", "+ 
								  "\"TAU_SHELL_CHAIN\", 0, 0, 0)";
		std::cout << "db follow chain: " << sql << std::endl;

		char *err_msg = nullptr;
        int ok = sqlite3_exec(m_db, sql.data(), nullptr, nullptr, &err_msg);
        if (ok != SQLITE_OK) {
            sqlite3_free(err_msg);
			std::cout << "sql insert new community error" << std::endl;
        	return false;
		}

		// update members
		for(auto i = peers.begin(); i != peers.end(); i++) {
			dht::public_key peer = *i;
			std::string key = aux::toHex(peer.bytes);
			std::cout << key << std::endl;
			sql = "INSERT INTO Members VALUES(";
			sql += "\"" + chain_id + "\", \"" + 
			   key + "\", 0, 0, 0, 0)";
			std::cout << sql << std::endl;
			char *err_msg = nullptr;
        	int ok = sqlite3_exec(m_db, sql.data(), nullptr, nullptr, &err_msg);
        	if (ok != SQLITE_OK) {
            	sqlite3_free(err_msg);
				std::cout << "sql insert new member error" << std::endl;
			}
		}

        return true;
	}

	bool tau_shell_sql::db_unfollow_chain(const std::string& chain_id)
	{

        std::string sql = "DELETE FROM Communities WHERE chainID=";
		sql += "\"" + chain_id + "\"";
		std::cout << sql << std::endl;

		char *err_msg = nullptr;
        int ok = sqlite3_exec(m_db, sql.data(), nullptr, nullptr, &err_msg);
        if (ok != SQLITE_OK) {
            sqlite3_free(err_msg);
			std::cout << "sql delete community error" << std::endl;
        	return false;
		}

		// update members
		sql = "DELETE FROM Members WHERE chainID=";
		sql += "\"" + chain_id + "\"";
		std::cout << sql << std::endl;
       	ok = sqlite3_exec(m_db, sql.data(), nullptr, nullptr, &err_msg);
       	if (ok != SQLITE_OK) {
           	sqlite3_free(err_msg);
			std::cout << "sql delete members error" << std::endl;
		}

        return true;
	}

	bool tau_shell_sql::db_add_new_transaction(const blockchain::transaction& tx, int status, std::int64_t block_number)
	{
	 	std::string hash = aux::to_hex(tx.sha1());
		std::cout << "add new tx, hash: " << hash  << " status: " << status << " bn: " << block_number << std::endl;

		//chain_id
        auto id = tx.chain_id();

        if(id.size()==0)
            return false;

		std::string chain_id = bytes_chain_id_to_string(id.data(), id.size());

		//timestamp
		std::int64_t tt = tx.timestamp();
		std::stringstream tx_time;
		tx_time << tt;

		//fee
		std::int64_t tf = tx.fee();
		std::stringstream tx_fee;
		tx_fee << tf;

		//amount
		std::int64_t ta = tx.amount();
		std::stringstream tx_amount;
		tx_amount << ta;
	
		//nonce
		std::int64_t tn = tx.nonce();
		std::stringstream tx_nonce;
		tx_nonce << tn;
			
		//sender
 		dht::public_key sender_pubkey = tx.sender();
		std::string sender = aux::toHex(sender_pubkey.bytes);

		//receiver
 		dht::public_key receiver_pubkey = tx.receiver();
		std::string receiver = aux::toHex(receiver_pubkey.bytes);

        //type
        int type = tx.type();
		std::stringstream tx_type;
        tx_type << type;

		//payload
		aux::bytes p = tx.payload();
		std::string payload = aux::toHex(p);

		//status
		std::stringstream tx_status;
		tx_status << status;

		//blocknumber
		std::stringstream tx_bn;
		tx_bn << block_number;

        std::string sql = "INSERT INTO Txs VALUES(";
		sql += "\"" + hash + "\", \"" + 
			   chain_id + "\", \"" +
			   sender + "\", " +
			   tx_fee.str() + ", " + 
               tx_time.str() + ", " +
			   tx_nonce.str() + ", " + 
			   tx_type.str() + ", \"" + 
			   payload + "\", " +
			   tx_status.str() + ", "+
			   tx_bn.str()+ ", \"" +
			   receiver + "\", " +
			   tx_amount.str() + 
               ", 0, \"TAU\", \"TAU\", \"TAU\")";

		std::cout << "add new tx sql: " << sql << std::endl;

		char *err_msg = nullptr;
        int ok = sqlite3_exec(m_db, sql.data(), nullptr, nullptr, &err_msg);
        if (ok != SQLITE_OK) {
            sqlite3_free(err_msg);
			std::cout << "sql insert new tx error" << std::endl;
        	return false;
		}

		return true;
	}

	bool tau_shell_sql::db_add_new_block(const blockchain::block& blk)
	{
		//hash
	 	std::string hash = aux::to_hex(blk.sha1());
		std::cout << hash << std::endl;

		//chain_id
        auto id = blk.chain_id();
		std::string chain_id = bytes_chain_id_to_string(id.data(), id.size());
        std::cout << chain_id << std::endl;

		//timestamp
		std::int64_t bt = blk.timestamp();
		std::stringstream blk_time;
		blk_time << bt;
		std::cout << blk_time.str() << std::endl;

		//blocknumber
		std::int64_t bn = blk.block_number();
		std::stringstream blk_number;
		blk_number << bn;
		std::cout << blk_number.str() << std::endl;

		//difficulty
		std::uint64_t bcd = blk.cumulative_difficulty();
		std::stringstream blk_cd;
		blk_cd << bcd;
		std::cout << blk_cd.str() << std::endl;
			
		//miner
 		dht::public_key miner_pubkey = blk.miner();
		std::string miner = aux::toHex(miner_pubkey.bytes);
		std::cout << miner << std::endl;

		//tx
		blockchain::transaction tx = blk.tx();
		this->db_add_new_transaction(tx, 1, bn);

		//reward
		std::int64_t br = tx.fee();
		std::stringstream blk_reward;
		blk_reward << br;
		std::cout << blk_reward.str() << std::endl;

        std::string sql = "INSERT INTO Blocks VALUES(";
		sql += "\"" + chain_id + "\", \"" + 
			   hash + "\", " +
			   blk_number.str() + ", \"" + 
               miner+ "\", " +
			   blk_reward.str() + ", " + 
			   blk_cd.str() + ", 0)";
		std::cout << "add new head block sql: " << sql << std::endl;

		char *err_msg = nullptr;
        int ok = sqlite3_exec(m_db, sql.data(), nullptr, nullptr, &err_msg);
        if (ok != SQLITE_OK) {
            sqlite3_free(err_msg);
			std::cout << "sql insert new blk error" << std::endl;
        	return false;
		}

		return true;
	}

	bool tau_shell_sql::db_delete_block(const blockchain::block& blk)
	{
		//hash
	 	std::string hash = aux::to_hex(blk.sha1());
		std::cout << "delete block when rollback, hash: " << hash << std::endl;

		//tx
		blockchain::transaction tx = blk.tx();
		this->db_delete_transaction(tx);

        std::string sql = "DELETE FROM Blocks Where blockHash=\"" + hash + "\"";
		std::cout << "delete block when rollback, sql: " << sql << std::endl;

		char *err_msg = nullptr;
        int ok = sqlite3_exec(m_db, sql.data(), nullptr, nullptr, &err_msg);
        if (ok != SQLITE_OK) {
            sqlite3_free(err_msg);
			std::cout << "sql delete blk error" << std::endl;
        	return false;
		}

		return true;
	}

	bool tau_shell_sql::db_delete_transaction(const blockchain::transaction& tx)
    {
	 	std::string hash = aux::to_hex(tx.sha1());
		std::cout << "delete tx when rollback, hash: " << hash << std::endl;

        std::string sql = "DELETE FROM Txs Where txID=\"" + hash + "\"";
		std::cout << "delete tx when rollback, sql: " << sql << std::endl;

		char *err_msg = nullptr;
        int ok = sqlite3_exec(m_db, sql.data(), nullptr, nullptr, &err_msg);
        if (ok != SQLITE_OK) {
            sqlite3_free(err_msg);
			std::cout << "sql delete tx error" << std::endl;
        	return false;
		}
        return true;
    }

	bool tau_shell_sql::db_update_community_status(const blockchain::block& blk, int type)
	{
		//chain_id
        auto id = blk.chain_id();
		std::string chain_id = bytes_chain_id_to_string(id.data(), id.size());
        std::cout << chain_id << std::endl;

		//blocknumber
		std::int64_t bn = blk.block_number();
		std::string block_hash = aux::to_hex(blk.sha1());
		std::stringstream blk_number;
		blk_number << bn;
		std::cout << blk_number.str() << std::endl;

        std::string sql;
		if(0 == type) {
        	sql = "UPDATE Communities SET headBlock = ";
		    sql += blk_number.str() + ", headBlockHash=\"" + block_hash + "\" WHERE chainID=\"";
		} else if (1 == type) {
        	sql = "UPDATE Communities SET tailBlock = ";
		    sql += blk_number.str() + ", tailBlockHash=\"" + block_hash + "\" WHERE chainID=\"";
		} else {
        	sql = "UPDATE Communities SET consensusBlock = ";
		    sql += blk_number.str() + ", consensusBlockHash=\"" + block_hash + "\" WHERE chainID=\"";
		}

        sql += chain_id + "\"";

		std::cout << sql << std::endl;

		char *err_msg = nullptr;
        int ok = sqlite3_exec(m_db, sql.data(), nullptr, nullptr, &err_msg);
        if (ok != SQLITE_OK) {
            sqlite3_free(err_msg);
			std::cout << "sql update community status error" << std::endl;
        	return false;
		}

		return true;
	}

	bool tau_shell_sql::db_get_chain_state(const std::string& chain_id, int* block_number, std::string* block_hash)
	{
		//chain_id
        std::string sql;
      	sql = "SELECT headBlock, consensusBlock, tailBlock, headBlockHash, consensusBlockHash, tailBlockHash FROM Communities WHERE chainID =\"" + chain_id + "\"";
		std::cout << sql << std::endl;

        sqlite3_stmt * stmt;
        int ok = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr);
        if (ok == SQLITE_OK) {
            std::cout << "In prepare" << std::endl;
            for (;sqlite3_step(stmt) == SQLITE_ROW;) {
                std::cout << "In prepare 0" << std::endl;
                block_number[0] = sqlite3_column_int(stmt, 0);
                block_number[1] = sqlite3_column_int(stmt, 1);
                block_number[2] = sqlite3_column_int(stmt, 2);

                const unsigned char *hash = sqlite3_column_text(stmt, 3);
                auto length = sqlite3_column_bytes(stmt, 3);
                std::string value1(hash, hash + length);
                block_hash[0] = value1;
                std::cout << "hash 0: " << block_hash[0] << std::endl;

                hash = sqlite3_column_text(stmt, 4);
                length = sqlite3_column_bytes(stmt, 4);
                std::string value2(hash, hash + length);
                block_hash[1] = value2;
                std::cout << "hash 1: " << block_hash[1] << std::endl;

                hash = sqlite3_column_text(stmt, 5);
                length = sqlite3_column_bytes(stmt, 5);
                std::string value3(hash, hash + length);
                block_hash[2] = value3;
                std::cout << "hash 2: " << block_hash[2] << std::endl;
            }
        } 
        sqlite3_finalize(stmt);

		return true;
	}

	int tau_shell_sql::db_get_tx_state(const std::string& tx_hash)
	{
		//chain_id
        std::string sql;
      	sql = "SELECT txID FROM Txs WHERE txID =\"" + tx_hash + "\"";
		std::cout << "get tx state sql: " << sql << std::endl;

        sqlite3_stmt * stmt;
        int ok = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr);
        if (ok == SQLITE_OK) {
            for (;sqlite3_step(stmt) == SQLITE_ROW;) {
                const unsigned char *hash = sqlite3_column_text(stmt, 0);
                auto length = sqlite3_column_bytes(stmt, 0);
                std::string value1(hash, hash + length);
                return 1;
            }
        }
        sqlite3_finalize(stmt);
		return 0;
	}
}
