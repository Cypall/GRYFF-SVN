// $Id: DocGrfControl.h 12 2005-08-14 01:42:51Z Rasqual $

// *
// *   gryff, a GRF archive management utility, src/DocGrfControl.h
// *   Copyright (C) 2003, 2004, 2005 Rasqual Twilight
// *
// *   This program is distributed in the hope that it will be useful,
// *   but WITHOUT ANY WARRANTY; without even the implied warranty of
// *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// *

// * DocGrfControl.h
// * Grf Document controller
// *
// *@author $Author: Rasqual $
// *@version $Revision: 12 $
// *

#ifndef __DOCGRFCONTROL_H__
#define __DOCGRFCONTROL_H__

#pragma once

#include  "GrfDesc.h"
#include  "gen_grfio.h"

///////////////////////////////////////////////////////////////////////////////
// Classes in this file:
//
// CGrfDocControllerImpl<T>
//   Base class to manipulate a document


template<class T>
class  CGrfDocControllerImpl
{
	// Type definitions
public:
	typedef int (*ResolutionCallback)(CGrfFileEntry *existing, CGrfFileEntry *replacement);
	///////////////////////// - CTOR / DTOR - /////////////////////////////
public:
	CGrfDocControllerImpl()
	{}
	~CGrfDocControllerImpl()
	{}

	///////////////////////// - data members - /////////////////////////////
protected:

	///////////////////////// - Methods - /////////////////////////////
public:
	//----- methods: document management -----//
	/**
		  Creates a directory, creating parent directories recursively if necessary.
		pAbsolutePath contains an path relative to the root of the document,
		with path components being separated by a backslash character. The string
		shall not being or end with a backslash.
		The function succeeds if the directory already exists.
		If pOutCreatedEntries is not 0, created entries are appended to the pointed list
		Returns: a pointer to the requested directory entry
	**/
	CGrfDirectoryEntry * MkdirRecursive(LPCTSTR pAbsolutePath, CAtlList< std::pair<uint8_t, CString> > *pOutCreatedEntries = 0)
	{
	T* pThis = static_cast<T*>(this);
	CGrfDirectoryEntry *pdirent;

		  // if requested dir is not present, create it
		if ( pThis->GetDirMap().Lookup(pAbsolutePath, pdirent) == false )
		{
			pdirent = new CGrfDirectoryEntry(pAbsolutePath);
			pThis->GetDirMap().SetAt( pAbsolutePath, pdirent );

			  // if created dir is root, stop recursion
			  // otherwise, create parent dir if necessary
			if ( *pAbsolutePath != _T('\0') )
			{
				CString strPath(pAbsolutePath), dn;
				::BreakPath(strPath, dn, strPath);
				pThis->MkdirRecursive(dn, pOutCreatedEntries);  // no need to hold results
			}
			if ( pOutCreatedEntries )
			{
				  // Put after recursive call. This way, parents are added to the list before children are.
				pOutCreatedEntries->AddTail(pair<uint8_t, CString>(2, CString(pAbsolutePath)));
			}
		}
		  // return either created or existing directory entry
		return pdirent;
	}  // CGrfDocControllerImpl::MkdirRecursive

	/**
		  Add a file entry to the document with its origin specified with nRecordType.
		If nRecordType is FROMFS, pFilePath is the file system absolute path to the file,
		and the file is inserted at <pInsertDir> and if pGrfPathOrFsForceName is not 0,
		the name is used for the entry.
		If nRecordType is FROMGRF, pFilePath is the absolute path inside the grf file whose
		file system absolute path is pGrfPathOrFsForceName. The file is inserted at <pInsertDir\pFilePath>,
		that is to say the internal structure of pGrfPath is preserved.
		pInsertDir shall not being or end with a backslash.
		To add a directory record, please use MkdirRecursive_()
		If pOutCreatedEntries is not 0, created entries are appended to the pointed list
		Returns: a pointer to the added file entry
	**/
	CGrfFileEntry * AddRecord(LPCTSTR pFilePath,
	  LPCTSTR pInsertDir,
	  bool PreserveDirLayout,
	  enum CGrfFileEntry::FILEORIG nRecordType=CGrfFileEntry::FROMFS,
	  LPCTSTR pGrfPathOrFsForceName = 0,
	  CAtlList< std::pair<uint8_t, CString> > *pOutCreatedEntries = 0)
	{
	CGrfDirectoryEntry *pdirent;
	CGrfFileEntry *precord;
	CString  dn, fn, internal_fn;

		::BreakPath(CString(pFilePath), dn, fn);
		if ( !PreserveDirLayout )
		{
			dn.Empty();
		}
		if ( nRecordType == CGrfFileEntry::FROMFS )
		{
			dn = pInsertDir;
			if ( pGrfPathOrFsForceName )
			{
				fn = pGrfPathOrFsForceName;
			}
		}
		else
		if ( *pInsertDir != _T('\0') )  // prepend before adding GRF rel. path
		{
			if ( dn.IsEmpty() )
			{
				dn = pInsertDir;
			}
			else
			{
				dn = CString(pInsertDir) + _T("\\") + dn;
			}
		} // otherwise, keep dn obtained from imported grf

		// If no directory entry exists, create it. Otherwise, get existing one.
		pdirent = this->MkdirRecursive(dn, pOutCreatedEntries);

		if ( pdirent->files.Lookup(fn, precord) != false )
		{
			// Replace existing entry
			delete precord;
		}

		precord = new CGrfFileEntry(pFilePath, nRecordType, nRecordType == CGrfFileEntry::FROMFS ? 0 : pGrfPathOrFsForceName);

		pdirent->files.SetAt( fn, precord );
		if ( pOutCreatedEntries )
		{
			pOutCreatedEntries->AddTail(pair<uint8_t, CString>(1, dn.IsEmpty()? fn : dn + _T("\\") + fn));
		}

		return precord;
	}  // CGrfDocControllerImpl::AddRecord

	/**
		  Imports the contents of a Grf file into the current document.
		pAbsoluteGrfMountDir specifies the directory of the local document
		where the contents of the imported document are "mounted".
		If pOutCreatedEntries is not 0, created entries are appended to the pointed list
		nOverwriteResolution and ResolutionCb are reserved parameters and shall be 0.
		Returns: number of imported entries
		Throws: an GrfImportError structure if import failed
	**/
	int  ImportGrf(LPCTSTR pGrfPath,
	  LPCTSTR pAbsoluteGrfMountDir = _T(""),
	  CAtlList< std::pair<uint8_t, CString> > *pOutCreatedEntries = 0,
	  int nOverwriteResolution = 0,
	  ResolutionCallback ResolutionCb = 0) throw(Gen::GrfImportError)
	{
	openkore::Grf *pGrf;
	openkore::GrfError errorcode;
	int nImportedEntries(0);
	CAtlList< pair<uint8_t,CString> > InvalidEntries;
	CAtlList< pair< pair<uint32_t,uint8_t>,CString> > ValidEntries;
	WCHAR EntryName[CGrfFileEntry::GRF_MAX_PATH];

		try
		{
			pGrf = Gen::GrfOpen(pGrfPath, &errorcode);
		}
		catch(...)
		{
		Gen::GrfImportError gie;
			gie.code = Gen::GrfImportError::LIBRARY_FAILURE;
			throw gie;
		}
		if ( pGrf == 0 )
		{
		Gen::GrfImportError gie;
			gie.code = Gen::GrfImportError::LIBRARY_ERROR;
			gie.message = openkore::grf_strerror(errorcode);
			throw gie;
		}

		if ( pGrf->nfiles == 0 )
		{
			Gen::GrfClose(pGrf);
			return 0;
		}

		openkore::GrfFile  *entry;
		for ( uint32_t i = 0;
		  i < pGrf->nfiles;
		  ++i )
		{
			entry = pGrf->files + i;
			::MultiByteToWideChar(0x3B5, 0, entry->name, -1, EntryName, CGrfFileEntry::GRF_MAX_PATH);
			CString strEntryName(EntryName), dn;
			bool isValid(true);
			  // Verify every part of the name
			do
			{
				::BreakPath(strEntryName, dn, strEntryName);
				if ( !T::IsValidPartName(strEntryName) )
				{
					InvalidEntries.AddTail( pair<uint8_t,CString>(entry->flags & 0x03, CString(EntryName)) );
					isValid = false;
					break;
				}
				strEntryName = dn;
			} while(!dn.IsEmpty());
			if ( isValid )
			{
				ValidEntries.AddTail( pair< pair<uint32_t,uint8_t>,CString>(pair<uint32_t,uint8_t>(i, entry->flags & 0x03), CString(EntryName)) );
			}
		}

		if ( !InvalidEntries.IsEmpty() )
		{
			static int i = 0;
			++i;
		}
		CString strTargetDir;
		for ( POSITION pos = ValidEntries.GetHeadPosition();
		  pos;
		)
		{
			++nImportedEntries;
			pair< pair<uint32_t,uint8_t>,CString> p( ValidEntries.GetNext(pos) );
			  // Directory
			if ( T::IsEntryTypeDir(p.first.second) )
			{
				::CombinePath(strTargetDir, pAbsoluteGrfMountDir, p.second);
				this->MkdirRecursive(strTargetDir, pOutCreatedEntries);
			}
			else
			{
				  // p.second == abspath in grf
				  // pAbsoluteGrfMountDir == where the grf is mounted.
				  // AddRecord glues the parts together by itself
			CGrfFileEntry *precord = this->AddRecord(p.second, pAbsoluteGrfMountDir, true, CGrfFileEntry::FROMGRF, pGrfPath, pOutCreatedEntries);
				memcpy(precord->in_grf_path, pGrf->files[p.first.first].name, CGrfFileEntry::GRF_MAX_PATH);
				precord->setSizes(pGrf->files[p.first.first].real_len, pGrf->files[p.first.first].compressed_len);
			}
		}

		{
			  // This code used when Unicode files are hardlinked to a temp file
			  // and left on the disk b/c temp file deletion failed (file being fopen()ed by libgrf)
			CString strGrfPath(CA2T(pGrf->filename)), foo, strFileName;
			::BreakPath(strGrfPath, foo, strFileName);

			Gen::GrfClose(pGrf);
			pGrf = 0;
			if ( strFileName[0] == ::szTempArchivePrefix[0] && strFileName.Right(4) == _T(".tmp") )
			{
				::DeleteFile(strGrfPath);
			}
		}
		return nImportedEntries;
	}  // CGrfDocControllerImpl::ImportGrf

	/**
		  Imports the selected files into the current document.
		All specified files are supposed to exist.
		pAbsoluteGrfMountDir specifies the directory of the local document
		where the contents of the imported tree are "mounted".
		If a directory is empty, an empty entry is created [[user option]]
		If pOutCreatedEntries is not 0, created entries are appended to the pointed list
		nOverwriteResolution and ResolutionCb are reserved parameters and shall be 0.
		Returns: number of imported entries
	**/
	LRESULT  ImportFiles(CAtlList< std::pair<uint8_t, CString> > *pFiles,
	  LPCTSTR pAbsoluteGrfMountDir = _T(""),
	  CAtlList< std::pair<uint8_t, CString> > *pOutCreatedEntries = 0,
	  int nOverwriteResolution = 0,
	  ResolutionCallback ResolutionCb = 0)
	{
	CGrfFileEntry *precord;
	CString  dn, fn, strPath;
	int nImportedEntries(0);
	POSITION pos = pFiles->GetHeadPosition();
		while ( pos )
		{
			pair<uint8_t, CString> p(pFiles->GetNext(pos));
			if ( p.first & 0x00000040 )
			{
			unsigned int  i, len = p.second.GetLength();
			CAutoVectorPtr<char> data(new char[len+1]);

				for ( i = 0U; i < len; ++i )
				{
					data[i] = static_cast<char>(p.second.GetAt(i) & 0xFF);
				}
				data[i] = 0;
				::MultiByteToWideChar(0x3B5, 0, data, -1, strPath.GetBufferSetLength(len + 1), len + 1);
				strPath.ReleaseBuffer();
				::BreakPath(strPath, dn, fn);
				precord = this->AddRecord(p.second, pAbsoluteGrfMountDir, true, CGrfFileEntry::FROMFS, fn, pOutCreatedEntries);
				ATLASSERT(precord);
				++nImportedEntries;
			}
			else
			{
				precord = this->AddRecord(p.second, pAbsoluteGrfMountDir, true, CGrfFileEntry::FROMFS, NULL, pOutCreatedEntries);
				ATLASSERT(precord);
				++nImportedEntries;
			}
			  // Set file size from FS
		HANDLE  hFile = ::CreateFile(p.second, 0, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if ( hFile != INVALID_HANDLE_VALUE )
			{
			DWORD  filesize = ::GetFileSize(hFile, NULL);
				if ( filesize != INVALID_FILE_SIZE )
				{
					precord->setSizes(filesize);
				}
				::CloseHandle(hFile);
			}
		}//while pos
		return nImportedEntries;
	}  // CGrfDocControllerImpl::ImportFiles

	/**
		  Imports the contents of a filesystem directory into the current document.
		The directory itself is not imported, only its files and subdirectories are.
		pAbsoluteGrfMountDir specifies the directory of the local document
		where the contents of the imported tree are "mounted".
		bUseEncodingConversion specifies whether the names are to be interpreted as CP-949,
		If a directory is empty, an empty entry is created [[user option]]
		If pOutCreatedEntries is not 0, created entries are appended to the pointed list
		nOverwriteResolution and ResolutionCb are reserved parameters and shall be 0.
		Returns: number of imported entries
	**/
	LRESULT  ImportDirectoryTree(LPCTSTR pDirectoryPath,
	  LPCTSTR pAbsoluteGrfMountDir = _T(""),
	  bool bUseEncodingConversion = false,
	  CAtlList< std::pair<uint8_t, CString> > *pOutCreatedEntries = 0,
	  int nOverwriteResolution = 0,
	  ResolutionCallback ResolutionCb = 0)
	{
		return  this->AddDirectoryContents_(pDirectoryPath, pAbsoluteGrfMountDir, _T(""), bUseEncodingConversion, pOutCreatedEntries, nOverwriteResolution, ResolutionCb);
	}  // CGrfDocControllerImpl::ImportDirectoryTree

protected:
	/**
		  Adds the contents of a directory recursively.
		The source directory is <pDirectoryPath\\pRelativeSubDir>
		The target directory is <pAbsoluteGrfMountDir\\pRelativeSubDir>
		bUseEncodingConversion specifies whether the names are to be interpreted as CP-949,
		If pOutCreatedEntries is not 0, created entries are appended to the pointed list
		nOverwriteResolution and ResolutionCb are reserved parameters and shall be 0.
	**/
	LRESULT  AddDirectoryContents_(LPCTSTR pDirectoryPath,
	  LPCTSTR pAbsoluteGrfMountDir,
	  LPCTSTR pRelativeSubDir,
	  bool bUseEncodingConversion,
	  CAtlList< std::pair<uint8_t, CString> > *pOutCreatedEntries,
	  int nOverwriteResolution = 0,
	  ResolutionCallback ResolutionCb = 0)
	{
	T* pThis = static_cast<T*>(this);
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	LRESULT records = 0;
	BOOL mayContinue = TRUE;
	CString strPath;
	CString strFindLocation;

		strFindLocation.Format(_T("%s\\%s"), pDirectoryPath, pRelativeSubDir);
		if ( strFindLocation.GetAt(strFindLocation.GetLength()-1) != _T('\\') )
		{
			strFindLocation += _T("\\");
		}

		if ( bUseEncodingConversion )
		{
			::AnsiToUnicode(strPath, pRelativeSubDir);
		}

	CString strRelIns;
		  // Target dir: where to "mount" the file
		if ( *pAbsoluteGrfMountDir != _T('\0') )  // Append to cwd if applicable
		{
			if ( *pRelativeSubDir == _T('\0') )  // How deep in recursion?
			{
				strRelIns = pAbsoluteGrfMountDir;  // 1st level
			}
			else
			{
				if ( bUseEncodingConversion )
				{
					strRelIns.Format(_T("%s\\%s"), pAbsoluteGrfMountDir, strPath);
				}
				else
				{
					strRelIns.Format(_T("%s\\%s"), pAbsoluteGrfMountDir, pRelativeSubDir);
				}
			}
		}
		else
		{
			if ( bUseEncodingConversion )
			{
				strRelIns = strPath;
			}
			else
			{
				strRelIns = pRelativeSubDir;
			}
		}

		// Ensure the directory has been created, even if empty
		// (Assumes only one directory is created at a time,
		// since we are before going deeper into recursion)
		if ( !pThis->DirExists(strRelIns) )
		{
			this->MkdirRecursive(strRelIns, pOutCreatedEntries);
			++records;
		}

	CString strDirSpec(strFindLocation);
		strDirSpec += _T("*");

		for ( hFind = ::FindFirstFile(strDirSpec, &FindFileData);
			  mayContinue && hFind != INVALID_HANDLE_VALUE;
			  mayContinue = ::FindNextFile(hFind, &FindFileData) )
		{
			if ( (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
			{
			CGrfFileEntry * precord(NULL);
			CString  strAddedPath = strFindLocation + FindFileData.cFileName;
				if ( bUseEncodingConversion )
				{
					::AnsiToUnicode(strPath, FindFileData.cFileName);
					precord = this->AddRecord(strAddedPath, strRelIns, true, CGrfFileEntry::FROMFS, strPath, pOutCreatedEntries);
				}
				else
				{
					precord = this->AddRecord(strAddedPath, strRelIns, true, CGrfFileEntry::FROMFS, 0, pOutCreatedEntries);
				}
				if ( 0 != precord )
				{
				HANDLE  hFile = ::CreateFile(strAddedPath, 0, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
					if ( hFile != INVALID_HANDLE_VALUE )
					{
					DWORD  filesize = ::GetFileSize(hFile, NULL);
						if ( filesize != INVALID_FILE_SIZE )
						{
							precord->setSizes(filesize);
						}
						::CloseHandle(hFile);
					}
					++records;
				}
			}
			else if ( lstrcmp(FindFileData.cFileName, _T(".")) != 0 && lstrcmp(FindFileData.cFileName, _T("..")) != 0 )
			{
				CString strTargetSubDir;
				if ( *pRelativeSubDir == _T('\0') )
				{
					strTargetSubDir = FindFileData.cFileName;
				}
				else
				{
					strTargetSubDir.Format(_T("%s\\%s"), pRelativeSubDir, FindFileData.cFileName);
				}
				records += this->AddDirectoryContents_(pDirectoryPath, pAbsoluteGrfMountDir, strTargetSubDir, bUseEncodingConversion, pOutCreatedEntries, nOverwriteResolution, ResolutionCb);
			}
		}
		DWORD dwError = ::GetLastError();
		if (dwError == ERROR_NO_MORE_FILES)
		{
			::FindClose(hFind);
		}
		else
		{
		CString strErrMessage;
			strErrMessage.Format(_T("FindNextFile error. Error is %u"), dwError);
			::MessageBox(HWND_DESKTOP, strErrMessage,0,0);
			if ( hFind != INVALID_HANDLE_VALUE )
			{
				::FindClose(hFind);
			}
		}
		return records;
	}  // CGrfDocControllerImpl::AddDirectoryContents_

public:
	/**
		  Imports the contents of a directory from another document into the current document.
		The directory itself is not imported, only its files and subdirectories are.
		pAbsoluteGrfMountDir specifies the directory of the local document
		where the contents of the imported tree are "mounted".
		bUseEncodingConversion specifies whether the names are to be interpreted as CP-949,
		If a directory is empty, an empty entry is created [[user option]]
		If pOutCreatedEntries is not 0, created entries are appended to the pointed list
		nOverwriteResolution and ResolutionCb are reserved parameters and shall be 0.
		Returns: number of imported entries
	**/
	LRESULT  ImportDirectoryEntry(T *pOtherDoc,
	  CGrfDirectoryEntry *pOtherDir,
	  LPCTSTR pAbsoluteGrfMountDir = _T(""),
	  CAtlList< std::pair<uint8_t, CString> > *pOutCreatedEntries = 0,
	  int nOverwriteResolution = 0,
	  ResolutionCallback ResolutionCb = 0)
	{
		  // Use recursive method to add
		return  this->AddDirectoryEntry_(pOtherDoc, pOtherDir, pAbsoluteGrfMountDir, _T(""), pOutCreatedEntries, nOverwriteResolution, ResolutionCb);
	}  // CGrfDocControllerImpl::ImportDirectoryEntry

protected:
	/**
		  Adds the contents of a directory from another document recursively.
		The source directory is <pOtherDir->dir\\pRelativeSubDir>
		The target directory is <pAbsoluteGrfMountDir\\pRelativeSubDir>
		If pOutCreatedEntries is not 0, created entries are appended to the pointed list
		nOverwriteResolution and ResolutionCb are reserved parameters and shall be 0.
		Returns: number of imported entries for specified subdir
	**/
	LRESULT  AddDirectoryEntry_(T *pOtherDoc,
	  CGrfDirectoryEntry *pOtherDir,
	  LPCTSTR pAbsoluteGrfMountDir,
	  LPCTSTR pRelativeSubDir,
	  CAtlList< std::pair<uint8_t, CString> > *pOutCreatedEntries,
	  int nOverwriteResolution = 0,
	  ResolutionCallback ResolutionCb = 0)
	{
	T* pThis = static_cast<T*>(this);
	LRESULT records = 0;
	BOOL mayContinue = TRUE;
	CString strName;
	CString strFindLocation;

		if ( *pRelativeSubDir != _T('\0') )
		{
			strFindLocation.Format(_T("%s\\%s"), pOtherDir->dir, pRelativeSubDir);
		}
		else
		{
			strFindLocation = pOtherDir->dir;
		}

	CString strRelIns;
		  // Target dir: where to "mount" the file
		if ( *pAbsoluteGrfMountDir != _T('\0') )  // Append to cwd if applicable
		{
			if ( *pRelativeSubDir == _T('\0') )  // How deep in recursion?
			{
				strRelIns = pAbsoluteGrfMountDir;  // 1st level
			}
			else
			{
				strRelIns.Format(_T("%s\\%s"), pAbsoluteGrfMountDir, pRelativeSubDir);
			}
		}
		else
		{
			strRelIns = pRelativeSubDir;
		}

		// Ensure the directory has been created, even if empty
		// (Assumes only one directory is created at a time,
		// since we are before going deeper into recursion)
		if ( !pThis->DirExists(strRelIns) )
		{
			this->MkdirRecursive(strRelIns, pOutCreatedEntries);
			++records;
		}

	CGrfDirectoryEntry * pdirent;
	CGrfFileEntry * precord, * precord_dupe;
		ATLVERIFY(pOtherDoc->GetDirMapPtr()->Lookup(strFindLocation, pdirent));

	CString  strBase(strFindLocation);
		if ( !strFindLocation.IsEmpty() )
		{
			strBase += _T('\\');
		}

		  // Do files
		for ( POSITION filepos = pdirent->files.GetHeadPosition(); filepos; pdirent->files.GetNext(filepos) )
		{
			pdirent->files.GetAt(filepos, strName, precord);
		CString strAddedPath(strBase);
			strAddedPath += strName;
			  // Add record to specified Insert location, without preserving directory tree (just filename)
			if ( 0 != (precord_dupe = this->AddRecord(strAddedPath, strRelIns, false, precord->origin, precord->grf_src? precord->grf_src->operator LPCTSTR() : 0, pOutCreatedEntries)) )
			{
				strncpy(precord_dupe->in_grf_path, precord->in_grf_path, CGrfFileEntry::GRF_MAX_PATH);
				precord_dupe->in_grf_path[CGrfFileEntry::GRF_MAX_PATH-1] = 0;
				precord_dupe->setSizes(precord->fullsize, precord->compsize);
				++records;
			}
		}

		// Do subdirs
	POSITION pos = pThis->GetDirMap().FindFirstKeyAfter(strFindLocation);
	CString  strDirectory, strParentDir;
		ATLASSERT( pos != 0 );  // "FindFirstKeyAfter" returns the key corresponding to strFindLocation actually, not the one right after it
	CString strTargetSubDir;
		for ( pThis->GetDirMap().GetNext(pos); pos; pThis->GetDirMap().GetNext(pos) )
		{
			pThis->GetDirMap().GetAt(pos, strDirectory, pdirent);
			::BreakPath(strDirectory, strParentDir, strName);
			if ( strParentDir != strFindLocation )
			{
				break;
			}
			if ( *pRelativeSubDir == _T('\0') )
			{
				strTargetSubDir = strName;
			}
			else
			{
				strTargetSubDir.Format(_T("%s\\%s"), pRelativeSubDir, strName);
			}
			records += this->AddDirectoryEntry_(pOtherDoc, pOtherDir, pAbsoluteGrfMountDir, strTargetSubDir, pOutCreatedEntries, nOverwriteResolution, ResolutionCb);
		}
		return records;
	}  // CGrfDocControllerImpl::AddDirectoryEntry_

public:
	/**
		  Deletes a directory, removing its files and sub-directories recursively if necessary.
		pAbsolutePath contains an path relative to the root of the document,
		with path components being separated by a backslash character. The string
		shall not being or end with a backslash.
		The function succeeds if the directory already exists.
		If pOutRemovedEntries is not 0, deleted entries are appended to the pointed list
		Returns the number of removed entries, including directories.
	**/
	LRESULT  RmdirRecursive(LPCTSTR pAbsolutePath, CAtlList< std::pair<uint8_t, CString> > *pOutRemovedEntries = 0)
	{
	T* pThis = static_cast<T*>(this);
	CGrfDirectoryEntry    *pdirent;
	CAtlList< pair<uint8_t, CString> > DeleteEntries;
	CString                strDirectory, strName, strCompleteName;

	POSITION pos = pThis->GetDirMap().FindFirstKeyAfter(CString(pAbsolutePath));
		ATLASSERT( pos != 0 );
		pThis->GetDirMap().GetAt(pos, strDirectory, pdirent);
		if ( !strDirectory.IsEmpty() )
		{
			DeleteEntries.AddTail(pair<uint8_t,CString>(2, strDirectory));
		}
		for ( POSITION pos2 = pdirent->files.GetTailPosition(); pos2; pdirent->files.GetPrev(pos2) )
		{
			strName = pdirent->files.GetKeyAt(pos2);
			if ( strDirectory.IsEmpty() )
			{
				strCompleteName = strName;
			}
			else
			{
				strCompleteName.Format(_T("%s\\%s"), strDirectory, strName);
			}
			DeleteEntries.AddTail(pair<uint8_t,CString>(1, strCompleteName));
		}
	CString strBaseDir(strDirectory);
		if ( !strDirectory.IsEmpty() )
		{
			strBaseDir += _T("\\");
		}

		for ( pThis->GetDirMap().GetNext(pos); pos; pThis->GetDirMap().GetNext(pos) )
		{
			pThis->GetDirMap().GetAt(pos, strDirectory, pdirent);
			if ( strDirectory.GetLength() <= strBaseDir.GetLength()
				|| strDirectory.Left(strBaseDir.GetLength()) != strBaseDir )
			{
				break;
			}

			DeleteEntries.AddTail(pair<uint8_t,CString>(2, strDirectory));
			for ( POSITION pos2 = pdirent->files.GetTailPosition(); pos2; pdirent->files.GetPrev(pos2) )
			{
				strName = pdirent->files.GetKeyAt(pos2);
				if ( strDirectory.IsEmpty() )
				{
					strCompleteName = strName;
				}
				else
				{
					strCompleteName.Format(_T("%s\\%s"), strDirectory, strName);
				}
				DeleteEntries.AddTail(pair<uint8_t,CString>(1, strCompleteName));
			}
		}
		if ( !DeleteEntries.IsEmpty() )
		{
			return this->RemoveEntries(&DeleteEntries, pOutRemovedEntries);
		}
		return 0;
	}  // CGrfDocControllerImpl::RmdirRecursive


	/**
		  Removes entries from the document, given a list of entries.
		Entries are deleted from end to beginning of list, assuming files come before
		container directory in the walking order.
		When a directory is deleted, the list must be ordered so that subitems are placed
		prior to that directory.
		If pOutRemovedEntries is not 0, deleted entries are appended to the pointed list
		nOverwriteResolution and ResolutionCb are reserved parameters and shall be 0.
		Returns: number of imported entries, which should match the count of the list.
		Throws: an GrfImportError structure if import failed
	**/
	// FIXME: TODO: (1.3) Copy/Move conflicts resolution callbacks (-> replace existing entries?)
	LRESULT  RemoveEntries(const CAtlList< std::pair<uint8_t, CString> > *pInvalidEntries, CAtlList< std::pair<uint8_t, CString> > *pOutRemovedEntries = 0)
	{
	T* pThis = static_cast<T*>(this);
	CString                strDirectory, strName, strCompleteName;
	CGrfDirectoryEntry    *pdirent;
	CGrfFileEntry         *precord;
	LRESULT                RemovedEntriesCount(0);

		ATLASSERT(pInvalidEntries != 0);
		if ( pInvalidEntries->IsEmpty() )
		{
			return 0;
		}
		for ( POSITION pos = pInvalidEntries->GetTailPosition();
			pos;
		)
		{
			pair<uint8_t, CString> p(pInvalidEntries->GetPrev(pos));
			if ( p.first == 2 )
			{
				pThis->GetDirMap().Lookup(p.second, pdirent);
				delete pdirent;
				pThis->GetDirMap().RemoveKey(p.second);
				++RemovedEntriesCount;
				if ( pOutRemovedEntries )
				{
					pOutRemovedEntries->AddTail(p);
				}
			}
			else
			{
			CString dn, fn;
				::BreakPath(p.second, dn, fn);
				pThis->GetDirMap().Lookup(dn, pdirent);
				pdirent->files.Lookup(fn, precord);
				delete precord;
				pdirent->files.RemoveKey(fn);
				++RemovedEntriesCount;
				if ( pOutRemovedEntries )
				{
					pOutRemovedEntries->AddTail(p);
				}
			}
		}
		return RemovedEntriesCount;
	}  // CGrfDocControllerImpl::RemoveEntries

	/**
		  Lists the contents of a directory, including its files and sub-directories
		recursively if necessary.
		pAbsolutePath contains an path relative to the root of the document,
		with path components being separated by a backslash character. The string
		shall not being or end with a backslash.
		pOutListedEntries cannot be 0 and points to a list that will receive the listing.
		Returns the number of removed entries, including directories.
	**/
	LRESULT  Populate(LPCTSTR pAbsolutePath, CAtlList< std::pair<uint8_t, CString> > *pOutListedEntries)
	{
		T* pThis = static_cast<T*>(this);
		CGrfDirectoryEntry    *pdirent;
		CString                strDirectory, strName, strCompleteName;

		POSITION pos = pThis->GetDirMap().FindFirstKeyAfter(CString(pAbsolutePath));
		ATLASSERT( pos != 0 );
		pThis->GetDirMap().GetAt(pos, strDirectory, pdirent);
		if ( !strDirectory.IsEmpty() )
		{
			pOutListedEntries->AddTail(pair<uint8_t,CString>(2, strDirectory));
		}
		for ( POSITION pos2 = pdirent->files.GetTailPosition(); pos2; pdirent->files.GetPrev(pos2) )
		{
			strName = pdirent->files.GetKeyAt(pos2);
			if ( strDirectory.IsEmpty() )
			{
				strCompleteName = strName;
			}
			else
			{
				strCompleteName.Format(_T("%s\\%s"), strDirectory, strName);
			}
			pOutListedEntries->AddTail(pair<uint8_t,CString>(1, strCompleteName));
		}
		CString strBaseDir(strDirectory);
		if ( !strDirectory.IsEmpty() )
		{
			strBaseDir += _T("\\");
		}

		for ( pThis->GetDirMap().GetNext(pos); pos; pThis->GetDirMap().GetNext(pos) )
		{
			pThis->GetDirMap().GetAt(pos, strDirectory, pdirent);
			if ( strDirectory.GetLength() <= strBaseDir.GetLength()
				|| strDirectory.Left(strBaseDir.GetLength()) != strBaseDir )
			{
				break;
			}

			pOutListedEntries->AddTail(pair<uint8_t,CString>(2, strDirectory));
			for ( POSITION pos2 = pdirent->files.GetTailPosition(); pos2; pdirent->files.GetPrev(pos2) )
			{
				strName = pdirent->files.GetKeyAt(pos2);
				if ( strDirectory.IsEmpty() )
				{
					strCompleteName = strName;
				}
				else
				{
					strCompleteName.Format(_T("%s\\%s"), strDirectory, strName);
				}
				pOutListedEntries->AddTail(pair<uint8_t,CString>(1, strCompleteName));
			}
		}
		return (LRESULT)(pOutListedEntries->GetCount());
	}  // CGrfDocControllerImpl::RmdirRecursive

};



#endif // __DOCGRFCONTROL_H__
