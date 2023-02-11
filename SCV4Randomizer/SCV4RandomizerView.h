
// SCV4RandomizerView.h : interface of the CSCV4RandomizerView class
//

#pragma once

#include "CodeEdit.h"

class CSCV4RandomizerView : public CFormView
{
protected: // create from serialization only
	CSCV4RandomizerView();
	DECLARE_DYNCREATE(CSCV4RandomizerView)

public:
#ifdef AFX_DESIGN_TIME
	enum{ IDD = IDD_SCV4RANDOMIZER_FORM };
#endif

// Attributes
public:
	CSCV4RandomizerDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL PreTranslateMessage(MSG *pMsg);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnInitialUpdate(); // called first time after construct

// Implementation
public:
	virtual ~CSCV4RandomizerView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	void AddCode(std::string);

protected:
	CToolTipCtrl ToolTip;
	CodeEdit mCodeEdit;

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedRandomize();
	afx_msg void OnNMCustomdrawCommonDifficulty(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnEnChangeCommonSeed();
	afx_msg void OnDestroy();
};

#ifndef _DEBUG  // debug version in SCV4RandomizerView.cpp
inline CSCV4RandomizerDoc* CSCV4RandomizerView::GetDocument() const
   { return reinterpret_cast<CSCV4RandomizerDoc*>(m_pDocument); }
#endif

