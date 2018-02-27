#pragma once

typedef struct _IMAGE_INFO_
{
	_IMAGE_INFO_()
	{
		nWidth = 0;
		nHeight = 0;
		nBitsPerPixel = 0;
		nBytesPerPixel = 0;
		nSamplesPerPixel = 0;
		nTotalAllocBytesPerPixel = 0;
		nTotalAllocBytesOnImage = 0;
		nBytesPerLine = 0;
		nW1 = 0;
		nW2 = 0;
	}

	void init()
	{
		nWidth = 0;
		nHeight = 0;
		nBitsPerPixel = 0;
		nBytesPerPixel = 0;
		nSamplesPerPixel = 0;
		nTotalAllocBytesPerPixel = 0;
		nTotalAllocBytesOnImage = 0;
		nBytesPerLine = 0;
		nW1 = 0;
		nW2 = 0;
	}

public:
	int nWidth;
	int nHeight;
	unsigned int nBitsPerPixel;
	unsigned int nBytesPerPixel;
	unsigned int nBytesPerLine;
	unsigned int nSamplesPerPixel;	// Plane count (RGB = 3, Gray = 1) 
	unsigned int nTotalAllocBytesPerPixel;
	unsigned int nTotalAllocBytesOnImage;	// Set this value when allocated at memory
	unsigned int nW1;
	unsigned int nW2;
	

} IMAGE_INFO;