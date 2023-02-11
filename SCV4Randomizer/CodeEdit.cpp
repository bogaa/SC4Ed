// CodeEdit.cpp : implementation file
//

#include "stdafx.h"
#include "SCV4Randomizer.h"
#include "CodeEdit.h"


// CodeEdit

IMPLEMENT_DYNAMIC(CodeEdit, CEdit)

CodeEdit::CodeEdit()
{

}

CodeEdit::~CodeEdit()
{
}


BEGIN_MESSAGE_MAP(CodeEdit, CEdit)
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()



// CodeEdit message handlers




void CodeEdit::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CEdit::OnLButtonUp(nFlags, point);
	SetSel(0, -1, TRUE);
	Copy();
}
