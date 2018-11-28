#include "stdafx.h"
#include "LayoutManager.h"
#include "math.h"

CLayoutManager::CLayoutManager(HWND hParentWnd)
{
	m_hViewWnd = hParentWnd;
	m_aryImageViewer.RemoveAll();
}


CLayoutManager::~CLayoutManager()
{
}

INT_PTR CLayoutManager::AddImageViewer(CString strInFileName)
{
	INT_PTR nAddViewerIndex = m_aryImageViewer.GetCount();

	if (nAddViewerIndex < 0)
		return -1;
	if (nAddViewerIndex >= MAX_VIEWER_COUNT)
		return -1;

	CImageViewer* pImageViewer = new CImageViewer(nAddViewerIndex, m_hViewWnd);
	if (!pImageViewer)
	{
		return -1;
	}

	pImageViewer->SetCompressedFileName(strInFileName);

	m_aryImageViewer.Add(pImageViewer);
	RecalcLayout();

	return nAddViewerIndex;
}

void CLayoutManager::FreeImageViewer()
{
	for (INT_PTR nIndex = 0; nIndex < m_aryImageViewer.GetCount(); nIndex++)
	{
		CImageViewer* pImageViewer = m_aryImageViewer.GetAt(nIndex);
		if (pImageViewer)
		{
			delete pImageViewer;
			pImageViewer = nullptr;
		}
	}

	m_aryImageViewer.RemoveAll();
}

void CLayoutManager::Init()
{
	CloseAllImageViewer();

	m_rtTotalCanvas.SetRectEmpty();
}

void CLayoutManager::CloseAllImageViewer()
{
	FreeImageViewer();
}

CImageViewer* CLayoutManager::GetImageViewer(INT_PTR nIndex)
{
	if (m_aryImageViewer.GetCount() <= 0)
		return nullptr;

	return m_aryImageViewer.GetAt(nIndex);
}

INT_PTR CLayoutManager::GetImageViewerCount()
{
	return m_aryImageViewer.GetCount();
}

INT_PTR CLayoutManager::GetTotalViewerCount()
{
	INT_PTR nImageViewerCount = m_aryImageViewer.GetCount();
	INT_PTR nRes = 0;

	for (INT_PTR iView = 0; iView*iView < MAX_VIEWER_COUNT; iView++)
	{
		if (nImageViewerCount <= (iView*iView))
		{
			nRes = iView*iView;
			break;
		}
	}

	return nRes;
}

INT_PTR CLayoutManager::GetBorderThick()
{
	return m_nBorderThick;
}

INT_PTR CLayoutManager::GetImageViewerIndexFromPos(CPoint ptPos)
{
	INT_PTR nRes = -1;
	CRect rtViewerCanvas;
	rtViewerCanvas.SetRectEmpty();

	for (INT_PTR iView = 0; iView < m_aryImageViewer.GetCount(); iView++)
	{
		rtViewerCanvas = m_aryImageViewer.GetAt(iView)->GetCanvasRect();

		if (rtViewerCanvas.PtInRect(ptPos) == TRUE)
		{
			nRes = iView;
			break;
		}
	}

	return nRes;
}

void CLayoutManager::SetTotalCanvasRect(CRect rtTotalCanvas)
{
	m_rtTotalCanvas = rtTotalCanvas;

	RecalcLayout();
}

void CLayoutManager::SetImageViewerCanvas(INT_PTR nIndexImageViewer, CRect rtCanvas)
{
	CImageViewer* pImageViewer = m_aryImageViewer.GetAt(nIndexImageViewer);
	pImageViewer->SetCanvas(rtCanvas);
}

void CLayoutManager::SetBorderThick(INT_PTR nThick)
{
	m_nBorderThick = nThick;
}

void CLayoutManager::SetEqualImagePos(INT_PTR nIndexStandardViewer)
{
	if (GetImageViewerCount() < 0)
		return;

	if (nIndexStandardViewer < 0)
		return;

	CPoint ptImagePos = GetImageViewer(nIndexStandardViewer)->GetCurImagePos();

	for (INT_PTR nIndex = 0; nIndex < GetImageViewerCount(); nIndex++)
	{
		GetImageViewer(nIndex)->SetCurImagePos(ptImagePos);
	}

}

void CLayoutManager::ManageCurrentPos(CPoint ptCurPos)
{
	if (GetImageViewerCount() <= 0)
		return;

	INT_PTR nClickIndex = GetImageViewerIndexFromPos(ptCurPos);

	if (nClickIndex < 0)
		return;

	for (INT_PTR iView = 0; iView < GetImageViewerCount(); iView++)
	{
		if (iView == nClickIndex)
		{
			GetImageViewer(iView)->UpdateCurMousePosPixelData();
		}
		else
		{
			GetImageViewer(iView)->HideMosPosWnd();
		}
	}
}

void CLayoutManager::DestroyCurrentPosWnd()
{
	if (GetImageViewerCount() <= 0)
		return;

	for (INT_PTR iView = 0; iView < GetImageViewerCount(); iView++)
	{
		GetImageViewer(iView)->HideMosPosWnd();
	}
}

void CLayoutManager::RecalcLayout()
{
	if (m_rtTotalCanvas.IsRectEmpty())
		return;

	INT_PTR nImageViewerCount = m_aryImageViewer.GetCount();

	if (nImageViewerCount < 0)
	{
		return;
	}

	INT_PTR nTotalViewerCount = GetTotalViewerCount();

	if (nTotalViewerCount <= 0)
		return;

	INT_PTR nSideCount = (INT_PTR)sqrt(nTotalViewerCount);
	INT_PTR nBorderCount = nSideCount + 1;
	
	if (nSideCount <= 0)
		return;
	
	CRect rtViewer;
	LONG lViewerWidth = (m_rtTotalCanvas.Width() / nSideCount) - (m_nBorderThick * nBorderCount);
	LONG lViewerHeight = (m_rtTotalCanvas.Height() / nSideCount) - (m_nBorderThick * nBorderCount);
	LONG lColPos = 0.0;
	LONG lRowPos = 0.0;
	
	INT_PTR iCol = 0;
	INT_PTR iRow = 0;

	for (INT_PTR iViewer = 0; iViewer < nImageViewerCount; iViewer++)
	{
		rtViewer.SetRectEmpty();

		lColPos = m_rtTotalCanvas.left + iCol*lViewerWidth + (iCol+1)*m_nBorderThick;
		lRowPos = m_rtTotalCanvas.top + iRow*lViewerHeight + (iRow+1)*m_nBorderThick;

		rtViewer.left = lColPos;
		rtViewer.right = lColPos + lViewerWidth;
		rtViewer.top = lRowPos;
		rtViewer.bottom = lRowPos + lViewerHeight;
		
		SetImageViewerCanvas(iViewer, rtViewer);
		GetImageViewer(iViewer)->UpdateCurMousePosPixelData();

		iCol++;

		if (iCol == nSideCount)
		{
			iCol = 0;
			iRow++;
		}
	}

}