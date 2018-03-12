
// ProgressDisplayView.h : interface of the CProgressDisplayView class
//

#pragma once
#include "ProgressDisplayDoc.h"
#include "ImageInfo.h"
#include "LayoutManager.h"

#define MODE_NORMAL		0
#define MODE_PAN		1
#define MODE_PAN_DOWN	2

struct DIBINFO : public BITMAPINFO
{
	RGBQUAD arColors[255];
	operator LPBITMAPINFO()
	{
		return (LPBITMAPINFO)this;
	}
	operator LPBITMAPINFOHEADER()
	{
		return &bmiHeader;
	}
	RGBQUAD* ColorTable()
	{
		return bmiColors;
	}
	void Init()
	{
		LPRGBQUAD pColorTable = ColorTable();
		for (UINT nColor = 0; nColor < 256; nColor++)
		{
			pColorTable[nColor].rgbReserved = 0;
			pColorTable[nColor].rgbBlue = nColor;
			pColorTable[nColor].rgbRed = nColor;
			pColorTable[nColor].rgbGreen = nColor;
		}
	}
};

class CProgressDisplayView : public CView
{
protected: // create from serialization only
	CProgressDisplayView();
	DECLARE_DYNCREATE(CProgressDisplayView)

// Attributes
public:
	CProgressDisplayDoc* GetDocument() const;

protected:
	void* m_pOutImage;

	IMAGE_INFO m_stOutImageInfo;
	INT_PTR m_nSelectedViewerIndex;

	CLayoutManager* m_pLayoutManager;

	DIBINFO m_DibInfo;
	HBITMAP m_hBitmap;

	BOOL m_bIsFirstPreview;
	BOOL m_bIsShowCurrentPosition;

	INT_PTR m_nOperationMode;

private:
	BOOL bFlagIsDecompressing;

// Operations
public:
	BOOL IsJpeg2000Format(CString strFileName);

protected:
	//
	void OpenJpegFile(CStringArray* pFilePathArray);
	void ChangeLayer(CString strFilePath, INT_PTR nLayer);
	void SetOperationModeByKey(BOOL bShift, BOOL bCtrl, BOOL bAlt, BOOL bLeftDown, BOOL bRightDown);
	DIBINFO* GetDibInfo();

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// Implementation
public:
	virtual ~CProgressDisplayView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnFileOpen();
	virtual void OnDraw(CDC* /*pDC*/);
	afx_msg void OnDropFiles(HDROP hDropInfo);
	
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnFileCloseallimages();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnViewSeefirstlayer();
	afx_msg void OnUpdateViewSeefirstlayer(CCmdUI *pCmdUI);
	afx_msg void OnViewShowCurrentPosition();
	afx_msg void OnUpdateViewShowCurrentPosition(CCmdUI *pCmdUI);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnOperationViewequalposition();
	afx_msg void OnMove(int x, int y);
};

#ifndef _DEBUG  // debug version in ProgressDisplayView.cpp
inline CProgressDisplayDoc* CProgressDisplayView::GetDocument() const
   { return reinterpret_cast<CProgressDisplayDoc*>(m_pDocument); }
#endif

