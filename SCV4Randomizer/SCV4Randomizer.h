
// SCV4Randomizer.h : main header file for the SCV4Randomizer application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols

#include <string>

// CUSTOM

#include "SC4Core.h"
extern SC4Core nmmx;

// CSCV4RandomizerApp:
// See SCV4Randomizer.cpp for the implementation of this class
//

class CSCV4RandomizerApp : public CWinApp
{
public:
	CSCV4RandomizerApp();

	std::string FileName = "";
	bool fileOk = false;
	std::string CodeName = "";

// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation
	afx_msg void OnAppAbout();
	afx_msg void OnFileOpen();
	afx_msg void OnFileCode();
	DECLARE_MESSAGE_MAP()


};

extern CSCV4RandomizerApp theApp;
