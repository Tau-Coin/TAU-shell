
namespace libTAU {

    int chain_id_size(int size);

	bool hex_char_to_bytes_char(const char* hex_char, char* bytes_char, int len);

	bool hex_chain_id_to_bytes_char(const char* hex_char, char* bytes_char, int len);

}
