// CodeDialog.cpp : implementation file
//

#include "stdafx.h"
#include "SCV4Randomizer.h"
#include "CodeDialog.h"
#include "afxdialogex.h"


// CodeDialog dialog

IMPLEMENT_DYNAMIC(CodeDialog, CDialogEx)

CodeDialog::CodeDialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_CODE, pParent)
	, Code(_T(""))
{

}

CodeDialog::~CodeDialog()
{
}

void CodeDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_ENTERCODE, Code);
}


BEGIN_MESSAGE_MAP(CodeDialog, CDialogEx)
	ON_EN_CHANGE(IDC_ENTERCODE, &CodeDialog::OnEnChangeEntercode)
END_MESSAGE_MAP()


// CodeDialog message handlers


void CodeDialog::OnEnChangeEntercode()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	CString t;
	GetDlgItemText(IDC_ENTERCODE, t);
	Code = t;
}
