
#ifndef NBT_HEADER
#define NBT_HEADER

#include <cstdint>
#include <exception>
#include <string>
#include <map>

// Require C++11
#if __cplusplus < 201103L
#	error NBT-CPP requires C++11
#endif

namespace NBT {

/*********
 * Types *
 *********/

struct Tag;

typedef uint8_t Byte;

enum class TagType : Byte {
	End,
	Byte,
	Short,
	Int,
	Long,
	Float,
	Double,
	ByteArray,
	String,
	List,
	Compound,
	IntArray,
};

struct ByteArray {
	uint32_t size;
	Byte *value;
};

struct String {
	uint16_t size;
	char *value;
};

struct List {
	TagType tagid;
	uint32_t size;
	Tag *value;
};

typedef std::map<std::string, Tag> Compound;

struct IntArray {
	uint16_t size;
	int32_t *value;
};

union Value {
	Byte      v_byte;
	int16_t   v_short;
	int32_t   v_int;
	int64_t   v_long;
	float     v_float;
	double    v_double;
	ByteArray v_byte_array;
	String    v_string;
	List      v_list;
	Compound *v_compound;
	IntArray  v_int_array;
};

class Tag {
public:
	Tag();
	Tag(const Byte *bytes);
	Tag(const TagType tag, uint32_t size = 1);

	Tag(const Byte x);
	Tag(const int16_t x);
	Tag(const int32_t x);
	Tag(const int64_t x);
	Tag(const float x);
	Tag(const double x);

	Tag(const Tag &t);
	Tag(Tag &&t);
	~Tag();

	Tag & operator=(const Tag &t);
	Tag & operator=(Tag &&t);
	Tag & operator[](const int32_t &k);
	Tag & operator[](const std::string &k);
	Tag & operator+=(const Byte &t);
	Tag & operator+=(const int32_t &t);
	Tag & operator+=(const Tag &t);
	Tag & operator+=(Tag &&t);

	void copy(const Tag &t);
	void free();
	void setTag(const TagType tag, uint32_t size = 1);

	void read(const Byte *bytes);
	std::string write() const;

	Byte       toByte();
	int16_t    toShort();
	int32_t    toInt();
	int64_t    toLong();
	float      toFloat();
	double     toDouble();
	ByteArray &toByteArray();
	String    &toString();
	List      &toList();
	Compound  &toCompound();
	IntArray  &toIntArray();

	void insert(const int32_t &k, const Byte &t);
	void insert(const int32_t &k, const int32_t &t);
	void insert(const int32_t &k, const Tag &t);
	void insert(const std::string &k, const Tag &t);

	TagType type;

protected:
	void       read_tag       (const Byte *bytes, uint64_t &index, TagType tag);

	Byte      read_byte      (const Byte *bytes, uint64_t &index);
	int16_t   read_short     (const Byte *bytes, uint64_t &index);
	int32_t   read_int       (const Byte *bytes, uint64_t &index);
	int64_t   read_long      (const Byte *bytes, uint64_t &index);
	float     read_float     (const Byte *bytes, uint64_t &index);
	double    read_double    (const Byte *bytes, uint64_t &index);
	ByteArray read_byte_array(const Byte *bytes, uint64_t &index);
	String    read_string    (const Byte *bytes, uint64_t &index);
	List      read_list      (const Byte *bytes, uint64_t &index);
	Compound *read_compound  (const Byte *bytes, uint64_t &index);
	IntArray  read_int_array (const Byte *bytes, uint64_t &index);

	uint64_t getSerializedSize() const;
	template <typename container, typename contained>
		void ensureSize(container *field, uint32_t size);

	Value value;
};

} // namespace NBT

#endif

