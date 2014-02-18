
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
	Tag(const TagType tag, UInt size = 1);

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
	Tag & operator+=(const Byte &t);
	Tag & operator+=(const Int &t);
	Tag & operator+=(const Tag &t);
	Tag & operator+=(Tag &&t);

	void copy(const Tag &t);
	void free();
	void setTag(const TagType tag, UInt size = 1);

	void read(const Byte *bytes);
	std::string write() const;

	Byte       toByte();
	Short      toShort();
	Int        toInt();
	Long       toLong();
	float      toFloat();
	double     toDouble();
	ByteArray &toByteArray();
	String    &toString();
	List      &toList();
	Compound  &toCompound();
	IntArray  &toIntArray();

	void insert(const Int &k, const Byte &b);
	void insert(const Int &k, const Int &i);
	void insert(const Int &k, const Tag &t);
	void insert(const std::string &k, const Tag &t);

	TagType type;

protected:
	void       read_tag      (const Byte *bytes, ULong &index, TagType tag);

	Byte      read_byte      (const Byte *bytes, ULong &index);
	int16_t   read_short     (const Byte *bytes, ULong &index);
	int32_t   read_int       (const Byte *bytes, ULong &index);
	int64_t   read_long      (const Byte *bytes, ULong &index);
	float     read_float     (const Byte *bytes, ULong &index);
	double    read_double    (const Byte *bytes, ULong &index);
	ByteArray read_byte_array(const Byte *bytes, ULong &index);
	String    read_string    (const Byte *bytes, ULong &index);
	List      read_list      (const Byte *bytes, ULong &index);
	Compound *read_compound  (const Byte *bytes, ULong &index);
	IntArray  read_int_array (const Byte *bytes, ULong &index);

	ULong getSerializedSize() const;
	template <typename container, typename contained>
		void ensureSize(container *field, UInt size);

	Value value;
};

} // namespace NBT

#endif

