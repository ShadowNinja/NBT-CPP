
#include <cstring>
#include <sstream>

#include "nbt.h"
#include "serialization.h"


#define NBT_BIG_ENDIAN    0x01020304UL
#define NBT_LITTLE_ENDIAN 0x04030201UL

namespace NBT {

constexpr UInt byteOrder() {
	return ((union {UByte bytes[4]; UInt i;}) {{1, 2, 3, 4}}).i;
}


inline void swapBytes(UByte * bytes, UByte len)
{
	UByte tmp;
	for (UByte i = 0; i < len / 2; i++) {
		tmp = bytes[len - i - 1];
		bytes[len - i - 1] = bytes[i];
		bytes[i] = tmp;
	}
}


/*****************
 * Serialization *
 *****************/

// Doesn't include size of tagid (always 1)
ULong Tag::getSerializedSize() const
{
	ULong size = 0;
	ULong i = 0;
	switch (type) {
	case TagType::End: return 0;
	case TagType::Byte: return sizeof(Byte);
	case TagType::Short: return sizeof(Short);
	case TagType::Int: return sizeof(Int);
	case TagType::Long: return sizeof(Long);
	case TagType::Float: return sizeof(float);
	case TagType::Double: return sizeof(double);
	case TagType::ByteArray:
		return sizeof(UInt) //Size field
			+ value.v_byte_array.size; // Array size
	case TagType::String:
		return sizeof(UShort) // Size field
			+ value.v_string.size; //String siza
	case TagType::List:
		for (; i < value.v_list.size; i++) {
			size += value.v_list.value[i].getSerializedSize();
		}
		return sizeof(UByte) // TagID
			+ sizeof(UInt) // Size
			+ size; // Items
	case TagType::Compound:
		for (auto &it : *value.v_compound) {
			size += 1 // Value type
				+ 2 // String size
				+ it.first.size() // String
				+ it.second.getSerializedSize(); // Value
		}
		return size
			+ sizeof(UByte); // End tag
	case TagType::IntArray:
		return sizeof(Short) // Size
			+ value.v_int_array.size * sizeof(Int); // Ints
	}
	return 0;
}


std::string Tag::write() const
{
	ULong index = 0;
	UInt i = 0;
	ULong size = getSerializedSize() + sizeof(UByte);  // Add tag size
	UByte bytes[size];
	std::string str;

	writeByte((bytes + index++), (UByte) type);

	switch (type) {
	case TagType::End:
		break;
	case TagType::Byte:
		writeByte((bytes + index), value.v_byte);
		index += sizeof(Byte);
		break;
	case TagType::Short:
		writeShort(bytes + index, value.v_short);
		index += sizeof(Short);
		break;
	case TagType::Int:
		writeInt(bytes + index, value.v_int);
		index += sizeof(Int);
		break;
	case TagType::Long:
		writeLong(bytes + index, value.v_long);
		index += sizeof(Long);
		break;
	case TagType::Float:
		writeFloat(bytes + index, value.v_float);
		index += sizeof(float);
		break;
	case TagType::Double:
		writeDouble(bytes + index, value.v_double);
		index += sizeof(double);
		break;
	case TagType::ByteArray:
		writeInt(bytes + index, value.v_byte_array.size);
		index += sizeof(Int);
		writeBytes(bytes + index, (UByte *) value.v_byte_array.value,
				value.v_byte_array.size);
		index += value.v_byte_array.size;
		break;
	case TagType::String:
		writeString(bytes + index, value.v_string.value, value.v_string.size);
		index += sizeof(Short) + value.v_string.size;
		break;
	case TagType::List:
		writeByte(bytes + index, (UByte) value.v_list.tagid);
		index += sizeof(Byte);
		writeInt(bytes + index, value.v_list.size);
		index += sizeof(Int);
		for (; i < value.v_list.size; i++) {
			str = value.v_list.value[i].write();
			// Skip first byte (tag id)
			writeBytes(bytes + index,
					(const UByte *) (str.data() + sizeof(UByte)),
					str.size() - sizeof(UByte));
			index += str.size() - sizeof(UByte);
		}
		break;
	case TagType::Compound:
		for (auto &it : *value.v_compound) {
			writeByte(bytes + index, (UByte) it.second.type);
			index += sizeof(Byte);
			writeString(bytes + index, it.first.data(), it.first.size());
			index += sizeof(Short) + it.first.size();
			str = it.second.write();
			// Skip first byte (tag id)
			writeBytes(bytes + index,
				(const UByte *) (str.data() + sizeof(UByte)),
				str.size() - sizeof(UByte));
			index += str.size() - sizeof(UByte);
		}
		writeByte(bytes + index, (UByte) TagType::End);
		index += sizeof(Byte);
		break;
	case TagType::IntArray:
		writeShort(bytes + index, value.v_int_array.size);
		index += sizeof(Short);
		// Can't use memcpy, or you have to account for endianess
		for (; i < value.v_int_array.size; i++) {
			writeInt(bytes + index, value.v_int_array.value[i]);
			index += sizeof(Int);
		}
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


inline void writeBytes(UByte * bytes, const UByte * write, UInt size)
{
	memcpy((void *) bytes, (void *) write, size);
}


inline void writeByte(UByte * bytes, UByte b)
{
	bytes[0] = b;
}


inline void writeShort(UByte * bytes, UShort s)
{
	bytes[0] = (UByte) (s >> 8) & 0xFF;
	bytes[1] = (UByte)  s       & 0xFF;
}


inline void writeInt(UByte * bytes, UInt i)
{
	bytes[0] = (UByte) (i >> 24) & 0xFF;
	bytes[1] = (UByte) (i >> 16) & 0xFF;
	bytes[2] = (UByte) (i >> 8 ) & 0xFF;
	bytes[3] = (UByte)  i        & 0xFF;
}


inline void writeLong(UByte * bytes, ULong l)
{
	bytes[0] = (UByte) (l >> 56) & 0xFF;
	bytes[1] = (UByte) (l >> 48) & 0xFF;
	bytes[2] = (UByte) (l >> 40) & 0xFF;
	bytes[3] = (UByte) (l >> 32) & 0xFF;
	bytes[4] = (UByte) (l >> 24) & 0xFF;
	bytes[5] = (UByte) (l >> 16) & 0xFF;
	bytes[6] = (UByte) (l >> 8 ) & 0xFF;
	bytes[7] = (UByte)  l        & 0xFF;
}


inline void writeFloat(UByte * bytes, float f)
{
	if (byteOrder() == NBT_LITTLE_ENDIAN) {
		swapBytes((UByte *) &f, sizeof(float));
	}
	memcpy((void *) bytes, (void *) &f, sizeof(float));
}


inline void writeDouble(UByte * bytes, double d)
{
	if (byteOrder() == NBT_LITTLE_ENDIAN) {
		swapBytes((UByte *) &d, sizeof(double));
	}
	memcpy((void *) bytes, (void *) &d, sizeof(double));
}


inline void writeString(UByte * bytes, const char * str, UShort size)
{
	writeShort(bytes, size);
	writeBytes(bytes + sizeof(Short), (const UByte *) str, size);
}


/*******************
 * Deserialization *
 *******************/

void Tag::read(const UByte *bytes)
{
	free();
	TagType tag = (TagType) readByte(bytes);
	ULong index = sizeof(Byte);
	readTag(bytes, index, tag);
}


void Tag::readTag(const UByte *bytes, ULong &index, TagType tag)
{
	type = tag;
	switch (tag) {
	case TagType::End:
		break;
	case TagType::Byte:
		value.v_byte = readByte(bytes + index);
		index += sizeof(Byte);
		break;
	case TagType::Short:
		value.v_short = readShort(bytes + index);
		index += sizeof(Short);
		break;
	case TagType::Int:
		value.v_int = readInt(bytes + index);
		index += sizeof(Int);
		break;
	case TagType::Long:
		value.v_long = readLong(bytes + index);
		index += sizeof(Long);
		break;
	case TagType::Float:
		value.v_float = readFloat(bytes + index);
		index += sizeof(float);
		break;
	case TagType::Double:
		value.v_double = readDouble(bytes + index);
		index += sizeof(double);
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


inline UByte readByte(const UByte * bytes)
{
	return bytes[0];
}


inline UShort readShort(const UByte * bytes)
{
	return ((UShort) bytes[0] << 8) | ((UShort) bytes[1]);
}


inline UInt readInt(const UByte * bytes)
{
	return ((UInt) bytes[0] << 24)
			| ((UInt) bytes[1] << 16)
			| ((UInt) bytes[2] << 8)
			| ((UInt) bytes[3]);
}


inline ULong readLong(const UByte *bytes)
{
	return ((ULong) bytes[0] << 56)
			| ((ULong) bytes[1] << 48)
			| ((ULong) bytes[2] << 40)
			| ((ULong) bytes[3] << 32)
			| ((ULong) bytes[4] << 24)
			| ((ULong) bytes[5] << 16)
			| ((ULong) bytes[6] << 8 )
			| ((ULong) bytes[7]);
}


inline float readFloat(const UByte *bytes)
{
	float x;
	memcpy((void *) &x, (void *) bytes, sizeof(float));
	if (byteOrder() == NBT_LITTLE_ENDIAN) {
		swapBytes((UByte *) &x, sizeof(float));
	}
	return x;
}


inline double readDouble(const UByte *bytes)
{
	double x;
	memcpy((void*) &x, (void *) bytes, sizeof(double));
	if (byteOrder() == NBT_LITTLE_ENDIAN) {
		swapBytes((UByte *) &x, sizeof(double));
	}
	return x;
}


ByteArray readByteArray(const UByte *bytes, ULong &index)
{
	ByteArray x;
	x.size = readInt(bytes + index);
	index += sizeof(Int);
	x.value = new Byte[x.size];
	for (UInt i = 0; i < x.size; i++) {
		x.value[i] = readByte(bytes + index);
		index += sizeof(Byte);
	}
	return x;
}


String readString(const UByte *bytes, ULong &index)
{
	String x;
	x.size = readShort(bytes + index);
	index += sizeof(Short);
	x.value = new char[x.size];
	for (UShort i = 0; i < x.size; i++) {
		x.value[i] = readByte(bytes + index);
		index += sizeof(Byte);
	}
	return x;
}


List readList(const UByte *bytes, ULong &index)
{
	List x;
	x.tagid = (TagType) readByte(bytes + index);
	index += sizeof(Byte);
	x.size = readInt(bytes + index);
	index += sizeof(Int);
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
	x.size = readShort(bytes + index);
	index += sizeof(Short);
	if (x.size > 0) {
		x.value = new Int[x.size];
	}
	for (UInt i = 0; i < x.size; i++) {
		x.value[i] = readInt(bytes + index);
		index += sizeof(Int);
	}
	return x;
}

} // namespace NBT

