#include "pch.h"
#include "BitArray.h"

BitArray::BitArray(int numArray)
{
    bitArray = (char*)malloc(numArray % 8 != 0 ? numArray / 8 + 1 : numArray / 8);
    Size = numArray;
}

BitArray::BitArray(char* input, unsigned int size)
{
    bitArray = input;
    Size = size;
}

void BitArray::set(unsigned int pos)
{
    *(char*)(bitArray + pos / 8) |= (0x80 >> pos % 8);
}

void BitArray::reset(unsigned int pos)
{
    *(char*)(bitArray + pos / 8) &= ~(0x80 >> pos % 8);
}

void BitArray::push(char Bit)
{
    if (Bit & 0x01)
        set(currIndex++);
    else
        reset(currIndex++);
}

char BitArray::next()
{
    return (*this)[nextIndex++];
}

char BitArray::operator[](unsigned int i)
{
    // TODO: insert return statement here
    return (*(unsigned char*)(bitArray + i / 8) >> (7 - i % 8))&0x01;
}

bool BitArray::operator==(BitArray& obj)
{
    for (int i = 0; i < Size; i++)
        if ((*this)[i] != obj[i])
            return false;
    return true;
}
