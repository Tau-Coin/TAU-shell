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
#include "libTAU/kademlia/ed25519.hpp"
#include "libTAU/alert.hpp"
#include "libTAU/alert_types.hpp"
#include "libTAU/error_code.hpp"
#include "libTAU/hex.hpp"
#include "libTAU/session.hpp"
#include "libTAU/session_params.hpp"
#include "libTAU/session_handle.hpp"

#include <signal.h>
#include <unistd.h> // for getpid()
#include <getopt.h> // for getopt_long
#include <stdlib.h> // for daemon()

const int FILE_LEN = 256;

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
    {"daemonize",         no_argument,         NULL, 'd'},
    {"initial",              no_argument,          NULL, 'i'},
    {"help",              no_argument,         NULL, 'h'},
};

void print_usage()
{
    fputs("libTAU-daemon usage:\n\n"
        "-c, --config           <config filename>\n"
        "-d, --daemonize\n"
        "-i, --initial\n"
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

    std::string config_file;
    int ch = 0;
    while ((ch = getopt_long(argc, argv, "c:d:i", cmd_line_options, NULL)) != -1)
    {
        switch (ch)
        {
            case 'c': config_file = optarg; break;
            case 'd': daemonize = true; break;
            case 'i': initial = true; break;
            default:
                print_usage();
                return 1;
        }
    }

    std::cout << "Configure from cmd line: " << std::endl;
    std::cout << "config file: " << config_file << std::endl;

    error_code ec;
    auth authorizer;
    ec.clear();
    authorizer.load_accounts("users.conf", ec);
    if (ec)
        authorizer.add_account("tau-shell", "tester", 0);
    ec.clear();

    //读取device_id, account_seed
    char device_id[KEY_LEN + 1]={}; //used for '\0'
    char account_seed[KEY_HEX_LEN + 1]={}; //used for '\0'
    char bootstrap_nodes[1024]={};

    char pid_file[FILE_LEN] = {};
    char error_log[FILE_LEN] = {};
    char debug_log[FILE_LEN] = {};

    int listen_port = 6881;
    int rpc_port = 8080;

    char shell_save_path[FILE_LEN] = {};
    char tau_save_path[FILE_LEN] = {};

    if(!config_file.empty())
    {
        FILE* f = fopen(config_file.c_str(), "r");
        if(f)
        {
            fscanf(f, "%s\n %s\n %s\n", device_id, account_seed, bootstrap_nodes);
            fscanf(f, "%s\n %s\n %s\n", pid_file, error_log, debug_log);
            fscanf(f, "%d\n %d\n", &listen_port, &rpc_port);
            fscanf(f, "%s\n %s\n", shell_save_path, tau_save_path);
            fclose(f);
        }
        else
        {
            fprintf(stderr, "failed to open config file \"%s\": %s\n"
                , config_file.c_str(), strerror(errno));
            exit(1);
        }
    }

    std::cout << "pid file: " << pid_file << std::endl;
    std::cout << "listen port: " << listen_port << std::endl;
    std::cout << "rpc port: " << rpc_port << std::endl;
    std::cout << "shell save path: " << shell_save_path << std::endl;
    std::cout << "tau save path: " << tau_save_path << std::endl;
    std::cout << "error log file: " << error_log << std::endl;
    std::cout << "debug log file: " << debug_log << std::endl;
    std::cout << "Initial CMD Parameters Over" << std::endl;

	//处理seed
	char* seed = new char[KEY_LEN];
	char* pubkey = new char[KEY_LEN];
	char* seckey = new char[KEY_HEX_LEN];
    if(!strcmp(account_seed, "null")){
        //产生随机数
        auto array_seed = dht::ed25519_create_seed();
        seed = array_seed.data();
        aux::to_hex(seed, KEY_LEN, account_seed);

    } else {
	    hex_char_to_bytes_char(account_seed, seed, KEY_HEX_LEN);
	    aux::ed25519_create_keypair(reinterpret_cast<unsigned char *>(pubkey), 
								reinterpret_cast<unsigned char *>(seckey), 
								reinterpret_cast<unsigned char const*>(account_seed));
    }
	dht::public_key m_pubkey(pubkey);	
	dht::secret_key m_seckey(seckey);	

    if (daemonize)
    {
        //输出pid
        if (strlen(pid_file) > 0)
        {
            FILE* f = fopen(pid_file, "w+");
            if (f)
            {
                fprintf(f, "%d", getpid());
                fclose(f);
            }
            else
            {
                fprintf(stderr, "failed to open pid file \"%s\": %s\n"
                    , pid_file, strerror(errno));
            }
        }

        //as daemon process

        daemon(1, 0);
    }
    std::cout << "Initial File Parameters Over" << std::endl;

    // open db for message store
    std::string home_dir = std::filesystem::path(getenv("HOME")).string();
    std::string const& sqldb_dir = home_dir + shell_save_path;
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
    if (strlen(debug_log) > 0)
    {
        debug_file = fopen(debug_log, "w+");
        if (debug_file == NULL)
        {
            fprintf(stderr, "failed to debug log \"%s\": %s\n"
                , debug_log, strerror(errno));
            exit(1);
        }
    }

    std::cout << "Log File Open Over" << std::endl;

    //定义session_params
    settings_pack sp_set;

    //bootstrap nodes
    sp_set.set_str(settings_pack::dht_bootstrap_nodes, bootstrap_nodes);
    std::cout <<  "bootstrap nodes: " << bootstrap_nodes << std::endl;

    //device_id
    sp_set.set_str(settings_pack::device_id, device_id);
    std::cout <<  "device id: " << device_id << std::endl;

    //account seed
    sp_set.set_str(settings_pack::account_seed, account_seed);
    std::cout <<  "account_seed: " << account_seed << std::endl;

    //listen port
    std::stringstream listen_interfaces;
    listen_interfaces << "0.0.0.0:" << listen_port << ",[::]:" << listen_port;
    std::cout << "listen port: " << listen_interfaces.str() << std::endl;
    sp_set.set_str(settings_pack::listen_interfaces, listen_interfaces.str());

    //tau save path
    std::cout << "libTAU save path: " << tau_save_path << std::endl;
    sp_set.set_str(settings_pack::db_dir, tau_save_path);

    //alert mask
    sp_set.set_int(settings_pack::alert_mask, alert::all_categories);    

    //alert mask
    sp_set.set_int(settings_pack::dht_item_lifetime, 7200);    

    //reopen time when peer is 0
    sp_set.set_int(settings_pack::max_time_peers_zero, 7200000);    

    //referable
    sp_set.set_bool(settings_pack::dht_non_referrable, true);

    //disable communication and blockchain
    //sp_set.set_bool(settings_pack::enable_communication, false);
    sp_set.set_bool(settings_pack::enable_blockchain, false);

    std::cout << "Session parameters' setting Over" << std::endl;

    session_params sp_param(sp_set) ;
    session ses(sp_param);
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

    //port
    std::uint16_t port = ses.get_port_from_pubkey(m_pubkey);
    std::cout << "port: "  << port << std::endl;

    std::vector<alert*> alert_queue;
    bool shutting_down = false;
    while (!quit)
    {
        ses.pop_alerts(&alert_queue);

        for (std::vector<alert*>::iterator i = alert_queue.begin()
            , end(alert_queue.end()); i != end; ++i)
        {
            auto now = std::chrono::system_clock::now(); 
            auto now_c = std::chrono::system_clock::to_time_t(now); 
            std::cout << std::put_time(std::localtime(&now_c), "%c") << " " << (*i)->message().c_str() << std::endl;
            fprintf(debug_file, "%s %s\n", std::put_time(std::localtime(&now_c), "%c"), (*i)->message().c_str());
            //std::cout << (*i)->type() <<  " " << log_alert::alert_type << std::endl;
            int alert_type = (*i)->type();
            switch(alert_type){
                case session_stats_alert::alert_type: 
                    a_handler.alert_on_session_stats(*i);
                    break;
                case log_alert::alert_type: 
                    //std::cout << ses.get_session_time()/1000 << " SESSION LOG: " << (*i)->message().c_str() << std::endl;
                    break;
                case dht_log_alert::alert_type:
                    //std::cout << ses.get_session_time()/1000 << " DHT LOG:  " << (*i)->message().c_str() << std::endl;
                    break;
				//communication
                case communication_new_device_id_alert::alert_type:
                    a_handler.alert_on_new_device_id(*i);
                    break;
                case communication_new_message_alert::alert_type:
                    //a_handler.alert_on_new_message(*i);
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
                    break;
				//blockchain
                case blockchain_log_alert::alert_type:
                    std::cout << ses.get_session_time()/1000 << " BLOCKCHAIN LOG:  " << (*i)->message().c_str() << std::endl;
                    break;
                case blockchain_new_head_block_alert::alert_type:
                    std::cout << ses.get_session_time()/1000 << " BLOCKCHAIN LOG New Head Block:  " << (*i)->message().c_str() << std::endl;
                    a_handler.alert_on_new_head_block(*i);
                    break;
                case blockchain_new_tail_block_alert::alert_type:
                    std::cout << ses.get_session_time()/1000 << " BLOCKCHAIN LOG New Tail Block:  " << (*i)->message().c_str() << std::endl;
                    a_handler.alert_on_new_tail_block(*i);
                    break;
                case blockchain_new_consensus_point_block_alert::alert_type:
                    std::cout << ses.get_session_time()/1000 << " BLOCKCHAIN LOG New Consensus Block:  " << (*i)->message().c_str() << std::endl;
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
        //ses.wait_for_alert(libTAU::milliseconds(500));
    }

    std::cout << "Cycle Over" << std::endl;

    if (debug_file) fclose(debug_file);

    std::cout << "Total Over" << std::endl;

    return 0;
}

