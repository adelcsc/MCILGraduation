#include "pch.h"
#include "PEAlgo.h"

void PEAlgo::Init(Mat pixels)
{
	if (_imagePixels.data == NULL)
		MessageBoxA(NULL, (LPCSTR)"This Image is not supported or invalid", (LPCSTR)"Error", MB_OK);
	else
	{
		OverFlowMapM = new BitArray(_imagePixels.rows*_imagePixels.cols); //Allocating a buff that's sufficient to hold all bits
		Payload = GeneratePayload();
		MessageBoxA(NULL, (LPCSTR)"Image Loaded Successfully !", (LPCSTR)"Success !", MB_OK);
	}
}

uchar PEAlgo::PixelVal(int row, int col)
{
	if (row<0 || col<0 )
		return 0;
	return _imagePixels.at<uchar>(row,col);
}


bool PEAlgo::CompareLocations(std::vector<uchar> inLocations)
{
	for (int i = 0; i < inLocations.size(); i++)
		if (Locations.at(i) != inLocations.at(i))
			return false;
	return true;
}

bool PEAlgo::isInRpRange(short prErr, uchar prVal)
{
	if (prErr >= -prVal && prErr <= 256 - 1 - prVal)
		return true;
	return false;
}

PEAlgo::PEAlgo(String fileName) : EEAlgo(fileName)
{
	Init(_imagePixels);
}

PEAlgo::PEAlgo(Mat pixels) : EEAlgo(pixels)
{
	Init(pixels);
}

void PEAlgo::CalcPE()
{
	// Calculating Predited Values
	for (int i = 0; i < _imagePixels.rows; i++)
		for (int j = 0; j < _imagePixels.cols; j++)
		{
			uchar c1 = PixelVal(i - 1, j - 1), c2 = PixelVal(i - 1, j), c3 = PixelVal(i, j - 1);
			if (c1 <= std::min(c2, c3))
				PredictedVal.at<uchar>(i, j) = 2*floor((float)std::max(c2, c3)/2);
			else if (c1 >= std::max(c1, c3))
				PredictedVal.at<uchar>(i, j) = 2 * floor((float)std::min(c2, c3)/2);
			else
				PredictedVal.at<uchar>(i, j) = 2 * floor((float)(c2 + c3 - c1)/2);
		}
	
	//Calculating Predicted Errors
	for (int i = 0; i < _imagePixels.rows; i++)
		for (int j = 0; j < _imagePixels.cols; j++)
			PredictedErrors.at<short>(i, j) = (short)_imagePixels.at<uchar>(i, j) - (short)PredictedVal.at<uchar>(i, j);
}

void PEAlgo::GetLocations()
{
	sizeOfLSBs = 0;
	for (int i = 0; i < _imagePixels.rows; i++)
		for (int j = 0; j < _imagePixels.cols; j++)
		{
			if (isExpandable(PredictedErrors.at<short>(i, j), PredictedVal.at<uchar>(i, j), &isInRpRange))
			{
				Locations.at(i * _imagePixels.rows + j) = EXPANDABLE;
				OverFlowMapM->set(i * _imagePixels.rows + j);
				sizeOfLSBs++;
				continue;
			}
			else if (isChangable(PredictedErrors.at<short>(i, j), PredictedVal.at<uchar>(i, j), &isInRpRange))
			{
				Locations.at(i * _imagePixels.rows + j) = CHANGABLE;
				sizeOfLSBs++;
			}
			else
				Locations.at(i * _imagePixels.rows + j) = NEITHER;
			OverFlowMapM->reset(i * _imagePixels.rows + j);
		}

}

void PEAlgo::GetDelta()
{
	unsigned int bits;
	for (delta = 0; delta < 256; delta++)
	{
		bits = 0;
		for (int i = 0; i < _imagePixels.rows; i++)
			for(int j = 0;j< _imagePixels.cols;j++)
				{
					if (!(PredictedErrors.at<short>(i,j) >= -(int)delta - 1 && PredictedErrors.at<short>(i, j) <= delta))
						continue;
					if (Locations.at(i* _imagePixels.rows+j) != EXPANDABLE)
						continue;
					Locations.at(i * _imagePixels.rows + j) = EXPANDABLE_IN_DELTA;
					sizeOfLSBs--;
					bits++;
				}
		//TODO: Payload Size+Compressed Size
		if (bits >= 72 + BS.aInfo.header.SizeOfPayload + BS.aInfo.header.SizeOfCompressedOverFlowMap)
			break;
	}
}

void PEAlgo::CompressOverFlowMap()
{
	std::string* output = new std::string;
	snappy::Compress((char*)OverFlowMapM->Data(), OverFlowMapM->sizeInBytes(), output);
	ComMap = new BitArray((char*)output->data(), output->size() * 8);
	BS.aInfo.header.SizeOfCompressedOverFlowMap = ComMap->size();
	BS.aInfo.header.SizeOfPayload = Payload->size();
}

void PEAlgo::OutterHistogramShift()
{
	for (int i = 0; i < _imagePixels.rows; i++)
		for(int j = 0;j<_imagePixels.cols;j++)
			{
				if ((PredictedErrors.at<short>(i, j) > delta) && (Locations.at(i * _imagePixels.rows + j) == EXPANDABLE))
					PredictedErrors.at<short>(i, j) += delta + 1;
				else if ((PredictedErrors.at<short>(i, j) < -(short)delta - 1) && (Locations.at(i * _imagePixels.rows + j) == EXPANDABLE))
					PredictedErrors.at<short>(i, j) -= (short)delta + 1;
			}
}

void PEAlgo::BuildBitStream()
{
	BS.aInfo.header.Delta = delta;
	//TODO: Size Of coMPRESSED OVERFLOW MAP
	BS.aInfo.header.SizeOfCompressedOverFlowMap = ComMap->size();
	//TODO: Size of Payload
	BS.aInfo.header.SizeOfPayload = Payload->size();
	BS.aInfo.overflowComp = ComMap->Data();
	BS.payload = Payload->Data();
	LSBs = new BitArray(sizeOfLSBs);
	if (BS.aInfo.header.SizeOfCompressedOverFlowMap + BS.aInfo.header.SizeOfPayload + sizeOfLSBs + 72 > _imagePixels.rows*_imagePixels.cols)
	{
		MessageBoxA(NULL, (LPCSTR)"Size of Information is too large !", (LPCSTR)"Error", MB_OK);
		exit(0);
	}
	//Save LSBs of C \ Ee
	for (int i = 0; i < _imagePixels.rows; i++)
		for(int j =0;j<_imagePixels.cols;j++)
			{
				if (!(Locations.at(i* _imagePixels.rows+j) == CHANGABLE || Locations.at(i * _imagePixels.rows + j) == EXPANDABLE))
					continue;
				LSBs->push((uchar)PredictedErrors.at<short>(i,j));
			}

	BS.LSBs = LSBs->Data();
}

void PEAlgo::EmbedBitStream()
{
	BitArray headerBits((char*)&BS.aInfo.header, 0), PayloadBits((char*)BS.payload, 0), LSBsBits((char*)BS.LSBs, 0);
	unsigned int bitsEmbedded = 0;
	for (int i = 0; i < _imagePixels.rows; i++)
		for(int j =0;j<_imagePixels.cols;j++)
			{
				if (Locations.at(i * _imagePixels.rows + j) == EXPANDABLE_IN_DELTA)
				{
					if (bitsEmbedded >= 72 && bitsEmbedded < 72 + BS.aInfo.header.SizeOfCompressedOverFlowMap)
						PredictedErrors.at<short>(i,j) = ExpandBit(PredictedErrors.at<short>(i,j), (*ComMap)[bitsEmbedded - 72]);

					else if (bitsEmbedded >= 72 + BS.aInfo.header.SizeOfCompressedOverFlowMap &&
						bitsEmbedded < 72 + BS.aInfo.header.SizeOfCompressedOverFlowMap + BS.aInfo.header.SizeOfPayload)

						PredictedErrors.at<short>(i,j) = ExpandBit(PredictedErrors.at<short>(i,j), PayloadBits[bitsEmbedded - 72 - (int)BS.aInfo.header.SizeOfCompressedOverFlowMap]);

					else if (bitsEmbedded >= 72 + BS.aInfo.header.SizeOfCompressedOverFlowMap + BS.aInfo.header.SizeOfPayload)
						PredictedErrors.at<short>(i,j) = ExpandBit(PredictedErrors.at<short>(i,j), LSBsBits[bitsEmbedded - 72 - (int)BS.aInfo.header.SizeOfCompressedOverFlowMap - (int)BS.aInfo.header.SizeOfPayload]);
					else if (bitsEmbedded >= sizeof(int) * 8 + sizeof(char) * 8 && bitsEmbedded < 72)
						PredictedErrors.at<short>(i,j) = ExpandBit(PredictedErrors.at<short>(i,j), headerBits[bitsEmbedded + 3 * 8]);
					else
						PredictedErrors.at<short>(i,j) = ExpandBit(PredictedErrors.at<short>(i,j), headerBits[bitsEmbedded]);
					bitsEmbedded++;
				}
				else if (Locations.at(i * _imagePixels.rows + j) == EXPANDABLE || Locations.at(i * _imagePixels.rows + j) == CHANGABLE)
				{
					if (bitsEmbedded >= 72 && bitsEmbedded < 72 + BS.aInfo.header.SizeOfCompressedOverFlowMap)
						PredictedErrors.at<short>(i,j) = ChangeBit(PredictedErrors.at<short>(i,j), (*ComMap)[bitsEmbedded - 72]);
					else if (bitsEmbedded >=
						72 + BS.aInfo.header.SizeOfCompressedOverFlowMap &&
						bitsEmbedded < 72 + BS.aInfo.header.SizeOfCompressedOverFlowMap + BS.aInfo.header.SizeOfPayload)
						PredictedErrors.at<short>(i,j) = ChangeBit(PredictedErrors.at<short>(i,j), PayloadBits[bitsEmbedded - 72 - (int)BS.aInfo.header.SizeOfCompressedOverFlowMap]);
					else if (bitsEmbedded >= 72 + BS.aInfo.header.SizeOfCompressedOverFlowMap + BS.aInfo.header.SizeOfPayload)
						PredictedErrors.at<short>(i,j) = ChangeBit(PredictedErrors.at<short>(i,j), LSBsBits[bitsEmbedded - 72 - (int)BS.aInfo.header.SizeOfCompressedOverFlowMap - (int)BS.aInfo.header.SizeOfPayload]);
					else if (bitsEmbedded >= sizeof(int) * 8 + sizeof(char) * 8 && bitsEmbedded < 72)
						PredictedErrors.at<short>(i,j) = ChangeBit(PredictedErrors.at<short>(i,j), headerBits[bitsEmbedded + 3 * 8]);
					else
						PredictedErrors.at<short>(i,j) = ChangeBit(PredictedErrors.at<short>(i,j), headerBits[bitsEmbedded]);
					bitsEmbedded++;
				}
			}
}

void PEAlgo::CompileImage()
{
	// Gets new Pixel image based on High and low values
	for (int i = 0; i < _imagePixels.rows; i++)
		for(int j =0;j<_imagePixels.cols;j++)
			_imagePixels.at<uchar>(i, j) = PredictedVal.at<uchar>(i,j) + PredictedErrors.at<short>(i,j);
}

void PEAlgo::GetCLocations()
{
	sizeOfLSBs = 0;
	for (int i = 0; i < _imagePixels.rows; i++)
		for(int j = 0;j<_imagePixels.cols;j++)
		{
			if (isChangable(PredictedErrors.at<short>(i, j), PredictedVal.at<uchar>(i, j), &isInRpRange))
			{
				Locations.at(i* _imagePixels.rows+j) = CHANGABLE;
				sizeOfLSBs++;
			}
			else
				Locations.at(i* _imagePixels.rows+j) = NEITHER;
		}
}

void PEAlgo::ExtractBitStream()
{
	BitArray BitStreamBuilder((char*)&BS, 0);

	//Fill the empty Space between char and int of the header variables which is 3*8

	for (size_t i = sizeof(int) * 8 + sizeof(char) * 8; i < 3 * 8 + sizeof(int) * 8 + sizeof(char) * 8; i++)
		BitStreamBuilder.reset(i);

	//Extract Header
	int i = 0;
	for (i=0; i < _imagePixels.cols * _imagePixels.rows; i++)
	{
		if (Locations.at(i) == CHANGABLE)
		{
			if (BitStreamBuilder.currIndex >= sizeof(int) * 8 + sizeof(char) * 8 && BitStreamBuilder.currIndex < 64)
				BitStreamBuilder.currIndex += 3 * 8;
			if (BitStreamBuilder.currIndex >= sizeof(Header) * 8)
				goto ExtractCompressedMap;
			BitStreamBuilder.push(_imagePixels.at<uchar>(i/_imagePixels.cols, i%_imagePixels.cols));
		}
	}
	//Extract Compressed Map
	ExtractCompressedMap:
	BitArray* ComMap = new BitArray(BS.aInfo.header.SizeOfCompressedOverFlowMap);
	BS.aInfo.overflowComp = ComMap->bitArray;
	for (; i < _imagePixels.cols*_imagePixels.rows; i++)
	{
		if (Locations.at(i) == CHANGABLE)
		{
			if (ComMap->currIndex >= BS.aInfo.header.SizeOfCompressedOverFlowMap)
				goto ExtractPayload;
			ComMap->push(_imagePixels.at<uchar>(i / _imagePixels.cols, i % _imagePixels.cols));
		}
	}
	// Extract Payload
	ExtractPayload:
	BitArray* payload = new BitArray(BS.aInfo.header.SizeOfPayload);
	BS.payload = payload->bitArray;
	for (; i < _imagePixels.cols * _imagePixels.rows; i++)
	{
		if (Locations.at(i) == CHANGABLE)
			{
				if (payload->currIndex >= BS.aInfo.header.SizeOfPayload)
					goto ExtractLSBs;
				payload->push(_imagePixels.at<uchar>(i / _imagePixels.cols, i % _imagePixels.cols));
			}
	}
	//Extract LSBs
	ExtractLSBs:
	BitArray* LSBs = new BitArray(Locations.size()); //TODO: the size is maxmimum
	BS.LSBs = LSBs->bitArray;
	for (; i < _imagePixels.cols * _imagePixels.rows; i++)
		if (Locations.at(i) == CHANGABLE)
			LSBs->push(_imagePixels.at<uchar>(i / _imagePixels.cols, i % _imagePixels.cols));
}

void PEAlgo::DecompressOverFlowMap()
{
	std::string* decompressed = new std::string();
	snappy::Uncompress((char*)BS.aInfo.overflowComp, BS.aInfo.header.SizeOfCompressedOverFlowMap / 8, decompressed);
	OverFlowMapM = new BitArray((char*)decompressed->data(), _imagePixels.rows*_imagePixels.cols);
}

void PEAlgo::IdentifyExpandedLocations()
{
	// Mark E locations
	for (int i = 0; i < _imagePixels.rows*_imagePixels.cols; i++)
		if ((*OverFlowMapM)[i])
			Locations.at(i) = EXPANDABLE;

	// Get Ee Locations
	for (int i = 0; i < _imagePixels.rows*_imagePixels.cols; i++)
		if (Locations.at(i) == EXPANDABLE)
			if (PredictedErrors.at<short>(i / _imagePixels.cols, i % _imagePixels.cols) >= -2 * (short)BS.aInfo.header.Delta - 2 && PredictedErrors.at<short>(i / _imagePixels.cols, i % _imagePixels.cols) <= 2 * BS.aInfo.header.Delta + 1)
			{
				Locations.at(i) = EXPANDABLE_IN_DELTA;
				sizeOfLSBs--;
			}
}
