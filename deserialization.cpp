
#include "nbt.h"
#include <cstring>


namespace NBT {

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


void Tag::read(const Byte *bytes)
{
	ULong index = 0;
	free();
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

inline Byte read_byte(const Byte *bytes, ULong &index)
{
	return bytes[index++];
}

inline Short read_short(const Byte *bytes, ULong &index)
{
	return ((Short) bytes[index++] << 8)
			| ((Short) bytes[index++]);
}

inline Int read_int(const Byte *bytes, ULong &index)
{
	return ((Int) bytes[index++] << 24)
			| ((Int) bytes[index++] << 16)
			| ((Int) bytes[index++] << 8)
			| ((Int) bytes[index++]);
}

inline Long read_long(const Byte *bytes, ULong &index)
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

inline float read_float(const Byte *bytes, ULong &index)
{
	float x;
	memcpy((void*) &x, (void*) (bytes + index), 4);
	index += 4;
	return x;
}

inline double read_double(const Byte *bytes, ULong &index)
{
	double x;
	memcpy((void*) &x, (void*) (bytes + index), 8);
	index += 8;
	return x;
}

ByteArray read_byte_array(const Byte *bytes, ULong &index)
{
	ByteArray x;
	x.size = read_int(bytes, index);
	x.value = new Byte[x.size];
	for (UInt i = 0; i < x.size; i++) {
		x.value[i] = read_byte(bytes, index);
	}
	return x;
}

String read_string(const Byte *bytes, ULong &index)
{
	String x;
	x.size = read_short(bytes, index);
	x.value = new char[x.size];
	for (UShort i = 0; i < x.size; i++) {
		x.value[i] = read_byte(bytes, index);
	}
	return x;
}

List read_list(const Byte *bytes, ULong &index)
{
	List x;
	x.tagid = (TagType) read_byte(bytes, index);
	x.size = read_int(bytes, index);
	if (x.size > 0) {
		x.value = new Tag[x.size];
	}
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

Compound *read_compound(const Byte *bytes, ULong &index)
{
	Compound *x = new Compound;
	TagType tag;
	while ((tag = (TagType) bytes[index++]) != TagType::End) {
		String name = read_string(bytes, index);
		(*x)[std::string(name.value, name.size)]
				.read_tag(bytes, index, tag);
		if (name.size > 0) {
			delete [] name.value;
		}
	}
	return x;
}

IntArray read_int_array(const Byte *bytes, ULong &index)
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

