
// SCV4Randomizer.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "SCV4Randomizer.h"
#include "MainFrm.h"

#include "SCV4RandomizerDoc.h"
#include "SCV4RandomizerView.h"
#include "CodeDialog.h"

#ifdef _DEBUG
#define DEBUG_NEW
#endif

SC4Core nmmx;

// CSCV4RandomizerApp

BEGIN_MESSAGE_MAP(CSCV4RandomizerApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, &CSCV4RandomizerApp::OnAppAbout)
	// Standard file based document commands
	//ON_COMMAND(ID_FILE_NEW, &CWinApp::OnFileNew)
	//ON_COMMAND(ID_FILE_OPEN, &CWinApp::OnFileOpen)
	ON_COMMAND(ID_FILE_OPEN, &CSCV4RandomizerApp::OnFileOpen)
	ON_COMMAND(ID_FILE_CODE, &CSCV4RandomizerApp::OnFileCode)
END_MESSAGE_MAP()

// CSCV4RandomizerApp construction

CSCV4RandomizerApp::CSCV4RandomizerApp()
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;
#ifdef _MANAGED
	// If the application is built using Common Language Runtime support (/clr):
	//     1) This additional setting is needed for Restart Manager support to work properly.
	//     2) In your project, you must add a reference to System.Windows.Forms in order to build.
	System::Windows::Forms::Application::SetUnhandledExceptionMode(System::Windows::Forms::UnhandledExceptionMode::ThrowException);
#endif

	// TODO: replace application ID string below with unique ID string; recommended
	// format for string is CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("SCV4Randomizer.AppID.NoVersion"));

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

// The one and only CSCV4RandomizerApp object

CSCV4RandomizerApp theApp;


// CSCV4RandomizerApp initialization

BOOL CSCV4RandomizerApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	EnableTaskbarInteraction(FALSE);

	// AfxInitRichEdit2() is required to use RichEdit control	
	// AfxInitRichEdit2();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));
	LoadStdProfileSettings(4);  // Load standard INI file options (including MRU)


	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CSCV4RandomizerDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CSCV4RandomizerView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);


	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);



	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The one and only window has been initialized, so show and update it
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();

	auto hIcon = LoadIcon(MAKEINTRESOURCE(IDI_ICON1));
	m_pMainWnd->SetIcon(hIcon, TRUE);

	return TRUE;
}

int CSCV4RandomizerApp::ExitInstance()
{
	//TODO: handle additional resources you may have added
	AfxOleTerm(FALSE);

	return CWinApp::ExitInstance();
}

// CSCV4RandomizerApp message handlers


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// App command to run the dialog
void CSCV4RandomizerApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// App command to run the dialog

// CSCV4RandomizerApp message handlers

void CSCV4RandomizerApp::OnFileOpen()
{
	// default Randomize button to disable
	CMainFrame *pMainWnd = (CMainFrame *)AfxGetMainWnd();
	auto v = pMainWnd->GetActiveView();
	CButton *b = (CButton *)v->GetDlgItem(IDC_RANDOMIZE);
	b->EnableWindow(false);
	fileOk = false;

	CFileDialog dlg(TRUE, nullptr, nullptr, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_FILEMUSTEXIST, _T("SNES Files (*.smc;*.sfc)|*.smc; *.sfc|All Files (*.*)|*.*||"));
	auto r = dlg.DoModal();
	if (r != IDOK) return;

	FileName = dlg.GetPathName();
	// check for valid ROM
	if (!nmmx.LoadNewRom(FileName.c_str()))
	{
		nmmx.FreeRom();
		return;
	}
	if (!nmmx.CheckROM())
	{
		//MessageBox(hWnd, "Wrong ROM. Please open MMX1, X2 or X3 ROM.", "Error", MB_ICONERROR);
		AfxMessageBox((LPCTSTR)"Wrong ROM. Please open Super Castlevania IV (US/JP) ROM.");
		nmmx.FreeRom();
		return;
	}
	if (nmmx.region == 0x1)
	{
		//MessageBox(hWnd, "Wrong ROM. Please open MMX1, X2 or X3 ROM.", "Error", MB_ICONERROR);
		AfxMessageBox((LPCTSTR)"JP ROM not currently supported");
		nmmx.FreeRom();
		return;
	}
	//m_pMainWnd->SetWindowText(FileName.c_str());
	nmmx.FreeRom();

	b->EnableWindow(true);
	fileOk = true;
}

void CSCV4RandomizerApp::OnFileCode()
{
	CodeDialog d;
	if (d.DoModal() == IDOK) {
		CMainFrame *pMainWnd = (CMainFrame *)AfxGetMainWnd();
		auto v = (CSCV4RandomizerView *)pMainWnd->GetActiveView();
		v->AddCode(std::string(d.Code));
	}
}
