// $Id: ProgBar.cpp 12 2005-08-14 01:42:51Z Rasqual $

// *
// *   gryff, a GRF archive management utility, src/ProgBar.cpp
// *   Copyright (C) 2003, 2004 Rasqual Twilight
// *
// *   This program is distributed in the hope that it will be useful,
// *   but WITHOUT ANY WARRANTY; without even the implied warranty of
// *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// *

// * ProgBar.cpp
// * Progress bar contained in the status bar (implementation)
// *
// *@author $Author: Rasqual $
// *@version $Revision: 12 $
// *

#include  "stdwtl.h"

#pragma hdrstop
#include  "resource.h"
#include  "ProgBar.h"


CProgStatusBar::CProgStatusBar()
{
}

CProgStatusBar::~CProgStatusBar()
{
}

/////////////////
// Status bar created: create progress bar too.
//
HWND CProgStatusBar::Create(HWND hWndParent, LPCTSTR lpstrText, DWORD dwStyle, UINT nID)
{
	if ( NULL == CMultiPaneStatusBarCtrl::Create(hWndParent, lpstrText, dwStyle, nID) )
	{
		return NULL;
	}
	RECT rect;
	this->GetRect(0, &rect);
	rect.left  += 2;
	rect.right = rect.left + 200;
	rect.top   += (rect.bottom - rect.top) / 4;
	rect.bottom -= (rect.bottom - rect.top) / 4;

	if ( !m_wndProgBar.Create(m_hWnd, rect, 0, WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN) )
	{
		return NULL;
	}
	m_wndProgBar.SetRange(0,100);
	return m_hWnd;
}

HWND CProgStatusBar::Create(HWND hWndParent, UINT nTextID, DWORD dwStyle, UINT nID)
{
	if ( NULL == CMultiPaneStatusBarCtrl::Create(hWndParent, nTextID, dwStyle, nID) )
	{
		return NULL;
	}
	int arrPanes[] = { ID_DEFAULT_PANE };
	this->SetPanes(arrPanes, sizeof(arrPanes) / sizeof(arrPanes[0]));
	RECT rect;
	this->GetRect(0, &rect);
	rect.left  += 2;
	rect.right = rect.left + 200;
	rect.top   += (rect.bottom - rect.top) / 4;
	rect.bottom -= (rect.bottom - rect.top) / 4;

//	TCHAR foo[6546];
//	wsprintf(foo, _T("%d %d %d %d"), rect.left, rect.right, rect.top, rect.bottom);
//	MessageBox(foo);

	if ( !m_wndProgBar.Create(m_hWnd, rect, 0, WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN) )
	{
		return NULL;
	}
	m_wndProgBar.SetRange(0,100);
	return m_hWnd;
}

//////////////////
// Status bar was sized: adjust size of progress bar to same as first
// pane (ready message). Note that the progress bar may or may not be
// visible.
//
LRESULT CProgStatusBar::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CMultiPaneStatusBarCtrl::OnSize(uMsg, wParam, lParam, bHandled);
	RECT rc;
	GetPaneRect(0, &rc);					  // item 0 = first pane, "ready" message
	  // If bar has been created
	if ( ::IsWindow(m_wndProgBar.m_hWnd) )
	{
		m_wndProgBar.MoveWindow(&rc, FALSE);  // move progress bar
	}
	return 0;
}

//////////////////
// Set progress bar position. pct is an integer from 0 to 100:
//
//  0 = hide progress bar and display ready message (done);
// >0 = (assemed 0-100) set progress bar position
//
// You should call this from your main frame to update progress.
//
void CProgStatusBar::OnProgress(int *pState)
{
	CProgressBarCtrl& pc = this->GetProgressCtrl();
	DWORD dwOldStyle = (UINT)::GetWindowLong(m_hWnd, GWL_STYLE);
	DWORD dwNewStyle(dwOldStyle);
	if ( *pState == 0 )
	{
		dwNewStyle &= ~WS_VISIBLE;
	}
	else
	{
		dwNewStyle |= WS_VISIBLE;
	}

	if (dwNewStyle != dwOldStyle)
	{
		// change state of hide/show
		  // FIXME: TODO: (x.x) include all info from pState
		this->SetWindowText(NULL);
		::SetWindowLong(pc.m_hWnd, GWL_STYLE, dwNewStyle);
	}

	// set progress bar position
	pc.SetPos(*pState);
	TCHAR foo[500];
	wsprintf(foo, _T("%d %%"), *pState);
	if ( ::GetAsyncKeyState(VK_CONTROL) == 0 )
	{
		MessageBox(foo, _T("GRF Loading"));
	}
}

