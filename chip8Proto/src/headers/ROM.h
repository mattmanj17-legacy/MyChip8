#ifndef ROM_H_
#define ROM_H_
#include <stdexcept>
#include <fstream>
#include <string>
#include <vector>
#include <iostream>
class ROM{
private:

	std::vector<unsigned char> data; // All of the actual data of the ROM

	std::fstream source; // A stream to the source file of the ROM

	// Copies the data from the file to the "data" vector
	void pullData(){
		char temp;
		unsigned char currentChar;
		while(source.get(temp)){
			currentChar=(unsigned char)temp;
			data.push_back(temp);
		}
	}

	// A constructor???
	ROM(const ROM &other){}

public:

	// The ROM constructor
	// @param: filename-> the filename of the file that the ROM is going to be loading its data from
	ROM(std::string filename){
		source.open(filename, std::ios_base::in | std::ios::binary);
		pullData();
	}

	// Returns the data of the ROM as a vector of 1-Byte unsigned characters
	const std::vector<unsigned char> &getData(){
		return data;
	}
};
#endif