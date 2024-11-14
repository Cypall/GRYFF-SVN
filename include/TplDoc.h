// $Id$
#ifndef __TPLDOC_H__
#define __TPLDOC_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __ATLCOLL_H__
   #error TplDoc.h requires <atlcoll.h> to be included first
#endif

#include  <utility>

class CDocumentBase;

namespace
{
	class FrameCloser
	{
	public:
		HWND  m_hWnd;
		FrameCloser(HWND hWnd=NULL) : m_hWnd(hWnd)
		{}
		~FrameCloser()
		{
			if (m_hWnd)
				::SendMessage(m_hWnd, WM_CLOSE, 1, 0x1313);
		}
		void Reset(HWND hWnd=NULL)
		{m_hWnd = hWnd;}
	};
}

///////////////////////////////////////////////////////////////////////////////
// Classes in this file:
//
// CDocTemplateBase
//   Abstract base class for doc templates
//
// CDocTemplate
//

class CDocTemplateBase
{
public:
	virtual CDocumentBase* GetDocument( int pos ) = 0;
	virtual int GetDocumentIndex( LPCTSTR lpszPathName ) const = 0;
	virtual int GetDocumentIndex( CDocumentBase* pDoc ) const = 0;
	virtual std::pair<HWND, CDocumentBase*> OpenDocumentFile( LPCTSTR lpszPathName, BOOL bMakeVisible = TRUE ) = 0;
	virtual int GetNumDocs() const = 0;
};


template <class TDoc, class TView, class TFrame, int nID>
class CDocTemplate : public CDocTemplateBase
{
protected:
	CSimpleArray<TDoc*> m_aDocuments;

public:
	HWND m_hWndClient;

public:
	CDocTemplate() : m_hWndClient(0)
	{
	}

	virtual ~CDocTemplate()
	{
		int ndocs = GetNumDocs();
		for ( int i = 0; i < ndocs; ++i )
		{
			delete m_aDocuments[ i ];
		}
		m_aDocuments.RemoveAll();
	}

	//#################
	// CDocTemplateBase
	//#################
	CDocumentBase* GetDocument( int pos )
	{
		return m_aDocuments[ pos ];
	}

	//#################
	// CDocTemplateBase
	//#################
	// Returns -1 if not found
	int GetDocumentIndex( LPCTSTR lpszPathName ) const
	{
		bool  TestingUntitled(_tcschr(lpszPathName, _T('\\')) == NULL);
		int ndocs = GetNumDocs();
		for ( int i = 0; i < ndocs; ++i )
		{
			if ( m_aDocuments[i]->MatchesPath(lpszPathName, TestingUntitled) )
			{
				return i;
			}
		}
		return -1;
	}

	//#################
	// CDocTemplateBase
	//#################
	// Returns -1 if not found
	int GetDocumentIndex( CDocumentBase* pDoc ) const
	{
		int ndocs = GetNumDocs();
		for ( int i = 0; i < ndocs; ++i )
		{
			if ( m_aDocuments[i] == pDoc )
			{
				return i;
			}
		}
		return -1;
	}

	//#################
	// CDocTemplateBase
	//#################
	std::pair<HWND, CDocumentBase*> OpenDocumentFile( LPCTSTR lpDocumentPath, BOOL bOpenAsUntitled = TRUE )
	{
		TDoc * pDoc = CreateNewDocument();
		if ( !pDoc )
		{
			return  std::pair<HWND, CDocumentBase*>(0, 0);
		}
		TFrame* pFrame = CreateNewFrame( pDoc, 0 );
		if ( !pFrame )
		{
			RemoveDocument(pDoc);
			delete pDoc;
			return  std::pair<HWND, CDocumentBase*>(0, 0);
		}
		FrameCloser closer_raii(pFrame->m_hWnd);

		if ( lpDocumentPath == 0 )
		{
			if ( !pDoc->OnNewDocument() )
			{
				return  std::pair<HWND, CDocumentBase*>(0, 0);
			}
		}
		else
		{
			if ( !pDoc->OnOpenDocument(lpDocumentPath) )
			{
				return  std::pair<HWND, CDocumentBase*>(0, 0);
			}
			pDoc->SetDocumentName(bOpenAsUntitled ? NULL : lpDocumentPath,
				bOpenAsUntitled ? true : false);
		}
		closer_raii.Reset();  // do not close
		pFrame->OnInitialUpdate();
		::SendMessage(pFrame->GetParent(), WM_MDIACTIVATE, (WPARAM)pFrame->operator HWND(), 0);
		pFrame->ShowWindow(SW_SHOW);
		pFrame->UpdateWindow();
		return  std::pair<HWND, CDocumentBase*>(pFrame->operator HWND(), pDoc);
	}
	//#################
	// CDocTemplateBase
	//#################
	int GetNumDocs() const
	{
		return m_aDocuments.GetSize();
	}

	virtual void AddDocument( TDoc* pDoc )
	{
		pDoc->m_pDocTemplate = this;
		m_aDocuments.Add( pDoc );
	}

	virtual TDoc* RemoveDocument( TDoc* pDoc )
	{
		int ndocs = GetNumDocs();

		for ( int i = 0; i < ndocs; i++ )
		{
			if ( m_aDocuments[ i ] == pDoc )
			{
				m_aDocuments.RemoveAt( i );
				break;
			}
		}
		return pDoc;
	}


	TFrame* CreateNewFrame( TDoc* pDoc, TFrame* pOther )
	{
		CCreateContext <TDoc, TView> context;
		context.m_pCurrentDoc = pDoc;
		context.m_pNewDocTemplate = this;

		TFrame::GetWndClassInfo().m_uCommonResourceID = nID;
		TFrame* pFrame = new TFrame;
		pFrame->CreateEx( m_hWndClient, NULL, NULL, TFrame::GetWndStyle(0) & (~WS_VISIBLE), 0, &context );

		return pFrame;
	}

	TDoc* CreateNewDocument()
	{
		TDoc * pDoc = new TDoc;
		AddDocument( pDoc );

		return pDoc;
	}

};




#endif // __TPLDOC_H__
