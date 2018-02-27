#pragma once
#include "ImageViewer.h"

#define MAX_VIEWER_COUNT		25

class CLayoutManager
{
public:
	CLayoutManager(HWND hParentWnd);
	~CLayoutManager();

	INT_PTR AddImageViewer(CString strInFileName);	// return ImageViewer Index
	void FreeImageViewer();

	// Init
	void Init();

	// Close
	void CloseAllImageViewer();

	// Get
	CImageViewer* GetImageViewer(INT_PTR nIndex);
	INT_PTR GetImageViewerCount();	// Viewer having Image
	INT_PTR GetTotalViewerCount();	// All Viewer (include empty viewer)
	INT_PTR GetBorderThick();
	INT_PTR GetImageViewerIndexFromPos(CPoint ptPos);

	// Set
	void SetTotalCanvasRect(CRect rtTotalCanvas);
	void SetImageViewerCanvas(INT_PTR nIndexImageViewer, CRect rtCanvas);
	void SetBorderThick(INT_PTR nThick);

	void RecalcLayout();

protected:
	CArray<CImageViewer*, CImageViewer*> m_aryImageViewer;
	CRect m_rtTotalCanvas;
	INT_PTR m_nBorderThick;

	HWND m_hViewWnd;
};

