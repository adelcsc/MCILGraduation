#include "pch.h"
#include "PEAlgo.h"

void PEAlgo::Init(Mat pixels)
{
	if (_imagePixels.data == NULL)
		MessageBoxA(NULL, (LPCSTR)"This Image is not supported or invalid", (LPCSTR)"Error", MB_OK);
	else
	{
		//OverFlowMapM = new BitArray(imageSize); //Allocating a buff that's sufficient to hold all bits
		Payload = GeneratePayload();
		//Locations = std::vector<uchar>(imageSize, 0);//Same here
		MessageBoxA(NULL, (LPCSTR)"Image Loaded Successfully !", (LPCSTR)"Success !", MB_OK);
	}
}

uchar PEAlgo::PixelVal(unsigned int row, unsigned int col)
{
	if (row<0 || col<0 )
		return 0;
	return _imagePixels.at<uchar>(row,col);
}

PEAlgo::PEAlgo(String fileName) : EEAlgo(fileName)
{
	Init(_imagePixels);
}

void PEAlgo::CalcPE()
{
	for (int i = 0; i < _imagePixels.rows; i++)
		for (int j = 0; j < _imagePixels.cols; j++)
		{
			uchar c1 = PixelVal(i - 1, j - 1), c2 = PixelVal(i - 1, j), c3 = PixelVal(i, j - 1);
			if (c1 <= std::min(c2, c3))
				PredictedVal.at<uchar>(i, j) = std::max(c2, c3);
			else if (c1 >= std::max(c1, c3))
				PredictedVal.at<uchar>(i, j) = std::min(c2, c3);
			else
				PredictedVal.at<uchar>(i, j) = c2 + c3 - c1;
		}
}
