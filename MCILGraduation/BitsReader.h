#pragma once
template <typename T>
class BitsReader
{
private:
	T* buff;
public:
	BitsReader(T* input);
	
	unsigned char operator[](unsigned int pos);
};
