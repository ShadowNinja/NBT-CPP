
#include <cstring>
#include <sstream>

#include "nbt.h"
#include "serialization.h"


namespace NBT {

/*****************
 * Serialization *
 *****************/

// Doesn't include size of tagid (always 1)
ULong Tag::getSerializedSize() const
{
	ULong size = 0;
	ULong i = 0;
	switch (type) {
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

	writeByte(bytes, index, (UByte) type);

	switch (type) {
	case TagType::Byte:
		writeByte(bytes, index, value.v_byte);
		break;
	case TagType::Short:
		writeShort(bytes, index, value.v_short);
		break;
	case TagType::Int:
		writeInt(bytes, index, value.v_int);
		break;
	case TagType::Long:
		writeLong(bytes, index, value.v_long);
		break;
	case TagType::Float:
		writeBytes(bytes, index, (UByte *) &value.v_float, sizeof(float));
		break;
	case TagType::Double:
		writeBytes(bytes, index, (UByte *) &value.v_double, sizeof(double));
		break;
	case TagType::ByteArray:
		writeInt(bytes, index, value.v_byte_array.size);
		writeBytes(bytes, index, (UByte *) value.v_byte_array.value,
				value.v_byte_array.size);
		break;
	case TagType::String:
		writeString(bytes, index, value.v_string.value, value.v_string.size);
		break;
	case TagType::List:
		writeByte(bytes, index, (UByte) value.v_list.tagid);
		writeInt(bytes, index, value.v_list.size);
		for (; i < value.v_list.size; i++) {
			str = value.v_list.value[i].write();
			strncpy((char*) (bytes + index), str.c_str(), str.size());
			index += str.size();
		}
		break;
	case TagType::Compound:
		for (auto &it : *value.v_compound) {
			writeByte(bytes, index, (UByte) it.second.type);
			writeString(bytes, index, it.first.c_str(), it.first.size());
			str = it.second.write();
			// Skip first byte (tag id)
			memcpy((void *) (bytes + index),
				(void *) (str.data() + sizeof(UByte)), str.size() - sizeof(UByte));
			index += str.size() - sizeof(UByte);
		}
		writeByte(bytes, index, (UByte) TagType::End);
		break;
	case TagType::IntArray:
		writeShort(bytes, index, value.v_int_array.size);
		// Can't use memcpy, or you have to account for endianess
		for (; i < value.v_int_array.size; i++) {
			writeInt(bytes, index, value.v_int_array.value[i]);
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


inline void writeBytes(UByte * bytes, ULong & index, const UByte * write, UInt size)
{
	memcpy((void *) (bytes + index), (void *) write, size);\
	index += size;
}


inline void writeByte(UByte * bytes, ULong & index, UByte b)
{
	bytes[index++] = (UByte) b;
}


inline void writeShort(UByte * bytes, ULong & index, UShort s)
{
	bytes[index++] = (UByte) (s >> 8) & 0xFF;
	bytes[index++] = (UByte)  s       & 0xFF;
}


inline void writeInt(UByte * bytes, ULong & index, UInt i)
{
	bytes[index++] = (UByte) (i >> 24) & 0xFF;
	bytes[index++] = (UByte) (i >> 16) & 0xFF;
	bytes[index++] = (UByte) (i >> 8 ) & 0xFF;
	bytes[index++] = (UByte)  i        & 0xFF;
}


inline void writeLong(UByte * bytes, ULong & index, ULong l)
{
	bytes[index++] = (UByte) (l >> 56) & 0xFF;
	bytes[index++] = (UByte) (l >> 48) & 0xFF;
	bytes[index++] = (UByte) (l >> 40) & 0xFF;
	bytes[index++] = (UByte) (l >> 32) & 0xFF;
	bytes[index++] = (UByte) (l >> 24) & 0xFF;
	bytes[index++] = (UByte) (l >> 16) & 0xFF;
	bytes[index++] = (UByte) (l >> 8 ) & 0xFF;
	bytes[index++] = (UByte)  l        & 0xFF;
}


inline void writeString(UByte * bytes, ULong & index, const char * str, UInt size)
{
	writeShort(bytes, index, size);
	writeBytes(bytes, index, (const UByte *) str, size);
}


/*******************
 * Deserialization *
 *******************/

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

