// $Id: MyPaneCont.cpp 9 2005-08-12 17:33:08Z Rasqual $

// *
// *   gryff, a GRF archive management utility, src/MyTreeCtrl.cpp
// *   Copyright (C) 2003, 2004 Rasqual Twilight
// *
// *   This program is distributed in the hope that it will be useful,
// *   but WITHOUT ANY WARRANTY; without even the implied warranty of
// *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// *

// * MyPaneCont.cpp
// * Pane Container control definition (implementation)
// *
// *@author $Author: Rasqual $
// *@version $Revision: 9 $
// *

#include  "stdwtl.h"
#pragma hdrstop
#include  "MyPaneCont.h"


///////////////////////////////////////////////////////////////////////////////

void  CMyPaneContainer::DrawPaneTitle(CDCHandle dc)
{
RECT rect;
	this->GetClientRect(&rect);
	dc.SaveDC();
	{
	  // Paint to a memory device context to reduce screen flicker.
	CMemDC  memDC(dc.m_hDC, &rect);

	COLORREF left    = ::GetSysColor(COLOR_ACTIVECAPTION);
	COLORREF right   = ::GetSysColor(COLOR_GRADIENTACTIVECAPTION);

	COLOR16  r = (COLOR16) ((left & 0x000000FF)<<8);
	COLOR16  g = (COLOR16) (left & 0x0000FF00);
	COLOR16  b = (COLOR16) ((left & 0x00FF0000)>>8);
	COLOR16  r2 = (COLOR16) ((right & 0x000000FF)<<8);
	COLOR16  g2 = (COLOR16) (right & 0x0000FF00);
	COLOR16  b2 = (COLOR16) ((right & 0x00FF0000)>>8);

	TRIVERTEX tv[] = {
		{ rect.left,  rect.top,               r,  g,  b,  0xff},
		{ rect.right, rect.top + m_cxyHeader, r2, g2, b2, 0xff}
	};
	GRADIENT_RECT gr = { 0, 1 };

		memDC.DrawEdge(&rect, EDGE_ETCHED, BF_LEFT | BF_TOP | BF_RIGHT | BF_ADJUST);
		memDC.GradientFill(tv, 2, &gr, 1, GRADIENT_FILL_RECT_H);

		  // draw title (only for horizontal pane container)
	int  modeOld = memDC.SetBkMode(TRANSPARENT);
	COLORREF  crOld = memDC.SetTextColor(::GetSysColor(COLOR_CAPTIONTEXT));
	HFONT hFontOld = memDC.SelectFont(this->GetTitleFont());
		rect.bottom = rect.top + m_cxyHeader;
		rect.left += m_cxyTextOffset;
		rect.right -= m_cxyTextOffset;
		if ( m_tb.m_hWnd != NULL )
		{
			rect.right -= m_cxToolBar;
		}
#ifndef _WIN32_WCE
		memDC.DrawText(m_szTitle, -1, &rect, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);
#else // CE specific
		memDC.DrawText(m_szTitle, -1, &rect, DT_LEFT | DT_SINGLELINE | DT_VCENTER);
#endif //_WIN32_WCE
	}  // memDC goes out of scope
	dc.RestoreDC(-1);
}

///////////////////////////////////////////////////////////////////////////////
