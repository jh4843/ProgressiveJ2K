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

	// Can override
public:
	virtual BOOL CreateCtrl(CWnd* pParent);
	virtual void ShowPopupMsg(BOOL bMakeCurMousePos);
	virtual void DrawPopupWnd(CDC* pDC);

public:
	// Set
	void SetBkColor(COLORREF crBk);
	void SetTextColor(COLORREF crText);
	void SetFontName(CString strFontName);
	void SetFontSize(float fFontSize);
	void SetMessage(CString strText);

protected:
	BOOL SelectInvalidateRgn(CDC* pDC);
	Gdiplus::SizeF MeasureString(CString strText);
	
	DECLARE_MESSAGE_MAP()
	afx_msg void OnDestroy();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
};

