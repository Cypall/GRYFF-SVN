// $Id: DlgInvalidEntries.h 9 2005-08-12 17:33:08Z Rasqual $

// *
// *   gryff, a GRF archive management utility, src/DlgInvalidEntries.h
// *   Copyright (C) 2003, 2004 Rasqual Twilight
// *
// *   This program is distributed in the hope that it will be useful,
// *   but WITHOUT ANY WARRANTY; without even the implied warranty of
// *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// *

// * DlgInvalidEntries.h
// * Wrapper for the IDD_EXPORT_INVALID_ENTRIES dialog
// *
// *@author $Author: Rasqual $
// *@version $Revision: 9 $
// *

#if !defined(__DLGINVALIDENTRIES_H__)
#define __DLGINVALIDENTRIES_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __ATLFRAME_H__
   #error DlgInvalidEntries.h requires <atlframe.h> to be included first
#endif

#include  <utility>  // pair
#include  "gen_grfio.h"

///////////////////////////////////////////////////////////////////////////////
// Classes in this file:
//
// CInvalidEntriesDialog
//   Class instantiating an 'invalid entries' dialog.
//   a CAtlList< std::pair<uint8_t, CString> > pointer is passed as the lParam of WM_INITDIALOG,
//   containing a list of invalid entries
//   to be used for filling an edit box of the dialog.

class CInvalidEntriesDialog : public CDialogImpl<CInvalidEntriesDialog>
{
public:
	enum { IDD = IDD_EXPORT_INVALID_ENTRIES };

protected:
	CString  m_Title;
public:
	CInvalidEntriesDialog(const CString& title = _T("")) : m_Title(title)
	{}

protected:
	LRESULT OnInitDialog(UINT, WPARAM, LPARAM lParam, BOOL&)
	{
		if ( m_Title.GetLength() != 0 )
		{
			this->SetWindowText(m_Title);
		}
		if ( lParam )
		{
		CAtlList< std::pair<uint8_t, CString> > *pEntries = reinterpret_cast<CAtlList< std::pair<uint8_t, CString> > *>(lParam);
		CString strEntries;
			for ( POSITION pos = pEntries->GetHeadPosition(); pos; )
			{
				std::pair<uint8_t, CString> p(pEntries->GetNext(pos));
				strEntries += (p.first & 1)? _T("F>> ") : _T("D ] ");
				strEntries += p.second;
				strEntries += _T("\r\n");
			}
			strEntries.Replace(_T("&"), _T("&&"));
			this->SetDlgItemText(IDC_EDIT_EXPORT_INVALID_ENTRIES_LIST, strEntries);
		}
		::MessageBeep(MB_ICONEXCLAMATION);
		return TRUE;
	}

	///////////////////////////////////////////////////////////////////////////////

	LRESULT OnOK(WORD, WORD wID, HWND, BOOL&)
	{
		EndDialog(wID);
		return 0;
	}

	///////////////////////////////////////////////////////////////////////////////

	LRESULT OnCancel(WORD, WORD wID, HWND, BOOL&)
	{
		EndDialog(wID);
		return 0;
	}

protected:
	BEGIN_MSG_MAP(CInvalidEntriesDialog)
	  MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	  COMMAND_ID_HANDLER(IDOK, OnOK)
	  COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	END_MSG_MAP()
};

#endif   // !defined(__DLGINVALIDENTRIES_H__)
