// $Id: DlgAbout.h 9 2005-08-12 17:33:08Z Rasqual $

// *
// *   gryff, a GRF archive management utility, src/DlgAbout.h
// *   Copyright (C) 2003, 2004 Rasqual Twilight
// *
// *   This program is distributed in the hope that it will be useful,
// *   but WITHOUT ANY WARRANTY; without even the implied warranty of
// *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// *

// * DlgAbout.h
// * About dialog box
// *
// *@author $Author: Rasqual $
// *@version $Revision: 9 $
// *

#if !defined(__DLGABOUT_H__)
#define __DLGABOUT_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __ATLFRAME_H__
   #error DlgAbout.h requires <atlframe.h> to be included first
#endif

#include  <viksoe/SimpleHtml.h>


///////////////////////////////////////////////////////////////////////////////
// Classes in this file:
//
// CAboutDialog
//   Class instantiating a dialog box.
//   Usage:  CAboutDialog dlg; dlg.DoModal();
//   See  :  CDialogImpl <atlwin.h>


class CAboutDialog : public CDialogImpl<CAboutDialog>, public CDialogResize<CAboutDialog>
{
public:
	enum { IDD = IDD_ABOUTBOX,
		RICHEDIT_TEXT_LIMIT = 32*1024U,
		URL_LENGTH_LIMIT = 2084U // 2084 is the IE limit
	};

protected:
	CSimpleHtmlCtrl  m_ctrlHTML;
	CHARRANGE        m_charg;

protected:
	LRESULT  OnInitDialog(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT  OnMsgFilter(LPNMHDR pnmh);
	LRESULT  OnLink(LPNMHDR pnmh);
	LRESULT  OnDestroyClipboard(UINT, WPARAM, LPARAM, BOOL&);

	 // Command handlers
	LRESULT  OnMenuOpenUrl(WORD, WORD wID, HWND, BOOL&);
	LRESULT  OnMenuCopyUrl(WORD, WORD wID, HWND, BOOL&);
	LRESULT  OnMenuRedo(WORD, WORD wID, HWND, BOOL&);
	LRESULT  OnMenuUndo(WORD, WORD wID, HWND, BOOL&);
	LRESULT  OnMenuCut(WORD, WORD wID, HWND, BOOL&);
	LRESULT  OnMenuCopy(WORD, WORD wID, HWND, BOOL&);
	LRESULT  OnMenuPaste(WORD, WORD wID, HWND, BOOL&);
	LRESULT  OnMenuDelete(WORD, WORD wID, HWND, BOOL&);
	LRESULT  OnMenuSelectAll(WORD, WORD wID, HWND, BOOL&);
	LRESULT  OnOK(WORD, WORD wID, HWND, BOOL&);
	LRESULT  OnCancel(WORD, WORD wID, HWND, BOOL&);

	void  GetURL_(LPTSTR lpszURL, UINT cch);

	BEGIN_MSG_MAP(CAboutDialog)
	  MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	  MESSAGE_HANDLER(WM_DESTROYCLIPBOARD, OnDestroyClipboard)
	  NOTIFY_HANDLER_EX(IDC_ABOUT_CREDITS, EN_MSGFILTER, OnMsgFilter)
	  NOTIFY_HANDLER_EX(IDC_ABOUT_CREDITS, EN_LINK, OnLink)

	  COMMAND_ID_HANDLER(IDM_OPEN_URL,  OnMenuOpenUrl)
	  COMMAND_ID_HANDLER(IDM_COPY_URL,  OnMenuCopyUrl)

	  COMMAND_ID_HANDLER(IDM_REDO,      OnMenuRedo)
	  COMMAND_ID_HANDLER(IDM_UNDO,      OnMenuUndo)

	  COMMAND_ID_HANDLER(IDM_CUT,       OnMenuCut)
	  COMMAND_ID_HANDLER(IDM_COPY,      OnMenuCopy)
	  COMMAND_ID_HANDLER(IDM_PASTE,     OnMenuPaste)
	  COMMAND_ID_HANDLER(IDM_DELETE,    OnMenuDelete)
	  COMMAND_ID_HANDLER(IDM_SELECTALL, OnMenuSelectAll)

	  COMMAND_ID_HANDLER(IDOK, OnOK)
	  COMMAND_ID_HANDLER(IDCANCEL, OnCancel)

	  CHAIN_MSG_MAP(CDialogResize<CAboutDialog>)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CAboutDialog)
	  DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_X)
	  DLGRESIZE_CONTROL(IDC_ABOUT_CREDITS, DLSZ_SIZE_X | DLSZ_SIZE_Y | DLSZ_REPAINT)
	END_DLGRESIZE_MAP()
};

#endif   // !defined(__DLGABOUT_H__)
