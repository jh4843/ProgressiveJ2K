#include "stdafx.h"
#include "MyPopupWnd.h"


CMyPopupWnd::CMyPopupWnd()
{
	m_strMessage.Empty();

	m_crBk = RGB(16, 16, 16);
	m_crText = RGB(255, 255, 255);
	m_strFontName = _T("Segoe UI");
	m_fFontSize = 11.0f;

	m_ptPopupPos = CPoint(0, 0);
	m_ptCurImagePos = CPoint(0, 0);

	m_bIsHangOn = FALSE;
}


CMyPopupWnd::~CMyPopupWnd()
{
}

BOOL CMyPopupWnd::CreateCtrl(CWnd* pParent)
{
	CString strClassName;
	strClassName = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW, NULL, (HBRUSH)GetStockObject(NULL_BRUSH), NULL);

	// No Activate and Top Most
	if (!CWnd::CreateEx(WS_EX_NOACTIVATE | WS_EX_LAYERED, strClassName, NULL, WS_POPUP | WS_VISIBLE, CRect(0, 0, 0, 0), pParent/*this*/, 0, NULL))
		return FALSE;

	m_pNotifyWnd = pParent;

	return TRUE;
}

void CMyPopupWnd::ShowPopupMsg()
{
	if (m_strMessage.IsEmpty())
		return;

	Gdiplus::SizeF size = MeasureString(m_strMessage);
	//
	size.Width += (size.Height * 2.0f) + 2.0f;
	size.Height += 4.0f;
	//
	CSize sizeNew((int)size.Width, (int)size.Height);
	//
	MoveWindow(m_ptPopupPos.x-2, m_ptPopupPos.y-2, sizeNew.cx+2, sizeNew.cy+2);

	if (!IsWindowVisible())
	{
		ShowWindow(SW_SHOW);
	}
	//
	Invalidate(FALSE);
	UpdateWindow();
}

void CMyPopupWnd::HidePopupMsg()
{
// 	if (m_bIsHangOn == TRUE)
// 		return;

	if (IsWindowVisible())
	{
		ShowWindow(SW_HIDE);
	}
}

void CMyPopupWnd::DrawPopupWnd(CDC* pDC)
{
	CRect rtClientWnd;

	GetClientRect(rtClientWnd);

	Gdiplus::Bitmap bmpScreen(rtClientWnd.Width(), rtClientWnd.Height());

	HBITMAP hBmpScreen = NULL;
	bmpScreen.GetHBITMAP(Gdiplus::Color(0, 0, 0, 0), &hBmpScreen);

	CDC memDC;
	memDC.CreateCompatibleDC(pDC);

	CDC* pMemDC = &memDC;
	pMemDC->SelectObject(hBmpScreen);

	Gdiplus::Graphics graphics(pMemDC->GetSafeHdc());
	graphics.Clear(Gdiplus::Color(0, 0, 0, 0));
	graphics.SetSmoothingMode(SmoothingModeHighQuality);
	graphics.SetTextRenderingHint(TextRenderingHintAntiAlias);

	Gdiplus::RectF rcClient;
	rcClient.X = 0.0f;
	rcClient.Y = 0.0f;
	rcClient.Width = (float)rtClientWnd.Width();
	rcClient.Height = (float)rtClientWnd.Height();

	Gdiplus::Color crBk(200, GetRValue(m_crBk), GetGValue(m_crBk), GetBValue(m_crBk));

	// 	//
	Gdiplus::Pen pen(crBk, rcClient.Height - 2.0f);
	pen.SetStartCap(LineCapRound);
	pen.SetEndCap(LineCapRound);
	//
	Gdiplus::PointF ptStart(rcClient.Height, rcClient.Height / 2.0f);
	Gdiplus::PointF ptEnd(rcClient.Width - rcClient.Height, ptStart.Y);

	graphics.DrawLine(&pen, ptStart, ptEnd);

	Gdiplus::Color crRed(200, GetRValue(200), GetGValue(0), GetBValue(0));
	if (m_bIsHangOn)
	{
		crRed.SetFromCOLORREF(RGB(0, 200, 100));
	}

	Gdiplus::Pen penRect(crRed, 1);

	graphics.DrawRectangle(&penRect, 0, 0, 3, 3);
	//
	//
	Gdiplus::StringFormat strFormat = Gdiplus::StringFormat::GenericTypographic();
	strFormat.SetTrimming(StringTrimmingEllipsisCharacter);
	strFormat.SetAlignment(StringAlignmentCenter);
	strFormat.SetLineAlignment(StringAlignmentCenter);

	Gdiplus::FontFamily fontFamily(m_strFontName);
	Gdiplus::Font font(&fontFamily, m_fFontSize, FontStyleRegular, UnitPixel);

	//
	Gdiplus::PointF ptOrigin(0.0f, 0.0f);
	Gdiplus::RectF rcMeasure(0.0f, 0.0f, 0.0f, 0.0f);
	graphics.MeasureString(m_strMessage, m_strMessage.GetLength(), &font, ptOrigin, &strFormat, &rcMeasure);

	//
	Gdiplus::Color crText;
	crText.SetFromCOLORREF(m_crText);
	//
	Gdiplus::SolidBrush brushText(crText);
	//
	Gdiplus::RectF rcText = rcMeasure;
	rcText.X = rcClient.Width / 2.0f - rcMeasure.Width / 2.0f;
	rcText.Y = rcText.Height / 2.0f - rcMeasure.Height / 2.0f;
	//
	graphics.DrawString(m_strMessage, m_strMessage.GetLength(), &font, rcText/*rcClient*/, &strFormat, &brushText);

	//
	CRect rcParent;
	//
	ASSERT(m_pNotifyWnd);
	m_pNotifyWnd->GetWindowRect(&rcParent);
	//
	GetWindowRect(&rcParent);
	//

	CPoint ptDest;
	ptDest.x = rcParent.left + (rcParent.Width() / 2 - rtClientWnd.Width() / 2);
	ptDest.y = rcParent.top + (rcParent.Height() / 2 - rtClientWnd.Height() / 2);
	//	ptDest = ptCursor;

	//
	CSize sizeDest = rtClientWnd.Size();
	CPoint ptSrc(0, 0);

	int nSourceConstantAlpha = 255;

	//
	BLENDFUNCTION blendfunction = { AC_SRC_OVER, 0, nSourceConstantAlpha, AC_SRC_ALPHA };
	UpdateLayeredWindow(pDC, &ptDest, &sizeDest, pMemDC, &ptSrc, RGB(0, 0, 0), &blendfunction, ULW_ALPHA);

	if (hBmpScreen)
	{
		::DeleteObject(hBmpScreen);
		hBmpScreen = NULL;
	}
}

void CMyPopupWnd::CompositeMsg()
{

}

void CMyPopupWnd::SetParentRect(CRect rtParent)
{
	m_rtParent = rtParent;
}

void CMyPopupWnd::SetCanvasRect(CRect rtCanvas)
{
	m_rtCanvas = rtCanvas;
}

void CMyPopupWnd::SetImageRect(CRect rtImage)
{
	m_rtImage = rtImage;
}


void CMyPopupWnd::SetBkColor(COLORREF crBk)
{
	m_crBk = crBk;
}

void CMyPopupWnd::SetTextColor(COLORREF crText)
{
	m_crText = crText;
}

void CMyPopupWnd::SetFontName(CString strFontName)
{
	m_strFontName = strFontName;
}

void CMyPopupWnd::SetFontSize(float fFontSize)
{
	m_fFontSize = fFontSize;
}

void CMyPopupWnd::SetMessage(CString strText)
{
	m_strMessage = strText;
}

void CMyPopupWnd::SetHangOn(BOOL bHangOn)
{
	m_bIsHangOn = bHangOn;
}

void CMyPopupWnd::SetCursorImagePos(CPoint ptCurPos)
{
	m_ptCurImagePos = ptCurPos;
}

void CMyPopupWnd::SetPopupPos(CPoint ptCurPos)
{
	m_ptPopupPos = ptCurPos;
}

BOOL CMyPopupWnd::GetHangOn()
{
	return m_bIsHangOn;
}

BOOL CMyPopupWnd::SelectInvalidateRgn(CDC* pDC)
{
	if (!pDC)
	{
		return FALSE;
	}

	pDC->SelectClipRgn(NULL);

	CWnd* pWndChild = GetWindow(GW_CHILD);
	while (pWndChild)
	{
		if (pWndChild->IsWindowEnabled() && pWndChild->IsWindowVisible())
		{
			CRect rcChild;
			pWndChild->GetWindowRect(&rcChild);
			ScreenToClient(&rcChild);

			CRgn rgnChild;
			rgnChild.CreateRectRgn(rcChild.left, rcChild.top, rcChild.right, rcChild.bottom);
			pDC->SelectClipRgn(&rgnChild, RGN_DIFF);
			rgnChild.DeleteObject();
			//
		}

		pWndChild = pWndChild->GetNextWindow();
	}
}

Gdiplus::SizeF CMyPopupWnd::MeasureString(CString strText)
{
	ASSERT(GetSafeHwnd());
	//
	Gdiplus::StringFormat strFormat = Gdiplus::StringFormat::GenericTypographic();
	strFormat.SetTrimming(StringTrimmingEllipsisCharacter);
	strFormat.SetAlignment(StringAlignmentCenter);
	strFormat.SetLineAlignment(StringAlignmentCenter);

	Gdiplus::FontFamily fontFamily(m_strFontName);

	Gdiplus::Font font(&fontFamily, m_fFontSize, FontStyleBold, UnitPixel);

	CClientDC dc(this);
	Gdiplus::Graphics graphics(dc.GetSafeHdc());
	//
	Gdiplus::PointF ptOrigin(0.0f, 0.0f);
	Gdiplus::RectF rcMeasure(0.0f, 0.0f, 0.0f, 0.0f);
	graphics.MeasureString(strText, strText.GetLength(), &font, ptOrigin, &strFormat, &rcMeasure);

	Gdiplus::SizeF size(0.0f, 0.0f);
	rcMeasure.GetSize(&size);

	return size;
}

BEGIN_MESSAGE_MAP(CMyPopupWnd, CWnd)
ON_WM_DESTROY()
ON_WM_CREATE()
ON_WM_PAINT()
END_MESSAGE_MAP()


void CMyPopupWnd::OnDestroy()
{
	CWnd::OnDestroy();

	// TODO: Add your message handler code here
}


int CMyPopupWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here

	return 0;
}


void CMyPopupWnd::OnPaint()
{
	CPaintDC dc(this); 

	SelectInvalidateRgn(&dc);

	DrawPopupWnd(&dc);
}

Gdiplus::PointF CMyPopupWnd::ConvertScreen2ImageCoordinate(CPoint point)
{
	Gdiplus::PointF ptCanvas(point.x - m_rtCanvas.left, point.y - m_rtCanvas.top);
	Gdiplus::PointF ptImage(0, 0);

	CRect rtCanvasAbsolutCoord;
	rtCanvasAbsolutCoord.left = 0;
	rtCanvasAbsolutCoord.top = 0;
	rtCanvasAbsolutCoord.right = rtCanvasAbsolutCoord.left + m_rtCanvas.Width();
	rtCanvasAbsolutCoord.bottom = rtCanvasAbsolutCoord.top + m_rtCanvas.Height();

	float fRatioX = (double)m_rtImage.Width() / (float)rtCanvasAbsolutCoord.Width();
	float fRatioY = (double)m_rtImage.Height() / (float)rtCanvasAbsolutCoord.Height();
	ptImage.X = (ptCanvas.X - rtCanvasAbsolutCoord.left)*fRatioX + m_rtImage.left;
	ptImage.Y = (ptCanvas.Y - rtCanvasAbsolutCoord.top)*fRatioY + m_rtImage.top;

	//
	return ptImage;
}