#include "stdafx.h"
#include "PixelDataPopupWnd.h"

CPixelDataPopupWnd::CPixelDataPopupWnd()
{
	m_rtParent.SetRectEmpty();
	m_rtCanvas.SetRectEmpty();
	m_rtImage.SetRectEmpty();
	m_clrDisplayPixelValue = COLORREF(RGB(0, 0, 0));
	m_ptCurImagePos = CPoint(0, 0);
}


CPixelDataPopupWnd::~CPixelDataPopupWnd()
{
}

void CPixelDataPopupWnd::SetDisplayPixelData(COLORREF clrPixelData)
{
	m_clrDisplayPixelValue = clrPixelData;
}

void CPixelDataPopupWnd::SetOutImagePixelData(BYTE bOutImagePixelValue)
{
	m_bOutImagePixelValue = bOutImagePixelValue;
}

void CPixelDataPopupWnd::SetInImagePixelData(WORD wInImagePixelValue)
{
	m_wInImagePixelValue = wInImagePixelValue;
}


void CPixelDataPopupWnd::CompositeMsg()
{
	CString strOut;
	strOut.Format(_T("x:%d y:%d R:%d G:%d B:%d Out Img:%d In Img:%d"), m_ptCurImagePos.x, m_ptCurImagePos.y, GetRValue(m_clrDisplayPixelValue), GetGValue(m_clrDisplayPixelValue), GetBValue(m_clrDisplayPixelValue), m_bOutImagePixelValue, m_wInImagePixelValue);

	SetMessage(strOut);
}

