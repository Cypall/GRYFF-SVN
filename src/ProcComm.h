// $Id: ProcComm.h 9 2005-08-12 17:33:08Z Rasqual $

// *
// *   gryff, a GRF archive management utility, src/ProcComm.h
// *   Copyright (C) 2003, 2004 Rasqual Twilight
// *
// *   This program is distributed in the hope that it will be useful,
// *   but WITHOUT ANY WARRANTY; without even the implied warranty of
// *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// *

// * ProcComm.h
// * Communication in the same process and between instances
// *
// *@author $Author: Rasqual $
// *@version $Revision: 9 $
// *

#ifndef __PROCCOMM_H__
#define __PROCCOMM_H__

#define UWM_UPDATE       (WM_USER+1)
#define UWM_PROGRESS     (WM_USER+2)
#define UWM_MDIDESTROY   (WM_USER+3)
#define UWM_WORKCANCEL   (WM_USER+4)
#define UWM_WORKCOMPLETE (WM_USER+5)
#define UWM_REPACK_FINISHED (WM_USER+6)
#define UWM_REGISTER_MRU (WM_USER+7)
#define UWM_NAVIGATE_HTREE (WM_USER+8)
#define UWM_MDIFINALMESSAGE  (WM_USER+9)
#define UWM_GETMDIFRAME  (WM_USER+10)
#define UWM_GETDOCPTR    (WM_USER+11)
#define UWM_DRAGDROP     (WM_USER+12)
#define UWM_ENTER_CHILD_FOCUS  (WM_USER+13)
#define UWM_GETSINGLEPANEMODE  (WM_USER+14)
#define UWM_APPENDLISTITEM     (WM_USER+15)
#define UWM_SUBVIEWGOTFOCUS    (WM_USER+16)
#define UWM_STATUS_TEXT        (WM_USER+17)

#define LISTVIEW_ID   1
#define TREEVIEW_ID   2

struct DragDropInfo
{
	SIZE_T       lStructSize;
	HWND         hWnd;
	DWORD        dwProcessId;
};


__interface IDataObject;
struct CGrfDirectoryEntry;

struct OleDropInfo
{
	HRESULT        *phr;
	IDataObject    *pDataObject;
	CGrfDirectoryEntry *pDirent;
};


#endif //__PROCCOMM_H__
