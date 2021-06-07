#pragma once
class BitArray
{
private:
	void* bitArray;
	unsigned int Size;
	unsigned int currIndex=0;
public:
	BitArray(int numArray);
	BitArray(void* input, unsigned int size=0);
	void set(unsigned int pos);

	void reset(unsigned int pos);
	void push(char Bit);
	char operator[](unsigned int i);
	void* Data() { return bitArray; }
	unsigned int size() { return Size; }
	unsigned int sizeInBytes() { 
		return Size % 8 != 0 ? Size / 8 + 1 : Size/8; 
	}
};

