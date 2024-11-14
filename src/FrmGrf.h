
// *
// *   gryff, a GRF archive management utility, src/FrmGrf.h
// *   Copyright (C) 2003, 2004 Rasqual Twilight
// *
// *   This program is distributed in the hope that it will be useful,
// *   but WITHOUT ANY WARRANTY; without even the implied warranty of
// *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// *

// * FrmGrf.h
// * MDI child Frame definition
// *
// *@author $Author: Rasqual $
// *@version $Revision: 12 $
// *

#ifndef __FRMGRF_H__
#define __FRMGRF_H__

#pragma once

#include  <utility>  // pair
#include  <memory>  // pair

#include  "ProcComm.h"


class CGrfView;
class CGrfDoc;
template <typename T> class IErrorCode;
struct CDocumentSaverData;
class IGrfEntry;
template <typename T> class SaveCommand;

///////////////////////////////////////////////////////////////////////////////
// Classes in this file:
//
// CGrfFrame
//   Class instantiating a frame window, MDI child of CMainWindow.
//   See  :  CMDIChildWindowImpl <atlframe.h>

typedef ATL::CWinTraits<WS_OVERLAPPEDWINDOW | WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, WS_EX_MDICHILD /*| WS_EX_ACCEPTFILES*/>	CDnDMDIChildWinTraits;


///////////////////////////////////////////////////////////////////////////////

class CGrfFrame : public CMDIChildWindowImpl<CGrfFrame, CMDIWindow, CDnDMDIChildWinTraits>,
                  public CFromHandle<CGrfFrame>,
                  public IDropTarget
{
private:
	typedef  CMDIChildWindowImpl<CGrfFrame, CMDIWindow, CDnDMDIChildWinTraits>  BaseFrameClass_;

public:
	DECLARE_FRAME_WND_CLASS(NULL, IDC_GRYFFCHILD)

	//////////////////////// - static methods - /////////////////////////////
public:

	/**
		Overridden to fix ATL bug when "delete this;" is performed OnFinalMessage()
		while message is not final.
	**/
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
	/**
		Returns whether a supported clipboard format is detected when
		dragging an object over the window
	**/
	static bool  IsAcceptedFormat_(IDataObject *);

	///////////////////////// - CTOR / DTOR - /////////////////////////////
public:
	/**
		Performs initialization.
	**/
	CGrfFrame();
	/**
		Performs cleanup.
	**/
	~CGrfFrame();

	///////////////////////// - data members - /////////////////////////////
protected:
	IErrorCode<int>      *m_pErrorCode;

	CGrfDoc              *m_pDoc;
	CGrfView             *m_pView;

	bool                 m_Busy;

	CCommandBarCtrl      m_LocalCmdBar;  // local command bar (for context menus)

    CComPtr<IDropTargetHelper>  m_pDropTargHlp;
    CComPtr<IDataObject>  m_pDataObject;  // obj being dragged
	LONG                 m_RefCount;
	bool                 m_AcceleratorsDisabled;
	HWND                 m_FocusedChild;
	LPARAM               m_SaveParam;

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

	//----- Overridden -----//
	virtual WNDPROC GetWindowProc()
	{
		return &CGrfFrame::WindowProc;
	}

	//----- Getters -----//
public:
	/**
		Retrieves a constant pointer to the document object.
		Use this when accessing the document in read-only mode
	**/
	const CGrfDoc  *GetDocPtr() const
	{
		return m_pDoc;
	}
protected:
	/**
		Retrieves a reference to the document object
		Use this when modifying the document
	**/
	CGrfDoc  &GetDoc()
	{
		return *m_pDoc;
	}
public:
	/**
		Retrieves a constant pointer to the view object.
		Use this when accessing the view in read-only mode
	**/
	const CGrfView  *GetViewPtr() const
	{
		return m_pView;
	}
protected:
	/**
		Retrieves a reference to the view object
		Use this when modifying the view
	**/
	CGrfView  &GetView()
	{
		return *m_pView;
	}

public:
	/**
		Called by the doc manager upon frame initialization
	**/
	virtual void OnInitialUpdate();

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

protected:
	/**
		"Note: You must allocate the object on the heap.
		If you do not, you must override OnFinalMessage()"
		This method deletes the current object, causing the view to be destroyed,
		and unsubscribing the view.
	**/
	virtual void  OnFinalMessage(HWND);

	////----- command handlers -----////
protected:
	/**
		Called upon reception of a MDI child frame close message
		If there is only one view associated to the document, and the doc has been modified,
		prompt for file save. User may then choose to cancel frame closing by clicking cancel,
		either on the MessageBox, or on the OpenFileSave dialogs.
		In all other cases, destroy the frame.
	**/
	void  OnClose();

	/**
		Unregister the MDI child frame and destroys the view and frame
	**/
	void  OnDestroy();

	/**
	**/
	LRESULT  OnMDIActivate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled);

	/**
		Gives focus to view.
	**/
	LRESULT  OnSetFocus(HWND);

	  // user-defined WMs
	LRESULT  OnUpdate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);   // UWM_UPDATE sent by contained view
	LRESULT  OnProgress(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);   // feedback for long operations
	LRESULT  OnRepackFinished(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);   // Worker thread for packing has finished, wParam is success code
	LRESULT  OnGetMDIFrame(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);   // request for MDI parent frame
	LRESULT  OnDragDrop(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);   // UWM_DRAGDROP forwarded by view
	LRESULT  OnEnterChildFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);  // A child control takes focus

	  // Pre-translate for view
	LRESULT  OnForwardMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	  // Command ID handlers
	LRESULT  OnFileClose    (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);  // Menu File->Close (IDM_FILE_CLOSE) delegate OnClose
	LRESULT  OnFileSave     (WORD, WORD, HWND, BOOL&);  // File->Save (IDM_FILE_SAVE) delegate OnSave for existing, RequestDocumentPath_ otherwise
	LRESULT  OnFileSaveAs   (WORD, WORD, HWND, BOOL&);  // File->Save As (IDM_FILE_SAVE_AS) delegate RequestDocumentPath_

public:
	/**
		Request to save the document.
		If no associated file exists, user is prompted for a destination with RequestDocumentPath_()
		If the file is correctly saved, the path is added to the MRU documents.
	**/
	LRESULT  OnSave(bool Asynchronously);
protected:
	/**
		Shows the common OpenFileName dialog and attempts to save the document if
		a path is provided. Otherwise, returns IDCANCEL.
		pDefaultPath allows to specify what the dialog's editbox is initialized with.
		It also sets the starting directory.
	**/
	LRESULT  RequestDocumentPath_(LPCTSTR pDefaultPath, bool Asynchronously);
protected:
	LRESULT  OnEditCreateDir(WORD, WORD, HWND, BOOL&);  // Edit->New Folder (IDM_EDIT_NEW_FOLDER) or context menu
	LRESULT  OnEditAddFiles (WORD, WORD, HWND, BOOL&);  // Edit->Add Files (IDM_EDIT_ADD_FILES) or context menu
protected:
	/**
		Attempts to import files into the current working directory.
		If some entries are invalid (length check), a dialog appears and prompts the user whether
		the import should continue without these entries.
		Returns IDCANCEL if the user cancelled the import
	**/
	LRESULT  OnImportFiles_(const CAtlList< std::pair<uint8_t, CString> > *pPaths, bool fClearSelection = true);
	void  AddToView_(const CAtlList< std::pair<uint8_t, CString> > *pAddedEntries, bool bClearSelection = false, bool bSetSelection = true, bool AreAllInCurrentWorkingDir = false);
	LRESULT OnEditAddDirectory(WORD, WORD, HWND, BOOL&);  // Edit->Add Directory (IDM_EDIT_ADD_DIRECTORY) or context menu
	LRESULT OnEditImportGrf(WORD, WORD, HWND, BOOL&);
	LRESULT  OnEditOpen    (WORD, WORD, HWND, BOOL&);  // Edit->Open or context menu
	LRESULT  OnEditExtract (WORD, WORD, HWND, BOOL&);  // Edit->Extract or context menu
	LRESULT  OnEditRename   (WORD, WORD, HWND, BOOL&);  // Edit->Rename or context menu
	LRESULT OnEditDelete   (WORD, WORD, HWND, BOOL&);
	LRESULT  OnViewExplorerPane(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnViewType     (WORD, WORD, HWND, BOOL&);
	LRESULT OnViewRefresh  (WORD, WORD, HWND, BOOL&);
	LRESULT OnWindowCascade(WORD, WORD, HWND, BOOL&);
	LRESULT OnWindowTile   (WORD, WORD, HWND, BOOL&);
	LRESULT OnWindowArrangeIcons(WORD, WORD, HWND, BOOL&);

	//----- methods: Others -----//
public:
	/**
		Returns whether document has been modified.
	**/
	bool  IsModified() const;

	/**
		Retrieves a constant pointer to the document name.
	**/
	const TCHAR * const  GetName() const;

	/**
		Returns whether there is a selection.
	**/
	bool  CanPerformOpen() const;

	/**
		Returns whether selection may be renamed.
	**/
	bool  CanPerformRename() const;

	/**
		Returns whether selection may be deleted.
	**/
	bool  CanPerformDelete() const;

protected:
	/**
		Prepare structures for saving the document.
		This may be called either from the GUI or worker thread, so no UI interaction.
		pOutUnresolvedEntries must point to a valid CAtlList.
		Returns 0 on success
	**/
	LRESULT  PrepareSaveDocument(LPCTSTR pFilePath, CDocumentSaverData&);

	/**
		Saves the document to the specified pFilePath.
		Returns true on success
	**/
	bool  SaveDocument_(LPCTSTR pFilePath, bool Asynchronously);

	/**
		Starts the worker thread
	**/
	LRESULT  BeginSaveDocumentThread_(LPCTSTR pFilePath, const CAtlList< std::pair<uint8_t, CString> > &EntriesToIgnore);

	/**
		Starts a modal Progress Dialog and wait for it to complete the save operation
	**/
	LRESULT  SaveDocumentSynchronous_(LPCTSTR pFilePath, const CAtlList< std::pair<uint8_t, CString> > &EntriesToIgnore);

	/**
		Puts the specified document path at top of Most Recently Used list.
	**/
	void  RegisterMRU_(LPCTSTR szPath);

//	LRESULT  OnLVItemChanged(LPNMHDR /*pnmh*/);
	LRESULT  OnNotifyKeyDown(LPNMHDR);
	LRESULT  OnLVEndLabelEdit(LPNMHDR /*pnmh*/);
	LRESULT  OnTVEndLabelEdit(LPNMHDR /*pnmh*/);

	LRESULT  OnTVDrop(OleDropInfo &);

	LRESULT  UpdateTitle();

public:
	/**
		Returns if a locking operation is in progress, such as extracting or saving.
	**/
	bool  IsDocumentBusy() const
	{
		return m_Busy;
	}

	int  GetViewTypeId() const;

	bool  IsSinglePane() const;

	void  UpdateUI();

public:
	/**
		static callbacks
	**/
	/**
		InitializeCallback for progress dialog
		Launches the worker thread (dialog is modal).
	**/
	static void  Save_DlgInit(LPARAM lParam);
	/**
		InitializeCallback
	**/
	static void  Save_Initialize(CGrfFrame *pThis, LPARAM lParam, SaveCommand<CGrfFrame> *pThisCommand);
	/**
		FinishCallback
	**/
	static void  Save_Finish(CGrfFrame *pThis, LPARAM lParam, SaveCommand<CGrfFrame> *pThisCommand);

	/**
		InitializeProgressCallback
	**/
	static void  Save_InitializeProgress(CGrfFrame *pThis, int minimum, int maximum);
	/**
		ProgressCallback
	**/
	static void  Save_Progress(CGrfFrame *pThis, int progress);
	/**
		StatusCallback
	**/
	static void  Save_StatusCaption(CGrfFrame *pThis, const CString &statusText);
	/**
		StatusCallback
	**/
	static void  Save_StatusCaptionEx(CGrfFrame *pThis, const CString &statusText);

	void  OnSaveInitialize(LPARAM lParam, SaveCommand<CGrfFrame> *pThisCommand);
	void  OnSaveFinish(LPARAM lParam, SaveCommand<CGrfFrame> *pThisCommand);
	void  OnSaveInitializeProgress(int minimum, int maximum);
	void  OnSaveProgress(int progress);
	void  OnSaveStatusCaption(const CString &statusText);
	void  OnSaveStatusCaptionEx(const CString &statusText);


	///////////////////////// - Message loop - /////////////////////////////

protected:
	BEGIN_MSG_MAP(CGrfFrame)
	  MSG_WM_CREATE(OnCreate)
	  MSG_WM_CLOSE(OnClose)
	  MSG_WM_DESTROY(OnDestroy)

	  MESSAGE_HANDLER(WM_MDIACTIVATE, OnMDIActivate)
	  MSG_WM_SETFOCUS(OnSetFocus)
	  MESSAGE_HANDLER(WM_FORWARDMSG, OnForwardMsg)


	  MESSAGE_HANDLER(UWM_UPDATE, OnUpdate)
      MESSAGE_HANDLER(UWM_PROGRESS, OnProgress)
      MESSAGE_HANDLER(UWM_GETMDIFRAME, OnGetMDIFrame)
 	  MESSAGE_HANDLER(UWM_DRAGDROP, OnDragDrop)
	  MESSAGE_HANDLER(UWM_ENTER_CHILD_FOCUS, OnEnterChildFocus)


	if ( uMsg == WM_COMMAND && IDM_FILE_CLOSE_ALL_BUT_CURRENT == LOWORD(wParam) )
	{
		this->SetMsgHandled(TRUE);
		lResult = ::SendMessage(this->GetMDIFrame(), uMsg, wParam, lParam);
		return TRUE;
	}

	  COMMAND_ID_HANDLER(IDM_FILE_CLOSE, OnFileClose)
	  COMMAND_ID_HANDLER(IDM_FILE_SAVE, OnFileSave)
	  COMMAND_ID_HANDLER(IDM_FILE_SAVE_AS, OnFileSaveAs)
	  COMMAND_ID_HANDLER(IDM_EDIT_NEW_FOLDER, OnEditCreateDir)
	  COMMAND_ID_HANDLER(IDM_EDIT_ADD_FILES, OnEditAddFiles)
	  COMMAND_ID_HANDLER(IDM_EDIT_ADD_DIRECTORY, OnEditAddDirectory)
	  COMMAND_ID_HANDLER(IDM_EDIT_MERGE_WITH_GRF, OnEditImportGrf)
	  COMMAND_ID_HANDLER(IDM_EDIT_OPEN, OnEditOpen)
	  COMMAND_ID_HANDLER(IDM_EDIT_EXTRACT, OnEditExtract)
	  COMMAND_ID_HANDLER(IDM_EDIT_RENAME, OnEditRename)
	  COMMAND_ID_HANDLER(IDM_EDIT_DELETE, OnEditDelete)
	  if ( uMsg == WM_COMMAND && IDM_VIEW_GRF_EXPLORER_PANE == LOWORD(wParam) )
	  {
		  lResult = this->OnViewExplorerPane(uMsg, wParam, lParam, bHandled);
		  return TRUE;
	  }
	  COMMAND_RANGE_HANDLER(IDM_VIEW_TYPE_TILE, IDM_VIEW_TYPE_ICONS, OnViewType)
	  COMMAND_ID_HANDLER(IDM_VIEW_REFRESH, OnViewRefresh)
	  COMMAND_ID_HANDLER(IDM_WINDOW_CASCADE, OnWindowCascade)
	  COMMAND_ID_HANDLER(IDM_WINDOW_TILE_HORZ, OnWindowTile)
	  COMMAND_ID_HANDLER(IDM_WINDOW_ARRANGE, OnWindowArrangeIcons)


      // View-forwarded notify messages
//	  NOTIFY_CODE_HANDLER_EX(LVN_ITEMCHANGED,  OnLVItemChanged)
	  NOTIFY_CODE_HANDLER_EX(LVN_KEYDOWN,  OnNotifyKeyDown)
//	  NOTIFY_CODE_HANDLER_EX(TVN_KEYDOWN,  OnNotifyKeyDown)
	  NOTIFY_CODE_HANDLER_EX(LVN_ENDLABELEDIT, 	OnLVEndLabelEdit)
	  NOTIFY_CODE_HANDLER_EX(TVN_ENDLABELEDIT, 	OnTVEndLabelEdit)

	    // Allows to retrieve a child frame pointer from anywhere (using a window message)
	  IMPLEMENT_FROMHANDLE()
	  CHAIN_MSG_MAP(BaseFrameClass_)
	END_MSG_MAP()

};



#endif //__FRMGRF_H__
