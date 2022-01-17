#include <string>
#include <sqlite3.h>

#include "libTAU/kademlia/types.hpp"
#include "libTAU/communication/message.hpp"
#include "libTAU/blockchain/transaction.hpp"

#ifndef LIBTAU_SHELL_DB_HPP
#define LIBTAU_SHELL_DB_HPP

namespace libTAU {

	struct tau_shell_sql
	{
		tau_shell_sql(std::string db_storage_file_name);
		~tau_shell_sql();

		void sqlite_db_initial();

		char* sql_process(char* s, int size_sql, char *t, int size_name);

		bool db_add_new_friend(std::string f);

		bool db_delete_friend(char* f);

		bool db_insert_friend_info();

		bool db_update_friend_info();

		bool db_add_new_message(const communication::message& msg);

		//blockchain
		bool db_follow_chain(const std::string& chain_id, std::set<dht::public_key> peers);
		bool db_unfollow_chain(const std::string& chain_id);

		bool db_add_new_transaction(const blockchain::transaction& tx);

	private:
		std::string m_db_path;
		sqlite3* m_sqldb;
	};
}
#endif
