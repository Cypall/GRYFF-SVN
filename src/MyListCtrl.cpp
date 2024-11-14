// $Id: MyListCtrl.cpp 12 2005-08-14 01:42:51Z Rasqual $

// *
// *   gryff, a GRF archive management utility, src/MyListCtrl.cpp
// *   Copyright (C) 2003-2005 Rasqual Twilight
// *
// *   This program is distributed in the hope that it will be useful,
// *   but WITHOUT ANY WARRANTY; without even the implied warranty of
// *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// *

// * MyListCtrl.cpp
// * List view control definition (implementation)
// *
// *@author $Author: Rasqual $
// *@version $Revision: 12 $
// *

#include  "stdwtl.h"
#pragma hdrstop
#include  "resource.h"
#include  "MyListCtrl.h"

#include  "ProcComm.h"
#include  "StringTraits.h"
#include  "imglist.h"
using std::pair;

CMyImageList *CMyListCtrl::static_pmiml = 0;

///////////////////////////////////////////////////////////////////////////////

// ** BEGIN local anonymous namespace
namespace {
struct GRAVITYTYPE
{
	LPCTSTR  ext;
	UINT     ids;
};

static const GRAVITYTYPE  gravity_file_types[] = {
	{ _T("spr"), IDS_SPR_TYPE_DESC },
	{ _T("act"), IDS_ACT_TYPE_DESC },
	{ _T("imf"), IDS_IMF_TYPE_DESC },
	{ _T("fna"), IDS_FNA_TYPE_DESC },
	{ _T("rsw"), IDS_RSW_TYPE_DESC },
	{ _T("gnd"), IDS_GND_TYPE_DESC },
	{ _T("gat"), IDS_GAT_TYPE_DESC },
	{ _T("rsm"), IDS_RSM_TYPE_DESC },
	{ _T("rsx"), IDS_RSX_TYPE_DESC },
	{ _T("gr2"), IDS_GR2_TYPE_DESC },
//	{ _T("txt"), IDS_TXT_TYPE_DESC },
//	{ _T("xml"), IDS_XML_TYPE_DESC },
//	{ _T("bmp"), IDS_BMP_TYPE_DESC },
//	{ _T("jpg"), IDS_JPG_TYPE_DESC },
//	{ _T("tga"), IDS_TGA_TYPE_DESC },
	{ _T("str"), IDS_STR_TYPE_DESC },
	{ _T("pal"), IDS_PAL_TYPE_DESC },
//	{ _T("wav"), IDS_WAV_TYPE_DESC },
//	{ _T("mp3"), IDS_MP3_TYPE_DESC },
	{ _T("bnk"), IDS_BNK_TYPE_DESC }
};

struct MYCOLUMNS
{
	UINT   ids;
	int    align;
	int    colWidth;
	int    minWidth;  // -1 if no special setting
	int    sortType;
};

static const MYCOLUMNS  my_columns[] = {
	{ IDS_LVC_NAME,         LVCFMT_LEFT,   LVSCW_AUTOSIZE,           120, CMyListCtrl::SORTTYPE_TEXT_NATURAL },  // LVCFMT_LEFT is mandatory for column 0
	{ IDS_LVC_SIZE,         LVCFMT_RIGHT,  LVSCW_AUTOSIZE_USEHEADER,  -1, CMyListCtrl::SORTTYPE_NUMBER_FORMATTED },
	{ IDS_LVC_PACKEDSIZE,   LVCFMT_RIGHT,  LVSCW_AUTOSIZE_USEHEADER,  -1, CMyListCtrl::SORTTYPE_NUMBER_FORMATTED },
	{ IDS_LVC_PACKRATIO,    LVCFMT_RIGHT,  LVSCW_AUTOSIZE_USEHEADER,  -1, CMyListCtrl::SORTTYPE_NUMBER_FORMATTED },
	{ IDS_LVC_TYPE,         LVCFMT_LEFT,   LVSCW_AUTOSIZE_USEHEADER,  -1, CMyListCtrl::SORTTYPE_TEXT_LEXICOGRAPHIC },
	{ IDS_LVC_ORIGIN,       LVCFMT_CENTER, LVSCW_AUTOSIZE_USEHEADER,  -1, CMyListCtrl::SORTTYPE_TEXT_LEXICOGRAPHIC },
	{ IDS_LVC_RESOURCENAME, LVCFMT_LEFT,   LVSCW_AUTOSIZE_USEHEADER,  -1, CMyListCtrl::SORTTYPE_TEXT_LEXICOGRAPHIC },
};

///////////////////////////////////////////////////////////////////////////////
}
// ** END local anonymous namespace


///////////////////////////////////////////////////////////////////////////////

// public static
bool  CMyListCtrl::GetGravityFileTypeName(const CString &strFileName, SHFILEINFO *psfi)
{
static TCHAR  filetype_name[sizeof(gravity_file_types) / sizeof(gravity_file_types[0])][sizeof(psfi->szTypeName) / sizeof(psfi->szTypeName[0])];
static bool  initialized = false;

	if ( !initialized )
	{
		for ( unsigned int i = 0; i < sizeof(filetype_name) / sizeof(filetype_name[0]); ++i )
		{
			#if (_ATL_VER >= 0x0700)
			::LoadString(ATL::_AtlBaseModule.GetResourceInstance(), gravity_file_types[i].ids, filetype_name[i], sizeof(psfi->szTypeName) / sizeof(psfi->szTypeName[0]));
			#else //!(_ATL_VER >= 0x0700)
			::LoadString(_Module.GetResourceInstance(), gravity_file_types[i].ids, filetype_name[i], sizeof(psfi->szTypeName) / sizeof(psfi->szTypeName[0]));
			#endif //!(_ATL_VER >= 0x0700)
		}
		initialized = true;
	}

int  dotIndex = strFileName.ReverseFind(_T('.'));
	if ( dotIndex != -1 )
	{
	CString  ext(strFileName.Right(strFileName.GetLength() - dotIndex - 1));
		ext.MakeLower();
		for ( unsigned int i = 0; i < sizeof(filetype_name) / sizeof(filetype_name[0]); ++i )
		{
			if ( ext == gravity_file_types[i].ext )
			{
				lstrcpyn(psfi->szTypeName, filetype_name[i], sizeof(psfi->szTypeName) / sizeof(psfi->szTypeName[0]));
				// FIXME: TODO: (1.x) Customize associated icon for GravFileTypes
				psfi->iIcon = static_pmiml->m_idefaultFile;
				return true;
			}
		}
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////

// public static
int CALLBACK  CMyListCtrl::CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParam)
{
CMyListCtrl * pListView = reinterpret_cast<CMyListCtrl*>(lParam);
	if ( pListView != 0 )
	{
		return pListView->CompareItems(lParam1, lParam2);
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT  CMyListCtrl::OnCreate(LPCREATESTRUCT lpcs)
{
LPMYLISTVIEWCREATIONINFO  pListViewCreationInfo = reinterpret_cast<LPMYLISTVIEWCREATIONINFO>(lpcs->lpCreateParams);
	ATLASSERT(pListViewCreationInfo);
	if ( static_pmiml == 0 )
	{
		static_pmiml = pListViewCreationInfo->pmiml;
	}
	m_hWndParentView = pListViewCreationInfo->hWnd;
	m_styleExFlags = pListViewCreationInfo->dwStyleEx;

	m_pcurrentTree = 0;

	SetMsgHandled(false);  // Chaining base methods, if any.
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT  CMyListCtrl::OnEraseBackground(HDC)
{
	  // Prevent Windows from erasing the bg
	return 1;
}

///////////////////////////////////////////////////////////////////////////////

void  CMyListCtrl::OnPaint(HDC hDC)
{
RECT rect;
	if ( this->GetUpdateRect(&rect, FALSE) )
	{
	CPaintDC  dc(m_hWnd);

	  // Paint to a memory device context to reduce screen flicker.
	CMemDC  memDC(dc.m_hDC, &rect);

//	RECT  headerRect;
	CHeaderCtrl  header = this->GetHeader();
		if ( !m_drawnSortArrow )
		{
			header.SetBitmapMargin(ListViewArrows<CMyListCtrl>::bitmapWidth);
			this->updateArrow();
			m_drawnSortArrow = true;
		}
//		header.GetWindowRect(&headerRect);
//		this->ScreenToClient(&headerRect);
//		dc.ExcludeClipRect(&headerRect);

	RECT  clip;
		memDC.GetClipBox(&clip);
		memDC.FillSolidRect(&clip, ::GetSysColor(COLOR_WINDOW));
		  // Let the window do its default painting...
	    this->DefWindowProc(WM_PAINT, (WPARAM)memDC.m_hDC, 0);
	    // When memDC goes out of scope, it paints back into the original DC
	}
}

///////////////////////////////////////////////////////////////////////////////

LRESULT  CMyListCtrl::OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	::SendMessage(m_hWndParentView, UWM_SUBVIEWGOTFOCUS, LISTVIEW_ID, 0);
	bHandled = FALSE;
	return 1;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT CMyListCtrl::OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	::PostMessage(m_hWndParentView, uMsg, wParam, lParam);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT  CMyListCtrl::OnColumnClick(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
{
    LPNMLISTVIEW pnmlv = reinterpret_cast<LPNMLISTVIEW>(pnmh);
	if ( pnmlv->iSubItem == m_sortOrder.first )
	{
		m_ascending.first = !m_ascending.first;
	}
//	else if ( pnmlv->iSubItem == m_sortOrder.second )
//	{
//		m_sortOrder = pair<int,int>(m_sortOrder.second, m_sortOrder.first);
//		m_ascending = pair<bool,bool>(m_ascending.second, m_ascending.first);
//	}
	else
	{
		m_sortOrder = pair<int,int>(pnmlv->iSubItem, m_sortOrder.first);
		m_ascending = pair<bool,bool>(true, m_ascending.first);
	}
	this->SortItemsEx(/*PFNLVCOMPARE*/&CMyListCtrl::CompareFunc, reinterpret_cast<LPARAM>(this));
	  // ListViewArrows<T>::updateArrow()
	this->updateArrow();
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

int CMyListCtrl::AppendItem(const CString &strEntryName, const CGrfFileEntry * const precord, const HTREEITEM * const pti)
{
MYLISTVIEWAPPENDINFO appendInfo = { &strEntryName, precord, pti };
	return (int)::SendMessage(m_hWndParentView, UWM_APPENDLISTITEM, 0, reinterpret_cast<LPARAM>(&appendInfo));
}

///////////////////////////////////////////////////////////////////////////////

BOOL  CMyListCtrl::Rmdir(HTREEITEM h)
{
	LVFINDINFO fi = {0};
	  // Find items bearing the searched tree item handle
	fi.flags = LVFI_PARAM;
	fi.lParam = reinterpret_cast<LPARAM>(h);
	int iItem = this->FindItem(&fi, -1);
	if ( iItem != -1 )
	{
		return this->DeleteItem(iItem);
	}
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////

void CMyListCtrl::Init()
{
	this->SetExtendedListViewStyle(WS_EX_CLIENTEDGE, m_styleExFlags);
//	if ( m_styleExFlags & LVS_EX_FLATSB )
	{
		this->FlatSB_Initialize();
	}

CString sColumnCaption;
	for ( int i = 0; i < sizeof(my_columns) / sizeof(my_columns[0]); ++i )
	{
		sColumnCaption.LoadString(my_columns[i].ids);
		  // FIXME: Set column order according to settings
		this->InsertColumn( i, sColumnCaption, my_columns[i].align, 0, i );
		if ( my_columns[i].minWidth != -1 )
		{
			this->SetColumnWidth( i, my_columns[i].minWidth );
		}
	}
	for ( int i = 0; i < sizeof(my_columns) / sizeof(my_columns[0]); ++i )
	{
		if ( my_columns[i].minWidth == -1 )
		{
			this->SetColumnWidth( i, my_columns[i].colWidth );
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

void  CMyListCtrl::ResizeColumns_()
{
	for ( int i = 0; i < sizeof(my_columns) / sizeof(my_columns[0]); ++i )
	{
		if ( my_columns[i].minWidth != -1 )
		{
			  // Attempt to resize to colWidth, fallback if not large enough
			this->SetColumnWidth( i, my_columns[i].colWidth );
			if ( this->GetColumnWidth(i) < my_columns[i].minWidth )
			{
				this->SetColumnWidth( i, my_columns[i].minWidth );
			}
		}
		else
		{
		const int  old_width = this->GetColumnWidth( i );
			this->SetColumnWidth( i, my_columns[i].colWidth );
			if ( this->GetColumnWidth(i) < old_width )
			{
				this->SetColumnWidth( i, old_width );
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

int  CMyListCtrl::CompareItems(LPARAM lParam1, LPARAM lParam2)
{
int  iItem1 = static_cast<int>(lParam1),
     iItem2 = static_cast<int>(lParam2);
int  nResult = CMyListCtrl::CompareItemsByColumn(iItem1, iItem2, m_sortOrder.first);
	if ( nResult == 0 )
	{
		nResult = CMyListCtrl::CompareItemsByColumn(iItem1, iItem2, m_sortOrder.second);
		if ( !m_ascending.second )
		{
			nResult = -nResult;
		}
	}
	else
	{
		if ( !m_ascending.first )
		{
			nResult = -nResult;
		}
	}
	return nResult;
}

///////////////////////////////////////////////////////////////////////////////

int  CMyListCtrl::CompareItemsByColumn(int  iItem1, int  iItem2, int  iColumn)
{
DWORD_PTR dwptr1 = GetItemData(iItem1), dwptr2 = GetItemData(iItem2);
	// One and only one is a folder (they are placed at top in ascending mode, regardless of sorting)
	if ( (dwptr1 != 0 && dwptr2 == 0) || (dwptr1 == 0 && dwptr2 != 0) )
	{
		return dwptr1 != 0 ? -1 : 1;
	}

CString  txt1, txt2;
	this->GetItemText(iItem1, iColumn, txt1);
	this->GetItemText(iItem2, iColumn, txt2);

	switch ( my_columns[iColumn].sortType )
	{
	case CMyListCtrl::SORTTYPE_NUMBER_FORMATTED:  // IDS_LVC_SIZE, IDS_LVC_PACKEDSIZE, IDS_LVC_PACKRATIO
		{
			if ( txt1.GetLength() > txt2.GetLength() )
			{
				return 1;
			}
			if ( txt1.GetLength() < txt2.GetLength())
			{
				return -1;
			}
			// Same length : fallback to usual lexicographical sort order
			break;
		}
	case CMyListCtrl::SORTTYPE_TEXT_NATURAL:
		{
			CompareHelper::CompareNaturalCStringsClass helper;
			return helper(txt1, txt2);
		}
	case CMyListCtrl::SORTTYPE_TEXT_LEXICOGRAPHIC:
		break;
	}

	return txt1.CompareNoCase(txt2);
}

///////////////////////////////////////////////////////////////////////////////

