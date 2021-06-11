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
