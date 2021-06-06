#pragma once
#include <opencv2/core/cvstd.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <vector>
using namespace cv;
class ThodiAlgo
{
private:
	Mat _imagePixels;
	std::vector<uchar> Low;
	std::vector<short> High;
	std::vector<uchar> OverFlowMapM;
	std::vector<uchar> Locations;
	std::vector<uchar> LSBs;
	unsigned int delta;
	enum {NEITHER,EXPANDABLE,CHANGABLE,EXPANDABLE_IN_DELTA};
	unsigned __int16 imageSize;

	struct Header {
		unsigned int SizeOfCompressedOverFlowMap;
		uchar Delta;
		unsigned int SizeOfPayload;
	};
	struct AuxilaryInformation
	{
		struct Header header;
		//TODO :Comrpessed OverFlowMap
		uchar* overflowComp;
	};
	struct BitStream
	{
		struct AuxilaryInformation aInfo;
		//TODO : Optimize this 
		uchar* payload;
		//TODO: Optimize this
		uchar* LSBs;
	};

	BitStream BS;
	// a function that checks if value is in RD Range
	bool isInRdRange(short val, uchar low);

	bool isExpandable(short high, uchar low) { return isInRdRange(high << 1 | (short)0x0001, low); }

	bool isChangable(short high, uchar low) { return isInRdRange(2 * (high / 2) | (short)0x0001, low); }

	short ExpandBit(short high, uchar Bit) { return high << 1 | (short)(Bit&0x01); }
	short ChangeBit(short high, uchar Bit) { return (high >> 1) << 1 | (short)(Bit&0x01); }

	
public:
	ThodiAlgo(const cv::String& filename, int flags = cv::IMREAD_GRAYSCALE);
	void showOriginal() { imshow("Original", _imagePixels); }
	void showInjected() { imshow("Injected", _imagePixels); }

	// a function that calculates high pass values using _imagePixels data
	void CalcHighPass();

	// determining changable and expandable locations
	void DetermineLocations();

	// Gets the right value for delta
	void GetDelta();

	void OutterHistogramShift();

	void BuildBitStream();
	
	void EmbedBitStream();

	void CompileImage();

	void Decode();
};

