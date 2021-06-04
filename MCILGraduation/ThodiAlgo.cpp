#include "pch.h"
#include "ThodiAlgo.h"
#include <algorithm>
ThodiAlgo::ThodiAlgo(const cv::String& filename, int flags)
{
	_imagePixels = imread(filename, IMREAD_GRAYSCALE);
	if (_imagePixels.data == NULL)
		MessageBoxA(NULL, (LPCSTR)"This Image is not supported or invalid", (LPCSTR)"Error", MB_OK);
	else
	{
		imageSize = (_imagePixels.cols * _imagePixels.rows / 2);
		OverFlowMapM = std::vector<uchar>(imageSize, 0);
		High= std::vector<short>(imageSize, 0);
		Low = std::vector<uchar>(imageSize, 0);
		Locations = std::vector<uchar>(imageSize, 0);
		MessageBoxA(NULL, (LPCSTR)"Image Loaded Successfully !", (LPCSTR)"Success !", MB_OK);
	}
}

void ThodiAlgo::CalcHighPass()
{
	// Calculating High and Low values for Input Image
	for (int i = 0; i < imageSize; i++)
	{
		High.at(i)=((short)(_imagePixels.data[2 * i] - _imagePixels.data[2 * i + 1]));
		Low.at(i)=((_imagePixels.data[2 * i] + _imagePixels.data[2 * i + 1]) / 2);
	}
}


void ThodiAlgo::DetermineLocations()
{
	for (int i = 0; i < imageSize; i++)
		// Checking if Hw is in Rd range
		if (isExpandable(High.at(i),Low.at(i)))
			Locations.at(i) = EXPANDABLE;
		else if (isChangable(High.at(i),Low.at(i)))
			Locations.at(i) = CHANGABLE;
		else
			Locations.at(i) = NEITHER;
}

void ThodiAlgo::GetDelta()
{
	unsigned bits = 0;
	for (delta = 0; delta < 256; delta++)
	{
		for (int i = 0; i < High.size(); i++)
		{
			if (!(High.at(i) >= delta && High.at(i) <= delta - 1))
				continue;
			if (Locations.at(i) != EXPANDABLE)
				continue;
			Locations.at(i) = EXPANDABLE_IN_DELTA;
			bits++;
			//TODO: Payload Size+Compressed Size
			if (bits >= sizeof(Header)*8)
				break;
		}
	}
}

void ThodiAlgo::OutterHistogramShift()
{
	for (short i : High)
	{
		if (i > delta)
			i += delta + 1;
		else if (i < -delta - 1)
			i -= -delta - 1;
	}
}

void ThodiAlgo::BuildBitStream()
{
	BS.aInfo.header.Delta = delta;
	//TODO: Size Of coMPRESSED OVERFLOW MAP
	BS.aInfo.header.SizeOfCompressedOverFlowMap = 17;
	//TODO: Size of Payload
	BS.aInfo.header.SizeOfPayload = 32;
	BS.aInfo.overflowComp = (uchar*)"0101010010001110";
	BS.payload = (uchar*)"1111001011100001010111001100101";
	
	//Save LSBs of C \ Ee
	for (int i = 0; i < Locations.size(); i++)
	{
		if (!(Locations.at(i) == CHANGABLE || Locations.at(i) == EXPANDABLE))
			continue;
		LSBs.push_back((uchar)(High.at(i) & 0x0001));
	}

	BS.LSBs = LSBs.data();
}

void ThodiAlgo::EmbedBitStream()
{
	unsigned int bitsEmbedded=0;
	for (int i = 0; i < Locations.size(); i++)
	{
		
		if (Locations.at(i) == EXPANDABLE_IN_DELTA)
		{
			if (bitsEmbedded > 72&&bitsEmbedded<=72+
				BS.aInfo.header.SizeOfCompressedOverFlowMap)
			{
				//TODO:Read OverFlow Map Buffer and write it.
			}
			else if (bitsEmbedded >
				72 + BS.aInfo.header.SizeOfCompressedOverFlowMap &&
				bitsEmbedded <= 72 + BS.aInfo.header.SizeOfCompressedOverFlowMap + BS.aInfo.header.SizeOfPayload)
			{
				//TODO : READ Payload Buffer and write it
			}
			High.at(i) = ExpandBit(High.at(i), *(uchar*)(&BS + bitsEmbedded / 8) >> (7 - bitsEmbedded % 8));
		}
		else if (Locations.at(i) == EXPANDABLE || Locations.at(i) == CHANGABLE)
		{
			if (bitsEmbedded >= 72 && bitsEmbedded <= 72 + BS.aInfo.header.SizeOfCompressedOverFlowMap)
			{
				//TODO : Read OverflowMap Buffer
			}
			else if (bitsEmbedded >
				72 + BS.aInfo.header.SizeOfCompressedOverFlowMap &&
				bitsEmbedded <= 72 + BS.aInfo.header.SizeOfCompressedOverFlowMap + BS.aInfo.header.SizeOfPayload)
			{
				//TODO : READ Payload Buffer and write it
			}
			High.at(i) = ChangeBit(High.at(i), *(uchar*)(&BS + bitsEmbedded / 8) >> (7 - bitsEmbedded % 8));
		}
	}
}

bool ThodiAlgo::isInRdRange(short val, uchar low)
{
	if (abs(val) >= 0 && abs(val) <= std::min(2 * (256 - 1 - low), 2 * low + 1))
		return true;
	return false;
}
