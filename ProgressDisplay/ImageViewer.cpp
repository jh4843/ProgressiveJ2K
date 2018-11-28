#include "stdafx.h"
#include "ImageViewer.h"
#include "MainFrm.h"
#include "ImageConverter.h"

#include "DecompressJPEG2000.h"

CImageViewer::CImageViewer(INT_PTR nIndexViewer, HWND hWndView)
{
	Init();

	m_nIndexViewer = nIndexViewer;
	m_hParentWnd = hWndView;

	CWnd* pWnd = CWnd::FromHandle(m_hParentWnd);
	m_pCursorWnd = new CPixelDataPopupWnd();
	m_pCursorWnd->CreateCtrl(pWnd);

	m_fDecodeTime = 0.0;
	m_bIsShowCurrentPosition = FALSE;
}


CImageViewer::~CImageViewer()
{
	Init();

	if (m_pCursorWnd)
		delete m_pCursorWnd;
}

void CImageViewer::Init()
{
	FreeInImage();
	FreeOutImage();

	m_strCompressedFileName.Empty();
	
	m_stOutImageInfo.init();
	m_stInImageInfo.init();

	m_nCurrentLayerNum = 0;
	m_nTotalLayerNum = 0;

	m_dCanvasPerImageRatio = 1.0;
	m_dZoomValue = 1.0;

	m_ptPanDelta = CPoint(0, 0);
	m_ptOldPointBeforePan = CPoint(0, 0);

	m_rtCanvas.SetRectEmpty();
	m_rtImage.SetRectEmpty();
}

void CImageViewer::SetCompressedFileName(CString strCompressedFileName)
{
	m_strCompressedFileName = strCompressedFileName;
}

void CImageViewer::SetInImageInfo(IMAGE_INFO stInImageInfo)
{
	m_stInImageInfo = stInImageInfo;

	SetOutImageInfo();
}

void CImageViewer::SetLayerNum(INT_PTR nCurLayer, INT_PTR nTotalLayer)
{
	CMainFrame* pMainFrm = (CMainFrame*)AfxGetMainWnd();

	m_nCurrentLayerNum = nCurLayer;
	m_nTotalLayerNum = nTotalLayer;

	CString strLayer;
	strLayer.Format(_T("%d - %d"), m_nCurrentLayerNum, m_nTotalLayerNum);

	pMainFrm->SetStatusBarText(3, strLayer);
}

void CImageViewer::SetCanvas(CRect rtCanvas)
{
	m_rtCanvas = rtCanvas;

	CalcImageRect();
	
}

void CImageViewer::SetDecodingTime(float fDecodeTime)
{
	m_fDecodeTime = fDecodeTime;
}

void CImageViewer::SetCurImagePos(CPoint ptImgPos)
{
	m_ptPixelDataPos = ptImgPos;
	m_pCursorWnd->SetHangOn(TRUE);

	UpdateViewer();
}

void CImageViewer::SetShowPixelWnd(BOOL bShow)
{
	m_bIsShowCurrentPosition = bShow;

	if (!bShow)
	{
		if (m_pCursorWnd)
		{
			m_pCursorWnd->HidePopupMsg();
		}
	}
}

void CImageViewer::SetOldMousePosBeforePan(CPoint ptOldPoint)
{
	m_ptOldPointBeforePan = ptOldPoint;
}

CString CImageViewer::GetCompressedFileName()
{
	return m_strCompressedFileName;
}

void* CImageViewer::GetInputImageStream()
{
	return m_pInImage;
}

CRect CImageViewer::GetCanvasRect()
{
	return m_rtCanvas;
}

CRect CImageViewer::GetImageRect()
{
	return m_rtImage;
}

IMAGE_INFO* CImageViewer::GetOutImageInfo()
{
	return &m_stOutImageInfo;
}

void* CImageViewer::GetOutImageData()
{
	return m_pOutImage;
}

INT_PTR CImageViewer::GetTotalLayer()
{
	return m_nTotalLayerNum;
}

INT_PTR CImageViewer::GetCurrentLayer()
{
	return m_nCurrentLayerNum;
}

CPoint CImageViewer::GetOldMousePointBeforePan()
{
	return m_ptOldPointBeforePan;
}

CPoint CImageViewer::GetCurImagePos()
{
	return m_ptPixelDataPos;
}

BYTE CImageViewer::GetOutImagePixelValue(CPoint ptImagePos)
{
	if (m_stOutImageInfo.nWidth == 0 ||
		m_stOutImageInfo.nHeight == 0)
		return -1;

	if (!m_pOutImage)
		return -1;

	INT_PTR nRow = m_stOutImageInfo.nHeight - ptImagePos.y;
	INT_PTR nCol = ptImagePos.x;

	if (nRow < 0 || nCol < 0)
		return -1;

	if (nRow >= m_stOutImageInfo.nHeight || nCol >= m_stOutImageInfo.nWidth)
		return -1;

	BYTE bRes;
	memset(&bRes, 0, sizeof(BYTE));

	BYTE* pByte = (BYTE*)m_pOutImage;
	pByte += nRow*m_stOutImageInfo.nWidth + nCol;
	bRes = (BYTE)*pByte;

	return bRes;
}

WORD CImageViewer::GetInImagePixelValue(CPoint ptImagePos)
{
	WORD wRes;
	memset(&wRes, 0, sizeof(WORD));

	if (m_stInImageInfo.nWidth == 0 ||
		m_stInImageInfo.nHeight == 0)
		return -1;

	if (!m_pInImage)
		return -1;

	INT_PTR nRow = m_stInImageInfo.nHeight - ptImagePos.y;
	INT_PTR nCol = ptImagePos.x;

	if (nRow < 0 || nCol < 0)
		return -1;

	if (nRow >= m_stInImageInfo.nHeight || nCol >= m_stInImageInfo.nWidth)
		return -1;

	BYTE* pByte = nullptr;
	WORD* pWord = nullptr;

	switch (m_stInImageInfo.nTotalAllocBytesPerPixel)
	{
	case 1:
		pByte = (BYTE*)m_pInImage;
		pByte += nRow*m_stInImageInfo.nWidth + nCol;
		wRes = (BYTE)*pByte;
		break;
	case 2:
		pWord = (WORD*)m_pInImage;
		pWord += nRow*m_stInImageInfo.nWidth + nCol;
		wRes = (WORD)*pWord;
		break;
	default:
		break;
	}

	return wRes;
}

double CImageViewer::GetZoomRatio()
{
	return m_dZoomValue;
}

BOOL CImageViewer::IsPosPopupWndVisible()
{
	BOOL bRes = FALSE;
	if (!m_pCursorWnd)
		return FALSE;

	if (m_pCursorWnd->IsWindowEnabled() && m_pCursorWnd->IsWindowVisible())
		return TRUE;

	return bRes;
}

BOOL CImageViewer::LoadImageFromStream()
{
	BOOL bRes = FALSE;

	bRes = AllocOutImage();
	if (!bRes)
		return FALSE;

	CImageConverter imageConverter;
	imageConverter.SetInImageInfo(&m_stInImageInfo);
	imageConverter.SetOutImageInfo(&m_stOutImageInfo);

	imageConverter.AdjustImage(m_pInImage, m_pOutImage, 0);

	//FreeInImage();	// To Display Original Image Value

	return bRes;
}

void CImageViewer::UpdateViewer()
{
	if (m_hParentWnd == nullptr)
		return;

	UpdateCurMousePosPixelData();

	CWnd* pWnd = CWnd::FromHandle(m_hParentWnd);

	if (!pWnd)
		return;

	pWnd->InvalidateRect(m_rtCanvas, FALSE);
	pWnd->RedrawWindow();
}

void CImageViewer::UpdateCompFileName()
{
	CMainFrame* pMainFrm = (CMainFrame*)AfxGetMainWnd();
	pMainFrm->SetStatusBarText(1, m_strCompressedFileName);
}

void CImageViewer::UpdateDecodingTime()
{
	CMainFrame* pMainFrm = (CMainFrame*)AfxGetMainWnd();
	CString strTimeStamp;

	strTimeStamp.Format(_T("Decoding time : %0.2f sec"), m_fDecodeTime);
	pMainFrm->SetStatusBarText(2, strTimeStamp);
}

void CImageViewer::UpdateLayerNum()
{
	CMainFrame* pMainFrm = (CMainFrame*)AfxGetMainWnd();

	CString strLayer;
	strLayer.Format(_T("%d - %d"), m_nCurrentLayerNum, m_nTotalLayerNum);

	pMainFrm->SetStatusBarText(3, strLayer);
}

void CImageViewer::UpdateCurMousePosPixelData()
{
	CPoint ptClientPos;
	CPoint ptScreenPos;
	
	if (!m_pCursorWnd->GetHangOn())
	{
		::GetCursorPos(&ptClientPos);
		::ScreenToClient(m_hParentWnd, &ptClientPos);
		m_ptPixelDataPos = ConvertScreen2ImageCoordinate(ptClientPos);
	}
	else
	{
		ptClientPos = ConvertImage2ScreenCoordinate(m_ptPixelDataPos);
	}

	if (!m_bIsShowCurrentPosition)
	{
		m_pCursorWnd->HidePopupMsg();
		return;
	}

	if (!m_rtCanvas.PtInRect(ptClientPos))
	{
		m_pCursorWnd->HidePopupMsg();
		return;
	}

	ptScreenPos = ptClientPos;
	::ClientToScreen(m_hParentWnd, &ptScreenPos);

	m_pCursorWnd->SetCursorImagePos(m_ptPixelDataPos);
	m_pCursorWnd->SetPopupPos(ptScreenPos);
	m_pCursorWnd->SetOutImagePixelData(GetOutImagePixelValue(m_ptPixelDataPos));
	m_pCursorWnd->SetInImagePixelData(GetInImagePixelValue(m_ptPixelDataPos));
	m_pCursorWnd->SetDisplayPixelData(GetPixelValueAtMousePos(ptClientPos));
	m_pCursorWnd->CompositeMsg();
	m_pCursorWnd->ShowPopupMsg();
}

void CImageViewer::UpdateHangOnPopupWndPos(CRect rtBeforeCanvas)
{
	if (!IsPosPopupWndVisible())
		return;

	if (!m_pCursorWnd->GetHangOn())
		return;

	double dWidthRatio = m_rtCanvas.Width() / rtBeforeCanvas.Width();
	double dHeightRatio = m_rtCanvas.Height() / rtBeforeCanvas.Height();

	m_ptPixelDataPos.x = m_ptPixelDataPos.x * dWidthRatio;
	m_ptPixelDataPos.y = m_ptPixelDataPos.y * dHeightRatio;

	m_pCursorWnd->SetCursorImagePos(ConvertScreen2ImageCoordinate(m_ptPixelDataPos));
	m_pCursorWnd->SetDisplayPixelData(GetPixelValueAtMousePos(m_ptPixelDataPos));
	m_pCursorWnd->CompositeMsg();
	m_pCursorWnd->ShowPopupMsg();
}

void CImageViewer::ResetPan()
{
	m_ptOldPointBeforePan = CPoint(0, 0);
	m_ptPanDelta = CPoint(0, 0);
}

void CImageViewer::ResetZoom()
{
	m_dZoomValue = 1.0;
}

void CImageViewer::OperatePan(CPoint point)
{
	CPoint ptDelta;
	ptDelta = point - m_ptOldPointBeforePan;

	ptDelta.x = (int)((double)ptDelta.x / m_dCanvasPerImageRatio*m_dZoomValue + 0.5);
	ptDelta.y = (int)((double)ptDelta.y / m_dCanvasPerImageRatio*m_dZoomValue + 0.5);
	m_ptPanDelta += ptDelta;

	SetOldMousePosBeforePan(point);

	CalcImageRect();
	UpdateViewer();
	
}

void CImageViewer::ZoomIn(BOOL bIsDetail)
{
	double dRatio = 0.01;
	
	if (bIsDetail)
		m_dZoomValue += dRatio * 5.0;	// 5%
	else
		m_dZoomValue += dRatio * 100.0;	// 100%
	
	if (m_dZoomValue < MIN_ZOOM_RATIO)
	{
		m_dZoomValue = MIN_ZOOM_RATIO;
	}
	else if (m_dZoomValue > MAX_ZOOM_RATIO)
	{
		m_dZoomValue = MAX_ZOOM_RATIO;
	}

	CalcImageRect();
	UpdateViewer();
}

void CImageViewer::ZoomOut(BOOL bIsDetail)
{
	double dRatio = 0.01;

	if (bIsDetail)
		m_dZoomValue -= dRatio * 5.0;	// 5%
	else
		m_dZoomValue -= dRatio * 30.0;	// 30%

	if (m_dZoomValue < MIN_ZOOM_RATIO)
	{
		m_dZoomValue = MIN_ZOOM_RATIO;
	}
	else if (m_dZoomValue > MAX_ZOOM_RATIO)
	{
		m_dZoomValue = MAX_ZOOM_RATIO;
	}

	CalcImageRect();
	UpdateViewer();
}

void CImageViewer::ChangeMosPosWndHangOn()
{
	if (!m_pCursorWnd)
		return;

	m_pCursorWnd->SetHangOn(!m_pCursorWnd->GetHangOn());
}

void CImageViewer::HideMosPosWnd()
{
	if (!m_pCursorWnd)
		return;

	if (m_pCursorWnd->GetHangOn())
		return;

	m_pCursorWnd->HidePopupMsg();
}

BOOL CImageViewer::AllocInImage()
{
	FreeInImage();

	int nAllocMemSize = m_stInImageInfo.nWidth * m_stInImageInfo.nHeight * m_stInImageInfo.nTotalAllocBytesPerPixel;
	m_pInImage = (BYTE*)::VirtualAlloc(NULL, nAllocMemSize, MEM_COMMIT | MEM_RESERVE | MEM_TOP_DOWN, PAGE_READWRITE);
	if (!m_pInImage)
	{
		AfxThrowMemoryException();
		return FALSE;
	}
	else
	{
		m_stInImageInfo.nTotalAllocBytesOnImage = nAllocMemSize;
	}

	return TRUE;
}

BOOL CImageViewer::AllocOutImage()
{
	FreeOutImage();

	int nAllocMemSize = m_stOutImageInfo.nWidth * m_stOutImageInfo.nHeight * m_stOutImageInfo.nTotalAllocBytesPerPixel;
	m_pOutImage = (BYTE*)::VirtualAlloc(NULL, nAllocMemSize, MEM_COMMIT | MEM_RESERVE | MEM_TOP_DOWN, PAGE_READWRITE);
	if (!m_pOutImage)
	{
		AfxThrowMemoryException();
		return FALSE;
	}
	else
	{
		m_stOutImageInfo.nTotalAllocBytesOnImage = nAllocMemSize;
	}

	return TRUE;
}

BOOL CImageViewer::FreeInImage()
{
	if (m_pInImage)
	{
		::VirtualFree(m_pInImage, m_stInImageInfo.nTotalAllocBytesOnImage, MEM_DECOMMIT);
		::VirtualFree(m_pInImage, 0, MEM_RELEASE);
		// 		delete[] m_pImageData;
		m_pInImage = nullptr;
	}

	return TRUE;
}

BOOL CImageViewer::FreeOutImage()
{
	if (m_pOutImage)
	{
		::VirtualFree(m_pOutImage, m_stOutImageInfo.nTotalAllocBytesOnImage, MEM_DECOMMIT);
		::VirtualFree(m_pOutImage, 0, MEM_RELEASE);
		// 		delete[] m_pImageData;
		m_pOutImage = nullptr;
	}

	return TRUE;
}

void CImageViewer::CalcImageRect()
{
	if (m_stOutImageInfo.nWidth == 0 ||
		m_stOutImageInfo.nHeight == 0)
		return;

	if (m_rtCanvas.IsRectEmpty())
		return;
	
	double dCanvasWidth = m_rtCanvas.Width();
	double dCanvasHeight = m_rtCanvas.Height();

	double dImgWidth = m_stOutImageInfo.nWidth;
	double dImgHeight = m_stOutImageInfo.nHeight;

	if (dCanvasWidth == 0.0 || dCanvasHeight == 0.0)
		return;

	if (dImgWidth == 0.0 || dImgHeight == 0.0)
		return;

	double dRatioCanvas = 0.0;
	double dExtendedLength = 0.0;

	if (dCanvasWidth >= dCanvasHeight)
	{
		dRatioCanvas = dCanvasWidth / dCanvasHeight;
		dExtendedLength = dImgHeight * dRatioCanvas;

		if (dExtendedLength >= dImgWidth)
		{
			m_rtImage.left = (int)(dImgWidth / 2) - (int)(dExtendedLength / 2);
			m_rtImage.top = 0;
			m_rtImage.right = m_rtImage.left + (int)(dExtendedLength + 0.5f);
			m_rtImage.bottom = (int)dImgHeight;

			CalcZoomAndPan();
			m_dCanvasPerImageRatio = dCanvasHeight / (float)m_rtImage.Height();
		}
		else
		{
			dRatioCanvas = dCanvasHeight / dCanvasWidth;
			dExtendedLength = dImgWidth * dRatioCanvas;

			m_rtImage.left = 0;
			m_rtImage.top = (int)(dImgHeight / 2) - (int)(dExtendedLength / 2);
			m_rtImage.right = (int)dImgWidth;
			m_rtImage.bottom = m_rtImage.top + (int)(dExtendedLength + 0.5f);

			CalcZoomAndPan();
			m_dCanvasPerImageRatio = dCanvasWidth / (float)m_rtImage.Width();
		}
	}
	else
	{
		dRatioCanvas = dCanvasHeight / dCanvasWidth;
		dExtendedLength = dImgWidth * dRatioCanvas;

		if (dExtendedLength >= dImgHeight)
		{
			m_rtImage.left = 0;
			m_rtImage.top = (int)(dImgHeight / 2) - (int)(dExtendedLength / 2);
			m_rtImage.right = (int)dImgWidth;
			m_rtImage.bottom = m_rtImage.top + (int)(dExtendedLength + 0.5f);

			CalcZoomAndPan();
			m_dCanvasPerImageRatio = dCanvasWidth / (float)m_rtImage.Width();
		}
		else
		{
			dRatioCanvas = dCanvasWidth / dCanvasHeight;
			dExtendedLength = dImgHeight * dRatioCanvas;

			m_rtImage.left = (int)(dImgWidth / 2) - (int)(dExtendedLength / 2);
			m_rtImage.top = 0;
			m_rtImage.right = m_rtImage.left + (int)(dExtendedLength + 0.5f);
			m_rtImage.bottom = (int)dImgHeight;

			CalcZoomAndPan();
			m_dCanvasPerImageRatio = dCanvasHeight / (float)m_rtImage.Height();
		}
	}

	if (m_pCursorWnd)
	{
		CWnd* pWnd = CWnd::FromHandle(m_hParentWnd);

		CRect rtParent;
		pWnd->GetClientRect(rtParent);

		m_pCursorWnd->SetParentRect(rtParent);
		m_pCursorWnd->SetCanvasRect(m_rtCanvas);
		m_pCursorWnd->SetImageRect(m_rtImage);
	}
		
}

void CImageViewer::CalcZoomAndPan()
{
	// zoom
	if (m_dZoomValue < MIN_ZOOM_RATIO)
	{
		m_dZoomValue = MIN_ZOOM_RATIO;
	}
	else if (m_dZoomValue > MAX_ZOOM_RATIO)
	{
		m_dZoomValue = MAX_ZOOM_RATIO;
	}

	//
	CPoint ptCenter = CPoint((int)((m_rtImage.right + m_rtImage.left)*0.5f), (int)((m_rtImage.bottom + m_rtImage.top)*0.5f));
	int nConst = (int)(m_rtImage.Width()*0.5f / m_dZoomValue);
	int nConst1 = (int)(m_rtImage.Width() / m_dZoomValue);
	m_rtImage.left = ptCenter.x - nConst;
	m_rtImage.right = m_rtImage.left + nConst1;
	nConst = (int)(m_rtImage.Height()*0.5f / m_dZoomValue);
	nConst1 = (int)(m_rtImage.Height() / m_dZoomValue);
	m_rtImage.top = ptCenter.y - nConst;
	m_rtImage.bottom = m_rtImage.top + nConst1;

	// panning
	double dPannedDeltaX = -1 * (double)m_ptPanDelta.x / m_dZoomValue;
	double dPannedDeltaY = (double)m_ptPanDelta.y / m_dZoomValue;
	m_rtImage.OffsetRect((int)(dPannedDeltaX + 0.5f), (int)(dPannedDeltaY + 0.5f));
}

void CImageViewer::SetOutImageInfo()
{
	m_stOutImageInfo.init();

	m_stOutImageInfo.nWidth = m_stInImageInfo.nWidth;
	m_stOutImageInfo.nHeight = m_stInImageInfo.nHeight;
	m_stOutImageInfo.nBitsPerPixel = 8;
	m_stOutImageInfo.nBytesPerPixel = 1;
	m_stOutImageInfo.nSamplesPerPixel = m_stInImageInfo.nSamplesPerPixel;

	m_stOutImageInfo.nTotalAllocBytesPerPixel = m_stInImageInfo.nSamplesPerPixel * m_stInImageInfo.nBytesPerPixel; // SamplesPerPixel 확인필요.
	m_stOutImageInfo.nBytesPerLine = m_stInImageInfo.nWidth * m_stInImageInfo.nTotalAllocBytesPerPixel;

	m_stOutImageInfo.nW1 = m_stInImageInfo.nW1;
	m_stOutImageInfo.nW2 = m_stInImageInfo.nW2;

	CalcImageRect();

}

COLORREF CImageViewer::GetPixelValueAtMousePos(CPoint ptPixelPos)
{
	CWnd* pWnd = CWnd::FromHandle(m_hParentWnd);

	if (!pWnd)
		return 0;

	CDC *pDCc = pWnd->GetDC();

	COLORREF color = pDCc->GetPixel(ptPixelPos);

	return color;
}

CPoint CImageViewer::ConvertScreen2ImageCoordinate(CPoint point)
{
	CPoint ptCanvas = CPoint(point.x - m_rtCanvas.left, point.y - m_rtCanvas.top);
	CPoint ptImage = CPoint(0, 0);

	CRect rtClient;
	rtClient.left = 0;
	rtClient.top = 0;
	rtClient.right = rtClient.left + m_rtCanvas.Width();
	rtClient.bottom = rtClient.top + m_rtCanvas.Height();

	float fRatioX = (double)m_rtImage.Width() / (float)rtClient.Width();
	float fRatioY = (double)m_rtImage.Height() / (float)rtClient.Height();
	ptImage.x = (ptCanvas.x - rtClient.left)*fRatioX + m_rtImage.left;
	ptImage.y = m_rtImage.Height() - (ptCanvas.y - rtClient.top)*fRatioY + m_rtImage.top;

	//
	return ptImage;
}

CPoint CImageViewer::ConvertImage2ScreenCoordinate(CPoint ptImage)
{
	CPoint ptCanvas = CPoint(0,0);

	//
	float fRatioX = (double)m_rtCanvas.Width() / (float)m_rtImage.Width();
	float fRatioY = (double)m_rtCanvas.Height() / (float)m_rtImage.Height();
	ptCanvas.x = (ptImage.x - m_rtImage.left)*fRatioX + m_rtCanvas.left;
	ptCanvas.y = m_rtCanvas.Height() - (ptImage.y - m_rtImage.top)*fRatioY + m_rtCanvas.top;
	//ptCanvas.y = (ptImage.y - m_rtImage.top)*fRatioY + m_rtCanvas.top;

	//
	return ptCanvas;
}