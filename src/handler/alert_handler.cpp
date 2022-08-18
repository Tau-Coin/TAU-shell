#include <fstream>
#include <iostream>
#include <cstring>
#include <string>
#include <vector>
#include <stdio.h>

extern "C" {
#include "util/jsmn.h"
}

#include "handler/alert_handler.hpp"
#include "util/json_util.hpp"

#include "libTAU/hex.hpp"
#include "libTAU/span.hpp"
#include "libTAU/performance_counters.hpp"
#include "libTAU/blockchain/constants.hpp"

namespace libTAU {

	alert_handler::alert_handler(tau_shell_sql* db)
	: m_db(db)	
	{
	}

	void alert_handler::alert_on_session_stats(alert* i){
        session_stats_alert* a = reinterpret_cast<session_stats_alert*>(i);
        span<std::int64_t const> sc = a -> counters();
        std::cout << "session nodes number: " << sc[counters::dht_nodes] << std::endl;
		return;
	}

	void alert_handler::alert_on_new_device_id(alert* i){
		return;
	}

	void alert_handler::alert_on_new_message(alert* i){
		communication_new_message_alert* a = reinterpret_cast<communication_new_message_alert*>(i);
		communication::message msg = a -> msg;
		m_db->db_add_new_message(msg, 1);
	}

	void alert_handler::alert_on_confirmation_root(alert* i){
		communication_confirmation_root_alert* a = reinterpret_cast<communication_confirmation_root_alert*>(i);
		dht::public_key pubkey = a -> peer;
		std::vector<sha1_hash> msg_hashes = a -> confirmation_roots;
		std::int64_t time_stamp = a -> time;
	
		for(auto i = msg_hashes.begin(); i != msg_hashes.end(); i++){
			m_db->db_update_message_status(*i, time_stamp, 2);
		}
	}

	void alert_handler::alert_on_syncing_message(alert* i){
		communication_syncing_message_alert* a = reinterpret_cast<communication_syncing_message_alert*>(i);
		dht::public_key pubkey = a -> peer;
		sha1_hash msg_hash = a -> syncing_msg_hash;
		std::int64_t time_stamp = a -> time;
		m_db->db_update_message_status(msg_hash, time_stamp, 1);
	}

	void alert_handler::alert_on_friend_info(alert* i){
		return;
	}

	void alert_handler::alert_on_last_seen(alert* i){
		communication_last_seen_alert* a = reinterpret_cast<communication_last_seen_alert*>(i);
		dht::public_key pubkey = a -> peer;
		std::int64_t time = a -> last_seen;
		m_db->db_update_friend_last_seen(pubkey, time);
	}
	
	void alert_handler::alert_on_new_head_block(alert *i){
		blockchain_new_head_block_alert* a = reinterpret_cast<blockchain_new_head_block_alert*>(i);
		blockchain::block blk = a -> blk;
		
		// deal with block
		m_db->db_add_new_block(blk);
		m_db->db_update_community_status(blk, 0);
	}

	void alert_handler::alert_on_new_tail_block(alert *i){
		blockchain_new_tail_block_alert* a = reinterpret_cast<blockchain_new_tail_block_alert*>(i);
		blockchain::block blk = a -> blk;

		// deal with block
		m_db->db_update_community_status(blk, 2);
	}

	void alert_handler::alert_on_new_consensus_point_block(alert *i){
		blockchain_new_consensus_point_block_alert* a = reinterpret_cast<blockchain_new_consensus_point_block_alert*>(i);
		blockchain::block blk = a -> blk;

		// deal with block
		m_db->db_update_community_status(blk, 1);
	}

	void alert_handler::alert_on_new_transaction(alert *i){
		blockchain_new_transaction_alert* a = reinterpret_cast<blockchain_new_transaction_alert*>(i);
		blockchain::transaction tx = a -> tx;

		// deal with tx
		m_db->db_add_new_transaction(tx);
	}
		
	void alert_handler::alert_on_rollback_block(alert *i){
		blockchain_rollback_block_alert* a = reinterpret_cast<blockchain_rollback_block_alert*>(i);
		blockchain::block blk = a -> blk;

		// deal with block
		m_db->db_delete_block(blk);
		return;
	}

	void alert_handler::alert_on_fork_point_block(alert *i){
		return;
	}

	void alert_handler::alert_on_top_three_votes(alert *i){
		return;
	}
		
}
