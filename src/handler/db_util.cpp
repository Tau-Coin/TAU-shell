#include <fstream>
#include <iostream>
#include <cstring>
#include <string>
#include <vector>
#include <stdio.h>

extern "C" {
#include "rpc/jsmn.h"
}

#include "rpc/json_util.hpp"
#include "handler/db_util.hpp"

namespace libTAU {

	void sqlite_db_initial()
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

		for(int i = 0; i< 2048; i++) {
			sql_info[tokens[i].end] = 0;
			std::cout << i << "0: " << tokens[i].type  << std::endl;
			std::cout << i << "1: " << tokens[i].start  << std::endl;
			std::cout << i << "2: " << tokens[i].end  << std::endl;
			std::cout << i << "3: " << tokens[i].size  << std::endl;
			std::cout << "=====================================" << std::endl << std::endl;
		}

		
		jsmntok_t* database = find_key(tokens, &sql_info[0], "database", JSMN_OBJECT);
		std::cout << "database type : " << database->type << std::endl;
		std::cout << "database start : " << database->start << std::endl;
		std::cout << "database end : " << database->end << std::endl;
		std::cout << "database size : " << database->size << std::endl;

		jsmntok_t* tables = find_key(database, &sql_info[0], "entities", JSMN_ARRAY);
		std::cout << "Table type : " << tables->type << std::endl;
		std::cout << "Table start : " << tables->start << std::endl;
		std::cout << "Table end : " << tables->end << std::endl;
		std::cout << "Table size : " << tables->size << std::endl;

		for(int i = 0; i < tables->size; i++) {
			jsmntok_t* tableName = find_key(tables, &sql_info[0], "tableName", JSMN_ARRAY);
			std::cout << "Table Name : " << tableName->end << " "  << tableName->start<< std::endl;
		}
		
	}
}
