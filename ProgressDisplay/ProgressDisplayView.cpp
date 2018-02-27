
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
ON_WM_LBUTTONDOWN()
ON_COMMAND(ID_FILE_CLOSEALLIMAGES, &CProgressDisplayView::OnFileCloseallimages)
ON_COMMAND(ID_VIEW_SEEFIRSTLAYER, &CProgressDisplayView::OnViewSeefirstlayer)
ON_UPDATE_COMMAND_UI(ID_VIEW_SEEFIRSTLAYER, &CProgressDisplayView::OnUpdateViewSeefirstlayer)
ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

// CProgressDisplayView construction/destruction

CProgressDisplayView::CProgressDisplayView()
{
	theApp.m_pProgView = this;
	bFlagIsDecompressing = FALSE;
	m_bIsFirstPreview = FALSE;

	m_nSelectedViewerIndex = -1;

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

	delete pDecompJPEG2000;
	bFlagIsDecompressing = FALSE;
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
	if (m_pLayoutManager->GetImageViewerCount() < 0)
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

	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}


void CProgressDisplayView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	INT_PTR nClickIndex = m_pLayoutManager->GetImageViewerIndexFromPos(point);

	if (nClickIndex < 0)
	{
		return;
	}

	m_nSelectedViewerIndex = nClickIndex;

	m_pLayoutManager->GetImageViewer(nClickIndex)->UpdateCompFileName();
	m_pLayoutManager->GetImageViewer(nClickIndex)->UpdateLayerNum();
	m_pLayoutManager->GetImageViewer(nClickIndex)->UpdateDecodingTime();

	RedrawWindow();

	CView::OnLButtonDown(nFlags, point);
}


void CProgressDisplayView::OnFileCloseallimages()
{
	m_nSelectedViewerIndex = -1;

	m_pLayoutManager->CloseAllImageViewer();

	RedrawWindow();
	// TODO: Add your command handler code here
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


BOOL CProgressDisplayView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	//return CView::OnEraseBkgnd(pDC);
	return FALSE;
}
