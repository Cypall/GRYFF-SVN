// $Id: DocGrf.h 9 2005-08-12 17:33:08Z Rasqual $

// *
// *   gryff, a GRF archive management utility, src/DocGrf.h
// *   Copyright (C) 2003-2005 Rasqual Twilight
// *
// *   This program is distributed in the hope that it will be useful,
// *   but WITHOUT ANY WARRANTY; without even the implied warranty of
// *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// *

// * DocGrf.h
// * Grf Document manipulation template
// *
// *@author $Author: Rasqual $
// *@version $Revision: 9 $
// *

#ifndef __DOCGRF_H__
#define __DOCGRF_H__

#pragma once

#include  <exception>
#include  <utility>  // pair

#include  <docview.h>

#include  "DocGrfControl.h"
#include  "StringTraits.h"

  // Define update hint constants
#define     HINT_DOCUMENT_MODIFIED        -2
#define     HINT_DOCUMENT_SERIALIZE_STATE -3
#define     HINT_UPDATE_DOCUMENT_PATH     -4
//#define     HINT_DOCUMENT_DIR_CONTENTS    -5
#define     HINT_UPDATE_DOCUMENT_REFRESH  -6

#define     HINT_DOCUMENT_FILES_ADDED     -7
#define     HINT_DOCUMENT_DIRS_ADDED      -8
#define     HINT_DOCUMENT_FILES_AND_DIRS_ADDED     -9

#define     HINT_DOCUMENT_FILES_REMOVED     -10
  // For coherence only. HINT_DOCUMENT_FILES_AND_DIRS_REMOVED is preferred.
#define     HINT_DOCUMENT_DIRS_REMOVED      -11
#define     HINT_DOCUMENT_FILES_AND_DIRS_REMOVED     -12
#define     HINT_DOCUMENT_ENTRIES_MODIFIED  -13
  // UIUpdate (menu and toolbars etc.)
#define     HINT_DOCUMENT_UI_UPDATE_MISC    -14

///////////////////////////////////////////////////////////////////////////////
// Classes in this file:
//
// CGrfDocView
//   typedef for a specialized CView
// CGrfDoc
//   Class instantiating a GRF document.


class CGrfDoc;  // FW declaration

///////////////////////////////////////////////////////////////////////////////

typedef CView<CGrfDoc> CGrfDocView;

///////////////////////////////////////////////////////////////////////////////

class  CGrfDoc :
  public CDocument<CGrfDoc>,
  public CGrfDocControllerImpl<CGrfDoc>
{
	// Type definitions
protected:
	typedef struct _ErrorMessageMapper { int   nCode; WORD  wResString; } ERRORMESSAGEMAPPER;

	///////////////////////// - Static - /////////////////////////////
public:
	//////////////////// - Static Methods - //////////////////////////
	/**
		Retrieves the resource string id associated to the nErrorCode.

	**/
	static WORD  GetErrorStringResID(int nErrorCode) throw();

	/**
		Validates a path name according to Windows conventions.
		Returns true if and only if after canonization, strPartName is not empty,
		does not contain bad characters and its length is less than GRF_MAX_PATH.
		If set, pstrCorrected is assigned a valid canonized part name if the part is valid.
	**/
	static bool  IsValidPartName(const CString &strPartName, CString *pstrCorrected = NULL);

	/**
		Returns a (unique) Untitled index.
	**/
	static int  GetUntitledIndex();

public:
	/**
		Returns true if the specified entry_type is a directory type.
	**/
	static bool  IsEntryTypeDir(const uint8_t entry_type);

public:
	////////////////// - Static Data Members - ///////////////////////
	/**
		List of recognized document extensions for the OpenFileName common dialog.
	**/
	static CString  strFilter;
	/**
		List of recognized extensions for the OpenFileName common dialog.
	**/
	static CString  strTypeFilter;


	///////////////////////// - CTOR / DTOR - /////////////////////////////
public:
	/**
		Performs initialization.
	**/
	CGrfDoc();
	/**
		Destroys document data, walking the directory entries map,
		delete()ing file entries, then delete()ing directory entries.
		Clearing the map is to be done by the map destructor.
	**/
	~CGrfDoc();

	///////////////////////// - data members - /////////////////////////////
protected:
	SortedGrfDirMap      m_GrfMap;  // walkable Tree-Map of directory entries.
	  // Info related to document
	TCHAR                m_strFilePath[MAX_PATH];
	TCHAR                m_strFileName[MAX_PATH];

	///////////////////////// - Methods - /////////////////////////////
public:
	//----- Implement abstract class CDocumentBase (from which CDocument<T> is derived) -----//
	/**
		Implements CDocumentBase::OnNewDocument().
		This is not called directly generally.
		Returns TRUE on success
	**/
	virtual BOOL  OnNewDocument();

	/**
		Implements CDocumentBase::OnOpenDocument()
		This is not called directly generally.
		Returns TRUE on success
	**/
	virtual BOOL  OnOpenDocument(LPCTSTR pDocumentPath);

	/**
		Implements CDocumentBase::SetDocumentName()
		If lpDocumentName is not 0, sets pDocumentName as document name,
		Else document is assigned a new "Untitled" value.
		IsModified specifies whether the document is to be flagged as modified,
		regardless of the pDocumentName value.
		It can be called directly.
		Returns TRUE on success
	**/
	virtual BOOL  SetDocumentName(LPCTSTR pDocumentName, bool IsModified = false);

public:
	//----- other public methods -----//
	/**
		Sets all entries to FROMGRF with the specified document path as source
	**/
	void  SetGrfOrigin(CGrfCache *pCache, LPCTSTR pGrfPath);
public:
	/**
		Unused
	**/
	void  OnCloseDocument();
	/**
		Unused
	**/
	void  DeleteContents();

	/**
		Opens Grf documents and puts their handles into the specified cache.
		If pOutUnresolvedEntries is not 0, failed to open documents are appended to the pointed list
	**/
	bool  CacheHandles(CGrfCache &, CAtlList< std::pair<uint8_t, CString> > *pOutUnresolvedEntries = 0) const;

	/**
		Tests whether there are invalid entries in the document.
		Invalid entries are:
		 . entries with bad characters (charset)
		 . entries whose full path is longer than GRF_MAX_PATH
		If pOutInvalidEntries is not 0, invalid entries are appended to the pointed list
	**/
	bool  HasInvalidEntries(CAtlList< std::pair<uint8_t, CString> > *pOutInvalidEntries = 0) const;

	/**
		Tests whether a directory called strAbsDirName exists in the document
	**/
	bool  DirExists(const CString &strAbsDirName, CGrfDirectoryEntry **ppdirent = 0) const;

	/**
		Tests whether a file called strAbsFileName exists in the document
	**/
	bool  FileExists(const CString &strAbsFileName, CGrfFileEntry **pprecord = 0 ) const;

	/**
		Tests whether pDocumentPath corresponds to this document
	**/
	bool  MatchesPath(LPCTSTR pDocumentPath, bool TestingUntitled = false) const;

	//----- Getters -----//
public:
	/**
		Retrieves a constant pointer to the directory entries map.
		Use this when accessing the map in read-only mode
	**/
	const SortedGrfDirMap * const  GetDirMapPtr() const
	{
		return &m_GrfMap;
	}
	/**
		Retrieves a reference to the directory entries map
		Use this when modifying elements in the map
	**/
	SortedGrfDirMap & GetDirMap()
	{
		return m_GrfMap;
	}
	/**
		Retrieves a constant pointer to the document path.
		If the document is Untitled, the string is empty.
	**/
	const TCHAR * const  GetPathName() const
	{
		return m_strFilePath;
	}
	/**
		Retrieves a constant pointer to the document name.
	**/
	const TCHAR * const  GetName() const
	{
		return m_strFileName;
	}

public:
	//----- Message to Controllers -----//
	/**
		Implements CDocument<T>::OnAllViewsUpdateDone
	**/
	virtual void  OnAllViewsUpdateDone( CGrfDocView* pSender,
	  LPARAM lHint,
	  LPVOID pHint );

	/**
		  Called to update views when entries are added.
		For efficiency reasons, the lHint passed to OnUpdate as a LPARAM may be
		specialized as follows:
		 lHint == 0 : Files
		 lHint == 1 : Directories
		 lHint == 2 : Files & Directories
		pModEntries is a pointer to a non-empty CAtlList< std::pair<uint8_t, CString> >
		which is passed to OnUpdate as a WPARAM.
		When applicable, test the entry type with the protected method IsEntryTypeDir().
		Please note, an entry may be marked as added while in fact it was overwritten.
		pSendingView specifies the sending view of the document and passed to OnUpdate
		as a pointer to CGrfDocView, a base class of CViewImpl<CGrfView, CGrfDocView>,
		which itself is a base class of CGrfView.
		When pSendingView is 0, OnUpdate receives 0 too, meaning all views shall be updated.
	**/
	void  OnEntriesAdded(LPARAM lHint, const CAtlList< std::pair<uint8_t, CString> > *pModEntries, CGrfDocView *pSendingView = 0);

	/**
		  Called to update views when entries are removed.
		For efficiency reasons, the lHint passed to OnUpdate as a LPARAM may be
		specialized as follows:
		 lHint == 0 : Files
		 lHint == 1 : Directories (Do not use)
		 lHint == 2 : Files & Directories
		pModEntries is a pointer to a non-empty CAtlList< std::pair<uint8_t, CString> >
		which is passed to OnUpdate as a WPARAM.
		When applicable, test the entry type with the protected method IsEntryTypeDir().
		pSendingView specifies the sending view of the document and passed to OnUpdate
		as a pointer to CGrfDocView, a base class of CViewImpl<CGrfView, CGrfDocView>,
		which itself is a base class of CGrfView.
		When pSendingView is 0, OnUpdate receives 0 too, meaning all views shall be updated.
	**/
	void  OnEntriesRemoved(LPARAM lHint, const CAtlList< std::pair<uint8_t, CString> > *pModEntries, CGrfDocView *pSendingView = 0);

	/**
		  Called to update views when entries are modified.
		Currently, it is only called when entries are moved or renamed (that is to say,
		a form of movement). The possible values for lHint passed to OnUpdate as a LPARAM are:
		 lHint == 0 : Moved
		pModEntries is a pointer to a non-empty ATL::CRBMap< CString, std::pair<uint8_t, CString> >,
		where the key is the name of the modified entry, and the value is a
		std::pair<uint8_t, CString> represents the new entry location and whose entry type may be
		tested with the protected method IsEntryTypeDir().
		pModEntries is passed to OnUpdate as a WPARAM.
		When applicable, test the entry type with the protected method IsEntryTypeDir().
		pSendingView specifies the sending view of the document and passed to OnUpdate
		as a pointer to CGrfDocView, a base class of CViewImpl<CGrfView, CGrfDocView>,
		which itself is a base class of CGrfView.
		When pSendingView is 0, OnUpdate receives 0 too, meaning all views shall be updated.
	**/
	void  OnEntriesModified(LPARAM lHint, const ATL::CRBMap< CString, std::pair<uint8_t, CString> > *pModEntries, CGrfDocView *pSendingView = 0);

};



#endif // __DOCGRF_H__
