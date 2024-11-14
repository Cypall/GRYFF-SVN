// $Id: MyTreeCtrl.h 9 2005-08-12 17:33:08Z Rasqual $

// *
// *   gryff, a GRF archive management utility, src/MyTreeCtrl.h
// *   Copyright (C) 2003-2005 Rasqual Twilight
// *
// *   This program is distributed in the hope that it will be useful,
// *   but WITHOUT ANY WARRANTY; without even the implied warranty of
// *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// *

// * MyTreeCtrl.h
// * Tree view control definition
// *
// *@author $Author: Rasqual $
// *@version $Revision: 9 $
// *

#ifndef __MYTREECTRL_H__
#define __MYTREECTRL_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __ATLCTRLS_H__
   #error MyTreeCtrl.h requires <atlctrls.h> to be included first
#endif

class CGrfView;
class CMyImageList;

///////////////////////////////////////////////////////////////////////////////
// Classes in this file:
//
// CTreeTraits
//   typedef
//
// CMyTreeCtrl
//

typedef CWinTraitsOR<
WS_BORDER |
TVS_HASBUTTONS | TVS_EDITLABELS |
TVS_SHOWSELALWAYS |
TVS_LINESATROOT | TVS_INFOTIP
 | TVS_TRACKSELECT
> CTreeTraits;

///////////////////////////////////////////////////////////////////////////////

class CMyTreeCtrl :
  public CWindowImpl<CMyTreeCtrl, CTreeViewCtrlEx, CTreeTraits>,
  public IDropTarget
{
	friend CGrfView;
public:
	typedef struct tagGrfTreeViewCreationInfo
	{
		HWND             hWnd;
		CMyImageList     *pmiml;
	} GRFTREEVIEWCREATIONINFO, UNALIGNED *LPGRFTREEVIEWCREATIONINFO;

public:
    DECLARE_WND_SUPERCLASS(NULL, CTreeViewCtrlEx::GetWndClassName())

	//////////////////////// - static methods - /////////////////////////////
public:
	/**
		Returns whether a supported clipboard format is detected when
		dragging an object over the window
	**/
	static bool  IsAcceptedFormat_(IDataObject *);

	////////////////// - Static Data Members - ///////////////////////
protected:
	/**
		Shared CMyImageList instance
		[sss][xxxxxx]
		First part are system imagelist images, then index-fixed known images
	**/
	static CMyImageList * static_pmiml;
	static ATL::CAtlMap< CString, std::pair<int, CString> > * static_pimgmap;

	///////////////////////// - CTOR / DTOR - /////////////////////////////
public:
	/**
		Performs initialization.
	**/
	CMyTreeCtrl();
	/**
		Performs cleanup.
	**/
	~CMyTreeCtrl();

	///////////////////////// - data members - /////////////////////////////
protected:
	HWND             m_hWndParentView;
	HTREEITEM            m_SelectionTarget;  // selected target when changing
    CComPtr<IDropTargetHelper>  m_pDropTargHlp;
    CComPtr<IDataObject>  m_pDataObject;  // obj being dragged
	LONG                 m_RefCount;


	///////////////////////// - Methods - /////////////////////////////
public:
	//----- Implement IDropTarget for drag and drop support -----//
	/**
		IUnknown QueryInterface()
		Supported interfaces are IUnknown + IDropTarget
	**/
	virtual HRESULT STDMETHODCALLTYPE  QueryInterface(REFIID, LPVOID *);

	/**
		IUnknown AddRef()
		Increase reference count
	**/
	virtual ULONG STDMETHODCALLTYPE  AddRef();

	/**
		IUnknown Release()
		Decrease reference count
	**/
	virtual ULONG STDMETHODCALLTYPE  Release();

	/**
		IDropTarget DragEnter()
	**/
	virtual HRESULT STDMETHODCALLTYPE  DragEnter(IDataObject *, DWORD, POINTL, DWORD *);

	/**
		IDropTarget DragOver()
	**/
	virtual HRESULT STDMETHODCALLTYPE  DragOver(DWORD, POINTL, DWORD *);

	/**
		IDropTarget DragLeave()
	**/
	virtual HRESULT STDMETHODCALLTYPE  DragLeave();

	/**
		IDropTarget Drop()
	**/
	virtual HRESULT STDMETHODCALLTYPE  Drop(IDataObject *pDataObject, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect);

	//----- Getters -----//
public:
	CMyImageList * GetImageListPtr() const
	{
		return static_pmiml;
	}

protected:
	//----- Window message loop related handling -----//
	////----- window message handlers -----////
	/**
		Called when a new child frame is created.
		The MDICREATESTRUCT lpCreateParams has a lParam member
		which is a pointer of to a CCreateContext<CGrfDoc, CGrfView> structure.
		It contains information that is used to create the view subordinated to the frame.
		The view subscribe for notifications with the CGrfDoc AddView() method,
		and unsubscribes when the view is destroyed, using RemoveView()
	**/
	LRESULT  OnCreate(LPCREATESTRUCT);
	void  OnDestroy();
	LRESULT  OnEraseBackground(HDC);
	void  OnPaint(HDC);
	LRESULT  OnSetFocus(UINT, WPARAM, LPARAM, BOOL& bHandled);

	////----- reflected NM handlers -----////
	LRESULT  OnClick(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& bHandled);
	LRESULT  OnSelChanged(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& bHandled);
	LRESULT  OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	//----- methods: Others -----//
public:
	void Init();
	CTreeItem InsertRoot( LPCTSTR strDocumentName );
	CTreeItem InsertFolder(	const CString &strFolderName, LPARAM lParam, HTREEITEM hParent, HTREEITEM hInsertAfter);
	CTreeItem InsertFolderByFullPath( const CString &strFolderName, LPARAM lParam, HTREEITEM hInsertAfter);
	CTreeItem MkdirRecursive(const CTreeItem &rootitem, LPCTSTR dirname, HTREEITEM hInsertAfter = TVI_SORT);
	CTreeItem  GetDir(LPCTSTR dirname);
	CTreeItem  RmdirRecursive(CTreeItem &ti);
	void  PerformSelChange(CTreeItem &ti);

	///////////////////////// - Message loop - /////////////////////////////

protected:
    BEGIN_MSG_MAP(CMyTreeCtrl)
	  MSG_WM_CREATE(OnCreate)
	  MSG_WM_DESTROY(OnDestroy)
	  MSG_WM_ERASEBKGND(OnEraseBackground)
	  MSG_WM_PAINT(OnPaint)
      MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
	  MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
      REFLECTED_NOTIFY_CODE_HANDLER(NM_CLICK, OnClick)
      REFLECTED_NOTIFY_CODE_HANDLER(TVN_SELCHANGED, OnSelChanged)
      DEFAULT_REFLECTION_HANDLER()
    END_MSG_MAP()

};


#endif //__MYTREECTRL_H__
