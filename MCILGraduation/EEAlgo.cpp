#include "pch.h"
#include "EEAlgo.h"
BitArray* EEAlgo::GeneratePayload()
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

uchar EEAlgo::getCurrentRegion(unsigned int bitsEmbedded)
{

	if (bitsEmbedded >= sizeof(int)*8+8 && bitsEmbedded < 72)
		return RANGE_HEADER_EMPTY_BYTES;
	if (bitsEmbedded >= 72 && bitsEmbedded < 72 + BS.aInfo.header.SizeOfCompressedOverFlowMap)
		return RANGE_COMPRESSED_OV_MAP;
	if (bitsEmbedded >= 72 + BS.aInfo.header.SizeOfCompressedOverFlowMap &&
		bitsEmbedded < 72 + BS.aInfo.header.SizeOfCompressedOverFlowMap + BS.aInfo.header.SizeOfPayload)
		return RANGE_PAYLOAD;
	if (bitsEmbedded >= 72 + BS.aInfo.header.SizeOfCompressedOverFlowMap + BS.aInfo.header.SizeOfPayload)
		return RANGE_LSBS;
	return RANGE_HEADER;
}

EEAlgo::EEAlgo(const cv::String& filename, int flags)
{
	_imagePixels = imread(filename, IMREAD_GRAYSCALE);
	_OriginalPixels = _imagePixels.clone();
}

EEAlgo::EEAlgo(Mat pixels)
{
	_imagePixels = pixels;
	_OriginalPixels = _imagePixels.clone();
}
bool EEAlgo::CompareBitStreams(BitStream inBS)
{
	//Comparing Header
	BitArray aHeader((char*)&inBS, sizeof(Header) * 8), bHeader((char*)&BS, sizeof(Header) * 8);
	if (!(aHeader == bHeader))
		return false;

	//Comparing Compressed OverFlowMap
	BitArray aOverFlowMap((char*)inBS.aInfo.overflowComp, inBS.aInfo.header.SizeOfCompressedOverFlowMap), bOverFlowMap((char*)BS.aInfo.overflowComp, BS.aInfo.header.SizeOfCompressedOverFlowMap);
	if (!(aOverFlowMap == bOverFlowMap))
		return false;

	//Compare Payload
	BitArray aPayload((char*)inBS.payload, inBS.aInfo.header.SizeOfPayload), bPayload((char*)BS.payload, BS.aInfo.header.SizeOfPayload);
	if (!(aPayload == bPayload))
		return false;

	sizeOfLSBs = _imagePixels.rows * _imagePixels.cols - 72 - BS.aInfo.header.SizeOfCompressedOverFlowMap - BS.aInfo.header.SizeOfPayload;
	//Compare LSBs
	BitArray aLSBs((char*)inBS.LSBs, sizeOfLSBs), bLSBs((char*)BS.LSBs, sizeOfLSBs);
	if (!(aLSBs == bLSBs))
		return false;

	return true;
}


bool EEAlgo::isEqualTo(Mat imagePixels)
{
	for (int i = 0; i < _imagePixels.rows*_imagePixels.cols; i++)
		if (_imagePixels.data[i] != imagePixels.data[i])
			return false;
	return true;
}