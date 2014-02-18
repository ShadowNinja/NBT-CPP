
#include <iostream>
#include <string>
#include <sstream>
#include <cassert>

#include "nbt.h"

std::string hexdump(std::string s);

int main(int argc, char *argv[]) {
	std::string data(
		"\x0A" // compound
			"\x01" // Byte
				"\x00\x01" // Size 1
				"\x41" // key "A"
				"\x40" // value, 0x40
			"\x03" // Int
				"\x00\x04" // Size 4
				"\x74\x65\x73\x74" // key "test"
				"\x12\x34\x56\x78" // value, 0x12345678
			"\x00" // End
		, 18);

	NBT::Tag tag((NBT::Byte*) data.c_str());
	std::cout << hexdump(data) << std::endl;
	std::cout << hexdump(tag.write()) << std::endl;

	assert(tag["A"].toByte() == 0x40);
	assert(tag["test"].toInt() == 0x12345678);

	assert(tag.write() == data);

	NBT::Tag root(NBT::TagType::Compound);
	//root.insert("foo", NBT::Tag(NBT::TagType::List));
	root["foo"] = NBT::TagType::List;

	assert(root["foo"].type == NBT::TagType::List);

	root["foo"][0] = NBT::TagType::List;
	root["foo"] += NBT::TagType::List;

	root["foo"][1][0] = 123L;
	assert(root["foo"][1][0].toLong() == 123L);

	std::cout << "Success!" << std::endl;
	return 0;
}

std::string hexdump(std::string s)
{
	std::ostringstream os;
	os << std::hex;
	for (char c : s) {
		os << (int) c << " ";
	}
	return os.str();
}

