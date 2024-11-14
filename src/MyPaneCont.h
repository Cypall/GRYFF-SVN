// $Id: MyPaneCont.h 9 2005-08-12 17:33:08Z Rasqual $

// *
// *   gryff, a GRF archive management utility, src/MyPaneCont.h
// *   Copyright (C) 2003, 2004 Rasqual Twilight
// *
// *   This program is distributed in the hope that it will be useful,
// *   but WITHOUT ANY WARRANTY; without even the implied warranty of
// *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// *

// * MyPaneCont.h
// * Pane container
// *
// *@author $Author: Rasqual $
// *@version $Revision: 9 $
// *

#ifndef __MYPANECONT_H__
#define __MYPANECONT_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __ATLCTRLX_H__
   #error MyPaneCtrl.h requires <atlctrlx.h> to be included first
#endif

class CMyPaneContainer : public CPaneContainerImpl<CMyPaneContainer>
{
public:
    DECLARE_WND_CLASS_EX(_T("GryffPaneContainer"), 0, -1)

	void  DrawPaneTitle(CDCHandle dc);
};

#endif //__MYPANECONT_H__
