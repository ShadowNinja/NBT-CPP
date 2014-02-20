
#include "nbt.h"
#include <cstring>


namespace NBT {

inline UByte  readByte  (const UByte *bytes, ULong &index);
inline UShort readShort (const UByte *bytes, ULong &index);
inline UInt   readInt   (const UByte *bytes, ULong &index);
inline ULong  readLong  (const UByte *bytes, ULong &index);
inline float  readFloat (const UByte *bytes, ULong &index);
inline double readDouble(const UByte *bytes, ULong &index);
ByteArray  readByteArray(const UByte *bytes, ULong &index);
String     readString   (const UByte *bytes, ULong &index);
List       readList     (const UByte *bytes, ULong &index);
Compound * readCompound (const UByte *bytes, ULong &index);
IntArray   readIntArray (const UByte *bytes, ULong &index);


void Tag::read(const UByte *bytes)
{
	ULong index = 0;
	free();
	TagType tag = (TagType) readByte(bytes, index);
	readTag(bytes, index, tag);
}

void Tag::readTag(const UByte *bytes, ULong &index, TagType tag)
{
	type = tag;
	switch (tag) {
	case TagType::End:
		break;
	case TagType::Byte:
		value.v_byte = readByte(bytes, index);
		break;
	case TagType::Short:
		value.v_short = readShort(bytes, index);
		break;
	case TagType::Int:
		value.v_int = readInt(bytes, index);
		break;
	case TagType::Long:
		value.v_long = readLong(bytes, index);
		break;
	case TagType::Float:
		value.v_float = readFloat(bytes, index);
		break;
	case TagType::Double:
		value.v_double = readDouble(bytes, index);
		break;
	case TagType::ByteArray:
		value.v_byte_array = readByteArray(bytes, index);
		break;
	case TagType::String:
		value.v_string = readString(bytes, index);
		break;
	case TagType::List:
		value.v_list = readList(bytes, index);
		break;
	case TagType::Compound:
		value.v_compound = readCompound(bytes, index);
		break;
	case TagType::IntArray:
		value.v_int_array = readIntArray(bytes, index);
		break;
	default:
		throw "Invalid tag type!";
	}
}

inline UByte readByte(const UByte *bytes, ULong &index)
{
	return bytes[index++];
}

inline UShort readShort(const UByte *bytes, ULong &index)
{
	return (bytes[index++] << 8) | (bytes[index++]);
}

inline UInt readInt(const UByte *bytes, ULong &index)
{
	return ((UInt) bytes[index++] << 24)
			| ((UInt) bytes[index++] << 16)
			| ((UInt) bytes[index++] << 8)
			| ((UInt) bytes[index++]);
}

inline ULong readLong(const UByte *bytes, ULong &index)
{
	return ((ULong) bytes[index++] << 56)
			| ((ULong) bytes[index++] << 48)
			| ((ULong) bytes[index++] << 40)
			| ((ULong) bytes[index++] << 32)
			| ((ULong) bytes[index++] << 24)
			| ((ULong) bytes[index++] << 16)
			| ((ULong) bytes[index++] << 8 )
			| ((ULong) bytes[index++]);
}

inline float readFloat(const UByte *bytes, ULong &index)
{
	float x;
	memcpy((void*) &x, (void*) (bytes + index), sizeof(float));
	index += sizeof(float);
	return x;
}

inline double readDouble(const UByte *bytes, ULong &index)
{
	double x;
	memcpy((void*) &x, (void*) (bytes + index), sizeof(double));
	index += sizeof(double);
	return x;
}

ByteArray readByteArray(const UByte *bytes, ULong &index)
{
	ByteArray x;
	x.size = readInt(bytes, index);
	x.value = new Byte[x.size];
	for (UInt i = 0; i < x.size; i++) {
		x.value[i] = readByte(bytes, index);
	}
	return x;
}

String readString(const UByte *bytes, ULong &index)
{
	String x;
	x.size = readShort(bytes, index);
	x.value = new char[x.size];
	for (UShort i = 0; i < x.size; i++) {
		x.value[i] = readByte(bytes, index);
	}
	return x;
}

List readList(const UByte *bytes, ULong &index)
{
	List x;
	x.tagid = (TagType) readByte(bytes, index);
	x.size = readInt(bytes, index);
	if (x.size > 0) {
		x.value = new Tag[x.size];
	}
	for (UInt i = 0; i < x.size; i++) {
		x.value[i].readTag(bytes, index, x.tagid);
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

Compound *readCompound(const UByte *bytes, ULong &index)
{
	Compound *x = new Compound;
	TagType tag;
	while ((tag = (TagType) bytes[index++]) != TagType::End) {
		String name = readString(bytes, index);
		(*x)[std::string(name.value, name.size)]
				.readTag(bytes, index, tag);
		if (name.size > 0) {
			delete [] name.value;
		}
	}
	return x;
}

IntArray readIntArray(const UByte *bytes, ULong &index)
{
	IntArray x;
	x.size = readShort(bytes, index);
	if (x.size > 0) x.value = new Int[x.size];
	for (UInt i = 0; i < x.size; i++) {
		x.value[i] = readInt(bytes, index);
	}
	return x;
}

} // namespace NBT

