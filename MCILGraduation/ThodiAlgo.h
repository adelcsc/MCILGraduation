#pragma once

#include <ctime>
#include <vector>
#include "BitArray.h"
#include <opencv2/core/cvstd.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <snappy.h>
using namespace cv;
class ThodiAlgo
{
private:
	Mat _imagePixels;
	std::vector<uchar> Low;
	
	BitArray* OverFlowMapM;
	
	std::vector<uchar> Locations;
	BitArray* LSBs;
	BitArray* Payload;
	uchar delta;
	size_t sizeOfLSBs;
	enum {NEITHER,EXPANDABLE,CHANGABLE,EXPANDABLE_IN_DELTA};
	unsigned __int32 imageSize;

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

	// a function that checks if value is in RD Range
	bool isInRdRange(short val, uchar low);

	bool isExpandable(short high, uchar low) { return isInRdRange(high << 1 | (short)0x0001, low); }

	bool isChangable(short high, uchar low) { return isInRdRange(2 * (high / 2) | (short)0x0001, low); }

	short ExpandBit(short high, uchar Bit) { return high << 1 | (short)(Bit&0x01); }
	short ChangeBit(short high, uchar Bit) { return (high >> 1) << 1 | (short)(Bit&0x01); }
	BitArray* GeneratePayload();
public:
	std::vector<short> High;
	Mat _OriginalPixels;
	std::vector<short> oHigh;
	BitStream BS;
	BitArray* ComMap;
	Mat getPixels() { return _imagePixels; }
	ThodiAlgo(const cv::String& filename, int flags = cv::IMREAD_GRAYSCALE);
	ThodiAlgo(Mat pixels);
	void Init(Mat pixels);
	void showOriginal() { imshow("Original", _imagePixels); waitKey(1); }
	void showInjected() { waitKey(1); imshow("Injected", _imagePixels); waitKey(1); }

	// a function that calculates high pass values using _imagePixels data
	void CalcHighPass();

	// determining changable and expandable locations
	void DetermineLocations();

	// Gets the right value for delta
	void GetDelta();

	void OutterHistogramShift();

	void BuildBitStream();
	
	void EmbedBitStream();

	void CompressOverFlowMap();

	void CompileImage();


	// Decoding Functions

	void GetCLocations();

	void ExtractBitStream();

	void DecompressOverFlowMap();

	void IdentifyExpandedLocations();

	void RestoreLSBs();

	void ReverseShift();

	void RestoreExpanded();
	// Statistics functions

	bool isEqualTo(Mat imagePixels);
	bool CompareBitStreams(BitStream inBS);
	bool CompareHigh(std::vector<short> tHigh);
};

