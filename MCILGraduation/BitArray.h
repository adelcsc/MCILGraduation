#pragma once
class BitArray
{
public:
	unsigned int Size;
	unsigned int currIndex=0;
	unsigned int nextIndex=0;
	char* bitArray;
public:
	BitArray(int numArray);
	BitArray(char* input,unsigned int size=0);
	void set(unsigned int pos);

	void reset(unsigned int pos);
	void push(char Bit);
	char next();
	void resetNext() { nextIndex = 0; }
	void resetGet() { currIndex = 0; }
	char operator[](unsigned int i);
	bool operator== (BitArray& obj);
	void* Data() { return bitArray; }
	unsigned int size() { return Size; }
	unsigned int sizeInBytes() { 
		return Size % 8 != 0 ? Size / 8 + 1 : Size/8; 
	}
};

