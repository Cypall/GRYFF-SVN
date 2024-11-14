// $Id: stdwtl.h 9 2005-08-12 17:33:08Z Rasqual $

// *
// *   gryff, a GRF archive management utility, src/stdwtl.h
// *   Copyright (C) 2003, 2004 Rasqual Twilight
// *
// *   This program is distributed in the hope that it will be useful,
// *   but WITHOUT ANY WARRANTY; without even the implied warranty of
// *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// *

// * stdwtl.h
// * include file for standard system include files,
// * or project specific include files that are used frequently, but
// * are changed infrequently
// *
// *@author $Author: Rasqual $
// *@version $Revision: 9 $
// *


#pragma once

// Change these values to use different versions
#define WINVER		0x0500
#define _WIN32_WINNT	0x0500
#define _WIN32_IE	0x0500
#define _RICHEDIT_VER	0x0200

#include  <atlcrack.h>

#include  <atlbase.h>
#include  <atltypes.h>
#include  <atlcoll.h>
#if defined(_WTL_NO_CSTRING)
# include  <atlstr.h>
#endif
#include  <atlapp.h>

#define  _WTL_MDIWINDOWMENU_TEXT  ::GetWindowMenuLabel()

extern CAppModule _Module;

#include  <atlwin.h>
#include  <atlmisc.h>
extern CString          strGryffRegKey;
extern CString          strFilter;
extern CString          strTypeFilter;
// Forward declaration of global function
CString  GetWindowMenuLabel();

#include  <atlframe.h>
#include  <atlctrls.h>
#include  <atldlgs.h>
#include  <atlctrlw.h>
#include  <atlctrlx.h>

#include  <atlsplit.h>
#include  <viksoe/atlgdix.h> // some classes have been merged into trunk since WTL 7.5 build 5002

#include  <shellapi.h>  // ShellExecute() when WIN32_LEAN_AND_MEAN, CommandLineToArgvW()
#include  <process.h>  // threading

#include  <CommCtrl.h>
#if !defined(LV_VIEW_ICON)
# define LV_VIEW_ICON        0x0000
# define LV_VIEW_DETAILS     0x0001
# define LV_VIEW_SMALLICON   0x0002
# define LV_VIEW_LIST        0x0003
# define LV_VIEW_TILE        0x0004
#endif

#include  <gen_types.h>

#include  <MgrDoc.h>
#include  <TplDoc.h>

#include  <steele/fromhandle.h>
#include  <sieka/ListViewArrows.h>
#include  <codeproject/win_adapter.h>
