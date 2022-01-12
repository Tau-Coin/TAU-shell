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

#include "handler/hex_util.hpp"
#include "handler/tau_constants.hpp"
#include "handler/tau_handler.hpp"
#include "rpc/json_util.hpp"
#include "rpc/base64.hpp"

#include <string.h> // for strcmp()
#include <stdio.h>
#include <vector>
#include <map>
#include <cstdint>
#include <boost/tuple/tuple.hpp>
#include <boost/asio/error.hpp>

extern "C" {
#include "rpc/local_mongoose.h"
#include "rpc/jsmn.h"
}

#include "libTAU/session.hpp"
#include "libTAU/session_status.hpp"
#include "handler/response_buffer.hpp" // for appendf
#include "handler/escape_json.hpp" // for escape_json

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

		if (args) buffer[args->end] = 0;
		printf("%s: %s\n", m, args ? buffer + args->start : "{}");

		(this->*handlers[i].fun)(buf, args, tag, buffer);
		std::cout << "Method Over" << std::endl;
		break;
	}

	if (!handled)
		printf("Unhandled: %s: %s\n", m, args ? buffer + args->start : "{}");

}

char const* to_bool(bool b) { return b ? "true" : "false"; }

void tau_handler::session_stats(std::vector<char>& buf, jsmntok_t* args, std::int64_t tag, char* buffer)
{
	// TODO: post session stats instead, and capture the performance counters
	std::cout << "Session Stats In TAU WebUI 0" << std::endl;
	session_status st = m_ses.status();
	std::cout << "Session Stats In TAU WebUI 1" << std::endl;

	appendf(buf, "{ \"result\": \"success\", \"tag\": %" "I64d" ", "
		"\"arguments\": { "
		"\"uploadSpeed\": %d,"
		"\"cumulative-stats\": {"
			"\"uploadedBytes\": %" "I64d" ","
			"\"downloadedBytes\": %" "I64d" ","
			"\"sessionCount\": %d,"
			"\"secondsActive\": %d"
			"},"
		"\"current-stats\": {"
			"\"uploadedBytes\": %" "I64d" ","
			"\"downloadedBytes\": %" "I64d" ","
			"\"sessionCount\": %d,"
			"\"secondsActive\": %d"
			"}"
		"}}", tag
		, st.payload_upload_rate
		// cumulative-stats (not supported)
		, st.total_payload_upload
		, st.total_payload_download
		, 1
		, time(nullptr) - m_start_time
		// current-stats
		, st.total_payload_upload
		, st.total_payload_download
		, 1
		, time(nullptr) - m_start_time);
}

//communication apis
void tau_handler::new_account_seed(std::vector<char>&, jsmntok_t* args, std::int64_t tag, char* buffer)
{

}

// main loop time interval
void tau_handler::set_loop_time_interval(std::vector<char>&, jsmntok_t* args, std::int64_t tag, char* buffer)
{
	jsmntok_t* ti = find_key(args, buffer, "time-interval", JSMN_PRIMITIVE);
	int time_interval = atoi(buffer + ti->start);
	std::cout << "Set Time Interval: " << time_interval << std::endl;
	m_ses.set_loop_time_interval(time_interval);
}

//friends
void tau_handler::add_new_friend(std::vector<char>&, jsmntok_t* args, std::int64_t tag, char* buffer)
{
	jsmntok_t* f = find_key(args, buffer, "friend", JSMN_STRING);
	buffer[f->end] = 0;
	char const* friend_pubkey_hex_char = &buffer[f->start];
	char* friend_pubkey_char = new char[KEY_LEN];
	hex_char_to_bytes_char(friend_pubkey_hex_char, friend_pubkey_char, KEY_HEX_LEN);
	dht::public_key friend_pubkey(friend_pubkey_char);
	m_ses.add_new_friend(friend_pubkey);
}

void tau_handler::delete_friend(std::vector<char>&, jsmntok_t* args, std::int64_t tag, char* buffer)
{
	jsmntok_t* f = find_key(args, buffer, "friend", JSMN_STRING);
	buffer[f->end] = 0;
	char const* friend_pubkey_hex_char = &buffer[f->start];
	char* friend_pubkey_char = new char[KEY_LEN];
	hex_char_to_bytes_char(friend_pubkey_hex_char, friend_pubkey_char, KEY_HEX_LEN);
	dht::public_key friend_pubkey(friend_pubkey_char);
	m_ses.delete_friend(friend_pubkey);
}

void tau_handler::get_friend_info(std::vector<char>&, jsmntok_t* args, std::int64_t tag, char* buffer)
{
	jsmntok_t* f = find_key(args, buffer, "friend", JSMN_STRING);
	buffer[f->end] = 0;
	char const* friend_pubkey_hex_char = &buffer[f->start];
	char* friend_pubkey_char = new char[KEY_LEN];
	hex_char_to_bytes_char(friend_pubkey_hex_char, friend_pubkey_char, KEY_HEX_LEN);
	dht::public_key friend_pubkey(friend_pubkey_char);
	std::vector<char> info = m_ses.get_friend_info(friend_pubkey);
	std::cout << "Friend Info: " << info.data() << std::endl;
}

void tau_handler::update_friend_info(std::vector<char>&, jsmntok_t* args, std::int64_t tag, char* buffer)
{
	jsmntok_t* f = find_key(args, buffer, "friend", JSMN_STRING);
	jsmntok_t* i = find_key(args, buffer, "info", JSMN_STRING);

	//friend
	buffer[f->end] = 0;
	char const* friend_pubkey_hex_char = &buffer[f->start];
	char* friend_pubkey_char = new char[KEY_LEN];
	hex_char_to_bytes_char(friend_pubkey_hex_char, friend_pubkey_char, KEY_HEX_LEN);
	dht::public_key friend_pubkey(friend_pubkey_char);

	//info
	std::vector<char> friend_info;
	friend_info.insert(friend_info.end(), &buffer[i->start], &buffer[i->end]);

	bool flag = m_ses.update_friend_info(friend_pubkey, friend_info);
}

// message
void tau_handler::add_new_message(std::vector<char>&, jsmntok_t* args, std::int64_t tag, char* buffer)
{
	jsmntok_t* m = find_key(args, buffer, "msg", JSMN_STRING);
}

//blockchain
void tau_handler::create_chain_id(std::vector<char>&, jsmntok_t* args, std::int64_t tag, char* buffer)
{
}

void tau_handler::create_new_community(std::vector<char>&, jsmntok_t* args, std::int64_t tag, char* buffer)
{
}

void tau_handler::follow_chain(std::vector<char>&, jsmntok_t* args, std::int64_t tag, char* buffer)
{
}

void tau_handler::unfollow_chain(std::vector<char>&, jsmntok_t* args, std::int64_t tag, char* buffer)
{
}

void tau_handler::submit_transaction(std::vector<char>&, jsmntok_t* args, std::int64_t tag, char* buffer)
{
}

void tau_handler::get_account_info(std::vector<char>&, jsmntok_t* args, std::int64_t tag, char* buffer)
{
}

void tau_handler::get_top_tip_block(std::vector<char>&, jsmntok_t* args, std::int64_t tag, char* buffer)
{
}

void tau_handler::get_median_tx_fee(std::vector<char>&, jsmntok_t* args, std::int64_t tag, char* buffer)
{
}

void tau_handler::get_block_by_number(std::vector<char>&, jsmntok_t* args, std::int64_t tag, char* buffer)
{
}

void tau_handler::get_block_by_hash(std::vector<char>&, jsmntok_t* args, std::int64_t tag, char* buffer)
{
}

tau_handler::tau_handler(session& s)
	: m_ses(s)
{

}

tau_handler::~tau_handler() {}

bool tau_handler::handle_http(mg_connection* conn, mg_request_info const* request_info)
{
	std::cout << "==============Incoming HTTP ==============" << std::endl;
	// we only provide access to paths under /web and /upload
	if (strcmp(request_info->uri, "/rpc"))
		return false;

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
