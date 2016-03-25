
#ifndef NBT_HEADER
#define NBT_HEADER

#include <cstdint>
#include <exception>
#include <string>
#include <map>

// Require C++11
#if __cplusplus < 201103L
	#error NBT-CPP requires C++11
#endif

static_assert(sizeof(float) == 4,  "NBT depends on IEEE-754 32-bit floats");
static_assert(sizeof(double) == 8, "NBT depends on IEEE-754 64-bit doubless");


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
	UInt size;
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
	Tag() : type(TagType::End), value() {}
	Tag(const UByte *bytes);
	Tag(const TagType tag, UInt size = 0, TagType subtype = TagType::End);

	Tag(Byte x)   : type(TagType::Byte)   { value.v_byte = x; }
	Tag(Short x)  : type(TagType::Short)  { value.v_short = x; }
	Tag(Int x)    : type(TagType::Int)    { value.v_int = x; }
	Tag(Long x)   : type(TagType::Long)   { value.v_long = x; }
	Tag(float x)  : type(TagType::Float)  { value.v_float = x; }
	Tag(double x) : type(TagType::Double) { value.v_double = x; }
	Tag(const std::string x);

	Tag(const Tag &t) : type(TagType::End) { copy(t); }
	Tag(Tag &&t) : type(t.type), value(t.value)
		{ t.type = TagType::End; }

	~Tag() { free(); }

	Tag & operator=(const Tag &t);
	Tag & operator=(Tag &&t);
	Tag & operator[](const Int &k);
	Tag & operator[](const std::string &k);
	Tag & operator[](const char *k);
	Tag & operator+=(const Byte &t);
	Tag & operator+=(const Int &t);
	Tag & operator+=(const Tag &t);
	Tag & operator+=(Tag &&t);

	operator Byte     () { return value.v_byte; }
	operator Short    () { return value.v_short; }
	operator Int      () { return value.v_int; }
	operator Long     () { return value.v_long; }
	operator float    () { return value.v_float; }
	operator double   () { return value.v_double; }
	operator ByteArray() { return value.v_byte_array; }
	operator String   () { return value.v_string; }
	operator List     () { return value.v_list; }
	operator Compound () { return *value.v_compound; }
	operator IntArray () { return value.v_int_array; }

	void copy(const Tag &t);
	void free();
	void setTag(const TagType tag, UInt size = 0, TagType subtype = TagType::End);

	void read(const UByte *bytes);
	std::string write() const;
	std::string dump() const;

	void insert(const Int k, const Byte b);
	void insert(const Int k, const Int i);
	void insert(const Int k, const Tag &t);
	void insert(const std::string &k, const Tag &t);

	TagType type;

protected:
	void readTag(const UByte *bytes, ULong &index, TagType tag);

	friend List      readList    (const UByte *bytes, ULong &index);
	friend Compound *readCompound(const UByte *bytes, ULong &index);

	ULong getSerializedSize() const;
	template <typename container, typename contained>
		void ensureSize(container *field, UInt size);

	Value value;
};

} // namespace NBT

#endif

