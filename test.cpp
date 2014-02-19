
#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <cassert>
#include <chrono>

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
	NBT::String foobar = root["foobar"];
	assert(std::string(foobar.value, foobar.size) == "<3 C++ 11");

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

	std::cout << "Testing reading performance..." << std::endl;
	using namespace std::chrono;
	high_resolution_clock::time_point start = high_resolution_clock::now();
	for (uint32_t i = 0; i < 10000; i++) {
		root.read((NBT::Byte*) data.c_str());
	}
	std::cout << "Completed 10,000 reads in " <<
			duration_cast<duration<double>>(high_resolution_clock::now() - start).count()
			<< " seconds." << std::endl;

	std::cout << "Testing writing performance..." << std::endl;
	start = high_resolution_clock::now();
	for (uint32_t i = 0; i < 10000; i++) {
		root.write();
	}
	std::cout << "Completed 10,000 writes in " <<
			duration_cast<duration<double>>(high_resolution_clock::now() - start).count()
			<< " seconds." << std::endl;

	std::cout << "Success!" << std::endl;
	return 0;
}

std::string hexdump(std::string s)
{
	std::ostringstream os;
	os << std::hex;
	for (char c : s) {
		os << std::setw(2) << std::setfill('0') << (int) c << " ";
	}
	return os.str();
}

