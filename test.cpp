
#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <cassert>

#include "nbt.h"

std::string hexdump(std::string s);

int main(int argc, char *argv[]) {
	std::string data(
		"\x0A" // compound
			"\x01" // Byte
				"\x00\x01" // Size 1
				"A" // key "A"
				"\x40" // value, 0x40
			"\x08" // String
				"\x00\x06"
				"foobar"
				"\x00\x09"
				"<3 C++ 11"
			"\x03" // Int
				"\x00\x04" // Size 4
				"test" // key "test"
				"\x12\x34\x56\x78" // value, 0x12345678
			"\x00" // End
		, 38);

	NBT::Tag root((NBT::Byte*) data.c_str());
	std::cout << hexdump(data) << std::endl;
	std::cout << hexdump(root.write()) << std::endl;

	assert((NBT::Byte) root["A"] == 0x40);
	assert((NBT::Int) root["test"] == 0x12345678);
	assert(strcmp(((NBT::String) root["foobar"]).value, "<3 C++ 11") == 0);

	//assert(root.write() == data); // Data is unordered

	root["foo"] = NBT::TagType::List;

	assert(root["foo"].type == NBT::TagType::List);

	root["foo"][0] = NBT::TagType::List;
	root["foo"] += NBT::TagType::List;

	root["foo"][1][0] = 123L;
	assert((NBT::Long) root["foo"][1][0] == 123L);
	std::cout << root.dump() << std::endl;

	root = NBT::TagType::Compound;  // Reset
	root["A"] = (NBT::Byte) 0x40;
	root["test"] = (NBT::Int) 0x12345678;	
	root["foobar"] = std::string("<3 C++ 11");
	std::cout << hexdump(root.write()) << std::endl;
	std::cout << root.dump() << std::endl;

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

