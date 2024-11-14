// $Id: ListViewArrows.h 36 2005-05-10 20:36:16Z Rasqual $
/*
* Copyright (C) 2001-2003 Jacek Sieka, j_s@telia.com
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

/// !! THIS IS A MODIFIED VERSION
///////////////////////////////////////////////////////////////////////////////
// Classes in this file:
//
// ListViewArrows<T>
//   Base class for CListViewCtrlT derived classes
//   See  :  CListViewCtrlT<TBase> <atlctrls.h>


#if !defined(AFX_LISTVIEW_ARROWS_H__07D44A33_1277_482D_AFB4_05E3473B4379__INCLUDED_)
#define AFX_LISTVIEW_ARROWS_H__07D44A33_1277_482D_AFB4_05E3473B4379__INCLUDED_

template<class T>
class ListViewArrows {
public:
	ListViewArrows() { }
	virtual ~ListViewArrows() { }

	typedef ListViewArrows<T> thisClass;

protected:
	CBitmap upArrow;
	CBitmap downArrow;

	static const int bitmapWidth = 8;
	static const int bitmapHeight = 8;

protected:
#if 1
	void rebuildArrows()
	{
		const int IDB_UP = 133, IDB_DOWN = 134;
		upArrow = (HBITMAP)::LoadImage(GetModuleHandleA("SHELL32"), MAKEINTRESOURCE(IDB_UP), IMAGE_BITMAP, 0, 0, LR_LOADMAP3DCOLORS);
		downArrow = (HBITMAP)::LoadImage(GetModuleHandleA("SHELL32"), MAKEINTRESOURCE(IDB_DOWN), IMAGE_BITMAP, 0, 0, LR_LOADMAP3DCOLORS);
	}
#else
	void rebuildArrows()
	{
		POINT pathArrowLong[9] = {{0L,7L},{7L,7L},{7L,6L},{6L,6L},{6L,4L},{5L,4L},{5L,2L},{4L,2L},{4L,0L}};
		POINT pathArrowShort[7] = {{0L,6L},{1L,6L},{1L,4L},{2L,4L},{2L,2L},{3L,2L},{3L,0L}};

		CDC dc;
		CBrushHandle brush;
		CPen penLight;
		CPen penShadow;

		const RECT rect = {0, 0, bitmapWidth, bitmapHeight};

		POINT *outerArrowPath = pathArrowLong;
		int outerArrowPathLength = sizeof(pathArrowLong) / sizeof(pathArrowLong[0]);
		POINT *innerArrowPath = pathArrowShort;
		int innerArrowPathLength = sizeof(pathArrowShort) / sizeof(pathArrowShort[0]);

		T* pThis = (T*)this;

		if(!dc.CreateCompatibleDC(pThis->GetDC()))
			return;

		if(!brush.CreateSysColorBrush(COLOR_3DFACE))
			return;

		if(!penLight.CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_3DHIGHLIGHT)))
			return;

		if(!penShadow.CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_3DSHADOW)))
			return;

		if (upArrow.IsNull())
			upArrow.CreateCompatibleBitmap(pThis->GetDC(), bitmapWidth, bitmapHeight);

		if (downArrow.IsNull())
			downArrow.CreateCompatibleBitmap(pThis->GetDC(), bitmapWidth, bitmapHeight);

		// create up arrow
		dc.SelectBitmap(upArrow);
		dc.FillRect(&rect, brush);
		dc.SelectPen(penLight);
		dc.Polyline(outerArrowPath, outerArrowPathLength);
		dc.SelectPen(penShadow);
		dc.Polyline(innerArrowPath, innerArrowPathLength);

		// create down arrow
		dc.SelectBitmap(downArrow);
		dc.FillRect(&rect, brush);
		for (int i=0; i < innerArrowPathLength; ++i)
		{
			POINT& pt = innerArrowPath[i];
			pt.x = bitmapWidth - pt.x;
			pt.y = bitmapHeight - pt.y;
		}
		dc.SelectPen(penLight);
		dc.Polyline(innerArrowPath, innerArrowPathLength);
		for (int i=0; i < outerArrowPathLength; ++i)
		{
			POINT& pt = outerArrowPath[i];
			pt.x = bitmapWidth - pt.x;
			pt.y = bitmapHeight - pt.y;
		}
		dc.SelectPen(penShadow);
		dc.Polyline(outerArrowPath, outerArrowPathLength);
	}
#endif

	void updateArrow() {
		if (upArrow.IsNull())
			return;

		T* pThis = (T*)this;
		HBITMAP bitmap = (pThis->isAscending() ? upArrow : downArrow);

		CHeaderCtrl headerCtrl = pThis->GetHeader();
		const int itemCount = headerCtrl.GetItemCount();
		for (int i=0; i < itemCount; ++i)
		{
			HDITEM item;
			item.mask = HDI_FORMAT;
			headerCtrl.GetItem(i, &item);
			item.mask = HDI_FORMAT | HDI_BITMAP;
			if (i == pThis->getSortColumn()) {
				item.fmt |= HDF_BITMAP | HDF_BITMAP_ON_RIGHT;
				item.hbm = bitmap;
			} else {
				item.fmt &= ~(HDF_BITMAP | HDF_BITMAP_ON_RIGHT);
				item.hbm = 0;
			}
			headerCtrl.SetItem(i, &item);
		}
	}

	LRESULT onCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
		rebuildArrows();
		T* pThis = (T*)this;
		_Module.AddSettingChangeNotify(pThis->m_hWnd);
		bHandled = FALSE;
		return 0;
	}

	LRESULT onDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
		T* pThis = (T*)this;
		_Module.RemoveSettingChangeNotify(pThis->m_hWnd);
		bHandled = FALSE;
		return 0;
	}

	LRESULT onSettingChange(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
		rebuildArrows();
		bHandled = FALSE;
		return 1;
	}

protected:
	BEGIN_MSG_MAP(thisClass)
		MESSAGE_HANDLER(WM_CREATE, onCreate)
		MESSAGE_HANDLER(WM_DESTROY, onDestroy)
		MESSAGE_HANDLER(WM_SETTINGCHANGE, onSettingChange)
	END_MSG_MAP()
};

#endif  // defined(AFX_LISTVIEW_ARROWS_H__07D44A33_1277_482D_AFB4_05E3473B4379__INCLUDED_)

#if 0
// Original version follows:

/**
* @file
* Id: ListViewArrows.h,v 1.2 2003/11/12 01:17:12 arnetheduck Exp
*/


template<class T>
class ListViewArrows {
public:
	ListViewArrows() { }
	virtual ~ListViewArrows() { }

	typedef ListViewArrows<T> thisClass;

	BEGIN_MSG_MAP(thisClass)
		MESSAGE_HANDLER(WM_CREATE, onCreate)
		MESSAGE_HANDLER(WM_DESTROY, onDestroy)
		MESSAGE_HANDLER(WM_SETTINGCHANGE, onSettingChange)
	END_MSG_MAP()

	void rebuildArrows()
	{
		POINT pathArrowLong[9] = {{0L,7L},{7L,7L},{7L,6L},{6L,6L},{6L,4L},{5L,4L},{5L,2L},{4L,2L},{4L,0L}};
		POINT pathArrowShort[7] = {{0L,6L},{1L,6L},{1L,4L},{2L,4L},{2L,2L},{3L,2L},{3L,0L}};

		CDC dc;
		CBrushHandle brush;
		CPen penLight;
		CPen penShadow;

		const int bitmapWidth = 8;
		const int bitmapHeight = 8;
		const RECT rect = {0, 0, bitmapWidth, bitmapHeight};

		T* pThis = (T*)this;

		if(!dc.CreateCompatibleDC(pThis->GetDC()))
			return;

		if(!brush.CreateSysColorBrush(COLOR_3DFACE))
			return;

		if(!penLight.CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_3DHIGHLIGHT)))
			return;

		if(!penShadow.CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_3DSHADOW)))
			return;

		if (upArrow.IsNull())
			upArrow.CreateCompatibleBitmap(pThis->GetDC(), bitmapWidth, bitmapHeight);

		if (downArrow.IsNull())
			downArrow.CreateCompatibleBitmap(pThis->GetDC(), bitmapWidth, bitmapHeight);

		// create up arrow
		dc.SelectBitmap(upArrow);
		dc.FillRect(&rect, brush);
		dc.SelectPen(penLight);
		dc.Polyline(pathArrowLong, sizeof(pathArrowLong)/sizeof(pathArrowLong[0]));
		dc.SelectPen(penShadow);
		dc.Polyline(pathArrowShort, sizeof(pathArrowShort)/sizeof(pathArrowShort[0]));

		// create down arrow
		dc.SelectBitmap(downArrow);
		dc.FillRect(&rect, brush);
		for (int i=0; i < sizeof(pathArrowShort)/sizeof(pathArrowShort[0]); ++i)
		{
			POINT& pt = pathArrowShort[i];
			pt.x = bitmapWidth - pt.x;
			pt.y = bitmapHeight - pt.y;
		}
		dc.SelectPen(penLight);
		dc.Polyline(pathArrowShort, sizeof(pathArrowShort)/sizeof(pathArrowShort[0]));
		for (i=0; i < sizeof(pathArrowLong)/sizeof(pathArrowLong[0]); ++i)
		{
			POINT& pt = pathArrowLong[i];
			pt.x = bitmapWidth - pt.x;
			pt.y = bitmapHeight - pt.y;
		}
		dc.SelectPen(penShadow);
		dc.Polyline(pathArrowLong, sizeof(pathArrowLong)/sizeof(pathArrowLong[0]));
	}

	void updateArrow() {
		if (upArrow.IsNull())
			return;

		T* pThis = (T*)this;
		HBITMAP bitmap = (pThis->isAscending() ? upArrow : downArrow);

		CHeaderCtrl headerCtrl = pThis->GetHeader();
		const int itemCount = headerCtrl.GetItemCount();
		for (int i=0; i < itemCount; ++i)
		{
			HDITEM item;
			item.mask = HDI_FORMAT;
			headerCtrl.GetItem(i, &item);
			item.mask = HDI_FORMAT | HDI_BITMAP;
			if (i == pThis->getSortColumn()) {
				item.fmt |= HDF_BITMAP | HDF_BITMAP_ON_RIGHT;
				item.hbm = bitmap;
			} else {
				item.fmt &= ~(HDF_BITMAP | HDF_BITMAP_ON_RIGHT);
				item.hbm = 0;
			}
			headerCtrl.SetItem(i, &item);
		}
	}

	LRESULT onCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
		rebuildArrows();
		T* pThis = (T*)this;
		_Module.AddSettingChangeNotify(pThis->m_hWnd);
		bHandled = FALSE;
		return 0;
	}

	LRESULT onDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
		T* pThis = (T*)this;
		_Module.RemoveSettingChangeNotify(pThis->m_hWnd);
		bHandled = FALSE;
		return 0;
	}

	LRESULT onSettingChange(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
		rebuildArrows();
		bHandled = FALSE;
		return 1;
	}
private:
	CBitmap upArrow;
	CBitmap downArrow;
};
#endif  // 0