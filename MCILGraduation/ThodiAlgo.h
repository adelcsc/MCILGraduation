#pragma once

#include "EEAlgo.h"
using namespace cv;
class ThodiAlgo : public EEAlgo
{
private:
	std::vector<uchar> Low;
	std::vector<uchar> Locations;
	unsigned __int32 imageSize;
	// a function that checks if value is in RD Range
	static bool isInRdRange(short val, uchar low);
	
public:
	std::vector<short> High;
	std::vector<short> oHigh;
	
	ThodiAlgo(const cv::String& filename, int flags = cv::IMREAD_GRAYSCALE);
	ThodiAlgo(Mat pixels);
	void Init(Mat pixels);
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

	bool CompareHigh(std::vector<short> tHigh);
};

