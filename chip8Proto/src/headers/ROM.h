#ifndef ROM_H_
#define ROM_H_
#include <fstream>
#include <string>
#include <vector>

class ROM
{
private:
	std::vector<unsigned char> data;
public:
	ROM(std::string filename)
	{
		std::fstream source;
		source.open(filename, std::ios_base::in | std::ios::binary);
		
		char currentChar;
		while(source.get(currentChar))
			data.push_back(currentChar);
	}

	const std::vector<unsigned char> &getData()
	{
		return data;
	}
};
#endif