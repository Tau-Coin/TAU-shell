#include <iostream>
#include <filesystem>
#include <sqlite3.h>

#include "rpc/webui.hpp"
#include "handler/alert_handler.hpp"
#include "handler/tau_handler.hpp"
#include "util/hex_util.hpp"
#include "util/db_util.hpp"
#include "util/tau_constants.hpp"

#include "libTAU/aux_/ed25519.hpp"
#include "libTAU/alert.hpp"
#include "libTAU/alert_types.hpp"
#include "libTAU/error_code.hpp"
#include "libTAU/session.hpp"
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
    {"initial",              no_argument,          NULL, 'i'},
    {"listen-port",       required_argument,   NULL, 'l'},
    {"rpc-port",          required_argument,   NULL, 'r'},
    {"shell-save-dir",required_argument,   NULL, 's'},
    {"libTAU-save-dir",   no_argument,         NULL, 't'},
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
        "-i, --initial\n"
        "-l, --listen-port      <libTAU listen port>\n"
        "-r, --rpc-port         <rpc listen port>\n"
        "-s, --shell-save-dir   <shell download directory>\n"
        "-t, --libTAU-save-dir  <libTAU download directory>\n"
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
    bool initial = false;
    int listen_port = 6881;
    int rpc_port = 8080;

    std::string config_file;
    std::string pid_file;
    std::string shell_save_path;
    std::string tau_save_path=".libTAU";
    std::string error_log;
    std::string debug_log;

    int ch = 0;
    while ((ch = getopt_long(argc, argv, "c:p:d:i:l:r:s:t:e:u:", cmd_line_options, NULL)) != -1)
    {
        std::cout << ch << std::endl;
        switch (ch)
        {
            case 'c': config_file = optarg; break;
            case 'p': pid_file = optarg; break;
            case 'd': daemonize = true; break;
            case 'i': initial = true; break;
            case 'l': listen_port = atoi(optarg); break;
            case 'r': rpc_port = atoi(optarg); break;
            case 's': shell_save_path = optarg; break;
            case 't': tau_save_path = optarg; break;
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
    std::cout << "save path: " << shell_save_path << std::endl;
    std::cout << "error log file: " << error_log << std::endl;
    std::cout << "debug log file: " << debug_log << std::endl;
    std::cout << "Initial CMD Parameters Over" << std::endl;

    error_code ec;
    auth authorizer;
    ec.clear();
    authorizer.load_accounts("users.conf", ec);
    if (ec)
        authorizer.add_account("tau-shell", "tester", 0);
    ec.clear();

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

	//处理seed
	char* seed = new char[KEY_LEN];
	char* pubkey = new char[KEY_LEN];
	char* seckey = new char[KEY_HEX_LEN];
	hex_char_to_bytes_char(account_seed, seed, KEY_HEX_LEN);
	aux::ed25519_create_keypair(reinterpret_cast<unsigned char *>(pubkey), 
								reinterpret_cast<unsigned char *>(seckey), 
								reinterpret_cast<unsigned char const*>(account_seed));
	dht::public_key m_pubkey(pubkey);	
	dht::secret_key m_seckey(seckey);	

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
    std::string const& sqldb_dir = home_dir + shell_save_path + "/TAU";
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
    tau_shell_sql tau_sql(sqldb_path);

    // initial sqlite3
    if(initial) {
        tau_sql.sqlite_db_initial();
        std::cout << "Sqlite3 DB initial success" << std::endl;
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

    //listen port
    std::stringstream listen_interfaces;
    //std::string listen_interfaces = "0.0.0.0:6881,[::]:6881";
    listen_interfaces << "0.0.0.0:" << listen_port << ",[::]:" << listen_port;
    std::cout << "listen port: " << listen_interfaces.str() << std::endl;
    sp_set.set_str(settings_pack::listen_interfaces, listen_interfaces.str());

    //tau save path
    std::cout << "libTAU save paht: " << tau_save_path << std::endl;
    sp_set.set_str(settings_pack::db_dir, tau_save_path.c_str());

    session_params sp_param(sp_set) ;
    
    session ses(sp_param);
    ses.set_alert_mask(~(alert::progress_notification | alert::debug_notification));    
    std::cout << "Session parameters' setting Over" << std::endl;

    //定义tau communication handle
    tau_handler t_handler(ses, &tau_sql, &authorizer, m_pubkey, m_seckey);
    alert_handler a_handler(&tau_sql);

    //定义启动webui
    webui_base webport;
    webport.add_handler(&t_handler);
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
            std::cout << (*i)->message().c_str() << std::endl;
            //std::cout << (*i)->type() <<  " " << log_alert::alert_type << std::endl;
            int alert_type = (*i)->type();
            switch(alert_type){
                case log_alert::alert_type: 
                    fprintf(debug_file, " %s\n", (*i)->message().c_str());
                    break;
                case dht_log_alert::alert_type:
                    fprintf(debug_file, " %s\n", (*i)->message().c_str());
                    break;
				//communication
                case communication_new_device_id_alert::alert_type:
                    a_handler.alert_on_new_device_id(*i);
                    break;
                case communication_new_message_alert::alert_type:
                    a_handler.alert_on_new_message(*i);
                    break;
                case communication_confirmation_root_alert::alert_type:
                    a_handler.alert_on_confirmation_root(*i);
                    break;
                case communication_syncing_message_alert::alert_type:
                    a_handler.alert_on_syncing_message(*i);
                    break;
                case communication_friend_info_alert::alert_type:
                    a_handler.alert_on_friend_info(*i);
                    break;
                case communication_last_seen_alert::alert_type:
                    a_handler.alert_on_last_seen(*i);
                    break;
                case communication_log_alert::alert_type:
                    fprintf(debug_file, " %s\n", (*i)->message().c_str());
                    break;
				//blockchain
                case blockchain_new_head_block_alert::alert_type:
                    a_handler.alert_on_new_head_block(*i);
                    break;
                case blockchain_new_tail_block_alert::alert_type:
                    a_handler.alert_on_new_tail_block(*i);
                    break;
                case blockchain_new_consensus_point_block_alert::alert_type:
                    a_handler.alert_on_new_consensus_point_block(*i);
                    break;
				case blockchain_new_transaction_alert::alert_type:
					a_handler.alert_on_new_transaction(*i);
                    break;
				//blockchain-useless, current
                case blockchain_rollback_block_alert::alert_type:
                    a_handler.alert_on_rollback_block(*i);
                    break;
                case blockchain_fork_point_block_alert::alert_type:
                    a_handler.alert_on_fork_point_block(*i);
                    break;
                case blockchain_top_three_votes_alert::alert_type:
                    a_handler.alert_on_top_three_votes(*i);
                    break;
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

