#pragma once

#include "ImageInfo.h"

class CImageConverter
{
public:
	CImageConverter();
	~CImageConverter();

	void SetInImageInfo(IMAGE_INFO* pImageInfo);
	void SetOutImageInfo(IMAGE_INFO* pImageInfo);

	BOOL AdjustImage(void* pInImage, void* pOutImage, int nOffset = 0);

private:
	BOOL DispMap16To8(WORD* pImage16, BYTE* pDispImage8, int nOffset = 0);
	BOOL DispMap8To8(BYTE* pImage8, BYTE* pDispImage8, int nOffset = 0);

protected:
	void* m_pInputImage;
	void* m_pOutImage;

	IMAGE_INFO* m_pInImageInfo;
	IMAGE_INFO* m_pOutImageInfo;
};

