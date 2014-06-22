#ifndef VCPU_H_
#define VCPU_H_
#include <stdexcept>
#include <string>
#include <random>
#include <time.h>
#include <thread>
#include "CallStack.h"
#include "ROM.h"
#include "Opcode.h"
#include <Windows.h>
#include <mmsystem.h>

unsigned char chip8_fontset[80] =
{ 
  0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
  0x20, 0x60, 0x20, 0x20, 0x70, // 1
  0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
  0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
  0x90, 0x90, 0xF0, 0x10, 0x10, // 4
  0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
  0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
  0xF0, 0x10, 0x20, 0x40, 0x40, // 7
  0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
  0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
  0xF0, 0x90, 0xF0, 0x90, 0x90, // A
  0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
  0xF0, 0x80, 0x80, 0x80, 0xF0, // C
  0xE0, 0x90, 0x90, 0x90, 0xE0, // D
  0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
  0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};
class VCPU{
private:
	friend class Debugger;

	unsigned char V[16]; // 16 8-bit registers
	
	unsigned char* Memory; // Memory of VCPU
	
	int memSize; // Size of the memory array
	
	CallStack* cstack; // The stack to allow returning from subroutines
	
	bool drawFlag; // Flag to be set when the screen needs to be re-drawn
	
	bool screen[64][32]; // A 2D array to hold values in the screen
	
	int delayTimer; // Timer, it's use isn't totally clear
	
	int soundTimer; // Timer for sound, when it switches to 0 a sound is supposed to play
	
	unsigned short indexRegister; // A register to hold an index in the memory array
	
	unsigned short programCounter; // The program counter, controls where we pull opcodes from in memory

	// Empties the key states register.
	void clearKeyStates(){
		/*for(int i=0;i<16;i++){
			keyStates[i]=false;
		}*/
		__asm{
			mov ebx, this
			mov ecx, 16
		loop1:
			mov [ebx]VCPU.keyStates, 0
			inc ebx
			loop loop1
		}
	}

	// Loads starting character set into memory, some ROMs use this 
	// standardized character set, so this is required at initialization.
	void loadCharacterSet(){
		for(int i=0; i<memSize;i++){
			Memory[i]=0x00;
		}

	/*	__asm{
			mov ebx, this
			mov ecx, [ebx]VCPU.memSize
		loop1:
			mov [ebx]VCPU.Memory, 0h
			inc ebx
			loop loop1
		}*/

		for(int i=0x050;i<0x0A0;i++){
			Memory[i]=chip8_fontset[i-0x050];
		}

	}

	// Loads ROM object into memory and sets program counter at start of ROM.
	// @param: data-> Selected ROM object, must have ROM data loaded into it
	void loadROM(ROM &data){
		for(int i=0x200; i<memSize;i++){
			Memory[i]=0x00;
		}
		for(int i=0x200;i<data.getData().size()+0x200;i++){
			Memory[i]=data.getData()[i-0x200];
		}
		programCounter=0x200;
	}

	// Clears the Screen array, does not redraw screen.
	void clearScreen(){
	/*	for(int i=0;i<64;i++){
			for(int j=0;j<32;j++){
				screen[i][j]=false;
			}
		}*/
		__asm{
			mov ebx, this
			mov ecx, 2048
		loop1:
			mov [ebx]VCPU.screen , 0
			add ebx, 1
			loop loop1
		}
	}

	// Sets the VCPU states back to initial values and loads new ROM into memory.
	// @param: data-> Selected ROM object, must have ROM data loaded into it
	void initNewRom(ROM &data){
		drawFlag=false;
		clearKeyStates();
		clearScreen();
		loadROM(data);
		delayTimer=0;
		soundTimer=0;
		for(int i=0;i<16;i++){
			V[i]=0x0;
		}
		cstack->clear();
	}
	

	// Exucutes single opcode, updating virtual machine hardware.
	// @param: instruction-> Opcode object, loaded from memory
	// **Read CHIP-8 specifications for descriptions of opcodes**
	void exucuteOpcode(Opcode &instruction){
		switch(instruction[0]){
		case(0x0):
			//note: the 0NNN opcode is treated as depreciated
			if(instruction.getCode()==0x00E0){
				//opcode 00E0: Clears the screen.
				drawFlag=true;
				clearScreen();
			}
			else if(instruction.getCode()==0x00EE){
				//opcode 00EE: Returns from a subroutine.
				programCounter = cstack->top();
				cstack->pop();
			}
			break;
		case(0x1):
			//opcode 1NNN: Jumps to address NNN.
			//programCounter=( instruction.get12BitConstant() );
			//programCounter-=2;//counteract the default advavce in exucuteOpcode()
			{
				unsigned short constant = instruction.get12BitConstant();
				__asm{
					mov ecx, this
						mov dx, constant
						sub dx, 2
						mov[ecx]VCPU.programCounter, dx
				}
			}
			break;
		case(0x2):
			//opcode 2NNN: Calls subroutine at NNN.
			cstack->push(programCounter);
			//programCounter=( instruction.get12BitConstant() );
			//programCounter-=2;//counteract the default advavce in exucuteOpcode()
		{
					 unsigned short constant = instruction.get12BitConstant();
					 __asm{
						 mov ecx, this
							 mov dx, constant
							 sub dx, 2
							 mov[ecx]VCPU.programCounter, dx
					 }
		}
			break;
		case(0x3):
			//opcode 3XNN: Skips the next instruction if VX equals NN.
			if(V[instruction[1]]==instruction.get8BitConstant()){
				/*programCounter+=2;*/
				__asm{
					mov ecx, this
					mov dx, [ecx]VCPU.programCounter
					add dx, 2
					mov [ecx]VCPU.programCounter, dx
				}
			}
			break;
		case(0x4):
			//opcode 4XNN: Skips the next instruction if VX doesn't equal NN.
			if(V[instruction[1]]!=instruction.get8BitConstant()){
				/*programCounter+=2;*/
				__asm{
					mov ecx, this
					mov dx, [ecx]VCPU.programCounter
					add dx, 2
					mov [ecx]VCPU.programCounter, dx
				}
			}
			break;
		case(0x5):
			if(instruction[3]==0x0){
				//opcode 5XY0: Skips the next instruction if VX equals VY.
				if(V[instruction[1]]==V[instruction[2]]){
					/*programCounter+=2;*/
					__asm{
						mov ecx, this
						mov dx, [ecx]VCPU.programCounter
						add dx, 2
						mov [ecx]VCPU.programCounter, dx
					}
				}
			}
			break;
		case(0x6):
			//opcode 6XNN: Sets VX to NN.
			V[instruction[1]]=instruction.get8BitConstant();
			break;
		case(0x7):
			//opcode 7XNN: Adds NN to VX. note:this opcode does not set the carry
			V[instruction[1]] += instruction.get8BitConstant();
	      /*unsigned short nnconst = instruction.get8bitconstant();
			int reg = instruction[1];
			__asm{

			}*/
			break;
		case(0x8):
			switch(instruction[3]){
			case(0x0):
				//opcode 8XY0: Sets VX to the value of VY.
				V[instruction[1]]=V[instruction[2]];
				break;
			case(0x1):
				//opcode 8XY1: Sets VX to VX or VY.
				V[instruction[1]] |= V[instruction[2]];
				break;
			case(0x2):
				//opcode 8XY2: Sets VX to VX and VY.
				V[instruction[1]] &= V[instruction[2]];
				break;
			case(0x3):
				//opcode 8XY3: Sets VX to VX xor VY.
				V[instruction[1]] ^= V[instruction[2]];
				break;
			case(0x4):
				//opcode 8XY4: Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't.
				if( V[instruction[2]] > ( 0xFF - V[instruction[1]] ) )
					V[0xF] = 1; //carry
				else 
					V[0xF] = 0;	
				V[instruction[1]]+=V[instruction[2]];
				break;
			case(0x5):
				//opcode 8XY5: VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
				if(V[instruction[2]]>V[instruction[1]]){
					V[0xF]=0x0;//borrow
				}
				else{
					V[0xF]=0x1;
				}
				V[instruction[1]]=V[instruction[1]]-V[instruction[2]];
				break;
			case(0x6):
				//opcode 8XY6: Shifts VX right by one. VF is set to the value of the least significant bit of VX before the shift.
				//On the original interpreter, the value of VY is shifted, and the result is stored into VX. On current implementations, Y is ignored.
				V[0xF]=(V[instruction[1]] & 0x1);
				V[instruction[1]]>>=1;
				break;
			case(0x7):
				//opcode 8XY7: Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
				if(V[instruction[2]]<V[instruction[1]])
					V[0xF]=0x0;//borrow
				else
					V[0xF]=0x1;
				V[instruction[1]]=V[instruction[2]]-V[instruction[1]];
				break;
			case(0xE):
				//opcode 8XYE: Shifts VX left by one. VF is set to the value of the most significant bit of VX before the shift.
				//On the original interpreter, the value of VY is shifted, and the result is stored into VX. On current implementations, Y is ignored.
				V[0xF]=(V[instruction[1]] >> 7 );
				V[instruction[1]]<<=1;
				break;
			}
			break;
		case(0x9):
			if(instruction[3]==0x0){
				//opcode 9XY0: Skips the next instruction if VX doesn't equal VY.
				if(V[instruction[1]]!=V[instruction[2]]){
					programCounter+=2;
				}
			}
			break;
		case(0xa):
			//opcode ANNN: Sets I to the address NNN.
			indexRegister=instruction.get12BitConstant();
			break;
		case(0xb):
			//opcode BNNN: Jumps to the address NNN plus V0.
			programCounter=V[0]+instruction.get8BitConstant();
			programCounter-=2;//couteract the default increment in exucuteOpcode()
			break;
		case(0xc):
			//opcode CXNN: Sets VX to a random number and NN.
			V[instruction[1]] = rand()%256 & instruction.get8BitConstant();
			break;
		case(0xd):
			//opcode DXYN: Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels.
			//Each row of 8 pixels is read as bit-coded (with the most significant bit of each byte displayed on the left) 
			//starting from memory location I; I value doesn't change after the execution of this instruction. 
			//Sprite pixels that are set flip the color of the corresponding screen pixel, while unset sprite pixels do nothing.
			//VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn, and to 0 if that doesn't happen.
			V[0xF] = 0;
			for (int yline = 0; yline < instruction[3]; yline++){
				for(int xline = 0; xline < 8; xline++){
					if((Memory[indexRegister + yline] & (0x80 >> xline)) != 0){
						if(screen[V[instruction[1]] + xline][V[instruction[2]] + yline] == true)
							V[0xF] = 1; 				
						screen[V[instruction[1]] + xline][V[instruction[2]] + yline] = !screen[V[instruction[1]] + xline][V[instruction[2]] + yline];
					}
				}
			}			
			drawFlag = true;



			break;
		case(0xe):
			if(instruction[2]==0x9 && instruction[3]==0xE){
				//opcode EX9E: Skips the next instruction if the key stored in VX is pressed.
				if(keyStates[V[instruction[1]]]==true){
					programCounter+=2;
				}
			}
			else if(instruction[2]==0xa && instruction[3]==0x1){
				//opcode EXA1: Skips the next instruction if the key stored in VX isn't pressed.
				if(keyStates[V[instruction[1]]]==false){
					programCounter+=2;
				}
			}
			break;
		case(0xf):
			if(instruction[2]==0x0 && instruction[3]==0x7){
				//opcode FX07:Sets VX to the value of the delay timer.
				V[instruction[1]]=(unsigned char)delayTimer;
			}
			if(instruction[2]==0x0 && instruction[3]==0xA){
				//opcode FX0A:A key press is awaited, and then stored in VX.
				bool inputFlag=false;
				//for(int i = 0; i<16; i++){
				//	if(keyStates[i]){
				//		V[instruction[1]]=(unsigned char)i;
				//		inputFlag=true;
				//		break;
				//	}
				//}
				//if(!inputFlag){
				//	programCounter-=2;//set the program counter to repeat this opcod until input is receved
				//}
				unsigned char saved;
				__asm{
					mov ecx, 16
					mov ebx, this
					mov edx, 0
					mov eax, 0
				forloop:
					cmp al, [ebx + edx]VCPU.keyStates
					jne successcase
					inc edx
					loop forloop

					mov ecx, this
					mov dx, [ecx]VCPU.programCounter
					add dx, 2
					mov [ecx]VCPU.programCounter, dx
					jmp escape

				successcase:
					mov saved, dl

				escape:
				}
				V[instruction[1]] = saved;
			}
			if(instruction[2]==0x1 && instruction[3]==0x5){
				//opcode FX15:Sets the delay timer to VX.
				delayTimer=V[instruction[1]];
			}
			if(instruction[2]==0x1 && instruction[3]==0x8){
				//opcode FX18:Sets the sound timer to VX.
				soundTimer=V[instruction[1]];
			}
			if(instruction[2]==0x1 && instruction[3]==0xE){
				//opcode FX1E:Adds VX to I.
				//VF is set to 1 when range overflow (I+VX>0xFFF), and 0 when there isn't. 
				//This is undocumented feature of the Chip-8, but is implemented for compleatness
				if( indexRegister+V[instruction[1]] >0xFFF){
					V[0xf]=0x1;
				}
				else{
					V[0xf]=0x0;
				}
				indexRegister+=V[instruction[1]];
			}
			if(instruction[2]==0x2 && instruction[3]==0x9){
				//opcode FX29:Sets I to the location of the sprite for the character in VX. 
				//Characters 0-F (in hexadecimal) are represented by a 4x5 font.
				indexRegister=  (5 * V[instruction[1]]) + 0x050;
			}
			if(instruction[2]==0x3 && instruction[3]==0x3){
				//opcode FX33:Stores the Binary-coded decimal representation of VX, 
				//with the most significant of three digits at the address in I, 
				//the middle digit at I plus 1, and the least significant digit at I plus 2. 
				//In other words, take the decimal representation of VX, place the hundreds digit in memory at location in I,
				//the tens digit at location I+1, and the ones digit at location I+2.
			/*	Memory[indexRegister]     =  V[(instruction[1])] / 100;
				Memory[indexRegister + 1] = (V[(instruction[1])] / 10) % 10;
				Memory[indexRegister + 2] = (V[(instruction[1])] % 100) % 10;*/
				{
					unsigned char instruct1 = instruction[1];
					__asm{
						mov edi, this
						movzx ebx, instruct1
						add edi, ebx
						mov al, [edi]VCPU.V
						mov instruct1, al
						sub edi, ebx
						mov edx, [edi]VCPU.Memory
						movzx eax, [edi]VCPU.indexRegister
						add edx, eax
						mov edi, edx

						movzx ax, instruct1
						mov bl, 100
						div bl
						mov [edi], al
						inc edi

						movzx ax, instruct1
						mov bl, 10
						div bl
						mov ah, 0
						div bl
						mov [edi], ah
						inc edi

						movzx ax, instruct1
						mov bl, 100
						div bl
						mov al, ah
						mov ah, 0
						mov bl, 10
						div bl
						mov [edi], ah
					}
				}
			}
			if(instruction[2]==0x5 && instruction[3]==0x5){
				//opcode FX55:Stores V0 to VX in memory starting at address I.
				//On the original interpreter, when the operation is done, I=I+X+1.
				for(int i=0;i<=instruction[1];i++){
					Memory[indexRegister+i]=V[i];
				}
				indexRegister+=instruction[1];
				indexRegister++;
			}
			if(instruction[2]==0x6 && instruction[3]==0x5){
				//opcode FX65:Fills V0 to VX with values from memory starting at address I.
				//On the original interpreter, when the operation is done, I=I+X+1.
				for(int i=0;i<=instruction[1];i++){
					V[i]=Memory[indexRegister+i];
				}
				indexRegister+=instruction[1];
				indexRegister++;
			}
			break;
		}
	}

	// Default VCPU constructor, sad and forgotten here at the end of
	// the private section, never to be called. I'm not even sure it 
	// can be called from in here....
	VCPU(){}

	

public:
	
	bool updateCounters(){
		bool ret = false;
		if(soundTimer > 0){
			if(soundTimer == 1){
				ret = true;
			}
			soundTimer--;
		}
		if(delayTimer > 0){
			delayTimer--;
		}
		return ret;
	}
	// VCPU constructor, initializes things
	// @param: stackSize-> Size of callstack to be used by the VCPU, not sure
	//                      but 16 should probably do, maybe bigger, I'm sure
	//                      size doesn't matter.
	// @param: data-> Selected ROM object, must have ROM data loaded into it
	VCPU(int stackSize, ROM &data){
		std::srand(time(NULL));
		Memory=new unsigned char[0x1000];
		memSize=0x1000;
		cstack=new CallStack(stackSize);
		loadCharacterSet();
		initNewRom(data);
	}

	// Returns a pointer to the VCPU's memory array.
	unsigned char* memDump(){
		return Memory;
	}

	// Executes a single opcode from memory and does timer stuff.
	void runCycle(){
		Opcode currentInstruction( Memory[programCounter] , Memory[programCounter+1] );
		exucuteOpcode(currentInstruction);
		
		if(delayTimer>0){
			//delayTimer--;
		}
		if(soundTimer>0){
			//soundTimer--;
		}
		programCounter+=2;
	}

	// Returns the value on a single bit in the screen, on or off.
	// @param: x-> The X coordinate of the bit to check
	// @param: y-> The Y coordinate of the bit to check
	bool getScreen(int x,int y){
		return screen[x][y];
	}

	// Returns the value of the draw flag.
	bool getDrawFlag(){
		return drawFlag;
	}

	// Sets the value of the draw flag.
	// @param: flag-> Value to set the draw flag to
	void setDrawFlag(bool flag){
		drawFlag=flag;
	}

	bool keyStates[16]; // An array for the 16 hex keys
};
#endif