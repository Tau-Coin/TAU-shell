#include <iostream>

#include "util/hex_util.hpp"
#include "util/tau_constants.hpp"

#include "libTAU/hex.hpp"
#include "libTAU/span.hpp"

using namespace libTAU;

namespace libTAU {

    int chain_id_size(int size) {
        return size - CID_KEY_CHAR_LEN;
    }

	bool hex_char_to_bytes_char(const char* hex_char, char* bytes_char, int len)
	{
		span<char const> hex_char_span(hex_char, len);		
		return aux::from_hex(hex_char_span, bytes_char);
	}

	std::vector<char> string_chain_id_to_bytes(const char* hex_char, int len)
	{
        int size_chain_id = chain_id_size(len);
        char* chain_bytes = new char[size_chain_id];
        hex_char_to_bytes_char(hex_char, chain_bytes, CID_KEY_HEX_LEN);

        for(int i = 0; i < len - CID_KEY_HEX_LEN; i++){
            chain_bytes[CID_KEY_CHAR_LEN + i] = hex_char[CID_KEY_HEX_LEN + i];
        }

        std::vector<char> result;
        result.insert(result.end(), chain_bytes, chain_bytes + strlen(chain_bytes));

        return result;
	}

	std::string bytes_chain_id_to_string(const char* chain_bytes, int len)
	{
        char* chain_str = new char[len + CID_KEY_CHAR_LEN + 1];

        aux::to_hex(chain_bytes, CID_KEY_CHAR_LEN, chain_str);

        for(int i = CID_KEY_CHAR_LEN; i < len; i++){
            chain_str[CID_KEY_CHAR_LEN + i] = chain_bytes[i];
        }

        chain_str[len + CID_KEY_CHAR_LEN] = '\0'; //last element '\0'

        return std::string(chain_str);
	}

	std::vector<char> hex_chain_id_to_bytes(const char* chain_hex, int len)
	{
        char* chain_bytes = new char[len/2+1];
        chain_bytes[len/2] = '\0';
        hex_char_to_bytes_char(chain_hex, chain_bytes, len);

        std::vector<char> result;
        result.insert(result.end(), chain_bytes, chain_bytes + strlen(chain_bytes));
        return result;
	}
}
