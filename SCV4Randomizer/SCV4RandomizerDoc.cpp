
// SCV4RandomizerDoc.cpp : implementation of the CSCV4RandomizerDoc class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "SCV4Randomizer.h"
#endif

#include "SCV4RandomizerDoc.h"

#include <propkey.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CSCV4RandomizerDoc

IMPLEMENT_DYNCREATE(CSCV4RandomizerDoc, CDocument)

BEGIN_MESSAGE_MAP(CSCV4RandomizerDoc, CDocument)
END_MESSAGE_MAP()


// CSCV4RandomizerDoc construction/destruction

CSCV4RandomizerDoc::CSCV4RandomizerDoc()
{
	// TODO: add one-time construction code here

}

CSCV4RandomizerDoc::~CSCV4RandomizerDoc()
{
}

BOOL CSCV4RandomizerDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}




// CSCV4RandomizerDoc serialization

void CSCV4RandomizerDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

#ifdef SHARED_HANDLERS

// Support for thumbnails
void CSCV4RandomizerDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// Modify this code to draw the document's data
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// Support for Search Handlers
void CSCV4RandomizerDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// Set search contents from document's data. 
	// The content parts should be separated by ";"

	// For example:  strSearchContent = _T("point;rectangle;circle;ole object;");
	SetSearchContent(strSearchContent);
}

void CSCV4RandomizerDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = NULL;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != NULL)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CSCV4RandomizerDoc diagnostics

#ifdef _DEBUG
void CSCV4RandomizerDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CSCV4RandomizerDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CSCV4RandomizerDoc commands

BOOL CSCV4RandomizerDoc::DoSave(LPCTSTR lpszPathName, BOOL bReplace) {
	CFileDialog dlg(FALSE, (LPCTSTR)".smc", (LPCTSTR)theApp.CodeName.c_str(), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("SNES Files (*.smc;*.sfc)|*.smc; *.sfc|All Files (*.*)|*.*||"));
	auto r = dlg.DoModal();
	if (r != IDOK) return FALSE;
	
	strcpy(nmmx.filePath, dlg.GetPathName());
	nmmx.SaveAsRom(nmmx.filePath);
	nmmx.SaveRom(nmmx.filePath);

	return TRUE;
}
