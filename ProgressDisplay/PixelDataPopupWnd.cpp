#include "stdafx.h"
#include "PixelDataPopupWnd.h"

CPixelDataPopupWnd::CPixelDataPopupWnd()
{
	m_rtParent.SetRectEmpty();
	m_rtCanvas.SetRectEmpty();
	m_rtImage.SetRectEmpty();
	m_clrValue = COLORREF(RGB(0, 0, 0));
	m_ptCurPos = CPoint(0, 0);
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
	strOut.Format(_T("x:%d y:%d R:%d G:%d B:%d"), m_ptCurPos.x, m_ptCurPos.y, GetRValue(m_clrValue), GetGValue(m_clrValue), GetBValue(m_clrValue));

	SetMessage(strOut);
}

