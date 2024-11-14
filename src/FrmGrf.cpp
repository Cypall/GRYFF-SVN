// $Id: FrmGrf.cpp 12 2005-08-14 01:42:51Z Rasqual $

// *
// *   gryff, a GRF archive management utility, src/FrmGrf.cpp
// *   Copyright (C) 2003-2005 Rasqual Twilight
// *
// *   This program is distributed in the hope that it will be useful,
// *   but WITHOUT ANY WARRANTY; without even the implied warranty of
// *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// *

// * FrmGrf.cpp
// * MDI child Frame definition (implementation)
// *
// *@author $Author: Rasqual $
// *@version $Revision: 12 $
// *

#include  "stdwtl.h"
#include  <codeproject/filedialogfilter.h>
#include  <memory>
#include  <exception>
#include  <new>

#include  <openkore/libgrf>
#include  <rasqual/CWinApiFileIoManager.h>
#include  <rasqual/CGrfPacker.h>
#include  <rasqual/CGrfEntry.h>

#include  <system.h>

#pragma hdrstop
#include  "resource.h"
#include  "FrmGrf.h"

#include  "MyTreeCtrl.h"
#include  "MyListCtrl.h"

#include  "fnmanip.h"
#include  "commoncontrols.h"

#include  "DocGrf.h"
#include  "VwGrf.h"

// headers you won't see anywhere else in the project
#include  "ThreadIntercom.h"
#include  "DlgInvalidEntries.h"
#include  "DlgProgress.h"
#include  "DlgCheckBoxMultiFileDialog.h"

///////////////////////////////////////////////////////////////////////////////

using std::auto_ptr;
using std::pair;
using namespace win;

extern LPCTSTR szTempArchivePrefix;
extern UINT  UCF_GRYFF_ENTRIES;
extern UINT  UCF_GRYFF_ENTRIES_INTERNAL;

///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////

/**
	Performs initialization.
**/
CGrfFrame::CGrfFrame() : m_pErrorCode(0), m_pDoc(0), m_pView(0), m_Busy(false), m_pDropTargHlp(0), m_pDataObject(0), m_RefCount(0), m_AcceleratorsDisabled(false), m_FocusedChild(NULL), m_SaveParam(0)
{
}

///////////////////////////////////////////////////////////////////////////////

/**
	Performs cleanup.
**/
CGrfFrame::~CGrfFrame()
{
	if ( m_RefCount >= 1 )
	{
		this->Release();
	}
	ATLASSERT(m_RefCount == 0);
}

///////////////////////////////////////////////////////////////////////////////

/**
	Called when a new child frame is created.
	The MDICREATESTRUCT lpCreateParams has a lParam member
	which is a pointer of to a CCreateContext<CGrfDoc, CGrfView> structure.
	It contains information that is used to create the view subordinated to the frame.
	The view subscribe for notifications with the CGrfDoc AddView() method,
	and unsubscribes when the view is destroyed, using RemoveView()
**/
LRESULT CGrfFrame::OnCreate(LPCREATESTRUCT lpcs)
{
	ATLASSERT(lpcs->lpCreateParams);  // Must be passed a structure address by a MDI Parent
LPMDICREATESTRUCT lpMdiCreateStruct = reinterpret_cast<LPMDICREATESTRUCT>(lpcs->lpCreateParams);
	ATLASSERT(lpMdiCreateStruct->lParam);

CCreateContext<CGrfDoc, CGrfView> * pContext =
	 reinterpret_cast< CCreateContext<CGrfDoc, CGrfView> * >(lpMdiCreateStruct->lParam);
	ATLASSERT(pContext->m_pCurrentView == 0);

	  // Retrieve the CGrfDoc object address
	m_pDoc = pContext->m_pCurrentDoc;
	  // Create a view for the document in the frame
	m_pView = pContext->m_pCurrentView = new CGrfView;
RECT rc;
	this->GetClientRect(&rc);
	m_hWndClient = m_pView->Create(m_hWnd, rc, NULL, WS_CHILD | WS_VISIBLE | WS_MAXIMIZE, WS_EX_CLIENTEDGE, 0, reinterpret_cast<LPVOID>(pContext));
	if ( !m_hWndClient )
	{
		// If create failed, m_pView invoked OnfinalMessage, thus delete'd itself
		return -1;
	}
	m_pDoc->AddView(m_pView);

	m_LocalCmdBar.Create(m_hWnd, rcDefault, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE);
CMenuHandle  topMenu;
	topMenu.CreateMenu();
CMenuHandle  hMenuLvBg;
CMenuHandle  hMenuLvItem;
CMenuHandle  hMenuTvBg;
CMenuHandle  hMenuTvItem;
	  // FIXME: TODO: (1.8) If a language file is set, read menu strings from it (and check missing lines)
	if ( false == true )
	{
		hMenuLvBg.LoadMenu(IDR_LISTVIEW_CTXT);
		hMenuLvItem.LoadMenu(IDR_LISTVIEW_ITEM_CTXT);
		hMenuTvBg.LoadMenu(IDR_TREEVIEW_CTXT);
		hMenuTvItem.LoadMenu(IDR_TREEVIEW_ITEM_CTXT);
	}
	else
	{
		hMenuLvBg.CreatePopupMenu();
		// Consistent with format of menus in resource (popup)
	CMenuHandle  hMenuSub;
		hMenuSub.CreatePopupMenu();
		hMenuSub.AppendMenu(MF_STRING, IDM_EDIT_NEW_FOLDER, CString(MAKEINTRESOURCE(IDM_EDIT_NEW_FOLDER)));
		hMenuSub.AppendMenu(MF_SEPARATOR);
		hMenuSub.AppendMenu(MF_STRING, IDM_EDIT_ADD_FILES, CString(MAKEINTRESOURCE(IDM_EDIT_ADD_FILES)));
		hMenuSub.AppendMenu(MF_STRING, IDM_EDIT_ADD_DIRECTORY, CString(MAKEINTRESOURCE(IDM_EDIT_ADD_DIRECTORY)));
		//hMenuSub.AppendMenu(MF_STRING, IDM_EDIT_ADD_FROM_LIST, CString(MAKEINTRESOURCE(IDM_EDIT_ADD_FROM_LIST)));
		hMenuSub.AppendMenu(MF_SEPARATOR);
		hMenuSub.AppendMenu(MF_STRING, IDM_EDIT_MERGE_WITH_GRF, CString(MAKEINTRESOURCE(IDM_EDIT_MERGE_WITH_GRF)));
		hMenuSub.AppendMenu(MF_SEPARATOR);
		hMenuSub.AppendMenu(MF_STRING, IDM_VIEW_REFRESH, CString(MAKEINTRESOURCE(IDM_VIEW_REFRESH)));
		hMenuLvBg.AppendMenu(MF_POPUP, (HMENU)hMenuSub, _T(""));  // noname popup

		hMenuLvItem.CreatePopupMenu();
	CMenuHandle  hMenuSub2;
		hMenuSub2.CreatePopupMenu();
		hMenuSub2.AppendMenu(MF_STRING, IDM_EDIT_OPEN, CString(MAKEINTRESOURCE(IDM_EDIT_OPEN)));
		hMenuSub2.AppendMenu(MF_STRING, IDM_EDIT_EXTRACT, CString(MAKEINTRESOURCE(IDM_EDIT_EXTRACT)));
		hMenuSub2.AppendMenu(MF_SEPARATOR);
		hMenuSub2.AppendMenu(MF_STRING, IDM_EDIT_RENAME, CString(MAKEINTRESOURCE(IDM_EDIT_RENAME)));
		hMenuSub2.AppendMenu(MF_STRING, IDM_EDIT_DELETE, CString(MAKEINTRESOURCE(IDM_EDIT_DELETE)));
		//hMenuSub2.AppendMenu(MF_SEPARATOR);
		//Properties hMenuSub2.AppendMenu(MF_STRING, , CString(MAKEINTRESOURCE()));
		hMenuLvItem.AppendMenu(MF_POPUP, (HMENU)hMenuSub2, _T(""));  // noname popup

		hMenuTvBg.CreatePopupMenu();
	CMenuHandle  hMenuSub3;
		hMenuSub3.CreatePopupMenu();
		hMenuSub3.AppendMenu(MF_STRING, IDM_EDIT_OPEN, CString(MAKEINTRESOURCE(IDM_EDIT_OPEN)));
		hMenuSub3.AppendMenu(MF_STRING, IDM_EDIT_EXTRACT, CString(MAKEINTRESOURCE(IDM_EDIT_EXTRACT)));
		hMenuSub3.AppendMenu(MF_SEPARATOR);
		hMenuSub3.AppendMenu(MF_STRING, IDM_FILE_CLOSE, CString(MAKEINTRESOURCE(IDM_FILE_CLOSE)));
		hMenuSub3.AppendMenu(MF_STRING, IDM_FILE_CLOSE_ALL_BUT_CURRENT, CString(MAKEINTRESOURCE(IDM_FILE_CLOSE_ALL_BUT_CURRENT)));
		hMenuSub3.AppendMenu(MF_SEPARATOR);
		hMenuSub3.AppendMenu(MF_STRING, IDM_FILE_SAVE, CString(MAKEINTRESOURCE(IDM_FILE_SAVE)));
		hMenuSub3.AppendMenu(MF_STRING, IDM_FILE_SAVE_AS, CString(MAKEINTRESOURCE(IDM_FILE_SAVE_AS)));
		hMenuSub3.AppendMenu(MF_SEPARATOR);
		hMenuSub3.AppendMenu(MF_STRING, IDM_VIEW_REFRESH, CString(MAKEINTRESOURCE(IDM_VIEW_REFRESH)));
		hMenuTvBg.AppendMenu(MF_POPUP, (HMENU)hMenuSub3, _T(""));  // noname popup

		hMenuTvItem.CreatePopupMenu();
	CMenuHandle  hMenuSub4;
		hMenuSub4.CreatePopupMenu();
		hMenuSub4.AppendMenu(MF_STRING, IDM_EDIT_OPEN, CString(MAKEINTRESOURCE(IDM_EDIT_OPEN)));
		hMenuSub4.AppendMenu(MF_STRING, IDM_EDIT_EXTRACT, CString(MAKEINTRESOURCE(IDM_EDIT_EXTRACT)));
		hMenuSub4.AppendMenu(MF_SEPARATOR);
		//Paste hMenuSub4.AppendMenu(MF_STRING, IDM_EDIT_PASTE, CString(MAKEINTRESOURCE(IDM_EDIT_PASTE)));
		//hMenuSub4.AppendMenu(MF_SEPARATOR);
		hMenuSub4.AppendMenu(MF_STRING, IDM_EDIT_RENAME, CString(MAKEINTRESOURCE(IDM_EDIT_RENAME)));
		hMenuSub4.AppendMenu(MF_STRING, IDM_EDIT_DELETE, CString(MAKEINTRESOURCE(IDM_EDIT_DELETE)));
		//hMenuSub4.AppendMenu(MF_SEPARATOR);
		//Properties hMenuSub4.AppendMenu(MF_STRING, , CString(MAKEINTRESOURCE()));
		hMenuTvItem.AppendMenu(MF_POPUP, (HMENU)hMenuSub4, _T(""));  // noname popup

	}
	///// MENU_CONTEXT__LV_BG
	topMenu.AppendMenu(MF_POPUP, (HMENU)hMenuLvBg, _T(""));
	///// MENU_CONTEXT__LV_ITEM
	topMenu.AppendMenu(MF_POPUP, (HMENU)hMenuLvItem, _T(""));
	///// MENU_CONTEXT__TV_BG
	topMenu.AppendMenu(MF_POPUP, (HMENU)hMenuTvBg, _T(""));
	///// MENU_CONTEXT__TV_ITEM
	topMenu.AppendMenu(MF_POPUP, (HMENU)hMenuTvItem, _T(""));

	m_LocalCmdBar.AttachMenu(topMenu);
CBitmap bmp;
	bmp.LoadBitmap(MAKEINTRESOURCE(IDM_EDIT_ADD_FILES));
SIZE size = { 0, 0 };
	bmp.GetSize(size);
	m_LocalCmdBar.SetImageSize(size.cx, size.cy);
DWORD  ControlsVersion = commoncontrols::GetComCtlVersion();
	if ( HIWORD(ControlsVersion) >= 6 )  // support for alpha blending of bitmaps w/o complicated things
	{
		m_LocalCmdBar.SetAlphaImages(true);
		m_LocalCmdBar.AddBitmap(MAKEINTRESOURCE(IDM_EDIT_ADD_FILES), IDM_EDIT_ADD_FILES);
		m_LocalCmdBar.AddBitmap(MAKEINTRESOURCE(IDM_EDIT_ADD_DIRECTORY), IDM_EDIT_ADD_DIRECTORY);

		m_LocalCmdBar.AddBitmap(MAKEINTRESOURCE(IDM_FILE_SAVE), IDM_FILE_SAVE);
	}
	else
	{
		m_LocalCmdBar.AddBitmap(MAKEINTRESOURCE(IDM_EDIT_ADD_FILES__24), IDM_EDIT_ADD_FILES);
		m_LocalCmdBar.AddBitmap(MAKEINTRESOURCE(IDM_EDIT_ADD_DIRECTORY__24), IDM_EDIT_ADD_DIRECTORY);

		m_LocalCmdBar.AddBitmap(MAKEINTRESOURCE(IDM_FILE_SAVE__24), IDM_FILE_SAVE);
	}

	// FIXME: TODO: (x.x) Add other menu item images here

	m_pView->SetCommandBar(&m_LocalCmdBar);

	::CoCreateInstance(CLSID_DragDropHelper, NULL, CLSCTX_INPROC_SERVER,
		IID_IDropTargetHelper, reinterpret_cast<LPVOID *>(&m_pDropTargHlp));

	if ( m_pDropTargHlp == 0 )
	{
		MessageBox(_T("Error when creating the IDropTargetHelper instance"), CString(MAKEINTRESOURCE(IDS_INIT_ERROR_MSGBOX_TITLE)), MB_ICONERROR | MB_OK);
		return -1;
	}
	m_RefCount = 1;

	::RegisterDragDrop(m_hWnd, this);

	this->SetMsgHandled(FALSE);
	return 0;
} // CGrfFrame::OnCreate

///////////////////////////////////////////////////////////////////////////////

/**
	Called by the doc manager upon frame initialization
**/
void  CGrfFrame::OnInitialUpdate()
{
	this->UpdateTitle();
	m_pView->OnInitialUpdate();
}

///////////////////////////////////////////////////////////////////////////////

/**
	"Note: You must allocate the object on the heap.
	If you do not, you must override OnFinalMessage()"
	This method deletes the current object, causing the view to be destroyed,
	and unsubscribing the view.
**/
void  CGrfFrame::OnFinalMessage(HWND hWnd)
{
	// ATL "self-delete bug" as reported in:
	//http://groups.yahoo.com/group/wtl/message/10095
	//http://sourceforge.net/tracker/index.php?func=detail&aid=1045037&group_id=109071&atid=652372
	//http://www.mvps.org/vcfaq/com/10.htm

	// FIX for crash when closing a background child window:
	//http://groups.yahoo.com/group/wtl/message/10639

	  // Note: view is remove()d by CGrfView::OnFinalMessage()
	if ( m_pDoc->GetNumViews() == 0 )
	{
		::SendMessage(this->GetMDIFrame(), UWM_MDIDESTROY, 0, reinterpret_cast<LPARAM>(m_pDoc));
	}

	delete this;
}

///////////////////////////////////////////////////////////////////////////////

/**
	Returns whether document has been modified.
**/
bool  CGrfFrame::IsModified() const
{
	return  this->GetDocPtr()->IsModified() != FALSE;
}

///////////////////////////////////////////////////////////////////////////////

/**
	Retrieves a constant pointer to the document name.
**/
const TCHAR * const  CGrfFrame::GetName() const
{
	return this->GetDocPtr()->GetName();
}

///////////////////////////////////////////////////////////////////////////////

/**
	Returns whether there is a selection.
**/
bool  CGrfFrame::CanPerformOpen() const
{
	return this->GetViewPtr()->CanPerformOpen();
}

///////////////////////////////////////////////////////////////////////////////

/**
	Returns whether selection may be renamed.
**/
bool  CGrfFrame::CanPerformRename() const
{
	return this->GetViewPtr()->CanPerformRename();
}

///////////////////////////////////////////////////////////////////////////////

/**
	Returns whether selection may be deleted.
**/
bool  CGrfFrame::CanPerformDelete() const
{
	return this->GetViewPtr()->CanPerformDelete();
}


///////////////////////////////////////////////////////////////////////////////

/**
	Called upon reception of a MDI child frame close message
	If there is only one view associated to the document, and the doc has been modified,
	prompt for file save. User may then choose to cancel frame closing by clicking cancel,
	either on the MessageBox, or on the OpenFileSave dialogs.
	In all other cases, destroy the frame.
**/
void  CGrfFrame::OnClose()
{
	const _ATL_MSG* pMsg(GetCurrentMessage());
	// Hack WM_CLOSE cookie. >> don't prompt for save
	if ( !(pMsg->wParam == 1 && pMsg->lParam == 0x1313) )
	{
		if ( m_pDoc->GetNumViews() == 1 && m_pDoc->IsModified() != FALSE )
		{
			CString strPrompt;
			BOOL dummy;
			INT_PTR nResponse;
			strPrompt.Format(CString(MAKEINTRESOURCE(IDS_MODIFIED_SAVE_PROMPT)), m_pDoc->GetName());
			if ( IDCANCEL == (nResponse=::MessageBoxEx(this->m_hWnd, strPrompt, CString(MAKEINTRESOURCE(IDS_APP_TITLE)), MB_ICONEXCLAMATION | MB_YESNOCANCEL, LANGIDFROMLCID(::GetThreadLocale())))
				|| (nResponse==IDYES && this->OnFileSave(0,0,0,dummy) == IDCANCEL) ) // User may cancel file save at MB and at OFN
			{
					// Return without destroying (Abort close)
				return;
			}
		}
	}
	SetMsgHandled(FALSE);
}

///////////////////////////////////////////////////////////////////////////////

/**
	Unregister the MDI child frame and destroys the view and frame
**/
void  CGrfFrame::OnDestroy()
{
	if ( m_RefCount >= 1 )
	{
		::RevokeDragDrop(m_hWnd);
	}

	if ( m_LocalCmdBar.m_hWnd )
	{
		m_LocalCmdBar.DestroyWindow();
		if ( m_LocalCmdBar.m_wndParent.IsWindow() )
		{
			m_LocalCmdBar.m_wndParent.UnsubclassWindow();
		}
	}

	  // If the specified window is a parent or owner window, DestroyWindow automatically destroys the associated child
	  // or owned windows when it destroys the parent or owner window. The function first destroys child or owned windows,
	  // and then it destroys the parent or owner window.
	SetMsgHandled(FALSE);
}

///////////////////////////////////////////////////////////////////////////////

LRESULT  CGrfFrame::OnMDIActivate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
{
	::SendMessage(this->GetMDIFrame(), WM_MDISETMENU, 0, 0);

	bHandled = FALSE;
	return 1;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT CGrfFrame::OnSetFocus(HWND)
{
	m_pView->SetFocus();
	::SendMessage(this->GetMDIFrame(), WM_MDISETMENU, 0, 0);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

  // UWM_UPDATE sent by contained view
LRESULT  CGrfFrame::OnUpdate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	switch ( wParam )  // Hint
	{
	case HINT_UPDATE_DOCUMENT_PATH :
	case HINT_UPDATE_DOCUMENT_REFRESH:
		{
			this->UpdateTitle();
		}
		break;
	}
	::SendMessage(this->GetMDIFrame(), WM_MDISETMENU, 0, 0);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

  // feedback for long operations
LRESULT  CGrfFrame::OnProgress(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	// FIXME: TODO: (x.x) Progress bar in status is not functional
	CString trace;
	trace.Format(_T("CGrfFrame::OnProgress %08x %08x (=%08x?)"), wParam, lParam, HINT_DOCUMENT_SERIALIZE_STATE);
	if ( ::GetAsyncKeyState(VK_CONTROL) == 0 )
	{
		::MessageBox(HWND_DESKTOP, trace, _T("OnUpdate"), MB_OK);
	}
	switch ( lParam )
	{
	case HINT_DOCUMENT_SERIALIZE_STATE :
		{
			::SendMessage(this->GetMDIFrame(), UWM_PROGRESS, wParam, lParam);
		}
		break;
	}
	return 0;
}


///////////////////////////////////////////////////////////////////////////////

LRESULT  CGrfFrame::OnGetMDIFrame(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	return  reinterpret_cast<LRESULT>(this->GetMDIFrame());
}

///////////////////////////////////////////////////////////////////////////////

LRESULT  CGrfFrame::OnDragDrop(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	OleDropInfo *pDropInfo = reinterpret_cast<OleDropInfo *>(lParam);
	if ( wParam == TREEVIEW_ID )
	{
		return OnTVDrop(*pDropInfo);
	}
	return 0;

} // OnDragDrop

///////////////////////////////////////////////////////////////////////////////

LRESULT  CGrfFrame::OnEnterChildFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	m_AcceleratorsDisabled = true;
	m_FocusedChild = reinterpret_cast<HWND>(lParam);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

  // Pre-translate for view
LRESULT  CGrfFrame::OnForwardMsg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	LPMSG pMsg = reinterpret_cast<LPMSG>(lParam);
	if ( pMsg )
	{
		if ( pMsg->hwnd == m_pView->m_hWnd )
		{
			if ( pMsg->message == WM_NOTIFY && ((LPNMHDR)(pMsg->lParam))->code == LVN_ENDLABELEDIT )
			{
				return this->OnLVEndLabelEdit((LPNMHDR)(pMsg->lParam));
			}
		}
	}
	return FALSE;  // discard
}

///////////////////////////////////////////////////////////////////////////////

  // Menu File->Close (IDM_FILE_CLOSE) delegate OnClose
LRESULT  CGrfFrame::OnFileClose(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	SendMessage(WM_CLOSE);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT  CGrfFrame::OnFileSave(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
LPCTSTR szFilePath = m_pDoc->GetPathName();
	  // Does document already exist?
	if ( szFilePath[0] != _T('\0') && m_pDoc->IsModified() != FALSE )
	{
		return  this->OnSave(true);
	}
	return this->RequestDocumentPath_(0, true);
}

///////////////////////////////////////////////////////////////////////////////

LRESULT  CGrfFrame::OnFileSaveAs(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
LPCTSTR szFilePath = m_pDoc->GetPathName();
	return this->RequestDocumentPath_(szFilePath, true);
}

///////////////////////////////////////////////////////////////////////////////

/**
	Request to save the document.
	If no associated file exists, user is prompted for a destination with RequestDocumentPath_()
	If the file is correctly saved, the path is added to the MRU documents.
**/
LRESULT  CGrfFrame::OnSave(bool Asynchronously)
{
LPCTSTR szFilePath = m_pDoc->GetPathName();
	if ( !this->SaveDocument_(szFilePath, Asynchronously) )
	{
		return IDCANCEL;
	}
	this->RegisterMRU_(szFilePath);
	return IDOK;
}

///////////////////////////////////////////////////////////////////////////////

/**
	Shows the common OpenFileName dialog and attempts to save the document if
	a path is provided. Otherwise, returns IDCANCEL.
	pDefaultPath allows to specify what the dialog's editbox is initialized with.
	It also sets the starting directory.
**/
LRESULT  CGrfFrame::RequestDocumentPath_(LPCTSTR pDefaultPath, bool Asynchronously)
{
	CFileDialogFilter cf_dlgfilter(CGrfDoc::strFilter);
	CFileDialog cf_dlg(FALSE,  // FileSaveAs
		_T("grf"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, cf_dlgfilter);
	if ( pDefaultPath != 0 )
	{
		ATLASSERT(!::IsBadStringPtr(pDefaultPath, MAX_PATH));
		lstrcpyn(cf_dlg.m_ofn.lpstrFile, pDefaultPath, MAX_PATH);
	}
	INT_PTR nResponse;
	if ( IDOK == (nResponse = cf_dlg.DoModal()) )
	{
		if ( !this->SaveDocument_(cf_dlg.m_szFileName, Asynchronously) )
		{
			return IDCANCEL;
		}
		this->RegisterMRU_(cf_dlg.m_szFileName);
	}
	return nResponse;
}

///////////////////////////////////////////////////////////////////////////////

/**
	Saves the document to the specified pFilePath.
	Returns true on success
**/
bool  CGrfFrame::SaveDocument_(LPCTSTR pFilePath, bool Asynchronously)
{
	ATLASSERT ( pFilePath != 0 );
const CGrfDoc          *pDoc( this->GetDocPtr() );
CAtlList< pair<uint8_t, CString> > InvalidEntries;

	// Find invalid (non CP949 charset) entries and prompt for continuing
	if ( pDoc->HasInvalidEntries(&InvalidEntries) )
	{
	CInvalidEntriesDialog dlg;
		if ( IDOK != dlg.DoModal(m_hWnd, reinterpret_cast<LPARAM>(&InvalidEntries)) )
		{
			return false;
		}
	}
	//'------
	// Asynchronously: The worker thread is spawn, the progress dialog is shown when a fixed timeout expires
	//                 worker thread prepares the document and reports back any error to the UI
	//                 through the CDocumentSaverData (messaging)
	// Synchronously: Document is prepared, maybe blocking UI update, and any error is immediately reported
	//                Then the progress dialog is shown and worker thread spawn.
if (false)//	if ( Asynchronously )
	{
		// Tell the GUI to refuse any operation while the current one is pending
		m_Busy = true;
		this->BeginSaveDocumentThread_(pFilePath, InvalidEntries);
	}
	else
	{
		if ( 0 != this->SaveDocumentSynchronous_(pFilePath, InvalidEntries) )
		{
			return false;
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT  CGrfFrame::PrepareSaveDocument(LPCTSTR pFilePath, CDocumentSaverData &saveData)
{
CString                dn, fn(pFilePath);
const CGrfDoc          *pDoc( this->GetDocPtr() );
	saveData.pThis = this;
auto_ptr<CGrfCache> pCache( new CGrfCache() );
auto_ptr<CWinApiFileIoManager> iomgr( new CWinApiFileIoManager() );
CGrfDirectoryEntry *pdirent;
CGrfFileEntry *precord;
CString strDirectory, strName, strCompleteName;
const SortedGrfDirMap * const pDirMap(pDoc->GetDirMapPtr());

	/**
		Preparation steps
		0. Assume all "invalid entries"  have been specified
		1. Cache grf handles for further use
		2. Obtain temporary output file name
	**/


	// #0000010: Cache is destroyed after repacking, allowing MoveFile
	// Put dependencies into cache, or fail
	if ( !pDoc->CacheHandles(*pCache, saveData.pErroneousEntries) )
	{
		return 1;
	}

	  // Retrieve destination filepath components
	::BreakPath(fn, dn, fn);

	  // Create a temporary output file
	if ( 0 == ::GetTempFileName(dn, ::szTempArchivePrefix, 0, saveData.tmppath) )
	{
		return 2;
	}
	  // Make way for the real output. In a security context, this may be
	::DeleteFile(saveData.tmppath);

	lstrcpy(saveData.path, pFilePath);

CAutoPtrArray<IGrfEntry> pEntries;
char  szEntryNameA[CGrfFileEntry::GRF_MAX_PATH];

	  // Browse the directories
	for ( POSITION pos = pDirMap->GetHeadPosition(); pos; pDirMap->GetNext(pos) )
	{
		pDirMap->GetAt(pos, strDirectory, pdirent);
		  // Try to find the directory in the list of invalid entries
		if ( !saveData.pInvalidEntries->IsEmpty() &&
			saveData.pInvalidEntries->Find(pair<uint8_t, CString>(2, strDirectory)) != 0 )
		{
			continue;
		}
		  // create entry for directory
		if ( !strDirectory.IsEmpty() )  // root is implicit
		{
			::WideCharToMultiByte(0x3B5, 0, strDirectory, -1, szEntryNameA, CGrfFileEntry::GRF_MAX_PATH, NULL, NULL);
			  // Add directory entry
			pEntries.Add(CAutoPtr<IGrfEntry>(new CGrfDirEntry(szEntryNameA)));
		}
		POSITION pos2 = pdirent->files.GetHeadPosition();
		while ( pos2 )
		{
			  // Sets strName
			pdirent->files.GetAt(pos2, strName, precord);
			if ( strDirectory.IsEmpty() )
			{
				strCompleteName = strName;
			}
			else
			{
				strCompleteName.Format(_T("%s\\%s"), strDirectory, strName);
			}
				// Try to find the file in the list of invalid entries
			if ( !saveData.pInvalidEntries->IsEmpty() &&
				saveData.pInvalidEntries->Find(pair<uint8_t, CString>(1, strCompleteName)) != 0 )
			{
				pdirent->files.GetNext(pos2);
				continue;
			}
			::WideCharToMultiByte(0x3B5, 0, strCompleteName, -1, szEntryNameA, CGrfFileEntry::GRF_MAX_PATH, NULL, NULL);
			switch ( precord->origin )
			{
			case CGrfFileEntry::FROMGRF:
				{
				uint32_t  idx;
				openkore::Grf * pGrf = pCache->get(*(precord->grf_src));
					if ( !openkore::grf_find(pGrf, precord->in_grf_path, &idx) )
					{
						// Show info both unencoded and encoded
						WCHAR  resName[CGrfFileEntry::GRF_MAX_PATH];
						WCHAR  tr_resName[CGrfFileEntry::GRF_MAX_PATH];
						char  *src  = precord->in_grf_path;
						TCHAR *dest = resName;
						while ( *dest++ = (TCHAR) *src++ )
						{
							;
						}
						*dest = _T('\0');
						::MultiByteToWideChar(0x3B5, 0, precord->in_grf_path, -1, tr_resName, CGrfFileEntry::GRF_MAX_PATH);
						CString  strMissing;
						strMissing.Format(_T("%s -> [%s] (%s) in [%s]."),
							strCompleteName, tr_resName, resName, *(precord->grf_src));
						saveData.pErroneousEntries->AddTail(pair<uint8_t, CString>(1, strMissing));
						break;
					}
				HANDLE hMapping = pCache->getFileMapping(*(precord->grf_src));
					if ( !hMapping )
					{
					LPVOID lpMsgBuf;
						::FormatMessage(
							FORMAT_MESSAGE_ALLOCATE_BUFFER |
							FORMAT_MESSAGE_FROM_SYSTEM |
							FORMAT_MESSAGE_IGNORE_INSERTS,
							NULL,
							::GetLastError(),
							MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
							(LPTSTR) &lpMsgBuf,
							0,
							NULL);
					CString  strMissing;
						strMissing.Format(_T("%s :: Unable to map [%s] into memory. "),
							strCompleteName, *(precord->grf_src));
						strMissing += (LPCTSTR)lpMsgBuf;
						::LocalFree(lpMsgBuf);
						saveData.pErroneousEntries->AddTail(pair<uint8_t, CString>(1, strMissing));
						break;
					}
					pEntries.Add(CAutoPtr<IGrfEntry>(new CGrfCompressedEntry(szEntryNameA, pGrf, idx, hMapping)));
				}
				break;
			case CGrfFileEntry::FROMFS:
				{
					pEntries.Add(CAutoPtr<IGrfEntry>(new CGrfEntry<CWinApiFileIoManager>(szEntryNameA, precord->realpath, iomgr.get())));
				}
				break;
			}
			pdirent->files.GetNext(pos2);
		}
	}
	if ( !saveData.pErroneousEntries->IsEmpty() )
	{
		return 3;
	}
	const unsigned int maxEntries = static_cast<unsigned int>(pEntries.GetCount());
	saveData.pEntries.SetCount(maxEntries);
	for ( unsigned int iE = 0; iE < maxEntries; ++iE )
	{
		saveData.pEntries.SetAt(iE, pEntries.GetAt(iE));
	}
	saveData.pCache = pCache.release();
	saveData.pIoManager = iomgr.release();

	return 0;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT  CGrfFrame::BeginSaveDocumentThread_(LPCTSTR pFilePath, const CAtlList< pair<uint8_t, CString> > &EntriesToIgnore)
{
//CAtlList< pair<uint8_t, CString> > UnresolvedEntries;
//POSITION pos, pos2;
//openkore::Grf           *pGrf = 0;
auto_ptr<CDocumentSaverData> pSaveData(new CDocumentSaverData());
JOBDATA jobData = { *this, JOBID_SAVEDOC, reinterpret_cast<LPARAM>(pSaveData.get()) };
LRESULT lResult = ClassifiedAds::GetJobDone(jobData);
	if ( IDOK == lResult )
	{
		  // ATTN: delete shall now be done at the end of thread processing, otherwise leaks
		pSaveData.release();
	}
	else
	{
	CString  strError;
		if ( jobData.pException )
		{
			strError = jobData.pException->ToString();
		}
		else
		{
			strError = _T("Unknown error while saving the document.");
		}
		this->MessageBox(strError, CString(MAKEINTRESOURCE(IDS_APP_TITLE)), MB_ICONERROR | MB_OK);
	}
	return lResult;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT  CGrfFrame::SaveDocumentSynchronous_(LPCTSTR pFilePath, const CAtlList< pair<uint8_t, CString> > &EntriesToIgnore)
{
			#if defined(_DEBUG)
			CString strOpInfo;
			FILETIME ft_b4, ft_af;
			// Begin timing
			::GetSystemTimeAsFileTime(&ft_b4);
			#endif
CDocumentSaverData  saveData;
	saveData.pInvalidEntries = &EntriesToIgnore;
CAtlList< pair<uint8_t, CString> > ErroneousEntries;
	saveData.pErroneousEntries = &ErroneousEntries;
LRESULT  prepareResult = this->PrepareSaveDocument(pFilePath, saveData);
	if ( prepareResult != 0 )
	{
		switch ( prepareResult )
		{
		case 1:
			{
			CInvalidEntriesDialog dlg(_T("Unresolved entries"));
				dlg.DoModal(m_hWnd, reinterpret_cast<LPARAM>(&ErroneousEntries));
			}
			break;
		case 2:
			this->MessageBox(CString(MAKEINTRESOURCE(IDS_CANNOT_CREATE_TEMPFILE)), CString(MAKEINTRESOURCE(IDS_APP_TITLE)), MB_ICONERROR | MB_OK);
			break;
		case 3:
			{
			CInvalidEntriesDialog dlg(_T("Missing entries"));
				dlg.DoModal(m_hWnd, reinterpret_cast<LPARAM>(&ErroneousEntries));
			}
			break;
		}
		return prepareResult;
	}
			#if defined(_DEBUG)
			// End timing
			::GetSystemTimeAsFileTime(&ft_af);
			strOpInfo.Format(_T("Save preparation completed in %d ms"), (ft_af.dwLowDateTime - ft_b4.dwLowDateTime) / 10000);
			::SendMessage(this->GetMDIFrame(), UWM_STATUS_TEXT, 0, reinterpret_cast<LPARAM>(strOpInfo.operator LPCTSTR()));
			#endif

auto_ptr<IGrfPacker> ppak(new CGrfPacker<CWinApiFileIoManager>(0, 0, reinterpret_cast<LPARAM>(&saveData), reinterpret_cast<const UTF16_t*>(saveData.tmppath)));
	saveData.pPak = ppak.get();

	// Create a modal Progress dialog
CProgressDialog  progressDialog(&CGrfFrame::Save_DlgInit, reinterpret_cast<LPARAM>(&saveData) );
	saveData.pDlg = &progressDialog;

INT_PTR nResult = progressDialog.DoModal(m_hWnd, reinterpret_cast<LPARAM>(&saveData));
	if ( nResult == IDOK )
	{
		ppak.release();
	}
	else
	{
		if ( nResult != IDCANCEL )
		{
			this->MessageBox(_T("Failed to start packing operation. Please free system resources and retry later."));
			return 4;
		}
		::MessageBeep(MB_OK);
		return 5;
	}

			#if defined(_DEBUG)
			// End timing
			::GetSystemTimeAsFileTime(&ft_af);
			strOpInfo.Format(_T("Save completed in %d ms"), (ft_af.dwLowDateTime - ft_b4.dwLowDateTime) / 10000);
			::SendMessage(this->GetMDIFrame(), UWM_STATUS_TEXT, 0, reinterpret_cast<LPARAM>(strOpInfo.operator LPCTSTR()));
			#endif

	::MessageBeep((UINT)-1);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT  CGrfFrame::OnEditCreateDir(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled)
{
CString strCwd(m_pView->GetCwd());
CString strBaseDirName(MAKEINTRESOURCE(IDS_NEW_FOLDER_NAME));  // Default localized name
CString strTargetDirName, strShortTargetDirName;
CMyListCtrl *pListCtrl = m_pView->GetListViewPtr();
CMyTreeCtrl *pTreeCtrl = m_pView->GetTreeViewPtr();
CGrfDirectoryEntry *pdirent;
CAtlList< pair<uint8_t, CString> > AddedEntries;

	  // Attempt to name a new directory
	if ( !strCwd.IsEmpty() )
	{
		strCwd += _T("\\");
	}
	strTargetDirName = strCwd + strBaseDirName;
	strShortTargetDirName = strBaseDirName;
	for ( int iTry = 1; m_pDoc->DirExists(strTargetDirName); ++iTry )
	{
		strShortTargetDirName.Format(_T("%s (%d)"), strBaseDirName, iTry);
		strTargetDirName.Format(_T("%s%s"), strCwd, strShortTargetDirName);
	}

	pdirent = m_pDoc->MkdirRecursive(strTargetDirName, &AddedEntries);
	if ( pdirent == 0 )
	{
		MessageBox(CString(MAKEINTRESOURCE(IDS_CANNOT_CREATE_DIRECTORY)), CString(MAKEINTRESOURCE(IDS_APP_TITLE)), MB_ICONERROR | MB_OK);
		return 0;
	}
	m_pDoc->SetModifiedFlag();

	{
		  // Update the current view
		  // ... in the tree view
		CTreeItem ti = pTreeCtrl->InsertFolder(strShortTargetDirName, reinterpret_cast<LPARAM>(pdirent),
											   pTreeCtrl->GetSelectedItem(), TVI_SORT);
		ti.EnsureVisible();
		//pTreeCtrl->GetSelectedItem().SortChildren();
		  // ... in the list view
		int iInsertedItem = pListCtrl->AppendItem(strShortTargetDirName, &(ti.m_hTreeItem));
		  // Unselect previously selected items
		if ( pListCtrl->GetFirstSelectedIndex() == -1 )
		{
			for ( int iItem = pListCtrl->GetFirstSelectedIndex(); iItem != -1; iItem = pListCtrl->GetNextItem(iItem, LVNI_ALL | LVNI_SELECTED))
			{
				pListCtrl->SetItemState(iItem, 0, LVIS_SELECTED);
			}
		}
		  // Set selection & focus
		pListCtrl->SetItemState(iInsertedItem, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
	}
	  // Notify other views via the document
	  // HINT_DOCUMENT_DIRS_ADDED
	m_pDoc->OnEntriesAdded(1, &AddedEntries, m_pView);
	this->UpdateTitle();
	::SendMessage(this->GetMDIFrame(), WM_MDISETMENU, 0, 0);
	  // Request new name
	this->OnEditRename(0,0,pListCtrl->m_hWnd, bHandled);

	return 0;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT  CGrfFrame::OnEditAddFiles(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
CAtlList< pair<uint8_t, CString> > ImportFiles;

	  // Instantiate a multi-file selector -- CMultiFileDialog
	CFileDialogFilter cf_dlgfilter(CGrfDoc::strTypeFilter);
	CCheckBoxMultiFileDialog<IDD_OFN_ENCODING, IDC_OFN_ENABLE_CONV> cf_dlg(
	    TRUE,  // FileOpen
	    NULL,
	    _T(""),
	    OFN_ALLOWMULTISELECT | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_ENABLETEMPLATE,
	    cf_dlgfilter,
	    NULL
	);
	// FIXME: TODO: (1.2) option to enable conversion by default
	//cf_dlg.Check(true);

	if ( IDOK == cf_dlg.DoModal() )
	{
		POSITION filelist_pos = cf_dlg.GetStartPosition();
		CString strFilePath;

		while ( filelist_pos )
		{
			strFilePath = cf_dlg.GetNextPathName(filelist_pos);

			ImportFiles.AddTail( pair<uint8_t, CString>(1 | (cf_dlg.IsChecked() ? 0x00000040 : 0), strFilePath) );
			  // Leaving loop cond: filelist_pos == NULL
		}
		ATLASSERT(!ImportFiles.IsEmpty());
		this->OnImportFiles_(&ImportFiles);
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

/**
	Attempts to import files into the current working directory.
	If some entries are invalid (length check), a dialog appears and prompts the user whether
	the import should continue without these entries.
	Returns ID_CANCEL if the user cancelled the import
**/
LRESULT  CGrfFrame::OnImportFiles_(const CAtlList< pair<uint8_t, CString> > *pPaths, bool fClearSelection)
{
CAtlList< pair<uint8_t, CString> > ValidEntries, InvalidEntries;
CString strCwd(m_pView->GetCwd());
CString dn, fn;
int nMbCount;

	POSITION pos = pPaths->GetHeadPosition();
	while ( pos )
	{
		pair<uint8_t, CString> entry(pPaths->GetNext(pos));
		bool bBad(false);
		CString strPath;
		if ( entry.first & 0x00000040 )
		{
			  // Test for invalid characters
			  // http://unicode.org/Public/MAPPINGS/VENDORS/MICSFT/WINDOWS/CP949.TXT
			  // Only 8-bit characters are welcome!
		unsigned int i, len = entry.second.GetLength();
			CAutoVectorPtr<char> data(new char[len+1]), szEntryNameA(new char[len+1]);
			  // Turn 16-bit characters to 8-bit
			for ( i = 0U; i < len; ++i )
			{
				TCHAR ch = entry.second.GetAt(i);
				if ( ch == 0x80 || ch >= 0xFF )
				{
					InvalidEntries.AddTail( entry );
					bBad = true;
					break;
				}
				data[i] = static_cast<char>(ch & 0xFF);
			}
            if ( !bBad )
			{
				data[i] = 0;
				  // all characters are in range. Convert to Unicode
				::MultiByteToWideChar(0x3B5, 0, data, -1, strPath.GetBufferSetLength(len + 1), len + 1);
				strPath.ReleaseBuffer();
				  // Back to 8-bit
				::WideCharToMultiByte(0x3B5, 0, strPath, -1, szEntryNameA, len + 1, NULL, NULL);
				if ( strcmp(data, szEntryNameA) )
				{
					InvalidEntries.AddTail( entry );
					bBad = true;
				}
				else
				{
					::BreakPath(strPath, dn, fn);
					// basic length check
					nMbCount = ::WideCharToMultiByte(0x3B5, 0, fn, -1, NULL, 0, NULL, NULL);
					if ( nMbCount > CGrfFileEntry::GRF_MAX_PATH )
					{
						InvalidEntries.AddTail( entry );
						bBad = true;
					}
				}
			}
            if ( !bBad )
			{
				ValidEntries.AddTail( entry );
			}
		}
		else
		{
			strPath = entry.second;
			::BreakPath(strPath, dn, fn);
			  // basic length check
			nMbCount = ::WideCharToMultiByte(0x3B5, 0, fn, -1, NULL, 0, NULL, NULL);
			if ( nMbCount > CGrfFileEntry::GRF_MAX_PATH )
			{
				InvalidEntries.AddTail( entry );
			}
			else
			{
				ValidEntries.AddTail( entry );
			}
		}
	}

	if ( !InvalidEntries.IsEmpty() )
	{
		CInvalidEntriesDialog dlg;
		if ( IDOK != dlg.DoModal(::GetActiveWindow(), reinterpret_cast<LPARAM>(&InvalidEntries)) )
		{
			  // Abort import
			return IDCANCEL;
		}
	}

	CAtlList< pair<uint8_t, CString> > AddedEntries;
	m_pDoc->ImportFiles(&ValidEntries, strCwd, &AddedEntries);
	if ( !AddedEntries.IsEmpty() )
	{
		this->AddToView_(&AddedEntries, fClearSelection, true, true);
		m_pDoc->SetModifiedFlag();
		  // HINT_DOCUMENT_FILES_ADDED
		m_pDoc->OnEntriesAdded(0, &AddedEntries, m_pView);
		this->UpdateTitle();
		::SendMessage(this->GetMDIFrame(), WM_MDISETMENU, 0, 0);
		  // set selection if none is set
	}
	return IDOK;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT CGrfFrame::OnEditAddDirectory(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
CString dn, fn;
CString strCwd(m_pView->GetCwd());
CMyListCtrl *pListCtrl = m_pView->GetListViewPtr();
CMyTreeCtrl *pTreeCtrl = m_pView->GetTreeViewPtr();
CString strInfo;
	strInfo.Format(CString(MAKEINTRESOURCE(IDS_MOUNT_LOCATION)), strCwd);
CFolderDialog bff_dlg(m_hWnd, strInfo, BIF_USENEWUI | BIF_RETURNONLYFSDIRS | BIF_SHAREABLE);
	//bff_dlg.m_bi.pidlRoot = CSIDL_DRIVES;
	//SetSelection()

	if ( IDOK == bff_dlg.DoModal() )
	{
		CAtlList< pair<uint8_t, CString> > AddedEntries;

		bool  UseConversion = (IDYES==this->MessageBox(CString(MAKEINTRESOURCE(IDS_USE_CONV_PROMPT)), CString(MAKEINTRESOURCE(IDS_APP_TITLE)), MB_ICONQUESTION | MB_YESNO));

		m_pDoc->ImportDirectoryTree(bff_dlg.GetFolderPath(), strCwd, UseConversion, &AddedEntries);
		if ( !AddedEntries.IsEmpty() )
		{
			this->AddToView_(&AddedEntries, true, true);
			m_pDoc->SetModifiedFlag();
			  // HINT_DOCUMENT_FILES_AND_DIRS_ADDED
			m_pDoc->OnEntriesAdded(2, &AddedEntries, m_pView);
			this->UpdateTitle();
			::SendMessage(this->GetMDIFrame(), WM_MDISETMENU, 0, 0);
		}
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

void  CGrfFrame::AddToView_(const CAtlList< pair<uint8_t, CString> > *pAddedEntries, bool bClearSelection, bool bSetSelection, bool AreAllInCurrentWorkingDir)
{
CString dn, fn;
CString strCwd(m_pView->GetCwd());
CMyListCtrl *pListCtrl = m_pView->GetListViewPtr();
CMyTreeCtrl *pTreeCtrl = m_pView->GetTreeViewPtr();
int nItemInitialCount(pListCtrl->GetItemCount());
CGrfDirectoryEntry *pdirent;
CGrfFileEntry *precord;
bool  isFirstItem(true);

	  // Unselect previously selected items
	if ( bClearSelection && pListCtrl->GetFirstSelectedIndex() != -1 )
	{
		for ( int iItem = pListCtrl->GetFirstSelectedIndex(); iItem != -1; iItem = pListCtrl->GetNextItem(iItem, LVNI_ALL | LVNI_SELECTED))
		{
			pListCtrl->SetItemState(iItem, 0, LVIS_SELECTED);
		}
	}

	if ( AreAllInCurrentWorkingDir )
	{
	POSITION pos = pAddedEntries->GetHeadPosition();
		while ( pos )
		{
			::BreakPath(pAddedEntries->GetNext(pos).second, dn, fn);
			m_pDoc->GetDirMap().Lookup(dn, pdirent);
			ATLASSERT(pdirent);
			pdirent->files.Lookup(fn, precord);
			ATLASSERT(precord);
			LVFINDINFO lvfi = { 0 };
			lvfi.flags = LVFI_STRING;
			lvfi.psz = fn;
			{
				int iItem = pListCtrl->FindItem(&lvfi, -1);
				if ( iItem == -1 )
				{
					  // For this view, add at end of list
					iItem = pListCtrl->AppendItem(fn, precord);
				}
				else
				{
					pListCtrl->SetItemText(iItem, 0, fn);
					m_pView->FormatItem_List(iItem, precord);
					// Column 4 does not change
				}
				if ( bSetSelection )
				{
					pListCtrl->SetItemState(iItem, LVIS_SELECTED, LVIS_SELECTED);
					if ( isFirstItem )
					{
						if ( bClearSelection )
						{
							pListCtrl->SetItemState(iItem, LVIS_FOCUSED, LVIS_FOCUSED);
						}
						isFirstItem = false;
					}
				}
			}
		}
	}
	else
	{
	POSITION pos = pAddedEntries->GetHeadPosition();
		while ( pos )
		{
			pair<uint8_t, CString> entry(pAddedEntries->GetNext(pos));
			  // Update the current view
			if ( entry.first == 2 )
			{
				  // IF DIRECTORY
				  // ... in the tree view
				this->GetDocPtr()->GetDirMapPtr()->Lookup(entry.second, pdirent);
				CTreeItem ti = pTreeCtrl->InsertFolderByFullPath(entry.second, reinterpret_cast<LPARAM>(pdirent), TVI_SORT);
				  // ... in the list view
				  // IF IsSubDir
				::BreakPath(entry.second, dn, fn);
				if ( dn == strCwd )
				{
					  // Insert folder with correct "file name"
					int iItem = pListCtrl->AppendItem(fn, &(ti.m_hTreeItem));
					if ( bSetSelection )
					{
						pListCtrl->SetItemState(iItem, LVIS_SELECTED, LVIS_SELECTED);
						if ( isFirstItem )
						{
							if ( bClearSelection )
							{
								pListCtrl->SetItemState(iItem, LVIS_FOCUSED, LVIS_FOCUSED);
							}
							isFirstItem = false;
						}
					}
				}
			}
			else
			{
				::BreakPath(entry.second, dn, fn);
				  // Adding in the current working dir?
				if ( dn == strCwd )
				{
					m_pDoc->GetDirMap().Lookup(dn, pdirent);
					ATLASSERT(pdirent);
					pdirent->files.Lookup(fn, precord);
					ATLASSERT(precord);
					int iItem = pListCtrl->AppendItem(fn, precord);
					if ( bSetSelection )
					{
						pListCtrl->SetItemState(iItem, LVIS_SELECTED, LVIS_SELECTED);
						if ( isFirstItem )
						{
							if ( bClearSelection )
							{
								pListCtrl->SetItemState(iItem, LVIS_FOCUSED, LVIS_FOCUSED);
							}
							isFirstItem = false;
						}
					}
				}
				else if ( ::IsSubDir(dn, strCwd) )
				{
					// This can only be possible if CWD's subdir already exists and new files go into the subdir
					if ( bSetSelection )
					{
					CString  strSubName, unrelevant;
						// Strip current working dir, then get top-level subdir name
						if ( strCwd.IsEmpty() )
						{
							strSubName = dn;
						}
						else
						{
							// Extract sub part
							strSubName = dn.Right(dn.GetLength() - (strCwd.GetLength() + 1));
						}
						::GetTopDir(strSubName, strSubName, unrelevant);
					LVFINDINFO findInfo;
						findInfo.flags = LVFI_STRING;
						findInfo.psz = strSubName.operator LPCTSTR();

					int iItem = pListCtrl->FindItem(&findInfo, -1);
						if (iItem != -1)
						{
							pListCtrl->SetItemState(iItem, LVIS_SELECTED, LVIS_SELECTED);
							if ( isFirstItem )
							{
								if ( bClearSelection )
								{
									pListCtrl->SetItemState(iItem, LVIS_FOCUSED, LVIS_FOCUSED);
								}
								isFirstItem = false;
							}
						}
					}
				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

LRESULT CGrfFrame::OnEditImportGrf(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CFileDialogFilter cf_dlgfilter(CGrfDoc::strFilter);
	CFileDialog cf_dlg(
	    TRUE,  // FileOpen
	    NULL,
	    _T(""),
	    OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
	    cf_dlgfilter,
	    NULL
	);

	if ( IDOK == cf_dlg.DoModal() )
	{
		CAtlList< pair<uint8_t, CString> > AddedEntries;
		try
		{
		CWaitCursor w(true, IDC_APPSTARTING, true);
			m_pDoc->ImportGrf(cf_dlg.m_ofn.lpstrFile, m_pView->GetCwd(), &AddedEntries);
			if ( !AddedEntries.IsEmpty() )
			{
				this->AddToView_(&AddedEntries, true, true);
				m_pDoc->SetModifiedFlag();
				  // HINT_DOCUMENT_FILES_AND_DIRS_ADDED
				m_pDoc->OnEntriesAdded(2, &AddedEntries, m_pView);
				this->UpdateTitle();
				::SendMessage(this->GetMDIFrame(), WM_MDISETMENU, 0, 0);
			}
		}
		catch ( const Gen::GrfImportError &gie)
		{
			CString strErrMsg;
			strErrMsg.Format(CString(MAKEINTRESOURCE(IDS_OPENDOC_LOAD_FAIL)), cf_dlg.m_ofn.lpstrFile, gie.code, CString(MAKEINTRESOURCE(CGrfDoc::GetErrorStringResID(gie.code))));
			MessageBox(strErrMsg, CString(MAKEINTRESOURCE(IDS_DOC_ERROR_MSGBOX_TITLE)), MB_ICONERROR | MB_OK);
			if ( gie.code == Gen::GrfImportError::LIBRARY_ERROR )
			{
				MessageBox(gie.message, CString(MAKEINTRESOURCE(IDS_APP_TITLE)), MB_ICONEXCLAMATION | MB_OK);
			}
		}
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT  CGrfFrame::OnEditOpen(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	return m_pView->OnEditOpen();
}

///////////////////////////////////////////////////////////////////////////////

LRESULT  CGrfFrame::OnEditExtract(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	return m_pView->OnEditExtract();
}

///////////////////////////////////////////////////////////////////////////////

LRESULT  CGrfFrame::OnEditRename(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	return m_pView->OnEditRename();
}

///////////////////////////////////////////////////////////////////////////////

LRESULT  CGrfFrame::OnEditDelete(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	return m_pView->OnEditDelete();
}

///////////////////////////////////////////////////////////////////////////////
#if 0
LRESULT CGrfFrame::OnEditDelete(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
CMyListCtrl *pListCtrl = m_pView->GetListViewPtr();
CGrfDirectoryEntry *pdirent(0);
CGrfFileEntry *precord(0);
CString strCwd(m_pView->GetCwd());
	pListCtrl->SetFocus();
	if ( pListCtrl->GetSelectedCount() > 0 )
	{
		int nResponse;
		CString strPrompt;
		CAtlList< pair<uint8_t, CString> > RemovedEntries;

		if ( pListCtrl->GetSelectedCount() == 1 )
		{
		CString strItemText;
			pListCtrl->GetItemText(pListCtrl->GetFirstSelectedIndex(), 0, strItemText);
			strPrompt.Format(CString(MAKEINTRESOURCE(IDS_DELETE_CONFIRMATION_PROMPT)), strItemText);
			nResponse = ::MessageBoxEx(m_hWnd, strPrompt, CString(MAKEINTRESOURCE(IDS_DELETE_WARNING_MSGBOX_TITLE)), MB_ICONWARNING | MB_YESNO, LANGIDFROMLCID(::GetThreadLocale()));
			if ( nResponse == IDNO )
			{
				return 0;
			}
		LVITEM item;
			pListCtrl->GetFirstSelectedItem(&item);
		CString strItem, strTarget;
			pListCtrl->GetItemText(item.iItem, 0, strItem);

		CString strFullPath(strCwd.IsEmpty()? strItem : strCwd + _T("\\") + strItem);

		DWORD_PTR dwptr = pListCtrl->GetItemData( item.iItem );
			  // Directory
			if ( dwptr != 0 )
			{
				CTreeItem entry(reinterpret_cast<HTREEITEM>(dwptr), m_pView->GetTreeViewPtr());
				m_pView->RmdirRecursive(entry);
				m_pDoc->RmdirRecursive(strFullPath, &RemovedEntries);
				m_pDoc->SetModifiedFlag();
				// HINT_DOCUMENT_FILES_AND_DIRS_REMOVED
				m_pDoc->OnEntriesRemoved(2, &RemovedEntries, m_pView);
				this->UpdateTitle();
				::SendMessage(this->GetMDIFrame(), WM_MDISETMENU, 0, 0);
			}
			else
			{
				m_pDoc->GetDirMap().Lookup(strCwd, pdirent);
				ATLASSERT(pdirent);
				pdirent->files.Lookup(strItem, precord);
				ATLASSERT(precord);
				pListCtrl->DeleteItem(item.iItem);
				pdirent->files.RemoveKey(strItem);
				delete(precord);
				RemovedEntries.AddTail( pair<uint8_t, CString>(1, strFullPath) );
				m_pDoc->SetModifiedFlag();
				// HINT_DOCUMENT_FILES_REMOVED
				m_pDoc->OnEntriesRemoved(0, &RemovedEntries, m_pView);
				this->UpdateTitle();
				::SendMessage(this->GetMDIFrame(), WM_MDISETMENU, 0, 0);
			}
		}
		else
		{
			strPrompt.Format(CString(MAKEINTRESOURCE(IDS_DELETE_MULTI_CONFIRMATION_PROMPT)), pListCtrl->GetSelectedCount());
			nResponse = ::MessageBoxEx(this->m_hWnd, strPrompt, CString(MAKEINTRESOURCE(IDS_DELETE_WARNING_MSGBOX_TITLE)), MB_ICONWARNING | MB_YESNO, LANGIDFROMLCID(::GetThreadLocale()));
			if ( nResponse == IDNO )
			{
				return 0;
			}
		int  iItem;
			  // Get Next from -1 because when entries are deleted, they are no longer selected!
			while ( (iItem = pListCtrl->GetNextItem(-1, LVNI_ALL | LVNI_SELECTED)) != -1 )
			{
			CString strItem, strTarget;
				pListCtrl->GetItemText(iItem, 0, strItem);
			CString strFullPath(strCwd.IsEmpty()? strItem : strCwd + _T("\\") + strItem);

				DWORD_PTR dwptr = pListCtrl->GetItemData( iItem );
				// Directory
				if ( dwptr != 0 )
				{
					CTreeItem entry(reinterpret_cast<HTREEITEM>(dwptr), m_pView->GetTreeViewPtr());
					m_pView->RmdirRecursive(entry);
					m_pDoc->RmdirRecursive(strFullPath, &RemovedEntries);
				}
				else
				{
					m_pDoc->GetDirMap().Lookup(strCwd, pdirent);
					ATLASSERT(pdirent);
					pdirent->files.Lookup(strItem, precord);
					ATLASSERT(precord);
					pListCtrl->DeleteItem(iItem);
					pdirent->files.RemoveKey(strItem);
					delete precord;
					RemovedEntries.AddTail( pair<uint8_t, CString>(1, strFullPath) );
				}
			}
			if ( !RemovedEntries.IsEmpty() )
			{
				m_pDoc->SetModifiedFlag();
				// HINT_DOCUMENT_FILES_REMOVED
				m_pDoc->OnEntriesRemoved(0, &RemovedEntries, m_pView);
				this->UpdateTitle();
				::SendMessage(this->GetMDIFrame(), WM_MDISETMENU, 0, 0);
			}
		}
	}
	return 0;
}

#endif
///////////////////////////////////////////////////////////////////////////////

LRESULT  CGrfFrame::OnViewExplorerPane(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
CGrfView &view( this->GetView() );
	return view.SendMessage(uMsg, MAKELONG(ID_PANE_CLOSE, 0), lParam);
}

///////////////////////////////////////////////////////////////////////////////

LRESULT CGrfFrame::OnViewType(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	switch ( wID )
	{
#if _WIN32_WINNT >= 0x501
		case IDM_VIEW_TYPE_TILE:
			// FIXME: add check for Win2k
			ListView_SetView(m_pView->GetListViewPtr()->m_hWnd, LV_VIEW_TILE);
			break;
#endif
		case IDM_VIEW_TYPE_ICONS:
			m_pView->GetListViewPtr()->SetViewType(LV_VIEW_ICON);
			break;
		case IDM_VIEW_TYPE_LIST:
			m_pView->GetListViewPtr()->SetViewType(LV_VIEW_LIST);
			break;
		case IDM_VIEW_TYPE_DETAILS:
			m_pView->GetListViewPtr()->SetViewType(LV_VIEW_DETAILS);
			break;
		default:
			return 1;
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT CGrfFrame::OnWindowCascade(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	MDICascade();
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT CGrfFrame::OnWindowTile(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	MDITile();
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT CGrfFrame::OnWindowArrangeIcons(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	MDIIconArrange();
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT CGrfFrame::OnViewRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
CString strCwd(m_pView->GetCwd());
CGrfView &view( this->GetView() );
	view.OnUpdate(0, HINT_UPDATE_DOCUMENT_REFRESH, reinterpret_cast<LPVOID>(&strCwd));
	return 0;
}


///////////////////////////////////////////////////////////////////////////////

// public

LRESULT CGrfFrame::UpdateTitle()
{
	LPCTSTR path = m_pDoc->GetPathName();
	LPCTSTR name = m_pDoc->GetName();
	int nPathLen = lstrlen(path);
	int nAppendLen = lstrlen(name);

	// 4 == "(*) " (optional)
	CString strFullTitle;

	if ( m_pDoc->IsModified() )
	{
		strFullTitle += _T("(*) ");
	}
	  // FIXME: TODO: (1.2) User option: whether to "show full document path" on MDI child titlebar
	if ( false == true )
	{
		strFullTitle += path;
	}
	else
	{
		strFullTitle += name;
	}

	this->SetWindowText(strFullTitle);

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Move document name to top of MRU. Save to registry and update menu entries

void  CGrfFrame::RegisterMRU_(LPCTSTR szPath)
{
	::SendMessage(this->GetMDIFrame(), UWM_REGISTER_MRU, 0, reinterpret_cast<LPARAM>(szPath));
}

///////////////////////////////////////////////////////////////////////////////

LRESULT  CGrfFrame::OnNotifyKeyDown(LPNMHDR pnmh)
{
HACCEL hAccel = ::LoadAccelerators(ATL::_AtlBaseModule.GetResourceInstance(), MAKEINTRESOURCE(IDC_GRYFF_ACCEL_EX));
LPNMLVKEYDOWN pnkd = reinterpret_cast<LPNMLVKEYDOWN>(pnmh);
MSG msg;
	msg.hwnd = m_pCurrentMsg->hwnd;
	msg.message = WM_KEYDOWN;
	msg.wParam = pnkd->wVKey;
	msg.lParam = 0;
	msg.time = m_pCurrentMsg->time;
	msg.pt = m_pCurrentMsg->pt;
	if ( !m_AcceleratorsDisabled &&
	  NULL != hAccel &&
	  0 != ::TranslateAccelerator(m_hWnd, hAccel, &msg) )
	{
		return 1;
	}
	return 0;
} // OnKeyDown

///////////////////////////////////////////////////////////////////////////////

LRESULT CGrfFrame::OnLVEndLabelEdit(LPNMHDR pnmh)
{
	NMLVDISPINFO *pnlvdi = reinterpret_cast<NMLVDISPINFO*>(pnmh);

	if ( pnlvdi->item.pszText )
	{
	CString s;
		if ( *pnlvdi->item.pszText == _T('\\') )
		{
			AnsiToUnicode(s, pnlvdi->item.pszText+1);
		}
		else
		{
			s = pnlvdi->item.pszText;
		}
		CString strCorrected;
		CString strOldName;
		CGrfDirectoryEntry *pdirent;
		CGrfFileEntry *precord;
		CMyListCtrl *pListCtrl = m_pView->GetListViewPtr();
		CMyTreeCtrl *pTreeCtrl = m_pView->GetTreeViewPtr();
		CString strCwd(m_pView->GetCwd());

		if ( !CGrfDoc::IsValidPartName(s, &strCorrected) )
		{
			::MessageBeep((UINT)-1);
			m_AcceleratorsDisabled = false;
			m_FocusedChild = NULL;
			return FALSE;
		}
		pListCtrl->GetItemText( pnlvdi->item.iItem, 0, strOldName);
		if ( strOldName == strCorrected )
		{
			m_AcceleratorsDisabled = false;
			m_FocusedChild = NULL;
			return FALSE;
		}
		CString strDest;
		if ( strCwd.IsEmpty() )
		{
			strDest.Format(_T("%s"), strCorrected);
		}
		else
		{
			strDest.Format(_T("%s\\%s"), strCwd, strCorrected);
		}
		  // Check if another file or directory has the same name
		if ( GetDoc().FileExists(strDest) || GetDoc().DirExists(strDest) )
		{
			CString strErrorMessage, strFormat(MAKEINTRESOURCE(IDS_CANNOT_RENAME_DUPLICATE));
			strErrorMessage.Format(strFormat, strOldName);
			MessageBox(strErrorMessage, _T("Error while renaming a file or folder"));
			m_AcceleratorsDisabled = false;
			m_FocusedChild = NULL;
			return FALSE;
		}

		DWORD_PTR dwptr = pListCtrl->GetItemData( pnlvdi->item.iItem );
		  // Directory
		if ( dwptr != 0 )
		{
			CTreeItem entry(reinterpret_cast<HTREEITEM>(dwptr), m_pView->GetTreeViewPtr());
			CGrfDirectoryEntry *pdirent = reinterpret_cast<CGrfDirectoryEntry *>(entry.GetData());
			m_pView->ReplaceDirPrefix(entry, pdirent->dir.GetLength(), strDest);
		}
		else
		{
			GetDoc().GetDirMap().Lookup(m_pView->GetCwd(), pdirent);
			pdirent->files.Lookup(strOldName, precord);
			pdirent->files.RemoveKey(strOldName);
			pdirent->files.SetAt(strCorrected, precord);
			  // strCorrected shall always be shorter or of same length as original
			m_pView->UpdateItemName_List(pnlvdi->item.iItem, strCorrected);
		}
		::lstrcpyn(pnlvdi->item.pszText, strCorrected, pnlvdi->item.cchTextMax);
		pListCtrl->SetItem(&(pnlvdi->item));
		GetDoc().SetModifiedFlag();
		// FIXME: TODO: (1.5) notify other views
		this->UpdateTitle();
		::SendMessage(this->GetMDIFrame(), WM_MDISETMENU, 0, 0);
		m_AcceleratorsDisabled = false;
		m_FocusedChild = NULL;
		return TRUE;
	}
	m_AcceleratorsDisabled = false;
	m_FocusedChild = NULL;
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT CGrfFrame::OnTVEndLabelEdit(LPNMHDR pnmh)
{
	NMTVDISPINFO *pntvdi = reinterpret_cast<NMTVDISPINFO*>(pnmh);

	if ( pntvdi->item.pszText )
	{
		CString s;
		if ( *pntvdi->item.pszText == _T('\\') )
		{
			AnsiToUnicode(s, pntvdi->item.pszText+1);
		}
		else
		{
			s = pntvdi->item.pszText;
		}
		CString strCorrected;
		CString strOldName;
		CMyTreeCtrl *pTreeCtrl = m_pView->GetTreeViewPtr();
		CGrfDirectoryEntry *pdirent;

		if ( !CGrfDoc::IsValidPartName(s, &strCorrected) )
		{
			::MessageBeep((UINT)-1);
			m_AcceleratorsDisabled = false;
			m_FocusedChild = NULL;
			return FALSE;
		}
		pTreeCtrl->GetItemText(pntvdi->item.hItem, strOldName);
		if ( strOldName == strCorrected )
		{
			m_AcceleratorsDisabled = false;
			m_FocusedChild = NULL;
			return FALSE;
		}
		CTreeItem entry(pntvdi->item.hItem, pTreeCtrl);
		pdirent = reinterpret_cast<CGrfDirectoryEntry *>(entry.GetData());
		CString strDest;
		CString strOriginal(pdirent->dir);
		  // Check currently renamed dir
		::BreakPath(pdirent->dir, s, strDest);
		if ( s.IsEmpty() )
		{
			strDest = strCorrected;
		}
		else
		{
			strDest.Format(_T("%s\\%s"), s, strCorrected);
		}
		  // Check if file in parent dir, or directory
		if ( GetDoc().FileExists(strDest) || GetDoc().DirExists(strDest) )
		{
			CString strErrorMessage, strFormat(MAKEINTRESOURCE(IDS_CANNOT_RENAME_DUPLICATE));
			strErrorMessage.Format(strFormat, strOldName);
			this->MessageBox(strErrorMessage, _T("Error while renaming a file or folder"), MB_ICONERROR | MB_OK);
			m_AcceleratorsDisabled = false;
			m_FocusedChild = NULL;
			return FALSE;
		}

		m_pView->ReplaceDirPrefix(entry, strOriginal.GetLength(), strDest);		  // Check if another file or directory has the same name
		::lstrcpyn(pntvdi->item.pszText, strCorrected, pntvdi->item.cchTextMax);
		GetDoc().SetModifiedFlag();
		// FIXME: TODO: (1.5) notify other views
		this->UpdateTitle();
		::SendMessage(this->GetMDIFrame(), WM_MDISETMENU, 0, 0);
		m_AcceleratorsDisabled = false;
		m_FocusedChild = NULL;
		return TRUE;
	}
	m_AcceleratorsDisabled = false;
	m_FocusedChild = NULL;
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT CGrfFrame::OnTVDrop(OleDropInfo &odi)
{
//const
static FORMATETC  fmt_1 = { UCF_GRYFF_ENTRIES_INTERNAL /* cfFormat */, NULL /* ptd */, DVASPECT_CONTENT /* dwAspect */, -1, TYMED_HGLOBAL /* tymed */};
STGMEDIUM  medium;
	{
		medium.tymed          = TYMED_HGLOBAL;
		medium.hGlobal        = 0;
		medium.pUnkForRelease = 0;
	}

bool  AttemptGeneralFormat(true);
CGrfDoc * pDocThis = m_pDoc; //reinterpret_cast<CGrfDoc *>(::SendMessage(m_hWndParentView, UWM_GETDOCPTR, 0, 0));

	// First, try the in-process transfer
	if ( SUCCEEDED(odi.pDataObject->GetData(&fmt_1, &medium)) )
	{
	do  // construct to break out easily
	{
		// FIXME: TODO: (1.5) Implement drag drop extensively
	CGrfDirectoryEntry *pdirent = odi.pDirent;
	CString strDropDir(pdirent->dir);
	HGLOBAL  hGlobal(medium.hGlobal);
	DragDropInfo *pInfoInterpret = reinterpret_cast<DragDropInfo *>(::GlobalLock(hGlobal));
		if ( pInfoInterpret == NULL )
		{
			*odi.phr = E_UNEXPECTED;
			break;
		}
		if ( !::IsWindow(pInfoInterpret->hWnd) ||
			pInfoInterpret->dwProcessId != ::GetCurrentProcessId() )
		{
			this->MessageBox(_T("Sorry, only drag and drop within the same document is allowed in this version"), CString(MAKEINTRESOURCE(IDS_APP_TITLE)), MB_ICONERROR | MB_OK);
			::GlobalUnlock(hGlobal);
			// Don't fail, attempt to use general format first
			break;
		}
//	SIZE_T  DataSize = pInfoInterpret->lStructSize;
	TCHAR *pData = (TCHAR*)((LPBYTE)(pInfoInterpret) + sizeof(DragDropInfo));
	TCHAR *pszDocPathName = pData;
		pData += lstrlen(pData) + 1;
	TCHAR *pszDocName = pData;
		pData += lstrlen(pData) + 1;

	TCHAR *pszSourceCwd = pData;
		pData += lstrlen(pData) + 1;

	BOOL   IsUntitled = *pszDocPathName == _T('\0')? TRUE : FALSE;
	TCHAR  *pszFullDocPath = IsUntitled? pszDocName : pszDocPathName;
		// FIXME: TODO: (1.5) Adjust this to transfer data between two documents. This requires a subscriber/listener model b/c doc changes are asynchronous.
	CGrfDoc * pDocSource = reinterpret_cast<CGrfDoc *>(::SendMessage(pInfoInterpret->hWnd, UWM_GETDOCPTR, 0, reinterpret_cast<LPARAM>(pszFullDocPath)));
		if ( pDocThis != pDocSource || pDocThis == 0 || pDocSource == 0)
		{
			::GlobalUnlock(hGlobal);
			*odi.phr = E_UNEXPECTED;
			break;
		}

	CAtlList< pair<uint8_t, CString> > AddedEntries;
		if ( !strDropDir.IsEmpty() )
		{
			strDropDir += _T('\\');
		}
	CString strSourceDir(pszSourceCwd);
		if ( !strSourceDir.IsEmpty() )
		{
			strSourceDir += _T('\\');
		}

	TCHAR *pFirstSourceEntry = pData;
		  // Test special restrictions when transfering within the same document
		if ( pDocThis == pDocSource )
		{
		CAtlList<CString>  DraggedFolders;
		CGrfDirectoryEntry *pSourceDirent;
			while ( *pData /* != _T('\0') */ )
			{
			CString  strDn, strFn;
			CString  strOrigDir = strSourceDir + pData;
				::BreakPath(strOrigDir, strDn, strFn);
				if ( strDn == pdirent->dir )
				{
					  // Trying to "do nothing"
					this->MessageBox(_T("Cannot move file or folder: The name of source and target entries are identical."), CString(MAKEINTRESOURCE(IDS_APP_TITLE)), MB_ICONEXCLAMATION | MB_OK);
					::GlobalUnlock(hGlobal);
					*odi.phr = E_UNEXPECTED;
					break;
				}
				if ( pDocSource->DirExists(strOrigDir) )
				{
					DraggedFolders.AddTail(strOrigDir);
					// Add sub-folders
					POSITION pos = this->GetDocPtr()->GetDirMapPtr()->FindFirstKeyAfter(strOrigDir);
				CString  strDirectory;
					ATLASSERT( pos != 0 );
				CString strTargetSubDir;
					for ( this->GetDocPtr()->GetDirMapPtr()->GetNext(pos); pos; this->GetDocPtr()->GetDirMapPtr()->GetNext(pos) )
					{
						this->GetDocPtr()->GetDirMapPtr()->GetAt(pos, strDirectory, pSourceDirent);
						if ( ::IsSubDir(strDirectory, strOrigDir) )
						{
							DraggedFolders.AddTail(strDirectory);
						}
						else
						{
							break;
						}
					}
				}
				pData += lstrlen(pData) + 1;
			}
			if ( FAILED(*odi.phr) )
			{
				break;
			}
			if ( DraggedFolders.Find(pdirent->dir) )
			{
				  // Trying to move a folder into one of its subfolders
				this->MessageBox(_T("Cannot move file or folder: A target entry is a subfolder of a source entry."), CString(MAKEINTRESOURCE(IDS_APP_TITLE)), MB_ICONEXCLAMATION | MB_OK);
				::GlobalUnlock(hGlobal);
				*odi.phr = E_UNEXPECTED;
				break;
			}

			// FIXME: TODO: (1.x) do additional DnD tests here. For instance, check the entry type of overwritten entries.
			  // Restore
			pData = pFirstSourceEntry;
		}
		while ( *pData /* != _T('\0') */ )
		{
		CString  strEntry(pData);
			// lookup from source
		CGrfDirectoryEntry *pSourceDirent;
		CGrfFileEntry *precord, *precordcopy;
			if ( pDocSource->FileExists(strSourceDir + strEntry, &precord) )
			{
				  // Handle "File" case
				precordcopy = new CGrfFileEntry(*precord);
				if ( pdirent->files.Lookup(strEntry, precord) != false )
				{
					// Replace existing entry
					delete precord;
				}
				precord = 0;
				// drop on target
				pdirent->files.SetAt(strEntry, precordcopy);
				AddedEntries.AddTail(pair<uint8_t, CString>(1, CString(strDropDir + pData)));
			}
			else if ( pDocSource->DirExists(strSourceDir + strEntry, &pSourceDirent) )
			{
			CString  strFullDirectoryPath(strDropDir + strEntry);
				GetDoc().ImportDirectoryEntry(pDocSource, pSourceDirent, strFullDirectoryPath, &AddedEntries);
				// TODO: merge with old if exists
			}
			pData += lstrlen(pData) + 1;
		}

		// Added directories
		if ( !AddedEntries.IsEmpty() )
		{
			pDocThis->SetModifiedFlag();
		CMyTreeCtrl *pTreeCtrl = m_pView->GetTreeViewPtr();
		POSITION pos = AddedEntries.GetHeadPosition();
			while ( pos )
			{
				pair<uint8_t, CString> entry(AddedEntries.GetNext(pos));
				// Update the current view
				if ( entry.first == 2 )
				{
					// IF DIRECTORY
					// ... in the tree view
					pDocThis->GetDirMapPtr()->Lookup(entry.second, pdirent);
					CTreeItem ti = pTreeCtrl->InsertFolderByFullPath(entry.second, reinterpret_cast<LPARAM>(pdirent), TVI_SORT);
				}
			}
			// HINT_DOCUMENT_FILES_AND_DIRS_ADDED
			pDocThis->OnEntriesAdded(2, &AddedEntries, NULL);
			this->UpdateTitle();
			::SendMessage(this->GetMDIFrame(), WM_MDISETMENU, 0, 0);
		}

		// Succeeded in using internal format
		AttemptGeneralFormat = false;
	} while ( 0 );  // breaking out easily
		if ( FAILED(*odi.phr) )
		{
			return 0;  // do NOT GlobalFree() on failure
		}
		if ( medium.pUnkForRelease )
		{
			::ReleaseStgMedium(&medium);
		}
		else
		{
			::GlobalFree(medium.hGlobal);
		}
	}

	if ( AttemptGeneralFormat )
	{
		// FIXME: TODO: UCF_GRYFF_ENTRIES
		*odi.phr = E_UNEXPECTED;
	}
	return 0;
}  // OnTVDrop

///////////////////////////////////////////////////////////////////////////////

int  CGrfFrame::GetViewTypeId() const
{
	return GetViewPtr()->GetViewTypeId();
}

///////////////////////////////////////////////////////////////////////////////

bool  CGrfFrame::IsSinglePane() const
{
	return  ::SendMessage(this->GetViewPtr()->m_hWnd, UWM_GETSINGLEPANEMODE, 0, 0) != (LRESULT)SPLIT_PANE_NONE;
}

///////////////////////////////////////////////////////////////////////////////

void  CGrfFrame::UpdateUI()
{
	m_pView->UpdateUI();
}

///////////////////////////////////////////////////////////////////////////////

/**
	Overridden to fix ATL bug when "delete this;" is performed OnFinalMessage()
	while message is not final.
**/
//static
LRESULT CALLBACK  CGrfFrame::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	BaseFrameClass_* pThis = (BaseFrameClass_*)hWnd;
	// set a ptr to this message and save the old value
	_ATL_MSG msg(pThis->m_hWnd, uMsg, wParam, lParam);
	const _ATL_MSG* pOldMsg = pThis->m_pCurrentMsg;
	pThis->m_pCurrentMsg = &msg;
	// pass to the message map to process
	LRESULT lRes;
	BOOL bRet = pThis->ProcessWindowMessage(pThis->m_hWnd, uMsg, wParam, lParam, lRes, 0);
	// restore saved value for the current message
	ATLASSERT(pThis->m_pCurrentMsg == &msg);

	// do the default processing if message was not handled
	if(!bRet)
	{
		if(uMsg != WM_NCDESTROY)
			lRes = pThis->DefWindowProc(uMsg, wParam, lParam);
		else
		{
			// unsubclass, if needed
			LONG_PTR pfnWndProc = ::GetWindowLongPtr(pThis->m_hWnd, GWLP_WNDPROC);
			lRes = pThis->DefWindowProc(uMsg, wParam, lParam);
			if(pThis->m_pfnSuperWindowProc != ::DefWindowProc && ::GetWindowLongPtr(pThis->m_hWnd, GWLP_WNDPROC) == pfnWndProc)
				::SetWindowLongPtr(pThis->m_hWnd, GWLP_WNDPROC, (LONG_PTR)pThis->m_pfnSuperWindowProc);
			// mark window as destryed
			pThis->m_dwState |= WINSTATE_DESTROYED;
		}
	}
	//FIX
	pThis->m_pCurrentMsg = pOldMsg;
	if((pThis->m_dwState & WINSTATE_DESTROYED) && pThis->m_pCurrentMsg == NULL)
	{
		// clear out window handle
		HWND hWnd = pThis->m_hWnd;
		pThis->m_hWnd = NULL;
		pThis->m_dwState &= ~WINSTATE_DESTROYED;
		// clean up after window is destroyed
		pThis->OnFinalMessage(hWnd);
	}
	return lRes;
}

///////////////////////////////////////////////////////////////////////////////

/**
	IUnknown QueryInterface()
	Supported interfaces are IUnknown + IDropTarget
**/
HRESULT STDMETHODCALLTYPE  CGrfFrame::QueryInterface(REFIID iid, LPVOID *ppvObject)
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
ULONG STDMETHODCALLTYPE  CGrfFrame::AddRef()
{
	return  static_cast<ULONG>(::InterlockedIncrement(&m_RefCount));
}

///////////////////////////////////////////////////////////////////////////////

/**
	IUnknown Release()
	Decrease reference count
**/
ULONG STDMETHODCALLTYPE  CGrfFrame::Release()
{
	return static_cast<ULONG>(::InterlockedDecrement(&m_RefCount));
}

///////////////////////////////////////////////////////////////////////////////

/**
	IDropTarget DragEnter()
**/
HRESULT STDMETHODCALLTYPE  CGrfFrame::DragEnter(IDataObject *pDataObject, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
HRESULT  func_hr(S_OK);
	m_pDataObject = pDataObject;
	if ( m_pDropTargHlp )
	{
	POINT pt = { ptl.x, ptl.y };
		m_pDropTargHlp->DragEnter(m_hWnd, pDataObject, &pt, *pdwEffect);
	}
	  // Inspect dragged data object
	if ( IsAcceptedFormat_(pDataObject) )
	{
		*pdwEffect = ::GetAsyncKeyState(VK_MENU) == 0 ? DROPEFFECT_COPY : DROPEFFECT_MOVE;
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
HRESULT STDMETHODCALLTYPE  CGrfFrame::DragOver(DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
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
	if ( IsAcceptedFormat_(m_pDataObject) )
	{
		*pdwEffect = ::GetAsyncKeyState(VK_MENU) == 0 ? DROPEFFECT_COPY : DROPEFFECT_MOVE;
	}
	else
	{
		*pdwEffect = DROPEFFECT_NONE;
	}
	return func_hr;
}

///////////////////////////////////////////////////////////////////////////////

/**
	IDropTarget DragLeave()
**/
HRESULT STDMETHODCALLTYPE  CGrfFrame::DragLeave()
{
	if ( m_pDropTargHlp )
	{
		m_pDropTargHlp->DragLeave();
	}
	m_pDataObject = 0;
	return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

/**
	IDropTarget Drop()
**/
HRESULT STDMETHODCALLTYPE  CGrfFrame::Drop(IDataObject *pDataObject, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
	if ( !m_pDropTargHlp )
	{
		return E_FAIL;
	}
	if ( m_pDropTargHlp )
	{
	POINT pt = { ptl.x, ptl.y };
		m_pDropTargHlp->Drop(pDataObject, &pt, *pdwEffect);
	}

FORMATETC  fmt = { 0 };

	fmt.ptd      = 0;
	fmt.dwAspect = DVASPECT_CONTENT;
	fmt.lindex   = -1;
	fmt.tymed    = TYMED_HGLOBAL;

STGMEDIUM  medium;

	medium.tymed          = TYMED_HGLOBAL;
	medium.hGlobal        = 0;
	medium.pUnkForRelease = 0;

HRESULT  func_hr(S_OK);
	  // First, try to retrieve a list of files
	fmt.cfFormat = CF_HDROP;
	if ( SUCCEEDED(pDataObject->GetData(&fmt, &medium)) )
	{
		CString strCwd(m_pView->GetCwd());
		HDROP  hDrop(reinterpret_cast<HDROP>(medium.hGlobal));
		TCHAR  szFileName[MAX_PATH];
		CAtlList< pair<uint8_t, CString> > ImportFiles, AddedEntries;
		bool  UseConversion(::GetAsyncKeyState(VK_MENU) != 0);
		for ( int i = 0; ::DragQueryFile(hDrop, i, szFileName, MAX_PATH) > 0; ++i )
		{
			if ( ::PathIsDirectory(szFileName) )
			{
			CString dn, fn(szFileName), s;
				::BreakPath(fn, dn, fn);
				if ( UseConversion )
				{
					AnsiToUnicode(s, fn);
				}
				else
				{
					s = fn;
				}
			CString  strFullDirectoryPath(strCwd.IsEmpty()? s : strCwd + _T("\\") + s);
				GetDoc().ImportDirectoryTree(szFileName, strFullDirectoryPath, UseConversion, &AddedEntries);
			}
			else
			{
				ImportFiles.AddTail( pair<uint8_t, CString>(1 | (UseConversion ? 0x00000040 : 0), szFileName) );
			}
		}

		// Added directories
		if ( !AddedEntries.IsEmpty() )
		{
			this->AddToView_(&AddedEntries, true, true);
			GetDoc().SetModifiedFlag();
			// HINT_DOCUMENT_FILES_AND_DIRS_ADDED
			GetDoc().OnEntriesAdded(2, &AddedEntries, m_pView);
			this->UpdateTitle();
			::SendMessage(this->GetMDIFrame(), WM_MDISETMENU, 0, 0);
		}

		// Now, add files registered in the ImportFiles collection (don't clear selection,
		// as it would unselect what's been selected above
		if ( !ImportFiles.IsEmpty() && this->OnImportFiles_(&ImportFiles, AddedEntries.IsEmpty()) != IDOK )
		{
			func_hr = E_FAIL;
		}

		if ( medium.pUnkForRelease )
		{
			::ReleaseStgMedium(&medium);
		}
		else
		{
			::GlobalFree(medium.hGlobal);
		}
	}
	else
	{
		func_hr = E_UNEXPECTED;
	}

	*pdwEffect = DROPEFFECT_NONE;
	if ( SUCCEEDED(func_hr) )
	{
		// Bring to front after drop
		if ( ::AllowSetForegroundWindow(::GetCurrentProcessId()) )
		{
			::SetForegroundWindow(this->GetMDIFrame());
			this->SetFocus();
		}
	}
	return func_hr;
}

///////////////////////////////////////////////////////////////////////////////

/**
	Returns whether a supported clipboard format is detected when
	dragging an object over the window
**/
bool  CGrfFrame::IsAcceptedFormat_(IDataObject *pDataObject)
{
bool  Accepted(false);

FORMATETC  fmt = { 0 };

	fmt.ptd      = 0;
	fmt.dwAspect = DVASPECT_CONTENT;
	fmt.lindex   = -1;
	fmt.tymed    = TYMED_HGLOBAL;

	fmt.cfFormat = CF_HDROP;
	if ( SUCCEEDED(pDataObject->QueryGetData(&fmt)) )
	{
		Accepted = true;
	}
	return Accepted;
}

///////////////////////////////////////////////////////////////////////////////
// public static
void  CGrfFrame::Save_DlgInit(LPARAM lParam)
{
CDocumentSaverData *pSaveData = reinterpret_cast<CDocumentSaverData *>(lParam);
SaveCommand<CGrfFrame>::WorkInvoker workInvoker =
	{
	/*InitializeCallback*/           &CGrfFrame::Save_Initialize,
	/*InitializeProgressCallback*/   &CGrfFrame::Save_InitializeProgress,
	/*ProgressCallback*/             &CGrfFrame::Save_Progress,
	/*StatusCallback*/               &CGrfFrame::Save_StatusCaption,
	/*StatusCallback*/               &CGrfFrame::Save_StatusCaptionEx,
	/*FinishCallback*/               &CGrfFrame::Save_Finish
	};

auto_ptr< SaveCommand<CGrfFrame> > pCommand(new SaveCommand<CGrfFrame>(pSaveData->pThis, lParam, workInvoker));
	pSaveData->pCmd = pCommand.get();

HANDLE hThread = pCommand->BeginInvoke();
	if ( hThread == 0 )
	{
		pSaveData->pThis->MessageBox(_T("Failed to create save thread"));
	}
	else
	{
		pCommand.release();
		::CloseHandle(hThread);
	}
}

// public static
void  CGrfFrame::Save_Initialize(CGrfFrame *pThis, LPARAM lParam, SaveCommand<CGrfFrame> *pThisCommand)
{
	pThis->OnSaveInitialize(lParam, pThisCommand);
}
// public static
void  CGrfFrame::Save_Finish(CGrfFrame *pThis, LPARAM lParam, SaveCommand<CGrfFrame> *pThisCommand)
{
	pThis->OnSaveFinish(lParam, pThisCommand);
}

// public static
void  CGrfFrame::Save_InitializeProgress(CGrfFrame *pThis, int minimum, int maximum)
{
	pThis->OnSaveInitializeProgress(minimum, maximum);
}
// public static
void  CGrfFrame::Save_Progress(CGrfFrame *pThis, int progress)
{
	pThis->OnSaveProgress(progress);
}
// public static
void  CGrfFrame::Save_StatusCaption(CGrfFrame *pThis, const CString &statusText)
{
	pThis->OnSaveStatusCaption(statusText);
}
// public static
void  CGrfFrame::Save_StatusCaptionEx(CGrfFrame *pThis, const CString &statusText)
{
	pThis->OnSaveStatusCaptionEx(statusText);
}

///////////////////////////////////////////////////////////////////////////////

void  CGrfFrame::OnSaveInitialize(LPARAM lParam, SaveCommand<CGrfFrame> *pThisCommand)
{
CDocumentSaverData * pSaveData = reinterpret_cast<CDocumentSaverData *>(lParam);
	m_SaveParam = lParam;
CString strBaseTitle(MAKEINTRESOURCE(IDS_PACKING_GRF));
	pSaveData->pDlg->SetBaseTitle(strBaseTitle);
	pSaveData->pDlg->SetStatus1(CString(MAKEINTRESOURCE(IDS_PACKING_GRF_ENTRY)));
}

///////////////////////////////////////////////////////////////////////////////

void  CGrfFrame::OnSaveFinish(LPARAM lParam, SaveCommand<CGrfFrame> *pThisCommand)
{
	if ( !pThisCommand->m_Cancelling )
	{
	CGrfDoc &doc( this->GetDoc() );
	CDocumentSaverData * pSaveData = reinterpret_cast<CDocumentSaverData *>(lParam);
		delete pSaveData->pCmd;
		delete pSaveData->pPak;
		delete pSaveData->pCache;
		delete pSaveData->pIoManager;
		m_SaveParam = 0;

	auto_ptr<CGrfCache> pCache( new CGrfCache() );
		// FixMe correct HasInvalidEntries() for order
	LRESULT  lResult = doc.RemoveEntries(pSaveData->pInvalidEntries);
		ATLASSERT(lResult == pSaveData->pInvalidEntries->GetCount());
		doc.SetGrfOrigin(pCache.get(), pSaveData->path);
		doc.SetDocumentName(pSaveData->path);
		doc.SetModifiedFlag(FALSE);
		pSaveData->pDlg->EndDialog(IDOK);

		// Better refresh all the doc.
		doc.UpdateAllViews(0, HINT_UPDATE_DOCUMENT_REFRESH, static_cast<LPVOID>(0));
	}
}

///////////////////////////////////////////////////////////////////////////////

void  CGrfFrame::OnSaveInitializeProgress(int minimum, int maximum)
{
CDocumentSaverData * pSaveData = reinterpret_cast<CDocumentSaverData *>(m_SaveParam);
	pSaveData->pDlg->SetRange(minimum, maximum);
}

///////////////////////////////////////////////////////////////////////////////

void  CGrfFrame::OnSaveProgress(int progress)
{
CDocumentSaverData * pSaveData = reinterpret_cast<CDocumentSaverData *>(m_SaveParam);
	pSaveData->pDlg->SetPos(progress);
const unsigned int count = static_cast<unsigned int>(pSaveData->pEntries.GetCount());
	if ( progress == count )
	{
		pSaveData->pEntries.RemoveAll();
		delete pSaveData->pCache;
		pSaveData->pCache = 0;
		pSaveData->pDlg->SetWindowCaption(_T("Save complete"));
	}
	else
	{
		if ( pSaveData->pDlg->IsCancelling() )
		{
			pSaveData->pCmd->Cancel();
		}
	CString strTitle;
		strTitle.Format(_T("%d%% done"), static_cast<int>(static_cast<double>(100*progress) / count));  // count cannot be 0
		pSaveData->pDlg->SetWindowCaption(strTitle);
	}
}

///////////////////////////////////////////////////////////////////////////////

void  CGrfFrame::OnSaveStatusCaption(const CString &statusText)
{
CDocumentSaverData * pSaveData = reinterpret_cast<CDocumentSaverData *>(m_SaveParam);
	pSaveData->pDlg->SetStatus1(statusText);
}

///////////////////////////////////////////////////////////////////////////////

void  CGrfFrame::OnSaveStatusCaptionEx(const CString &statusText)
{
CDocumentSaverData * pSaveData = reinterpret_cast<CDocumentSaverData *>(m_SaveParam);
	pSaveData->pDlg->SetStatus2(statusText);
}
