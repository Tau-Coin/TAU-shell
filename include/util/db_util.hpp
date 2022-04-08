#include <string>
#include <sqlite3.h>

#include "libTAU/kademlia/types.hpp"
#include "libTAU/communication/message.hpp"
#include "libTAU/blockchain/block.hpp"
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

		bool db_update_friend_last_seen(const dht::public_key& pubkey, std::int64_t time);

		bool db_update_friend_last_comm(const dht::public_key& pubkey, std::int64_t time);

		bool db_insert_friend_info();

		bool db_update_friend_info();

		bool db_add_new_message(const communication::message& msg, int status = 0);

		bool db_update_message_status(const sha256_hash& msg_hash, std::int64_t time, int status); 

		//blockchain
		bool db_follow_chain(const std::string& chain_id, std::set<dht::public_key> peers);
		bool db_unfollow_chain(const std::string& chain_id);

		bool db_add_new_transaction(const blockchain::transaction& tx, int status = -1, std::int64_t block_number = -1);
		bool db_delete_transaction(const blockchain::transaction& tx);

		bool db_add_new_block(const blockchain::block& blk);
		bool db_delete_block(const blockchain::block& blk);

		bool db_get_chain_state(const std::string& chain_id_hex_str, int* block_number, std::string* block_hash);

		int db_get_tx_state(const std::string& tx_hash);

		bool db_update_community_status(const blockchain::block& blk, int type);

	private:
		std::string m_db_path;
		sqlite3* m_db;
	};
}
#endif
