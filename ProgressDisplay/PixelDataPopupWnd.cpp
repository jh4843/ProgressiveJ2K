#include "stdafx.h"
#include "PixelDataPopupWnd.h"

CPixelDataPopupWnd::CPixelDataPopupWnd()
{
	m_rtParent.SetRectEmpty();
	m_rtCanvas.SetRectEmpty();
	m_rtImage.SetRectEmpty();
	m_clrValue = COLORREF(RGB(0, 0, 0));
	m_ptCurPos = Gdiplus::PointF(0, 0);
}


CPixelDataPopupWnd::~CPixelDataPopupWnd()
{
}

void CPixelDataPopupWnd::SetPixelData(COLORREF clrPixelData)
{
	m_clrValue = clrPixelData;
}

void CPixelDataPopupWnd::CompositeMsg()
{
	CString strOut;
	strOut.Format(_T("x:%.2f y:%.2f R:%d G:%d B:%d"), m_ptCurPos.X, m_ptCurPos.Y, GetRValue(m_clrValue), GetGValue(m_clrValue), GetBValue(m_clrValue));

	SetMessage(strOut);
}

