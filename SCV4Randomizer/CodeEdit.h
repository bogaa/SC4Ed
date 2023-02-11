#pragma once


// CodeEdit

class CodeEdit : public CEdit
{
	DECLARE_DYNAMIC(CodeEdit)

public:
	CodeEdit();
	virtual ~CodeEdit();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
};


