
#include <cstring>
#include <cassert>

#include "nbt.h"

namespace NBT {

// Converts relative negative indexes to positive indexes
#define TOABS(x, s) (x < 0 ? s + x : x)

/*******************
 * con/destructors *
 *******************/

Tag::Tag() :
	type(TagType::End),
	value()
{}

Tag::Tag(const UByte *bytes) :
	type(TagType::End)
{
	read(bytes);
}

Tag::Tag(const TagType tag, UInt size) :
	type(TagType::End)
{
	setTag(tag, size);
}

Tag::Tag(Byte x)   : type(TagType::Byte)    { value.v_byte = x; }
Tag::Tag(Short x)  : type(TagType::Short)   { value.v_short = x; }
Tag::Tag(Int x)    : type(TagType::Int)     { value.v_int = x; }
Tag::Tag(Long x)   : type(TagType::Long)    { value.v_long = x; }
Tag::Tag(float x)  : type(TagType::Float)   { value.v_float = x; }
Tag::Tag(double x) : type(TagType::Double)  { value.v_double = x; }
Tag::Tag(std::string x)  : type(TagType::String)  {
	value.v_string.size = x.size();
	value.v_string.value = new char[x.size()];
	memcpy((void*) value.v_string.value, (void*) x.c_str(), x.size());
}

Tag::Tag(const Tag &t) :
	type(TagType::End)
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

/*************
 * Operators *
 *************/

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
	free();
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

Tag & Tag::operator[](const char *k)
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



/**************
 * Converters *
 **************/

Tag::operator Byte()      { return value.v_byte; }
Tag::operator Short()     { return value.v_short; }
Tag::operator Int()       { return value.v_int; }
Tag::operator Long()      { return value.v_long; }
Tag::operator float()     { return value.v_float; }
Tag::operator double()    { return value.v_double; }
Tag::operator ByteArray() { return value.v_byte_array; }
Tag::operator String()    { return value.v_string; }
Tag::operator List()      { return value.v_list; }
Tag::operator Compound()  { return *value.v_compound; }
Tag::operator IntArray()  { return value.v_int_array; }



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
		if (size) value.v_int_array.value = new Int[size];
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
		if (value.v_byte_array.size) delete [] value.v_byte_array.value;
		break;
	case TagType::String:
		if (value.v_string.size) delete [] value.v_string.value;
		break;
	case TagType::List:
		if (value.v_list.size) delete [] value.v_list.value;
		break;
	case TagType::Compound:
		delete value.v_compound;
		break;
	case TagType::IntArray:
		if (value.v_int_array.size) delete [] value.v_int_array.value;
		break;
	default:
		break;
	}
	type = TagType::End;  // Prevent double free
}



/**********
 * Insert *
 **********/

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

void Tag::insert(const Int k, const Byte b)
{
	assert(type == TagType::ByteArray);
	UInt ak = TOABS(k, value.v_list.size);
	ensureSize<ByteArray, Byte>(&value.v_byte_array, ak + 1);
	value.v_byte_array.value[ak] = b;
}

void Tag::insert(const Int k, const Int i)
{
	assert(type == TagType::IntArray);
	UInt ak = TOABS(k, value.v_list.size);
	ensureSize<IntArray, Int>(&value.v_int_array, ak + 1);
	value.v_int_array.value[ak] = i;
}

void Tag::insert(const Int k, const Tag &t)
{
	assert(type == TagType::List);
	assert(t.type == value.v_list.tagid);
	UInt ak = TOABS(k, value.v_list.size);
	ensureSize<List, Tag>(&value.v_list, ak + 1);
	value.v_list.value[ak] = t;
}

void Tag::insert(const std::string &k, const Tag &t)
{
	assert(type == TagType::Compound);
	(*value.v_compound)[k] = t;
}

} // namespace NBT

