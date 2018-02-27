#pragma once
#include "ImageInfo.h"

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

	CRect m_rtCanvas;
	CRect m_rtImage;

	float m_fDecodeTime;

private:
	INT_PTR m_nIndexViewer;
	HWND m_hWndView;

public:
	// Init
	void Init();

	// Set
	void SetCompressedFileName(CString strCompressedFileName);
	void SetInImageInfo(IMAGE_INFO stInImageInfo);
	void SetLayerNum(INT_PTR nCurLayer, INT_PTR nTotalLayer);
	void SetCanvas(CRect rtCanvas);
	void SetDecodingTime(float fDecodeTime);

	// Get
	CString GetCompressedFileName();
	void* GetInputImageStream();
	CRect GetCanvasRect();
	CRect GetImageRect();
	IMAGE_INFO* GetOutImageInfo();
	void* GetOutImageData();
	INT_PTR GetTotalLayer();
	INT_PTR GetCurrentLayer();

	BOOL LoadImageFromStream();
	void UpdateViewer();
	void UpdateCompFileName();
	void UpdateDecodingTime();
	void UpdateLayerNum();

	// Alloc 
	BOOL AllocInImage();
	BOOL AllocOutImage();

	// Free
	BOOL FreeInImage();
	BOOL FreeOutImage();

protected:
	void CalcImageRect();
	void SetOutImageInfo();
};

