#include<vector>

namespace libTAU {

    int chain_id_size(int size);

	bool hex_char_to_bytes_char(const char* hex_char, char* bytes_char, int len);

	std::vector<char> string_chain_id_to_bytes(const char* hex_char, int len);

	std::string bytes_chain_id_to_string(const char* chain_bytes, int len);

    std::vector<char> hex_chain_id_to_bytes(const char* chain_hex, int len);
}
