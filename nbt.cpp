
#include "nbt.h"
#include <cstring>
#include <cassert>
#include <sstream>

namespace NBT {

// Converts relative negative indexes to positive indexes
#define TOABS(x, s) (x < 0 ? s + x : x)

/******************************
 * con/destructors, operators *
 ******************************/

Tag::Tag() :
	type(), value()
{}

Tag::Tag(const Byte *bytes)
{
	read(bytes);
}

Tag::Tag(const TagType tag, UInt size)
{
	type = NBT::TagType::End;
	setTag(tag, size);
}

Tag::Tag(const Byte x)   : type(TagType::Byte)    { value.v_byte = x; }
Tag::Tag(const Short x)  : type(TagType::Short)   { value.v_short = x; }
Tag::Tag(const Int x)    : type(TagType::Int)     { value.v_int = x; }
Tag::Tag(const Long x)   : type(TagType::Long)    { value.v_long = x; }
Tag::Tag(const float x)  : type(TagType::Float)   { value.v_float = x; }
Tag::Tag(const double x) : type(TagType::Double)  { value.v_double = x; }
Tag::Tag(const std::string x)  : type(TagType::String)  {
	value.v_string.size = x.size();
	value.v_string.value = new char[x.size()];
	memcpy((void*) value.v_string.value, (void*) x.c_str(), x.size());
}

Tag::Tag(const Tag &t)
{
	copy(t);
}

Tag::Tag(Tag &&t) :
		type(t.type),
		value(t.value)
{
	t.type = TagType::End;
}

Tag::~Tag()
{
	free();
}

Tag & Tag::operator=(const Tag &t)
{
	assert(this != NULL);
	if (this == &t) {
		return *this;
	}
	copy(t);
	return *this;
}

Tag & Tag::operator=(Tag &&t)
{
	assert(this != NULL);
	if (this == &t) {
		return *this;
	}
	type = t.type;
	value = t.value;
	t.type = TagType::End;
	return *this;
}

Tag & Tag::operator[](const Int &k)
{
	UInt ak = TOABS(k, value.v_list.size);
	ensureSize<List, Tag>(&value.v_list, ak + 1);
	return value.v_list.value[ak];
}

Tag & Tag::operator[](const std::string &k)
{
	return (*value.v_compound)[k];
}

Tag & Tag::operator+=(const Byte &b)
{
	assert(type == TagType::ByteArray);
	ensureSize<ByteArray, Byte>(&value.v_byte_array, value.v_byte_array.size + 1);
	value.v_byte_array.value[value.v_byte_array.size - 1] = b;
	return *this;
}

Tag & Tag::operator+=(const Int &i)
{
	assert(type == TagType::IntArray);
	ensureSize<IntArray, Int>(&value.v_int_array, value.v_int_array.size + 1);
	value.v_int_array.value[value.v_int_array.size - 1] = i;
	return *this;
}

Tag & Tag::operator+=(const Tag &t)
{
	assert(type == TagType::List);
	ensureSize<List, Tag>(&value.v_list, value.v_list.size + 1);
	value.v_list.value[value.v_list.size - 1] = t;
	return *this;
}

Tag & Tag::operator+=(Tag &&t)
{
	assert(type == TagType::List);
	ensureSize<List, Tag>(&value.v_list, value.v_list.size + 1);
	value.v_list.value[value.v_list.size - 1] = std::move(t);
	return *this;
}



/********
 * Misc *
 ********/

void Tag::setTag(const TagType tag, UInt size)
{
	free();
	type = tag;
	switch (type) {
	case TagType::ByteArray:
		value.v_byte_array.size = size;
		if (size) value.v_byte_array.value = new Byte[size];
		break;
	case TagType::String:
		value.v_string.size = size;
		if (size) value.v_string.value = new char[size];
		break;
	case TagType::List:
		value.v_list.size = size;
		if (size) value.v_list.value = new Tag[size];
		break;
	case TagType::Compound:
		value.v_compound = new Compound;
		break;
	case TagType::IntArray:
		value.v_int_array.size = size;
		if (size) value.v_int_array.value = new Int[size];
		break;
	default:
		memset((void*) &value, 0, sizeof(value));
	}
}

void Tag::copy(const Tag &t)
{
	free();
	ULong size;
	type = t.type;
	switch (type) {
	case TagType::ByteArray:
		size = t.value.v_byte_array.size;
		value.v_byte_array.size = size;
		value.v_byte_array.value = new Byte[size];
		memcpy((void*) value.v_byte_array.value,
				(void*) t.value.v_byte_array.value, size);
		break;
	case TagType::String:
		size = t.value.v_string.size;
		value.v_string.size = size;
		value.v_string.value = new char[size];
		strncpy(value.v_string.value,
				t.value.v_string.value, size);
		break;
	case TagType::List:
		size = t.value.v_list.size;
		value.v_list.tagid = t.value.v_list.tagid;
		value.v_list.size = size;
		value.v_list.value = new Tag[size];
		for (UInt i = 0; i < size; i++) {
			value.v_list.value[i] = t.value.v_list.value[i];
		}
		break;
	case TagType::Compound:
		value.v_compound = new Compound(*t.value.v_compound);
		break;
	case TagType::IntArray:
		size = t.value.v_int_array.size;
		value.v_int_array.size = size;
		value.v_int_array.value = new Int[size];
		memcpy((void*) value.v_int_array.value,
				(void*) t.value.v_int_array.value, size * 4);
		break;
	default:
		value = t.value;
	}
}

void Tag::free()
{
	switch (type) {
	case TagType::ByteArray:
		delete [] value.v_byte_array.value;
		break;
	case TagType::String:
		delete [] value.v_string.value;
		break;
	case TagType::List:
		delete [] value.v_list.value;
		break;
	case TagType::Compound:
		delete value.v_compound;
		break;
	case TagType::IntArray:
		delete [] value.v_int_array.value;
		break;
	}
	type = TagType::End;  // Prevent double free
}



/*****************
 * insert/append *
 *****************/

template <typename container, typename contained>
	void Tag::ensureSize(container *field, UInt size)
{
	if (size > field->size) {
		container newc;
		newc.size = size;
		newc.value = new contained[size];
		for (UInt i = 0; i < field->size; i++) {
			newc.value[i] = std::move(field->value[i]);
		}
		if (field->size) delete [] field->value;
		*field = newc;
	}
}

void Tag::insert(const Int &k, const Byte &b)
{
	assert(type == TagType::ByteArray);
	UInt ak = TOABS(k, value.v_list.size);
	ensureSize<ByteArray, Byte>(&value.v_byte_array, ak);
	value.v_byte_array.value[ak] = b;
}

void Tag::insert(const Int &k, const Int &i)
{
	assert(type == TagType::IntArray);
	UInt ak = TOABS(k, value.v_list.size);
	ensureSize<IntArray, Int>(&value.v_int_array, ak);
	value.v_int_array.value[ak] = i;
}

void Tag::insert(const Int &k, const Tag &t)
{
	assert(type == TagType::List);
	UInt ak = TOABS(k, value.v_list.size);
	ensureSize<List, Tag>(&value.v_list, ak);
	value.v_list.value[ak] = t;
}

void Tag::insert(const std::string &k, const Tag &t)
{
	assert(type == TagType::Compound);
	(*value.v_compound)[k] = t;
}



/*********
 * toX() *
 *********/

Byte       Tag::toByte()      { return value.v_byte; }
Short      Tag::toShort()     { return value.v_short; }
Int        Tag::toInt()       { return value.v_int; }
Long       Tag::toLong()      { return value.v_long; }
float      Tag::toFloat()     { return value.v_float; }
double     Tag::toDouble()    { return value.v_double; }
ByteArray &Tag::toByteArray() { return value.v_byte_array; }
String    &Tag::toString()    { return value.v_string; }
List      &Tag::toList()      { return value.v_list; }
Compound  &Tag::toCompound()  { return *value.v_compound; }
IntArray  &Tag::toIntArray()  { return value.v_int_array; }



/*****************
 * Serialization *
 *****************/

// Doesn't include size of tagid (always 1)
ULong Tag::getSerializedSize() const
{
	ULong size = 0;
	ULong i = 0;
	switch (type) {
	case TagType::Byte: return 1;
	case TagType::Short: return 2;
	case TagType::Int: return 4;
	case TagType::Long: return 8;
	case TagType::Float: return 4;
	case TagType::Double: return 8;
	case TagType::ByteArray:
		return 4 //Size field
			+ value.v_byte_array.size; // Array size
	case TagType::String:
		return 2 // Size field
			+ value.v_string.size; //String siza
	case TagType::List:
		for (; i < value.v_list.size; i++) {
			size += value.v_list.value[i].getSerializedSize();
		}
		return 1 // TagID
			+ 4 // Size
			+ size; // Items
	case TagType::Compound:
		for (auto &it : *value.v_compound) {
			size += 1 // Value type
				+ 2 // String size
				+ it.first.size() // String
				+ it.second.getSerializedSize(); // Value
		}
		return size
			+ 1; // End tag
	case TagType::IntArray:
		return 4 // Size
			+ value.v_int_array.size * 4; // Ints
	}
	return 0;
}


#define WRITE_BYTES(b, n)\
	memcpy((void*) (bytes + index), (void*) &(b), (n));\
	index += (n);
#define WRITE_BYTE(x) bytes[index++] = (Byte) x;

#define WRITE_BYTE_O(x, offset)\
	bytes[index++] = (Byte) ((x) >> (8 * (offset))) & 0xFF;
#define WRITE_SHORT_O(x, offset)\
	WRITE_BYTE_O(x, offset + 1)\
	WRITE_BYTE_O(x, offset)
#define WRITE_INT_O(x, offset)\
	WRITE_SHORT_O(x, offset + 2)\
	WRITE_SHORT_O(x, offset)
#define WRITE_LONG(x)\
	WRITE_INT_O(x, 4)\
	WRITE_INT_O(x, 0)

#define WRITE_SHORT(x) WRITE_SHORT_O(x, 0)
#define WRITE_INT(x) WRITE_INT_O(x, 0)

#define WRITE_STRING(str, size)\
	WRITE_SHORT(size)\
	memcpy((void*) (bytes + index), (void*) str, size);\
	index += size;

std::string Tag::write() const
{
	ULong index = 0;
	UInt i = 0;
	ULong size = getSerializedSize() + 1;  // Add tag size
	Byte bytes[size];
	std::string str;

	WRITE_BYTE(type)

	switch (type) {
	case TagType::Byte:
		WRITE_BYTE(value.v_byte)
		break;
	case TagType::Short:
		WRITE_SHORT(value.v_short)
		break;
	case TagType::Int:
		WRITE_INT(value.v_int)
		break;
	case TagType::Long:
		WRITE_LONG(value.v_long)
		break;
	case TagType::Float:
		WRITE_BYTES(value.v_float, 4);
		break;
	case TagType::Double:
		WRITE_BYTES(value.v_double, 8);
		break;
	case TagType::ByteArray:
		WRITE_INT(value.v_byte_array.size)
		WRITE_BYTES(value.v_byte_array.value,
				value.v_byte_array.size)
		break;
	case TagType::String:
		WRITE_STRING(value.v_string.value, value.v_string.size)
		break;
	case TagType::List:
		WRITE_BYTE(value.v_list.tagid)
		WRITE_INT(value.v_list.size)
		for (; i < value.v_list.size; i++) {
			str = value.v_list.value[i].write();
			strncpy((char*) (bytes + index), str.c_str(), str.size());
			index += str.size();
		}
		break;
	case TagType::Compound:
		for (auto &it : *value.v_compound) {
			WRITE_BYTE(it.second.type)
			WRITE_STRING(it.first.c_str(), it.first.size())
			str = it.second.write();
			// Skip first byte (tag id)
			memcpy((void*) (bytes + index),
				(void*) (str.c_str() + 1), str.size() - 1);
			index += str.size() - 1;
		}
		WRITE_BYTE(TagType::End);
		break;
	case TagType::IntArray:
		WRITE_SHORT(value.v_int_array.size)
		WRITE_BYTES(value.v_int_array.value,
				value.v_int_array.size * sizeof(Int))
		break;
	}

	return std::string((char*) bytes, size);
}

std::string Tag::dump() const
{
	bool first = true;
	std::ostringstream os;
	switch (type) {
	case TagType::End:
		os << "<END>";
		break;
	case TagType::Byte:
		os << (Short) value.v_byte;
		break;
	case TagType::Short:
		os << value.v_short;
		break;
	case TagType::Int:
		os << value.v_int;
		break;
	case TagType::Long:
		os << value.v_long;
		break;
	case TagType::Float:
		os << value.v_float;
		break;
	case TagType::Double:
		os << value.v_double;
		break;
	case TagType::ByteArray:
		os << "byte[";
		for (UInt i = 0; i < value.v_byte_array.size; i++) {
			if (i != 0) {
				os << ", ";
			}
			os << (Short) value.v_byte_array.value[i];
		}
		os << ']';
		break;
	case TagType::String:
		os << '"' << std::string(value.v_string.value, value.v_string.size) << '"';
		break;
	case TagType::List:
		os << '[';
		for (UInt i = 0; i < value.v_list.size; i++) {
			if (i != 0) {
				os << ", ";
			}
			os << value.v_list.value[i].dump();
		}
		os << ']';
		break;
	case TagType::Compound:
		os << '{';
		for (auto &it : *value.v_compound) {
			if (!first) {
				os << ", ";
			}
			first = false;
			os << '"' << it.first << "\" = " << it.second.dump();
		}
		os << '}';
		break;
	case TagType::IntArray:
		os << "int[";
		for (UInt i = 0; i < value.v_int_array.size; i++) {
			if (i != 0) {
				os << ", ";
			}
			os << value.v_int_array.value[i];
		}
		os << ']';
		break;
	default:
		os << "<UNKNOWN TAG>";
	}
	return os.str();
}



/*******************
 * Deserialization *
 *******************/

void Tag::read(const Byte *bytes)
{
	ULong index = 0;
	TagType tag = (TagType) read_byte(bytes, index);
	read_tag(bytes, index, tag);
}

void Tag::read_tag(const Byte *bytes, ULong &index, TagType tag)
{
	type = tag;
	switch (tag) {
	case TagType::End:
		break;
	case TagType::Byte:
		value.v_byte = read_byte(bytes, index);
		break;
	case TagType::Short:
		value.v_short = read_short(bytes, index);
		break;
	case TagType::Int:
		value.v_int = read_int(bytes, index);
		break;
	case TagType::Long:
		value.v_long = read_long(bytes, index);
		break;
	case TagType::Float:
		value.v_float = read_float(bytes, index);
		break;
	case TagType::Double:
		value.v_double = read_double(bytes, index);
		break;
	case TagType::ByteArray:
		value.v_byte_array = read_byte_array(bytes, index);
		break;
	case TagType::String:
		value.v_string = read_string(bytes, index);
		break;
	case TagType::List:
		value.v_list = read_list(bytes, index);
		break;
	case TagType::Compound:
		value.v_compound = read_compound(bytes, index);
		break;
	case TagType::IntArray:
		value.v_int_array = read_int_array(bytes, index);
		break;
	default:
		throw "Invalid tag type!";
	}
}

inline Byte Tag::read_byte(const Byte *bytes, ULong &index)
{
	return bytes[index++];
}

inline Short Tag::read_short(const Byte *bytes, ULong &index)
{
	return ((Short) bytes[index++] << 8)
			| ((Short) bytes[index++]);
}

inline Int Tag::read_int(const Byte *bytes, ULong &index)
{
	return ((Int) bytes[index++] << 24)
			| ((Int) bytes[index++] << 16)
			| ((Int) bytes[index++] << 8)
			| ((Int) bytes[index++]);
}

inline Long Tag::read_long(const Byte *bytes, ULong &index)
{
	return ((Long) bytes[index++] << 56)
			| ((Long) bytes[index++] << 48)
			| ((Long) bytes[index++] << 40)
			| ((Long) bytes[index++] << 32)
			| ((Long) bytes[index++] << 24)
			| ((Long) bytes[index++] << 16)
			| ((Long) bytes[index++] << 8 )
			| ((Long) bytes[index++]);
}

inline float Tag::read_float(const Byte *bytes, ULong &index)
{
	float x;
	memcpy((void*) &x, (void*) (bytes + index), 4);
	index += 4;
	return x;
}

inline double Tag::read_double(const Byte *bytes, ULong &index)
{
	double x;
	memcpy((void*) &x, (void*) (bytes + index), 8);
	index += 8;
	return x;
}

ByteArray Tag::read_byte_array(const Byte *bytes, ULong &index)
{
	ByteArray x;
	x.size = read_int(bytes, index);
	x.value = new Byte[x.size];
	for (UInt i = 0; i < x.size; i++) {
		x.value[i] = read_byte(bytes, index);
	}
	return x;
}

String Tag::read_string(const Byte *bytes, ULong &index)
{
	String x;
	x.size = read_short(bytes, index);
	x.value = new char[x.size];
	for (UShort i = 0; i < x.size; i++) {
		x.value[i] = read_byte(bytes, index);
	}
	return x;
}

List Tag::read_list(const Byte *bytes, ULong &index)
{
	List x;
	x.tagid = (TagType) read_byte(bytes, index);
	x.size = read_int(bytes, index);
	x.value = new Tag[x.size];
	for (UInt i = 0; i < x.size; i++) {
		x.value[i].read_tag(bytes, index, x.tagid);
	}
	return x;
}

/*
 * TAG_Compound format:
 * TagType typeid = TagType::Compound
 * Repeat for each entry:
 *     TagType entrytype
 *     UShort keylen
 *     char key[keylen]
 *     TagType valtype
 *     Tag value
 * TagType entrytype = TagType::End
 */

Compound *Tag::read_compound(const Byte *bytes, ULong &index)
{
	Compound *x = new Compound;
	TagType tag;
	while ((tag = (TagType) bytes[index++]) != TagType::End) {
		String name = read_string(bytes, index);
		(*x)[std::string(name.value, name.size)]
				.read_tag(bytes, index, tag);
		delete [] name.value;
	}
	return x;
}

IntArray Tag::read_int_array(const Byte *bytes, ULong &index)
{
	IntArray x;
	x.size = read_int(bytes, index);
	x.value = new Int[x.size];
	for (UInt i = 0; i < x.size; i++) {
		x.value[i] = read_int(bytes, index);
	}
	return x;
}

} // namespace NBT

