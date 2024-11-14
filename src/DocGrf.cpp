// $Id: DocGrf.cpp 9 2005-08-12 17:33:08Z Rasqual $

// *
// *   gryff, a GRF archive management utility, src/DocGrf.cpp
// *   Copyright (C) 2003, 2004 Rasqual Twilight
// *
// *   This program is distributed in the hope that it will be useful,
// *   but WITHOUT ANY WARRANTY; without even the implied warranty of
// *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// *

// * DocGrf.cpp
// * Grf Document manipulation template (implementation)
// *
// *@author $Author: Rasqual $
// *@version $Revision: 9 $
// *

#include  "stdwtl.h"
#pragma hdrstop
#include  "resource.h"
#include  "DocGrf.h"

#define NO_OFN_DIALOG
#include  "fnmanip.h"

using std::pair;
extern LPCTSTR szTempArchivePrefix;

///////////////////////////////////////////////////////////////////////////////

/**
	Performs initialization.
**/
CGrfDoc::CGrfDoc()
{
static bool initialized_static(false);
	if ( !initialized_static )
	{
		initialized_static = true;
		  // Initialize static data
		CGrfDoc::strFilter.Format(
		 _T("%s|*.grf;*.gpf|")
		  _T("%s|*.*|")
		  _T(""),
		 CString(MAKEINTRESOURCE(IDS_FILTER_GRF_FILES)), CString(MAKEINTRESOURCE(IDS_FILTER_ALL_FILES)));
		CGrfDoc::strTypeFilter.Format(
		 _T("%s|*.spr;*.act;*.imf;*.fna;*.rsw;*.gnd;*.gat;*.rsm;*.rsx;*.gr2;*.txt;*.xml;*.bmp;*.jpg;*.tga;*.str;*.pal;*.wav;*.mp3;*.bik|")
		  _T("%s|*.spr;*.act;*.imf;*.fna|")
		  _T("%s|*.rsw;*.gnd;*.gat|")
		  _T("%s|*.rsm;*.rsx;*.gr2|")
		  _T("%s|*.txt;*.xml|")
		  _T("%s|*.bmp;*.jpg;*.tga;*.str;*.pal|")
		  _T("%s|*.wav;*.mp3;*.bik|")
		  _T("%s|*.*|")
		  _T(""), CString(MAKEINTRESOURCE(IDS_FILTER_ALL_MULTIFILES)),
		  CString(MAKEINTRESOURCE(IDS_FILTER_ANIM_MULTIFILES)),
		  CString(MAKEINTRESOURCE(IDS_FILTER_MAP_MULTIFILES)),
		  CString(MAKEINTRESOURCE(IDS_FILTER_MODELS_MULTIFILES)),
		  CString(MAKEINTRESOURCE(IDS_FILTER_CFG_MULTIFILES)),
		  CString(MAKEINTRESOURCE(IDS_FILTER_GFX_MULTIFILES)),
		  CString(MAKEINTRESOURCE(IDS_FILTER_SND_MULTIFILES)),
		  CString(MAKEINTRESOURCE(IDS_FILTER_ALL_FILES)));
	}
	m_strFilePath[0] = _T('\0');
	m_strFileName[0] = _T('\0');
}  // CGrfDoc()

///////////////////////////////////////////////////////////////////////////////

/**
	Destroys document data, walking the directory entries map,
	delete()ing file entries, then delete()ing directory entries.
	Clearing the map is to be done by the map destructor.
**/
CGrfDoc::~CGrfDoc()
{
	// Walk the directory entries...
	// In these entries, delete the File entries, then delete the Directory entry
POSITION pos = m_GrfMap.GetHeadPosition();
	while ( pos )
	{
	CGrfDirectoryEntry *pdirent(m_GrfMap.GetValueAt(pos));
	POSITION pos2(pdirent->files.GetHeadPosition());
		while ( pos2 )
		{
			delete /* (CGrfFileEntry*) */ pdirent->files.GetValueAt(pos2);
			pdirent->files.GetNext(pos2);
		}
		delete pdirent;
		m_GrfMap.GetNext(pos);
	}
}  // ~CGrfDoc()

///////////////////////////////////////////////////////////////////////////////

/**
	Implements CDocumentBase::OnNewDocument().
	This is not called directly generally.
	Returns TRUE on success
**/
BOOL  CGrfDoc::OnNewDocument()
{
	ATLASSERT(lstrlen(m_strFilePath) == 0);

	// FIXME: TODO: (1.2) User option: whether to "create data folder for new documents"
	// or create root directory only
	this->MkdirRecursive( true == true ? _T("data") : _T("") );
	  // Untitled document
	this->SetDocumentName(NULL);

	return TRUE;
}  // OnNewDocument()

///////////////////////////////////////////////////////////////////////////////

/**
	Implements CDocumentBase::OnOpenDocument()
	This is not called directly generally.
	CALL THIS FROM A WORKER THREAD.
	Throws: a LONG error code.
	Returns TRUE on success
**/
BOOL  CGrfDoc::OnOpenDocument(LPCTSTR pDocumentPath)
{
	ATLASSERT( pDocumentPath != 0 );

#if defined(_DEBUG)
//CString strOpInfo;

//FILETIME ft_b4, ft_af;
//		::GetSystemTimeAsFileTime(&ft_b4);
#endif
	this->ImportGrf(pDocumentPath);
#if defined(_DEBUG)
//		::GetSystemTimeAsFileTime(&ft_af);
//		strOpInfo.Format(_T("Import [%s] completed in %d ms"), pDocumentPath, (ft_af.dwLowDateTime - ft_b4.dwLowDateTime) / 10000);
//		::MessageBox(NULL, strOpInfo, _T("CGrfDoc::OnOpenDocument"), MB_OK);
		::MessageBeep((UINT)-1);
#endif

	return TRUE;
}  // OnOpenDocument()

///////////////////////////////////////////////////////////////////////////////

/**
	Implements CDocumentBase::SetDocumentName()
	If lpDocumentName is not 0, sets lpDocumentName as document name,
	Else document is assigned a new "Untitled" value.
	bSetModifiedFlag specifies whether the document is to be flagged as modified,
	regardless of the lpDocumentName value.
	It can be called directly.
	Returns TRUE on success
**/
BOOL  CGrfDoc::SetDocumentName(LPCTSTR lpDocumentName, bool bSetModifiedFlag)
{
	if ( bSetModifiedFlag )
	{
		this->SetModifiedFlag();
	}

	if ( lpDocumentName == 0 )
	{
	CString strUntitled(MAKEINTRESOURCE(IDS_UNTITLED_NAME));  // Default localized name

		wsprintf(m_strFileName, _T("%s%d"), strUntitled, CGrfDoc::GetUntitledIndex());
		m_strFilePath[0] = _T('\0');
	}
	else
	{
	CString s(lpDocumentName);
	CString dn, fn;

		::BreakPath(s, dn, fn);
		lstrcpyn(m_strFilePath, s, MAX_PATH);
		lstrcpyn(m_strFileName, fn, MAX_PATH);
	}
	return TRUE;
}  // SetDocumentName

///////////////////////////////////////////////////////////////////////////////

/**
	Sets all entries to FROMGRF with the specified document path as source
**/
void  CGrfDoc::SetGrfOrigin(CGrfCache *pCache, LPCTSTR pGrfPath)
{
CGrfDirectoryEntry    *pdirent;
CGrfFileEntry         *precord;
CString                strDirectory, strName, strCompleteName;

	for ( POSITION pos = m_GrfMap.GetHeadPosition(); pos; m_GrfMap.GetNext(pos) )
	{
		m_GrfMap.GetAt(pos, strDirectory, pdirent);
		for ( POSITION pos2 = pdirent->files.GetHeadPosition(); pos2; pdirent->files.GetNext(pos2) )
		{
			pdirent->files.GetAt(pos2, strName, precord);
			precord->origin = CGrfFileEntry::FROMGRF;
			if ( strDirectory.IsEmpty() )
			{
				strCompleteName = strName;
			}
			else
			{
				strCompleteName.Format(_T("%s\\%s"), strDirectory, strName);
			}
			::WideCharToMultiByte(0x3B5, 0, strCompleteName, -1, precord->in_grf_path, CGrfFileEntry::GRF_MAX_PATH, NULL, NULL);
			if ( precord->grf_src == 0 )
			{
				precord->grf_src = new CString(pGrfPath);
			}
			else
			{
				*(precord->grf_src) = pGrfPath;
			}
		uint32_t  idx;
		openkore::Grf * pGrf = pCache->get(pGrfPath);
			openkore::grf_find(pGrf, precord->in_grf_path, &idx);
			precord->setSizes(pGrf->files[idx].real_len, pGrf->files[idx].compressed_len);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

/**
	Opens Grf documents and puts their handles into the specified cache.
	CALL THIS FROM A WORKER THREAD.
	If pOutUnresolvedEntries is not 0, failed to open documents are appended to the pointed list
**/
bool  CGrfDoc::CacheHandles(CGrfCache &cache, CAtlList< pair<uint8_t, CString> > *pOutUnresolvedEntries) const
{
bool bUnresolvedEntries(false);
POSITION pos( m_GrfMap.GetHeadPosition() );
CGrfDirectoryEntry *pdirent;
CGrfFileEntry *precord;
CString strDirectory, strName, strCompleteName, strMessage;

	for ( ; pos; m_GrfMap.GetNext(pos) )
	{
		pdirent = m_GrfMap.GetValueAt(pos);
		for ( POSITION pos2 = pdirent->files.GetHeadPosition(); pos2; pdirent->files.GetNext(pos2) )
		{
			precord = pdirent->files.GetValueAt(pos2);
			if ( precord->origin == CGrfFileEntry::FROMGRF )
			{
				try
				{
					cache.get(*precord->grf_src);
				}
				catch ( ... )
				{
					if ( pOutUnresolvedEntries )
					{
						strDirectory = m_GrfMap.GetKeyAt(pos);
						if ( strDirectory.IsEmpty() )
						{
							strCompleteName = pdirent->files.GetKeyAt(pos2);
						}
						else
						{
							strCompleteName.Format(_T("%s\\%s"), strDirectory, pdirent->files.GetKeyAt(pos2));
						}
						strMessage.Format(_T("Unresolved Grf <%s> for <%s>"), *precord->grf_src, strCompleteName);

						pOutUnresolvedEntries->AddTail(pair<uint8_t, CString>(0, strMessage));
					}
					bUnresolvedEntries = true;
				}
			}
		}
	}

	return !bUnresolvedEntries;
}  // CacheHandles

///////////////////////////////////////////////////////////////////////////////

/**
	Tests whether there are invalid entries in the document.
	Invalid entries are:
	 . entries with bad characters (charset)
	 . entries whose full path is longer than GRF_MAX_PATH
	If pOutInvalidEntries is not 0, invalid entries are appended to the pointed list
**/
bool  CGrfDoc::HasInvalidEntries(CAtlList< pair<uint8_t, CString> > *pOutInvalidEntries) const
{
bool bInvalidEntries(false);
POSITION pos( m_GrfMap.GetHeadPosition() );
CGrfDirectoryEntry *pdirent;
CGrfFileEntry *precord;
int nMbCount;
CString strDirectory, strName, strCompleteName;

	for ( ; pos; m_GrfMap.GetNext(pos) )
	{
		  // Sets strDirectory
		m_GrfMap.GetAt(pos, strDirectory, pdirent);
		nMbCount = ::WideCharToMultiByte(0x3B5, 0, strDirectory, -1, NULL, 0, NULL, NULL);
		if ( nMbCount >= CGrfFileEntry::GRF_MAX_PATH )
		{
			if ( pOutInvalidEntries )
			{
				pOutInvalidEntries->AddTail(pair<uint8_t, CString>(2, strCompleteName));
				  // All subitems are invalid
				for ( POSITION pos2 = pdirent->files.GetHeadPosition(); pos2; pdirent->files.GetNext(pos2) )
				{
					  // Sets strName
					pdirent->files.GetAt(pos2, strName, precord);
					if ( strDirectory.IsEmpty() )
					{
						strCompleteName = strName;
					}
					else
					{
						strCompleteName.Format(_T("%s\\%s"), strDirectory, strName);
					}
					pOutInvalidEntries->AddTail(pair<uint8_t, CString>(1, strCompleteName));
				}
			}
			bInvalidEntries = true;
		}
		else
		{
			for ( POSITION pos2 = pdirent->files.GetHeadPosition(); pos2; pdirent->files.GetNext(pos2) )
			{
				// Sets strName
				pdirent->files.GetAt(pos2, strName, precord);
				if ( strDirectory.IsEmpty() )
				{
					strCompleteName = strName;
				}
				else
				{
					strCompleteName.Format(_T("%s\\%s"), strDirectory, strName);
				}
				nMbCount = ::WideCharToMultiByte(0x3B5, 0, strCompleteName, -1, NULL, 0, NULL, NULL);
				if ( nMbCount >= CGrfFileEntry::GRF_MAX_PATH )
				{
					if ( pOutInvalidEntries )
					{
						pOutInvalidEntries->AddTail(pair<uint8_t, CString>(1, strCompleteName));
					}
					bInvalidEntries = true;
					continue;
				}
				else
				{
					// Round-trip conversion
				char  szEntryNameA[CGrfFileEntry::GRF_MAX_PATH];
				WCHAR szEntryNameW[CGrfFileEntry::GRF_MAX_PATH];
					::WideCharToMultiByte(0x3B5, 0, strCompleteName, -1, szEntryNameA, nMbCount, NULL, NULL);
					::MultiByteToWideChar(0x3B5, 0, szEntryNameA, -1, szEntryNameW, CGrfFileEntry::GRF_MAX_PATH);
					if ( strCompleteName != szEntryNameW )
					{
						if ( pOutInvalidEntries )
						{
							pOutInvalidEntries->AddTail(pair<uint8_t, CString>(1, strCompleteName));
						}
						bInvalidEntries = true;
					}
				}
			}
		}  // else not directory is invalid
	}  // end for each dir

	return bInvalidEntries;
}  // HasInvalidEntries

///////////////////////////////////////////////////////////////////////////////

/**
	Tests whether a directory called strAbsDirName exists in the document
**/
bool CGrfDoc::DirExists(const CString &strAbsDirName, CGrfDirectoryEntry **ppdirent) const
{
CGrfDirectoryEntry *pdirent;
	if ( ppdirent )
	{
		*ppdirent = 0;
	}
	return m_GrfMap.Lookup(strAbsDirName, ppdirent? *ppdirent : pdirent);
}  // DirExists

///////////////////////////////////////////////////////////////////////////////

/**
	Tests whether a file called strAbsFileName exists in the document
**/
bool CGrfDoc::FileExists(const CString &strAbsFileName, CGrfFileEntry **pprecord) const
{
CString dn, fn;
	::BreakPath(strAbsFileName, dn, fn);
CGrfDirectoryEntry *pdirent;
CGrfFileEntry *precord;
	if ( pprecord )
	{
		*pprecord = 0;
	}
	return m_GrfMap.Lookup(dn, pdirent) && pdirent->files.Lookup(fn, pprecord? *pprecord : precord);

}  // FileExists

///////////////////////////////////////////////////////////////////////////////

/**
	Tests whether pDocumentPath corresponds to this document
**/
bool CGrfDoc::MatchesPath(LPCTSTR pDocumentPath, bool TestingUntitled) const
{
	if ( TestingUntitled )
	{
		return ::lstrcmp(pDocumentPath, this->m_strFileName) == 0;
	}
	return ::lstrcmp(pDocumentPath, m_strFilePath) == 0;
}  // MatchesPath

///////////////////////////////////////////////////////////////////////////////

/**
	Implements CDocument<T>::OnAllViewsUpdateDone
**/
void CGrfDoc::OnAllViewsUpdateDone(CGrfDocView* pSender, LPARAM Hint, LPVOID pHint)
{
}

///////////////////////////////////////////////////////////////////////////////

/**
	  Called to update views when entries are added.
	For efficiency reasons, the lHint passed to OnUpdate as a LPARAM may be
	specialized as follows:
	 lHint == 0 : Files
	 lHint == 1 : Directories
	 lHint == 2 : Files & Directories
	pModEntries is a pointer to a non-empty CAtlList< pair<uint8_t, CString> >
	which is passed to OnUpdate as a WPARAM.
	When applicable, test the entry type with the protected method IsEntryTypeDir().
	Please note, an entry may be marked as added while in fact it was overwritten.
	pSendingView specifies the sending view of the document and passed to OnUpdate
	as a pointer to CGrfDocView, a base class of CViewImpl<CGrfView, CGrfDocView>,
	which itself is a base class of CGrfView.
	When pSendingView is 0, OnUpdate receives 0 too, meaning all views shall be updated.
**/
void CGrfDoc::OnEntriesAdded(LPARAM lHint, const CAtlList< pair<uint8_t, CString> > *pModEntries, CGrfDocView *pSendingView)
{
	UpdateAllViews( pSendingView, HINT_DOCUMENT_FILES_ADDED - lHint, reinterpret_cast<LPVOID>(const_cast<CAtlList< pair<uint8_t, CString> > *>(pModEntries)) );
}

///////////////////////////////////////////////////////////////////////////////

/**
	  Called to update views when entries are removed.
	For efficiency reasons, the lHint passed to OnUpdate as a LPARAM may be
	specialized as follows:
	 lHint == 0 : Files
	 lHint == 1 : Directories
	 lHint == 2 : Files & Directories
	pModEntries is a pointer to a non-empty CAtlList< pair<uint8_t, CString> >
	which is passed to OnUpdate as a WPARAM.
	When applicable, test the entry type with the protected method IsEntryTypeDir().
	pSendingView specifies the sending view of the document and passed to OnUpdate
	as a pointer to CGrfDocView, a base class of CViewImpl<CGrfView, CGrfDocView>,
	which itself is a base class of CGrfView.
	When pSendingView is 0, OnUpdate receives 0 too, meaning all views shall be updated.
**/
void CGrfDoc::OnEntriesRemoved(LPARAM lHint, const CAtlList< pair<uint8_t, CString> > *pModEntries, CGrfDocView *pSendingView)
{
	UpdateAllViews( pSendingView, HINT_DOCUMENT_FILES_REMOVED - lHint, reinterpret_cast<LPVOID>(const_cast<CAtlList< pair<uint8_t, CString> > *>(pModEntries)) );
}

///////////////////////////////////////////////////////////////////////////////

/**
	  Called to update views when entries are modified.
	Currently, it is only called when entries are moved or renamed (that is to say,
	a form of movement). The possible values for lHint passed to OnUpdate as a LPARAM are:
	 lHint == 0 : Moved
	pModEntries is a pointer to a non-empty ATL::CRBMap< CString, pair<uint8_t, CString> >,
	where the key is the name of the modified entry, and the value is a
	pair<uint8_t, CString> represents the new entry location and whose entry type may be
	tested with the protected method IsEntryTypeDir().
	pModEntries is passed to OnUpdate as a WPARAM.
	When applicable, test the entry type with the protected method IsEntryTypeDir().
	pSendingView specifies the sending view of the document and passed to OnUpdate
	as a pointer to CGrfDocView, a base class of CViewImpl<CGrfView, CGrfDocView>,
	which itself is a base class of CGrfView.
	When pSendingView is 0, OnUpdate receives 0 too, meaning all views shall be updated.
**/
void CGrfDoc::OnEntriesModified(LPARAM lHint, const ATL::CRBMap< CString, pair<uint8_t, CString> > *pModEntries, CGrfDocView *pSendingView)
{
	UpdateAllViews( pSendingView, HINT_DOCUMENT_ENTRIES_MODIFIED - lHint, reinterpret_cast<LPVOID>(const_cast<ATL::CRBMap< CString, pair<uint8_t, CString> > *>(pModEntries)) );
}

///////////////////////////////////////////////////////////////////////////////

	///////////////////////// - Static - /////////////////////////////

CString  CGrfDoc::strFilter;
CString  CGrfDoc::strTypeFilter;
///////////////////////////////////////////////////////////////////////////////

/**
	Retrieves the resource string id associated to the nErrorCode.
	Return IDS_GRFFRM_UNKNOWN_ERROR if no mapped string ID was found.
**/
WORD  CGrfDoc::GetErrorStringResID(int nErrorCode) throw()
{
const static ERRORMESSAGEMAPPER mmap[] =
	{
		{ 0, IDS_LIBGRF_UNKNOWN_ERROR },
		{ 1, IDS_LIBGRF_LIBRARY_ERROR },
		{ 2, IDS_LIBGRF_LIBRARY_FAILURE }
	};
const static unsigned int nMax = sizeof(mmap) / sizeof(mmap[0]);

	for ( unsigned int i = 0; i < nMax; ++i )
	{
		if ( mmap[i].nCode == nErrorCode )
		{
			return mmap[i].wResString;
		}
	}
	return  IDS_LIBGRF_UNKNOWN_ERROR;
}  // GetErrorStringResID()

///////////////////////////////////////////////////////////////////////////////

/**
	Validates a path name according to Windows conventions.
	Returns true if and only if after canonization, strPartName is not empty,
	does not contain bad characters and its length is less than GRF_MAX_PATH.
	If set, pstrCorrected is assigned a valid canonized part name if the part is valid.
**/
bool  CGrfDoc::IsValidPartName(const CString &strPartName, CString *pstrCorrected)
{
	  // TrimLeft() operates on the object on some implementations,
	  // therefore we create a copy
CString strWorkingCopy(strPartName);
	  // If that's the case, TrimLeft() returns a reference and
	  // operator= shall recognize a self-assignment.
	strWorkingCopy = strWorkingCopy.TrimLeft();

	  // Trim the string on the right. Characters to trim are spaces and dots.
	{
	TCHAR t;
	int  iChar = strWorkingCopy.GetLength() - 1;
		while ( !strWorkingCopy.IsEmpty() && ( (t = strWorkingCopy[iChar]) == _T('.') || t == _T(' ') ) )
		{
			// Loop condition: Last character is a period or a whitespace
			strWorkingCopy.Delete(iChar);
			--iChar;
		}
		// Postcondition: file ends with a valid char, or string is empty
	}

	if ( !strWorkingCopy.IsEmpty()
		&& strWorkingCopy.GetLength() < CGrfFileEntry::GRF_MAX_PATH
		&& strWorkingCopy.FindOneOf(_T("<>:\"/\\|?*")) == -1)
	{
		if ( pstrCorrected )
		{
			*pstrCorrected = strWorkingCopy;
		}
		return true;
	}
	return false;
}  // IsValidPartName()

///////////////////////////////////////////////////////////////////////////////

/**
	Returns a (unique) Untitled index.
**/
int  CGrfDoc::GetUntitledIndex()
{
static int s_idxUntitled = 0;
	return ++s_idxUntitled;
}  // GetUntitledIndex()

///////////////////////////////////////////////////////////////////////////////

/**
	Returns true if the specified entry_type is a directory type.
**/
bool  CGrfDoc::IsEntryTypeDir(const uint8_t entry_type)
{
	return entry_type == 2;
}  // IsEntryTypeDir()

///////////////////////////////////////////////////////////////////////////////
