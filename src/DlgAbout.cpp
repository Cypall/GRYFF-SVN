// $Id: DlgAbout.cpp 9 2005-08-12 17:33:08Z Rasqual $

// *
// *   gryff, a GRF archive management utility, src/DlgAbout.cpp
// *   Copyright (C) 2003, 2004 Rasqual Twilight
// *
// *   This program is distributed in the hope that it will be useful,
// *   but WITHOUT ANY WARRANTY; without even the implied warranty of
// *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// *

// * DlgAbout.cpp
// * About dialog box
// * Remember to LoadLibrary Richedit's dll prior to creation
// *
// *@author $Author: Rasqual $
// *@version $Revision: 9 $
// *

#include  "stdwtl.h"
#include  <codeproject/url.h>

#pragma hdrstop
#include  "resource.h"
#include  "DlgAbout.h"

///////////////////////////////////////////////////////////////////////////////

LRESULT CAboutDialog::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
{
	// Init the CDialogResize code
	DlgResize_Init();

	m_ctrlHTML.SubclassWindow(GetDlgItem(IDC_ABOUT_CREDITS));
	m_ctrlHTML.LimitText(RICHEDIT_TEXT_LIMIT);
	m_ctrlHTML.SetEventMask(ENM_MOUSEEVENTS | ENM_SCROLLEVENTS | ENM_LINK | ENM_SCROLL);
	m_ctrlHTML.SendMessage(EM_SETTYPOGRAPHYOPTIONS, (WPARAM) TO_ADVANCEDTYPOGRAPHY, (LPARAM) TO_ADVANCEDTYPOGRAPHY);

COLORREF  rBkClr = ::GetSysColor(COLOR_BTNFACE);
COLORREF  rTxtClr = RGB(0x12,0x10,0xff);
TCHAR  lpszHTMLData[RICHEDIT_TEXT_LIMIT];
	::LoadString(
#if (_ATL_VER >= 0x0700)
	  ATL::_AtlBaseModule.GetResourceInstance(),
#else //!(_ATL_VER >= 0x0700)
	  _Module.GetResourceInstance(),
#endif //!(_ATL_VER >= 0x0700)
	  IDS_ABOUT_CREDITS, lpszHTMLData, RICHEDIT_TEXT_LIMIT);
	m_ctrlHTML.Load(lpszHTMLData, 0, &rTxtClr, &rBkClr);

	this->CenterWindow();
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT CAboutDialog::OnMsgFilter(LPNMHDR pnmh)
{
	MSGFILTER  *mf = reinterpret_cast<MSGFILTER*>(pnmh);
	ATLASSERT(mf);
	ATLASSERT(!::IsBadReadPtr(mf, sizeof(MSGFILTER)));
	if ( mf->msg == WM_RBUTTONDOWN )
	{
		::SetFocus(m_ctrlHTML.m_hWnd);
		// handle right-click
	CPoint  pt(static_cast<DWORD>(mf->lParam));
		m_ctrlHTML.ClientToScreen(&pt);

		if ( m_ctrlHTML.GetOptions() & ECO_READONLY )
		{
			m_ctrlHTML.SetReadOnly();
		}

		// FIXME: TODO: (1.8) If a language file is set, read menu strings from it (and check missing lines)
		// Load pop-up menu resources and display menu
	CMenu  menuPopup;
		menuPopup.CreatePopupMenu();
		menuPopup.AppendMenu(MF_STRING, IDM_REDO,       CString(MAKEINTRESOURCE(IDS_REDO)));
		menuPopup.AppendMenu(MF_STRING, IDM_UNDO,       CString(MAKEINTRESOURCE(IDS_UNDO)));
		menuPopup.AppendMenu(MF_SEPARATOR);
		menuPopup.AppendMenu(MF_STRING, IDM_CUT,        CString(MAKEINTRESOURCE(IDS_CUT)));
		menuPopup.AppendMenu(MF_STRING, IDM_COPY,       CString(MAKEINTRESOURCE(IDS_COPY)));
		menuPopup.AppendMenu(MF_STRING, IDM_PASTE,      CString(MAKEINTRESOURCE(IDS_PASTE)));
		menuPopup.AppendMenu(MF_STRING, IDM_DELETE,     CString(MAKEINTRESOURCE(IDS_DELETE)));
		menuPopup.AppendMenu(MF_SEPARATOR);
		menuPopup.AppendMenu(MF_STRING, IDM_SELECTALL,  CString(MAKEINTRESOURCE(IDS_SELECTALL)));

		menuPopup.EnableMenuItem(IDM_REDO, m_ctrlHTML.CanRedo()? MF_ENABLED: MF_GRAYED);
		menuPopup.EnableMenuItem(IDM_UNDO, m_ctrlHTML.CanUndo()? MF_ENABLED: MF_GRAYED);

		menuPopup.EnableMenuItem(IDM_CUT,    m_ctrlHTML.CanCut()  ? MF_ENABLED: MF_GRAYED);
		menuPopup.EnableMenuItem(IDM_COPY,   m_ctrlHTML.CanCopy() ? MF_ENABLED: MF_GRAYED);
		menuPopup.EnableMenuItem(IDM_PASTE,  m_ctrlHTML.CanPaste()? MF_ENABLED: MF_GRAYED);
		menuPopup.EnableMenuItem(IDM_DELETE, m_ctrlHTML.CanClear()? MF_ENABLED: MF_GRAYED);
		menuPopup.EnableMenuItem(IDM_SELECTALL, m_ctrlHTML.GetTextLength()? MF_ENABLED: MF_GRAYED);

		menuPopup.TrackPopupMenu(
			TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON, pt.x, pt.y, m_hWnd);
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT CAboutDialog::OnLink(LPNMHDR pnmh)
{
ENLINK * penlink = reinterpret_cast<ENLINK*>(pnmh);
	ATLASSERT(penlink);
	ATLASSERT(!::IsBadReadPtr(penlink, sizeof(ENLINK)));
	if ( penlink->msg == WM_LBUTTONDOWN )
	{
		::SetFocus(m_ctrlHTML.m_hWnd);
		m_charg = penlink->chrg;
	CPoint  pt(static_cast<DWORD>(penlink->lParam));
		m_ctrlHTML.ClientToScreen(&pt);
	CMenu  menuPopup;
		menuPopup.CreatePopupMenu();
		menuPopup.AppendMenu(MF_STRING, IDM_OPEN_URL,   CString(MAKEINTRESOURCE(IDS_OPEN_URL)));
		menuPopup.AppendMenu(MF_STRING, IDM_COPY_URL,   CString(MAKEINTRESOURCE(IDS_COPY_URL)));
		menuPopup.SetMenuDefaultItem(IDM_OPEN_URL);
		menuPopup.TrackPopupMenu(
			TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON, pt.x, pt.y, m_hWnd);
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT CAboutDialog::OnMenuOpenUrl(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
TCHAR  lpszURL[URL_LENGTH_LIMIT];
	this->GetURL_(lpszURL, URL_LENGTH_LIMIT);
CURL  url;
	url.Open(lpszURL);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT CAboutDialog::OnMenuCopyUrl(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
TCHAR  lpszURL[URL_LENGTH_LIMIT];
	this->GetURL_(lpszURL, URL_LENGTH_LIMIT);
	if ( ::OpenClipboard(m_hWnd) )
	{
		::EmptyClipboard();
		  //Determine required buffer length
#ifdef UNICODE
	int nMbCount = ::WideCharToMultiByte(::GetACP(), 0, lpszURL, -1, 0, 0, NULL, NULL);
#else
	int nMbCount(URL_LENGTH_LIMIT);
#endif
	HANDLE  hClipboardData = ::GlobalAlloc(GHND, nMbCount * sizeof(char));
		if ( hClipboardData )
		{
		char *lpstrCopy = (char*)::GlobalLock(hClipboardData);
			if ( lpstrCopy == 0 )
			{
				::GlobalFree(hClipboardData);
			}
			else
			{
#ifdef UNICODE
				::WideCharToMultiByte(::GetACP(), 0, lpszURL, -1, lpstrCopy, nMbCount, NULL, NULL);
#else
				lstrcpyn( lpstrCopy, lpszURL, nMbCount);
#endif
				::GlobalUnlock(hClipboardData);
                ::SetClipboardData(CF_TEXT, hClipboardData);
			}
		}
		  //Unicode only: CF_UNICODETEXT format
#ifdef UNICODE
		hClipboardData = ::GlobalAlloc(GHND, URL_LENGTH_LIMIT * sizeof(TCHAR));
		if ( hClipboardData )
		{
		TCHAR *lpstrCopy = (TCHAR*)::GlobalLock(hClipboardData);
			if ( lpstrCopy == 0 )
			{
				::GlobalFree(hClipboardData);
			}
			else
			{
				lstrcpyn( lpstrCopy, lpszURL, URL_LENGTH_LIMIT);
				::GlobalUnlock(hClipboardData);
                ::SetClipboardData(CF_UNICODETEXT, hClipboardData);
			}
		}
#endif

		::CloseClipboard();
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT CAboutDialog::OnDestroyClipboard(UINT, WPARAM, LPARAM, BOOL&)
{
	// DO NOT DO THIS!
//	if ( m_hClipboardData )
//	{
//		::GlobalFree(m_hClipboardData);
//		m_hClipboardData = 0;
//	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT CAboutDialog::OnMenuRedo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	m_ctrlHTML.Redo();
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT CAboutDialog::OnMenuUndo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	m_ctrlHTML.OnEditUndo(wNotifyCode, wID, hWndCtl, bHandled);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT CAboutDialog::OnMenuCut(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	m_ctrlHTML.OnEditCut(wNotifyCode, wID, hWndCtl, bHandled);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT CAboutDialog::OnMenuCopy(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	m_ctrlHTML.OnEditCopy(wNotifyCode, wID, hWndCtl, bHandled);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT CAboutDialog::OnMenuPaste(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	m_ctrlHTML.OnEditPaste(wNotifyCode, wID, hWndCtl, bHandled);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT CAboutDialog::OnMenuDelete(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	m_ctrlHTML.OnEditClear(wNotifyCode, wID, hWndCtl, bHandled);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT CAboutDialog::OnMenuSelectAll(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	m_ctrlHTML.OnEditSelectAll(wNotifyCode, wID, hWndCtl, bHandled);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT CAboutDialog::OnOK(WORD, WORD wID, HWND, BOOL&)
{
	EndDialog(wID);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT CAboutDialog::OnCancel(WORD, WORD wID, HWND, BOOL&)
{
	EndDialog(wID);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

void  CAboutDialog::GetURL_(LPTSTR lpszURL, UINT cch)
{
	m_ctrlHTML.ExtractLink(m_charg, lpszURL, cch);
}

