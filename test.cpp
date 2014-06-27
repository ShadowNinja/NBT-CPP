
#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <cassert>
#include <chrono>

#include "nbt.h"
#include "compression.h"

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

	NBT::Tag root((NBT::UByte *) data.c_str());
	std::cout << "Original: " << hexdump(data) << std::endl;
	std::cout << "Written:  " << hexdump(root.write()) << std::endl;

	NBT::ustring udata = NBT::ustring((const unsigned char *) data.data(), data.size());
	NBT::ustring comp = NBT::compress(udata, 1);
	std::cout << "Compressed: " << hexdump(std::string((const char *) comp.data(), comp.size())) << std::endl;
	NBT::ustring decomp = NBT::decompress(comp);
	assert(decomp == udata);

	assert((NBT::Byte) root["A"] == 0x40);
	assert((NBT::Int) root["test"] == 0x12345678);
	NBT::String foobar = root["foobar"];
	assert(std::string(foobar.value, foobar.size) == "<3 C++ 11");

	//assert(root.write() == data); // Data is unordered

	root["foo"] = NBT::TagType::List;

	assert(root["foo"].type == NBT::TagType::List);

	root["foo"][0] = NBT::TagType::List;
	root["foo"] += NBT::TagType::List;

	root["foo"][1][0] = -123L;
	assert((NBT::Long) root["foo"][1][0] == -123L);

	root = NBT::TagType::Compound;  // Reset
	root["A"] = (NBT::Byte) 0x40;
	root["test"] = (NBT::Int) 0x12345678;
	root["foobar"] = std::string("<3 C++ 11");
	std::cout << "Manual:   " << hexdump(root.write()) << std::endl;
	std::cout << "Manual dump: " << root.dump() << std::endl;

	std::cout << "Testing reading performance..." << std::endl;
	// Generate big list
	root = NBT::TagType::IntArray;
	// In reverse order to minimize reallocations
	for (int32_t i = 999; i >= 0; i--) {
		root.insert(i, (const NBT::Int) (i - 10));
	}
	data = root.write();

	using namespace std::chrono;
	high_resolution_clock::time_point start = high_resolution_clock::now();
	for (uint32_t i = 0; i < 1000; i++) {
		root.read((NBT::UByte*) data.c_str());
	}
	std::cout << "Completed 1,000 reads of 1,000 integers in " <<
			duration_cast<duration<double>>(high_resolution_clock::now() - start).count()
			<< " seconds." << std::endl;

	std::cout << "Testing writing performance..." << std::endl;
	start = high_resolution_clock::now();
	for (uint32_t i = 0; i < 1000; i++) {
		root.write();
	}
	std::cout << "Completed 1,000 writes of 1,000 integers in " <<
			duration_cast<duration<double>>(high_resolution_clock::now() - start).count()
			<< " seconds." << std::endl;

	std::cout << "Integer array write: " << hexdump(root.write()).substr(0, 100) << "..." << std::endl;
	std::cout << "Integer array dump: "  << root.dump().substr(0, 100) << "..." << std::endl;

	root = NBT::Tag(NBT::TagType::List, 1000);
	for (int32_t i = 0; i < 1000; i++) {
		root.insert(i, NBT::Tag((float) (i / 2)));
	}
	data = root.write();
	start = high_resolution_clock::now();
	for (uint32_t i = 0; i < 1000; i++) {
		root.read((NBT::UByte *) data.c_str());
	}
	std::cout << "Completed 1,000 reads of 1,000 floats in " <<
			duration_cast<duration<double>>(high_resolution_clock::now() - start).count()
			<< " seconds." << std::endl;

	std::cout << "Testing writing performance..." << std::endl;
	start = high_resolution_clock::now();
	for (uint32_t i = 0; i < 1000; i++) {
		root.write();
	}
	std::cout << "Completed 1,000 writes of 1,000 floats in " <<
			duration_cast<duration<double>>(high_resolution_clock::now() - start).count()
			<< " seconds." << std::endl;

	std::cout << "Success!" << std::endl;
	return 0;
}

std::string hexdump(std::string s)
{
	std::ostringstream os;
	os << std::hex << std::uppercase;
	for (char c : s) {
		os << std::setw(2) << std::setfill('0') << ((int16_t) c & 0xFF) << " ";
	}
	return os.str();
}

