
#include "nbt.h"
#include <cstring>
#include <cassert>

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

Tag::Tag(const TagType tag, uint32_t size)
{
	type = NBT::TagType::End;
	setTag(tag, size);
}

Tag::Tag(const Byte x)    : type(TagType::Byte)    { value.v_byte = x; }
Tag::Tag(const int16_t x) : type(TagType::Short)   { value.v_short = x; }
Tag::Tag(const int32_t x) : type(TagType::Int)     { value.v_int = x; }
Tag::Tag(const int64_t x) : type(TagType::Long)    { value.v_long = x; }
Tag::Tag(const float x)   : type(TagType::Float)   { value.v_float = x; }
Tag::Tag(const double x)  : type(TagType::Double)  { value.v_double = x; }

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

Tag & Tag::operator[](const int32_t &k)
{
	uint32_t ak = TOABS(k, value.v_list.size);
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

Tag & Tag::operator+=(const int32_t &i)
{
	assert(type == TagType::IntArray);
	ensureSize<IntArray, int32_t>(&value.v_int_array, value.v_int_array.size + 1);
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

void Tag::setTag(const TagType tag, uint32_t size)
{
	free();
	type = tag;
	switch (type) {
	case TagType::ByteArray:
		value.v_byte_array.size = size;
		value.v_byte_array.value = new Byte[size];
		break;
	case TagType::String:
		value.v_string.size = size;
		value.v_string.value = new char[size];
		break;
	case TagType::List:
		value.v_list.size = size;
		value.v_list.value = new Tag[size];
		break;
	case TagType::Compound:
		value.v_compound = new Compound;
		break;
	case TagType::IntArray:
		value.v_int_array.size = size;
		value.v_int_array.value = new int32_t[size];
		break;
	default:
		memset((void*) &value, 0, sizeof(value));
	}
}

void Tag::copy(const Tag &t)
{
	free();
	uint64_t size;
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
		for (uint32_t i = 0; i < size; i++) {
			value.v_list.value[i] = t.value.v_list.value[i];
		}
		break;
	case TagType::Compound:
		value.v_compound = new Compound(*t.value.v_compound);
		break;
	case TagType::IntArray:
		size = t.value.v_int_array.size;
		value.v_int_array.size = size;
		value.v_int_array.value = new int32_t[size];
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
	void Tag::ensureSize(container *field, uint32_t size)
{
	if (size > field->size) {
		container newc;
		newc.size = size;
		newc.value = new contained[size];
		for (uint32_t i = 0; i < field->size; i++) {
			newc.value[i] = std::move(field->value[i]);
		}
		delete [] field->value;
		*field = newc;
	}
}

void Tag::insert(const int32_t &k, const Byte &b)
{
	assert(type == TagType::ByteArray);
	uint32_t ak = TOABS(k, value.v_list.size);
	ensureSize<ByteArray, Byte>(&value.v_byte_array, ak);
	value.v_byte_array.value[ak] = b;
}

void Tag::insert(const int32_t &k, const int32_t &i)
{
	assert(type == TagType::IntArray);
	uint32_t ak = TOABS(k, value.v_list.size);
	ensureSize<IntArray, int32_t>(&value.v_int_array, ak);
	value.v_int_array.value[ak] = i;
}

void Tag::insert(const int32_t &k, const Tag &t)
{
	assert(type == TagType::List);
	uint32_t ak = TOABS(k, value.v_list.size);
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
int16_t    Tag::toShort()     { return value.v_short; }
int32_t    Tag::toInt()       { return value.v_int; }
int64_t    Tag::toLong()      { return value.v_long; }
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
uint64_t Tag::getSerializedSize() const
{
	uint64_t size = 0;
	uint64_t i = 0;
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
	strncpy((char*) (bytes + index), str, size);\
	index += size;

std::string Tag::write() const
{
	uint64_t index = 0;
	uint32_t i = 0;
	uint64_t size = getSerializedSize() + 1;  // Add tag size
	uint16_t slen;
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
			slen = it.first.size();
			WRITE_STRING(it.first.c_str(), slen)
			str = it.second.write();
			// Skip first byte (tag id)
			strncpy((char*) (bytes + index), str.c_str() + 1, str.size() - 1);
			index += str.size() - 1;
		}
		WRITE_BYTE(TagType::End);
		break;
	case TagType::IntArray:
		WRITE_SHORT(value.v_int_array.size)
		WRITE_BYTES(value.v_int_array.value,
				value.v_int_array.size * sizeof(int32_t))
		break;
	}

	return std::string((char*) bytes, size);
}



/*******************
 * Deserialization *
 *******************/

void Tag::read(const Byte *bytes)
{
	uint64_t index = 0;
	TagType tag = (TagType) read_byte(bytes, index);
	read_tag(bytes, index, tag);
}

void Tag::read_tag(const Byte *bytes, uint64_t &index, TagType tag)
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

inline Byte Tag::read_byte(const Byte *bytes, uint64_t &index)
{
	return bytes[index++];
}

inline int16_t Tag::read_short(const Byte *bytes, uint64_t &index)
{
	return ((int16_t) bytes[index++] << 8)
			| ((int16_t) bytes[index++]);
}

inline int32_t Tag::read_int(const Byte *bytes, uint64_t &index)
{
	return ((int32_t) bytes[index++] << 24)
			| ((int32_t) bytes[index++] << 16)
			| ((int32_t) bytes[index++] << 8)
			| ((int32_t) bytes[index++]);
}

inline int64_t Tag::read_long(const Byte *bytes, uint64_t &index)
{
	return ((int64_t) bytes[index++] << 56)
			| ((int64_t) bytes[index++] << 48)
			| ((int64_t) bytes[index++] << 40)
			| ((int64_t) bytes[index++] << 32)
			| ((int64_t) bytes[index++] << 24)
			| ((int64_t) bytes[index++] << 16)
			| ((int64_t) bytes[index++] << 8 )
			| ((int64_t) bytes[index++]);
}

inline float Tag::read_float(const Byte *bytes, uint64_t &index)
{
	float x;
	memcpy((void*) &x, (void*) (bytes + index), 4);
	index += 4;
	return x;
}

inline double Tag::read_double(const Byte *bytes, uint64_t &index)
{
	double x;
	memcpy((void*) &x, (void*) (bytes + index), 8);
	index += 8;
	return x;
}

ByteArray Tag::read_byte_array(const Byte *bytes, uint64_t &index)
{
	ByteArray x;
	x.size = read_int(bytes, index);
	x.value = new Byte[x.size];
	for (uint32_t i = 0; i < x.size; i++) {
		x.value[i] = read_byte(bytes, index);
	}
	return x;
}

String Tag::read_string(const Byte *bytes, uint64_t &index)
{
	String x;
	x.size = read_short(bytes, index);
	x.value = new char[x.size];
	for (uint16_t i = 0; i < x.size; i++) {
		x.value[i] = read_byte(bytes, index);
	}
	return x;
}

List Tag::read_list(const Byte *bytes, uint64_t &index)
{
	List x;
	x.tagid = (TagType) read_byte(bytes, index);
	x.size = read_int(bytes, index);
	x.value = new Tag[x.size];
	for (uint32_t i = 0; i < x.size; i++) {
		x.value[i].read_tag(bytes, index, x.tagid);
	}
	return x;
}

/*
 * TAG_Compound format:
 * TagType typeid = TagType::Compound
 * Repeat for each entry:
 *     TagType entrytype
 *     uint16_t keylen
 *     char key[keylen]
 *     TagType valtype
 *     Tag value
 * TagType entrytype = TagType::End
 */

Compound *Tag::read_compound(const Byte *bytes, uint64_t &index)
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

IntArray Tag::read_int_array(const Byte *bytes, uint64_t &index)
{
	IntArray x;
	x.size = read_int(bytes, index);
	x.value = new int32_t[x.size];
	for (uint32_t i = 0; i < x.size; i++) {
		x.value[i] = read_int(bytes, index);
	}
	return x;
}

} // namespace NBT

