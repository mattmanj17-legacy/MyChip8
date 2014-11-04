#ifndef ROM_H_
#define ROM_H_
#include <fstream>
#include <string>
#include <vector>

class ROM
{
private:
	std::vector<unsigned char> data;
	std::fstream source;
public:
	ROM(std::string filename)
	{
		source.open(filename, std::ios_base::in | std::ios::binary);
		
		char currentChar;
		while(source.get(currentChar))
			data.push_back(currentChar);

		source.close();
	}

	const std::vector<unsigned char> &getData()
	{
		return data;
	}
};
#endif