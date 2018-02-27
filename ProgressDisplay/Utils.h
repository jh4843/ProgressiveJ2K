#pragma once


class CUtils  
{
public:
	CUtils();
	virtual ~CUtils();

	static CString GetExeRootPath();
	static CString GetExeRootPath(CString strProcessName);
	static CString GetExeFileName();
	static BOOL ExtractQATCommands(CString strQATCommands, CList<UINT,UINT>* plistQATCommands);
	static CString FormatStrQATCommands(CList<UINT,UINT>* plistQATCommands);
	// File 
	static BOOL CalcFileSize(CString strFilePath, double& fFileSize);
	static ULONGLONG GetTotalFileSize(CString strFolder);
	static CString ExtractFileName(CString strFileName);
	static CString ExtractFileNameOnly(CString strFileName);
	static CString ExtractWholeFileName(CString strFileName);
	static CString ExtractFilePath(CString strFullPath, BOOL bWithLastDelimiter=FALSE);
	static CString ExtractFileExt(CString strFileName, BOOL bMakeUpperCase = TRUE);
	static CString ExtractString(CString strString, CString strDelimiter, BOOL bReverse);
	static BOOL IsFolderEmpty(CString strFolderPath);
	static int GetFileListInCurrentFolder(CStringList *pFileNameList, LPCTSTR lpszDir, LPCTSTR lpszFileExt = NULL);
	static int GetFileListInFolder(CStringList* pFileNameList, LPCTSTR lpszDir, LPCTSTR lpszFileExt = NULL);
	static int GetFileListInFolder(CStringArray* pAryFileName, LPCTSTR lpszDir, BOOL& bStopRead, BOOL& bSubDir);
	static int GetFolderListInFolder(CStringList* pFileNameList, LPCTSTR lpszDir);
	// Folder
	static BOOL CreateFolder(CString strFolderPath);
	static BOOL	RemoveFolder(CString strFolderPath);
	static BOOL DeleteFolder(const CString &strFolder);
	// System Folder
	static CString GetCommonAppDataPath();
	static CString GetVieworksAppDataPath();
	// Convert
	static CString IntToStr(int nData);
	static CString LongToStr(long nData);
	static CString FloatToStr(float fData);
	static CString BoolToStr(BOOL bBool);
	static CString ByteToStr(BYTE bByte);
	//
	static int StrToInt(CString strValue);
	static BOOL StrToBool(CString strValue);
	static double StrToDouble(CString strValue);
	static int GetTwipSize(CString strValue);
	static CString TwipsToPointString(int nTwips);
	static BOOL IsWhiteSpaceOnly(CString strValue);
	// Base64
	static CStringA EncodeStringAsBase64(CStringW strSource);
	static CStringW DecodeStringAsBase64(CStringA strSource);
	// 
	static CString FormatDICOMTime(CString strTime, CString strFormat = _T("%H:%M:%S"));
	static CString FormatDICOMDate(CString strDate, CString strFormat = _T("%Y-%m-%d"));
	static CString GetDateTimeString(COleDateTime& dtDateTime, CString strFormat = _T(""));
	static BOOL IsValidPatientBirthDate(COleDateTime& dtBirthDate);
	static BOOL IsValidDateForOleDateTime(INT_PTR nYear, INT_PTR nMonth, INT_PTR nDay, INT_PTR nHour=0, INT_PTR nMin=0, INT_PTR nSec=0);
	//
	static BOOL IsProcessRunning(CString strProcessName);
	static BOOL ExecuteProcess(CString strFilePath, CString strArg);
	static BOOL ExecuteProcess(CString strFilePath, CString strArg, ULONG fMask);
	static BOOL KillProcessByName(CString strProcessName);
	//
	static HICON LoadIcon(CString strIconPath, INT_PTR nSize);
	static CString GetSystemErrorMessage(DWORD dwMessageID);
	//
	static BOOL GetFileVersionInfo(CString strFilePath, DWORD dwVersion[4]);
	static void CreateFileAsUtf8(CString strFilePath);
	//
	static BOOL GetSystemFontNames(CStringArray* pStringArray);
};
