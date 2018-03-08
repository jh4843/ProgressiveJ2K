#pragma once
#include "afxwin.h"
class CMyPopupWnd :	public CWnd
{
public:
	CMyPopupWnd();
	~CMyPopupWnd();

protected:
	CWnd* m_pNotifyWnd;

	// To display
	CString m_strMessage;
	COLORREF m_crBk;
	COLORREF m_crText;
	CString m_strFontName;
	float m_fFontSize;
	CPoint m_ptCurPos;
	CPoint m_ptHangOnPos;

	CRect m_rtParent;
	CRect m_rtCanvas;
	CRect m_rtImage;

	BOOL m_bIsHangOn;

	// Can override
public:
	virtual BOOL CreateCtrl(CWnd* pParent);
	virtual void ShowPopupMsg();
	virtual void HidePopupMsg();
	virtual void DrawPopupWnd(CDC* pDC);
	virtual void CompositeMsg();

public:
	// Set
	void SetParentRect(CRect rtParent);	// Parent's Client Rect
	void SetCanvasRect(CRect rtCanvas);
	void SetImageRect(CRect rtImage);

	void SetBkColor(COLORREF crBk);
	void SetTextColor(COLORREF crText);
	void SetFontName(CString strFontName);
	void SetFontSize(float fFontSize);
	void SetMessage(CString strText);
	void SetHangOn(BOOL bHangOn);
	void SetCursorPos(CPoint ptCurPos);

	// Get
	BOOL GetHangOn();

protected:
	BOOL SelectInvalidateRgn(CDC* pDC);
	Gdiplus::SizeF MeasureString(CString strText);
	
	DECLARE_MESSAGE_MAP()
	afx_msg void OnDestroy();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();

private:
	Gdiplus::PointF ConvertScreen2ImageCoordinate(CPoint ptPoint);
};

