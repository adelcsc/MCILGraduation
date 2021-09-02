#pragma once
#include <ctime>
#include <vector>
#include "BitArray.h"
#include <opencv2/core/cvstd.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <fstream>
#include <snappy.h>
using namespace cv;
class EEAlgo
{
protected:
	Mat _imagePixels;
	Mat _OriginalPixels;
	BitArray* OverFlowMapM;
	BitArray* Payload;
	BitArray* ComMap;
	BitArray* LSBs;
	uchar delta;
	size_t sizeOfLSBs;
	float _bbp=0;
	unsigned int embeddingSize = 0;
	enum { NEITHER, EXPANDABLE, CHANGABLE, EXPANDABLE_IN_DELTA };
	short ExpandBit(short high, uchar Bit) { return high << 1 | (short)(Bit & 0x01); }
	short ChangeBit(short high, uchar Bit) { return (high >> 1) << 1 | (short)(Bit & 0x01); }
	bool isExpandable(short high, uchar low, bool (*isInRange)(short, uchar)) { return isInRange(high << 1 | (short)0x0001, low); }
	bool isChangable(short high, uchar low, bool (*isInRange)(short, uchar)) { 
		return isInRange((short)(2 * floor((float)high / 2)) | (short)0x0001, low); }
	BitArray* GeneratePayload(unsigned int sizeInBits);
	uchar getCurrentRegion(unsigned int bitsEmbedded);
	enum { RANGE_HEADER, RANGE_COMPRESSED_OV_MAP, RANGE_PAYLOAD, RANGE_LSBS , RANGE_HEADER_EMPTY_BYTES};
	struct Header {
		unsigned int SizeOfCompressedOverFlowMap;
		uchar Delta;
		unsigned int SizeOfPayload;
	};
	struct AuxilaryInformation
	{
		struct Header header;
		void* overflowComp;
	};
	struct BitStream
	{
		struct AuxilaryInformation aInfo; 
		void* payload;
		void* LSBs;
	};
	BitStream BS;
public:
	BitStream GetBitStream() { return BS; }
	void showOriginal() { imshow("Original", _OriginalPixels); waitKey(1); }
	void showInjected() { waitKey(1); imshow("Injected", _imagePixels); waitKey(1); }
	void showRestored() { imshow("Restored", _imagePixels); waitKey(1); }
	Mat getPixels() { return _imagePixels; }
	Mat getOriginalPixels() { return _OriginalPixels; }
	bool isEqualTo(Mat imagePixels);
	float getBppRate();
	float getPSNR();
	static void toCsv(std::vector<float> one, std::vector<float> second,std::string fileName);
	static void toCsv(float, std::string fileName);
	float getMaxBpp();
	float getDeltaValue() { return (float)delta; }
	float getMaxCapacity();
	bool CompareBitStreams(BitStream inBS);
	EEAlgo(const cv::String& filename, int flags = cv::IMREAD_GRAYSCALE,float bpp=0);
	EEAlgo(Mat pixels,float bpp);
};

