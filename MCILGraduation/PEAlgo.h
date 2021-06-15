#pragma once
#include "EEAlgo.h"
class PEAlgo : public EEAlgo
{
private : 
	Mat PredictedErrors = Mat(_imagePixels.rows, _imagePixels.cols, CV_16FC1);
	Mat PredictedVal= Mat(_imagePixels.rows, _imagePixels.cols, CV_8UC1);
	
	void Init(Mat pixels); 
	uchar PixelVal(int row, int col);
	
public:
	PEAlgo(String fileName);
	PEAlgo(Mat pixels);
	void CalcPE();
	std::vector<unsigned char> Locations = std::vector<unsigned char>(_imagePixels.rows * _imagePixels.cols);
	void GetLocations();
	void GetDelta();
	void CompressOverFlowMap();
	void OutterHistogramShift();
	void BuildBitStream();
	void EmbedBitStream();
	void CompileImage();
	void GetCLocations();
	void ExtractBitStream();
	void DecompressOverFlowMap();
	void IdentifyExpandedLocations();
	void RecoverOriginalValues();
	//Compare
	bool CompareLocations(std::vector<uchar> inLocations);
	static bool isInRpRange(short prErr, uchar prVal);
};

