#pragma once

#include "EEAlgo.h"
using namespace cv;
class DEAlgo : public EEAlgo
{
private:
	std::vector<uchar> Low;
	std::vector<uchar> Locations;
	unsigned __int32 imageSize;
	// a function that checks if value is in RD Range
	static bool isInRdRange(short val, uchar low);
public:
	std::vector<short> High;
	DEAlgo(const cv::String& filename, int flags = cv::IMREAD_GRAYSCALE,float bpp=0);
	DEAlgo(Mat pixels,float bpp=0);
	void Init(Mat pixels);
	void CalcHighPass();
	void DetermineLocations();
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
	bool CompareHigh(std::vector<short> tHigh);
};

