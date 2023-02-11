
// SCV4RandomizerDoc.h : interface of the CSCV4RandomizerDoc class
//


#pragma once


class CSCV4RandomizerDoc : public CDocument
{
protected: // create from serialization only
	CSCV4RandomizerDoc();
	DECLARE_DYNCREATE(CSCV4RandomizerDoc)

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// Implementation
public:
	virtual ~CSCV4RandomizerDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	virtual BOOL DoSave(LPCTSTR lpszPathName, BOOL bReplace);

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// Helper function that sets search content for a Search Handler
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
};
