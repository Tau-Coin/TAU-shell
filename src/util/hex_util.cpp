#include "util/hex_util.hpp"

#include "libTAU/hex.hpp"
#include "libTAU/span.hpp"

using namespace libTAU;

namespace libTAU {

	bool hex_char_to_bytes_char(const char* hex_char, char* bytes_char, int len)
	{
		span<char const> hex_char_span(hex_char, len);		
		return aux::from_hex(hex_char_span, bytes_char);
	}
}
