// $Id: WndMain.cpp 12 2005-08-14 01:42:51Z Rasqual $

// *
// *   gryff, a GRF archive management utility, src/WndMain.cpp
// *   Copyright (C) 2003, 2004 Rasqual Twilight
// *
// *   This program is distributed in the hope that it will be useful,
// *   but WITHOUT ANY WARRANTY; without even the implied warranty of
// *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// *

// * WndMain.cpp
// * Main window definition (implementation file.)
// *
// *@author $Author: Rasqual $
// *@version $Revision: 12 $
// *

#include  "stdwtl.h"
#include  <set>
#include  <codeproject/filedialogfilter.h>

#pragma hdrstop
#include  "resource.h"
#include  "WndMain.h"

#include  "DlgAbout.h"
#include  "FrmGrf.h"  // CGrfFrame
#include  "DocGrf.h"  // CGrfDoc

using std::pair;
using std::set;

// ** BEGIN local anonymous namespace
namespace {

///////////////////////////////////////////////////////////////////////////////

// These structures are used when enumerating child frames
// e.g. when closing all of them.

struct CMdiEnumerator
{
public:
	CMainWindow      *pWndMain;
	CAtlList<HWND>   EligibleWindows;
public:
	CMdiEnumerator(CMainWindow *pMainWindow) : pWndMain(pMainWindow)
	{
	}
};

///////////////////////////////////////////////////////////////////////////////

struct CCloseContext : public CMdiEnumerator
{
public:
	WORD             wID;      // command ID
	CGrfFrame        *pFrame;  // current active frame, or 0 in certain conditions
public:
	CCloseContext(CMainWindow *pMainWindow, WORD wCommandId, CGrfFrame *pActiveFrame) : CMdiEnumerator(pMainWindow), wID(wCommandId), pFrame(pActiveFrame)
	{
	}
};

///////////////////////////////////////////////////////////////////////////////

struct CMatchDocContext : public CMdiEnumerator
{
public:
	CGrfDoc        *pDoc;  // Retrieved document
public:
	CMatchDocContext(CMainWindow *pMainWindow, CGrfDoc *pDocument) : CMdiEnumerator(pMainWindow), pDoc(pDocument)
	{
	}
};

///////////////////////////////////////////////////////////////////////////////

/**
	Enumerates CGrfFrame child windows to be closed and modifies the
	lParam parameter which is actually a pointer to a CCloseContext structure.
	Based on the (lParam)->wID value and active (lParam)->pFrame,
	window handles are added to the (lParam)->EligibleWindows member.
	Busy windows which are not in a working state (busy) are not added.
**/
BOOL CALLBACK  EnumChildProc_ForCloseAll(HWND hWnd, LPARAM lParam)
{
CCloseContext* pctxt = reinterpret_cast<CCloseContext*>(lParam);
	ATLASSERT(!::IsBadReadPtr(pctxt, sizeof(CCloseContext)));
	ATLASSERT(pctxt->pWndMain);
	  // if .pFrame is 0, all non-busy child windows shall be closed unconditionally
	if ( pctxt->pFrame == 0 )
	{
		CGrfFrame *pChild = CGrfFrame::FromHandle(hWnd);
		  // Is a MDI child frame
		if ( pChild && !pChild->IsDocumentBusy() )
		{
			pctxt->EligibleWindows.AddTail(hWnd);
		}
	}
	else
	{
		  // Not current frame
		if ( hWnd != pctxt->pFrame->m_hWnd )
		{
			CGrfFrame *pChild = CGrfFrame::FromHandle(hWnd);
			  // Is a MDI child frame
			if ( pChild && !pChild->IsDocumentBusy() )
			{
				if ( pctxt->wID == IDM_WINDOW_CLOSE_ALL_BUT_CURRENT )
				{
					pctxt->EligibleWindows.AddTail(hWnd);
				}
				else if ( pctxt->wID == IDM_FILE_CLOSE_ALL_BUT_CURRENT )
				{
					  // is not view to same doc
					if ( pChild->GetDocPtr() != pctxt->pFrame->GetDocPtr() )
					{
						pctxt->EligibleWindows.AddTail(hWnd);
					}
				}
			}
		}
	}
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////

/**
	Enumerates CGrfFrame child windows having a view of the
	document specified in the lParam parameter which is actually a pointer
	to a CMatchDocContext structure.
	Based on the (lParam)->pDoc value, window handles are added to the
	(lParam)->EligibleWindows member.
**/
BOOL CALLBACK  EnumChildProc_ForSpecDoc(HWND hWnd, LPARAM lParam)
{
CMatchDocContext* pctxt = reinterpret_cast<CMatchDocContext*>(lParam);
	ATLASSERT(!::IsBadReadPtr(pctxt, sizeof(CMatchDocContext)));
	ATLASSERT(pctxt->pWndMain);

	CGrfFrame *pChild = CGrfFrame::FromHandle(hWnd);
	  // Is a MDI child frame
	if ( pChild && pctxt->pDoc == pChild->GetDocPtr() )
	{
		pctxt->EligibleWindows.AddTail(hWnd);
	}
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK  EnumChildProc_ForSaveAll(HWND hWnd, LPARAM lParam)
{
CCloseContext* pctxt = reinterpret_cast<CCloseContext*>(lParam);
	ATLASSERT(!::IsBadReadPtr(pctxt, sizeof(CCloseContext)));
	ATLASSERT(pctxt->pWndMain);
set<HWND> windows;
CGrfFrame *pChild = CGrfFrame::FromHandle(hWnd);
	  // Is a MDI child frame
	if ( pChild && !pChild->IsDocumentBusy() && pChild->IsModified() && *(pChild->GetDocPtr()->GetPathName()) != _T('\0') )
	{
		windows.insert(hWnd);
	}
	for ( set<HWND>::const_iterator it = windows.begin();
		it != windows.end(); ++it )
	{
		pctxt->EligibleWindows.AddTail(*it);
	}
	return TRUE;
}

}
// ** END local anonymous namespace

///////////////////////////////////////////////////////////////////////////////

CMainWindow::CMainWindow() : m_pGrfDocTemplate(new CDocTemplate<CGrfDoc, CGrfView, CGrfFrame, IDC_GRYFFCHILD>), m_RefCount(0)
{
	if ( UWM_ARE_YOU_ME == 0 )
	{
		UWM_ARE_YOU_ME = ::RegisterWindowMessage(UWM_ARE_YOU_ME_MSG);
	}
	if ( UCF_GRYFF_ENTRIES == 0 )
	{
		UCF_GRYFF_ENTRIES = ::RegisterClipboardFormat(UCF_GRYFF_ENTRIES_FMT);
		UCF_GRYFF_ENTRIES_INTERNAL = ::RegisterClipboardFormat(UCF_GRYFF_ENTRIES_INTERNAL_FMT);
	}
}

///////////////////////////////////////////////////////////////////////////////

CMainWindow::~CMainWindow()
{
	delete m_pGrfDocTemplate;
	m_pGrfDocTemplate = 0;
	if ( m_RefCount >= 1 )
	{
		ATLVERIFY(this->Release() == 0);
	}
}

///////////////////////////////////////////////////////////////////////////////

BOOL CMainWindow::PreTranslateMessage(MSG* pMsg)
{
	if ( baseFrameClass_::PreTranslateMessage(pMsg) )
	{
		return TRUE;  // translated
	}
	  // Forward to active frame, if applicable
	HWND hWnd = this->MDIGetActive();
	if ( hWnd != NULL )
	{
		return (BOOL)::SendMessage(hWnd, WM_FORWARDMSG, 0, (LPARAM)pMsg);
	}
	return FALSE;  // not translated
}

///////////////////////////////////////////////////////////////////////////////

LRESULT CMainWindow::OnCreate(LPCREATESTRUCT /*lpcs*/)
{
	m_pDropTargHlp = 0;  // shouldn't be necessary
	::CoCreateInstance(CLSID_DragDropHelper, NULL, CLSCTX_INPROC_SERVER,
		IID_IDropTargetHelper, reinterpret_cast<LPVOID *>(&m_pDropTargHlp));

	if ( m_pDropTargHlp == 0 )
	{
		MessageBox(_T("Error when creating the IDropTargetHelper instance"), CString(MAKEINTRESOURCE(IDS_INIT_ERROR_MSGBOX_TITLE)), MB_ICONERROR | MB_OK);
		return -1;
	}
	m_RefCount = 1;

	::RegisterDragDrop(m_hWnd, this);

	// create command bar window
	HWND hWndCmdBar;
	if ( (hWndCmdBar = m_CmdBar.Create(m_hWnd, rcDefault, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE)) == NULL )
	{
		MessageBox(_T("Error when creating the Command Bar"), CString(MAKEINTRESOURCE(IDS_INIT_ERROR_MSGBOX_TITLE)), MB_ICONERROR | MB_OK);
		return -1;
	}

const int  FILE_MENU_POSITION(0);

	// attach menu (actually a rebar)
	// FIXME: TODO: (1.8) If a language file is set, read menu strings from it (and check missing lines)
	if ( 1 )
	{
		m_CmdBar.AttachMenu(this->GetMenu());
	}
	else
	{
	CMenu  m_mainMenu;
		m_mainMenu.CreateMenu();

	CMenuHandle mhFile;
		mhFile.CreatePopupMenu();
		{ // BEGIN "File" menu popup
			mhFile.AppendMenu(MF_STRING, IDM_FILE_NEW,                CString(MAKEINTRESOURCE(IDM_FILE_NEW)));
			mhFile.AppendMenu(MF_STRING, IDM_FILE_NEW_FROM_GRF,       CString(MAKEINTRESOURCE(IDM_FILE_NEW_FROM_GRF)));
			mhFile.AppendMenu(MF_STRING, IDM_FILE_OPEN,               CString(MAKEINTRESOURCE(IDM_FILE_OPEN)));
			mhFile.AppendMenu(MF_STRING, IDM_FILE_CLOSE,              CString(MAKEINTRESOURCE(IDM_FILE_CLOSE)));
			mhFile.AppendMenu(MF_STRING, IDM_FILE_CLOSE_ALL_BUT_CURRENT, CString(MAKEINTRESOURCE(IDM_FILE_CLOSE_ALL_BUT_CURRENT)));
			mhFile.AppendMenu(MF_STRING, IDM_FILE_SAVE,               CString(MAKEINTRESOURCE(IDM_FILE_SAVE)));
			mhFile.AppendMenu(MF_STRING, IDM_FILE_SAVE_AS,            CString(MAKEINTRESOURCE(IDM_FILE_SAVE_AS)));
			mhFile.AppendMenu(MF_STRING, IDM_FILE_SAVE_ALL,           CString(MAKEINTRESOURCE(IDM_FILE_SAVE_ALL)));
			mhFile.AppendMenu(MF_SEPARATOR);

			mhFile.AppendMenu(MF_STRING, IDM_FILE__EXPORT_PLAINTEXT,  CString(MAKEINTRESOURCE(IDM_FILE__EXPORT_PLAINTEXT)));
			mhFile.AppendMenu(MF_STRING, IDM_FILE__EXPORT_OLDSTYLEGRF, CString(MAKEINTRESOURCE(IDM_FILE__EXPORT_OLDSTYLEGRF)));
			mhFile.AppendMenu(MF_SEPARATOR);

			mhFile.AppendMenu(MF_STRING, ID_FILE_MRU_FILE1,           CString(MAKEINTRESOURCE(ID_FILE_MRU_FILE1)));
			mhFile.AppendMenu(MF_SEPARATOR);

			mhFile.AppendMenu(MF_STRING, IDM_FILE_EXIT,               CString(MAKEINTRESOURCE(IDM_FILE_EXIT)));
		} // END "File" menu popup
		m_mainMenu.AppendMenu(MF_POPUP, (HMENU)mhFile, CString(MAKEINTRESOURCE(IDS_MENU_FILE)));
		// FIXME: TODO: (1.8) Load menu strings + fallback if necessary
		// ...
		m_CmdBar.AttachMenu(m_mainMenu);
	}

	  // remove old menu
	this->SetMenu(NULL);

	  // load command bar images and Add Command Bar rebar
	m_CmdBar.LoadImages(IDC_GRYFF);
	m_CmdBar.AddBitmap(MAKEINTRESOURCE(IDM_FILE_CLOSE), IDM_FILE_CLOSE);
	if ( !this->CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE)
	  || !this->AddSimpleReBarBand(hWndCmdBar))
	{
		MessageBox(_T("Error when creating the rebar control (Command Bar)"), CString(MAKEINTRESOURCE(IDS_INIT_ERROR_MSGBOX_TITLE)), MB_ICONERROR | MB_OK);
		return -1;
	}

	  // Create toolbar and add it to rebar
	HWND hWndToolBar;
	if ( NULL == (hWndToolBar = this->CreateSimpleToolBarCtrl(m_hWnd, IDC_GRYFF, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE))
	  || !this->AddSimpleReBarBand(hWndToolBar, NULL, TRUE))
	{
		MessageBox(_T("Error when creating the rebar control (Toolbar)"), CString(MAKEINTRESOURCE(IDS_INIT_ERROR_MSGBOX_TITLE)), MB_ICONERROR | MB_OK);
		return -1;
	}

	  // Create a progress statusbar
	if ( NULL == (m_hWndStatusBar = m_Status.Create(m_hWnd)) )
	{
		MessageBox(_T("Error when creating the rebar control (Status Bar)"), CString(MAKEINTRESOURCE(IDS_INIT_ERROR_MSGBOX_TITLE)), MB_ICONERROR | MB_OK);
		return -1;
	}
	  // Load recently used docs
	HMENU hMenu = m_CmdBar.GetMenu();
	HMENU hFileMenu = ::GetSubMenu(hMenu, FILE_MENU_POSITION);
	m_mru.SetMenuHandle(hFileMenu);
	m_mru.ReadFromRegistry(::strGryffRegKey);
	  // FIXME: TODO: (1.2) SetMaxEntries(nMaxEntries); ATLASSERT(nMaxEntries >= m_nMaxEntries_Min && nMaxEntries <= m_nMaxEntries_Max);

	if ( NULL == (m_hWndMDIClient = this->CreateMDIClient()) )
	{
		MessageBox(_T("Error when creating the MDI client area"), CString(MAKEINTRESOURCE(IDS_INIT_ERROR_MSGBOX_TITLE)), MB_ICONERROR | MB_OK);
		return -1;
	}
	  // Currently always fails
	m_CmdBar.SetMDIClient(m_hWndMDIClient);

	  // CDocManager<T> : register the document template
	this->AddDocTemplate(m_pGrfDocTemplate);

	  // register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	  // Associate UI update
	UIAddToolBar(hWndToolBar);
	UISetBlockAccelerators(true);

	this->ProcessCommandLine_(::GetCommandLine());
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

void  CMainWindow::ProcessCommandLine_(LPCTSTR cmdLine, bool UseFirstAsCwd)
{
int     wargc;
LPWSTR  *wargv(0);
	wargv = ::CommandLineToArgvW(cmdLine, &wargc);

	if ( wargc < 2 )
	{
		  // HACK: Empty string denotes no files to open
		m_cmd_files.push(CString(_T("")));
	}
	else
	{
		if ( UseFirstAsCwd )
		{
			CString strArg;
			for ( int i = 1; i < wargc; ++i )
			{
				if ( lstrlen(wargv[i]) > 2 &&
					(wargv[i][1] == _T(':') || (wargv[i][0] == _T('\\') && wargv[i][1] == _T('\\')))
				)
				{
					// Abspath
					m_cmd_files.push(CString(wargv[i]));
				}
				else
				{
					strArg.Format(_T("%s\\%s"), wargv[0], wargv[i]);
                    m_cmd_files.push(strArg);
				}
			}
		}
		else
		{
			for ( int i = 1; i < wargc; ++i )
			{
				m_cmd_files.push(CString(wargv[i]));
			}
		}
	}
	::GlobalFree(wargv);
}

///////////////////////////////////////////////////////////////////////////////

void  CMainWindow::OnClose()
{
	if ( this->MDIGetActive() != NULL )
	{
	  // Close all mdi frames
	CCloseContext ctxt(this, 0, 0);

		::EnumChildWindows(this->m_hWndMDIClient, &EnumChildProc_ForCloseAll, reinterpret_cast<LPARAM>(&ctxt));
		if ( this->RequestClose_(ctxt.EligibleWindows) != 0 )
		{
			SetMsgHandled(FALSE);
		}
	}
	else
	{
		SetMsgHandled(FALSE);
	}
}

///////////////////////////////////////////////////////////////////////////////

/**
	Called when Main frame is destroyed.
	Not called directly.
**/
void  CMainWindow::OnDestroy()
{
	::RevokeDragDrop(m_hWnd);
//	::PostQuitMessage(0);
	SetMsgHandled(FALSE);
}

///////////////////////////////////////////////////////////////////////////////

/**
	Called when client area is painted.
	Not called directly.
**/
void  CMainWindow::OnPaint(HDC hDC)
{
	if (m_pDataObject && m_pDropTargHlp)
	{
		m_pDropTargHlp->Show(FALSE);
	}

	DefWindowProc();

	if (m_pDataObject && m_pDropTargHlp)
	{
		m_pDropTargHlp->Show(TRUE);
	}
}

///////////////////////////////////////////////////////////////////////////////
// CIdleHandler impl.
BOOL CMainWindow::OnIdle( )
{
	UIUpdateToolBar();
	  // Process queued on the command line / drag and dropped files
	if ( m_cmd_files.size() == 1 && m_cmd_files.front().IsEmpty() )
	{
		m_cmd_files.pop();
		  // It is possible that the program is launched with no arg, then another instance transmits files to open,
		  // making the queue non-empty
		if ( m_cmd_files.size() == 0 )
		{
			  // FIXME: TODO: (1.2) Check Options: start with blank document?
			if ( 1 )
			{
				if ( NewChild_().second == 0 && ::GetAsyncKeyState(VK_CONTROL) == 0 )
				{
					CString strErrMsg;
					if ( this->GetErrorCode() == -1 )
					{
						strErrMsg.Format(CString(MAKEINTRESOURCE(IDS_OPENDOC_EMPTY_FAIL)), this->GetErrorCode(), m_DocExceptionMessage);
					}
					else
					{
						strErrMsg.Format(CString(MAKEINTRESOURCE(IDS_OPENDOC_EMPTY_FAIL)), this->GetErrorCode(), CString(MAKEINTRESOURCE(CGrfDoc::GetErrorStringResID(this->GetErrorCode()))));
					}
					this->MessageBox(strErrMsg, CString(MAKEINTRESOURCE(IDS_DOC_ERROR_MSGBOX_TITLE)), MB_ICONERROR | MB_OK);
				}
			}
		}
	}

	while ( !m_cmd_files.empty() )
	{
		if ( !m_cmd_files.front().IsEmpty()
		  && NewChild_(m_cmd_files.front()).second == 0  // creation failed
		  && ::GetAsyncKeyState(VK_CONTROL) == 0 )       // and error is not skipped
		{
		CString strErrMsg;
			if ( this->GetErrorCode() == -1 )
			{
				strErrMsg.Format(CString(MAKEINTRESOURCE(IDS_OPENDOC_LOAD_FAIL)), m_cmd_files.front(), this->GetErrorCode(), m_DocExceptionMessage);
			}
			else
			{
				strErrMsg.Format(CString(MAKEINTRESOURCE(IDS_OPENDOC_LOAD_FAIL)), m_cmd_files.front(), this->GetErrorCode(), CString(MAKEINTRESOURCE(CGrfDoc::GetErrorStringResID(this->GetErrorCode()))));
			}
			this->MessageBox(strErrMsg, CString(MAKEINTRESOURCE(IDS_DOC_ERROR_MSGBOX_TITLE)), MB_ICONERROR | MB_OK);
		}
		m_cmd_files.pop();
	}

	this->SendMessage(WM_MDISETMENU);

	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT  CMainWindow::OnMDISetMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
HWND  hWndMdiFrame;
CGrfFrame *pFrame;

	// By default, uncheck all
	UISetCheck(IDM_VIEW_TYPE_TILE,    BST_UNCHECKED);
	UISetCheck(IDM_VIEW_TYPE_ICONS,   BST_UNCHECKED);
	UISetCheck(IDM_VIEW_TYPE_LIST,    BST_UNCHECKED);
	UISetCheck(IDM_VIEW_TYPE_DETAILS, BST_UNCHECKED);


	if ( ::IsWindow(m_hWndMDIClient) &&
	     (hWndMdiFrame=this->MDIGetActive()) != NULL &&
	     (pFrame=CGrfFrame::FromHandle(hWndMdiFrame)) != 0 )
	{
	CMdiEnumerator ctxt(this);
		::EnumChildWindows(this->m_hWndMDIClient, &EnumChildProc_ForSaveAll, reinterpret_cast<LPARAM>(&ctxt));

		  // Let frame update its view (own command bar, etc.)
		pFrame->UpdateUI();

		  // Locked operations?
		if ( pFrame->IsDocumentBusy() )
		{
			UIEnable(IDM_FILE_CLOSE, FALSE);
			UIEnable(IDM_FILE_SAVE, FALSE);
			UIEnable(IDM_FILE_SAVE_AS, FALSE);
			UIEnable(IDM_FILE_SAVE_ALL, ctxt.EligibleWindows.IsEmpty() ? FALSE : TRUE);

			UIEnable(IDM_EDIT_OPEN, FALSE);
			UIEnable(IDM_EDIT_RENAME, FALSE);
			UIEnable(IDM_EDIT_DELETE, FALSE);
			UIEnable(IDM_EDIT_NEW_FOLDER, FALSE);
			UIEnable(IDM_EDIT_ADD_FILES, FALSE);
			UIEnable(IDM_EDIT_ADD_DIRECTORY, FALSE);
			UIEnable(IDM_EDIT_ADD_FROM_LIST, FALSE);
			UIEnable(IDM_EDIT_MERGE_WITH_GRF, FALSE);
		}
		else
		{
			UIEnable(IDM_FILE_CLOSE, TRUE);
			// Has document been modified?
			UIEnable(IDM_FILE_SAVE, pFrame->IsModified());
			UIEnable(IDM_FILE_SAVE_AS, TRUE);
			UIEnable(IDM_FILE_SAVE_ALL, ctxt.EligibleWindows.IsEmpty() ? FALSE : TRUE);

			UIEnable(IDM_EDIT_OPEN, pFrame->CanPerformOpen());
			UIEnable(IDM_EDIT_RENAME, pFrame->CanPerformRename());
			UIEnable(IDM_EDIT_DELETE, pFrame->CanPerformDelete());
			UIEnable(IDM_EDIT_NEW_FOLDER, TRUE);
			UIEnable(IDM_EDIT_ADD_FILES, TRUE);
			UIEnable(IDM_EDIT_ADD_DIRECTORY, TRUE);
			UIEnable(IDM_EDIT_ADD_FROM_LIST, TRUE);
			UIEnable(IDM_EDIT_MERGE_WITH_GRF, TRUE);
		}

		UIEnable(IDM_FILE_CLOSE_ALL_BUT_CURRENT, TRUE);
		UIEnable(IDM_WINDOW_CLOSE_ALL_BUT_CURRENT, TRUE);

		UIEnable(IDM_VIEW_GRF_EXPLORER_PANE, TRUE);
		UISetCheck(IDM_VIEW_GRF_EXPLORER_PANE, (pFrame->IsSinglePane())? BST_UNCHECKED : BST_CHECKED);
		UIEnable(IDM_VIEW_TYPE_TILE, TRUE);  // Disable when applicable
		UIEnable(IDM_VIEW_TYPE_ICONS, TRUE);
		UIEnable(IDM_VIEW_TYPE_LIST, TRUE);
		UIEnable(IDM_VIEW_TYPE_DETAILS, TRUE);
		UISetCheck( pFrame->GetViewTypeId(), BST_CHECKED);
		UIEnable(IDM_VIEW_REFRESH, TRUE);

		UIEnable(IDM_WINDOW_CASCADE, TRUE);
		UIEnable(IDM_WINDOW_TILE_HORZ, TRUE);
		UIEnable(IDM_WINDOW_ARRANGE, TRUE);
	}
	else
	{
		// No active frame
		UIEnable(IDM_FILE_NEW, TRUE);
		UIEnable(IDM_FILE_NEW_FROM_GRF, TRUE);
		UIEnable(IDM_FILE_OPEN, TRUE);
		UIEnable(IDM_FILE_CLOSE, FALSE);
		UIEnable(IDM_FILE_CLOSE_ALL_BUT_CURRENT, FALSE);
		UIEnable(IDM_FILE_SAVE, FALSE);
		UIEnable(IDM_FILE_SAVE_AS, FALSE);
		UIEnable(IDM_FILE_SAVE_ALL, FALSE);

		UIEnable(IDM_EDIT_OPEN, FALSE);
		UIEnable(IDM_EDIT_RENAME, FALSE);
		UIEnable(IDM_EDIT_DELETE, FALSE);
		UIEnable(IDM_EDIT_NEW_FOLDER, FALSE);
		UIEnable(IDM_EDIT_ADD_FILES, FALSE);
		UIEnable(IDM_EDIT_ADD_DIRECTORY, FALSE);
		UIEnable(IDM_EDIT_ADD_FROM_LIST, FALSE);
		UIEnable(IDM_EDIT_MERGE_WITH_GRF, FALSE);

		UIEnable(IDM_VIEW_GRF_EXPLORER_PANE, FALSE);
		UIEnable(IDM_VIEW_TYPE_LIST, FALSE);
		UIEnable(IDM_VIEW_TYPE_DETAILS, FALSE);
		UIEnable(IDM_VIEW_REFRESH, FALSE);

		UIEnable(IDM_WINDOW_CLOSE_ALL_BUT_CURRENT, FALSE);
		UIEnable(IDM_WINDOW_CASCADE, FALSE);
		UIEnable(IDM_WINDOW_TILE_HORZ, FALSE);
		UIEnable(IDM_WINDOW_ARRANGE, FALSE);
	}
	bHandled = FALSE;
	return 1;
}

///////////////////////////////////////////////////////////////////////////////
// CMainWindow::UWM_ARE_YOU_ME registered message handler
// Responds to a request "Are you the single instance?"

LRESULT CMainWindow::OnAreYouMe(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return UWM_ARE_YOU_ME;
}

///////////////////////////////////////////////////////////////////////////////
// CMainWindow::UWM_PROGRESS registered message handler
// Updates the UI

LRESULT CMainWindow::OnProgress(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	  // FIXME: TODO: (x.x) CMainWindow::OnProgress
#if 0
int *pState = reinterpret_cast<int*>(wParam);
	m_sb.OnProgress(pState);
	if ( *pState == 0 )
	{
		const int cchMax = 128;   // max text length is 127 for status bars (+1 for null)
		TCHAR szText[cchMax];
		szText[0] = 0;
#if (_ATL_VER >= 0x0700)
		::LoadString(ATL::_AtlBaseModule.GetResourceInstance(), ATL_IDS_IDLEMESSAGE, szText, cchMax);
#else //!(_ATL_VER >= 0x0700)
		::LoadString(_Module.GetResourceInstance(), ATL_IDS_IDLEMESSAGE, szText, cchMax);
#endif //!(_ATL_VER >= 0x0700)

		m_sb.SetWindowText(szText);
	}
#endif
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// CMainWindow::UWM_MDIDESTROY registered message handler
// Last view of a document is destroyed

LRESULT CMainWindow::OnChildDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
CGrfDoc *pDoc = reinterpret_cast<CGrfDoc *>(lParam);
	ATLASSERT( pDoc );
	ATLASSERT( pDoc->GetNumViews() == 0 );
CGrfDocTemplate *pDocTemplate = dynamic_cast<CGrfDocTemplate *>(pDoc->GetDocTemplate());
	ATLASSERT(pDocTemplate);
	pDocTemplate->RemoveDocument(pDoc);
	delete pDoc;
	this->SendMessage(WM_MDISETMENU);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// CMainWindow::UWM_REGISTER_MRU registered message handler
LRESULT  CMainWindow::OnRegisterMru(UINT, WPARAM, LPARAM lParam, BOOL&)
{
	LPCTSTR  pPath = reinterpret_cast<LPCTSTR>(lParam);
	if ( pPath )
	{
		m_mru.AddToList(pPath);
		m_mru.WriteToRegistry(::strGryffRegKey);
	}
	return 0;
}


///////////////////////////////////////////////////////////////////////////////
// CMainWindow::UWM_MDIFINALMESSAGE registered message handler
LRESULT  CMainWindow::OnChildFinalize(UINT, WPARAM, LPARAM lParam, BOOL&)
{
	delete reinterpret_cast<CGrfFrame *>(lParam);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// CMainWindow::UWM_GETDOCPTR registered message handler
LRESULT  CMainWindow::OnGetDocumentPtr(UINT, WPARAM, LPARAM lParam, BOOL&)
{
	CDocTemplateBase *pDocTemplateBase = this->GetDocTemplate(0);
	int  iDocument = pDocTemplateBase->GetDocumentIndex(reinterpret_cast<LPCTSTR>(lParam));
	if ( iDocument != -1 )
	{
		return reinterpret_cast<LRESULT>(dynamic_cast<CGrfDoc *>(dynamic_cast<CDocument<CGrfDoc> *>(pDocTemplateBase->GetDocument(iDocument))));
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// CMainWindow::UWM_STATUS_TEXT registered message handler
LRESULT  CMainWindow::OnStatusText(UINT, WPARAM, LPARAM lParam, BOOL&)
{
	this->m_Status.SetWindowText(reinterpret_cast<LPCTSTR>(lParam));
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// IErrorCode<int> impl.
void CMainWindow::SetErrorCode(const int &nCode)
{
	m_nErrorCode = nCode;
}

///////////////////////////////////////////////////////////////////////////////
// IErrorCode<int> impl.
const int &CMainWindow::GetErrorCode() const
{
	return m_nErrorCode;
}

LRESULT  CMainWindow::OnCopyData(HWND hWnd, PCOPYDATASTRUCT pcds)
{
	if ( pcds )
	{
		TCHAR *text = reinterpret_cast<TCHAR *>(pcds->lpData);
		if ( text && *text != _T('\0') )
		{
			if ( *text != _T('*') )
			{
				CString str;
				str.Format(_T("gryff %s"), text);
				this->ProcessCommandLine_(str);
			}
			else
			{
				this->ProcessCommandLine_(text+1, true);
			}
		}
		::FlashWindow(m_hWnd, FALSE);
	}
	return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
// Menu Commands handlers
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////

LRESULT CMainWindow::OnMenuFileNew(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if ( NewChild_().second == 0 && ::GetAsyncKeyState(VK_CONTROL) == 0 )
	{
	CString strErrMsg;
		if ( this->GetErrorCode() == -1 )
		{
			strErrMsg.Format(CString(MAKEINTRESOURCE(IDS_OPENDOC_EMPTY_FAIL)), this->GetErrorCode(), m_DocExceptionMessage);
		}
		else
		{
			strErrMsg.Format(CString(MAKEINTRESOURCE(IDS_OPENDOC_EMPTY_FAIL)), this->GetErrorCode(), CString(MAKEINTRESOURCE(CGrfDoc::GetErrorStringResID(this->GetErrorCode()))));
		}
		this->MessageBox(strErrMsg, CString(MAKEINTRESOURCE(IDS_DOC_ERROR_MSGBOX_TITLE)), MB_ICONERROR | MB_OK);
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT CMainWindow::OnMenuFileNewFromGrf(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	return this->OnMenuFileOpen(wNotifyCode, wID, hWndCtl, bHandled);
}

///////////////////////////////////////////////////////////////////////////////

LRESULT CMainWindow::OnMenuFileOpen(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
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
		if ( NewChild_(cf_dlg.m_szFileName, wID == IDM_FILE_NEW_FROM_GRF).second == 0 )
		{
			if ( ::GetAsyncKeyState(VK_CONTROL) == 0 )
			{
			CString strErrMsg;
				if ( this->GetErrorCode() == -1 )
				{
					strErrMsg.Format(CString(MAKEINTRESOURCE(IDS_OPENDOC_LOAD_FAIL)), cf_dlg.m_szFileName, this->GetErrorCode(), m_DocExceptionMessage);
				}
				else
				{
					strErrMsg.Format(CString(MAKEINTRESOURCE(IDS_OPENDOC_LOAD_FAIL)), cf_dlg.m_szFileName, this->GetErrorCode(), CString(MAKEINTRESOURCE(CGrfDoc::GetErrorStringResID(this->GetErrorCode()))));
				}
				this->MessageBox(strErrMsg, CString(MAKEINTRESOURCE(IDS_DOC_ERROR_MSGBOX_TITLE)), MB_ICONERROR | MB_OK);
			}
		}
		else if ( wID != IDM_FILE_NEW_FROM_GRF )
		{
			m_mru.AddToList(cf_dlg.m_szFileName);
			m_mru.WriteToRegistry(::strGryffRegKey);
		}
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT CMainWindow::OnMenuFileCloseAllButCurrent(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled)
{
HWND hWndActive = this->MDIGetActive();

	if ( NULL == hWndActive )
	{
		return 0;
	}
	CCloseContext ctxt(this, wID, CGrfFrame::FromHandle(hWndActive));
	ATLASSERT(ctxt.pFrame != 0);

	::EnumChildWindows(this->m_hWndMDIClient, &EnumChildProc_ForCloseAll, reinterpret_cast<LPARAM>(&ctxt));
	return this->RequestClose_(ctxt.EligibleWindows);
}

///////////////////////////////////////////////////////////////////////////////

LRESULT CMainWindow::RequestClose_(const CAtlList<HWND> &Windows)
{
int nResult = IDYES;
	for ( POSITION pos = Windows.GetHeadPosition();
	      pos; )
	{
		if ( IDCANCEL == this->CheckCloseChild_(Windows.GetNext(pos)) )
		{
			  // Abort prematurely
			return 0;
		}
	}

	for ( POSITION pos = Windows.GetHeadPosition();
	      pos; )
	{
		// "Cookie" specifying that no prompt needed since already done above
		::SendMessage(Windows.GetNext(pos), WM_CLOSE, 1, 0x1313);
	}
	return 1;
}

///////////////////////////////////////////////////////////////////////////////

INT_PTR CMainWindow::CheckCloseChild_(HWND hWnd)
{
	INT_PTR nRet = IDYES;
	CGrfFrame *pChild = CGrfFrame::FromHandle(hWnd);
	ATLASSERT(pChild != 0);
	if ( pChild->IsModified() != FALSE )
	{
		CString strPrompt;
		strPrompt.Format(CString(MAKEINTRESOURCE(IDS_MODIFIED_SAVE_PROMPT)), pChild->GetName());
		if ( IDCANCEL == (nRet=::MessageBoxEx(this->m_hWnd, strPrompt, CString(MAKEINTRESOURCE(IDS_APP_TITLE)), MB_ICONEXCLAMATION | MB_YESNOCANCEL, LANGIDFROMLCID(::GetThreadLocale())))
		  || (nRet==IDYES && pChild->OnSave(false) == IDCANCEL) ) // User may cancel file save at MB and at OFN
		{
			  // Abort close
			return IDCANCEL;
		}
	}
	return nRet;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT CMainWindow::OnMenuFileSaveAll(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
CMdiEnumerator ctxt(this);
	::EnumChildWindows(this->m_hWndMDIClient, &EnumChildProc_ForSaveAll, reinterpret_cast<LPARAM>(&ctxt));
	for ( POSITION pos = ctxt.EligibleWindows.GetHeadPosition();
	      pos; )
	{
		CGrfFrame  *pChild(CGrfFrame::FromHandle(ctxt.EligibleWindows.GetNext(pos)));
		ATLASSERT(pChild);
		// Save multiple children synchronously
		if ( IDCANCEL == pChild->OnSave(false) )
		{
			  // Abort prematurely
			return 0;
		}
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT CMainWindow::OnMenuFileRecent(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// get file name from the MRU list
TCHAR  szFileName[MAX_PATH];
	if ( m_mru.GetFromList(wID, szFileName) )
	{
		if ( NewChild_(szFileName, false).second == 0 )
		{
			if ( ::GetAsyncKeyState(VK_CONTROL) == 0 )
			{
			CString strErrMsg;
				if ( this->GetErrorCode() == -1 )
				{
					strErrMsg.Format(CString(MAKEINTRESOURCE(IDS_OPENDOC_LOAD_FAIL)), szFileName, this->GetErrorCode(), m_DocExceptionMessage);
				}
				else
				{
					strErrMsg.Format(CString(MAKEINTRESOURCE(IDS_OPENDOC_LOAD_FAIL)), szFileName, this->GetErrorCode(), CString(MAKEINTRESOURCE(CGrfDoc::GetErrorStringResID(this->GetErrorCode()))));
				}
				this->MessageBox(strErrMsg, CString(MAKEINTRESOURCE(IDS_DOC_ERROR_MSGBOX_TITLE)), MB_ICONERROR | MB_OK);
			}
			m_mru.RemoveFromList(wID);
		}
		else
		{
			m_mru.MoveToTop(wID);
		}
		m_mru.WriteToRegistry(::strGryffRegKey);
	}
	else
	{
		::MessageBeep(MB_ICONERROR);
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT CMainWindow::OnMenuFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	this->PostMessage(WM_CLOSE);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT  CMainWindow::OnMenuHelpOnlineTutorial(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	//::ShellExecute(m_hWnd, _T("open"), _T("http://gryff.ragnarok-online.info/?act=gryff&amp;section=tutorial"), NULL, NULL, SW_SHOWNORMAL);
	::MessageBeep(MB_ICONERROR);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT CMainWindow::OnMenuHelpAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
CAboutDialog  dlg;
HINSTANCE hInstRich = ::LoadLibrary(CRichEditCtrl::GetLibraryName());
INT_PTR nRet = dlg.DoModal();
	::FreeLibrary(hInstRich);
	return static_cast<LRESULT>(nRet);
}

///////////////////////////////////////////////////////////////////////////////
/**
	Creates and registers a document model/controller/view.
	Returns a pointer to the child window frame.
	If the document is already opened and only one view per document is allowed,
	a pointer to the existing frame is returned.
		. lpFilePath : Path to the Grf document to be opened. If 0, a blank document is created.
		. bOpenAsUntitled : If true, the document is opened as if imported into a blank document.
		Ignored when lpFilePath is 0.
**/
pair<CGrfFrame *, CGrfDoc *>  CMainWindow::NewChild_(LPCTSTR lpFilePath, bool bOpenAsUntitled)
{
CGrfFrame *pFrame(0);
CGrfDoc *pDocument(0);
CDocTemplateBase *pDocTemplateBase = GetDocTemplate(0);
int nDocs = pDocTemplateBase->GetNumDocs();

	  // If already open and not requesting Untitled, create a new view of the document
	if ( !bOpenAsUntitled && lpFilePath != 0 )
	{
		int iDocument = pDocTemplateBase->GetDocumentIndex(lpFilePath);
		if ( iDocument != -1 )
		{

			pDocument = dynamic_cast<CGrfDoc *>(dynamic_cast<CDocument<CGrfDoc> *>(pDocTemplateBase->GetDocument(iDocument)));
		CMatchDocContext ctxt(this, pDocument);
			::EnumChildWindows(this->m_hWndMDIClient, &EnumChildProc_ForSpecDoc, reinterpret_cast<LPARAM>(&ctxt));
			if ( !ctxt.EligibleWindows.IsEmpty() )
			{
				pFrame = CGrfFrame::FromHandle(ctxt.EligibleWindows.GetHead());
			}
			  // FIXME: TODO: (1.5) Create an additional view
			this->MDIActivate(pFrame->operator HWND());
			this->SendMessage(WM_MDISETMENU);
			return  pair<CGrfFrame *, CGrfDoc *>(pFrame, pDocument);
		}
		// Not opened, continue processing...
	}

	try
	{
	CWaitCursor w(true, IDC_APPSTARTING, true);

	pair<HWND, CDocumentBase*> p( pDocTemplateBase->OpenDocumentFile(lpFilePath, bOpenAsUntitled) );
		if ( p.first == 0 ||
		  p.second == 0 ||
		  (pFrame = CGrfFrame::FromHandle(p.first)) == 0 ||
		  (pDocument = dynamic_cast<CGrfDoc *>(dynamic_cast<CDocument<CGrfDoc> *>(p.second))) == 0 )
		{
			this->SetErrorCode(0);
			::MessageBeep((UINT)-1);
			return  pair<CGrfFrame *, CGrfDoc *>(0,0);
		}
	}
	catch ( const Gen::GrfImportError &gie )
	{
		this->SetErrorCode(-1);
		::MessageBeep((UINT)-1);
		m_DocExceptionMessage = gie.message;
		return  pair<CGrfFrame *, CGrfDoc *>(0,0);
	}
	this->SendMessage(WM_MDISETMENU);
	return  pair<CGrfFrame *, CGrfDoc *>(pFrame, pDocument);
}

///////////////////////////////////////////////////////////////////////////////

/**
	IUnknown QueryInterface()
	Supported interfaces are IUnknown + IDropTarget
**/
HRESULT STDMETHODCALLTYPE  CMainWindow::QueryInterface(REFIID iid, LPVOID *ppvObject)
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
ULONG STDMETHODCALLTYPE  CMainWindow::AddRef()
{
	return  static_cast<ULONG>(::InterlockedIncrement(&m_RefCount));
}

///////////////////////////////////////////////////////////////////////////////

/**
	IUnknown Release()
	Decrease reference count
**/
ULONG STDMETHODCALLTYPE  CMainWindow::Release()
{
	return  static_cast<ULONG>(::InterlockedDecrement(&m_RefCount));
}

///////////////////////////////////////////////////////////////////////////////

/**
	IDropTarget DragEnter()
**/
HRESULT STDMETHODCALLTYPE  CMainWindow::DragEnter(IDataObject *pDataObject, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
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
		*pdwEffect = DROPEFFECT_LINK;
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
HRESULT STDMETHODCALLTYPE  CMainWindow::DragOver(DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
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
		*pdwEffect = DROPEFFECT_LINK;
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
HRESULT STDMETHODCALLTYPE  CMainWindow::DragLeave()
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
HRESULT STDMETHODCALLTYPE  CMainWindow::Drop(IDataObject *pDataObject, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
	if ( !m_pDropTargHlp )
	{
		return E_FAIL;
	}
	if ( m_pDropTargHlp )
	{
	POINT pt = { ptl.x, ptl.y };
		m_pDropTargHlp->Drop(pDataObject, &pt, *pdwEffect);
		//m_pDataObject->Release();//no need to with smart ptr
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
	bool  shallSetForeground(false);
	HDROP  hDrop(reinterpret_cast<HDROP>(medium.hGlobal));
    TCHAR  szFileName[MAX_PATH];
		for ( int i = 0; ::DragQueryFile(hDrop, i, szFileName, MAX_PATH) > 0; ++i )
		{
			if ( NewChild_(szFileName, false).second == 0 )
			{
				if ( ::GetAsyncKeyState(VK_CONTROL) == 0 )
				{
				CString strErrMsg;
					if ( this->GetErrorCode() == -1 )
					{
						strErrMsg.Format(CString(MAKEINTRESOURCE(IDS_OPENDOC_LOAD_FAIL)), szFileName, this->GetErrorCode(), m_DocExceptionMessage);
					}
					else
					{
						strErrMsg.Format(CString(MAKEINTRESOURCE(IDS_OPENDOC_LOAD_FAIL)), szFileName, this->GetErrorCode(), CString(MAKEINTRESOURCE(CGrfDoc::GetErrorStringResID(this->GetErrorCode()))));
					}
					this->MessageBox(strErrMsg, CString(MAKEINTRESOURCE(IDS_DOC_ERROR_MSGBOX_TITLE)), MB_ICONERROR | MB_OK);
				}
			}
			else
			{
				m_mru.AddToList(szFileName);
				m_mru.WriteToRegistry(::strGryffRegKey);
				shallSetForeground = true;
			}
		}

		if ( medium.pUnkForRelease )
		{
			::ReleaseStgMedium(&medium);
		}
		else
		{
			::GlobalFree(medium.hGlobal);
		}

		if ( shallSetForeground )
		{
			// Bring to front after drop
			if ( ::AllowSetForegroundWindow(::GetCurrentProcessId()) )
			{
				::SetForegroundWindow(this->GetMDIFrame());
				this->SetFocus();
			}
		}
	}
	else
	{
		func_hr = E_UNEXPECTED;
	}

	*pdwEffect = DROPEFFECT_NONE;
	return func_hr;
}

///////////////////////////////////////////////////////////////////////////////

/**
	Returns whether a supported clipboard format is detected when
	dragging an object over the window
**/
bool  CMainWindow::IsAcceptedFormat_(IDataObject *pDataObject)
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

LRESULT  CMainWindow::OnMenuSelect(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	LRESULT  baseMethodResult = baseFrameClass_::OnMenuSelect(uMsg, wParam, lParam, bHandled);

	// menu is showing
	if ( HIWORD(wParam) != 0xFFFF && !(HIWORD(wParam) & MF_POPUP) )
	{
	const int cchBuff = 256;
	TCHAR szBuff[cchBuff];
		szBuff[0] = 0;
	WORD wID = LOWORD(wParam);
		// check for special cases
		if(wID >= 0xF000 && wID < 0xF1F0)                              // system menu IDs
			wID = (WORD)(((wID - 0xF000) >> 4) + ATL_IDS_SCFIRST);
		else if(wID >= ID_FILE_MRU_FIRST && wID <= ID_FILE_MRU_LAST)   // MRU items
			wID = ATL_IDS_MRU_FILE;
		else if(wID >= ATL_IDM_FIRST_MDICHILD)                         // MDI child windows
			wID = ATL_IDS_MDICHILD;

		wID = wID + IDM_IDS_OFFSET;

#if (_ATL_VER >= 0x0700)
		int nRet = ::LoadString(ATL::_AtlBaseModule.GetResourceInstance(), wID, szBuff, cchBuff);
#else //!(_ATL_VER >= 0x0700)
		int nRet = ::LoadString(_Module.GetResourceInstance(), wID, szBuff, cchBuff);
#endif //!(_ATL_VER >= 0x0700)
		if ( nRet > 0 )
		{
			::SendMessage(m_hWndStatusBar, SB_SETTEXT, (255 | SBT_NOBORDERS), (LPARAM)szBuff);
		}
	}

	// Prevent the base handler being called twice, and call
	// DefWindowProc if necessary when our own job is done
	if ( !bHandled )
	{
		baseMethodResult = this->DefWindowProc(uMsg, wParam, lParam);
	}
	bHandled = TRUE;
	return baseMethodResult;
}
