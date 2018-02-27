#include "stdafx.h"
#include "Utils.h"
#include <TlHelp32.h>
#include <ShlObj.h>
#include <Psapi.h>
#include <atlenc.h>

#pragma comment( lib, "psapi.lib" )
#pragma comment( lib, "version.lib" )

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static BOOL CALLBACK EnumFontProcEx(LPLOGFONT lplf, LPTEXTMETRIC lptm, DWORD dwType, LPARAM lpData)
{
	CString str;

	CStringArray* pArayFonts = (CStringArray*)(lpData);
	str.Format(_T("%s"), lplf->lfFaceName);

	if ((str.Find(_T("Wingdings")) != -1) || (str.Find(_T("Webdings")) != -1))
	{
		return TRUE;
	}

	//pArayFonts->Add( str );
	INT_PTR nInsertIndex = -1;
	INT_PTR nCount = pArayFonts->GetCount();
	for (INT_PTR nIndex = 0; nIndex < nCount; nIndex++)
	{
		CString strOld = pArayFonts->GetAt(nIndex);
		//if (_tcsicmp(strOld, str) < 0);
		if (strOld.Compare(str) > 0)
		{
			nInsertIndex = nIndex;
			break;
		}
	}

	if (nInsertIndex >= 0)
	{
		pArayFonts->InsertAt(nInsertIndex, str);
	}
	else
	{
		pArayFonts->Add(str);
	}

	return TRUE;
}

CUtils::CUtils()
{

}

CUtils::~CUtils()
{

}

CString	CUtils::GetExeRootPath()
{
	TCHAR szRootPath[MAX_PATH] = {0,};
	TCHAR szDrive[3] = {0,};
	TCHAR szPath[MAX_PATH] = {0,};
	::GetModuleFileName(::GetModuleHandle(NULL), szRootPath, MAX_PATH -1);
	_tsplitpath(szRootPath, szDrive, szPath, NULL, NULL);
	memset(szRootPath, NULL, MAX_PATH);
	_tmakepath(szRootPath, szDrive, szPath, NULL, NULL);

	CString strRootPath;
	strRootPath.Format(_T("%s"), szRootPath);
	//
	CStringArray aryPath;
	int nCurPos = 0;
	CString strToken = strRootPath.Tokenize(_T("\\"), nCurPos);
	while (!strToken.IsEmpty())
	{
		aryPath.Add(strToken);
		//
		strToken = strRootPath.Tokenize(_T("\\"), nCurPos);
	};
	
	// Remove previous folder if "..' found
	for (INT_PTR nIndex = 0; nIndex < aryPath.GetCount(); nIndex++)
	{
		CString strPath = aryPath.GetAt(nIndex);
		if (strPath.Compare(_T("..")) == 0 && nIndex != 0)
		{
			aryPath.RemoveAt(nIndex);
			//
			aryPath.RemoveAt(nIndex - 1);
			// 
			nIndex = 0;
		}
	}

	strRootPath.Empty();
	for (INT_PTR nIndex = 0; nIndex < aryPath.GetCount(); nIndex++)
	{
		CString strPath = aryPath.GetAt(nIndex);
		strRootPath += strPath + _T("\\");
	}

	//
	return strRootPath;
}

CString	CUtils::GetExeFileName()
{
	CString strResult;
	WCHAR szExePath[MAX_PATH] = {0, };

	HMODULE hMod = ::GetModuleHandle(NULL);
	//
	if (hMod)
	{
		::GetModuleFileName(hMod, szExePath, MAX_PATH);
		strResult = szExePath;
	}

	return strResult;
}

CString CUtils::GetExeRootPath(CString strProcessName)
{
	CString strFullPath;

	HANDLE hSnapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPALL|TH32CS_SNAPMODULE|TH32CS_SNAPMODULE32, NULL);


	PROCESSENTRY32 procEntry;
	procEntry.dwSize = sizeof(procEntry);

	BOOL bFlag = ::Process32First(hSnapshot, &procEntry);

	strProcessName = strProcessName.MakeUpper();
	while (bFlag)
	{
		CString strCurExeName = procEntry.szExeFile;
		strCurExeName = strCurExeName.MakeUpper();

		if (strCurExeName.Find(strProcessName) != -1)
		{
			HANDLE hHandle = ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, procEntry.th32ProcessID);
			if (hHandle)
			{
				TCHAR szFileName[MAX_PATH];
				if (GetModuleFileNameEx(hHandle, NULL, szFileName, MAX_PATH) != 0)
				{
					strFullPath = szFileName;
					//
					strFullPath = ExtractFilePath(strFullPath, TRUE);
				}
//				::CloseHandle(hHandle);
			}
 			
			break;
		}

		//
		bFlag = ::Process32Next(hSnapshot, &procEntry);
	}

	::CloseHandle(hSnapshot);

	

	
	return strFullPath;
}

BOOL CUtils::ExtractQATCommands(CString strQATCommands, CList<UINT,UINT>* plistQATCommands)
{
	int nStartPos = 0;
	int nEndPos = 0;
	UINT nCommand = 0;
	CString strTemp(_T(""));
	
	plistQATCommands->RemoveAll();


	CString sDelimitor = _T("|");
	int nCurPos = 0;
	CString strToken = strQATCommands.Tokenize(sDelimitor, nCurPos);
	while (!strToken.IsEmpty())
	{
		UINT nCommand = _wtoi((LPTSTR)(LPCTSTR)strToken);
		plistQATCommands->AddTail(nCommand);
		strToken = strQATCommands.Tokenize(sDelimitor, nCurPos);
	};


// 	nEndPos = strQATCommands.Find(_T("|"), nStartPos);
// 	while(nEndPos>0)
// 	{
// 		strTemp.Format(_T("%s"), strQATCommands.Mid(nStartPos, nEndPos-nStartPos));
// 		
// 		nCommand = _wtoi((LPTSTR)(LPCTSTR)strTemp);
// 		plistQATCommands->AddTail(nCommand);
// 		
// 		nStartPos = nEndPos+1;
// 		nEndPos = strQATCommands.Find(_T("|"), nStartPos);
// 	}

	return TRUE;
}

CString CUtils::FormatStrQATCommands(CList<UINT,UINT>* plistQATCommands)
{
	CString strQATCommands;
	CString sDelimitor = _T("|");

	strQATCommands.Empty();

	POSITION pos = plistQATCommands->GetHeadPosition();
	while(pos)
	{
		CString strCommand;
		strCommand.Format(_T("%d"), plistQATCommands->GetNext(pos));
		if (strCommand.Compare(_T("0")) == 0)
			continue;

		strQATCommands += (strCommand + sDelimitor);
	}

	return strQATCommands;
}

BOOL CUtils::CalcFileSize(CString strFilePath, double& fFileSize)
{
	fFileSize = 0;

	BOOL bRet = FALSE;
	FILE* pFile = NULL;

	TRY 
	{
		pFile = _tfopen((LPCTSTR)strFilePath, _T("rb"));
		if (pFile)
		{
			if (fseek(pFile, 0, SEEK_END)==0)
			{
				fFileSize = (float)ftell(pFile);
				fFileSize /= 1048576;
				bRet = TRUE;
			}
		}
	}
	CATCH_ALL (e)
	{
		bRet = FALSE;
	}
	END_CATCH_ALL

	if (pFile)
	{
		fclose(pFile);
	}

	return bRet;
}

ULONGLONG CUtils::GetTotalFileSize(CString strFolder)
{
	ULONG uFileSize = 0;

	CString strFilePath = strFolder + L"\\*.*";

	CFileFind finder;
	BOOL bResult = finder.FindFile(strFilePath);
	while (bResult)
	{
		bResult = finder.FindNextFile();
		if(finder.IsDots())
		{
			continue;
		}

		if (finder.IsDirectory())
		{
			uFileSize += GetTotalFileSize(strFolder + L"\\" + finder.GetFileName());
		}

		uFileSize += finder.GetLength();
	}

	return uFileSize;
}

CString CUtils::ExtractFileName(CString strFileName)
{
	CString strFName = strFileName;

	if (!strFileName.IsEmpty())
	{
		int nResult = strFileName.ReverseFind(TCHAR('.'));
		if (nResult != -1)
		{
			strFName = strFileName.Left(nResult);
		}
	}

	return strFName;
}

CString CUtils::ExtractFileNameOnly(CString strFileName)
{
	if (strFileName.Find(_T("\\")) != -1)
	{
		strFileName = CUtils::ExtractWholeFileName(strFileName);
	}


	return ExtractFileName(strFileName);
}

CString CUtils::ExtractWholeFileName(CString strFileName)
{
	CString strFileFullName = strFileName;

	if (!strFileName.IsEmpty())
	{
		int nResult = strFileName.ReverseFind(TCHAR('\\'));
		if (nResult != -1)
		{
			strFileFullName = strFileName.Mid(nResult+1);
		}
	}

	return strFileFullName;
}

CString CUtils::ExtractFilePath(CString strFullPath, BOOL bWithLastDelimiter)
{
	CString strFilePath = strFullPath;

	if (!strFullPath.IsEmpty())
	{
		int nResult = strFullPath.ReverseFind(TCHAR('\\'));
		if (nResult != -1)
		{
			if (bWithLastDelimiter)
			{
				nResult +=1;
			}
			strFilePath = strFullPath.Left(nResult);
		}
	}

	return strFilePath;
}

CString CUtils::ExtractFileExt(CString strFileName, BOOL bMakeUpperCase/* = TRUE*/)
{
	CString strFileExt;

	if (!strFileName.IsEmpty())
	{
		int nResult = strFileName.ReverseFind(TCHAR('.'));
		if (nResult != -1) strFileExt = strFileName.Right(strFileName.GetLength() - nResult - 1);
	}

	if (bMakeUpperCase) strFileExt.MakeUpper();

	return strFileExt;
}

CString CUtils::ExtractString(CString strString, CString strDelimiter, BOOL bReverse)
{
	CString strNew;

	int nPos = 0;
	if (bReverse)
	{
		// FIXME:
		nPos = strString.ReverseFind(strDelimiter.GetAt(0));
		strNew = strString.Mid(nPos+1);
	}
	else
	{
		nPos = strString.Find(strDelimiter);
		strNew = strString.Left(nPos);
	}

	return strNew;
}

BOOL CUtils::IsFolderEmpty(CString strFolderPath)
{
	HANDLE hSearch;
	BOOL fFinished = FALSE; 
	WIN32_FIND_DATA findData;

	hSearch = FindFirstFile(strFolderPath+_T("*.*"), &findData);

	if (hSearch != INVALID_HANDLE_VALUE)
	{
		while (!fFinished) 
		{ 
			if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
			{
				CString str = findData.cFileName;
				if (str != _T(".") && str != _T(".."))
				{
					FindClose(hSearch);
					//
					return FALSE;
				}
			}

			if (!FindNextFile(hSearch, &findData)) 
			{
				if (GetLastError() == ERROR_NO_MORE_FILES)
				{
					fFinished = TRUE; 
				}
				else
				{
					break;
				}
			}
		} 

		FindClose(hSearch);
	}

	return TRUE;
}

int CUtils::GetFileListInCurrentFolder(CStringList *pFileNameList, LPCTSTR lpszDir, LPCTSTR lpszFileExt)
{
	ASSERT(pFileNameList != NULL);

	// Exception string check
	const WCHAR szExceptionDir[MAX_PATH] = _T("Application Data\\Application Data\\Application Data");
	const WCHAR szExcetionDir2[MAX_PATH] = _T("$RECYCLE.BIN");

	CString strExceptionCheck;
	strExceptionCheck.Format(_T("%s"), lpszDir);

	if (strExceptionCheck.Find(szExceptionDir) != -1)
	{
		return 0;
	}

	strExceptionCheck.MakeLower();
	CString strExceptionDir2 = szExcetionDir2;
	strExceptionDir2.MakeLower();
	if (strExceptionCheck.Find(strExceptionDir2) != -1)
	{
		return 0;
	}

	if (_taccess_s(lpszDir, 04) == -1)
	{
		return 0;
	}

	int nTotal = 0;

	if (pFileNameList)
	{
		HANDLE hSearch;
		BOOL fFinished = FALSE;
		WIN32_FIND_DATA FileData;
		WCHAR szSearchFile[MAX_PATH];
		WCHAR szTemp[MAX_PATH];
		CString strFileExt;
		strFileExt.Format(_T("%s"), lpszFileExt);
		if (!lpszFileExt)
		{
			strFileExt.Empty();
		}
		strFileExt.MakeUpper();

		swprintf_s(szSearchFile, _countof(szSearchFile), _T("%s\\*.*"), lpszDir);
		hSearch = FindFirstFile(szSearchFile, &FileData);

		if (hSearch != INVALID_HANDLE_VALUE)
		{
			while (!fFinished)
			{
				if ((FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
				{
					;
				}
				else
				{
					if (strFileExt.IsEmpty() || (!strFileExt.IsEmpty() && ExtractFileExt(FileData.cFileName) == strFileExt))
					{
						swprintf_s(szTemp, _countof(szTemp), _T("%s\\%s"), lpszDir, FileData.cFileName);
						pFileNameList->AddTail(szTemp);
						nTotal++;
					}
				}
				if (!FindNextFile(hSearch, &FileData))
				{
					if (GetLastError() == ERROR_NO_MORE_FILES) fFinished = TRUE;
					else break;
				}
			}

			FindClose(hSearch);
		}
	}

	return nTotal;
}

int CUtils::GetFileListInFolder(CStringList* pFileNameList, LPCTSTR lpszDir, LPCTSTR lpszFileExt/* = NULL*/)
{
	ASSERT(pFileNameList != NULL);

	// Exception string check
	const WCHAR szExceptionDir[MAX_PATH] = _T("Application Data\\Application Data\\Application Data");
	const WCHAR szExcetionDir2[MAX_PATH] = _T("$RECYCLE.BIN");

	CString strExceptionCheck;
	strExceptionCheck.Format(_T("%s"), lpszDir);

	if (strExceptionCheck.Find(szExceptionDir) != -1)
	{
		return 0;
	}

	strExceptionCheck.MakeLower();
	CString strExceptionDir2 = szExcetionDir2;
	strExceptionDir2.MakeLower();
	if (strExceptionCheck.Find(strExceptionDir2) != -1)
	{
		return 0;
	}

	if (_taccess_s(lpszDir, 04) == -1)
	{
		return 0;
	}

	int nTotal = 0;

	if (pFileNameList)
	{
		HANDLE hSearch;
		BOOL fFinished = FALSE; 
		WIN32_FIND_DATA FileData;
		WCHAR szSearchFile[MAX_PATH];
		WCHAR szTemp[MAX_PATH];
		CString strFileExt;
		strFileExt.Format(_T("%s"), lpszFileExt);
		if (!lpszFileExt)
		{
			strFileExt.Empty();
		}
		strFileExt.MakeUpper();

		swprintf_s(szSearchFile, _countof(szSearchFile), _T("%s\\*.*"), lpszDir);
		hSearch = FindFirstFile(szSearchFile, &FileData);

		if (hSearch != INVALID_HANDLE_VALUE)
		{
			while (!fFinished) 
			{ 
				if ((FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
				{
					CString str = FileData.cFileName;
					if (str != _T(".") && str != _T(".."))
					{
						swprintf_s(szTemp, _countof(szTemp), _T("%s\\%s"), lpszDir, FileData.cFileName);
						GetFileListInFolder(pFileNameList, szTemp, lpszFileExt);
					}
				}
				else
				{
					if (strFileExt.IsEmpty() || (!strFileExt.IsEmpty() && ExtractFileExt(FileData.cFileName) == strFileExt))
					{
						swprintf_s(szTemp, _countof(szTemp), _T("%s\\%s"), lpszDir, FileData.cFileName);
						pFileNameList->AddTail(szTemp);
						nTotal++;
					}
				}
				if (!FindNextFile(hSearch, &FileData)) 
				{
					if (GetLastError() == ERROR_NO_MORE_FILES) fFinished = TRUE; 
					else break;
				}
			} 

			FindClose(hSearch);
		}
	}

	return nTotal;
}


int CUtils::GetFileListInFolder(CStringArray* pAryFileName, LPCTSTR lpszDir, BOOL& bStopRead, BOOL& bSubDir)
{
	ASSERT(pAryFileName != NULL);

	// Exception string check
	const WCHAR szExceptionDir[MAX_PATH] = _T("Application Data\\Application Data\\Application Data");
	const WCHAR szExcetionDir2[MAX_PATH] = _T("$RECYCLE.BIN");

	CString strExceptionCheck;
	strExceptionCheck.Format(_T("%s"), lpszDir);

	if (strExceptionCheck.Find(szExceptionDir) != -1)
	{
		return 0;
	}

	strExceptionCheck.MakeLower();
	CString strExceptionDir2 = szExcetionDir2;
	strExceptionDir2.MakeLower();
	if (strExceptionCheck.Find(strExceptionDir2) != -1)
	{
		return 0;
	}

	if (_taccess_s(lpszDir, 04) == -1)
	{
		return 0;
	}

	int nTotal = 0;

	if (bStopRead)
	{
		return -1;
	}

	if (pAryFileName)
	{
		HANDLE hSearch;
		BOOL fFinished = FALSE;
		WIN32_FIND_DATA FileData;
		TCHAR szSearchFile[1024] = { 0, };
		TCHAR szTemp[1024] = { 0, };

		swprintf_s(szSearchFile, _countof(szSearchFile), _T("%s\\*.*"), lpszDir);
		hSearch = FindFirstFile(szSearchFile, &FileData);

		if (hSearch != INVALID_HANDLE_VALUE)
		{
			while (!fFinished)
			{
				if (bStopRead)
				{
					FindClose(hSearch);
					return -1;
				}
				if ((FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
				{
					CString str = FileData.cFileName;
					if (str != _T(".") && str != _T("..") && bSubDir)
					{
						swprintf_s(szTemp, _countof(szTemp), _T("%s\\%s"), lpszDir, FileData.cFileName);
						GetFileListInFolder(pAryFileName, szTemp, bStopRead, bSubDir);
					}
				}
				else
				{
					CString strPath;
					strPath.Format(_T("%s\\%s"), lpszDir, FileData.cFileName);
					pAryFileName->Add(strPath);
					nTotal++;
				}
				if (!FindNextFile(hSearch, &FileData))
				{
					if (GetLastError() == ERROR_NO_MORE_FILES)
					{
						fFinished = TRUE;
					}
					else
					{
						break;
					}
				}

			}

			FindClose(hSearch);
		}
	}

	return nTotal;
}


int CUtils::GetFolderListInFolder(CStringList* pFileNameList, LPCTSTR lpszDir)
{
	ASSERT(pFileNameList != NULL);

	int nTotal = 0;

	if (pFileNameList)
	{
		HANDLE hSearch;
		BOOL fFinished = FALSE; 
		WIN32_FIND_DATA FileData;
		WCHAR szSearchFile[MAX_PATH];
		WCHAR szTemp[MAX_PATH];


		swprintf_s(szSearchFile, _countof(szSearchFile), _T("%s\\*.*"), lpszDir);
		hSearch = FindFirstFile(szSearchFile, &FileData);

		if (hSearch != INVALID_HANDLE_VALUE)
		{
			while (!fFinished) 
			{ 
				if ((FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
				{
					CString str = FileData.cFileName;
					if (str != _T(".") && str != _T(".."))
					{
						swprintf_s(szTemp, _countof(szTemp), _T("%s"), FileData.cFileName);
						pFileNameList->AddTail(szTemp);
						GetFileListInFolder(pFileNameList, szTemp);
					}
				}

				if (!FindNextFile(hSearch, &FileData)) 
				{
					if (GetLastError() == ERROR_NO_MORE_FILES) fFinished = TRUE; 
					else break;
				}
			} 

			FindClose(hSearch);
		}
	}

	return nTotal;
}

BOOL CUtils::CreateFolder(CString strFolderPath)
{
	size_t nPathLen = _tcslen(strFolderPath);
	if(nPathLen > MAX_PATH)
		return FALSE;
	TCHAR	szFolder[MAX_PATH+1];
	memset(szFolder,0,sizeof(szFolder));
	_tcscpy_s(szFolder, strFolderPath);
	TCHAR*	pStart = szFolder;
	TCHAR*	pEnd	= pStart + _tcslen(szFolder);
	TCHAR*	p = pEnd;

	// Try 10 times to create the directory
	for(int i=0; i<10; i++)
	{
		BOOL	bOk		= CreateDirectory(szFolder,NULL);
		DWORD	dwError = GetLastError();
		if(!bOk && dwError == ERROR_PATH_NOT_FOUND)
		{
			while(*p != '\\')
			{
				if(p == pStart)
					return FALSE;
				p--;
			}
			*p = NULL;
		}
		else if(bOk || (ERROR_ALREADY_EXISTS == dwError))
		{
			if(p == pEnd)
				return TRUE;
			*p = '\\';
			while(*p)
				p++;
		}
		else
		{
			break;
		}
	}
	return FALSE;
}

BOOL CUtils::RemoveFolder(CString strFolderPath)
{
	TCHAR szFullPathFileName[MAX_PATH];
	TCHAR szFilename[MAX_PATH];

	if (!::RemoveDirectory(strFolderPath))
	{
		DWORD err = GetLastError();
		if (err != ERROR_DIR_NOT_EMPTY)
			return FALSE;
	}

	// remove sub folders and files.

	WIN32_FIND_DATA FileData = {0};
	BOOL bFinished = FALSE; 
	DWORD dwSize = 0;

	_stprintf(szFullPathFileName, _T("%s\\*.*"), strFolderPath);
	HANDLE hSearch = FindFirstFile(szFullPathFileName, &FileData); 
	if (hSearch == INVALID_HANDLE_VALUE) 
		return 0;

	while (!bFinished)
	{
		_stprintf(szFilename, _T("%s\\%s"),strFolderPath,FileData.cFileName);
		if (FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (_tcscmp(FileData.cFileName,TEXT(".")) && _tcscmp(FileData.cFileName,TEXT("..")))
			{
				RemoveFolder(szFilename);
				RemoveDirectory(szFilename);
			}
		}
		else
		{
			if (FileData.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
				SetFileAttributes(szFilename, FILE_ATTRIBUTE_NORMAL);

			if (!::DeleteFile(szFilename))
			{
				FindClose(hSearch);
				return FALSE;
			}
		}
		if (!FindNextFile(hSearch, &FileData)) 
		{
			if (GetLastError() == ERROR_NO_MORE_FILES) 
				bFinished = TRUE;
		} 
	}
	FindClose(hSearch);

	// Here the directory is empty.
	::RemoveDirectory(strFolderPath);
	return TRUE;
}

BOOL CUtils::DeleteFolder(const CString &strFolder)
{
	SHFILEOPSTRUCT FileOp = {0};
	WCHAR szTemp[MAX_PATH];

	_tcsncpy_s(szTemp, MAX_PATH, strFolder, _TRUNCATE);

	szTemp[strFolder.GetLength() + 1] = NULL; // NULL문자가 두개 들어가야 한다.

	FileOp.hwnd = NULL;
	FileOp.wFunc = FO_DELETE;
	FileOp.pFrom = NULL;
	FileOp.pTo = NULL;
	FileOp.fFlags = FOF_NO_UI;//FOF_NOCONFIRMATION | FOF_NOERRORUI; // 확인메시지가 안뜨도록 설정
	FileOp.fAnyOperationsAborted = false;
	FileOp.hNameMappings = NULL;
	FileOp.lpszProgressTitle = NULL;

	FileOp.pFrom = szTemp;
	SHFileOperation(&FileOp);

	return true;
}

CString CUtils::GetCommonAppDataPath()
{
	TCHAR szPath[MAX_PATH];
	if (FAILED(::SHGetFolderPath(NULL, CSIDL_COMMON_APPDATA, NULL, 0, szPath)))
	{
		AfxDebugBreak();
	}

	return CString(szPath);
}

CString CUtils::GetVieworksAppDataPath()
{
	CString strPath;
	strPath.Format(_T("%s\\Vieworks\\QXLink3\\"), GetCommonAppDataPath());

	return strPath;
}

CString CUtils::IntToStr(int nData)
{
	CString strReturn;
	strReturn.Format(_T("%d"), nData);
	return strReturn;
// 	CString strResult;
// 	char szTemp[MAX_PATH] = {0, };
// 
// 	_ltoa_s(nData, szTemp, _countof(szTemp), 10);
// 	strResult = szTemp;
// 
// 	return strResult;
}

CString CUtils::LongToStr(long nData)
{
	CString strReturn;
	strReturn.Format(_T("%ld"), nData);
	return strReturn;
}

CString CUtils::FloatToStr(float fData)
{
	CString strResult;
	strResult.Format(_T("%f"), fData);

	return strResult;
}

CString CUtils::BoolToStr(BOOL bBool)
{
	CString strResult = _T("1");
	if (!bBool)
	{
		strResult = _T("0");
	}

	return strResult;
}

CString CUtils::ByteToStr(BYTE bByte)
{
	CString strReturn;
	strReturn.Format(_T("%d"), bByte);
	return strReturn;
}

int CUtils::StrToInt(CString strValue)
{
	int nData = _ttoi((LPTSTR)(LPCTSTR)strValue);
	return nData;
}

double CUtils::StrToDouble(CString strValue)
{
	double dData = _wtof((LPTSTR)(LPCTSTR)strValue);
	return dData;
}

int CUtils::GetTwipSize(CString strValue)
{
	LPCTSTR lpszText = strValue;

	while (*lpszText == ' ' || *lpszText == '\t')
	{
		lpszText++;
	}

	if (lpszText[0] == NULL)
	{
		return -1; // no text in control
	}

	double d = _tcstod(lpszText, (LPTSTR*)&lpszText);
	while (*lpszText == ' ' || *lpszText == '\t')
	{
		lpszText++;
	}

	if (*lpszText != NULL)
	{
		return -2;   // not terminated properly
	}

	return (d < 0.f) ? 0 : (int)(d * 20.0f);
}

CString CUtils::TwipsToPointString(int nTwips)
{
	CString str;
	if (nTwips >= 0)
	{
		// round to nearest half point
		nTwips = (nTwips + 5) / 10;

		if ((nTwips % 2) == 0)
		{
			str.Format (_T("%ld"), nTwips/2);
		}
		else
		{
			str.Format (_T("%.1f"), (float) nTwips / 2.0f);
		}
	}

	return str;
}

BOOL CUtils::IsWhiteSpaceOnly(CString strValue)
{
	if (strValue.Right(1) == _T('\0'))
	{
		strValue.Delete(strValue.GetLength()-1);
	}

	BOOL bIsWhiteSpaceOnly = TRUE;
	for (INT_PTR nIndex = 0; nIndex < strValue.GetLength(); nIndex++)
	{
		TCHAR chOne = strValue.GetAt(nIndex);
		if (chOne != _T('\r') && chOne != _T('\n') && chOne != _T('\t'))
		{
			bIsWhiteSpaceOnly = FALSE;
			break;
		}
	}

	return bIsWhiteSpaceOnly;
}

CStringA CUtils::EncodeStringAsBase64(CStringW strSource)
{
	USES_CONVERSION;

	CStringA strValue = CW2A(strSource);
	int nRequiredLength = ::Base64EncodeGetRequiredLength(strValue.GetLength()) + 1;
	//
	CStringA strEncoded;
	LPSTR lpString = strEncoded.GetBufferSetLength(nRequiredLength);
	//
	::Base64Encode((BYTE*)strValue.GetBuffer(0), strValue.GetLength(), lpString, &nRequiredLength);
	lpString[nRequiredLength] = '\0';
	strEncoded.ReleaseBufferSetLength(nRequiredLength);
	
	return strEncoded;
}

CStringW CUtils::DecodeStringAsBase64(CStringA strSource)
{
	USES_CONVERSION;

	int nRequiredLength = Base64DecodeGetRequiredLength(strSource.GetLength()) + 1;
	//
	CStringA strDecoded;
	LPSTR lpString = strDecoded.GetBufferSetLength(nRequiredLength);
	//
	Base64Decode(strSource.GetBuffer(0), strSource.GetLength(), (BYTE*)lpString, &nRequiredLength);
	lpString[nRequiredLength] = '\0';
	strDecoded.ReleaseBufferSetLength(nRequiredLength);

	CStringW strDecodedNew = CA2W(strDecoded);
	return strDecodedNew;
}

BOOL CUtils::StrToBool(CString strValue)
{
	int nData = StrToInt(strValue);
	if (nData > 0)
		return TRUE;

	return FALSE;
}

CString CUtils::FormatDICOMDate(CString strDate, CString strFormat)
{
	CString strOutput;

	if (strDate.GetLength() >= 8)
	{
		int nYear = StrToInt(strDate.Mid(0, 4));
		int nMon = StrToInt(strDate.Mid(4, 2));
		int nDay = StrToInt(strDate.Mid(6, 2));

		if (nYear < 100 || nYear > 9999) nYear = 1900;
		if (nMon < 1 || nMon > 12) nMon = 1;
		if (nDay < 1 || nDay > 31) nDay = 1;

		COleDateTime tmTemp(nYear, nMon, nDay, 0, 0, 0);
		strOutput = tmTemp.Format(strFormat);
		if (strOutput == _T("19000101"))
		{
			strOutput.Empty();
		}
	}

	return strOutput;
}

CString CUtils::GetDateTimeString(COleDateTime& dtDateTime, CString strFormat)
{
	CString strDttm;
	if (dtDateTime.GetStatus() != COleDateTime::valid)
	{
		return strDttm;
	}

	//
	if (strFormat.IsEmpty())
	{
		strFormat.Format(_T("%s (%s)"), _T("%Y-%m-%d"), _T("%H:%M:%S"));
	}

	//
	strDttm = dtDateTime.Format(strFormat);
	return strDttm;
}

BOOL CUtils::IsValidPatientBirthDate(COleDateTime& dtBirthDate)
{
	if (dtBirthDate.GetStatus() != COleDateTime::valid)
	{
		return FALSE;
	}

	if (dtBirthDate.GetYear() == 1900)
	{
		if (dtBirthDate.GetMonth() == 01)
		{
			if (dtBirthDate.GetDay() == 01)
			{
				return FALSE;
			}
		}
	}


	return TRUE;
}

BOOL CUtils::IsValidDateForOleDateTime(INT_PTR nYear, INT_PTR nMonth, INT_PTR nDay, INT_PTR nHour, INT_PTR nMin, INT_PTR nSec)
{
	if (nYear < 1900 || nYear > 9999)
		return FALSE;
	if (nMonth < 0 || nMonth > 12)
		return FALSE;
	if (nDay < 0 || nDay > 31)
		return FALSE;
	if (nHour < 0 || nHour > 23)
		return FALSE;
	if (nMin < 0 || nMin > 59)
		return FALSE;
	if (nSec < 0 || nSec > 59)
		return FALSE;

	return TRUE;
}

CString CUtils::FormatDICOMTime(CString strTime, CString strFormat)
{
	CString strOutput;
	COleDateTime tmNow = COleDateTime::GetCurrentTime();

	// ex. "070907.0705"
	if (strTime.GetLength() >= 6)
	{
		int nHour = StrToInt(strTime.Mid(0, 2));
		int nMin = StrToInt(strTime.Mid(2, 2));
		int nSec = StrToInt(strTime.Mid(4, 2));

		if (nHour < 0 || nHour > 23) nHour = 0;
		if (nMin < 0 || nMin > 59) nMin = 0;
		if (nSec < 0 || nSec > 59) nSec = 0;

		COleDateTime tmTemp(tmNow.GetYear() , tmNow.GetMonth(), tmNow.GetDay(), nHour, nMin, nSec);
		strOutput = tmTemp.Format(strFormat);
	}
	// ex. "1010"
	else if (strTime.GetLength() >= 4)
	{
		int nHour = StrToInt(strTime.Mid(0, 2));
		int nMin = StrToInt(strTime.Mid(2, 2));

		if (nHour < 0 || nHour > 23) nHour = 0;
		if (nMin < 0 || nMin > 59) nMin = 0;

		COleDateTime tmTemp(tmNow.GetYear() , tmNow.GetMonth(), tmNow.GetDay(), nHour, nMin, 0);
		strOutput = tmTemp.Format(strFormat);
	}

	return strOutput;
}

BOOL CUtils::IsProcessRunning(CString strProcessName)
{
	strProcessName = strProcessName.MakeUpper();
	//

	HANDLE hSnapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPALL, NULL);
	PROCESSENTRY32 procEntry;
	procEntry.dwSize = sizeof(procEntry);

	BOOL bFlag = ::Process32First(hSnapshot, &procEntry);

	while (bFlag)
	{
		CString strCurExeName = procEntry.szExeFile;
		strCurExeName = strCurExeName.MakeUpper();

		if (strCurExeName.Find(strProcessName) != -1)
		{
			::CloseHandle(hSnapshot);
			return TRUE;
		}

		//
		bFlag = ::Process32Next(hSnapshot, &procEntry);
	}
	
	::CloseHandle(hSnapshot);

	return FALSE;
}

BOOL CUtils::ExecuteProcess(CString strFilePath, CString strArg)
{
	ULONG fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_NOASYNC;

	return ExecuteProcess(strFilePath, strArg, fMask);
}

BOOL CUtils::ExecuteProcess(CString strFilePath, CString strArg, ULONG fMask)
{
	SHELLEXECUTEINFO si;
	ZeroMemory(&si, sizeof(SHELLEXECUTEINFO));
	si.cbSize = sizeof(SHELLEXECUTEINFO);
	//si.lpVerb = _T("runas");
	si.lpVerb = _T("open");
	si.lpFile = strFilePath;
	//
	si.nShow = SW_HIDE;
	si.fMask = fMask;// SEE_MASK_NOCLOSEPROCESS | SEE_MASK_NOASYNC/*SEE_MASK_FLAG_DDEWAIT*/;
	//
	int nLen = strArg.GetLength();
	LPCWSTR pString = strArg.GetBuffer(strArg.GetLength() + 1);
	si.lpParameters = (LPCWSTR)pString;
	//
	BOOL bRet = ::ShellExecuteEx(&si);

	if (bRet)
	{
		if (si.hProcess)
		{
			return TRUE;
		}
	}

	return FALSE;
// 	if (bRet)
// 	{
// 		WaitForSingleObject(si.hProcess, 3000);
// 	}
}

BOOL CUtils::KillProcessByName(CString strProcessName)
{
	HANDLE hProcessSnap = NULL;
	DWORD Return = FALSE;
	PROCESSENTRY32 pe32 = { 0 };
	
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (hProcessSnap == INVALID_HANDLE_VALUE)
		return (DWORD)INVALID_HANDLE_VALUE;

	pe32.dwSize = sizeof(PROCESSENTRY32);

	if (Process32First(hProcessSnap, &pe32))
	{
		DWORD Code = 0;
		DWORD dwPriorityClass;

		do {
			HANDLE hProcess;
			hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
			dwPriorityClass = GetPriorityClass(hProcess);

			CString Temp = pe32.szExeFile;

			if (Temp == strProcessName)
			{
				if (TerminateProcess(hProcess, 0))
					GetExitCodeProcess(hProcess, &Code);
				else
					return GetLastError();
			}
			CloseHandle(hProcess);
		} while (Process32Next(hProcessSnap, &pe32));
		Return = TRUE;
	}
	else
		Return = FALSE;

	CloseHandle(hProcessSnap);

	return Return;
}

HICON CUtils::LoadIcon(CString strIconPath, INT_PTR nSize)
{
	HICON hIcon = (HICON)::LoadImage(AfxGetInstanceHandle(), 
		strIconPath.GetBuffer(strIconPath.GetLength()), 
		IMAGE_ICON, 
		nSize, 
		nSize,
		LR_LOADFROMFILE|LR_LOADTRANSPARENT);

	//
	return hIcon;
}

CString CUtils::GetSystemErrorMessage(DWORD dwMessageID)
{
	LPVOID lpMsgBuf = NULL;
	//
	::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);

	CString strMsg;
	strMsg = (LPTSTR)lpMsgBuf;
	//
	::LocalFree(lpMsgBuf);

	return strMsg;
}

BOOL CUtils::GetFileVersionInfo(CString strFilePath, DWORD dwVersion[4])
{
	if (_taccess_s(strFilePath, 04) != 0)
	{
		return FALSE;
	}

	//
	VS_FIXEDFILEINFO* pFileInfo = nullptr;
	UINT uLen = 0;

	DWORD dwHdlDest = 0;
	DWORD dwSizeDest = 0;
	DWORD dwRet = 0;

	BOOL bIsVersionDetected = FALSE;
	dwSizeDest = ::GetFileVersionInfoSize(strFilePath, &dwHdlDest);
	if (dwSizeDest)
	{
		TCHAR* pDestData = new TCHAR[dwSizeDest + 1];
		if (::GetFileVersionInfo(strFilePath, dwHdlDest, dwSizeDest, pDestData))
		{
			if (::VerQueryValue(pDestData, _T("\\"), (void**)&pFileInfo, &uLen))
			{
				dwVersion[0] = (pFileInfo->dwFileVersionMS / 65536);
				dwVersion[1] = (pFileInfo->dwFileVersionMS % 65536);
				dwVersion[2] = (pFileInfo->dwFileVersionLS / 65536);
				dwVersion[3] = (pFileInfo->dwFileVersionLS % 65536);
				//
				bIsVersionDetected = TRUE;
			}
		}

		//
		delete[] pDestData;
	}

	return bIsVersionDetected;
}

void CUtils::CreateFileAsUtf8(CString strFilePath)
{
	CFile file;
	file.Open(strFilePath, CFile::modeWrite | CFile::modeCreate);
	// BOM
	const char utf8_bom[] = { '\xef', '\xbb', '\xbf' };
	file.Write(utf8_bom, sizeof(utf8_bom));
	file.Close();
}

BOOL CUtils::GetSystemFontNames(CStringArray* pStringArray)
{
	//CClientDC dc(this);
	HWND hWnd = ::GetDesktopWindow();
	if (!hWnd)
	{
		return FALSE;
	}

	CWnd* pWnd = CWnd::FromHandle(hWnd);

	CClientDC dc(pWnd);

	::EnumFonts(dc, 0, (FONTENUMPROC)EnumFontProcEx, (LPARAM)pStringArray);

	//
	for (INT_PTR nIndex = 0; nIndex < pStringArray->GetCount(); nIndex++)
	{
		CString strFont = pStringArray->GetAt(nIndex);
		//
		Gdiplus::FontFamily fontFamily(strFont);
		//
		if (fontFamily.GetLastStatus() != Gdiplus::Ok)
		{
			pStringArray->RemoveAt(nIndex);
			nIndex--;
		}
	}

	return TRUE;
}