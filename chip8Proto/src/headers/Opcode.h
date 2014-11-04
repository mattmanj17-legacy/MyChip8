#ifndef OPCODE_H_
#define OPCODE_H_
#include <stdexcept>
#include <string>
#include <sstream>
#include <iostream>
class Opcode{
private:
	unsigned short code;
public:
	Opcode(unsigned short input)
	{
		code=input;
	}

	// first-> the most significant Byte
	// second-> the least significant Byte
	Opcode(unsigned char first, unsigned char second)
	{
		unsigned short firstAsShort = (unsigned short)first;
		unsigned short secondAsShort = (unsigned short)second;

		firstAsShort <<= 8;
		firstAsShort |= secondAsShort;

		code = firstAsShort;
	}

	// get nibble of opcode by index
	// 0 -> most significant nibble : 3 -> least significant nible
	unsigned char operator[](int index)
	{
		unsigned short nibble = code;

		int bits = 4*(3-index);
		unsigned short indexNibble = 0xf << bits;
		
		nibble &= indexNibble;

		//convert to char
		nibble >>= bits;
		return (unsigned char)nibble;
	}

	// Returns the opcode in one 2-Byte piece
	unsigned short getCode(){
		return code;
	}

	// Returns the least significant Byte
	unsigned char get8BitConstant(){
		unsigned short toreturn;
		__asm{
			mov ecx, this
			mov ax, [ecx]Opcode.code
			and ax, 00FFh
			mov toreturn, ax
		}
		return toreturn;
	}

	// Returns the least significant Byte and a half
	unsigned short get12BitConstant(){
		unsigned short toreturn;
		__asm{
			mov ecx, this
			mov ax, [ecx]Opcode.code
			and ax, 0FFFh
			mov toreturn, ax
		}
		return toreturn;
	}

	// Returns the hexidecimal representation of the opcode
	std::string toString(){
		std::stringstream out;
		out<< std::hex <<(int) this->operator[](0);
		out<<std::hex<<(int) this->operator[](1);
		out<<std::hex<<(int) this->operator[](2);
		out<<std::hex<<(int) this->operator[](3);
		std::string result;
		out>>result;
		return result;
	}
};
#endif