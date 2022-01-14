#include <fstream>
#include <iostream>
#include <cstring>
#include <string>
#include <vector>
#include <stdio.h>

extern "C" {
#include "rpc/jsmn.h"
}

#include "handler/db_util.hpp"
#include "rpc/json_util.hpp"

namespace libTAU {

	tau_shell_sql::tau_shell_sql(std::string db_storage_file_name)
	{
		m_db_path = db_storage_file_name;
	    int sqlrc = sqlite3_open_v2(m_db_path.c_str(), &m_sqldb, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_SHAREDCACHE, NULL);
    		if (sqlrc != SQLITE_OK) {
        	//报错退出-打开数据库失败
        	fprintf(stderr, "failed to open sqlite db");
        	exit(1);
    	} 
	}

	void tau_shell_sql::sqlite_db_initial()
	{
		// read db info from config.json
		std::vector<char> sql_info;
		FILE *fp = fopen("./config/1.json", "r");
		if(fp != NULL) {
			while(!feof(fp)) {
				sql_info.push_back(fgetc(fp));
			}
		}

		int i = sql_info.size() - 1;
		while(sql_info[i] != '}') {
			sql_info.pop_back();
			i--;
		}
		sql_info[sql_info.size()]='\0';

		// parse into map(string, string), 1st is table name, 2nd is create sql
		jsmntok_t tokens[2048];
    	jsmn_parser p;
    	jsmn_init(&p);

    	int r = jsmn_parse(&p, &sql_info[0], tokens, sizeof(tokens)/sizeof(tokens[0]));
    	if (r == JSMN_ERROR_INVAL)
    	{   
        	std::cout << "request not JSON" << std::endl;
    	}   
    	else if (r == JSMN_ERROR_NOMEM)
    	{   
        	std::cout << "request too big" << std::endl;
    	}   
    	else if (r == JSMN_ERROR_PART)
    	{   
        	std::cout << "request truncated" << std::endl;
    	}   
    	else if (r != JSMN_SUCCESS)
    	{   
        	std::cout << "invalid request" << std::endl;
    	}

		// delete 
		for(int i = 0; i< 2048; i++) {
			sql_info[tokens[i].end] = 0;
		}

		// database -> entities		
		jsmntok_t* database = find_key(tokens, &sql_info[0], "database", JSMN_OBJECT);
		if(!database) {
			std::cout << "no key: database in config file" << std::endl;
		}

		jsmntok_t* tables = find_key(database, &sql_info[0], "entities", JSMN_ARRAY);
		if(!tables) {
			std::cout << "no key: entities in config file" << std::endl;
		}

		// entities -> tables and create
		for(int i = 0; i < tables->size; i++) {
			jsmntok_t* table_name = find_key_in_array(tokens, &sql_info[0], "tableName", i , 2048, JSMN_STRING);
			if(!table_name) {
				std::cout << "no key: tableName in config file" << std::endl;
			}
			std::cout << "Table Name Type : " << table_name->type << std::endl;
			int tn_size = table_name->end - table_name->start + 1;
			char *tn = new char[tn_size];
			memcpy(tn, &sql_info[table_name->start], tn_size);
			
			jsmntok_t* sql_table = find_key_in_array(tokens, &sql_info[0], "createSql", i , 2048, JSMN_STRING);
			if(!sql_table) {
				std::cout << "no key: createSql in config file" << std::endl;
			}

			int sql_size = sql_table->end - sql_table->start + 1;
			char *sql_temp = new char[sql_size];
			memcpy(sql_temp, &sql_info[sql_table->start], sql_size);

			// process sql into sql standard
			char* sql = sql_process(sql_temp, sql_size, tn, tn_size);
			std::cout << tn << std::endl;
			std::cout << sql << std::endl;

			// create db table
			char *err_msg = nullptr;
            int ok = sqlite3_exec(m_sqldb, sql, nullptr, nullptr, &err_msg);
            if (ok != SQLITE_OK) {
                sqlite3_free(err_msg);
				std::cout << "sql create table exec error" << std::endl;
            }

		}
			
	}

	char* tau_shell_sql::sql_process(char *s, int s_s, char *t, int s_t)
	{
		//relpace ` into "
		int index = -1;
		for(int i = 0; i < s_s ; i++) {
			if(s[i]=='`') {
				s[i] = '"';
				if(-1 == index) {
					index = i;
				}
			}
		}
		int new_size = s_s + s_t - 14;
		char *c = new char[new_size]; //quit ${TABLE_NAME}
		//1st "
		memcpy(c, &s[0], index + 1);
		//2nd table name
		memcpy(c + index + 1, &t[0], s_t - 1);
		//3rd
		memcpy(c + index + s_t, &s[index + 14], s_s - index - 14);

		return c;
	}

	bool tau_shell_sql::db_add_new_friend(std::string f)
	{
		sqlite3_stmt * stmt;
        char* sql = "INSERT INTO Friends VALUES(?)";
        int ok = sqlite3_prepare_v2(m_sqldb, sql, -1, &stmt, nullptr);
        if (ok != SQLITE_OK) {
        	return false;
        }

		std::string friend_info = "\"" + f + "\", "+ 
								  "\"" + f + "\", "+
								 "-1, -1, -1, -1, -1";
        sqlite3_bind_text(stmt, 1, friend_info.c_str(), friend_info.size(), nullptr);
        ok = sqlite3_step(stmt);
        if (ok != SQLITE_DONE) {
            return false;
        }
        sqlite3_finalize(stmt);

        return true;
	}

	bool tau_shell_sql::db_delete_friend(char* f)
	{
		return true;
	}

	bool tau_shell_sql::db_insert_friend_info()
	{
		return true;
	}

	bool tau_shell_sql::db_update_friend_info()
	{
		return true;
	}

	bool tau_shell_sql::db_add_new_message()
	{
		return true;
	}

}
