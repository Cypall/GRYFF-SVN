// $Id: fnmanip.h 12 2005-08-14 01:42:51Z Rasqual $

// *
// *   gryff, a GRF archive management utility, src/fnmanip.h
// *   Copyright (C) 2003, 2004 Rasqual Twilight
// *
// *   This program is distributed in the hope that it will be useful,
// *   but WITHOUT ANY WARRANTY; without even the implied warranty of
// *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// *

// * fnmanip.h
// * Miscellaneous file-related utilities.
// *
// *@author $Author: Rasqual $
// *@version $Revision: 12 $
// *

#ifndef __FNMANIP_H__
#define __FNMANIP_H__

#if defined(_WTL_NO_CSTRING)
#ifndef __ATLSTR_H__
   #error fnmanip.h requires <atlstr.h> to be included first
#endif
#endif

#if !defined(NO_OFN_DIALOG)
#ifndef __ATLDLGS_H__
   #error fnmanip.h requires <atldlgs.h> to be included first
#endif
#endif

void BreakPath(const CString& strPath, CString& strDir, CString& strName);
CString &CombinePath(CString& strCombinedPath, LPCTSTR pDir, LPCTSTR pName);
void GetTopDir(const CString& strPath, CString& strDir, CString& strRemaining);
bool IsSubDir(const CString& strPseudoSub, const CString& strPseudoParent);
CString &AnsiToUnicode(CString& strDest, LPCTSTR pAnsiEncoded);
HANDLE StartApplication(LPCTSTR path, HWND parentWindow);

class CTempFile
{
	CString m_strFileName;

public:
	CTempFile(LPCTSTR lpFileName)
	{
		if ( lpFileName )
		{
			m_strFileName = lpFileName;
		}
	}

	void Attach(LPCTSTR lpFileName)
	{
		if ( lpFileName == 0 || m_strFileName != lpFileName )
		{
			Detach();
		}
		if ( lpFileName )
		{
			m_strFileName = lpFileName;
		}
	}

	void Detach()
	{
		if ( !m_strFileName.IsEmpty() )
		{
			::DeleteFile(m_strFileName);
		}
		m_strFileName.Empty();
	}

	~CTempFile()
	{
		Detach();
	}
};

class CTempDirectory
{
	CString m_strDirName;
	bool m_DeleteOnDestroy;

public:
	CTempDirectory() : m_DeleteOnDestroy(false)
	{}

	~CTempDirectory()
	{
		Delete();
	}

	const CString & GetPath() const
	{
		return m_strDirName;
	}

	void  EnableDeleting(bool Enable = true)
	{
		m_DeleteOnDestroy = Enable;
	}

	void  DisableDeleting()
	{
		EnableDeleting(false);
	}

	bool  Create(LPCTSTR prefix)
	{
		Delete();
		return (m_DeleteOnDestroy = CTempDirectory::CreateTempDirectory(prefix, m_strDirName)); 
	}

	bool  Delete()
	{
		if ( m_DeleteOnDestroy )
		{
			m_DeleteOnDestroy = !CTempDirectory::RemoveDirectoryWithSubItems(m_strDirName);
		}
		return (!m_DeleteOnDestroy);
	}

	static bool  CreateTempDirectory(LPCTSTR prefix, CString &dirName)
	{
		TCHAR  TempPath[MAX_PATH], TempDirPath[MAX_PATH];
		::GetTempPath(MAX_PATH, TempPath);
		if ( !::GetTempFileName(TempPath, prefix, 0, TempDirPath) )
		{
			return false;
		}
		if ( !::DeleteFile(TempDirPath) )
		{
			return false;
		}
		if ( FALSE != ::CreateDirectory(TempDirPath, NULL) )
		{
			dirName = TempDirPath;
			return true;
		}
		return false;
	}

	static bool  RemoveDirectoryWithSubItems(const CString &DirPath)
	{
		WIN32_FIND_DATA  FindFileData;
		HANDLE  hFind;
		BOOL  mayContinue = TRUE;
		TCHAR  FindExpr[MAX_PATH];
		::PathCombine(FindExpr, DirPath, _T("*"));

		for ( hFind = ::FindFirstFile(FindExpr, &FindFileData);
			mayContinue && hFind != INVALID_HANDLE_VALUE;
			mayContinue = ::FindNextFile(hFind, &FindFileData) )
		{
			if ( (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
			{
				::PathCombine(FindExpr, DirPath, FindFileData.cFileName);
				::SetFileAttributes(FindExpr, 0);
				::DeleteFile(FindExpr);
			}
			else
			{
				if ( lstrcmp(FindFileData.cFileName, _T(".")) != 0 && lstrcmp(FindFileData.cFileName, _T("..")) != 0 )
				{
					::PathCombine(FindExpr, DirPath, FindFileData.cFileName);
					return RemoveDirectoryWithSubItems(FindExpr);
				}
			}
		}
		if ( FALSE == ::SetFileAttributes(DirPath, 0) )
		{
			return false;
		}
		return ( FALSE != ::RemoveDirectory(DirPath) );
	}


};


#if !defined(NO_OFN_DIALOG)

// Inspired from:
// Multiple Selection in a File Dialog By PJ Arends
// http://www.codeproject.com/dialog/pja_multiselect.asp

// Using a modified version of CFileDialogImpl<> with an extra parameter (defaulting to false)
// specifying whether a hook is used. Setting it to false enables the places bar on Win9x
class CMultiFileDialog : public CFileDialogImpl<CMultiFileDialog, true>
{
	typedef CFileDialogImpl<CMultiFileDialog, true> __baseClass;
public:
	CMultiFileDialog(BOOL bOpenFileDialog, // TRUE for FileOpen, FALSE for FileSaveAs
		LPCTSTR lpszDefExt = NULL,
		LPCTSTR lpszFileName = NULL,
		DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		LPCTSTR lpszFilter = NULL,
		HWND hWndParent = NULL)
		: CFileDialogImpl<CMultiFileDialog, true>(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, hWndParent), m_pstrFiles(0), m_pstrDirectory(0), m_bParsed(false)
	{ }

	~CMultiFileDialog()
	{
		if ( m_pstrFiles )
		{
			delete[] m_pstrFiles;
			delete[] m_pstrDirectory;
		}
	}

	INT_PTR DoModal(HWND hWndParent = ::GetActiveWindow());
	POSITION GetStartPosition();
	CString GetNextPathName(POSITION &pos) const;
	void OnSelChange(LPOFNOTIFY /*lpon*/);
	//BOOL OnFileOK(LPOFNOTIFY lpon);


// Message map and handlers
	BEGIN_MSG_MAP(CMultiFileDialog)
//		NOTIFY_CODE_HANDLER(CDN_SELCHANGE, OnSelChange)
	    CHAIN_MSG_MAP(__baseClass)
	END_MSG_MAP()

protected:
	LPTSTR m_pstrFiles;
	size_t  m_uCurrentFBSize;
	LPTSTR m_pstrDirectory;
	size_t  m_uCurrentFDSize;
	bool   m_bParsed;
};



#endif


#endif //__FNMANIP_H__
