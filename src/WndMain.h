// $Id: WndMain.h 12 2005-08-14 01:42:51Z Rasqual $

// *
// *   gryff, a GRF archive management utility, src/WndMain.h
// *   Copyright (C) 2003, 2004 Rasqual Twilight
// *
// *   This program is distributed in the hope that it will be useful,
// *   but WITHOUT ANY WARRANTY; without even the implied warranty of
// *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// *

// * WndMain.h
// * Main frame class definition
// *
// *@author $Author: Rasqual $
// *@version $Revision: 12 $
// *
// * See Also
// Multiple Document Types, Views, and Frame Windows (Microsoft Corp)
// http://msdn.microsoft.com/library/en-us/vccore98/HTML/_core_multiple_document_types.2c_.views.2c_.and_frame_windows.asp
// Draft for a Doc/View approach (Gabriel Kriznik)
// http://www.argsoft.com/Wtl/DocView.html
// MVC (or Doc-View) for WTL (Paul Selormey)
// http://groups.yahoo.com/group/wtl/message/9945
// *

#ifndef __WNDMAIN_H__
#define __WNDMAIN_H__

#pragma once

#include  <IErrorCode.h>
#include  <queue>

#include  <MgrDoc.h>
#include  "ProcComm.h"

#include  "ProgBar.h"

///////////////////////////////////////////////////////////////////////////////
// Classes in this file:
//
// CRecentDocumentListEmbed
//   Handles recent docs, embedding into a menu (not a submenu)
//
// CMainWindow
//   Class instantiating a window with a MDI client area.
//   See  :  CMDIFrameWindowImpl <atlframe.h>

#define  UWM_ARE_YOU_ME_MSG  _T("GRYFF_UWM_ARE_YOU_ME-AAA")
extern UINT  UWM_ARE_YOU_ME;
#define  UCF_GRYFF_ENTRIES_FMT _T("Gryff Entries ClipFormat")
extern UINT  UCF_GRYFF_ENTRIES;
#define  UCF_GRYFF_ENTRIES_INTERNAL_FMT _T("Gryff Entries ClipFormat--Internal")
extern UINT  UCF_GRYFF_ENTRIES_INTERNAL;

// MVC classes
class CGrfDoc;
class CGrfView;
class CGrfFrame;
template <typename TDoc, typename TView, typename TFrame, int nID> class CDocTemplate;
typedef CDocTemplate<CGrfDoc, CGrfView, CGrfFrame, IDC_GRYFFCHILD> CGrfDocTemplate;

typedef  ATL::CWinTraits<WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, WS_EX_APPWINDOW | WS_EX_WINDOWEDGE /*| WS_EX_ACCEPTFILES*/>		CDnDFrameWinTraits;

///////////////////////////////////////////////////////////////////////////////

class CRecentDocumentListEmbed : public CRecentDocumentListBase<CRecentDocumentListEmbed, MAX_PATH, ID_FILE_MRU_FILE1, ID_FILE_MRU_FILE16>
{
public:
// nothing here
};


///////////////////////////////////////////////////////////////////////////////

class CMainWindow : public CMDIFrameWindowImpl<CMainWindow, CMDIWindow, CDnDFrameWinTraits>,
                    public CUpdateUI<CMainWindow>,
                    public CIdleHandler,
                    public CMessageFilter,
                    public IErrorCode<int>,
                    public CDocManager<CMainWindow>,
                    public IDropTarget
{
private:
	typedef CMDIFrameWindowImpl<CMainWindow, CMDIWindow, CDnDFrameWinTraits> baseFrameClass_;

	///////////////////////// - Static - /////////////////////////////
public:
	DECLARE_FRAME_WND_CLASS(_T("Gryff-MainWindowClass"), IDC_GRYFF);

	/**
		Returns whether a supported clipboard format is detected when
		dragging an object over the window
	**/
	static bool  IsAcceptedFormat_(IDataObject *);

	///////////////////////// - CTOR / DTOR - /////////////////////////////
public:
	/**
		Registers user window messages.
	**/
	CMainWindow();
	/**
		Destroys document templates.
	**/
	~CMainWindow();

	///////////////////////// - data members - /////////////////////////////
protected:
	int                  m_nErrorCode;  // used when IErrorCode<T> members are used
	CString              m_DocExceptionMessage;
	CMDICommandBarCtrl   m_CmdBar;
	CRecentDocumentListEmbed  m_mru;
	CProgStatusBar       m_Status;
	CGrfDocTemplate      *m_pGrfDocTemplate;
	std::queue<CString>  m_cmd_files;   // queued to-be-opened file names
	bool                 m_Busy;

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

public:
	//----- Implement abstract class IErrorCode<T> with T=int -----//
	/**
		Implements IErrorCode<T>::SetErrorCode().
	**/
	void       SetErrorCode(const int &nCode);
	/**
		Implements IErrorCode<T>::GetErrorCode().
	**/
	const int  &GetErrorCode() const;

protected:
	//----- Window message loop related handling -----//
	/**
		Uses built-in pre-translation, and if not handled, try with active MDI child.
		Returns TRUE when translated, FALSE otherwise.
		Not called directly.
	**/
	virtual BOOL  PreTranslateMessage(LPMSG);

	/**
		Called when a new main frame is being created.
		Creates the child controls and MDI framework.
		On failure (non-0 return value), Create()/CreateEx() returns NULL to its caller.
		Not called directly.
	**/
	LRESULT  OnCreate(LPCREATESTRUCT);

	/**
		Called upon reception of a top-level-frame close request (WM_CLOSE).
		User is prompted if there are open unsaved documents.
		User may choose to cancel closing by clicking cancel,
		either on the MessageBox, or on the OpenFileSave dialogs.
		In all other cases, destroy all child frames.
		Not called directly generally.
		See: RequestClose_(), OnDestroy()
	**/
	void  OnClose();

	/**
		Called when Main frame is destroyed.
		Not called directly.
	**/
	void  OnDestroy();

	//void  OnNonClientDestroy() { ::OutputDebugStringA("WndMain WM_NCDESTROY\n"); }

	/**
		Called when client area is painted.
		Not called directly.
	**/
	void  OnPaint(HDC);

	////----- Implement CIdleHandler -----////
	/**
		Idle handler. Updates UI and opens queued documents.
		See: ProcessCommandLine_()
	**/
	BOOL  OnIdle();

	/**
	**/
	LRESULT  OnMDISetMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	////----- registered message handlers -----////
	/**
		UWM_ARE_YOU_ME registered message handler
		Responds to a request "Are you the single instance?"
		Simply returns UWM_ARE_YOU_ME, since the default window proc
		returns 0 on unprocessed messages, that user messages are.
	**/
	LRESULT  OnAreYouMe(UINT, WPARAM, LPARAM, BOOL&);

	/**
		UWM_PROGRESS registered message handler
		Updates the status bar when opening a document,
		as this operation may seem to freeze the UI otherwise.
	**/
	LRESULT  OnProgress(UINT, WPARAM, LPARAM, BOOL&);

//	LRESULT  OnMenuSelect(UINT, WPARAM, LPARAM, BOOL&);


	/**
		UWM_MDIDESTROY registered message handler
		Message is sent when the last view of a document is destroyed.
	**/
	LRESULT  OnChildDestroy(UINT, WPARAM, LPARAM, BOOL&);

	/**
		UWM_REGISTER_MRU registered message handler
		Adds the specified LPCTSTR document path to the Most Recently Used
		list, passed as lParam.
	**/
	LRESULT  OnRegisterMru(UINT, WPARAM, LPARAM, BOOL&);

	/**
		UWM_MDIFINALMESSAGE registered message handler
		Message is sent when the a MDI child frame is destroyed
	**/
	LRESULT  OnChildFinalize(UINT, WPARAM, LPARAM lParam, BOOL&);

	/**
		UWM_GETDOCPTR registered message handler
		Message is sent to retrieve a CGrfDoc object address from within this process
	**/
	LRESULT  OnGetDocumentPtr(UINT, WPARAM, LPARAM lParam, BOOL&);

	/**
		UWM_STATUS_TEXT registered message handler
		Sets status bar length
	**/
	LRESULT  OnStatusText(UINT, WPARAM, LPARAM lParam, BOOL&);

	////----- window message handlers -----////
	/**
		WM_COPYDATA message handler
		Message is sent when another instance passes data such as
		a command line.
		See: ::_tWinMain()
	**/
	LRESULT  OnCopyData(HWND, PCOPYDATASTRUCT);

	////----- command handlers -----////
	/**
		Creates a new document and adds it to the document manager.
		See: NewChild_()
	**/
	LRESULT  OnMenuFileNew(WORD, WORD, HWND, BOOL&);
	/**
		Creates a new document based on an existing one, while marking it as untitled *and* modified.
		Adds it to the document manager.
		See: OnMenuFileOpen()
	**/
	LRESULT  OnMenuFileNewFromGrf(WORD, WORD, HWND, BOOL&);
	/**
		Prompts user for an existing document, opens it and adds it to the document manager.
		See: NewChild_()
	**/
	LRESULT  OnMenuFileOpen(WORD, WORD, HWND, BOOL&);

	/**
		Handles both IDM_FILE_CLOSE_ALL_BUT_CURRENT (close all child frames but the currently active one)
		and IDM_WINDOW_CLOSE_ALL_BUT_CURRENT (closes all documents except the ones corresponding to
		the current working document) commands.
		See: RequestClose_(), ::EnumChildProc_ForCloseAll()
	**/
	LRESULT  OnMenuFileCloseAllButCurrent(WORD, WORD, HWND, BOOL&);

	/**
		Saves all modified documents in the current environment.
	**/
	LRESULT  OnMenuFileSaveAll(WORD, WORD, HWND, BOOL&);

	/**
		Opens a recently opened entry.
	**/
	LRESULT  OnMenuFileRecent(WORD, WORD, HWND, BOOL&);

	/**
		Quits the application.
		Whether the application relly quits is defined by the WM_CLOSE handler.
	**/
	LRESULT  OnMenuFileExit(WORD, WORD, HWND, BOOL&);

	/**
		Launches the program tutorial in a browser window.
	**/
	LRESULT  OnMenuHelpOnlineTutorial(WORD, WORD, HWND, BOOL&);

	/**
		Shows the application about box.
	**/
	LRESULT  OnMenuHelpAbout(WORD, WORD, HWND, BOOL&);


	//----- methods: Others -----//
	/**
		Processes the specified command line, queuing paths of documents to open.
		If UseFirstAsCwd is true, the first argument (in the command-line meaning)
		is considered to be the current working directory. Any subsequent argument
		that is not an absolute path, will be prepended by this path.
	**/
	void  ProcessCommandLine_(LPCTSTR cmdLine, bool UseFirstAsCwd = false);

	/**
		Creates and registers a document model/controller/view.
		Returns a pointer to the child window frame.
		If the document is already opened and only one view per document is allowed,
		a pointer to the existing frame is returned.
		 . lpFilePath : Path to the Grf document to be opened. If 0, a blank document is created.
		 . bOpenAsUntitled : If true, the document is opened as if imported into a blank document.
		 	Ignored when lpFilePath is 0.
	**/
	std::pair<CGrfFrame *, CGrfDoc *>  NewChild_(LPCTSTR lpFilePath = 0, bool bOpenAsUntitled = false);

	/**
		Returns 0 if the child could not be closed,
		non-0 otherwise.
		See: CheckCloseChild_()
	**/
	LRESULT  RequestClose_(const CAtlList<HWND> &Windows);

	/**
		Checks whether a child frame may be closed, prompting the user for save if necessary.
		Returns IDOK if the child can be closed, or IDCANCEL if the user cancelled the operation.
		 . hWnd : Window handle of a CGrfFrame child frame.
	**/
	INT_PTR  CheckCloseChild_(HWND hWnd);

	/**
		Overrides base class WM_MENUSELECT handler, setting the status text to something else.
	**/
	LRESULT  OnMenuSelect(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);



	///////////////////////// - Message loop - /////////////////////////////

	BEGIN_MSG_MAP(CMainWindow)
	  MSG_WM_CREATE(OnCreate)
	  MSG_WM_CLOSE(OnClose)
	  MSG_WM_DESTROY(OnDestroy)
	  //MSG_WM_NCDESTROY(OnNonClientDestroy)
	  MSG_WM_PAINT(OnPaint)

	  MESSAGE_HANDLER(UWM_ARE_YOU_ME, OnAreYouMe)
	  MESSAGE_HANDLER(UWM_PROGRESS, OnProgress)
      MESSAGE_HANDLER(UWM_MDIDESTROY, OnChildDestroy)
      MESSAGE_HANDLER(UWM_REGISTER_MRU, OnRegisterMru)
      MESSAGE_HANDLER(UWM_MDIFINALMESSAGE, OnChildFinalize)
      MESSAGE_HANDLER(UWM_GETDOCPTR, OnGetDocumentPtr)
	  MESSAGE_HANDLER(UWM_STATUS_TEXT, OnStatusText)
	  // Lets base class process WM_MENUSELECT then do our own job
	  MESSAGE_HANDLER(WM_MENUSELECT, OnMenuSelect)

	  MSG_WM_COPYDATA(OnCopyData)

		if(m_bBlockAccelerators && HIWORD(wParam) == 1)   // accelerators only
		{
			int nID = LOWORD(wParam);
			if((UIGetState(nID) & UPDUI_DISABLED) == UPDUI_DISABLED)
			{
				ATLTRACE2(atlTraceUI, 0, _T("CMainWindow::(Message Map) - blocked disabled command 0x%4.4X\n"), nID);
				bHandled = TRUE;   // eat the command, UI item is disabled
				return TRUE;
			}
		}


	  COMMAND_ID_HANDLER(IDM_FILE_NEW, OnMenuFileNew)
	  COMMAND_ID_HANDLER(IDM_FILE_NEW_FROM_GRF, OnMenuFileNewFromGrf)
	  COMMAND_ID_HANDLER(IDM_FILE_OPEN, OnMenuFileOpen)
	  COMMAND_ID_HANDLER(IDM_FILE_CLOSE_ALL_BUT_CURRENT, OnMenuFileCloseAllButCurrent)
	  COMMAND_ID_HANDLER(IDM_FILE_SAVE_ALL, OnMenuFileSaveAll)
	  COMMAND_RANGE_HANDLER(ID_FILE_MRU_FIRST, ID_FILE_MRU_LAST, OnMenuFileRecent)
	  COMMAND_ID_HANDLER(IDM_FILE_EXIT, OnMenuFileExit)
	    // Same handler as IDM_FILE_CLOSE_ALL_BUT_CURRENT, does close other views (not current doc)
	  COMMAND_ID_HANDLER(IDM_WINDOW_CLOSE_ALL_BUT_CURRENT, OnMenuFileCloseAllButCurrent)
	  COMMAND_ID_HANDLER(IDM_HELP__ONLINE_TUTORIAL, OnMenuHelpOnlineTutorial)
	  COMMAND_ID_HANDLER(IDM_HELP_ABOUT, OnMenuHelpAbout)
	  MESSAGE_HANDLER(WM_MDISETMENU, OnMDISetMenu)
	    // Let the MDI child process unhandled commands
	  CHAIN_MDI_CHILD_COMMANDS()
	    // Let base classes take care of their own messages when not handled here
	  CHAIN_MSG_MAP(CUpdateUI<CMainWindow>)
	  CHAIN_MSG_MAP(baseFrameClass_)
	END_MSG_MAP()

	BEGIN_UPDATE_UI_MAP(CMainWindow)
	  UPDATE_ELEMENT(IDM_FILE_NEW,                  UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
	  UPDATE_ELEMENT(IDM_FILE_NEW_FROM_GRF,         UPDUI_MENUPOPUP)
	  UPDATE_ELEMENT(IDM_FILE_OPEN,                 UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
	  UPDATE_ELEMENT(IDM_FILE_CLOSE,                UPDUI_MENUPOPUP)
	  UPDATE_ELEMENT(IDM_FILE_CLOSE_ALL_BUT_CURRENT, UPDUI_MENUPOPUP)
	  UPDATE_ELEMENT(IDM_FILE_SAVE,                 UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
	  UPDATE_ELEMENT(IDM_FILE_SAVE_AS,              UPDUI_MENUPOPUP)
	  UPDATE_ELEMENT(IDM_FILE_SAVE_ALL,             UPDUI_MENUPOPUP)

	  UPDATE_ELEMENT(IDM_EDIT_OPEN,                 UPDUI_MENUPOPUP)
	  UPDATE_ELEMENT(IDM_EDIT_EXTRACT,              UPDUI_MENUPOPUP)
	  UPDATE_ELEMENT(IDM_EDIT_RENAME,               UPDUI_MENUPOPUP)
	  UPDATE_ELEMENT(IDM_EDIT_DELETE,               UPDUI_MENUPOPUP)
	  UPDATE_ELEMENT(IDM_EDIT_NEW_FOLDER,           UPDUI_MENUPOPUP)
	  UPDATE_ELEMENT(IDM_EDIT_ADD_FILES,            UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
	  UPDATE_ELEMENT(IDM_EDIT_ADD_DIRECTORY,        UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
	  UPDATE_ELEMENT(IDM_EDIT_ADD_FROM_LIST,        UPDUI_MENUPOPUP)
	  UPDATE_ELEMENT(IDM_EDIT_MERGE_WITH_GRF,       UPDUI_MENUPOPUP)

	  UPDATE_ELEMENT(IDM_VIEW_GRF_EXPLORER_PANE,    UPDUI_MENUPOPUP)
	  UPDATE_ELEMENT(IDM_VIEW_TYPE_LIST,            UPDUI_MENUPOPUP)
	  UPDATE_ELEMENT(IDM_VIEW_TYPE_DETAILS,         UPDUI_MENUPOPUP)
	  UPDATE_ELEMENT(IDM_VIEW_REFRESH,              UPDUI_MENUPOPUP)

	  UPDATE_ELEMENT(IDM_WINDOW_CLOSE_ALL_BUT_CURRENT, UPDUI_MENUPOPUP)
	  UPDATE_ELEMENT(IDM_WINDOW_CASCADE, UPDUI_MENUPOPUP)
	  UPDATE_ELEMENT(IDM_WINDOW_TILE_HORZ, UPDUI_MENUPOPUP)
	  UPDATE_ELEMENT(IDM_WINDOW_ARRANGE, UPDUI_MENUPOPUP)
	END_UPDATE_UI_MAP()

};

#endif  // ndef __WNDMAIN_H__
