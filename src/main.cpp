#include <iostream>
#include <filesystem>
#include <leveldb/db.h>
#include <sqlite3.h>

#include "rpc/webui.hpp"
#include "handler/tau_communication_webui.hpp"

#include "libTAU/session.hpp"
#include "libTAU/alert.hpp"
#include "libTAU/alert_types.hpp"
#include "libTAU/session_params.hpp"
#include "libTAU/session_handle.hpp"

#include <signal.h>
#include <unistd.h> // for getpid()
#include <getopt.h> // for getopt_long
#include <stdlib.h> // for daemon()

bool quit = false;
bool force_quit = false;

void sighandler(int s)
{
	quit = true;
}

void sighandler_forcequit(int s)
{
	force_quit = true;
}

using namespace libTAU;

struct option cmd_line_options[] =
{
	{"config",            required_argument,   NULL, 'c'},
	{"pid",               required_argument,   NULL, 'p'},
	{"daemonize",         no_argument,         NULL, 'd'},
	{"listen-port",       required_argument,   NULL, 'l'},
	{"rpc-port",          required_argument,   NULL, 'r'},
	{"save-dir",          required_argument,   NULL, 's'},
	{"error-log",         required_argument,   NULL, 'e'},
	{"debug-log",         required_argument,   NULL, 'u'},
	{"help",              no_argument,         NULL, 'h'},
	{NULL,                0,                   NULL, 0}
};

void print_usage()
{
	fputs("libTAU-daemon usage:\n\n"
		"-c, --config           <config filename>\n"
		"-p, --pid              <pid-filename>\n"
		"-d, --daemonize\n"
		"-l, --listen-port      <libTAU listen port>\n"
		"-r, --rpc-port         <rpc listen port>\n"
		"-s, --save-dir         <download directory>\n"
		"-e, --error-log        <error log filename>\n"
		"-u, --debug-log        <debug log filename>\n"
		"-h, --help\n"
		"\n"
		, stderr);
}

int main(int argc, char *const argv[])
{
	// general configuration of network ranges/peer-classes
	// and storage

	bool daemonize = false;
	int listen_port = 6881;
	int rpc_port = 8080;

	std::string config_file;
	std::string pid_file;
	std::string save_path;
	std::string error_log;
	std::string debug_log;

	int ch = 0;
	while ((ch = getopt_long(argc, argv, "c:p:d::l::r::s::e::u:", cmd_line_options, NULL)) != -1)
	{
		switch (ch)
		{
			case 'c': config_file = optarg; break;
			case 'p': pid_file = optarg; break;
			case 'd': daemonize = true; break;
			case 'l': listen_port = atoi(optarg); break;
			case 'r': rpc_port = atoi(optarg); break;
			case 's': save_path = optarg; break;
			case 'e': error_log = optarg; break;
			case 'u': debug_log = optarg; break;
			default:
				print_usage();
				return 1;
		}
	}

	argc -= optind;
	argv += optind;

	std::cout << "Configure from cmd line: " << std::endl;
	std::cout << "config file: " << config_file << std::endl;
	std::cout << "pid file: " << pid_file << std::endl;
	std::cout << "listen port: " << listen_port << std::endl;
	std::cout << "rpc port: " << rpc_port << std::endl;
	std::cout << "save path: " << save_path << std::endl;
	std::cout << "error log file: " << error_log << std::endl;
	std::cout << "debug log file: " << debug_log << std::endl;
	std::cout << "Initial CMD Parameters Over" << std::endl;

	//读取device_id, account_seed
	char device_id[32];
	char account_seed[64];
	char bootstrap_nodes[1024];
	if(!config_file.empty())
	{
		FILE* f = fopen(config_file.c_str(), "r");
		if(f)
		{
			fscanf(f, "%s\n", device_id);
			fscanf(f, "%s", account_seed);
			fscanf(f, "%s", bootstrap_nodes);
			fclose(f);
		}
		else
		{
			fprintf(stderr, "failed to open config file \"%s\": %s\n"
				, config_file.c_str(), strerror(errno));
			exit(1);
		}
	}

	if (daemonize)
	{
		//输出pid
		if (!pid_file.empty())
		{
			FILE* f = fopen(pid_file.c_str(), "w+");
			if (f)
			{
				fprintf(f, "%d", getpid());
				fclose(f);
			}
			else
			{
				fprintf(stderr, "failed to open pid file \"%s\": %s\n"
					, pid_file.c_str(), strerror(errno));
			}
		}

		//as daemon process
		daemon(1, 0);
	}
	std::cout << "Initial File Parameters Over" << std::endl;

	// open db for message store
	std::string home_dir = std::filesystem::path(getenv("HOME")).string();
	std::string const& sqldb_dir = home_dir + save_path + "/TAU";
	std::string const& sqldb_path = sqldb_dir + "/tau_sql.db";

	// create the directory for storing sqldb data
	if(!std::filesystem::is_directory(sqldb_dir)) {
		if(!std::filesystem::create_directories(sqldb_dir)){
			//报错退出-创建文件失败
			fprintf(stderr, "failed to create db file");
			exit(1);
		}
	}

	// open sqldb - sqlite3
	sqlite3* sqldb;
	int sqlrc = sqlite3_open_v2(sqldb_path.c_str(), &sqldb, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_SHAREDCACHE, NULL);
    if (sqlrc != SQLITE_OK) {
		//报错退出-打开数据库失败
		fprintf(stderr, "failed to open sqlite db");
		exit(1);
	}
	std::cout << "DB File Open Over" << std::endl;

	// 输出debug日志
	FILE* debug_file = NULL;
	if (!debug_log.empty())
	{
		debug_file = fopen(debug_log.c_str(), "w+");
		if (debug_file == NULL)
		{
			fprintf(stderr, "failed to debug log \"%s\": %s\n"
				, debug_log.c_str(), strerror(errno));
			exit(1);
		}
	}

	std::cout << "Log File Open Over" << std::endl;

	//定义session_params
	settings_pack sp_set;

	//bootstrap nodes
	sp_set.set_str(settings_pack::dht_bootstrap_nodes, bootstrap_nodes);

	//device_id
	sp_set.set_str(settings_pack::device_id, device_id);

	//account seed
	sp_set.set_str(settings_pack::account_seed, account_seed);

	session_params sp_param(sp_set) ;
	
	session ses(sp_param);
	ses.set_alert_mask(~(alert::progress_notification | alert::debug_notification));	
	std::cout << "Session parameters' setting Over" << std::endl;

	//定义tau communication handle
	tau_communication_webui tc_handler(ses);

	//定义启动webui
	webui_base webport;
    webport.add_handler(&tc_handler);
    webport.start(rpc_port, 30);
    if (!webport.is_running())
    {
		fprintf(stderr, "failed to start web server\n");
		return 1;
	}
	std::cout << "Web UI RPC Start Over" << std::endl;

	signal(SIGTERM, &sighandler);
	signal(SIGINT, &sighandler);
	signal(SIGPIPE, SIG_IGN);

	std::vector<alert*> alert_queue;
	bool shutting_down = false;
	while (!quit)
	{
		ses.pop_alerts(&alert_queue);

		for (std::vector<alert*>::iterator i = alert_queue.begin()
			, end(alert_queue.end()); i != end; ++i)
		{
			//std::cout << (*i)->message().c_str() << std::endl;
			//std::cout << (*i)->type() << std::endl;
			int alert_type = (*i)->type();
			switch(alert_type){
				case log_alert::alert_type: 
					fprintf(debug_file, " %s\n", (*i)->message().c_str());
					break;
				case dht_log_alert::alert_type:
					fprintf(debug_file, " %s\n", (*i)->message().c_str());
					break;
				case communication_new_device_id_alert::alert_type:
					//on_new_device_id(*i, db);
					break;
				case communication_new_message_alert::alert_type:
					//on_new_message(*i, db);
					break;
				case communication_confirmation_root_alert::alert_type:
					//on_confirmation_root(*i, db);
					break;
				case communication_syncing_message_alert::alert_type:
					//on_syncing_message(*i, db);
					break;
				case communication_friend_info_alert::alert_type:
					//on_friend_info(*i, db);
					break;
				case communication_last_seen_alert::alert_type:
					//on_last_seen(*i, db);
					break;
			}
		}

		if (debug_file)
		{
			for (std::vector<alert*>::iterator i = alert_queue.begin()
				, end(alert_queue.end()); i != end; ++i)
			{
				fprintf(debug_file, " %s\n", (*i)->message().c_str());
			}
		}

		if (quit && !shutting_down)
		{
			shutting_down = true;
			signal(SIGTERM, &sighandler_forcequit);
			signal(SIGINT, &sighandler_forcequit);
		}
		if (force_quit) break;
		usleep(100000);
	}
	std::cout << "Cycle Over" << std::endl;

	if (debug_file) fclose(debug_file);

	std::cout << "Total Over" << std::endl;

	return 0;
}

