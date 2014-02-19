
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

typedef int8_t  Byte;
typedef int16_t Short;
typedef int32_t Int;
typedef int64_t Long;
typedef uint8_t  UByte;
typedef uint16_t UShort;
typedef uint32_t UInt;
typedef uint64_t ULong;

enum class TagType : UByte {
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
	UInt size;
	Byte *value;
};

struct String {
	UShort size;
	char *value;
};

struct List {
	TagType tagid;
	UInt size;
	Tag *value;
};

typedef std::map<std::string, Tag> Compound;

struct IntArray {
	UShort size;
	Int *value;
};

union Value {
	Byte      v_byte;
	Short     v_short;
	Int       v_int;
	Long      v_long;
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
	Tag(const TagType tag, UInt size = 0);

	Tag(const Byte x);
	Tag(const Short x);
	Tag(const Int x);
	Tag(const Long x);
	Tag(const float x);
	Tag(const double x);
	Tag(const std::string x);

	Tag(const Tag &t);
	Tag(Tag &&t);
	~Tag();

	Tag & operator=(const Tag &t);
	Tag & operator=(Tag &&t);
	Tag & operator[](const Int &k);
	Tag & operator[](const std::string &k);
	Tag & operator[](const char *k);
	Tag & operator+=(const Byte &t);
	Tag & operator+=(const Int &t);
	Tag & operator+=(const Tag &t);
	Tag & operator+=(Tag &&t);

	operator Byte();
	operator Short();
	operator Int();
	operator Long();
	operator float();
	operator double();
	operator ByteArray();
	operator String();
	operator List();
	operator Compound();
	operator IntArray();

	void copy(const Tag &t);
	void free();
	void setTag(const TagType tag, UInt size = 0);

	void read(const Byte *bytes);
	std::string write() const;
	std::string dump() const;

	void insert(const Int &k, const Byte &b);
	void insert(const Int &k, const Int &i);
	void insert(const Int &k, const Tag &t);
	void insert(const std::string &k, const Tag &t);

	TagType type;

protected:
	void       read_tag      (const Byte *bytes, ULong &index, TagType tag);

	friend List      read_list      (const Byte *bytes, ULong &index);
	friend Compound *read_compound  (const Byte *bytes, ULong &index);

	ULong getSerializedSize() const;
	template <typename container, typename contained>
		void ensureSize(container *field, UInt size);

	Value value;
};

} // namespace NBT

#endif

