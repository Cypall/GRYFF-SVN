// $Id: VwGrf.cpp 12 2005-08-14 01:42:51Z Rasqual $

// *
// *   gryff, a GRF archive management utility, src/VwGrf.cpp
// *   Copyright (C) 2003-2005 Rasqual Twilight
// *
// *   This program is distributed in the hope that it will be useful,
// *   but WITHOUT ANY WARRANTY; without even the implied warranty of
// *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// *

// * VwGrf.cpp
// * MDI child Frame's associated view definition (implementation)
// *
// *@author $Author: Rasqual $
// *@version $Revision: 12 $
// *

#include  "stdwtl.h"
#include  <codeproject/filedialogfilter.h>

#pragma hdrstop
#include  "resource.h"
#include  "VwGrf.h"

#include  "MyPaneCont.h"
#include  "MyTreeCtrl.h"
#include  "MyListCtrl.h"

#include  "imglist.h"

#include  "ole_impl.h"

#define NO_OFN_DIALOG
#include  "fnmanip.h"


using std::pair;

extern CMyImageList *g_pImageList;
extern UINT  UCF_GRYFF_ENTRIES;
extern UINT  UCF_GRYFF_ENTRIES_INTERNAL;
extern LPCTSTR szTempArchivePrefix;

const int  MENU_CONTEXT__LV_BG    = 0;
const int  MENU_CONTEXT__LV_ITEM  = 1;
const int  MENU_CONTEXT__TV_BG    = 2;
const int  MENU_CONTEXT__TV_ITEM  = 3;

///////////////////////////////////////////////////////////////////////////////

CGrfView::CGrfView() : m_pDoc(0), m_pwndLeft(new CMyPaneContainer), m_pwndLeftSub(new CMyTreeCtrl), m_pwndRight(new CMyListCtrl), m_ListIsLastFocused(true), m_MemorizedSelection(0)
{
}

///////////////////////////////////////////////////////////////////////////////

CGrfView::~CGrfView()
{
	// Not in a SDI, document is not going to be reused (-> OnDeleteContents impl.)
	if ( m_pwndLeftSub )
	{
		delete m_pwndLeftSub;
	}
	if ( m_pwndLeft )
	{
		delete m_pwndLeft;
	}
	if ( m_pwndRight )
	{
		delete m_pwndRight;
	}
}

///////////////////////////////////////////////////////////////////////////////

BOOL CGrfView::PreTranslateMessage(MSG* pMsg)
{
	  // Forward to active frame, if applicable
	HWND hWnd = ::GetParent(m_hWnd);
	if ( hWnd != NULL )
	{
		return (BOOL)::SendMessage(hWnd, WM_FORWARDMSG, 0, (LPARAM)pMsg);
	}
	return FALSE;  // not translated
}

///////////////////////////////////////////////////////////////////////////////

LRESULT CGrfView::OnCreate(LPCREATESTRUCT lpcs)
{
	CCreateContext<CGrfDoc, CGrfView> * pContext =
	 reinterpret_cast< CCreateContext<CGrfDoc, CGrfView> * >(lpcs->lpCreateParams);

	m_pDoc = pContext->m_pCurrentDoc;

	const DWORD dwSplitStyle = WS_CHILD | WS_VISIBLE |
						WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
				dwSplitExStyle = WS_EX_TOOLWINDOW;

	m_wndVertSplit.Create ( *this, rcDefault, NULL,
						dwSplitStyle, dwSplitExStyle );

	const DWORD dwTreeViewStyle = WS_CHILD | WS_VISIBLE |
		  WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
				dwTreeViewStyleEx = WS_EX_LEFT | WS_EX_RIGHTSCROLLBAR;

	  // Create the left pane
	m_pwndLeft->Create(m_wndVertSplit, IDS_PANE_TITLE);
	RECT rc;
	m_pwndLeft->GetClientRect(&rc);

CMyTreeCtrl::GRFTREEVIEWCREATIONINFO gtvci = { 0 };
CMyListCtrl::MYLISTVIEWCREATIONINFO glvci = { 0 };
	gtvci.hWnd = glvci.hWnd = this->m_hWnd;
	gtvci.pmiml = glvci.pmiml = ::g_pImageList;
	  // Options
	glvci.dwStyleEx = LVS_EX_REGIONAL | LVS_EX_FLATSB | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP;

	  // Create the treeview, and set it as the left pane's client
	m_pwndLeftSub->Create(m_pwndLeft->m_hWnd, rc, 0, dwTreeViewStyle, dwTreeViewStyleEx, 0U, reinterpret_cast<LPVOID>(&gtvci));
	m_pwndLeft->SetClient(m_pwndLeftSub->m_hWnd);

	  // Create the listview in the splitter's right part
	m_pwndRight->Create(m_wndVertSplit, rcDefault, 0, 0, 0, 0U, reinterpret_cast<LPVOID>(&glvci));

	  // Set up the splitter panes
	m_wndVertSplit.SetSplitterPanes( m_pwndLeft->m_hWnd, m_pwndRight->m_hWnd );

	  // Set the splitter as the client area window, and resize
	  // the splitter to match the frame size.
	m_hWndClient = m_wndVertSplit;
	this->UpdateLayout();

	  // Position the splitter bar.
	m_wndVertSplit.SetSplitterPos( 200 );

	  // how the splitter position changes when the parent window is resized
	m_wndVertSplit.SetSplitterExtendedStyle(0);

	this->GetTreeViewPtr()->Init();
	this->GetListViewPtr()->Init();

	SetMsgHandled(false);  // Chaining base methods, if any.
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

void CGrfView::OnDestroy()
{
	  // Clear the tree and list views
	this->GetTreeViewPtr()->DeleteAllItems();
	this->GetListViewPtr()->DeleteAllItems();
	this->SetMsgHandled(FALSE);
}

///////////////////////////////////////////////////////////////////////////////

void CGrfView::OnFinalMessage(HWND /*hWnd*/)
{
	m_pDocument->RemoveView(this);  // notify the view owner
	delete this;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT  CGrfView::OnEraseBackground(HDC)
{
	  // Prevent Windows from erasing the bg
	return 1;
}

///////////////////////////////////////////////////////////////////////////////

void  CGrfView::OnPaint(HDC)
{
	SetMsgHandled(FALSE);
}

///////////////////////////////////////////////////////////////////////////////

int  CGrfView::GetViewTypeId() const
{
#if _WIN32_WINNT >= 0x501
	// FIXME: add check for Win2k
	DWORD  viewType = ListView_GetView(this->GetListViewPtr()->m_hWnd);
	if ( viewType == LV_VIEW_TILE )
	{
		return IDM_VIEW_TYPE_TILE;
	}
#endif
	const int ListViewStyle = this->GetListViewPtr()->GetStyle() & LVS_TYPEMASK;
	int nRet = 0;
	switch ( ListViewStyle )
	{
		case LV_VIEW_ICON:  nRet = IDM_VIEW_TYPE_ICONS; break;
		case LV_VIEW_SMALLICON: break;
		case LV_VIEW_LIST:  nRet = IDM_VIEW_TYPE_LIST; break;
		case LV_VIEW_DETAILS:  nRet = IDM_VIEW_TYPE_DETAILS; break;
	}
	return  nRet;
}

///////////////////////////////////////////////////////////////////////////////

/**
	Returns whether there is a selection.
**/
bool  CGrfView::CanPerformOpen() const
{
	if ( m_ListIsLastFocused )
	{
		return this->GetListViewPtr()->GetSelectedCount() == 1;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////

/**
	Returns whether selection may be renamed.
**/
bool  CGrfView::CanPerformRename() const
{
	if ( m_ListIsLastFocused )
	{
		return this->GetListViewPtr()->GetSelectedCount() == 1;
	}
	return const_cast<CMyTreeCtrl*>(this->GetTreeViewPtr())->GetSelectedItem() != const_cast<CMyTreeCtrl*>(this->GetTreeViewPtr())->GetRootItem();
}

///////////////////////////////////////////////////////////////////////////////
/**
	Returns whether selection may be deleted.
**/
bool  CGrfView::CanPerformDelete() const
{
	if ( m_ListIsLastFocused )
	{
		return this->GetListViewPtr()->GetSelectedCount() > 0;
	}
	return const_cast<CMyTreeCtrl*>(this->GetTreeViewPtr())->GetSelectedItem() != const_cast<CMyTreeCtrl*>(this->GetTreeViewPtr())->GetRootItem();
}

///////////////////////////////////////////////////////////////////////////////

void  CGrfView::OnUpdate(CGrfDocView* pSender, LPARAM Hint, LPVOID pHint)
{
	if ( pSender != this )
	{
		switch ( Hint )
		{
		case HINT_UPDATE_DOCUMENT_PATH:
			{
				// Called by UpdateAllViews().
				::PostMessage(this->GetParentFrame(), UWM_UPDATE, /*wParam*/Hint, reinterpret_cast<LPARAM>(pHint));
				CTreeItem rootItem = this->GetTreeViewPtr()->GetRootItem();
				rootItem.SetText((LPCTSTR)m_pDoc->GetName());
				return;
			}
			break;
		case HINT_UPDATE_DOCUMENT_REFRESH:
			{
				::PostMessage(this->GetParentFrame(), UWM_UPDATE, /*wParam*/Hint, reinterpret_cast<LPARAM>(pHint));
				this->GetTreeViewPtr()->DeleteAllItems();
				this->GetListViewPtr()->DeleteAllItems();
				CString *dir = reinterpret_cast<CString *>(pHint);
				CString tmp;
				if ( !dir )
				{
					tmp = this->GetCwd();
					dir = &tmp;
				}
				_OnUpdateDefault(dir);
			}
			break;
		case HINT_DOCUMENT_SERIALIZE_STATE:
			{
				::PostMessage(this->GetParentFrame(), UWM_PROGRESS, reinterpret_cast<WPARAM>(pHint), /*lParam*/Hint);
				return;
			}
			break;
		case HINT_DOCUMENT_FILES_ADDED:
			{
				_OnUpdateAddedFiles(reinterpret_cast<CAtlList< pair<uint8_t, CString> > *>(pHint));
			}
			break;
		case HINT_DOCUMENT_DIRS_ADDED:
			{
				_OnUpdateAddedDirectories(reinterpret_cast<CAtlList< pair<uint8_t, CString> > *>(pHint));
			}
			break;
		case HINT_DOCUMENT_FILES_AND_DIRS_ADDED:
			{
				_OnUpdateAddedFilesDirectories(reinterpret_cast<CAtlList< pair<uint8_t, CString> > *>(pHint));
			}
			break;
			// FIXME: TODO: *REMOVED hints handlers?
		default:
			::PostMessage(this->GetParentFrame(), UWM_UPDATE, /*wParam*/Hint, reinterpret_cast<LPARAM>(pHint));
			this->GetTreeViewPtr()->DeleteAllItems();
			this->GetListViewPtr()->DeleteAllItems();
			_OnUpdateDefault();
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

void CGrfView::_OnUpdateDefault(CString *pChDir)
{
	CTreeItem rootItem( GetTreeViewPtr()->InsertRoot(m_pDoc->GetName()) );
	for ( POSITION pos = m_pDoc->GetDirMapPtr()->GetHeadPosition();
	      pos;
	      m_pDoc->GetDirMapPtr()->GetNext(pos) )
	{
		CGrfDirectoryEntry *pdirent = m_pDoc->GetDirMapPtr()->GetValueAt(pos);

		CTreeItem subFolderTI( this->GetTreeViewPtr()->MkdirRecursive(rootItem, pdirent->dir) );
		subFolderTI.SetData(reinterpret_cast<DWORD_PTR>(pdirent));
	}
	this->GetTreeViewPtr()->SelectItem(0);

	  // Select specified directory
	if ( pChDir != 0 )
	{
		CTreeItem chDirTI( this->GetTreeViewPtr()->GetDir(*pChDir) );
		if ( chDirTI.m_hTreeItem )
		{
			chDirTI.Select();
//			this->GetTreeViewPtr()->PerformSelChange(chDirTI);
		}
	}

	  // Make sure an item is selected
	if ( GetTreeViewPtr()->GetSelectedItem() == 0 )
	{
	// Non-trunk patch: select root data subfolder if exists
	CTreeItem child_data( this->GetTreeViewPtr()->GetDir(_T("data")) );
		if ( true == true && child_data.m_hTreeItem != 0 )
		{
			child_data.Select();
		}
		else
		{
			rootItem.Select();
		}
		m_ListIsLastFocused = true;
	}
}

///////////////////////////////////////////////////////////////////////////////
// FIXME: TODO: (1.5) this part remains untested.

void CGrfView::_OnUpdateAddedFiles(CAtlList< pair<uint8_t, CString> > *pAddedFiles)
{
	CString dn, fn;
	POSITION pos = pAddedFiles->GetHeadPosition();
	while ( pos )
	{
		::BreakPath(pAddedFiles->GetNext(pos).second, dn, fn);
		if ( GetCwd() == dn )
		{
			CGrfDirectoryEntry *pdirent;
			CGrfFileEntry *precord;

			ATLVERIFY(m_pDoc->GetDirMapPtr()->Lookup(dn, pdirent));
			pdirent->files.Lookup(fn, precord);
			GetListViewPtr()->AppendItem(fn, precord);
		}
	}

}

///////////////////////////////////////////////////////////////////////////////
// FIXME: TODO: (1.5) this part remains untested.

void CGrfView::_OnUpdateAddedDirectories(CAtlList< pair<uint8_t, CString> > *pAddedFiles)
{
}

///////////////////////////////////////////////////////////////////////////////

void CGrfView::_OnUpdateAddedFilesDirectories(CAtlList< pair<uint8_t, CString> > *pAddedFiles)
{
}

///////////////////////////////////////////////////////////////////////////////

LRESULT CGrfView::OnSize(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
	this->UpdateLayout();
	bHandled = FALSE;
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT  CGrfView::OnNavigateHtree(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	ATLASSERT(lParam != 0);
bool  SetListFocus(static_cast<BOOL>(wParam) == TRUE);
CTreeItem  entry(reinterpret_cast<HTREEITEM>(lParam), this->GetTreeViewPtr());
	Navigate(entry, SetListFocus);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// CGrfView::UWM_GETDOCPTR registered message handler
LRESULT  CGrfView::OnGetDocumentPtr(UINT, WPARAM, LPARAM, BOOL&)
{
	return reinterpret_cast<LRESULT>(m_pDoc);
}

///////////////////////////////////////////////////////////////////////////////

LRESULT CGrfView::OnDoubleClick(LPNMHDR pnmh)
{
	if ( pnmh->hwndFrom == this->GetListViewPtr()->m_hWnd )
	{
		return this->_OnLVDoubleClick(pnmh);
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT  CGrfView::OnRClick(LPNMHDR pnmh)
{
	if ( pnmh->hwndFrom == this->GetTreeViewPtr()->m_hWnd )
	{
		return this->_OnTVContextMenu(pnmh->hwndFrom, CPoint(::GetMessagePos()));
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT  CGrfView::OnKeyDown(LPNMHDR pnmh)
{
	// According to the headers NMTVKEYDOWN and NMLVKEYDOWN have
	// the exact same structure, but this may change in the future.
	// Watch this.
	LPNMLVKEYDOWN pnkd = reinterpret_cast<LPNMLVKEYDOWN>(pnmh);
	if ( pnkd->wVKey == VK_TAB )
	{
		if ( m_ListIsLastFocused )
		{
			#ifdef _DEBUG
			::OutputDebugString(_T("Focus List->Tree"));
			this->GetTreeViewPtr()->SetFocus();
			#endif
		}
		else
		{
			#ifdef _DEBUG
			::OutputDebugString(_T("Focus Tree->List"));
			#endif
			this->GetListViewPtr()->SetFocus();
		}
		this->SetMsgHandled(TRUE);
		return 0;
	}

	if ( m_ListIsLastFocused )
	{
		switch ( pnkd->wVKey )
		{
		case VK_RETURN:
			{
				if ( ::GetAsyncKeyState(VK_MENU) > 0 )
				{
					// FIXME: TODO: (1.9) Properties
				}
				else
				{
					// Get the current activated item
					LRESULT lResult = this->GetListViewPtr()->GetSelectedCount();
					if ( lResult == 1 )
					{
						this->SetMsgHandled(TRUE);
						// Ctrl+Return = Extract
						return this->OnEditOpen(::GetAsyncKeyState(VK_CONTROL) != 0);
					}
					return this->OnEditOpen(true);
				}
			}
			break;
		case VK_BACK:
			{
				CString  dn, fn;
				::BreakPath(this->GetCwd(), dn, fn);
				CTreeItem chDirTI( this->GetTreeViewPtr()->GetDir(dn) );
				if ( chDirTI.m_hTreeItem )
				{
					chDirTI.Select();
					m_ListIsLastFocused = true;
					this->SetMsgHandled(TRUE);
					return 0;
				}
			}
			break;
		}
	}
	this->SetMsgHandled(FALSE);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
LRESULT  CGrfView::OnEditExtract()
{
	return this->OnEditOpen(true);
}

///////////////////////////////////////////////////////////////////////////////

LRESULT  CGrfView::OnEditOpen(bool PromptExtractPlace)
{
CGrfCache  cache;
CGrfDirectoryEntry *pdirent;
CGrfFileEntry *precord;

	if ( m_ListIsLastFocused )
	{
	CMyListCtrl * pListCtrl = this->GetListViewPtr();
	int  iSelected = pListCtrl->GetNextItem(-1, LVNI_SELECTED);
		if ( iSelected != -1 )
		{
		DWORD_PTR  dwptr = this->GetListViewPtr()->GetItemData(iSelected);
			  // Directory and not extracting
			if ( dwptr != 0 && !PromptExtractPlace )
			{
			CTreeItem  entry(reinterpret_cast<HTREEITEM>(dwptr), this->GetTreeViewPtr());
				entry.Select();
				m_ListIsLastFocused = true;
			}
			else
			{
				  // Open System Viewer
			CString  strCwd(this->GetCwd()), strItemText;
				// Get the item text
				pListCtrl->GetItemText(iSelected, 0, strItemText);
			TCHAR  TempPath[MAX_PATH];
				if ( pListCtrl->GetSelectedCount() == 1 && dwptr == 0 )
				{
					m_pDoc->GetDirMapPtr()->Lookup(strCwd, pdirent);
					pdirent->files.Lookup(strItemText, precord);
					if ( precord->origin == CGrfFileEntry::FROMFS )
					{
						CWaitCursor w(true, IDC_APPSTARTING, true);
						// Run from disk
						::CloseHandle(StartApplication(precord->realpath, m_hWnd));
					}
					else
					{
						CString  strRealPath;
						::MultiByteToWideChar(0x3B5, 0, precord->in_grf_path, -1, strRealPath.GetBufferSetLength(CGrfFileEntry::GRF_MAX_PATH), CGrfFileEntry::GRF_MAX_PATH);
						strRealPath.ReleaseBuffer();

						if ( PromptExtractPlace )
						{
							CString  strFilter;
							strFilter.Format(_T("%s|*.*|"), CString(MAKEINTRESOURCE(IDS_FILTER_ALL_FILES)));
							CFileDialogFilter  filter(strFilter);
							TCHAR  StartDirPath[MAX_PATH];

							LPMALLOC pMalloc;
							::SHGetMalloc(&pMalloc);

							LPITEMIDLIST  pidlDocFiles;
							HRESULT hr = ::SHGetFolderLocation(NULL,
								CSIDL_PERSONAL,
								NULL,
								0,
								&pidlDocFiles);
							hr = ::SHGetPathFromIDList(pidlDocFiles, StartDirPath);
							CString  dn, strFileName;
							::BreakPath(strRealPath, dn, strFileName);

							::PathCombine(TempPath, StartDirPath, strFileName);
							pMalloc->Free(pidlDocFiles);
							pMalloc->Release();
							// Extract one
							CFileDialog saveAsDialog(FALSE,
								NULL, TempPath, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, filter);

							if ( IDOK != saveAsDialog.DoModal() )
							{
								return 0;
							}
							lstrcpyn(TempPath, saveAsDialog.m_szFileName, MAX_PATH);
						}
						else
						{
							CTempDirectory tmpdir;
							tmpdir.Create(::szTempArchivePrefix);
							tmpdir.DisableDeleting();
							::PathCombine(TempPath, tmpdir.GetPath(), strRealPath);
							CString strParent, fn;
							::BreakPath(CString(TempPath), strParent, fn);
							::SHCreateDirectoryEx(NULL, strParent, NULL);
						}
						CWaitCursor w(true, IDC_APPSTARTING, true);
						openkore::Grf * pGrf;
						try
						{
							pGrf = cache.get(*(precord->grf_src));
							uint32_t  idx, size;
							DWORD  WrittenBytes;
							openkore::grf_find(pGrf, precord->in_grf_path, &idx);
							LPVOID  data = openkore::grf_index_get(pGrf, idx, &size, 0);
							HANDLE  hFile = ::CreateFile(TempPath, GENERIC_WRITE, 0 /* file share */, NULL /* inherit ACL */, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL /* template handle */);
							::WriteFile(hFile, data, size, &WrittenBytes, 0);
							::CloseHandle(hFile);
						}
						catch ( ... )
						{
							this->MessageBox(CString(_T("Failed to extract ")) + strRealPath, CString(MAKEINTRESOURCE(IDS_APP_TITLE)), MB_ICONERROR | MB_OK);
						}
						if ( !PromptExtractPlace )
						{
							::CloseHandle(StartApplication(TempPath, m_hWnd));
						}
					}
				}
				else
				{
					// Extracting a folder or several files and/or folders
					CAtlList< pair<uint8_t, CString> >  EntriesToExtract;
					int  iItem = -1;
					CWaitCursor w(true, IDC_APPSTARTING, true);

					// Get Next from -1 because when entries are deleted, they are no longer selected!
					while ( (iItem = pListCtrl->GetNextItem(iItem, LVNI_ALL | LVNI_SELECTED)) != -1 )
					{
						pListCtrl->GetItemText(iItem, 0, strItemText);
						if ( !strCwd.IsEmpty() )
						{
							strItemText = strCwd + _T("\\") + strItemText;
						}
						DWORD_PTR dwptr = pListCtrl->GetItemData( iItem );
						// Directory
						if ( dwptr != 0 )
						{
							m_pDoc->Populate(strItemText, &EntriesToExtract);
						}
						else
						{
							EntriesToExtract.AddTail( pair<uint8_t, CString>(1, strItemText) );
						}
					}
					// List complete, prompt for target directory and extract them
					return this->RequestExtractionPath_(_T(""), cache, EntriesToExtract);
				}
			}
		}
	}
	else
	{
		  // Treeview, only via context menu
	CMyTreeCtrl * pTreeView = this->GetTreeViewPtr();
	CTreeItem  curSel;
		if ( m_MemorizedSelection != 0 )
		{
			curSel = CTreeItem(m_MemorizedSelection, pTreeView);
			m_MemorizedSelection = 0;
		}
		else
		{
			curSel = pTreeView->GetSelectedItem();
			if ( curSel == 0 )
			{
				return 0;
			}
		}
		if ( !PromptExtractPlace )
		{
			curSel.Select();
		}
		else
		{
			CAtlList< pair<uint8_t, CString> >  EntriesToExtract;
			pdirent = reinterpret_cast<CGrfDirectoryEntry *>(curSel.GetData());
			m_pDoc->Populate(pdirent->dir, &EntriesToExtract);
			return this->RequestExtractionPath_(_T(""), cache, EntriesToExtract);
		}
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT  CGrfView::RequestExtractionPath_(LPCTSTR pDefaultPath, CGrfCache &cache, const CAtlList< std::pair<uint8_t, CString> > &EntriesToExtract)
{
	TCHAR  TempPath[MAX_PATH];
	CGrfDirectoryEntry *pdirent;
	CGrfFileEntry *precord;

	CFolderDialog bff_dlg(m_hWnd, _T("Please select the location where to place extracted files."), BIF_USENEWUI | BIF_RETURNONLYFSDIRS | BIF_SHAREABLE);
	if ( IDOK != bff_dlg.DoModal() )
	{
		return 0;
	}
	for ( POSITION pos = EntriesToExtract.GetTailPosition();
		pos;
		)
	{
		pair<uint8_t, CString> p(EntriesToExtract.GetPrev(pos));
		if ( p.first == 2 )
		{
		}
		else
		{
			::PathCombine(TempPath, bff_dlg.GetFolderPath(), p.second);
			CString  dn, fn;
			::BreakPath(CString(TempPath), dn, fn);
			::SHCreateDirectoryEx(NULL, dn, NULL);
			::BreakPath(p.second, dn, fn);

			CWaitCursor w(true, IDC_APPSTARTING, true);

			m_pDoc->GetDirMapPtr()->Lookup(dn, pdirent);
			pdirent->files.Lookup(fn, precord);
			if ( precord->origin == CGrfFileEntry::FROMFS )
			{
				::SetFileAttributes(TempPath, 0);
				::CopyFile(precord->realpath, TempPath, FALSE);
			}
			else
			{
				CString  strRealPath;
				::MultiByteToWideChar(0x3B5, 0, precord->in_grf_path, -1, strRealPath.GetBufferSetLength(CGrfFileEntry::GRF_MAX_PATH), CGrfFileEntry::GRF_MAX_PATH);
				strRealPath.ReleaseBuffer();
				try
				{
					openkore::Grf * pGrf = cache.get(*(precord->grf_src));
					uint32_t  idx, size;
					DWORD  WrittenBytes;
					openkore::grf_find(pGrf, precord->in_grf_path, &idx);
					LPVOID  data = openkore::grf_index_get(pGrf, idx, &size, 0);
					HANDLE  hFile = ::CreateFile(TempPath, GENERIC_WRITE, 0 /* file share */, NULL /* inherit ACL */, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL /* template handle */);
					::WriteFile(hFile, data, size, &WrittenBytes, 0);
					::CloseHandle(hFile);
				}
				catch ( ... )
				{
					this->MessageBox(CString(_T("Failed to extract ")) + strRealPath, CString(MAKEINTRESOURCE(IDS_APP_TITLE)), MB_ICONERROR | MB_OK);
				}
			}
		}
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT  CGrfView::OnEditRename()
{
	if ( m_ListIsLastFocused )
	{
	CMyListCtrl * pListCtrl = this->GetListViewPtr();
	int  iFocused = pListCtrl->GetFocusedIndex();
		if ( iFocused != -1 )
		{
			pListCtrl->EditLabel(iFocused);
			HWND  hEdit = (HWND)pListCtrl->SendMessage(LVM_GETEDITCONTROL);
			::SendMessage(this->GetParentFrame(), UWM_ENTER_CHILD_FOCUS, 0, reinterpret_cast<LPARAM>(hEdit));
		}
	}
	else
	{
	CMyTreeCtrl * pTreeView = this->GetTreeViewPtr();
	CTreeItem  curSel;
		if ( m_MemorizedSelection != 0 )
		{
			curSel = CTreeItem(m_MemorizedSelection, pTreeView);
			m_MemorizedSelection = 0;
		}
		else
		{
			curSel = pTreeView->GetSelectedItem();
			if ( curSel == 0 )
			{
				return 0;
			}
		}
		pTreeView->EditLabel(curSel);
	HWND  hEdit = (HWND)pTreeView->SendMessage(TVM_GETEDITCONTROL);
		::SendMessage(this->GetParentFrame(), UWM_ENTER_CHILD_FOCUS, 0, reinterpret_cast<LPARAM>(hEdit));
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT  CGrfView::OnEditDelete()
{
int  nResponse;
CString  strPrompt;
CAtlList< pair<uint8_t, CString> >  RemovedEntries;

	if ( m_ListIsLastFocused )
	{
	CMyListCtrl * pListCtrl = this->GetListViewPtr();
		if ( pListCtrl->GetSelectedCount() == 0 )
		{
			return 0;
		}
	CGrfDirectoryEntry * pdirent(0);
	CGrfFileEntry * precord(0);
	CString  strCwd(this->GetCwd());

		if ( pListCtrl->GetSelectedCount() == 1 )
		{
		CString  strItemText;
			pListCtrl->GetItemText(pListCtrl->GetFirstSelectedIndex(), 0, strItemText);
			strPrompt.Format(CString(MAKEINTRESOURCE(IDS_DELETE_CONFIRMATION_PROMPT)), strItemText);
			nResponse = this->MessageBox(strPrompt, CString(MAKEINTRESOURCE(IDS_DELETE_WARNING_MSGBOX_TITLE)), MB_ICONWARNING | MB_YESNO);
			if ( nResponse == IDNO )
			{
				return 0;
			}
		LVITEM item;
			pListCtrl->GetFirstSelectedItem(&item);
			pListCtrl->GetItemText(item.iItem, 0, strItemText);

		DWORD_PTR dwptr = pListCtrl->GetItemData( item.iItem );
			// Directory
			if ( dwptr != 0 )
			{
				if ( !strCwd.IsEmpty() )
				{
					strItemText = strCwd + _T("\\") + strItemText;
				}
			CTreeItem entry(reinterpret_cast<HTREEITEM>(dwptr), this->GetTreeViewPtr());
				this->RmdirRecursive(entry);
				m_pDoc->RmdirRecursive(strItemText, &RemovedEntries);
				m_pDoc->SetModifiedFlag();
				// HINT_DOCUMENT_FILES_AND_DIRS_REMOVED
				m_pDoc->OnEntriesRemoved(2, &RemovedEntries, this);
			}
			else
			{
				m_pDoc->GetDirMap().Lookup(strCwd, pdirent);
				ATLASSERT(pdirent);
				pdirent->files.Lookup(strItemText, precord);
				ATLASSERT(precord);
				pListCtrl->DeleteItem(item.iItem);
				pdirent->files.RemoveKey(strItemText);
				delete precord;
				precord = 0;
				if ( !strCwd.IsEmpty() )
				{
					strItemText = strCwd + _T("\\") + strItemText;
				}

				RemovedEntries.AddTail( pair<uint8_t, CString>(1, strItemText) );
				m_pDoc->SetModifiedFlag();
				// HINT_DOCUMENT_FILES_REMOVED
				m_pDoc->OnEntriesRemoved(0, &RemovedEntries, this);
			}
		}
		else  // picked several items
		{
			strPrompt.Format(CString(MAKEINTRESOURCE(IDS_DELETE_MULTI_CONFIRMATION_PROMPT)), pListCtrl->GetSelectedCount());
			nResponse = this->MessageBox(strPrompt, CString(MAKEINTRESOURCE(IDS_DELETE_WARNING_MSGBOX_TITLE)), MB_ICONWARNING | MB_YESNO);
			if ( nResponse == IDNO )
			{
				return 0;
			}
		int  iItem;
		CString  strItemText;
			// Get Next from -1 because when entries are deleted, they are no longer selected!
			while ( (iItem = pListCtrl->GetNextItem(-1, LVNI_ALL | LVNI_SELECTED)) != -1 )
			{
				pListCtrl->GetItemText(iItem, 0, strItemText);
			DWORD_PTR dwptr = pListCtrl->GetItemData( iItem );
				// Directory
				if ( dwptr != 0 )
				{
					if ( !strCwd.IsEmpty() )
					{
						strItemText = strCwd + _T("\\") + strItemText;
					}
				CTreeItem entry(reinterpret_cast<HTREEITEM>(dwptr), this->GetTreeViewPtr());
					this->RmdirRecursive(entry);
					m_pDoc->RmdirRecursive(strItemText, &RemovedEntries);
				}
				else
				{
					m_pDoc->GetDirMap().Lookup(strCwd, pdirent);
					ATLASSERT(pdirent);
					pdirent->files.Lookup(strItemText, precord);
					ATLASSERT(precord);
					pListCtrl->DeleteItem(iItem);
					pdirent->files.RemoveKey(strItemText);
					delete precord;
					precord = 0;
					if ( !strCwd.IsEmpty() )
					{
						strItemText = strCwd + _T("\\") + strItemText;
					}

					RemovedEntries.AddTail( pair<uint8_t, CString>(1, strItemText) );
				}
			}
			if ( !RemovedEntries.IsEmpty() )
			{
				m_pDoc->SetModifiedFlag();
				// HINT_DOCUMENT_FILES_REMOVED
				m_pDoc->OnEntriesRemoved(0, &RemovedEntries, this);
			}
		}
	}
	else  // treeview
	{
	CMyTreeCtrl * pTreeView = this->GetTreeViewPtr();
	CTreeItem  curSel;
		if ( m_MemorizedSelection != 0 )
		{
			curSel = CTreeItem(m_MemorizedSelection, pTreeView);
			m_MemorizedSelection = 0;
		}
		else
		{
			curSel = pTreeView->GetSelectedItem();
			if ( curSel == 0 )
			{
				return 0;
			}
		}

	CGrfDirectoryEntry * pdirent = reinterpret_cast<CGrfDirectoryEntry *>(curSel.GetData());
		strPrompt.Format(CString(MAKEINTRESOURCE(IDS_DELETE_CONFIRMATION_PROMPT)), pdirent->dir);
		nResponse = this->MessageBox(strPrompt, CString(MAKEINTRESOURCE(IDS_DELETE_WARNING_MSGBOX_TITLE)), MB_ICONWARNING | MB_YESNO);
		if ( nResponse == IDNO )
		{
			return 0;
		}
		this->RmdirRecursive(curSel);
		m_pDoc->RmdirRecursive(pdirent->dir, &RemovedEntries);
		m_pDoc->SetModifiedFlag();
		// HINT_DOCUMENT_FILES_AND_DIRS_REMOVED
		m_pDoc->OnEntriesRemoved(2, &RemovedEntries, this);
	}
	  // Update title and UI ('modified' document state)
	m_pDoc->UpdateAllViews(0, HINT_UPDATE_DOCUMENT_PATH, reinterpret_cast<LPVOID>(0));

	return 0;
}

///////////////////////////////////////////////////////////////////////////////

void  CGrfView::UpdateUI()
{
CMenuHandle  menu = m_pcmd->GetMenu();
CMenuItemInfo  mii;
	mii.fMask = MIIM_STATE;
	mii.wID = IDM_FILE_SAVE;
	if ( m_pDoc->IsModified() )
	{
		mii.fState = MFS_ENABLED;
	}
	else
	{
		mii.fState = MFS_DISABLED | MFS_GRAYED;
	}
	menu.GetSubMenu(MENU_CONTEXT__TV_BG).GetSubMenu(0).SetMenuItemInfo(IDM_FILE_SAVE, FALSE, &mii);

	mii.fMask = MIIM_STATE;
	mii.wID = IDM_EDIT_OPEN;
	if ( this->CanPerformOpen() )
	{
		mii.fState = MFS_ENABLED;
	}
	else
	{
		mii.fState = MFS_DISABLED | MFS_GRAYED;
	}
	menu.GetSubMenu(MENU_CONTEXT__LV_ITEM).GetSubMenu(0).SetMenuItemInfo(IDM_EDIT_OPEN, FALSE, &mii);
	menu.GetSubMenu(MENU_CONTEXT__TV_BG).GetSubMenu(0).SetMenuItemInfo(IDM_EDIT_OPEN, FALSE, &mii);

	mii.wID = IDM_EDIT_EXTRACT;
	  // Same as IDM_EDIT_OPEN
	menu.GetSubMenu(MENU_CONTEXT__LV_ITEM).GetSubMenu(0).SetMenuItemInfo(IDM_EDIT_EXTRACT, FALSE, &mii);
	menu.GetSubMenu(MENU_CONTEXT__TV_BG).GetSubMenu(0).SetMenuItemInfo(IDM_EDIT_EXTRACT, FALSE, &mii);
}

///////////////////////////////////////////////////////////////////////////////

LRESULT CGrfView::OnLVBeginLabelEdit(LPNMHDR pnmh)
{
HWND  hEdit = (HWND)::SendMessage(pnmh->hwndFrom, LVM_GETEDITCONTROL, 0, 0L);
	::SendMessage(this->GetParentFrame(), UWM_ENTER_CHILD_FOCUS, 0, reinterpret_cast<LPARAM>(hEdit));
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT CGrfView::OnTVBeginLabelEdit(LPNMHDR pnmh)
{
	// Forbid renaming of the root
	if( this->GetTreeViewPtr()->GetRootItem() == reinterpret_cast<LPNMTVDISPINFO>(pnmh)->item.hItem )
	{
		::MessageBeep((UINT)-1);
		return TRUE;  // Cancel
	}
HWND  hEdit = (HWND)::SendMessage(pnmh->hwndFrom, LVM_GETEDITCONTROL, 0, 0L);
	::SendMessage(this->GetParentFrame(), UWM_ENTER_CHILD_FOCUS, 0, reinterpret_cast<LPARAM>(hEdit));
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT CGrfView::_OnLVDoubleClick(LPNMHDR pnmh)
{
	// Get the item we're clicking on
	NMITEMACTIVATE * const item = (NMITEMACTIVATE*) pnmh;

	if ( item->iItem != -1 )
	{
		return this->OnEditOpen();
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT CGrfView::OnContextMenu(HWND hWnd, const CPoint &point)
{
	if ( hWnd == this->GetListViewPtr()->m_hWnd )
	{
		return _OnLVContextMenu(hWnd, point);
	}
	else if ( hWnd == this->GetTreeViewPtr()->m_hWnd )
	{
		return _OnTVContextMenu(hWnd, point);
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT CGrfView::_OnLVContextMenu(HWND /*hWnd*/, const CPoint &point)
{
CPoint popupPoint;
CMyListCtrl *pListView = GetListViewPtr();
	if ( pListView->GetSelectedCount() == 0 )
	{
		if ( point.x == -1 )
		{
			popupPoint.SetPoint(14, 7);
			pListView->ClientToScreen(&popupPoint);
		}
		else
		{
			popupPoint.SetPoint(point.x, point.y);
		}
	CMenuHandle menu = m_pcmd->GetMenu();
		m_pcmd->TrackPopupMenu(menu.GetSubMenu(MENU_CONTEXT__LV_BG).GetSubMenu(0), TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON, popupPoint.x, popupPoint.y, NULL);
	}
	else
	{
		LVITEM item;
		pListView->GetFirstSelectedItem(&item);
		  // Non mouse-initiated message
		if ( point.x == -1 )
		{
			RECT rect;
			pListView->GetItemRect(item.iItem, &rect, LVIR_BOUNDS);
			pListView->ClientToScreen( &rect );
			  // Offset the popup menu origin so we can read some of the text
			popupPoint.SetPoint(rect.left + 15, rect.top + 8);
		}
		else
		{
			popupPoint.SetPoint(point.x, point.y);
		}

	CMenuHandle menu = m_pcmd->GetMenu();
		m_pcmd->TrackPopupMenu(menu.GetSubMenu(MENU_CONTEXT__LV_ITEM).GetSubMenu(0), TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON, popupPoint.x, popupPoint.y, NULL);
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT CGrfView::_OnTVContextMenu(HWND /*hWnd*/, const CPoint &point)
{
CPoint popupPoint;
CMyTreeCtrl *pTreeView = GetTreeViewPtr();
int  menuIndex;
TVHITTESTINFO hti = { 0 };
	hti.pt = point;
	pTreeView->ScreenToClient(&(hti.pt));
	pTreeView->HitTest(&hti);

	if ( hti.hItem == 0 || hti.hItem == pTreeView->GetRootItem() )
	{
		// Close, CloseOther, Save, SaveAs
		// Refresh
		// Properties

		menuIndex = MENU_CONTEXT__TV_BG;
		  // Non mouse-initiated message
		if ( hti.hItem && point.x == -1 )
		{
		RECT rect;
			pTreeView->GetItemRect(hti.hItem, &rect, TRUE);
			pTreeView->ClientToScreen( &rect );
			  // Offset the popup menu origin so we can read some of the text
			popupPoint.SetPoint(rect.left + 15, rect.top + 8);
		}
		else
		{
			hti.hItem = pTreeView->GetRootItem();
			if ( point.x == -1 )
			{
			CTreeItem  curSel = pTreeView->GetSelectedItem();
				  // A subitem is selected while context accelerator is pressed
				if ( curSel != pTreeView->GetRootItem() )
				{
					menuIndex = MENU_CONTEXT__TV_ITEM;
					hti.hItem = curSel;
				}
			RECT rect;
				pTreeView->GetItemRect(curSel, &rect, TRUE);
				pTreeView->ClientToScreen( &rect );
				// Offset the popup menu origin so we can read some of the text
				popupPoint.SetPoint(rect.left + 15, rect.top + 8);
			}
			else
			{
				popupPoint.SetPoint(point.x, point.y);
			}
		}
	}
	else
	{
		  // Non mouse-initiated message
		if ( point.x == -1 )
		{
		RECT rect;
			pTreeView->GetItemRect(hti.hItem, &rect, TRUE);
			pTreeView->ClientToScreen( &rect );
			  // Offset the popup menu origin so we can read some of the text
			popupPoint.SetPoint(rect.left + 15, rect.top + 8);
		}
		else
		{
			popupPoint.SetPoint(point.x, point.y);
		}
		menuIndex = MENU_CONTEXT__TV_ITEM;
	}

	if ( /*menuIndex == MENU_CONTEXT__TV_BG &&*/ hti.flags == TVHT_NOWHERE )
	{
		CMenuHandle  menu = m_pcmd->GetMenu();
		CMenuItemInfo  mii;
		mii.fMask = MIIM_STATE;
		mii.wID = IDM_EDIT_OPEN;
		mii.fState = MFS_DISABLED | MFS_GRAYED;
		menu.GetSubMenu(MENU_CONTEXT__TV_BG).GetSubMenu(0).SetMenuItemInfo(IDM_EDIT_OPEN, FALSE, &mii);
		mii.wID = IDM_EDIT_EXTRACT;
		menu.GetSubMenu(MENU_CONTEXT__TV_BG).GetSubMenu(0).SetMenuItemInfo(IDM_EDIT_EXTRACT, FALSE, &mii);
	}


CMenuHandle menu = m_pcmd->GetMenu();
	UINT  command = m_pcmd->TrackPopupMenu(menu.GetSubMenu(menuIndex).GetSubMenu(0), TPM_RETURNCMD | TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON, popupPoint.x, popupPoint.y, NULL);

	this->UpdateUI();

	if ( command != 0 )
	{
		m_MemorizedSelection = hti.hItem;
		::PostMessage(this->GetParentFrame(), WM_COMMAND, MAKELPARAM(command, 0), 0);
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT  CGrfView::RmdirRecursive(const CString &strFullPath)
{
CTreeItem ti(this->GetTreeViewPtr()->GetDir(strFullPath), this->GetTreeViewPtr());
	return this->RmdirRecursive(ti);
}

///////////////////////////////////////////////////////////////////////////////

LRESULT  CGrfView::RmdirRecursive(CTreeItem &ti)
{
	this->GetListViewPtr()->Rmdir(ti.m_hTreeItem);
	this->GetTreeViewPtr()->RmdirRecursive(ti);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT CGrfView::ReplaceDirPrefix(CTreeItem &ti, int nCurrentPrefixLength, const CString &strPrefix)
{
	CGrfDirectoryEntry *pdirent = reinterpret_cast<CGrfDirectoryEntry *>(ti.GetData());
	CString &strDir = pdirent->dir;
	CString topPart, endPart;
	m_pDoc->GetDirMap().RemoveKey(strDir);
CString strCwd(this->GetCwd());
bool IsBelowCurrentDir(::IsSubDir(strCwd, strDir));
CString strOrg(strDir);

	strDir.Format(_T("%s%s"), strPrefix, strDir.Right( strDir.GetLength() - nCurrentPrefixLength ));
	if ( IsBelowCurrentDir )
	{
		strCwd.Replace(strOrg, strDir);
	}
	m_pDoc->GetDirMap().SetAt(strDir, pdirent);
	::BreakPath(strDir, topPart, endPart);
	ti.SetText(endPart);
	// Insert Subfolders
	if ( ti.HasChildren() )
	{
		CTreeItem child = ti.GetChild();
		do
		{
			ReplaceDirPrefix(child, nCurrentPrefixLength, strPrefix);
			if ( TreeView_GetNextSibling(*child.m_pTreeView, child.m_hTreeItem) == NULL )
			{
				break;
			}
			child = child.GetNextSibling();
		} while ( 1 );
	}
	m_pDoc->UpdateAllViews(this, HINT_UPDATE_DOCUMENT_REFRESH, reinterpret_cast<LPVOID>(&strCwd));
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT CGrfView::UpdateView_List(const CTreeItem &ti, bool ForceRedraw)
{
CWaitCursor  w(true, IDC_APPSTARTING, true);
CMyListCtrl * pListView = this->GetListViewPtr();

SHFILEINFO sfi;
int iListPos = 0;
  // pdirent == 0 means no file (but subdirs maybe)
CGrfDirectoryEntry *pdirent = reinterpret_cast<CGrfDirectoryEntry *>(ti.GetData());

	// If the selected folder is the same, do not refresh
	if ( ForceRedraw || pListView->m_pcurrentTree != ti.m_hTreeItem )
	{
		  // FIXME: TODO: (2.0) use a listview cache system rather than destroy.
		  // This will improve performance for dirs with a very large number of files
		pListView->SetRedraw(FALSE);

		pListView->DeleteAllItems();
		pListView->m_pcurrentTree = ti.m_hTreeItem;

		// Insert Subfolders
		if ( ti.HasChildren() )
		{
		CTreeItem child = ti.GetChild();
		pair<int, CString> shellInfo;
			  // fetch cached shell call results
			CMyListCtrl::static_pmiml->m_ImageMapper.Lookup(_T("."), shellInfo);

			// "Borrow" folder list from TreeView's brotherhood
			do
			{
				CString text;
				child.GetText(text);

				pListView->InsertItem(iListPos, text, CMyListCtrl::static_pmiml->m_iclosedFolder);
				pListView->SetItemText( iListPos, 4, shellInfo.second);
				pListView->SetItemData( iListPos, reinterpret_cast<DWORD_PTR>(child.m_hTreeItem) );
				++iListPos;

				if ( TreeView_GetNextSibling(*child.m_pTreeView, child.m_hTreeItem) == NULL )
				{
					break;
				}
				child = child.GetNextSibling();
			} while ( 1 );
		}

		// Insert Folder items (file entries)
		if ( pdirent )
		{
		POSITION pos = pdirent->files.GetHeadPosition();

			while ( pos )
			{
			CGrfFileEntry *precord;
			CString strEntryName;
			LPCTSTR pstrTypeName;
				pdirent->files.GetAt(pos, strEntryName, precord);
				this->GetShellFileInfo(strEntryName, sfi, pstrTypeName);

				this->InsertItem_List(iListPos, precord, 0, strEntryName, sfi.iIcon, pstrTypeName);
				pdirent->files.GetNext(pos);
			}
		}  // Folder child Items
		ListView_SetImageList(pListView->m_hWnd, CMyListCtrl::static_pmiml->m_himl, LVSIL_NORMAL);
		ListView_SetImageList(pListView->m_hWnd, CMyListCtrl::static_pmiml->m_himlSmall, LVSIL_SMALL);
		// The current image list will be destroyed when the list-view control is destroyed
		// unless the LVS_SHAREIMAGELISTS style is set. If you use this message to replace
		// one image list with another, your application must explicitly destroy all
		// image lists other than the current one.
		// -- Bottom line:
		// Since we set the *same* IL for both, shell will attempt to destroy both ILs
		// Therefore, we set the LV style and destroy the IL in the listview destructor


//		if ( !(pListView->m_sortOrder.first == 0 && pListView->m_ascending.first) )
		{
			pListView->SortItemsEx(/*PFNLVCOMPARE*/&CMyListCtrl::CompareFunc, reinterpret_cast<LPARAM>(pListView));
		}
		if ( LV_VIEW_DETAILS == this->GetListViewPtr()->GetViewType() )
		{
		CHeaderCtrl  header = pListView->GetHeader();
			header.SetRedraw(FALSE);
			pListView->ResizeColumns_();
			header.SetRedraw(TRUE);
		}
		pListView->SetSelectionMark(0);
		pListView->FocusItem(0);
		pListView->SetRedraw(TRUE);
		pListView->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN);
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////

void  CGrfView::UpdateItemName_List(int iItem, const CString &strNew)
{
CWaitCursor  w(true, IDC_APPSTARTING, true);
CMyListCtrl * pListView = this->GetListViewPtr();
SHFILEINFO sfi;
LVITEM lvi;
	lvi.mask = LVIF_IMAGE;
	lvi.iItem = iItem; lvi.iSubItem = 0;
	pListView->GetItem(&lvi);

LPCTSTR pstrTypeName;
	this->GetShellFileInfo(strNew, sfi, pstrTypeName);
	lvi.iImage = sfi.iIcon;
	pListView->SetItem(&lvi);
	pListView->SetItemText( iItem, 4, pstrTypeName);
}


///////////////////////////////////////////////////////////////////////////////

LRESULT  CGrfView::OnSetFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	if ( ::IsWindow(m_hWndClient) )
	{
		if ( m_ListIsLastFocused && ::IsWindow(*(this->GetListViewPtr())) )
		{
			this->GetListViewPtr()->SetFocus();
		}
		else
		{
			this->GetTreeViewPtr()->SetFocus();
		}
		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN);
	}
	return 0;
}


///////////////////////////////////////////////////////////////////////////////

LRESULT CGrfView::Navigate(const CTreeItem &ti, bool bSetFocus)
{
	CMyTreeCtrl *pTree = this->GetTreeViewPtr();
	if ( (HTREEITEM)ti.GetParent() == NULL )
	{
		m_strCwd = _T("");
	}
	else
	{
		CGrfDirectoryEntry *pdirent = reinterpret_cast<CGrfDirectoryEntry *>(ti.GetData());
		m_strCwd = pdirent->dir;
	}

	if ( pTree->GetSelectedItem() == 0 )
	{
		pTree->Select(ti.m_hTreeItem, TVGN_CARET /*| TVSI_NOSINGLEEXPAND*/ );
	}

	this->UpdateView_List(ti, true);
	pTree->EnsureVisible(ti.m_hTreeItem);

	m_ListIsLastFocused = bSetFocus;

	return 0;
}

///////////////////////////////////////////////////////////////////////////////

// Ret. value has no special meaning
LRESULT  CGrfView::OnLVBeginDrag(LPNMHDR pnmh)
{
LPNMLISTVIEW pnmv = reinterpret_cast<LPNMLISTVIEW>(pnmh);
HWND  hWnd(pnmv->hdr.hwndFrom);

UINT  ItemsCount = this->GetListViewPtr()->GetSelectedCount();
	if ( ItemsCount == 0 )
	{
		return 0;
	}
CString strCwd(this->GetCwd());

	// Create our implementation of IDropSource
IDropSource * pDropSource = new CDropSource();
    if ( pDropSource )
	{
		// Create our implementation of IDataObject
	IDataObject * pDataObject = new CDataObject();
		if ( pDataObject )
		{
		DWORD  dwChars = lstrlen(m_pDoc->GetPathName()) + lstrlen(m_pDoc->GetName()) + strCwd.GetLength() + 3;
			// Get the indexes of the selected items
		int  *SelectedIndexes = new  int[ItemsCount];
		int  nStart = -1;
		CString strItemText;
			for ( UINT i = 0; i < ItemsCount; ++i )
			{
                nStart = SelectedIndexes[i] = this->GetListViewPtr()->GetNextItem(nStart, LVNI_SELECTED);
                this->GetListViewPtr()->GetItemText(SelectedIndexes[i], 0, strItemText);
                dwChars += strItemText.GetLength() + 1;
            }

		// FIXME: TODO: (1.5) Design clipboard format(s) so that data transfer may be done within a doc, across two docs and across processes


			// We needed to distinguish between intra-doc and outer-doc DnD operations
		const size_t  ReqSize = sizeof(DragDropInfo) + (dwChars + 1) * sizeof(TCHAR) /*+ ItemsCount * sizeof(ULONG_PTR)*/;
		HGLOBAL  hGlobal = ::GlobalAlloc(GHND | GMEM_SHARE, ReqSize);
			if ( hGlobal == NULL )
			{
				return  0;
			}
		DragDropInfo *pInfoInterpret = reinterpret_cast<DragDropInfo *>(::GlobalLock(hGlobal));
			if ( pInfoInterpret == NULL )
			{
				::GlobalFree(hGlobal);
				return  0;
			}
			{
				pInfoInterpret->lStructSize = (DWORD)ReqSize;
				pInfoInterpret->hWnd = reinterpret_cast<HWND>(::SendMessage(this->GetParentFrame() /* ->GetMDIFrame() */, UWM_GETMDIFRAME, 0, 0));
				pInfoInterpret->dwProcessId = ::GetCurrentProcessId();
			}
		TCHAR *pData = (TCHAR*)((LPBYTE)(pInfoInterpret) + sizeof(DragDropInfo));

			lstrcpy(pData, m_pDoc->GetPathName());
			pData += lstrlen(pData) + 1;
			lstrcpy(pData, m_pDoc->GetName());
			pData += lstrlen(pData) + 1;
			lstrcpy(pData, strCwd);
			pData += strCwd.GetLength() + 1;
			for ( UINT i = 0; i < ItemsCount; ++i )
			{
                this->GetListViewPtr()->GetItemText(SelectedIndexes[i], 0, strItemText);
				lstrcpy(pData, strItemText);
				pData += strItemText.GetLength() + 1;
            }
            *pData++ = _T('\0');
            ATLASSERT(((LPBYTE)(pInfoInterpret) + pInfoInterpret->lStructSize) == (LPBYTE)pData);

			::GlobalUnlock(hGlobal);

			// Put the data in the data source.
		static FORMATETC etc = { UCF_GRYFF_ENTRIES_INTERNAL, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
		static STGMEDIUM stgm = { TYMED_HGLOBAL, NULL, NULL };
			stgm.hGlobal = hGlobal;
			pDataObject->SetData(&etc, &stgm, TRUE);  // released unless failed

			// Prepare the drag images.
			// In this particular case, ListView supports DI_GETDRAGIMAGE
			// therefore we use IDragSourceHelper::InitializeFromWindow
			// otherwise we would have to go the SHDRAGIMAGE + InitializeFromBitmap() way.
		IDragSourceHelper  *pDragSrcHlp;
			if ( SUCCEEDED( ::CoCreateInstance(CLSID_DragDropHelper, NULL,
								CLSCTX_ALL, IID_IDragSourceHelper, (LPVOID *)&pDragSrcHlp) ) )
			{
				pDragSrcHlp->InitializeFromWindow(this->GetListViewPtr()->m_hWnd, &pnmv->ptAction, pDataObject);
			}
			else
			{
				pDragSrcHlp = 0;
			}

			// All is prepared, do the drag/drop
		DWORD  dwEffect;
		HRESULT  hr;
			// Allow copying and moving
			if ( SUCCEEDED((hr = ::DoDragDrop(pDataObject, pDropSource, DROPEFFECT_MOVE | DROPEFFECT_COPY, &dwEffect))) )
			{
				if ( DRAGDROP_S_DROP == hr && (dwEffect & DROPEFFECT_MOVE) )
				{
					// FIXME: TODO: (1.5) Perform operation on source when drop succeeded - Delete file from temp when moving to shell?
//					::DeleteFile(g_pszTarget);
				CMyListCtrl * pListCtrl = GetListViewPtr();
				CString  strItem;
				CAtlList< pair<uint8_t, CString> > RemovedEntries;
				CGrfDirectoryEntry *pdirent(0);
				CGrfFileEntry *precord(0);
				int  iItem;
					// Get Next from -1 because when entries are deleted, they are no longer selected!
					while ( (iItem = pListCtrl->GetNextItem(-1, LVNI_ALL | LVNI_SELECTED)) != -1 )
					{
					CString strItem, strTarget;
						pListCtrl->GetItemText(iItem, 0, strItem);
					CString strFullPath(strCwd.IsEmpty()? strItem : strCwd + _T("\\") + strItem);

					DWORD_PTR dwptr = pListCtrl->GetItemData( iItem );
						// Directory
						if ( dwptr != 0 )
						{
							CTreeItem entry(reinterpret_cast<HTREEITEM>(dwptr), this->GetTreeViewPtr());
							this->RmdirRecursive(entry);
							m_pDoc->RmdirRecursive(strFullPath, &RemovedEntries);
						}
						else
						{
							m_pDoc->GetDirMap().Lookup(strCwd, pdirent);
							ATLASSERT(pdirent);
							pdirent->files.Lookup(strItem, precord);
							ATLASSERT(precord);
							pListCtrl->DeleteItem(iItem);
							pdirent->files.RemoveKey(strItem);
							delete precord;
							RemovedEntries.AddTail( pair<uint8_t, CString>(1, strFullPath) );
						}
					}
					if ( !RemovedEntries.IsEmpty() )
					{
						m_pDoc->SetModifiedFlag();
						// HINT_DOCUMENT_FILES_REMOVED
						m_pDoc->OnEntriesRemoved(0, &RemovedEntries, this);
						m_pDoc->UpdateAllViews(this, HINT_UPDATE_DOCUMENT_REFRESH, reinterpret_cast<LPVOID>(&strCwd));
					}

				}
			}
			else
			{
				// Clean-up manually when failed
				hGlobal = ::GlobalFree(hGlobal);
			}

			if ( pDragSrcHlp )
			{
				pDragSrcHlp->Release();
			}


			pDataObject->Release();
		}
		pDropSource->Release();
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT  CGrfView::OnLVItemChanged(LPNMHDR pnmh)
{
	this->UpdateUI();
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

void  CGrfView::OnTogglePane(UINT /*uCode*/, int /*nID*/, HWND /*hwndCtrl*/)
{
	  // already in single-pane?
	if ( m_wndVertSplit.GetSinglePaneMode() != SPLIT_PANE_NONE )
	{
		  // Show both panes
		m_wndVertSplit.SetSinglePaneMode(SPLIT_PANE_NONE);
	}
	else
	{
		  // Hide left pane
		m_wndVertSplit.SetSinglePaneMode(SPLIT_PANE_RIGHT);
	}
	::SendMessage(this->GetParentFrame(), UWM_UPDATE, WPARAM(HINT_DOCUMENT_UI_UPDATE_MISC), 0);
}

///////////////////////////////////////////////////////////////////////////////

LRESULT  CGrfView::OnGetSinglePaneMode(UINT, WPARAM, LPARAM, BOOL&)
{
	return m_wndVertSplit.GetSinglePaneMode();
}

///////////////////////////////////////////////////////////////////////////////

LRESULT  CGrfView::OnAppendItem_List(UINT, WPARAM, LPARAM lParam, BOOL&)
{
CMyListCtrl * pListView = this->GetListViewPtr();
CMyListCtrl::LPMYLISTVIEWAPPENDINFO pAppendInfo = reinterpret_cast<CMyListCtrl::LPMYLISTVIEWAPPENDINFO>(lParam);
int iListPos(pListView->GetItemCount());
SHFILEINFO sfi;

LPCTSTR pstrTypeName;
	if ( pAppendInfo->precord )
	{
		this->GetShellFileInfo(*(pAppendInfo->pstrEntryName), sfi, pstrTypeName);
	}
	else
	{
		::SHGetFileInfo(_T(""), FILE_ATTRIBUTE_DIRECTORY, &sfi, sizeof(SHFILEINFO), SHGFI_USEFILEATTRIBUTES/* | SHGFI_SYSICONINDEX*/ | SHGFI_TYPENAME);
		sfi.iIcon = CMyListCtrl::static_pmiml->m_iclosedFolder;
		pstrTypeName = sfi.szTypeName;
	}
	return this->InsertItem_List(iListPos, pAppendInfo->precord, pAppendInfo->ptreeitem, *(pAppendInfo->pstrEntryName), sfi.iIcon, pstrTypeName);
}

///////////////////////////////////////////////////////////////////////////////

int __stdcall  CGrfView::InsertItem_List(int &iListPos, const CGrfFileEntry *precord, const HTREEITEM *ptreeitem, const CString &strEntryName, int iIcon, LPCTSTR pstrTypeName)
{
CMyListCtrl * pListView = this->GetListViewPtr();
int nPos = pListView->InsertItem(iListPos, strEntryName, iIcon);

	if ( nPos != -1 )
	{
		pListView->SetItemText( iListPos, 4, pstrTypeName);
		if ( ptreeitem )
		{
			pListView->SetItemData( iListPos, reinterpret_cast<DWORD_PTR>(*(ptreeitem)) );
		}
		else  // file entry: compute ratios and format numbers
		{
			pListView->SetItemData( iListPos, DWORD_PTR(0) );  // not a folder

			this->FormatItem_List(iListPos, precord);
		}
		++iListPos;
	}
	return nPos;
}

///////////////////////////////////////////////////////////////////////////////

void __stdcall  CGrfView::FormatItem_List(const int &iListPos, const CGrfFileEntry *precord)
{
CMyListCtrl * pListView = this->GetListViewPtr();
CString  numericvalue, localizedvalue;
LCID  lcid = ::GetThreadLocale();
static NUMBERFMT  nbfmt = {0};
static bool  retrievedFormat = false;
static TCHAR  buf[50];
int  reqbufsiz;

	if ( !retrievedFormat )
	{
		  // Format the numbers according to current thread locale
		nbfmt.lpDecimalSep = _T("");
		::GetLocaleInfo(lcid, LOCALE_SGROUPING, buf, sizeof(buf) / sizeof(buf[0]));
		nbfmt.Grouping = _ttoi(buf);
		::GetLocaleInfo(lcid, LOCALE_STHOUSAND, buf, sizeof(buf) / sizeof(buf[0]));
		nbfmt.lpThousandSep = buf;
		retrievedFormat = true;
	}

	  // default value as well
	numericvalue.Format(_T("%u"), precord->fullsize);
	reqbufsiz = ::GetNumberFormat(lcid, 0, numericvalue, &nbfmt, NULL, 0);
	  // If for some reason system doesn't manage, give up formatting and dump the value as is
	if ( reqbufsiz == 0 && ::GetLastError() != ERROR_INSUFFICIENT_BUFFER )
	{
		pListView->SetItemText( iListPos, 1, numericvalue);
	}
	else
	{
		::GetNumberFormat(lcid, 0, numericvalue, &nbfmt, localizedvalue.GetBufferSetLength(reqbufsiz), reqbufsiz);
		localizedvalue.ReleaseBuffer();
		pListView->SetItemText( iListPos, 1, localizedvalue);
	}

	  // Do the same for compressed size if applicable, then compute the ratio
	if ( precord->compsize != static_cast<DWORD>(-1) )
	{
		numericvalue.Format(_T("%u"), precord->compsize);
		reqbufsiz = ::GetNumberFormat(lcid, 0, numericvalue, &nbfmt, NULL, 0);

		if ( reqbufsiz == 0 && ::GetLastError() != ERROR_INSUFFICIENT_BUFFER )
		{
			pListView->SetItemText( iListPos, 2, numericvalue);
		}
		else
		{
			::GetNumberFormat(lcid, 0, numericvalue, &nbfmt, localizedvalue.GetBufferSetLength(reqbufsiz), reqbufsiz);
			localizedvalue.ReleaseBuffer();
			pListView->SetItemText( iListPos, 2, localizedvalue);
		}

		if ( precord->fullsize == 0 )
		{
			pListView->SetItemText( iListPos, 3, _T("0%"));
		}
		else
		{
		unsigned int  ratio = (100 * precord->compsize) / precord->fullsize;
			numericvalue.Format(_T("%u%%"), ratio);
			pListView->SetItemText( iListPos, 3, numericvalue);
		}
	}
	else
	{
		pListView->SetItemText( iListPos, 2, _T(""));
		pListView->SetItemText( iListPos, 3, _T(""));
	}
	pListView->SetItemText( iListPos, 5, CString(MAKEINTRESOURCE(precord->origin == CGrfFileEntry::FROMGRF ? IDS_GRFORIGIN_GRF : IDS_GRFORIGIN_FS)));
}

///////////////////////////////////////////////////////////////////////////////

void __stdcall  CGrfView::GetShellFileInfo(const CString& strEntryName, SHFILEINFO &sfi, LPCTSTR &pstrTypeName)
{
CMyListCtrl *pListView = GetListViewPtr();
	// Get customized type names, fill sfi if successful
	if ( !CMyListCtrl::GetGravityFileTypeName(strEntryName, &sfi) )
	{
		  // Lookup shell extension, first in cache, otherwise call ::SHGetFileInfo
	int pos = strEntryName.ReverseFind(_T('.'));
	CString extensionKey(pos < 0 ? _T("") : (strEntryName.operator LPCTSTR()) + pos);
	pair<int, CString> shellInfo;
		  // fetch cached shell call results
		if ( CMyListCtrl::static_pmiml->m_ImageMapper.Lookup(extensionKey, shellInfo) )
		{
			sfi.iIcon = shellInfo.first;
			pstrTypeName = shellInfo.second.operator LPCTSTR();
		}
		else
		{
			  // Call shell function and add to view's imagelist
		/*HIMAGELIST  hSysImlSmall = (HIMAGELIST)*/::SHGetFileInfo(extensionKey, FILE_ATTRIBUTE_ARCHIVE, &sfi, sizeof(SHFILEINFO), SHGFI_USEFILEATTRIBUTES | SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_TYPENAME);
			pstrTypeName = sfi.szTypeName;
			  // Register in cache
			CMyListCtrl::static_pmiml->m_ImageMapper.SetAt(extensionKey, pair<int, CString>(sfi.iIcon, pstrTypeName));
		}
	}
	else
	{
		pstrTypeName = sfi.szTypeName;
	}
}

///////////////////////////////////////////////////////////////////////////////

LRESULT  CGrfView::OnConfirmFocus(UINT, WPARAM wParam, LPARAM, BOOL&)
{
	m_ListIsLastFocused = (wParam == LISTVIEW_ID);
	return 0;
}

