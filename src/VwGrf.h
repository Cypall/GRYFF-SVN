// $Id: VwGrf.h 12 2005-08-14 01:42:51Z Rasqual $

// *
// *   gryff, a GRF archive management utility, src/VwGrf.h
// *   Copyright (C) 2003-2005 Rasqual Twilight
// *
// *   This program is distributed in the hope that it will be useful,
// *   but WITHOUT ANY WARRANTY; without even the implied warranty of
// *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// *

// * VwGrf.h
// * MDI child Frame's associated view definition
// *
// *@author $Author: Rasqual $
// *@version $Revision: 12 $
// *

#ifndef __VWGRF_H__
#define __VWGRF_H__

#pragma once

#include  <atlsplit.h>

#include  <docview.h>

#include  "DocGrf.h"

#include  "ProcComm.h"

class CMyPaneContainer;
class CMyTreeCtrl;
class CMyListCtrl;

#define FORWARD_NOTIFY_CODE(cd) \
	if ( uMsg == WM_NOTIFY && ((LPNMHDR)lParam)->code == cd ) \
	{ \
		SetMsgHandled(TRUE); \
		lResult = GetParent().SendMessage(uMsg, wParam, lParam); \
		return TRUE; \
	}

// Only set handled if message return value is not zero
#define FORWARD_NOTIFY_CODE_RETURN_NONZERO(cd) \
	if ( uMsg == WM_NOTIFY && ((LPNMHDR)lParam)->code == cd ) \
	{ \
		if ( 0 != (lResult = GetParent().SendMessage(uMsg, wParam, lParam)) ) \
		{ \
			SetMsgHandled(TRUE); \
			return TRUE; \
		} \
	}

#define BEGIN_NOTIFY_CODE_INLINE(cd) \
	if ( uMsg == WM_NOTIFY && ((LPNMHDR)lParam)->code == cd ) \
	{
#define END_NOTIFY_CODE_INLINE(cd) \
	}

#define FORWARD_MESSAGE(msg) \
	if ( uMsg == msg ) \
	{ \
		SetMsgHandled(TRUE); \
		lResult = GetParent().SendMessage(uMsg, wParam, lParam); \
		return TRUE; \
	}

///////////////////////////////////////////////////////////////////////////////
// Classes in this file:
//
// CGrfView
//   Class instantiating a view of the document contained in the frame window.
//   See  :  CFrameWindowImpl <atlframe.h>


class CGrfView : public CFrameWindowImpl<CGrfView>,
                 public CViewImpl<CGrfView, CGrfDocView>
{
public:
	DECLARE_WND_CLASS(NULL)

	///////////////////////// - CTOR / DTOR - /////////////////////////////
public:
	CGrfView();
	~CGrfView();

	///////////////////////// - data members - /////////////////////////////
protected:
	CGrfDoc          *m_pDoc;

	CSplitterWindow  m_wndVertSplit;
	CMyPaneContainer *m_pwndLeft;
	CMyTreeCtrl      *m_pwndLeftSub;
	CMyListCtrl      *m_pwndRight;

	CCommandBarCtrl  *m_pcmd;

	CString          m_strCwd;
	bool             m_ListIsLastFocused;
	HTREEITEM        m_MemorizedSelection;

public:
	CMyTreeCtrl *GetTreeViewPtr()
	{
		return m_pwndLeftSub;
	}

	CMyListCtrl *GetListViewPtr()
	{
		return m_pwndRight;
	}

	const CMyTreeCtrl *GetTreeViewPtr() const
	{
		return m_pwndLeftSub;
	}

	const CMyListCtrl *GetListViewPtr() const
	{
		return m_pwndRight;
	}

	const CString &GetCwd() const
	{
		return m_strCwd;
	}

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

	int  GetViewTypeId() const;
	void OnUpdate(CGrfDocView* pSender, LPARAM Hint, LPVOID pHint);
	LRESULT ReplaceDirPrefix(CTreeItem &ti, int nCurrentPrefixLength, const CString &strPrefix);
	LRESULT  RmdirRecursive(const CString &strFullPath);
	LRESULT  RmdirRecursive(CTreeItem &ti);
	LRESULT  UpdateView_List(const CTreeItem &ti, bool ForceRedraw = false);
	void  UpdateItemName_List(int iItem, const CString &strNewName);
	LRESULT  OnSetFocus(UINT, WPARAM, LPARAM, BOOL& bHandled);
	LRESULT Navigate(const CTreeItem &ti, bool bSetFocus = true);
	BOOL  SetCommandBar(CCommandBarCtrl *pCmd)
	{
		m_pcmd = pCmd;
		return TRUE;
	}

protected:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
protected:
	 /// CGrfView::OnCreate()
	 //  Called when a new document view is created by the frame
	LRESULT OnCreate(LPCREATESTRUCT lpcs);
	void OnDestroy();
	void OnFinalMessage(HWND hWnd);

	LRESULT  OnEraseBackground(HDC);
	/**
		Painting handler
	**/
	void  OnPaint(HDC);

	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	/**
		UWM_NAVIGATE_HTREE message handler
	**/
	LRESULT OnNavigateHtree(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	/**
		UWM_GETDOCPTR registered message handler
		Message is sent to retrieve a CGrfDoc object address from within this process
	**/
	LRESULT  OnGetDocumentPtr(UINT, WPARAM, LPARAM, BOOL&);

	/**
		UWM_GETSINGLEPANEMODE
		returns either SPLIT_PANE_NONE or SPLIT_PANE_RIGHT.
	**/
	LRESULT  OnGetSinglePaneMode(UINT, WPARAM, LPARAM, BOOL&);

	/**
		UWM_APPENDLISTITEM
		Message is sent to add an item to the list. lParam is the address to a GRFLISTVIEWAPPENDINFO structure
	**/
	LRESULT  OnAppendItem_List(UINT, WPARAM, LPARAM, BOOL&);

public:
	void __stdcall  GetShellFileInfo(const CString& strEntryName, SHFILEINFO &sfi, LPCTSTR &pstrTypeName);
	int __stdcall  InsertItem_List(int &iListPos, const CGrfFileEntry *precord, const HTREEITEM *ptreeitem, const CString &strEntryName, int iIcon, LPCTSTR pstrTypeName);

	/**
		Formats the number columns and origin column for file entries
		(you must fill the others yourself)
	**/
	void __stdcall  FormatItem_List(const int &iListPos, const CGrfFileEntry *precord);

	/**
		Extract selected item(s).
	**/
	LRESULT  OnEditExtract();

	/**
		Opens selected item(s).
		PromptExtractPlace argument is true when user should be prompted for destination.
		Otherwise, uses a temporary directory and llaunches through ::ShellExecuteEx().
	**/
	LRESULT  OnEditOpen(bool PromptExtractPlace = false);

protected:
	LRESULT  RequestExtractionPath_(LPCTSTR pDefaultPath, CGrfCache &, const CAtlList< std::pair<uint8_t, CString> > &EntriesToExtract);

public:
	LRESULT  OnEditRename();

	LRESULT  OnEditDelete();

	void  UpdateUI();

protected:
	/**
		UWM_SUBVIEWGOTFOCUS
		Message is sent by a child subview to notify it has got focus.
		wParam is either LISTVIEW_ID or TREEVIEW_ID
	**/
	LRESULT  OnConfirmFocus(UINT, WPARAM wParam, LPARAM, BOOL&);

	LRESULT  OnDoubleClick(LPNMHDR pnmh);

	LRESULT  OnRClick(LPNMHDR pnmh);

	/**
		Tries to handle <TAB>
	**/
	LRESULT  OnKeyDown(LPNMHDR pnmh);

	LRESULT OnLVBeginLabelEdit(LPNMHDR /* pnmh */);
	LRESULT OnTVBeginLabelEdit(LPNMHDR /* pnmh */);

	LRESULT _OnLVDoubleClick(LPNMHDR pnmh);

	void _OnUpdateDefault(CString *pChDir = 0);
	void _OnUpdateAddedFiles(CAtlList< std::pair<uint8_t, CString> > *pAddedFiles);
	void _OnUpdateAddedDirectories(CAtlList< std::pair<uint8_t, CString> > *pAddedFiles);
	void _OnUpdateAddedFilesDirectories(CAtlList< std::pair<uint8_t, CString> > *pAddedFiles);
	LRESULT OnContextMenu(HWND hWnd, const CPoint&);
	LRESULT _OnLVContextMenu(HWND hWnd, const CPoint&);
	LRESULT _OnTVContextMenu(HWND hWnd, const CPoint&);
	LRESULT  OnLVBeginDrag(LPNMHDR pnmh);
	LRESULT  OnLVItemChanged(LPNMHDR pnmh);

	// Command handlers
	void  OnTogglePane(UINT uCode, int nID, HWND hwndCtrl);

protected:
	BEGIN_MSG_MAP(CGrfView)
	  MSG_WM_CREATE(OnCreate)
	  MSG_WM_DESTROY(OnDestroy)
	  MSG_WM_ERASEBKGND(OnEraseBackground)
	  MSG_WM_PAINT(OnPaint)
      MSG_WM_CONTEXTMENU(OnContextMenu)
	  MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
	  MESSAGE_HANDLER(WM_SIZE, OnSize)

	  MESSAGE_HANDLER(UWM_NAVIGATE_HTREE, OnNavigateHtree)
 	  MESSAGE_HANDLER(UWM_GETDOCPTR, OnGetDocumentPtr)
 	  MESSAGE_HANDLER(UWM_GETSINGLEPANEMODE, OnGetSinglePaneMode)
 	  MESSAGE_HANDLER(UWM_APPENDLISTITEM, OnAppendItem_List)
 	  MESSAGE_HANDLER(UWM_SUBVIEWGOTFOCUS, OnConfirmFocus)

	  FORWARD_MESSAGE(UWM_DRAGDROP)

	  NOTIFY_CODE_HANDLER_EX(LVN_KEYDOWN, OnKeyDown)
	  NOTIFY_CODE_HANDLER_EX(NM_DBLCLK, OnDoubleClick)
	  NOTIFY_CODE_HANDLER_EX(NM_RCLICK, OnRClick)
//reflect	  NOTIFY_CODE_HANDLER_EX(LVN_COLUMNCLICK, OnLVColumnClick)
	  // Try to have keypresses handled by document frame
	  FORWARD_NOTIFY_CODE_RETURN_NONZERO(LVN_KEYDOWN)
	  FORWARD_NOTIFY_CODE_RETURN_NONZERO(TVN_KEYDOWN)
	  // Handle <TAB>
	  NOTIFY_CODE_HANDLER_EX(LVN_KEYDOWN, OnKeyDown)
	  NOTIFY_CODE_HANDLER_EX(TVN_KEYDOWN, OnKeyDown)

	  NOTIFY_CODE_HANDLER_EX(LVN_BEGINLABELEDIT, OnLVBeginLabelEdit)
	  FORWARD_NOTIFY_CODE(LVN_ENDLABELEDIT)
	  NOTIFY_CODE_HANDLER_EX(TVN_BEGINLABELEDIT, OnTVBeginLabelEdit)
	  FORWARD_NOTIFY_CODE(TVN_ENDLABELEDIT)
	  NOTIFY_CODE_HANDLER_EX(LVN_BEGINDRAG, OnLVBeginDrag)
	  NOTIFY_CODE_HANDLER_EX(LVN_ITEMCHANGED, OnLVItemChanged)
	  COMMAND_ID_HANDLER_EX(ID_PANE_CLOSE, OnTogglePane)
	  REFLECT_NOTIFICATIONS()

	END_MSG_MAP()
};





#endif //__VWGRF_H__
