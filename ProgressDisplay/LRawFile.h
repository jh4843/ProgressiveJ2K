#pragma once
#include "C:\Program Files (x86)\LEAD Technologies\LEADTOOLS 17\Include\ClassLib\LtcFile.h"

typedef struct tagRAWLOADDATA
{
	L_INT nWidth; //width of image 
	L_INT nHeight; //height of image 
	L_INT nBitsPerPixel; //bits per pixel of image--if palettized, a gray palette is generated 
	L_INT nViewPerspective; //view perspective of raw data (TOP_LEFT, BOTTOM_LEFT, etc) 
	L_INT nOrder; //ORDER_RGB or ORDER_BGR 
	L_INT nOffset; //offset into file where raw data begins 
	L_INT bPadding; //TRUE if each line of data is padded to four bytes 
	L_INT bReverseBits; //TRUE if the bits of each byte are reversed 
} RAWLOADDATA, *LPRAWLOADDATA;

class LRawFile : public LFile
{
public:
	LRawFile();
	~LRawFile();

protected:
	RAWLOADDATA m_rawData;
public:
	void SetRawLoadData(LPRAWLOADDATA pRawData);
	L_INT LoadInfoCallBack(L_INT fd, pLOADINFO pInfo);
	L_INT LoadRawData(L_TCHAR *pszFile, LBitmapBase* pLEADBitmap, LPRAWLOADDATA pRawData);
};

