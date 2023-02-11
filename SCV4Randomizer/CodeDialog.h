#pragma once


// CodeDialog dialog

class CodeDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CodeDialog)

public:
	CodeDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CodeDialog();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_CODE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString Code;
	afx_msg void OnEnChangeEntercode();
};
