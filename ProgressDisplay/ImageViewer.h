#pragma once
#include "ImageInfo.h"

#include "PixelDataPopupWnd.h"

#define MIN_ZOOM_RATIO	0.1
#define MAX_ZOOM_RATIO	100.0

class CImageViewer
{
public:
	CImageViewer(INT_PTR nIndexViewer, HWND hWndView);
	~CImageViewer();

protected:
	void* m_pInImage;
	void* m_pOutImage;

	CString m_strCompressedFileName; // .jp2, .j2k

	IMAGE_INFO m_stInImageInfo;
	IMAGE_INFO m_stOutImageInfo;

	INT_PTR m_nCurrentLayerNum;
	INT_PTR m_nTotalLayerNum;

	CPoint m_ptPixelDataPos;

	double m_dCanvasPerImageRatio;
	double m_dZoomValue;

	CPoint m_ptPanDelta;
	CPoint m_ptOldPointBeforePan;

	CRect m_rtCanvas;
	CRect m_rtImage;

	float m_fDecodeTime;

	CPixelDataPopupWnd* m_pCursorWnd;

	BOOL m_bIsShowCurrentPosition;

private:
	INT_PTR m_nIndexViewer;
	HWND m_hParentWnd;

public:
	// Init
	void Init();

	// Set
	void SetCompressedFileName(CString strCompressedFileName);
	void SetInImageInfo(IMAGE_INFO stInImageInfo);
	void SetLayerNum(INT_PTR nCurLayer, INT_PTR nTotalLayer);
	void SetCanvas(CRect rtCanvas);
	void SetDecodingTime(float fDecodeTime);
	void SetOldMousePosBeforePan(CPoint ptOldPoint);
	void SetCurImagePos(CPoint ptImgPos);
	void SetShowPixelWnd(BOOL bShow);

	// Get
	CString GetCompressedFileName();
	void* GetInputImageStream();
	CRect GetCanvasRect();
	CRect GetImageRect();
	IMAGE_INFO* GetOutImageInfo();
	void* GetOutImageData();
	INT_PTR GetTotalLayer();
	INT_PTR GetCurrentLayer();
	CPoint GetOldMousePointBeforePan();
	CPoint GetCurImagePos();
	BYTE GetOutImagePixelValue(CPoint ptImagePos);
	WORD GetInImagePixelValue(CPoint ptImagePos);
	double GetZoomRatio();

	BOOL IsPosPopupWndVisible();

	BOOL LoadImageFromStream();
	void UpdateViewer();
	void UpdateCompFileName();
	void UpdateDecodingTime();
	void UpdateLayerNum();
	void UpdateCurMousePosPixelData();
	void UpdateHangOnPopupWndPos(CRect rtBeforeCanvas);

	void ResetPan();
	void ResetZoom();

	void OperatePan(CPoint point);

	void ZoomIn(BOOL bIsDetail);
	void ZoomOut(BOOL bIsDetail);
	
	void ChangeMosPosWndHangOn();
	void HideMosPosWnd();

	// Alloc 
	BOOL AllocInImage();
	BOOL AllocOutImage();

	// Free
	BOOL FreeInImage();
	BOOL FreeOutImage();

protected:
	void CalcImageRect();
	void CalcZoomAndPan();
	void SetOutImageInfo();

private:
	COLORREF GetPixelValueAtMousePos(CPoint ptPixelPos);

	CPoint ConvertScreen2ImageCoordinate(CPoint ptScreen);
	CPoint ConvertImage2ScreenCoordinate(CPoint ptImage);
};

