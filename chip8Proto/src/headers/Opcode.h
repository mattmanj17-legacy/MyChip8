#ifndef OPCODE_H_
#define OPCODE_H_
#include <stdexcept>
#include <string>
#include <sstream>
#include <iostream>
class Opcode{
private:

	unsigned short code; // The individual 2-Byte opcode

public:

	// Opcode constructor 
	// @param: input-> the 2-Byte value of the opcode
	Opcode(unsigned short input){
		code=input;
	}

	// Opcode constructor
	// @param: first-> the most significant Byte of the opcode
	// @param: second-> the least significant Byte of the opcode
	Opcode(unsigned char first, unsigned char second){
		unsigned short firstAsShort=(unsigned short)(first);
		unsigned short secondAsShort=(unsigned short)(second);
		firstAsShort<<=8;
		firstAsShort|=secondAsShort;
		code=firstAsShort;
	}

	// [] operator
	// @param: index-> index of the nible to return from the opcode
	//                 0 -> most significant nible : 3 -> least significant nible
	unsigned char operator[](int index){
		unsigned short codeafterAnd=code & ( ( 0xf << ( 4*(3-index) ) ) );
		unsigned char codeAfterShift= (unsigned char) ( ( codeafterAnd >> (4*(3-index) ) ) );
		return codeAfterShift;
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