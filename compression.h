#ifndef NBT_COMPRESSION_HEADER
#define NBT_COMPRESSION_HEADER

#include <stdexcept>
#include <string>
#include <zlib.h>

namespace NBT {

typedef std::basic_string<unsigned char> ustring;

extern ustring compress(const ustring & data, int level = Z_DEFAULT_COMPRESSION);
extern ustring decompress(const ustring & data);

class CompressionError : public std::runtime_error {
	using std::runtime_error::runtime_error;
};

} // namespace NBT

#endif // NBT_COMPRESSION_HEADER

