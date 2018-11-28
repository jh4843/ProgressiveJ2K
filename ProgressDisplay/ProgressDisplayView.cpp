
// ProgressDisplayView.cpp : implementation of the CProgressDisplayView class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "ProgressDisplay.h"
#endif

#include "ProgressDisplayView.h"
#include "DecompressJPEG2000.h"

#include "ImageConverter.h"
#include "Utils.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CProgressDisplayView

IMPLEMENT_DYNCREATE(CProgressDisplayView, CView)

BEGIN_MESSAGE_MAP(CProgressDisplayView, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CProgressDisplayView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_COMMAND(ID_FILE_OPEN, &CProgressDisplayView::OnFileOpen)
ON_WM_DROPFILES()
ON_WM_CREATE()
ON_WM_SIZE()
ON_WM_KEYDOWN()
ON_WM_MOUSEMOVE()
ON_WM_LBUTTONDOWN()
ON_WM_MOUSEWHEEL()
ON_COMMAND(ID_FILE_CLOSEALLIMAGES, &CProgressDisplayView::OnFileCloseallimages)
ON_WM_ERASEBKGND()
ON_COMMAND(ID_VIEW_SEEFIRSTLAYER, &CProgressDisplayView::OnViewSeefirstlayer)
ON_UPDATE_COMMAND_UI(ID_VIEW_SEEFIRSTLAYER, &CProgressDisplayView::OnUpdateViewSeefirstlayer)
ON_COMMAND(ID_VIEW_SHOWCURRENTPOSITION, &CProgressDisplayView::OnViewShowCurrentPosition)
ON_UPDATE_COMMAND_UI(ID_VIEW_SHOWCURRENTPOSITION, &CProgressDisplayView::OnUpdateViewShowCurrentPosition)
ON_WM_SETCURSOR()
ON_WM_LBUTTONDBLCLK()
ON_COMMAND(ID_OPERATION_VIEWEQUALPOSITION, &CProgressDisplayView::OnOperationViewequalposition)
ON_WM_MOVE()
END_MESSAGE_MAP()

// CProgressDisplayView construction/destruction

CProgressDisplayView::CProgressDisplayView()
{
	theApp.m_pProgView = this;
	bFlagIsDecompressing = FALSE;
	m_bIsFirstPreview = FALSE;
	m_bIsShowCurrentPosition = TRUE;

	m_nSelectedViewerIndex = -1;

	m_nOperationMode = MODE_NORMAL;

	m_hBitmap = NULL;

	m_DibInfo.Init();
}

CProgressDisplayView::~CProgressDisplayView()
{
	delete m_pLayoutManager;
}

BOOL CProgressDisplayView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

DIBINFO* CProgressDisplayView::GetDibInfo()
{
	return &m_DibInfo;
}

void CProgressDisplayView::DrawZoomInfo(CDC* pDC, INT_PTR nViewerIndex)
{
	if (!pDC)
		return;

	if (nViewerIndex < 0)
		return;

	INT_PTR nLeftMargin = 3;
	INT_PTR nBottomMargin = 20;
	INT_PTR nSpace = 1;
	COLORREF crTextColor = RGB(170, 90, 0);

	INT_PTR nFontSize = 85;
	CFont font;

	font.CreateFont(12, 0, 0, 0, FW_BOLD, FALSE, FALSE, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");

	CImageViewer* pImageViewer = m_pLayoutManager->GetImageViewer(nViewerIndex);

	if (pImageViewer == nullptr)
		return;

	CRect rtDrawCanvas = pImageViewer->GetCanvasRect();
	double dZoomRatio = pImageViewer->GetZoomRatio();

	CFont* pOldFont = pDC->SelectObject(&font);

	INT_PTR nPosX = rtDrawCanvas.left + nLeftMargin;
	INT_PTR nPosY = rtDrawCanvas.bottom - nBottomMargin;

	CString strZoomRatio;
	strZoomRatio.Format(_T("Zoom Ratio : %.2f (%%)"),dZoomRatio);

	pDC->SetTextColor(crTextColor);
	pDC->TextOut(nPosX, nPosY, strZoomRatio);

	pDC->SelectObject(pOldFont);

	return;
}

BOOL CProgressDisplayView::IsJpeg2000Format(CString strFileName)
{
	BOOL bRes = FALSE;

	CString strFormat;

	strFormat = strFileName.Right(3);

	if (strFormat.CompareNoCase(_T("j2k")) == 0 ||
		strFormat.CompareNoCase(_T("jp2")) == 0)
	{
		bRes = TRUE;
	}

	return bRes;

}

void CProgressDisplayView::RedrawWnd()
{


	Invalidate(FALSE);
}

void CProgressDisplayView::OpenJpegFile(CStringArray* pFilePathArray)
{
	if (bFlagIsDecompressing == TRUE)
		return;

	bFlagIsDecompressing = TRUE;

	CString strInFile;
	CDecompressJPEG2000* pDecompJPEG2000 = new CDecompressJPEG2000();

	clock_t clockStart, clockEnd;

	CString strTimeStamp;

	INT_PTR nImageViewerIndex = -1;
	INT_PTR nFileCount = pFilePathArray->GetCount();
	CImageViewer* pImageViewer = nullptr;
	
	for (INT_PTR i = 0; i < nFileCount; i++)
	{
		strInFile = pFilePathArray->GetAt(i);
		nImageViewerIndex = m_pLayoutManager->AddImageViewer(strInFile);

		if (nImageViewerIndex < 0)
			continue;

		pImageViewer = m_pLayoutManager->GetImageViewer(nImageViewerIndex);

		pImageViewer->SetShowPixelWnd(m_bIsShowCurrentPosition);
		
		pDecompJPEG2000->SetImageViewer(pImageViewer);

		strInFile = pFilePathArray->GetAt(i);

		pDecompJPEG2000->SetInFileName(strInFile);

		CMainFrame* pMainFrm = (CMainFrame*)AfxGetMainWnd();

		m_nSelectedViewerIndex = nImageViewerIndex;

		clockStart = clock();

		pDecompJPEG2000->DoDecompress();

		clockEnd = clock();

		pImageViewer->UpdateCompFileName();

		pImageViewer->SetDecodingTime((float)(clockEnd - clockStart) / 1000);
		pImageViewer->UpdateDecodingTime();
	}

	delete pDecompJPEG2000;
	bFlagIsDecompressing = FALSE;
}

void CProgressDisplayView::ChangeLayer(CString strFilePath, INT_PTR nLayer)
{
	if (bFlagIsDecompressing == TRUE)
		return;

	if (strFilePath.IsEmpty())
		return;

	bFlagIsDecompressing = TRUE;

	CString strOutFile;
	CDecompressJPEG2000* pDecompJPEG2000 = new CDecompressJPEG2000();

	if (m_nSelectedViewerIndex < 0)
		return;

	CImageViewer* pImageViewer = m_pLayoutManager->GetImageViewer(m_nSelectedViewerIndex);

	pDecompJPEG2000->SetImageViewer(pImageViewer);
	
	clock_t clockStart, clockEnd;

	CString strTimeStamp;

	pDecompJPEG2000->SetInFileName(strFilePath);

	CMainFrame* pMainFrm = (CMainFrame*)AfxGetMainWnd();

	clockStart = clock();

	pDecompJPEG2000->DoDecompress(nLayer, nLayer+1);

	clockEnd = clock();

	pImageViewer->UpdateCompFileName();

	pImageViewer->SetDecodingTime((float)(clockEnd - clockStart) / 1000);
	pImageViewer->UpdateDecodingTime();
	pImageViewer->UpdateLayerNum();
	pImageViewer->UpdateCurMousePosPixelData();

	delete pDecompJPEG2000;
	bFlagIsDecompressing = FALSE;
}

void CProgressDisplayView::SetOperationModeByKey(BOOL bShift, BOOL bCtrl, BOOL bAlt, BOOL bLeftDown, BOOL bRightDown)
{
	switch (m_nOperationMode)
	{
	case MODE_NORMAL:
		if (bCtrl)
		{
			m_nOperationMode = MODE_PAN;
		}
		break;
	case MODE_PAN:
		if (bLeftDown)
		{
			m_nOperationMode = MODE_PAN_DOWN;
		}
		break;
	case MODE_PAN_DOWN:
		if (bCtrl && bLeftDown)
		{
			m_nOperationMode = MODE_PAN_DOWN;
		}
		else
		{
			m_nOperationMode = MODE_NORMAL;
		}
		break;
	default:
		m_nOperationMode = MODE_NORMAL;
		break;
	}
}

void CProgressDisplayView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CProgressDisplayView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CProgressDisplayView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CProgressDisplayView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void CProgressDisplayView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CProgressDisplayView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CProgressDisplayView diagnostics

#ifdef _DEBUG
void CProgressDisplayView::AssertValid() const
{
	CView::AssertValid();
}

void CProgressDisplayView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CProgressDisplayDoc* CProgressDisplayView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CProgressDisplayDoc)));
	return (CProgressDisplayDoc*)m_pDocument;
}
#endif //_DEBUG


// CProgressDisplayView message handlers

int CProgressDisplayView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	ChangeWindowMessageFilterEx(GetSafeHwnd(), WM_DROPFILES, MSGFLT_ALLOW, NULL);
	ChangeWindowMessageFilterEx(GetSafeHwnd(), WM_COPYDATA, MSGFLT_ALLOW, NULL);
	ChangeWindowMessageFilterEx(GetSafeHwnd(), WM_COPYGLOBALDATA, MSGFLT_ALLOW, NULL);

	m_pLayoutManager = new CLayoutManager(GetSafeHwnd());
	m_pLayoutManager->SetBorderThick(2);

	DragAcceptFiles(true);

	return 0;
}


void CProgressDisplayView::OnFileOpen()
{
	CFileDialog dlgDcm(TRUE);

	CStringArray aryFilePath;

	TCHAR szFileName[32767];
	szFileName[0] = '\0';

	dlgDcm.GetOFN().lpstrFile = szFileName;
	dlgDcm.GetOFN().nMaxFile = _countof(szFileName);

	if (dlgDcm.DoModal() == IDOK)
	{
		POSITION pos = dlgDcm.GetStartPosition();
		while (pos)
		{
			CString strFileName = dlgDcm.GetNextPathName(pos);
			if (!strFileName.IsEmpty())
			{
				aryFilePath.Add(strFileName);
			}
		}
	}

	OpenJpegFile(&aryFilePath);
	
	// TODO: Add your command handler code here
}

void CProgressDisplayView::OnDraw(CDC* pDC)
{
	CDC dcMem;
	dcMem.CreateCompatibleDC(pDC);

	CProgressDisplayDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	if (!pDC)
	{
		return ;
	}

	CMemDC memDC(*pDC, this);
	pDC = &memDC.GetDC();

	CClientDC dc(this);

	CRect rtCanvas;
	CRect rtImage;

	GetClientRect(&rtCanvas);
	pDC->FillSolidRect(rtCanvas, RGB(255, 255, 255));

	m_pLayoutManager->SetTotalCanvasRect(rtCanvas);

	CImageViewer* pImageViewer = nullptr;
	IMAGE_INFO* infoDrawImage = nullptr;
	void* pDrawImageData = nullptr;

	BITMAPINFO bmi;
	//
	BYTE* pvBits = nullptr;
	ZeroMemory(&bmi, sizeof(BITMAPINFO));
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = (rtCanvas.Width() / 4) * 4;
	bmi.bmiHeader.biHeight = (rtCanvas.Height() / 4) * 4;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 24;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biSizeImage = rtCanvas.Width() * rtCanvas.Height() * 3;

	m_hBitmap = ::CreateDIBSection(dc.GetSafeHdc(), &bmi, DIB_RGB_COLORS, (void**)&pvBits, NULL, 0);

	if (!m_hBitmap)
	{
		AfxDebugBreak();
		return;
	}

	CBitmap* pOldBitmap = dcMem.SelectObject(CBitmap::FromHandle(m_hBitmap));

	Graphics g(pDC->GetSafeHdc());

	CRect rtDrawCanvas;
	
	for (INT_PTR iViewer = 0; iViewer < m_pLayoutManager->GetImageViewerCount(); iViewer++)
	{
		pImageViewer = m_pLayoutManager->GetImageViewer(iViewer);

		if (pImageViewer == nullptr)
			continue;

		rtDrawCanvas = pImageViewer->GetCanvasRect();
		rtImage = pImageViewer->GetImageRect();
		infoDrawImage = pImageViewer->GetOutImageInfo();
		pDrawImageData = pImageViewer->GetOutImageData();

		if (iViewer == m_nSelectedViewerIndex)
		{
			CRect rtTotalCanvas;
			rtTotalCanvas.left = rtDrawCanvas.left - m_pLayoutManager->GetBorderThick();
			rtTotalCanvas.right = rtDrawCanvas.right + m_pLayoutManager->GetBorderThick();
			rtTotalCanvas.top = rtDrawCanvas.top - m_pLayoutManager->GetBorderThick();
			rtTotalCanvas.bottom = rtDrawCanvas.bottom + m_pLayoutManager->GetBorderThick();
			pDC->FillSolidRect(rtTotalCanvas, RGB(150, 200, 230));
		}
			

		if (pDrawImageData == nullptr || infoDrawImage == nullptr)
			continue;

		BITMAPINFOHEADER& bih = GetDibInfo()->bmiHeader;
		bih.biSize = sizeof(BITMAPINFOHEADER);
		bih.biWidth = infoDrawImage->nWidth;
		bih.biHeight = -infoDrawImage->nHeight;
		bih.biPlanes = 1;
		bih.biBitCount = 8 * infoDrawImage->nSamplesPerPixel;
		bih.biCompression = BI_RGB;
		bih.biSizeImage = infoDrawImage->nTotalAllocBytesOnImage;
		bih.biXPelsPerMeter = 0;
		bih.biYPelsPerMeter = 0;
		bih.biClrUsed = 0;
		bih.biClrImportant = 0;

		Bitmap bitmap((BITMAPINFO*)GetDibInfo(), pDrawImageData);

		::SetStretchBltMode(pDC->GetSafeHdc(), COLORONCOLOR);
		::StretchDIBits(pDC->GetSafeHdc(),
			rtDrawCanvas.left,
			rtDrawCanvas.top,
			rtDrawCanvas.Width(),
			rtDrawCanvas.Height(),
			rtImage.left,
			rtImage.top,
			rtImage.Width(),
			rtImage.Height(),
			pDrawImageData,
			(BITMAPINFO*)GetDibInfo(),
			DIB_RGB_COLORS,
			SRCCOPY);

		DrawZoomInfo(pDC, iViewer);
	}

	dcMem.SelectObject(pOldBitmap);
	dcMem.DeleteDC();

	return ;
}


void CProgressDisplayView::OnDropFiles(HDROP hDropInfo)
{
	// TODO: Add your message handler code here and/or call default

	UINT nFileCount = ::DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, 0);
	if (nFileCount <= 0)
	{
		return;
	}

	CStringArray strarrFilePath;
	wchar_t szFileName[MAX_PATH];
	for (UINT nIndex = 0; nIndex < nFileCount; nIndex++)
	{
		::DragQueryFile(hDropInfo, nIndex, szFileName, MAX_PATH);

		// parse folder
		if (::GetFileAttributes(szFileName) & FILE_ATTRIBUTE_DIRECTORY)
		{
			
			CStringList lstFileInFolder;
			//
			CUtils::GetFileListInFolder(&lstFileInFolder, szFileName);
			if (lstFileInFolder.GetCount() > 0)
			{
				for (INT_PTR nSubIndex = 0; nSubIndex < lstFileInFolder.GetCount(); nSubIndex++)
				{
					CString str = lstFileInFolder.GetAt(lstFileInFolder.FindIndex(nSubIndex));

					if (IsJpeg2000Format(str))
						strarrFilePath.Add(str);
				}
			}
		}
		else
		{
			if (IsJpeg2000Format(szFileName))
				strarrFilePath.Add(szFileName);
		}
	}

	::DragFinish(hDropInfo);

	if (!strarrFilePath.GetCount())
	{
		AfxMessageBox(_T("No JPEG2000 Files (.j2k, .jp2)."), MB_ICONERROR);
		return;
	}

	OpenJpegFile(&strarrFilePath);

	CView::OnDropFiles(hDropInfo);
}

void CProgressDisplayView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	CRect rtCanvas;
	GetClientRect(&rtCanvas);

	m_pLayoutManager->SetTotalCanvasRect(rtCanvas);

	// TODO: Add your message handler code here
}

void CProgressDisplayView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (m_pLayoutManager->GetImageViewerCount() <= 0)
		return;

	CImageViewer* pImageViewer = m_pLayoutManager->GetImageViewer(m_nSelectedViewerIndex);

	if (!pImageViewer)
		return;

	INT_PTR nCurrentLayerNum = pImageViewer->GetCurrentLayer();
	INT_PTR nTotalLayerNum = pImageViewer->GetTotalLayer();

	if (nChar == VK_LEFT)
	{
		if (nCurrentLayerNum <= 1)
			return;

		ChangeLayer(pImageViewer->GetCompressedFileName(), --nCurrentLayerNum);
	}
	else if (nChar == VK_RIGHT)
	{
		if (nCurrentLayerNum >= nTotalLayerNum)
			return;

		ChangeLayer(pImageViewer->GetCompressedFileName(), ++nCurrentLayerNum);
	}
	
	if (nChar == VK_CONTROL && m_nOperationMode != MODE_PAN_DOWN)
	{
		m_nOperationMode = MODE_PAN;
		OnSetCursor(NULL, 0, 0);
	}

	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CProgressDisplayView::OnMouseMove(UINT nFlags, CPoint point)
{
	BOOL bIsShiftKeyPressed = (nFlags & MK_SHIFT) ? TRUE : FALSE;
	BOOL bIsCtrlKeyPressed = (nFlags & MK_CONTROL) ? TRUE : FALSE;
	BOOL bIsAltKeyPressed = (nFlags & MK_ALT) ? TRUE : FALSE;
	BOOL bIsLButtonPressed = (nFlags & MK_LBUTTON) ? TRUE : FALSE;
	BOOL bIsRButtonPressed = (nFlags & MK_RBUTTON) ? TRUE : FALSE;

	SetOperationModeByKey(bIsShiftKeyPressed, bIsCtrlKeyPressed, bIsAltKeyPressed, bIsLButtonPressed, bIsRButtonPressed);
	OnSetCursor(NULL, 0, 0);

	if (!m_pLayoutManager)
		return;

	if (!m_bIsShowCurrentPosition)
	{
		
		return;
	}

	switch (m_nOperationMode)
	{
	case MODE_PAN:
		break;
	case MODE_PAN_DOWN:
	{
		INT_PTR nClickIndex = m_pLayoutManager->GetImageViewerIndexFromPos(point);

		if (nClickIndex < 0)
			break;

		if (nClickIndex != m_nSelectedViewerIndex)
			break;

		CImageViewer* pImageViewer = m_pLayoutManager->GetImageViewer(nClickIndex);

		if (!pImageViewer)
			break;

		pImageViewer->OperatePan(point);
		break;
	}

	case MODE_NORMAL:
	default:
		m_pLayoutManager->ManageCurrentPos(point);
		break;
	}

	CView::OnMouseMove(nFlags, point);
}

void CProgressDisplayView::OnLButtonDown(UINT nFlags, CPoint point)
{
	BOOL bIsShiftKeyPressed = (nFlags & MK_SHIFT) ? TRUE : FALSE;
	BOOL bIsCtrlKeyPressed = (nFlags & MK_CONTROL) ? TRUE : FALSE;
	BOOL bIsAltKeyPressed = (nFlags & MK_ALT) ? TRUE : FALSE;
	BOOL bIsLButtonPressed = (nFlags & MK_LBUTTON) ? TRUE : FALSE;
	BOOL bIsRButtonPressed = (nFlags & MK_RBUTTON) ? TRUE : FALSE;

	SetOperationModeByKey(bIsShiftKeyPressed, bIsCtrlKeyPressed, bIsAltKeyPressed, bIsLButtonPressed, bIsRButtonPressed);

	// TODO: Add your message handler code here and/or call default
	if (!m_pLayoutManager)
		return;

	INT_PTR nClickIndex = m_pLayoutManager->GetImageViewerIndexFromPos(point);

	if (nClickIndex < 0)
	{
		return;
	}

	INT_PTR nOldIndex = m_nSelectedViewerIndex;
	m_nSelectedViewerIndex = nClickIndex;

	CImageViewer* pImageViewer = m_pLayoutManager->GetImageViewer(nClickIndex);

	pImageViewer->UpdateCompFileName();
	pImageViewer->UpdateLayerNum();
	pImageViewer->UpdateDecodingTime();

	switch (m_nOperationMode)
	{
	case MODE_PAN:
		break;
	case MODE_PAN_DOWN:
		pImageViewer->SetOldMousePosBeforePan(point);
		break;
	case MODE_NORMAL:
	default:
		if (nOldIndex == m_nSelectedViewerIndex)
		{
			pImageViewer->ChangeMosPosWndHangOn();
		}
		break;
	}

	m_pLayoutManager->RecalcLayout();

	RedrawWindow();

	CView::OnLButtonDown(nFlags, point);
}

BOOL CProgressDisplayView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	if (!m_pLayoutManager)
		return CView::OnMouseWheel(nFlags, zDelta, pt);

	if (m_pLayoutManager->GetImageViewerCount() <= 0)
		return CView::OnMouseWheel(nFlags, zDelta, pt);

	BOOL bIsCtrlKeyPressed = (nFlags & MK_CONTROL) ? TRUE : FALSE;
	BOOL bIsAltKeyPressed = (nFlags & MK_ALT) ? TRUE : FALSE;

	if (bIsCtrlKeyPressed)
	{
		CImageViewer* pImageViewer = m_pLayoutManager->GetImageViewer(m_nSelectedViewerIndex);

		if (zDelta > 0)
		{
			pImageViewer->ZoomIn(bIsAltKeyPressed);
		}
		else if (zDelta < 0)
		{
			pImageViewer->ZoomOut(bIsAltKeyPressed);
		}
	}

	return CView::OnMouseWheel(nFlags, zDelta, pt);
}

void CProgressDisplayView::OnFileCloseallimages()
{
	m_nSelectedViewerIndex = -1;

	m_pLayoutManager->CloseAllImageViewer();

	RedrawWindow();
	// TODO: Add your command handler code here
}

BOOL CProgressDisplayView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	//return CView::OnEraseBkgnd(pDC);
	return FALSE;
}

void CProgressDisplayView::OnViewSeefirstlayer()
{
	if (m_bIsFirstPreview)
	{
		m_bIsFirstPreview = FALSE;
	}
	else
	{
		m_bIsFirstPreview = TRUE;
	}
}


void CProgressDisplayView::OnUpdateViewSeefirstlayer(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(TRUE);

	if (m_bIsFirstPreview)
	{
		pCmdUI->SetCheck(TRUE);
	}
	else
	{
		pCmdUI->SetCheck(FALSE);
	}
}

void CProgressDisplayView::OnViewShowCurrentPosition()
{
	if (m_bIsShowCurrentPosition)
	{
		m_bIsShowCurrentPosition = FALSE;
	}
	else
	{
		m_bIsShowCurrentPosition = TRUE;
	}

	CImageViewer* pImageViewer = nullptr;
	for (INT_PTR iViewer = 0; iViewer < m_pLayoutManager->GetImageViewerCount(); iViewer++)
	{
		pImageViewer = m_pLayoutManager->GetImageViewer(iViewer);
		pImageViewer->SetShowPixelWnd(m_bIsShowCurrentPosition);
	}
}

void CProgressDisplayView::OnUpdateViewShowCurrentPosition(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(TRUE);

	if (m_bIsShowCurrentPosition)
	{
		pCmdUI->SetCheck(TRUE);
	}
	else
	{
		pCmdUI->SetCheck(FALSE);
	}
}

BOOL CProgressDisplayView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	// TODO: Add your message handler code here and/or call default

	switch (m_nOperationMode)
	{
	case MODE_PAN:
		SetCursor((HCURSOR)AfxGetApp()->LoadCursor(IDC_CURSOR_PAN));
		return TRUE;
	case MODE_PAN_DOWN:
		SetCursor((HCURSOR)AfxGetApp()->LoadCursor(IDC_CURSOR_PAN_DOWN));
		return TRUE;
	}

	return CView::OnSetCursor(pWnd, nHitTest, message);
}


void CProgressDisplayView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	if (!m_pLayoutManager)
		return;

	if (m_pLayoutManager->GetImageViewerCount() <= 0)
		return;

	if (m_nSelectedViewerIndex < 0)
		return;

	CImageViewer* pImageViewer = m_pLayoutManager->GetImageViewer(m_nSelectedViewerIndex);

	pImageViewer->ResetZoom();
	pImageViewer->ResetPan();
	
	m_pLayoutManager->RecalcLayout();

	CView::OnLButtonDblClk(nFlags, point);
}


void CProgressDisplayView::OnOperationViewequalposition()
{
	if (!m_pLayoutManager)
		return;

	if (m_pLayoutManager->GetImageViewerCount() <= 0)
		return;

	if (m_nSelectedViewerIndex < 0)
		return;

	m_pLayoutManager->SetEqualImagePos(m_nSelectedViewerIndex);
	// TODO: Add your command handler code here
}


void CProgressDisplayView::OnMove(int x, int y)
{
	CView::OnMove(x, y);

	CRect rtCanvas;
	GetClientRect(&rtCanvas);

	m_pLayoutManager->SetTotalCanvasRect(rtCanvas);

	// TODO: Add your message handler code here
}
