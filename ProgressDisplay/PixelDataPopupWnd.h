#pragma once
#include "MyPopupWnd.h"
class CPixelDataPopupWnd : public CMyPopupWnd
{
public:
	CPixelDataPopupWnd();
	~CPixelDataPopupWnd();

protected:
	COLORREF m_clrValue;

public:
	void SetPixelData(COLORREF clrPixelData);
	// override
public:
	virtual void CompositeMsg();
};

