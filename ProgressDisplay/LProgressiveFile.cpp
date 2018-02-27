#include "stdafx.h"
#include "LProgressiveFile.h"

LEAD_IMPLEMENTOBJECT(LProgressiveFile)

LProgressiveFile::LProgressiveFile()
{
	m_hWnd = 0;
}

LProgressiveFile::~LProgressiveFile()
{
}

L_VOID LProgressiveFile::SetDisplayWnd(HWND hWnd)
{
	m_hWnd = hWnd;
}

L_INT LProgressiveFile::LoadFile(L_INT nBitsPerPixel, L_INT nOrder, L_UINT uFlags, pLOADFILEOPTION pLoadFileOption)
{
	L_BOOL bTemp;
	L_INT nRet;
	LOADFILEOPTION LoadFileOption;
	if (pLoadFileOption == NULL)
	{
		LBaseFile::GetDefaultLoadFileOption(&LoadFileOption, sizeof(LOADFILEOPTION));
		pLoadFileOption = &LoadFileOption;
	}
	
	if (!IsWindow(m_hWnd))
		LEAD_ERROR_RETURN(WRPERR_INVALID_PARAMETERS);
	bTemp = EnableCallBack(TRUE);
	nRet = LFile::LoadFile(nBitsPerPixel, nOrder, uFlags, pLoadFileOption);
	
	return(nRet);
}
L_INT LProgressiveFile::SaveBitmap(L_INT nFormat, L_INT nBitsPerPixel, L_INT nQFactor, L_UINT uFlags, pSAVEFILEOPTION pSaveFileOption)
{
	L_INT iOldPasses;
	SAVEFILEOPTION SaveFileOption;
	if (IsValid())
	{
		if (pSaveFileOption == NULL)
		{
			LBaseFile::GetDefaultSaveFileOption(&SaveFileOption, sizeof(SAVEFILEOPTION));
			pSaveFileOption = &SaveFileOption;
		}
		iOldPasses = pSaveFileOption->Passes;
		pSaveFileOption->Passes = 10;
	}
	L_INT nRet = LFile::Save(nFormat, nBitsPerPixel, nQFactor, uFlags, pSaveFileOption);
	pSaveFileOption->Passes = iOldPasses;
	return nRet;
}
L_INT LProgressiveFile::LoadFileCallBack(pFILEINFO pFileInfo, LBitmapBase * pLBitmap, LBuffer * pLBuffer, L_UINT uFlags, L_INT nRow, L_INT nLines)
{
	LBuffer LBufferExp;
	HDC hDC;
	RECT Rect;

	UNREFERENCED_PARAMETER(pFileInfo);
	UNREFERENCED_PARAMETER(uFlags);
	UNREFERENCED_PARAMETER(nLines);
	hDC = ::GetDC(m_hWnd);
	pLBitmap->Paint()->SetDC(hDC);
	pLBitmap->Paint()->PaintDCBuffer(*pLBuffer, nRow);

// 	LBuffer LBufferExp;
// 	HDC hDC;
// 	RECT Rect;
// 	if (!IsWindow(m_hWnd))
// 		return(SUCCESS);
// 	::GetClientRect(m_hWnd, &Rect);
// 	if (uFlags&FILEREAD_COMPRESSED)
// 		LBufferExp.ExpandRows(*pLBuffer, pLBitmap->GetWidth(), nLines);
// 	hDC = ::GetDC(m_hWnd);
// 	pLBitmap->SetSrcRect(&Rect);
// 	pLBitmap->SetClipSrcRect(&Rect);
// 	pLBitmap->SetDstRect(&Rect);
// 	pLBitmap->SetClipDstRect(&Rect);
// 	pLBitmap->Paint()->SetDC(hDC);
// 	pLBitmap->Paint()->PaintDCBuffer(*pLBuffer, nRow);
// 	pLBitmap->Paint()->SetDC(0);
// 	Sleep(3);
	::ReleaseDC(m_hWnd, hDC);
	return(SUCCESS);
}