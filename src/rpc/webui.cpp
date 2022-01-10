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

#include <deque>
#include <getopt.h> // for getopt_long
#include <stdlib.h> // for daemon()
#include <syslog.h>
#include <boost/unordered_map.hpp>

#include "rpc/webui.hpp"

extern "C" {
#include "rpc/local_mongoose.h"
}

using namespace libTAU;

static int handle_http_request(mg_connection* conn)
{
	const mg_request_info *request_info = mg_get_request_info(conn);
	if (request_info->user_data == NULL) return 0;

	return reinterpret_cast<webui_base*>(request_info->user_data)->handle_http(
		conn, request_info);
}

static int log_message(const mg_connection*, const char* msg)
{
	fprintf(stderr, "%s\n", msg);
	return 1;
}

static void end_request(mg_connection const* c, int reply_status_code)
{
	mg_connection* conn = const_cast<mg_connection*>(c);
	const mg_request_info *request_info = mg_get_request_info(conn);
	if (request_info->user_data == NULL) return;

	reinterpret_cast<webui_base*>(request_info->user_data)->handle_end_request(conn);
}

webui_base::webui_base()
	: m_document_root(".")
	, m_ctx(NULL)
{}

webui_base::~webui_base() {}

void webui_base::remove_handler(http_handler* h)
{
	std::vector<http_handler*>::iterator i = std::find(m_handlers.begin(), m_handlers.end(), h);
	if (i != m_handlers.end()) m_handlers.erase(i);
}

bool webui_base::handle_http(mg_connection* conn
	, mg_request_info const* request_info)
{
	for (std::vector<http_handler*>::iterator i = m_handlers.begin()
		, end(m_handlers.end()); i != end; ++i)
	{
		if ((*i)->handle_http(conn, request_info)) return true;
	}
	return false;
}

void webui_base::handle_end_request(mg_connection* conn)
{
	for (std::vector<http_handler*>::iterator i = m_handlers.begin()
		, end(m_handlers.end()); i != end; ++i)
	{
		(*i)->handle_end_request(conn);
	}
}

bool webui_base::is_running() const
{
	return m_ctx;
}

void webui_base::start(int port, int num_threads)
{
	if (m_ctx) mg_stop(m_ctx);

	m_listen_port = port;

	// start web interface
	char port_str[20];
	snprintf(port_str, sizeof(port_str), "%d", port);
	const char *options[20];
	memset(options, 0, sizeof(options));
	int i = 0;
	options[i++] = "document_root";
	options[i++] = m_document_root.c_str();
	options[i++] = "enable_keep_alive";
	options[i++] = "yes";

	options[i++] = "listening_ports";
	options[i++] = port_str;

	char threads_str[20];
	snprintf(threads_str, sizeof(threads_str), "%d", num_threads);

	options[i++] = "num_threads";
	options[i++] = threads_str;
	options[i++] = NULL;

	mg_callbacks cb;
	memset(&cb, 0, sizeof(cb));
	cb.begin_request = &handle_http_request;
	cb.log_message = &log_message;
	cb.end_request = &end_request;

	m_ctx = mg_start(&cb, this, options);
}

void webui_base::stop()
{
	if (m_ctx) mg_stop(m_ctx);
	m_ctx = NULL;
}

