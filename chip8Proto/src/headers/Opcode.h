#ifndef OPCODE_H_
#define OPCODE_H_
#include <string>
#include <sstream>

typedef unsigned short word;
typedef unsigned char byte;

class Opcode
{
private:
	unsigned short code;
public:
	Opcode(word input)
	{
		code=input;
	}

	// first-> the most significant Byte
	// second-> the least significant Byte
	Opcode(byte first, byte second)
	{
		/*word firstAsShort = (word)first;
		word secondAsShort = (word)second;

		firstAsShort <<= 8;
		firstAsShort |= secondAsShort;

		code = firstAsShort;*/
		__asm
		{
			mov al, first
			shl ax, 8
			mov al, second
			mov ecx, this
			mov [ecx]Opcode.code, ax
		}
	}

	// get nibble of opcode by index
	// 0 -> most significant nibble : 3 -> least significant nible
	byte operator[](int index)
	{
		/*
			//example: code = 0x4H3A, index = 2
			word nibble = code;
			int bits = 4*(3-index);
			word indexNibble = 0xf << bits;
			//indexNibble: 0000000011110000
			//or in hex : 0x0F00
		
			//   0x43HA
			// & 0x00F0
			//   ------
			// = 0x00H0
			nibble &= indexNibble;

			//convert to char
			// 0x00H0 -> 0x0H
			nibble >>= bits;
			return (byte)nibble;
		*/
		word nibble = code;
		__asm
		{
			//int bits = 4*(3-index);
			//eax = 4*(3-index);
			mov eax, 3
			sub eax, index
			mov ebx, 4
			mul ebx

			// int nibble = code
			//ebx = code;
			mov ebx, 0
			mov bx, nibble

			//int indexNibble = 0xf << edx
			//edx = 0xf << eax;
			mov edx, 000Fh
			mov cl, al
			shl edx, cl
			
			//nibble &= indexNibble;
			//ebx &= edx;
			and ebx, edx

			//nibble >>= bits;
			//ebx >>= edx;
			shr ebx, cl

			//return (byte)nibble;
			//nibble = bx
			mov nibble, bx
		}
		return nibble;
	}

	word getCode()
	{
		return code;
	}

	// if opcode = XXNN , returns NN
	byte get8BitConstant()
	{
		byte toreturn;
		__asm
		{
			mov ecx, this
			mov ax, [ecx]Opcode.code
			and al, 0FFh
			mov toreturn, al
		}
		return toreturn;
	}

	// if opcode = XNNN , returns NNN
	word get12BitConstant()
	{
		word toreturn;
		__asm
		{
			mov ecx, this
			mov ax, [ecx]Opcode.code
			and ax, 0FFFh
			mov toreturn, ax
		}
		return toreturn;
	}

	// returns opcode in hex
	std::string toString()
	{
		std::stringstream out;

		out << std::hex << (int) this->operator[](0);
		out << std::hex << (int) this->operator[](1);
		out << std::hex << (int) this->operator[](2);
		out << std::hex << (int) this->operator[](3);
		
		std::string result;
		out>>result;

		return result;
	}
};
#endif