
#include "nbt.h"
#include <cstring>
#include <sstream>

#define WRITE_BYTES(b, n)\
	memcpy((void*) (bytes + index), (void*) &(b), (n));\
	index += (n);

#define WRITE_BYTE(x)\
	bytes[index++] = (Byte) x;
#define WRITE_SHORT(x)\
	bytes[index++] = (Byte) ((x) >> 8 ) & 0xFF;\
	bytes[index++] = (Byte)  (x)        & 0xFF;
#define WRITE_INT(x)\
	bytes[index++] = (Byte) ((x) >> 24) & 0xFF;\
	bytes[index++] = (Byte) ((x) >> 16) & 0xFF;\
	bytes[index++] = (Byte) ((x) >> 8 ) & 0xFF;\
	bytes[index++] = (Byte)  (x)        & 0xFF;
#define WRITE_LONG(x)\
	bytes[index++] = (Byte) ((x) >> 56) & 0xFF;\
	bytes[index++] = (Byte) ((x) >> 48) & 0xFF;\
	bytes[index++] = (Byte) ((x) >> 40) & 0xFF;\
	bytes[index++] = (Byte) ((x) >> 32) & 0xFF;\
	bytes[index++] = (Byte) ((x) >> 24) & 0xFF;\
	bytes[index++] = (Byte) ((x) >> 16) & 0xFF;\
	bytes[index++] = (Byte) ((x) >> 8 ) & 0xFF;\
	bytes[index++] = (Byte)  (x)        & 0xFF;

#define WRITE_STRING(str, size)\
	WRITE_SHORT(size)\
	memcpy((void*) (bytes + index), (void*) str, size);\
	index += size;

namespace NBT {

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
		WRITE_BYTES(value.v_float, sizeof(float));
		break;
	case TagType::Double:
		WRITE_BYTES(value.v_double, sizeof(double));
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
				(void*) (str.c_str() + sizeof(UByte)), str.size() - sizeof(UByte));
			index += str.size() - sizeof(UByte);
		}
		WRITE_BYTE(TagType::End);
		break;
	case TagType::IntArray:
		WRITE_SHORT(value.v_int_array.size)
		// Can't use memcpy, or you have to account for endianess
		for (; i < value.v_int_array.size; i++) {
			WRITE_INT(value.v_int_array.value[i])
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

} // namespace NBT

