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

#ifndef TORRENT_TRANSMISSION_WEBUI_HPP
#define TORRENT_TRANSMISSION_WEBUI_HPP

#define INT64_FMT  "I64d"

#include "rpc/auth.hpp"
#include "rpc/webui.hpp"
#include "util/db_util.hpp"

#include "libTAU/session.hpp"
#include "libTAU/kademlia/types.hpp"

extern "C" {
#include "util/jsmn.h"
}

#include <vector>
#include <set>

using namespace libTAU;
namespace libTAU
{
	struct tau_handler : http_handler
	{
		tau_handler(session& s, tau_shell_sql* sqldb, auth_interface const* auth, dht::public_key& pubkey, dht::secret_key& seckey);
		~tau_handler();

		virtual bool handle_http(mg_connection* conn,
			mg_request_info const* request_info);

		//获取当前sesstion状态
		void session_stats(std::vector<char>&, jsmntok_t* args, std::int64_t tag, char* buffer);

		//communication apis
		void new_account_seed(std::vector<char>&, jsmntok_t* args, std::int64_t tag, char* buffer);

		// main loop time interval
		void set_loop_time_interval(std::vector<char>&, jsmntok_t* args, std::int64_t tag, char* buffer);

		//friends
		void add_new_friend(std::vector<char>&, jsmntok_t* args, std::int64_t tag, char* buffer);

		void delete_friend(std::vector<char>&, jsmntok_t* args, std::int64_t tag, char* buffer);

		void get_friend_info(std::vector<char>&, jsmntok_t* args, std::int64_t tag, char* buffer);

		void update_friend_info(std::vector<char>&, jsmntok_t* args, std::int64_t tag, char* buffer);

		// message
		void add_new_message(std::vector<char>&, jsmntok_t* args, std::int64_t tag, char* buffer);

		//blockchain
		void create_chain_id(std::vector<char>&, jsmntok_t* args, std::int64_t tag, char* buffer);

		void create_new_community(std::vector<char>&, jsmntok_t* args, std::int64_t tag, char* buffer);

		void follow_chain(std::vector<char>&, jsmntok_t* args, std::int64_t tag, char* buffer);

		void follow_chain_mobile(std::vector<char>&, jsmntok_t* args, std::int64_t tag, char* buffer);

		void unfollow_chain(std::vector<char>&, jsmntok_t* args, std::int64_t tag, char* buffer);

		void unfollow_chain_mobile(std::vector<char>&, jsmntok_t* args, std::int64_t tag, char* buffer);

		void submit_note_transaction(std::vector<char>&, jsmntok_t* args, std::int64_t tag, char* buffer);

		void submit_transaction(std::vector<char>&, jsmntok_t* args, std::int64_t tag, char* buffer);

		void get_account_info(std::vector<char>&, jsmntok_t* args, std::int64_t tag, char* buffer);

		void get_top_tip_block(std::vector<char>&, jsmntok_t* args, std::int64_t tag, char* buffer);

		void get_median_tx_fee(std::vector<char>&, jsmntok_t* args, std::int64_t tag, char* buffer);

		void get_block_by_number(std::vector<char>&, jsmntok_t* args, std::int64_t tag, char* buffer);

		void get_block_by_hash(std::vector<char>&, jsmntok_t* args, std::int64_t tag, char* buffer);

		void get_chain_state(std::vector<char>&, jsmntok_t* args, std::int64_t tag, char* buffer);

		void get_tx_state(std::vector<char>&, jsmntok_t* args, std::int64_t tag, char* buffer);

        void send_data(std::vector<char>& buf, jsmntok_t* args, std::int64_t tag, char* buffer);

	private:

		void handle_json_rpc(std::vector<char>& buf, jsmntok_t* tokens, char* buffer);

		time_t m_start_time;
		session& m_ses;
		dht::public_key& m_pubkey;
		dht::secret_key& m_seckey;
		tau_shell_sql* m_db;
		auth_interface const* m_auth;
	};
}

#endif
