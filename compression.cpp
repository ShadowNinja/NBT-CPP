
#include <cstdint>
#include <cassert>

#include <zlib.h>

#include "compression.h"

#define BUFSIZE (128 * 1024)


namespace NBT {

ustring compress(const ustring & data, int level)
{
	int res = 0;
	ustring buffer;
	unsigned char temp_buffer[BUFSIZE];

	z_stream strm;
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.next_in = Z_NULL;
	strm.avail_in = data.size();

	if (deflateInit(&strm, level) != Z_OK) {
		throw CompressionError("Error initializing stream");
	}

	strm.next_in = const_cast<unsigned char *>(data.data());
	do {
		strm.avail_out = BUFSIZE;
		strm.next_out = temp_buffer;
		res = deflate(&strm, Z_FINISH);
		assert(strm.avail_out > 0);
		buffer += ustring(temp_buffer, BUFSIZE - strm.avail_out);
	} while (res == Z_OK);

	if (res != Z_STREAM_END) {
		throw CompressionError("Deflation error");
	}

	(void) deflateEnd(&strm);

	return buffer;
}


ustring decompress(const ustring & data)
{
	int res = 0;
	ustring buffer;
	unsigned char temp_buffer[BUFSIZE];

	z_stream strm;
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.next_in = Z_NULL;
	strm.avail_in = data.size();

	if (inflateInit(&strm) != Z_OK) {
		throw CompressionError("Error initializing stream");
	}

	strm.next_in = const_cast<unsigned char *>(data.data());
	do {
		strm.next_out = temp_buffer;
		strm.avail_out = BUFSIZE;
		res = inflate(&strm, Z_FINISH);
		assert(strm.avail_out > 0);
		buffer += ustring(temp_buffer, BUFSIZE - strm.avail_out);
	} while (res == Z_OK);

	if (res != Z_STREAM_END) {
		throw CompressionError("Inflation error");
	}

	(void) inflateEnd(&strm);

	return buffer;
}

} // namespace NBT

#undef BUFSIZE

