#include <string>

#include "handler/db_util.hpp"

#include "libTAU/alert.hpp"
#include "libTAU/alert_types.hpp"

#ifndef LIBTAU_SHELL_ALERT_HANDLER_HPP
#define LIBTAU_SHELL_ALERT_HANDLER_HPP

namespace libTAU {

	struct alert_handler
	{
		alert_handler(tau_shell_sql* db);

		//communication
		void alert_on_new_device_id(alert* i);

		void alert_on_new_message(alert* i);

		void alert_on_confirmation_root(alert* i);

		void alert_on_syncing_message(alert* i);

		void alert_on_friend_info(alert* i);

		void alert_on_last_seen(alert* i);

		//blockchain
		void alert_on_new_head_block(alert *i);

		void alert_on_new_tail_block(alert *i);

		void alert_on_new_consensus_point_block(alert *i);

		void alert_on_new_transaction(alert *i);
		
		void alert_on_rollback_block(alert *i);

		void alert_on_fork_point_block(alert *i);

		void alert_on_top_three_votes(alert *i);
		
	private:
		tau_shell_sql* m_db;
	};

}
#endif
