// $Id$
#if !defined(__DOCVIEW_H__)
#define __DOCVIEW_H__

#define     HINT_UPDATE_ALL_DOCUMENTS     -1

template <class TDoc> class CView;
class CDocTemplateBase;

///////////////////////////////////////////////////////////////////////////////
// Classes in this file:
//
// CDocumentBase
//   abstract class just for polymorphism
//
// CDocument
//
// CView
//
// CViewImpl
//
// CCreateContext
//  Helper struct for frame creation
//

class CDocumentBase
{
public:
	~CDocumentBase() {}
	virtual BOOL  OnNewDocument() = 0;
	virtual BOOL  OnOpenDocument(LPCTSTR lpDocumentPath) = 0;
	virtual BOOL  SetDocumentName(LPCTSTR lpDocumentName, bool bSetModifiedFlag) = 0;
};


template <typename T, typename TDocTemplate = CDocTemplateBase, typename TDoc = CDocumentBase>
class CDocument : public TDoc
{
public:
	CDocument()
	{
		m_bModified = FALSE;
	}

	virtual ~CDocument() {}

	void UpdateAllViews( CView <T>* pSender, LPARAM lHint = 0,
	                     LPVOID pHint = NULL )
	{
		int count = m_aViews.GetSize();

		for ( int i = 0; i < count; i++ )
		{
			CView <T>* pView = m_aViews[ i ];

			if ( pView != ( CView <T>* ) pSender )
				pView->Update( pSender, lHint, pHint );
		}
		OnAllViewsUpdateDone( pSender, lHint, pHint );
	}

	virtual void OnAllViewsUpdateDone( CView <T>* pSender, LPARAM lHint,
	                                   LPVOID pHint )
	{ }

	CView<T>* AddView( CView<T>* pView )
	{
		pView->SetDocument( static_cast<T*>( this ) );
		CView <T>* pV = static_cast<CView <T>*>( pView );
		m_aViews.Add( pV );
		return pView;
	}

	void RemoveView( CView<T>* pView )
	{
		int count = m_aViews.GetSize();

		for ( int i = 0; i < count; i++ )
		{
			if ( m_aViews[ i ] == pView )
			{
				m_aViews.RemoveAt( i );
				pView->SetDocument( NULL );
				break;
			}
		}
	}

	int GetNumViews() const
	{
		return m_aViews.GetSize();
	}

	CView<T>* GetView( const int pos )
	{
		_ASSERTE( pos < m_aViews.GetSize() );

		return m_aViews[ pos ];
	}

	BOOL IsModified() const
	{
		return m_bModified;
	}

	void SetModifiedFlag( BOOL bModified = TRUE )
	{
		m_bModified = bModified;
	}

	TDocTemplate* GetDocTemplate()
	{
		return m_pDocTemplate;
	}

protected:
	CSimpleArray<CView<T>*> m_aViews;
	BOOL m_bModified;

public:
	TDocTemplate* m_pDocTemplate;
};


/* abstract class for all the views belonging to a document */
template <class TDoc>
class CView
{
protected:
	TDoc* m_pDocument;

public:
	virtual void Update( CView <TDoc>* pSender, LPARAM lHint,
						 LPVOID pHint ) = 0;

	TDoc* GetDocument()
	{
		return m_pDocument;
	}

	void SetDocument( TDoc* pDoc )
	{
		m_pDocument = pDoc;
	}

	/* useful function for updating all docs from a particular template
	   in MDI architectures */
	void UpdateAllDocs(LPARAM lHint = HINT_UPDATE_ALL_DOCUMENTS, LPVOID pHint = NULL)
	{
		CDocTemplateBase * pDocTemplate = m_pDocument->GetDocTemplate();
		int ndocs = pDocTemplate->GetNumDocs();

		for ( int j = 0; j < ndocs; j++ )
		{
			TDoc* pDoc = static_cast<TDoc*>( pDocTemplate->GetDocument( j ) );

			if ( pDoc )
				pDoc->UpdateAllViews( this, lHint, pHint );
		}
	}
};


/* base class for view implementations */
template <class T, class TView>
class CViewImpl : public TView
{
public:
	virtual void Update( TView* pSender, LPARAM lHint, LPVOID pHint )
	{
		T * pT = static_cast<T*>( this );
		pT->OnUpdate( pSender, lHint, pHint );
	}

	virtual void OnInitialUpdate()
	{
		T * pT = static_cast<T*>( this );
		pT->OnUpdate( 0, 0, 0 );
	}

	HWND GetParentFrame()
	{
		T * pT = static_cast<T*>( this );
		HWND hWnd = pT->GetParent();

		while ( !( GetWindowLong( hWnd, GWL_EXSTYLE ) & WS_EX_MDICHILD ) )
			hWnd = ::GetParent( hWnd );

		return hWnd;
	}

};


template <class TDoc, class TView>
struct CCreateContext
{
	TDoc* m_pCurrentDoc;
	TView* m_pCurrentView;
	CDocTemplateBase* m_pNewDocTemplate;

	CCreateContext()
	{
		memset( this, 0, sizeof( *this ) );
	}
};

#endif  // !defined(__DOCVIEW_H__)
