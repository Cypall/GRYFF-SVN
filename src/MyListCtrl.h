// $Id: MyListCtrl.h 12 2005-08-14 01:42:51Z Rasqual $

// *
// *   gryff, a GRF archive management utility, src/MyListCtrl.h
// *   Copyright (C) 2003-2005 Rasqual Twilight
// *
// *   This program is distributed in the hope that it will be useful,
// *   but WITHOUT ANY WARRANTY; without even the implied warranty of
// *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// *

// * MyListCtrl.h
// * List view control definition
// *
// *@author $Author: Rasqual $
// *@version $Revision: 12 $
// *

#ifndef __MYLISTCTRL_H__
#define __MYLISTCTRL_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __ATLCTRLS_H__
   #error MyListCtrl.h requires <atlctrls.h> to be included first
#endif

#include  <utility>  // pair

class CMyImageList;
struct CGrfFileEntry;
class CGrfView;

///////////////////////////////////////////////////////////////////////////////
// Classes in this file:
//
// CListTraits
//   typedef
//
// CMyListCtrl
//   Class instantiating a frame window, MDI child of CMainWindow.
//   See  :  CMyListCtrl <atlctrls.h>
//           ListViewArrows <sieka/ListViewArrows.h>

typedef CWinTraitsOR<
LVS_SHOWSELALWAYS |
LVS_SHAREIMAGELISTS |  // because some global images are shared, the image list is destroyed in the class destructor
LVS_EDITLABELS |
LVS_REPORT  // FIXME: TODO: (1.2) Option, no need for this later
> CListTraits;

///////////////////////////////////////////////////////////////////////////////

class CMyListCtrl :
  public CWindowImpl<CMyListCtrl, CListViewCtrl, CListTraits>,
  public CFlatScrollBarImpl<CMyListCtrl>,
  public ListViewArrows<CMyListCtrl>
{
	friend CGrfView;
public:
	typedef struct tagMyListViewCreationInfo
	{
		CMyImageList     *pmiml;
		HWND             hWnd;
		DWORD            dwStyleEx;
	} MYLISTVIEWCREATIONINFO, UNALIGNED *LPMYLISTVIEWCREATIONINFO;

	typedef struct tagMyListViewAppendInfo
	{
		const CString  * pstrEntryName;
		const CGrfFileEntry * precord;
		const HTREEITEM * ptreeitem;
	} MYLISTVIEWAPPENDINFO, UNALIGNED *LPMYLISTVIEWAPPENDINFO;

	/**
		Possible sort types
	**/
	enum
	{
		SORTTYPE_TEXT_LEXICOGRAPHIC = 0,
		SORTTYPE_NUMBER_FORMATTED,
		SORTTYPE_TEXT_NATURAL,
		_SORTTYPE_FIRST = SORTTYPE_TEXT_LEXICOGRAPHIC,
		_SORTTYPE_LAST = SORTTYPE_TEXT_NATURAL
	};

public:
    DECLARE_WND_SUPERCLASS(NULL, WC_LISTVIEW)

	//////////////////////// - static methods - /////////////////////////////
public:
	/**
		Translates an extension to a type description.
		Fills the 'szTypeName' and 'iIcon' fields of the SHFILEINFO structure
		if the specified file name is a Gravity type.
		psfi is not modified otherwise.
		Returns true if the specified file name is a Gravity type.
		See: gravity_file_types (in implementation) for supported types
		Remarks: Some general types such as txt xml bmp jpg tga wav mp2 are not considered to be Gtypes
	**/
	static bool  GetGravityFileTypeName(const CString &strFileName, /* [out] */ SHFILEINFO *psfi);

	/**
		Callback (PFNLVCOMPARE) comparing two listview items.
		The custom data, lParam, is the address of a CMyListCtrl instance.
		Return <0, 0, >0
		See also: CompareItems
	**/
	static int CALLBACK  CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParam);

	////////////////// - Static Data Members - ///////////////////////
protected:
	/**
		Shared CMyImageList instance
		[sss][xxxxxx]
		First part are system imagelist images, then index-fixed known images
	**/
	static CMyImageList * static_pmiml;

	///////////////////////// - CTOR / DTOR - /////////////////////////////
public:
	/**
		Performs initialization.
		Default sort is: primary on label, secondary on type
	**/
	CMyListCtrl() : m_hWndParentView(0), m_pcurrentTree(0), m_sortOrder(0,5), m_ascending(true,true), m_styleExFlags(0), m_drawnSortArrow(false)
	{}
	/**
		Performs cleanup.
	**/
	~CMyListCtrl()
	{
	}

	///////////////////////// - data members - /////////////////////////////
protected:
	HWND                         m_hWndParentView;
	HTREEITEM                    m_pcurrentTree;
	std::pair<int,int>           m_sortOrder;
	std::pair<bool,bool>         m_ascending;

	DWORD                        m_styleExFlags;
	bool                         m_drawnSortArrow;

	///////////////////////// - Methods - /////////////////////////////
	//----- Getters -----//
public:
	int  getSortColumn() const { return m_sortOrder.first; }  // for ListViewArrows<>
	bool  isAscending() const { return m_ascending.first; }  // for ListViewArrows<>

protected:
	//----- Window message loop related handling -----//
	////----- window message handlers -----////
	/**
		Called when a new list view is created.
		The lpCreateStruct has a lpCreateParams member
		which is a pointer of to a CMyListCtrl::MYLISTVIEWCREATIONINFO structure.
		It contains information that is used to create the listview
	**/
	LRESULT  OnCreate(LPCREATESTRUCT lpCreateStruct);
	/**
		Prevent Windows from erasing the backgound, to avoid flicker when drawing the list contents
	**/
	LRESULT  OnEraseBackground(HDC);
	void  OnPaint(HDC);
	LRESULT  OnSetFocus(UINT, WPARAM, LPARAM, BOOL& bHandled);
	LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT  OnColumnClick(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled);

public:
	//----- methods: CListViewCtrl (general utils) -----//
	BOOL  FocusItem(int nIndex)
	{
		ATLASSERT(::IsWindow(m_hWnd));

		BOOL bRet = SetItemState(nIndex, LVIS_FOCUSED, LVIS_FOCUSED);
		return bRet;
	}

	int  GetFocusedIndex() const
	{
		return this->GetNextItem(-1, LVNI_ALL | LVNI_FOCUSED);
	}

	BOOL  GetFocusedItem(LPLVITEM pItem) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(pItem != NULL);
		pItem->iItem = (int)::SendMessage(m_hWnd, LVM_GETNEXTITEM, (WPARAM)-1, MAKELPARAM(LVNI_ALL | LVNI_FOCUSED, 0));
		if(pItem->iItem == -1)
			return FALSE;
		return (BOOL)::SendMessage(m_hWnd, LVM_GETITEM, 0, (LPARAM)pItem);
	}

	int  GetFirstSelectedIndex() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		  // avoid the stupid WTL assert about LVS_SINGLESEL not being set
		return this->GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	}

	BOOL  GetFirstSelectedItem(LPLVITEM pItem) const
	{
		ATLASSERT(pItem != NULL);
		pItem->iItem = this->GetFirstSelectedIndex();
		if(pItem->iItem == -1)
			return FALSE;
		return this->GetItem(pItem);
	}

	//----- methods: CListViewCtrl overloads for specific datatypes -----//
public:
	/**
		Append a file entry
	**/
	int  AppendItem(const CString &strEntryName, const CGrfFileEntry * const precord)
	{
		return AppendItem(strEntryName, precord, NULL);
	}
	/**
		Append a directory entry
	**/
	int  AppendItem(const CString &strEntryName, const HTREEITEM * const pti)
	{
		return AppendItem(strEntryName, NULL, pti);
	}
	/**
		Append a file entry xor a directory entry
		See: CGrfView::OnAppendItem_List
	**/
	int  AppendItem(const CString &strEntryName, const CGrfFileEntry * const precord, const HTREEITEM * const pti);

	/**
		Removes a directory from the view, based on its corresponding HTREEITEM.
		The lParam member of the list items is examined to determine the correct items to
		remove, if applicable.
		returns TRUE if the directory item was found and deleted.
	**/
	BOOL  Rmdir(HTREEITEM);

	//----- methods: Others -----//
protected:
	/**
		Sets size of columns
	**/
	void  Init();
	/**
		Resizes columns based on contents, never shrinks them
	**/
	void  ResizeColumns_();
	/**
		Compares two items based on list's current sorting settings
	**/
	int  CompareItems(LPARAM lParam1, LPARAM lParam2);
	/**
		Compares two items in regard to a given column
	**/
	int  CompareItemsByColumn(int  iItem1, int  iItem2, int  iColumn);

	///////////////////////// - Message loop - /////////////////////////////

protected:
    BEGIN_MSG_MAP(CMyListCtrl)
      MSG_WM_CREATE(OnCreate)
      // Note: if implementing OnDestroy(), do not forget to SetMsgHandled(false) for ListViewArrows<T>::OnDestroy() to be called (or call it directly)
	  MSG_WM_ERASEBKGND(OnEraseBackground)
	  MSG_WM_PAINT(OnPaint)
      MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
	  MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
//      REFLECTED_NOTIFY_CODE_HANDLER(LVN_KEYDOWN, OnKeyDown)
      REFLECTED_NOTIFY_CODE_HANDLER(LVN_COLUMNCLICK, OnColumnClick)
      DEFAULT_REFLECTION_HANDLER()
      CHAIN_MSG_MAP(ListViewArrows<CMyListCtrl>)
    END_MSG_MAP()

};


#endif //__MYLISTCTRL_H__
