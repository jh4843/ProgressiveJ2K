#pragma once
#include "MyPopupWnd.h"
class CPixelDataPopupWnd : public CMyPopupWnd
{
public:
	CPixelDataPopupWnd();
	~CPixelDataPopupWnd();

protected:
	COLORREF m_clrDisplayPixelValue;
	BYTE m_bOutImagePixelValue;
	WORD m_wInImagePixelValue;

public:
	void SetDisplayPixelData(COLORREF clrPixelData);
	void SetOutImagePixelData(BYTE dImagePixelValue);
	void SetInImagePixelData(WORD dImagePixelValue);
	// override
public:
	virtual void CompositeMsg();
};

