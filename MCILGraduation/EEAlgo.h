#pragma once
#include <ctime>
#include <vector>
#include "BitArray.h"
#include <opencv2/core/cvstd.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
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
	enum { NEITHER, EXPANDABLE, CHANGABLE, EXPANDABLE_IN_DELTA };
	short ExpandBit(short high, uchar Bit) { return high << 1 | (short)(Bit & 0x01); }
	short ChangeBit(short high, uchar Bit) { return (high >> 1) << 1 | (short)(Bit & 0x01); }
	bool isExpandable(short high, uchar low, bool (*isInRange)(short, uchar)) { return isInRange(high << 1 | (short)0x0001, low); }
	bool isChangable(short high, uchar low, bool (*isInRange)(short, uchar)) { return isInRange((short)(2 * floor((float)high / 2)) | (short)0x0001, low); }
	BitArray* GeneratePayload();
	struct Header {
		unsigned int SizeOfCompressedOverFlowMap;
		uchar Delta;
		unsigned int SizeOfPayload;
	};
	struct AuxilaryInformation
	{
		struct Header header;
		//TODO :Comrpessed OverFlowMap
		void* overflowComp;
	};
	struct BitStream
	{
		struct AuxilaryInformation aInfo;
		//TODO : Optimize this 
		void* payload;
		//TODO: Optimize this
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
	bool CompareBitStreams(BitStream inBS);
	EEAlgo(const cv::String& filename, int flags = cv::IMREAD_GRAYSCALE);
	EEAlgo(Mat pixels);
};

