// $Id: ProgBar.h 9 2005-08-12 17:33:08Z Rasqual $

// *
// *   gryff, a GRF archive management utility, src/ProgBar.cpp
// *   Copyright (C) 2003, 2004 Rasqual Twilight
// *
// *   This program is distributed in the hope that it will be useful,
// *   but WITHOUT ANY WARRANTY; without even the implied warranty of
// *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// *

// * ProgBar.cpp
// * Progress bar contained in the status bar
// *
// *@author $Author: Rasqual $
// *@version $Revision: 9 $
// *

#ifndef __PROGBAR_H__
#define __PROGBAR_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#ifndef __ATLCTRLX_H__
   #error ProgBar.h requires <atlctrlx.h> to be included first
#endif

///////////////////////////////////////////////////////////////////////////////
// Classes defined in this file:
//   CProgStatusBar
//    A status bar with a progress bar. Handles the UWM_PROGRESS user message.

class CProgStatusBar : public CMultiPaneStatusBarCtrl
{
public:
	CProgStatusBar();
	virtual ~CProgStatusBar();

public:
	  // Override
	HWND Create(HWND hWndParent, LPCTSTR lpstrText, DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | SBARS_SIZEGRIP, UINT nID = ATL_IDW_STATUS_BAR);
	HWND Create(HWND hWndParent, UINT nTextID = ATL_IDS_IDLEMESSAGE, DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | SBARS_SIZEGRIP, UINT nID = ATL_IDW_STATUS_BAR);

	CProgressBarCtrl& GetProgressCtrl()
	{
		return m_wndProgBar;
	}
	void OnProgress(int *pState);

protected:
	LRESULT  OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	CProgressBarCtrl     m_wndProgBar;

	BEGIN_MSG_MAP(CProgStatusBar)
	  MESSAGE_HANDLER(WM_SIZE, OnSize)
	END_MSG_MAP()

};

#endif  // __PROGBAR_H__
