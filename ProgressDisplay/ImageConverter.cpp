#include "stdafx.h"
#include "ImageConverter.h"


CImageConverter::CImageConverter()
{
}


CImageConverter::~CImageConverter()
{
}

void CImageConverter::SetInImageInfo(IMAGE_INFO* pImageInfo)
{
	m_pInImageInfo = pImageInfo;
}

void CImageConverter::SetOutImageInfo(IMAGE_INFO* pImageInfo)
{
	m_pOutImageInfo = pImageInfo;
}

BOOL CImageConverter::AdjustImage(void* pInImage, void* pOutImage, int nOffset)
{
	BOOL bRes = FALSE;

	if (!pInImage)
	{
		return FALSE;
	}

	if (!pOutImage)
	{
		return FALSE;
	}

	if (!m_pInImageInfo)
	{
		return FALSE;
	}

	if (!m_pOutImageInfo)
	{
		return FALSE;
	}

	switch (m_pOutImageInfo->nTotalAllocBytesPerPixel)
	{
	case 2:
		DispMap16To8((WORD*)pInImage, (BYTE*)pOutImage, nOffset);
		break;
	case 1:
		DispMap8To8((BYTE*)pInImage, (BYTE*)pOutImage, nOffset);
		break;
	default:
		bRes = FALSE;
		break;
	}

	return bRes;
}

BOOL CImageConverter::DispMap16To8(WORD* pImage16, BYTE* pDispImage8, int nOffset)
{
	if (!pImage16)
	{
		return FALSE;
	}

	if (!pDispImage8)
	{
		return FALSE;
	}

	int nPixelCount = m_pInImageInfo->nWidth* m_pInImageInfo->nHeight;
	float fWndSt = (float)m_pInImageInfo->nW1;
	float fWndEd = (float)m_pInImageInfo->nW2;
	float fMaxValue = (float)(m_pInImageInfo->nW2 - m_pInImageInfo->nW1);
	float fPixelValue;

	// Max value of 16bit value
	if (fMaxValue <= 0)
	{
		fMaxValue = 65535;
	}

	float fCoef;
	if (fMaxValue > 0)
	{
		fCoef = 255 / fMaxValue;
	}

	for (int i = 0; i < nPixelCount; i++)
	{
		fPixelValue = (float)(WORD)(*(pImage16 + i)) + nOffset;
		fPixelValue = (float)(fPixelValue - fWndSt)*fCoef;
		if (fPixelValue < 0)
		{
			fPixelValue = 0;
		}
		if (fPixelValue > 255)
		{
			fPixelValue = 255;
		}
		*(pDispImage8 + i) = ((BYTE)fPixelValue) & 0xFF;
	}

	return TRUE;
}

BOOL CImageConverter::DispMap8To8(BYTE* pImage8, BYTE* pDispImage8, int nOffset)
{
	if (!pImage8)
	{
		return FALSE;
	}

	if (!pDispImage8)
	{
		return FALSE;
	}

	int nPixelCount = m_pInImageInfo->nWidth * m_pInImageInfo->nHeight;
	//int nPixelCount = m_pInImageInfo->nTotalAllocBytesOnImage;
	float fWndSt = (float)m_pInImageInfo->nW1;
	float fWndEd = (float)m_pInImageInfo->nW2;
	float fMaxValue = (float)(m_pInImageInfo->nW2 - m_pInImageInfo->nW1);
	float fPixelValue;

	float fCoef;
	if (fMaxValue > 0)
	{
		fCoef = 255 / fMaxValue;
	}
	else
	{
		fCoef = 255;
	}

	for (int i = 0; i < nPixelCount; i++)
	{
		fPixelValue = (BYTE)(*(pImage8 + i) + nOffset);
		fPixelValue = (float)(fPixelValue - fWndSt)*fCoef;
		if (fPixelValue < 0)
		{
			fPixelValue = 0;
		}

		if (fPixelValue > 255)
		{
			fPixelValue = 255;
		}

		*(pDispImage8 + i) = ((BYTE)fPixelValue) & 0xFF;
	}

	return TRUE;
}