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

#include "rpc/webui.hpp"
#include "libTAU/session.hpp"

extern "C" {
#include "rpc/jsmn.h"
}

#include <vector>
#include <set>

using namespace libTAU;
namespace libTAU
{
	struct tau_communication_webui : http_handler
	{
		tau_communication_webui(session& s);
		~tau_communication_webui();

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

		void set_chatting_friend(std::vector<char>&, jsmntok_t* args, std::int64_t tag, char* buffer);

		void unset_chatting_friend(std::vector<char>&, jsmntok_t* args, std::int64_t tag, char* buffer);

		void update_friend_info(std::vector<char>&, jsmntok_t* args, std::int64_t tag, char* buffer);

		void set_active_friends(std::vector<char>&, jsmntok_t* args, std::int64_t tag, char* buffer);

		// message
		void add_new_message(std::vector<char>&, jsmntok_t* args, std::int64_t tag, char* buffer);

	private:

		void handle_json_rpc(std::vector<char>& buf, jsmntok_t* tokens, char* buffer);

		time_t m_start_time;
		session& m_ses;
	};
}

#endif
