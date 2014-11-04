#ifndef CALLSTACK_H_
#define CALLSTACK_H_
#include <stdexcept>
class CallStack{
private:

	unsigned short* stackSpace; // The array that will hold all the values pushed onto the stack
	
	int numItems; // How many items are pushed onto the stack
	
	int capacity; // The capacity of the "stackSpace" array

public:

	// CallStack constructor
	// @param: size-> The capacity of the stack
	CallStack(int size){
		numItems=0;
		capacity=size;
		stackSpace=new unsigned short[size];
	}

	// Returns the capacity of the array acting as the stack.
	int getCapacity(){
		return capacity;
	}

	// Pushes an address onto the stack.
	// @param: address-> the address to be pushed onto the stack
	void push(unsigned short address){
		stackSpace[numItems]=address;
		numItems++;
	}

	// Pops the top item off the stack.
	void pop(){
		numItems--;
	}

	// Returns the value currently on top of the stack.
	unsigned short top(){
		return stackSpace[numItems-1];
	}

	// Empties the stack of all values.
	void clear(){
		numItems=0;
	}
};
#endif