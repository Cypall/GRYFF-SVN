// $Id: DlgCheckBoxMultiFileDialog.h 9 2005-08-12 17:33:08Z Rasqual $

// *
// *   gryff, a GRF archive management utility, src/DlgCheckBoxMultiFileDialog.h
// *   Copyright (C) 2003, 2004, 2005 Rasqual Twilight
// *
// *   This program is distributed in the hope that it will be useful,
// *   but WITHOUT ANY WARRANTY; without even the implied warranty of
// *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// *

// * DlgCheckBoxMultiFileDialog.h
// * A File dialog that can select multiple files and adds a checkbox from a resource dialog
// *
// *@author $Author: Rasqual $
// *@version $Revision: 9 $
// *

#if !defined(__DLGCHECKBOXMULTIFILEDIALOG_H__)
#define __DLGCHECKBOXMULTIFILEDIALOG_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __ATLFRAME_H__
   #error DlgCheckBoxMultiFileDialog.h requires "fnmanip.h" to be included first
#endif

///////////////////////////////////////////////////////////////////////////////
// Classes in this file:
//
// CCheckBoxMultiFileDialog
//   CMultiFileDialog with a checkbox
//

//CMultiFileEncodingDialog
template <UINT t_TemplateName, UINT t_CheckBoxResId>
class CCheckBoxMultiFileDialog : public CMultiFileDialog
{
protected:
	bool  m_IsChecked;
public:
	CCheckBoxMultiFileDialog(BOOL bOpenFileDialog, // TRUE for FileOpen, FALSE for FileSaveAs
		LPCTSTR lpszDefExt = NULL,
		LPCTSTR lpszFileName = NULL,
		DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		LPCTSTR lpszFilter = NULL,
		HWND hWndParent = NULL)
		: CMultiFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, hWndParent), m_IsChecked(false)
	{
		#if (_ATL_VER >= 0x0700)
		m_ofn.hInstance = ATL::_AtlBaseModule.GetResourceInstance();
		#else
		m_ofn.hInstance = _Module.GetResourceInstance();
		#endif //!(_ATL_VER >= 0x0700)
		m_ofn.lpTemplateName = MAKEINTRESOURCE(t_TemplateName);
	}
public:
	bool  IsChecked() const { return m_IsChecked; }
	void  Check(bool Checked = true) { m_IsChecked = Checked; }
protected:
	// Message map and handlers
	BEGIN_MSG_MAP(CCheckBoxMultiFileDialog)
		if ( uMsg == WM_INITDIALOG )
		{
			::SendDlgItemMessage(hWnd, t_CheckBoxResId, BM_SETCHECK, m_IsChecked ? BST_CHECKED : BST_UNCHECKED, 0);
		}
		else if ( uMsg == WM_COMMAND && t_CheckBoxResId == LOWORD(wParam) )
		{
			m_IsChecked = ::SendDlgItemMessage(hWnd, t_CheckBoxResId, BM_GETCHECK, 0, 0) == FALSE ? false : true;
			return TRUE;
		}
	    CHAIN_MSG_MAP(CMultiFileDialog)
	END_MSG_MAP()
};

#endif   // !defined(__DLGCHECKBOXMULTIFILEDIALOG_H__)
