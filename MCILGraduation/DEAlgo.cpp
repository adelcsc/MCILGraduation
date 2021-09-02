#include "pch.h"
#include "DEAlgo.h"
#include <algorithm>

DEAlgo::DEAlgo(const cv::String& filename,float bpp) : EEAlgo(filename,0,bpp)
{
	Init(_imagePixels);
}

DEAlgo::DEAlgo(Mat pixels,float bpp) : EEAlgo(pixels,bpp)
{
	Init(pixels);
}

void DEAlgo::Init(Mat _imagePixels)
{
	if (_imagePixels.data == NULL)
		MessageBoxA(NULL, (LPCSTR)"This Image is not supported or invalid", (LPCSTR)"Error", MB_OK);
	else
	{
		imageSize = (_imagePixels.cols * _imagePixels.rows) / 2;
		OverFlowMapM = new BitArray(imageSize); //Allocating a buff that's sufficient to hold all bits
		High = std::vector<short>(imageSize, 0);
		Low = std::vector<uchar>(imageSize, 0);
		//TODO: Generate Payload
		Locations = std::vector<uchar>(imageSize, 0);//Same here
	}
}

void DEAlgo::CalcHighPass()
{
	// Calculating High and Low values for Input Image
	for (int i = 0; i < imageSize; i++)
	{
		High.at(i)= (short)_imagePixels.data[2 * i] - (short)_imagePixels.data[2 * i + 1];
		Low.at(i)=floor(((float)_imagePixels.data[2 * i] + _imagePixels.data[2 * i + 1]) / 2);
	}
}


void DEAlgo::DetermineLocations()
{
	sizeOfLSBs = 0;
	for (int i = 0; i < Locations.size(); i++)
	{
		// Checking if Hw is in Rd range
		if (isExpandable(High.at(i), Low.at(i), &isInRdRange))
		{
			Locations.at(i) = EXPANDABLE;
			OverFlowMapM->set(i);
			sizeOfLSBs++;
			embeddingSize++;
			continue;
		}
		else if (isChangable(High.at(i), Low.at(i), &isInRdRange))
		{
			Locations.at(i) = CHANGABLE;
			sizeOfLSBs++;
		}
		else
			Locations.at(i) = NEITHER;
		OverFlowMapM->reset(i);
	}
}

void DEAlgo::GetDelta()
{
	unsigned int bits=0;
	embeddingSize -= BS.aInfo.header.SizeOfCompressedOverFlowMap + 72;
	Payload = GeneratePayload(embeddingSize * _bbp);
	BS.aInfo.header.SizeOfPayload = Payload->size();
	for (delta = 0; delta < 256; delta++)
	{
		for (int i = 0; i < High.size(); i++)
		{
			if (!(High.at(i) >= -(int)delta-1 && High.at(i) <= delta))
				continue;
			if (Locations.at(i) != EXPANDABLE)
				continue;
			Locations.at(i) = EXPANDABLE_IN_DELTA;
			sizeOfLSBs--;
			bits++;
		}
		if (bits >= 72 + BS.aInfo.header.SizeOfPayload + BS.aInfo.header.SizeOfCompressedOverFlowMap)
			break;
	}
}

void DEAlgo::OutterHistogramShift()
{
	for (int i = 0 ;i<Locations.size();i++)
	{
		if ((High.at(i) > delta)&&(Locations.at(i)==EXPANDABLE))
			High.at(i) += delta + 1;
		else if ((High.at(i) < -(short)delta - 1) && (Locations.at(i) == EXPANDABLE))
			High.at(i) -= (short)delta + 1;
	}
}

void DEAlgo::BuildBitStream()
{
	BS.aInfo.header.Delta = delta;
	//TODO: Size Of coMPRESSED OVERFLOW MAP
	BS.aInfo.header.SizeOfCompressedOverFlowMap = ComMap->size();
	BS.aInfo.header.SizeOfPayload = Payload->size();
	BS.aInfo.overflowComp = ComMap->Data();
	BS.payload = Payload->Data();
	LSBs = new BitArray(sizeOfLSBs);
	//Save LSBs of C \ Ee
	for (int i = 0; i < Locations.size(); i++)
	{
		if (!(Locations.at(i) == CHANGABLE || Locations.at(i) == EXPANDABLE))
			continue;
		LSBs->push((uchar)High.at(i));
	}

	BS.LSBs = LSBs->Data();
}

void DEAlgo::EmbedBitStream()
{
	BitArray headerBits((char*)&BS.aInfo.header,0),PayloadBits((char*)BS.payload,0),LSBsBits((char*)BS.LSBs,0);
	unsigned int bitsEmbedded=0;
	uchar location;
	for (int i = 0; i < Locations.size(); i++)
	{
		location = Locations.at(i);
		if (bitsEmbedded > 72 + BS.aInfo.header.SizeOfCompressedOverFlowMap + BS.aInfo.header.SizeOfPayload + sizeOfLSBs)
			break;
		switch (getCurrentRegion(bitsEmbedded))
		{
		case RANGE_HEADER:
			if (location == EXPANDABLE_IN_DELTA)
				High.at(i) = ExpandBit(High.at(i), headerBits[bitsEmbedded++]);
			else if (location == EXPANDABLE || location == CHANGABLE)
				High.at(i) = ChangeBit(High.at(i), headerBits[bitsEmbedded++]);
			break;
		case RANGE_HEADER_EMPTY_BYTES:
			if (location == EXPANDABLE_IN_DELTA)
				High.at(i) = ExpandBit(High.at(i), headerBits[bitsEmbedded++ + 3 * 8]);
			else if (location == EXPANDABLE || location == CHANGABLE)
				High.at(i) = ChangeBit(High.at(i), headerBits[bitsEmbedded++ + 3 * 8]);
			break;
		case RANGE_COMPRESSED_OV_MAP:
			if (location == EXPANDABLE_IN_DELTA)
				High.at(i) = ExpandBit(High.at(i), (*ComMap)[bitsEmbedded++ - 72]);
			else if (location == EXPANDABLE || location == CHANGABLE)
				High.at(i) = ChangeBit(High.at(i), (*ComMap)[bitsEmbedded++ - 72]);
			break;
		case RANGE_PAYLOAD:
			if (location == EXPANDABLE_IN_DELTA)
				High.at(i) = ExpandBit(High.at(i), PayloadBits[bitsEmbedded++ - 72 - (int)BS.aInfo.header.SizeOfCompressedOverFlowMap]);
			else if (location == EXPANDABLE || location == CHANGABLE)
				High.at(i) = ChangeBit(High.at(i), PayloadBits[bitsEmbedded++ - 72 - (int)BS.aInfo.header.SizeOfCompressedOverFlowMap]);
			break;
		case RANGE_LSBS:
			if (location == EXPANDABLE_IN_DELTA)
				High.at(i) = ExpandBit(High.at(i), LSBsBits[bitsEmbedded++ - 72 - (int)BS.aInfo.header.SizeOfCompressedOverFlowMap - (int)BS.aInfo.header.SizeOfPayload]);
			else if (location == EXPANDABLE || location == CHANGABLE)
				High.at(i) = ChangeBit(High.at(i), LSBsBits[bitsEmbedded++ - 72 - (int)BS.aInfo.header.SizeOfCompressedOverFlowMap - (int)BS.aInfo.header.SizeOfPayload]);
			break;
		}
	}
}

void DEAlgo::CompressOverFlowMap()
{
	std::string* output = new std::string;
	snappy::Compress((char*)OverFlowMapM->Data(), OverFlowMapM->sizeInBytes(), output);
	ComMap = new BitArray((char*)output->data(),output->size()*8);
	BS.aInfo.header.SizeOfCompressedOverFlowMap = ComMap->size();
}

void DEAlgo::CompileImage()
{
	for (int i = 0; i < imageSize; i++)
	{
		_imagePixels.data[2 * i] = Low.at(i) + floor(((float)High.at(i) + 1) / 2);
		_imagePixels.data[2 * i + 1] = Low.at(i) - floor((float)High.at(i) / 2);
	}
}

void DEAlgo::GetCLocations()
{
	sizeOfLSBs = 0;
	for (int i = 0; i < Locations.size(); i++)
	{
		if (isChangable(High.at(i), Low.at(i), &isInRdRange))
		{
			Locations.at(i) = CHANGABLE;
			sizeOfLSBs++;
		}
		else
			Locations.at(i) = NEITHER;
	}
}

void DEAlgo::ExtractBitStream()
{
	BitArray BitStreamBuilder((char*)&BS,0);

	//Fill the empty Space between char and int of the header variables which is 3*8

	for (size_t i = sizeof(int)*8+sizeof(char)*8; i < 3 * 8+ sizeof(int) * 8 + sizeof(char) * 8; i++)
		BitStreamBuilder.reset(i);

	//Extract Header
	int i = 0;
	for (i = 0; i < Locations.size(); i++)
	{
		if (Locations.at(i) == EXPANDABLE || Locations.at(i) == CHANGABLE)
		{
			if (BitStreamBuilder.currIndex >= sizeof(int) * 8 + sizeof(char) * 8 && BitStreamBuilder.currIndex < 64)
				BitStreamBuilder.currIndex += 3 * 8;
			if (BitStreamBuilder.currIndex >= sizeof(Header) * 8)
				break;
			BitStreamBuilder.push(High.at(i));
		}	
	}
	BitArray* ComMap = new BitArray(BS.aInfo.header.SizeOfCompressedOverFlowMap);
	BS.aInfo.overflowComp = ComMap->bitArray;

	//Extract Compressed Map
	for (; i < Locations.size(); i++)
	{
		if (Locations.at(i) == EXPANDABLE || Locations.at(i) == CHANGABLE)
		{
			if (ComMap->currIndex >= BS.aInfo.header.SizeOfCompressedOverFlowMap)
				break;
			ComMap->push(High.at(i));
		}
	}
	// Extract Payload
	BitArray *payload = new BitArray(BS.aInfo.header.SizeOfPayload);
	BS.payload = payload->bitArray;
	for (; i < Locations.size(); i++)
	{
		if (Locations.at(i) == EXPANDABLE || Locations.at(i) == CHANGABLE)
		{
			if (payload->currIndex >= BS.aInfo.header.SizeOfPayload)
				break;
			payload->push(High.at(i));
		}
	}
	BitArray *LSBs = new BitArray(Locations.size()); //TODO: the size is maxmimum
	BS.LSBs = LSBs->bitArray;
	//Extract LSBs
	for (; i < Locations.size(); i++)
	{
		if (Locations.at(i) == EXPANDABLE || Locations.at(i) == CHANGABLE)
		{
			LSBs->push(High.at(i));
		}
	}
}

void DEAlgo::DecompressOverFlowMap()
{
	std::string *decompressed = new std::string();
	snappy::Uncompress((char*)BS.aInfo.overflowComp, BS.aInfo.header.SizeOfCompressedOverFlowMap/8, decompressed);
	OverFlowMapM = new BitArray((char*)decompressed->data(), Locations.size());
}

void DEAlgo::IdentifyExpandedLocations()
{
	// Mark E locations
	for (int i = 0; i < Locations.size(); i++)
		if ((*OverFlowMapM)[i])
			Locations.at(i) = EXPANDABLE;

	// Get Ee Locations
	for (int i = 0; i < Locations.size(); i++)
		if (Locations.at(i) == EXPANDABLE)
			if (High.at(i) >= -2 * (short)BS.aInfo.header.Delta - 2 && High.at(i) <= 2 * BS.aInfo.header.Delta + 1)
			{
				Locations.at(i) = EXPANDABLE_IN_DELTA;
				sizeOfLSBs--;
			}
	
}

void DEAlgo::RestoreLSBs()
{
	BitArray LSBs = BitArray((char*)BS.LSBs, 0);
	LSBs.resetNext();
	for (int i = 0; i < Locations.size(); i++)
		if (Locations.at(i) == CHANGABLE || Locations.at(i) == EXPANDABLE)
			if (LSBs.next())
				High.at(i) |= 0x0001;
			else
				High.at(i) &= ~0x0001;
}

void DEAlgo::ReverseShift()
{
	for (int i = 0; i < Locations.size(); i++)
		if (Locations.at(i) == EXPANDABLE)
			if (High.at(i) > 2 * (short)BS.aInfo.header.Delta + 1)
				High.at(i) += -(short)BS.aInfo.header.Delta - 1;
			else if (High.at(i) < -2 * (short)BS.aInfo.header.Delta - 2)
				High.at(i) += BS.aInfo.header.Delta + 1;
}

void DEAlgo::RestoreExpanded()
{
	for (int i = 0; i < Locations.size(); i++)
	{
		if (Locations.at(i) == EXPANDABLE_IN_DELTA)
			High.at(i) >>= 1;
	}
}

bool DEAlgo::CompareHigh(std::vector<short> tHigh)
{
	for (int i = 0; i < High.size(); i++)
		if (High.at(i) != tHigh.at(i))
			return false;
	return true;
}

bool DEAlgo::isInRdRange(short val, uchar low)
{
	if (abs(val) >= 0 && abs(val) <= std::min(2 * (256 - 1 - low), 2 * low + 1))
		return true;
	return false;
}
