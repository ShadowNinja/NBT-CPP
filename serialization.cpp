
#include "nbt.h"
#include <cstring>
#include <sstream>

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

namespace NBT {

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

} // namespace NBT

