// $Id: DlgWelcome.h 9 2005-08-12 17:33:08Z Rasqual $

// *
// *   gryff, a GRF archive management utility, src/DlgWelcome.h
// *   Copyright (C) 2003-2005 Rasqual Twilight
// *
// *   This program is distributed in the hope that it will be useful,
// *   but WITHOUT ANY WARRANTY; without even the implied warranty of
// *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// *

// * DlgWelcome.h
// * Wrapper for the IDD_WELCOME dialog
// *
// *@author $Author: Rasqual $
// *@version $Revision: 9 $
// *

#if !defined(__DLGWELCOME_H__)
#define __DLGWELCOME_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __ATLFRAME_H__
   #error DlgWelcome.h requires <atlframe.h> to be included first
#endif

///////////////////////////////////////////////////////////////////////////////
// Classes in this file:
//
// CWelcomeDialog
//   License notice shown on first run, or when performing a major upgrade.
//

class CWelcomeDialog : public CDialogImpl<CWelcomeDialog>
{
public:
	enum { IDD = IDD_WELCOME };

protected:
	LRESULT  OnInitDialog(UINT, WPARAM, LPARAM lParam, BOOL&)
	{
		this->showVirus();
		return TRUE;
	}

	void  showVirus()
	{
#if (_ATL_VER >= 0x0700)
		HINSTANCE  hResInstance = ATL::_AtlBaseModule.GetResourceInstance();
#else
		HINSTANCE  hResInstance = _Module.GetResourceInstance();
#endif

		HRSRC  hRSRC = ::FindResource(hResInstance, (LPCTSTR)IDR_VIRUS, _T("LEGALESE"));

		if ( hRSRC )
		{
			HGLOBAL hGlobal = ::LoadResource(hResInstance, hRSRC);
			if ( hGlobal )
			{
				LPVOID  lpData = ::LockResource(hGlobal);

				if ( lpData )
				{
					::SendMessage(::GetDlgItem(m_hWnd, IDC_LICENSE), WM_SETFONT, (WPARAM)::GetStockObject(ANSI_FIXED_FONT), MAKELPARAM(TRUE, 0));
					::SendMessageA(::GetDlgItem(m_hWnd, IDC_LICENSE), WM_SETTEXT, 0, (LPARAM)lpData);
					::FreeResource(hGlobal);
					return;
				}
				::FreeResource(hGlobal);
			}
		}
		this->EndDialog(0);
	}

protected:
	BEGIN_MSG_MAP(CWelcomeDialog)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		if ( uMsg == WM_COMMAND &&
		  (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) )
		{
			bHandled = TRUE;
			this->EndDialog(LOWORD(wParam));
			return TRUE;
		}
	END_MSG_MAP()
};

#endif   // !defined(__DLGWELCOME_H__)
