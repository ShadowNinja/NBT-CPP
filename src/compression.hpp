#ifndef NBT_COMPRESSION_HEADER
#define NBT_COMPRESSION_HEADER

#include <string>
#include <zlib.h>

namespace NBT {

extern bool compress(std::string * out, const char * in, size_t size,
		int level = Z_DEFAULT_COMPRESSION);
extern bool decompress(std::string * out, const char * in, size_t size);

} // namespace NBT

#endif // NBT_COMPRESSION_HEADER

