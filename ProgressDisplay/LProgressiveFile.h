#pragma once
#include "LtcFile.h"
class LProgressiveFile : public LFile
{
	LEAD_DECLAREOBJECT(LProgressiveFile);

private:
	HWND m_hWnd;

protected:
	virtual L_INT LoadFileCallBack(pFILEINFO pFileInfo, LBitmapBase * pLBitmap, LBuffer * pLBuffer, L_UINT uFlags, L_INT nRow, L_INT nLines);

public:
	LProgressiveFile();
	~LProgressiveFile();

	L_VOID SetDisplayWnd(HWND hWnd);
	L_INT LoadFile(L_INT nBitsPerPixel = 0, L_INT nOrder = ORDER_BGRORGRAY, L_UINT uFlags = LOADFILE_ALLOCATE | LOADFILE_STORE, pLOADFILEOPTION pLoadFileOption = NULL);
	L_INT SaveBitmap(L_INT nFormat, L_INT nBitsPerPixel = 0, L_INT nQFactor = 2, L_UINT uFlags = MULTIPAGE_OPERATION_OVERWRITE, pSAVEFILEOPTION pSaveFileOption = NULL);
};

