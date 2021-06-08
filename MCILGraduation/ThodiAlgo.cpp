#include "pch.h"
#include "ThodiAlgo.h"
#include <algorithm>
BitArray* ThodiAlgo::GeneratePayload()
{
	std::srand(std::time(nullptr));
	BitArray* payload = new BitArray(((float)std::rand() / RAND_MAX) * 10000);
	for (int i = 0; i < payload->size(); i++)
		if ((float)std::rand() / RAND_MAX < 0.5)
			payload->set(i);
		else
			payload->reset(i);
	return payload;
}
ThodiAlgo::ThodiAlgo(const cv::String& filename, int flags)
{
	_imagePixels = imread(filename, IMREAD_GRAYSCALE);
	Init(_imagePixels);
}

ThodiAlgo::ThodiAlgo(Mat pixels)
{
	_imagePixels = pixels;
	Init(pixels);
}

void ThodiAlgo::Init(Mat _imagePixels)
{
	if (_imagePixels.data == NULL)
		MessageBoxA(NULL, (LPCSTR)"This Image is not supported or invalid", (LPCSTR)"Error", MB_OK);
	else
	{
		imageSize = (_imagePixels.cols * _imagePixels.rows / 2);
		OverFlowMapM = new BitArray(imageSize); //Allocating a buff that's sufficient to hold all bits
		High = std::vector<short>(imageSize, 0);
		Low = std::vector<uchar>(imageSize, 0);
		Payload = GeneratePayload();
		Locations = std::vector<uchar>(imageSize, 0);//Same here
		MessageBoxA(NULL, (LPCSTR)"Image Loaded Successfully !", (LPCSTR)"Success !", MB_OK);
	}
}

void ThodiAlgo::CalcHighPass()
{
	// Calculating High and Low values for Input Image
	for (int i = 0; i < imageSize; i++)
	{
		High.at(i)= (short)_imagePixels.data[2 * i] - (short)_imagePixels.data[2 * i + 1];
		Low.at(i)=floor(((float)_imagePixels.data[2 * i] + _imagePixels.data[2 * i + 1]) / 2);
	}
}


void ThodiAlgo::DetermineLocations()
{
	sizeOfLSBs = 0;
	for (int i = 0; i < imageSize; i++)
	{
		// Checking if Hw is in Rd range
		if (isExpandable(High.at(i), Low.at(i)))
		{
			Locations.at(i) = EXPANDABLE;
			OverFlowMapM->set(i);
			sizeOfLSBs++;
			continue;
		}
		else if (isChangable(High.at(i), Low.at(i)))
		{
			Locations.at(i) = CHANGABLE;
			sizeOfLSBs++;
		}
		else
			Locations.at(i) = NEITHER;
		OverFlowMapM->reset(i);
	}
}

void ThodiAlgo::GetDelta()
{
	unsigned int bits = 0;
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
		//TODO: Payload Size+Compressed Size
		if (bits >= 72 + BS.aInfo.header.SizeOfPayload + BS.aInfo.header.SizeOfCompressedOverFlowMap)
			break;
	}
	
}

void ThodiAlgo::OutterHistogramShift()
{
	for (int i = 0 ;i<Locations.size();i++)
	{
		if ((High.at(i) > delta)&&(Locations.at(i)==EXPANDABLE))
			High.at(i) += delta + 1;
		else if ((High.at(i) < -(short)delta - 1) && (Locations.at(i) == EXPANDABLE))
			High.at(i) -= (short)delta + 1;
	}
}

void ThodiAlgo::BuildBitStream()
{
	BS.aInfo.header.Delta = delta;
	//TODO: Size Of coMPRESSED OVERFLOW MAP
	BS.aInfo.header.SizeOfCompressedOverFlowMap = ComMap->size();
	//TODO: Size of Payload
	BS.aInfo.header.SizeOfPayload = Payload->size();
	BS.aInfo.overflowComp = ComMap->Data();
	BS.payload = Payload->Data();
	
	LSBs = new BitArray(sizeOfLSBs);
	if (BS.aInfo.header.SizeOfCompressedOverFlowMap + BS.aInfo.header.SizeOfPayload + sizeOfLSBs + 72 > High.size())
	{
		MessageBoxA(NULL, (LPCSTR)"Size of Information is too large !", (LPCSTR)"Error", MB_OK);
		exit(0);
	}
	//Save LSBs of C \ Ee
	for (int i = 0; i < Locations.size(); i++)
	{
		if (!(Locations.at(i) == CHANGABLE || Locations.at(i) == EXPANDABLE))
			continue;
		LSBs->push((uchar)High.at(i));
	}

	BS.LSBs = LSBs->Data();
}

void ThodiAlgo::EmbedBitStream()
{
	BitArray headerBits((char*)&BS.aInfo.header,0),PayloadBits((char*)BS.payload,0),LSBsBits((char*)BS.LSBs,0);
	//char hi = (*NikTizomek)[0];
	unsigned int bitsEmbedded=0;
	for (int i = 0; i < Locations.size(); i++)
	{
		
		if (Locations.at(i) == EXPANDABLE_IN_DELTA)
		{
			if (bitsEmbedded >= 72 && bitsEmbedded < 72 +
				BS.aInfo.header.SizeOfCompressedOverFlowMap)
				High.at(i) = ExpandBit(High.at(i), (*ComMap)[bitsEmbedded - 72]);
			else if (bitsEmbedded >=
				72 + BS.aInfo.header.SizeOfCompressedOverFlowMap &&
				bitsEmbedded < 72 + BS.aInfo.header.SizeOfCompressedOverFlowMap + BS.aInfo.header.SizeOfPayload)
				High.at(i) = ExpandBit(High.at(i), PayloadBits[bitsEmbedded - 72 - (int)BS.aInfo.header.SizeOfCompressedOverFlowMap]);
			else if (bitsEmbedded >= 72 + BS.aInfo.header.SizeOfCompressedOverFlowMap + BS.aInfo.header.SizeOfPayload)
				High.at(i) = ExpandBit(High.at(i), LSBsBits[bitsEmbedded - 72 - (int)BS.aInfo.header.SizeOfCompressedOverFlowMap - (int)BS.aInfo.header.SizeOfPayload]);
			else if (bitsEmbedded >= sizeof(int) * 8 + sizeof(char) * 8 && bitsEmbedded < 72)
				High.at(i) = ExpandBit(High.at(i), headerBits[bitsEmbedded+3*8]);
			else
				High.at(i) = ExpandBit(High.at(i), headerBits[bitsEmbedded]);
			bitsEmbedded++;
		}
		else if (Locations.at(i) == EXPANDABLE || Locations.at(i) == CHANGABLE)
		{
			if (bitsEmbedded >= 72 && bitsEmbedded < 72 + BS.aInfo.header.SizeOfCompressedOverFlowMap)
				High.at(i) = ChangeBit(High.at(i), (*ComMap)[bitsEmbedded - 72]);
			else if (bitsEmbedded >=
				72 + BS.aInfo.header.SizeOfCompressedOverFlowMap &&
				bitsEmbedded < 72 + BS.aInfo.header.SizeOfCompressedOverFlowMap + BS.aInfo.header.SizeOfPayload)
				High.at(i) = ChangeBit(High.at(i), PayloadBits[bitsEmbedded - 72 - (int)BS.aInfo.header.SizeOfCompressedOverFlowMap]);
			else if (bitsEmbedded >= 72 + BS.aInfo.header.SizeOfCompressedOverFlowMap + BS.aInfo.header.SizeOfPayload)
				High.at(i) = ChangeBit(High.at(i), LSBsBits[bitsEmbedded - 72 - (int)BS.aInfo.header.SizeOfCompressedOverFlowMap - (int)BS.aInfo.header.SizeOfPayload]);
			else if (bitsEmbedded >= sizeof(int) * 8 + sizeof(char) * 8 && bitsEmbedded < 72)
				High.at(i) = ChangeBit(High.at(i), headerBits[bitsEmbedded + 3 * 8]);
			else
				High.at(i) = ChangeBit(High.at(i), headerBits[bitsEmbedded]);
			bitsEmbedded++;
		}
	}
}

void ThodiAlgo::CompressOverFlowMap()
{
	std::string* output = new std::string;
	snappy::Compress((char*)OverFlowMapM->Data(), OverFlowMapM->sizeInBytes(), output);
	ComMap = new BitArray((char*)output->data(),output->size()*8);
}

void ThodiAlgo::CompileImage()
{
	// Gets new Pixel image based on High and low values
	for (int i = 0; i < imageSize; i++)
	{
		_imagePixels.data[2 * i] = Low.at(i) + floor(((float)High.at(i) + 1) / 2);
		_imagePixels.data[2 * i + 1] = Low.at(i) - floor((float)High.at(i) / 2);
	}
}

void ThodiAlgo::GetCLocations()
{
	DetermineLocations(); // an EXPANDABLE value is also CHANGABLE
}

void ThodiAlgo::ExtractBitStream()
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
	BS.aInfo.overflowComp = new BitArray(BS.aInfo.header.SizeOfCompressedOverFlowMap);

	//Extract Compressed Map
	for (; i < Locations.size(); i++)
	{
		if (Locations.at(i) == EXPANDABLE || Locations.at(i) == CHANGABLE)
		{
			if ((*(BitArray*)BS.aInfo.overflowComp).currIndex >= BS.aInfo.header.SizeOfCompressedOverFlowMap)
				break;
			(*(BitArray*)BS.aInfo.overflowComp).push(High.at(i));
		}
	}

	BS.payload = new BitArray(BS.aInfo.header.SizeOfPayload);
	// Extract Payload
	for (; i < Locations.size(); i++)
	{
		if (Locations.at(i) == EXPANDABLE || Locations.at(i) == CHANGABLE)
		{
			if ((*(BitArray*)BS.payload).currIndex >= BS.aInfo.header.SizeOfPayload)
				break;
			(*(BitArray*)BS.payload).push(High.at(i));
		}
	}

	BS.LSBs = new BitArray(Locations.size() - (72 + BS.aInfo.header.SizeOfCompressedOverFlowMap + BS.aInfo.header.SizeOfPayload));

	//Extract LSBs
	for (; i < Locations.size(); i++)
	{
		if (Locations.at(i) == EXPANDABLE || Locations.at(i) == CHANGABLE)
		{
			(*(BitArray*)BS.LSBs).push(High.at(i));
		}
	}
}

bool ThodiAlgo::isInRdRange(short val, uchar low)
{
	if (abs(val) >= 0 && abs(val) <= std::min(2 * (256 - 1 - low), 2 * low + 1))
		return true;
	return false;
}
