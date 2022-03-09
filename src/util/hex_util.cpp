#include "util/hex_util.hpp"

#include "libTAU/hex.hpp"
#include "libTAU/span.hpp"

using namespace libTAU;

namespace libTAU {

    int chain_id_size(int size) {
        return size - 8;
    }

	bool hex_char_to_bytes_char(const char* hex_char, char* bytes_char, int len)
	{
		span<char const> hex_char_span(hex_char, len);		
		return aux::from_hex(hex_char_span, bytes_char);
	}

	bool hex_chain_id_to_bytes_char(const char* hex_char, char* bytes_char, int len)
	{
        int size_id = 16;
        hex_char_to_bytes_char(hex_char, bytes_char, size_id);

        for(int i = 0; i < len - 16; i++){
            bytes_char[8+i] = hex_char[15 + i];
        }

        return true;
	}
}
