#include "stdafx.h"
#include "LRawFile.h"


LRawFile::LRawFile()
{
	ZeroMemory(&m_rawData, sizeof(RAWLOADDATA));
}


LRawFile::~LRawFile()
{
}

void LRawFile::SetRawLoadData(LPRAWLOADDATA pRawData)
{
	m_rawData.nWidth = pRawData->nWidth;
	m_rawData.nHeight = pRawData->nHeight;
	m_rawData.nBitsPerPixel = pRawData->nBitsPerPixel;
	m_rawData.nViewPerspective = pRawData->nViewPerspective;
	m_rawData.nOrder = pRawData->nOrder;
	m_rawData.nOffset = pRawData->nOffset;
	m_rawData.bPadding = pRawData->bPadding;
	m_rawData.bReverseBits = pRawData->bReverseBits;
}

L_INT LRawFile::LoadInfoCallBack(L_INT fd, pLOADINFO pInfo)
{
	UNREFERENCED_PARAMETER(fd);
	pInfo->Format = FILE_RAW;
	pInfo->Width = m_rawData.nWidth;
	pInfo->Height = m_rawData.nHeight;
	pInfo->BitsPerPixel = m_rawData.nBitsPerPixel;
	pInfo->XResolution = 150;
	pInfo->YResolution = 150;
	pInfo->Offset = m_rawData.nOffset;
	pInfo->Flags = 0;
	if (m_rawData.bPadding)
		pInfo->Flags |= LOADINFO_PAD4;
	if (m_rawData.nOrder == ORDER_RGB)
		pInfo->Flags |= LOADINFO_ORDERRGB;
	if (m_rawData.bReverseBits)
		pInfo->Flags |= LOADINFO_LSB;
	switch (m_rawData.nViewPerspective)
	{
	case TOP_LEFT:
		pInfo->Flags |= LOADINFO_TOPLEFT;
		break;
	case BOTTOM_LEFT:
		pInfo->Flags |= LOADINFO_REVERSE;
		break;
	case TOP_RIGHT://BOTTOM_LEFT180 
		pInfo->Flags |= LOADINFO_BOTTOMLEFT180;
		break;
	case BOTTOM_RIGHT: //TOP_LEFT180 
		pInfo->Flags |= LOADINFO_TOPLEFT180;
		break;
	case RIGHT_TOP: //TOP_LEFT90 
		pInfo->Flags |= LOADINFO_TOPLEFT90;
		break;
	case LEFT_BOTTOM: //TOP_LEFT270 
		pInfo->Flags |= LOADINFO_TOPLEFT270;
		break;
	case LEFT_TOP: //BOTTOM_LEFT90 
		pInfo->Flags |= LOADINFO_BOTTOMLEFT90;
		break;
	case RIGHT_BOTTOM: //BOTTOM_LEFT270 
		pInfo->Flags |= LOADINFO_BOTTOMLEFT270;
		break;
	}
	//if image is palettized create a grayscale palette 
	if (pInfo->BitsPerPixel <= 8)
	{
		L_INT nColors, i;
		nColors = 1 << pInfo->BitsPerPixel;
		pInfo->Flags |= LOADINFO_PALETTE;
		for (i = 0; i < nColors; i++)
		{
			pInfo->rgbQuad[i].rgbBlue = (i * 256) / nColors;
			pInfo->rgbQuad[i].rgbGreen = (i * 256) / nColors;
			pInfo->rgbQuad[i].rgbRed = (i * 256) / nColors;
			pInfo->rgbQuad[i].rgbReserved = 0;
		}
	}
	return SUCCESS;
}

//This example loads RAW data into a LEAD LBitmapBase object. 
//Raw data parameters are set in the overridden LUserFile::LoadInfoCallBack function, 
//which gets called when the file format is FILE_RAW, or any unrecognized file format. 
//The LUserFile::LoadInfoCallBack obtains information through a user-defined structure. 
//In this example it is RAWLOADDATA 
L_INT LRawFile::LoadRawData(L_TCHAR *pszFile, LBitmapBase* pLEADBitmap, LPRAWLOADDATA pRawData)
{
	L_INT nRet = FALSE;
	FILEINFO fileInfo;
	LRawFile file;
	memset(&fileInfo, 0, sizeof(fileInfo));
	fileInfo.uStructSize = sizeof(fileInfo);
	fileInfo.Format = FILE_RAW;
	fileInfo.Flags = FILEINFO_FORMATVALID;

	file.SetBitmap(pLEADBitmap);
	file.SetFileName(pszFile);
	file.SetRawLoadData(pRawData);
	file.EnableLoadInfoCallBack(TRUE);
	nRet = file.LoadFile(0, ORDER_BGR, LOADFILE_ALLOCATE | LOADFILE_STORE, NULL, &fileInfo);
	file.EnableLoadInfoCallBack(FALSE);

	return nRet;
}