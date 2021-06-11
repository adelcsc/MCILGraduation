#pragma once
#include "EEAlgo.h"
class PEAlgo : public EEAlgo
{
private : 
	Mat PredictedErrors = Mat(_imagePixels.rows, _imagePixels.cols, CV_8UC1);
	Mat PredictedVal= Mat(_imagePixels.rows, _imagePixels.cols, CV_8UC1);;
	void Init(Mat pixels);
	uchar PixelVal(unsigned int row, unsigned int col);
public:
	PEAlgo(String fileName);
	void CalcPE();
};

