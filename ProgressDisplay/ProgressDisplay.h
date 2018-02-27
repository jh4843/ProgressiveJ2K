
// ProgressDisplay.h : main header file for the ProgressDisplay application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols

#include "ProgressDisplayDoc.h"
#include "ProgressDisplayView.h"

// CProgressDisplayApp:
// See ProgressDisplay.cpp for the implementation of this class
//

class CProgressDisplayApp : public CWinAppEx
{
public:
	CProgressDisplayApp();

public:
	CProgressDisplayView* m_pProgView;

private:
	ULONG_PTR m_nGdiPlusToken;

public:
	CProgressDisplayView* GetProgView();

// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation
	BOOL  m_bHiColorIcons;

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CProgressDisplayApp theApp;
