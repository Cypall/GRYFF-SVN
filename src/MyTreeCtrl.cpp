// $Id: MyTreeCtrl.cpp 9 2005-08-12 17:33:08Z Rasqual $

// *
// *   gryff, a GRF archive management utility, src/MyTreeCtrl.cpp
// *   Copyright (C) 2003-2005 Rasqual Twilight
// *
// *   This program is distributed in the hope that it will be useful,
// *   but WITHOUT ANY WARRANTY; without even the implied warranty of
// *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// *

// * MyTreeCtrl.cpp
// * Tree view control definition (implementation)
// *
// *@author $Author: Rasqual $
// *@version $Revision: 9 $
// *

#include  "stdwtl.h"
#pragma hdrstop
#include  "resource.h"
#include  "MyTreeCtrl.h"

#include  "ProcComm.h"

#define NO_OFN_DIALOG
#include  "fnmanip.h"
#include  "imglist.h"

using std::pair;

extern UINT  UCF_GRYFF_ENTRIES;
extern UINT  UCF_GRYFF_ENTRIES_INTERNAL;

CMyImageList *CMyTreeCtrl::static_pmiml = 0;


///////////////////////////////////////////////////////////////////////////////

CMyTreeCtrl::CMyTreeCtrl() : m_hWndParentView(0), m_SelectionTarget(0), m_pDropTargHlp(0), m_pDataObject(0), m_RefCount(0)
{
}

///////////////////////////////////////////////////////////////////////////////

CMyTreeCtrl::~CMyTreeCtrl()
{
	if ( m_RefCount >= 1 )
	{
		this->Release();
	}
	ATLVERIFY(m_RefCount == 0);
}

///////////////////////////////////////////////////////////////////////////////

/**
	IUnknown QueryInterface()
	Supported interfaces are IUnknown + IDropTarget
**/
HRESULT STDMETHODCALLTYPE  CMyTreeCtrl::QueryInterface(REFIID iid, LPVOID *ppvObject)
{
	if ( ppvObject == 0 )
	{
		return E_INVALIDARG;
	}
	if ( IsEqualIID(iid, IID_IUnknown) )
	{
		this->AddRef();
		*ppvObject = reinterpret_cast<IUnknown *>(this);
		return S_OK;
	}
	if ( IsEqualIID(iid, IID_IDropTarget) )
	{
		this->AddRef();
		*ppvObject = reinterpret_cast<IDropTarget *>(this);
		return S_OK;
	}
	*ppvObject = 0;
	return E_NOINTERFACE;
}

///////////////////////////////////////////////////////////////////////////////

/**
	IUnknown AddRef()
	Increase reference count
**/
ULONG STDMETHODCALLTYPE  CMyTreeCtrl::AddRef()
{
	return  static_cast<ULONG>(::InterlockedIncrement(&m_RefCount));
}

///////////////////////////////////////////////////////////////////////////////

/**
	IUnknown Release()
	Decrease reference count
**/
ULONG STDMETHODCALLTYPE  CMyTreeCtrl::Release()
{
	return static_cast<ULONG>(::InterlockedDecrement(&m_RefCount));
}

///////////////////////////////////////////////////////////////////////////////

LRESULT CMyTreeCtrl::OnCreate(LPCREATESTRUCT lpcs)
{
	LPGRFTREEVIEWCREATIONINFO lpgtvci = reinterpret_cast<LPGRFTREEVIEWCREATIONINFO>(lpcs->lpCreateParams);
	if ( static_pmiml == 0 )
	{
		static_pmiml = lpgtvci->pmiml;
	}
	m_hWndParentView = lpgtvci->hWnd;

	::CoCreateInstance(CLSID_DragDropHelper, NULL, CLSCTX_INPROC_SERVER,
		IID_IDropTargetHelper, reinterpret_cast<LPVOID *>(&m_pDropTargHlp));

	if ( m_pDropTargHlp == 0 )
	{
		MessageBox(_T("Error when creating the IDropTargetHelper instance"), CString(MAKEINTRESOURCE(IDS_INIT_ERROR_MSGBOX_TITLE)), MB_ICONERROR | MB_OK);
		return -1;
	}
	m_RefCount = 1;

	::RegisterDragDrop(m_hWnd, this);

	SetMsgHandled(FALSE);  // Chaining base methods, if any.
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

void CMyTreeCtrl::OnDestroy()
{
	if ( m_RefCount >= 1 )
	{
		::RevokeDragDrop(m_hWnd);
	}
	SetMsgHandled(FALSE);  // Chaining base methods
}

///////////////////////////////////////////////////////////////////////////////

void CMyTreeCtrl::Init()
{
	SetIndent(0);
	  // Associate the image list with the tree-view control.
	TreeView_SetImageList(m_hWnd, static_pmiml->m_himlSmall, TVSIL_NORMAL);
	SetUnicodeFormat();
	//::SetWindowLong(m_hWnd, GWL_EXSTYLE, ::GetWindowLong(m_hWnd, GWL_EXSTYLE) | WS_EX_LEFT | WS_EX_RIGHTSCROLLBAR);
}

///////////////////////////////////////////////////////////////////////////////

LRESULT  CMyTreeCtrl::OnEraseBackground(HDC)
{
	SetMsgHandled(FALSE);
	return 0;
//	return 1;
}

///////////////////////////////////////////////////////////////////////////////

void  CMyTreeCtrl::OnPaint(HDC hDC)
{
	SetMsgHandled(FALSE);
#if 0
RECT rect;
	// redraw only if necessary
	if ( this->GetUpdateRect(&rect, FALSE) )
	{
	CPaintDC  dc(m_hWnd);
		  // Paint to a memory device context to reduce screen flicker.
		  // When memDC goes out of scope, it paints back into the original DC
	CMemDC  memDC(dc.m_hDC, &rect);
		this->DefWindowProc(WM_PAINT, (WPARAM)memDC.m_hDC, 0);
	}
#endif
}

///////////////////////////////////////////////////////////////////////////////

LRESULT  CMyTreeCtrl::OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	::SendMessage(m_hWndParentView, UWM_SUBVIEWGOTFOCUS, TREEVIEW_ID, 0);
	bHandled = FALSE;
	return 1;
}

///////////////////////////////////////////////////////////////////////////////

CTreeItem CMyTreeCtrl::InsertRoot( LPCTSTR strDocumentName )
{
	TVINSERTSTRUCT tvins = { 0 };
	tvins.hParent = TVI_ROOT;
	tvins.hInsertAfter = TVI_LAST;
	tvins.itemex.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
	tvins.itemex.pszText = (LPTSTR)strDocumentName;
	tvins.itemex.cchTextMax = MAX_PATH;
	tvins.itemex.iImage = static_pmiml->m_igrfArchive;
	tvins.itemex.iSelectedImage = static_pmiml->m_igrfArchive;
	tvins.itemex.iIntegral = 1;
	tvins.itemex.lParam = 0;

	CTreeItem tiroot(this->InsertItem(&tvins));
	tiroot.SetState(TVIS_BOLD, TVIS_BOLD);
	return tiroot;
}

///////////////////////////////////////////////////////////////////////////////

CTreeItem CMyTreeCtrl::InsertFolder( const CString &strFolderName,
                                     LPARAM lParam, HTREEITEM hParent,
                                     HTREEITEM hInsertAfter )
{
	CTreeItem ti(this->InsertItem(TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM,
	                        strFolderName, static_pmiml->m_iclosedFolder, static_pmiml->m_iopenFolder,
	                        0, 0, lParam, hParent, hInsertAfter));
	if ( hParent != TVI_ROOT )
	{
		TVITEM tvitem;
		tvitem.hItem = hParent;
		tvitem.mask = TVIF_CHILDREN;
		tvitem.cChildren = 1;
		TreeView_SetItem(m_hWnd, &tvitem);
	}
	return ti;
}

///////////////////////////////////////////////////////////////////////////////

CTreeItem  CMyTreeCtrl::InsertFolderByFullPath( const CString &strFolderName, LPARAM lParam, HTREEITEM hInsertAfter)
{
	CTreeItem ti(this->MkdirRecursive(CTreeItem(this->GetRootItem(), this), strFolderName));
	ti.SetData(lParam);
	return ti;
}

///////////////////////////////////////////////////////////////////////////////

CTreeItem CMyTreeCtrl::MkdirRecursive(const CTreeItem &rootitem, LPCTSTR dirname, HTREEITEM hInsertAfter)
{

	CString parse(dirname);
	CString dn;

	CTreeItem ti(rootitem);

	while ( !parse.IsEmpty() )
	{
		::GetTopDir(parse, dn, parse);

		// Find dn in the children
		if ( !ti.HasChildren() )
		{
			ti = this->InsertFolder(dn, static_cast<LPARAM>(0),	ti, hInsertAfter);
		}
		else
		{
			CTreeItem child = ti.GetChild();
			bool bFound = false;
			do
			{
				CString text;
				child.GetText(text);
				if ( text == dn )
				{
					ti = child;
					bFound = true;
					break;
				}
				if ( TreeView_GetNextSibling(*child.m_pTreeView, child.m_hTreeItem) == NULL )
				{
					break;
				}
				child = child.GetNextSibling();
			} while ( 1 );

			if ( !bFound )
			{
				ti = this->InsertFolder(dn, static_cast<LPARAM>(0),	ti, hInsertAfter);
			}
		}
	}
	return ti;
}

///////////////////////////////////////////////////////////////////////////////

CTreeItem CMyTreeCtrl::GetDir(LPCTSTR dirname)
{
	CString parse(dirname);
	CString dn;

	CTreeItem ti(this->GetRootItem());

	while ( !parse.IsEmpty() )
	{
		::GetTopDir(parse, dn, parse);

		// Find dn in the children
		CTreeItem child = ti.GetChild();
		bool bFound = false;
		do
		{
			CString text;
			child.GetText(text);
			if ( text == dn )
			{
				ti = child;
				bFound = true;
				break;
			}
			if ( TreeView_GetNextSibling(*child.m_pTreeView, child.m_hTreeItem) == NULL )
			{
				break;
			}
			child = child.GetNextSibling();
		} while ( 1 );

		if ( !bFound )
		{
			return 0;
		}
	}
	return ti;
}

///////////////////////////////////////////////////////////////////////////////

CTreeItem CMyTreeCtrl::RmdirRecursive(CTreeItem &ti)
{
CTreeItem parent(ti.GetParent());
	if ( parent.m_hTreeItem != 0 &&
	     parent.m_hTreeItem != TVI_ROOT &&
	     TreeView_GetNextSibling(*ti.m_pTreeView, ti.m_hTreeItem) == NULL &&
	     TreeView_GetPrevSibling(*ti.m_pTreeView, ti.m_hTreeItem) == NULL )
	{
		TVITEM tvitem;
		tvitem.hItem = parent.m_hTreeItem;
		tvitem.mask = TVIF_CHILDREN;
		tvitem.cChildren = 0;  // Remove the bullet
		ti.m_pTreeView->SetItem(&tvitem);
	}

	ti.Delete();
	return parent;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT CMyTreeCtrl::OnClick(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& bHandled)
{
	// Get the item we're clicking on
	DWORD dwPos = ::GetMessagePos();
	POINT pt = { GET_X_LPARAM(dwPos), GET_Y_LPARAM(dwPos) };
	this->ScreenToClient(&pt);
	UINT uFlags;
	CTreeItem hItem = HitTest(pt, &uFlags);
	CTreeItem curSel = this->GetSelectedItem();

	// Send click event
	if ( hItem && (uFlags & (LVHT_ONITEMICON | LVHT_ONITEMLABEL)) && hItem.m_hTreeItem != curSel.m_hTreeItem )
	{
		m_SelectionTarget = hItem.m_hTreeItem;  // To avoid double refresh
		this->PerformSelChange(hItem);
	}

	bHandled = FALSE;
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT CMyTreeCtrl::OnSelChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled)
{
	NMTREEVIEW *pnmtv = reinterpret_cast<NMTREEVIEW *>(pnmh);
	CTreeItem entry(pnmtv->itemNew.hItem, this);

	// FIXME: TODO: (x.x) enhancement: delay navigation like in Explorer

	if ( m_SelectionTarget != entry.m_hTreeItem )
	{
		this->PerformSelChange(entry);
		m_SelectionTarget = 0;
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////

void  CMyTreeCtrl::PerformSelChange(CTreeItem &ti)
{
	::SendMessage(m_hWndParentView, UWM_NAVIGATE_HTREE, FALSE, reinterpret_cast<LPARAM>(static_cast<HTREEITEM>(ti)));
	ti.Expand();
}

///////////////////////////////////////////////////////////////////////////////

LRESULT  CMyTreeCtrl::OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	::PostMessage(m_hWndParentView, uMsg, wParam, lParam);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

/**
	IDropTarget DragEnter()
**/
HRESULT STDMETHODCALLTYPE  CMyTreeCtrl::DragEnter(IDataObject *pDataObject, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
HRESULT  func_hr(S_OK);
	m_pDataObject = 0;
POINT pt = { ptl.x, ptl.y };
	if ( m_pDropTargHlp )
	{
		m_pDropTargHlp->DragEnter(m_hWnd, pDataObject, &pt, *pdwEffect);
	}
	  // Inspect dragged data object
	if ( IsAcceptedFormat_(pDataObject) )
	{
	TVHITTESTINFO hti = { 0 };
		hti.pt = pt;
		this->ScreenToClient(&(hti.pt));
		this->HitTest(&hti);

		m_pDataObject = pDataObject;
		if ( hti.flags != TVHT_NOWHERE )
		{
			*pdwEffect = (grfKeyState & MK_CONTROL) ? DROPEFFECT_COPY : DROPEFFECT_MOVE;
		}
		else
		{
			*pdwEffect = DROPEFFECT_NONE;
		}
	}
	else
	{
		*pdwEffect = DROPEFFECT_NONE;
	}
	return func_hr;
}

///////////////////////////////////////////////////////////////////////////////

/**
	IDropTarget DragOver()
**/
HRESULT STDMETHODCALLTYPE  CMyTreeCtrl::DragOver(DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
	if ( !m_pDataObject )
	{
		return E_INVALIDARG;
	}
HRESULT  func_hr(S_OK);
	if ( m_pDropTargHlp )
	{
	POINT pt = { ptl.x, ptl.y };
		m_pDropTargHlp->DragOver(&pt, *pdwEffect);
	}
	  // Inspect dragged data object
	if ( m_pDropTargHlp && IsAcceptedFormat_(m_pDataObject) )
	{
	POINT pt = { ptl.x, ptl.y };
	TVHITTESTINFO hti = { 0 };
		hti.pt = pt;
		this->ScreenToClient(&(hti.pt));
		this->HitTest(&hti);

		if ( hti.flags != TVHT_NOWHERE )
		{
			*pdwEffect = (grfKeyState & MK_CONTROL) ? DROPEFFECT_COPY : DROPEFFECT_MOVE;
			this->SelectDropTarget(hti.hItem);
		}
		else
		{
			this->SelectDropTarget(NULL);
			*pdwEffect = DROPEFFECT_NONE;
		}
	}
	else
	{
		this->SelectDropTarget(NULL);
		*pdwEffect = DROPEFFECT_NONE;
	}
	return func_hr;
}

///////////////////////////////////////////////////////////////////////////////

/**
	IDropTarget DragLeave()
**/
HRESULT STDMETHODCALLTYPE  CMyTreeCtrl::DragLeave()
{
	if ( m_pDropTargHlp )
	{
		m_pDropTargHlp->DragLeave();
	}
	this->SelectDropTarget(NULL);
	//m_pDataObject->Release();//no need to with smart ptr
	return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

/**
	IDropTarget Drop()
**/
HRESULT STDMETHODCALLTYPE  CMyTreeCtrl::Drop(IDataObject *pDataObject, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
	if ( !m_pDropTargHlp )
	{
		return E_FAIL;
	}
POINT pt = { ptl.x, ptl.y };
	if ( m_pDropTargHlp )
	{
		m_pDropTargHlp->Drop(pDataObject, &pt, *pdwEffect);
	}
TVHITTESTINFO hti = { 0 };
	hti.pt = pt;
	this->ScreenToClient(&(hti.pt));
CTreeItem tidrop(this->HitTest(&hti));

HRESULT  func_hr(S_OK);
OleDropInfo odi;
	odi.phr = &func_hr;
	odi.pDataObject = pDataObject;
	odi.pDirent = reinterpret_cast<CGrfDirectoryEntry *>(tidrop.GetData());

	::SendMessage(m_hWndParentView, UWM_DRAGDROP, TREEVIEW_ID, reinterpret_cast<LPARAM>(&odi));
	this->SelectDropTarget(NULL);
	if ( FAILED(func_hr) )
	{
		*pdwEffect = DROPEFFECT_NONE;
	}
	else
	{
		*pdwEffect = (grfKeyState & MK_CONTROL) ? DROPEFFECT_COPY : DROPEFFECT_MOVE;
	}
	return func_hr;
}

///////////////////////////////////////////////////////////////////////////////

/**
	Returns whether a supported clipboard format is detected when
	dragging an object over the window
**/
bool  CMyTreeCtrl::IsAcceptedFormat_(IDataObject *pDataObject)
{
//const
static FORMATETC  fmt_1 = { UCF_GRYFF_ENTRIES_INTERNAL /* cfFormat */, NULL /* ptd */, DVASPECT_CONTENT /* dwAspect */, -1, TYMED_HGLOBAL /* tymed */};
static FORMATETC  fmt_2 = { UCF_GRYFF_ENTRIES /* cfFormat */, NULL /* ptd */, DVASPECT_CONTENT /* dwAspect */, -1, TYMED_HGLOBAL /* tymed */};
	return SUCCEEDED(pDataObject->QueryGetData(&fmt_1)) ||
		//SUCCEEDED(pDataObject->QueryGetData(&fmt_2));
		false;
}

///////////////////////////////////////////////////////////////////////////////
