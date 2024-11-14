// $Id: fnmanip.cpp 12 2005-08-14 01:42:51Z Rasqual $

// *
// *   gryff, a GRF archive management utility, src/fnmanip.cpp
// *   Copyright (C) 2003, 2004 Rasqual Twilight
// *
// *   This program is distributed in the hope that it will be useful,
// *   but WITHOUT ANY WARRANTY; without even the implied warranty of
// *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// *

// * fnmanip.cpp
// * Miscellaneous file-related utilities.
// *
// *@author $Author: Rasqual $
// *@version $Revision: 12 $
// *

#include  "stdwtl.h"

#pragma hdrstop
#include  "fnmanip.h"

///////////////////////////////////////////////////////////////////////////////
// BreakPath()
//
//  Splits a path into a directory and a basename.
//  The directory separator shall be a backslash.
//  When the path is a top level name, the resulting directory is empty.
//  As a side effect, strPath and strName can be a reference to the same object
//  and the function will still work (but strPath will be modified).
//
void BreakPath(const CString& strPath, CString& strDir, CString& strName)
{
	CString  fn;
	int      pos = strPath.ReverseFind(_T('\\'));
	if ( pos != -1 )
	{
		strDir = strPath.Left(pos);
		strName = strPath.Right(strPath.GetLength() - (pos + 1));  // Keep filename only
	}
	else
	{
		strDir.Empty();
		strName = strPath;
	}
}

///////////////////////////////////////////////////////////////////////////////
// CombinePath()
//
//  Combines path elements.
//  Note: make sure strCombinedPath is a separate object
//
CString &CombinePath(CString& strCombinedPath, LPCTSTR pDir, LPCTSTR pName)
{
	if ( *pDir == _T('\0') )
	{
		strCombinedPath = pName;
	}
	else
	{
		if ( *pName  == _T('\0') )
		{
			strCombinedPath = pDir;
		}
		else
		{
			strCombinedPath.Format(_T("%s\\%s"), pDir, pName);
		}
	}
	return strCombinedPath;
}


///////////////////////////////////////////////////////////////////////////////
// GetTopDir()
//
//  Splits a path into a top parent directory and the rest of the path.
//  The directory separator shall be a backslash.
//  When the path is a top level name, the rest of the path is empty.
//
void GetTopDir(const CString& strPath, CString& strDir, CString& strRemaining)
{
	CString  fn;
	int      pos = strPath.Find(_T('\\'));  // Find first occurence
	if ( pos != -1 )
	{
		strDir = strPath.Left(pos);
		strRemaining = strPath.Right(strPath.GetLength() - (pos + 1));
	}
	else
	{
		strDir = strPath;
		strRemaining.Empty();
	}
}

///////////////////////////////////////////////////////////////////////////////
// IsSubDir()
//
//  Tests whether a directory is a subdirectory of another (direct or not)
//  '..' are not taken into account
//  The directory separator shall be a backslash.
//
bool IsSubDir(const CString& strPseudoSub, const CString& strPseudoParent)
{
	if ( strPseudoSub.GetLength() > strPseudoParent.GetLength() )
	{
		if ( strPseudoParent.IsEmpty() )
		{
			return true;
		}
		return ( strPseudoSub.Find(strPseudoParent) == 0 && strPseudoSub[strPseudoParent.GetLength()] == _T('\\') );
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
// AnsiToUnicode()
//
//  Convert a CP-949 encoded byte array into a Unicode string.
//  Note: make sure strDest is a previously allocated object
//
CString &AnsiToUnicode(CString& strDest, LPCTSTR pAnsiEncoded)
{
	unsigned int i, len = lstrlen(pAnsiEncoded);
	CAutoVectorPtr<char> data(new char[len+1]);

	for ( i = 0U; i < len; ++i )
	{
		data[i] = static_cast<char>(pAnsiEncoded[i] & 0xFF);
	}
	data[i] = 0;
	::MultiByteToWideChar(0x3B5, 0, data, -1, strDest.GetBufferSetLength(len + 1), len + 1);
	strDest.ReleaseBuffer();

	return strDest;
}

HANDLE StartApplication(LPCTSTR path, HWND parentWindow)
{
	SHELLEXECUTEINFO execInfo = { 0 };
	execInfo.cbSize = sizeof(execInfo);
	execInfo.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_DDEWAIT;
	execInfo.hwnd = parentWindow;
	execInfo.lpVerb = NULL;
	execInfo.lpFile = path;
	execInfo.lpParameters = NULL;
	execInfo.lpDirectory = NULL;
	execInfo.nShow = SW_SHOWNORMAL;
	execInfo.hProcess = 0;
	bool succeeded = (FALSE != ::ShellExecuteEx(&execInfo));
	if ( !succeeded )
	{
		LPVOID lpMsgBuf;
		::FormatMessage(
		  FORMAT_MESSAGE_ALLOCATE_BUFFER |
		  FORMAT_MESSAGE_FROM_SYSTEM |
		  FORMAT_MESSAGE_IGNORE_INSERTS,
		  NULL,
		  ::GetLastError(),
		  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		  (LPTSTR) &lpMsgBuf,
		  0,
		  NULL
		);
		CString  strMessage;
		strMessage.Format(_T("%s :: %s"),
			path, (LPCTSTR)lpMsgBuf);
		::LocalFree(lpMsgBuf);
		::MessageBox(parentWindow, strMessage, NULL, MB_ICONSTOP | MB_OK);
	}
	return execInfo.hProcess;
}


///////////////////////////////////////////////////////////////////////////////
// Implementation of CMultiFileDialog


INT_PTR CMultiFileDialog::DoModal(HWND hWndParent)
{
	if ( m_pstrFiles )
	{
		delete[] m_pstrFiles;
		m_pstrFiles = NULL;
		delete[] m_pstrDirectory;
		m_pstrDirectory = NULL;
	}

	m_ofn.nFileOffset = 0;
	m_ofn.lpstrFile[0] = _T('\0');
	  // Resolve being able to enter name(s) and IDOK without changing selection
	ATLASSERT(m_ofn.nMaxFile >= MAX_PATH);

	INT_PTR ret = CFileDialogImpl<CMultiFileDialog, true>::DoModal(hWndParent);

	if ( ret == IDCANCEL )
	{
		DWORD err = CommDlgExtendedError();
		if ( err == FNERR_BUFFERTOOSMALL/*0x3003*/ && m_pstrFiles )
		{
			ret = IDOK;
		}
	}
	return ret;
}

POSITION CMultiFileDialog::GetStartPosition()
{
	if ( m_pstrFiles == 0 )
	{
		if ( m_ofn.nFileOffset == 0 )
		{
			return reinterpret_cast<POSITION>(m_ofn.lpstrFile);
		}
		// Is this multi-selection?
		LPTSTR dn = m_ofn.lpstrFile;
		WORD idx = 0;
		while ( *dn++ )
		{
			if ( ++idx >= m_ofn.nFileOffset )
			{
				m_ofn.nFileOffset = 0;
				return reinterpret_cast<POSITION>(m_ofn.lpstrFile);
			}
		}

		return reinterpret_cast<POSITION>(m_ofn.lpstrFile + m_ofn.nFileOffset);
	}
	  // Has this selection been parsed?
	if ( !m_bParsed )
	{
		CString temp(m_pstrFiles);
		  // use | as separator token
		temp.Replace(_T("\" \""), _T("|"));
		temp.TrimRight();
		temp.Replace(_T("\""), _T(""));

		lstrcpy(m_pstrFiles, temp);
		m_pstrFiles[lstrlen(m_pstrFiles)+1] = _T('\0');  // trailing double \0

		LPTSTR ptr = m_pstrFiles;
		while ( *ptr )
		{
			if ( _T('|') == *ptr )
			{
				*ptr = _T('\0');
			}
			++ptr;
		}
		m_bParsed = true;
	}
	return reinterpret_cast<POSITION>(m_pstrFiles);
}

CString CMultiFileDialog::GetNextPathName(POSITION &pos) const
{
	LPTSTR ptr = reinterpret_cast<LPTSTR>(pos);
	if ( m_pstrFiles == 0 )
	{
		if ( m_ofn.nFileOffset == 0 )
		{
			pos = 0;
			return CString(m_ofn.lpstrFile);
		}
		CString tmp;
		if ( m_ofn.lpstrFile[lstrlen(m_ofn.lpstrFile)-1] == _T('\\') )
		{
			tmp.Format(_T("%s%s"), m_ofn.lpstrFile, ptr);
		}
		else
		{
			tmp.Format(_T("%s\\%s"), m_ofn.lpstrFile, ptr);
		}

		  // HACK: on loop out, pFilename is one char ahead of \0
		  // that is to say the file list
		while ( *ptr++ ) ;
		if ( *ptr == 0 )
		{
			pos = 0;
		}
		else
		{
			pos = reinterpret_cast<POSITION>(ptr);
		}
		return tmp;
	}
	CString strPathName;
	if ( m_pstrDirectory[lstrlen(m_pstrDirectory)-1] == _T('\\') )
	{
		strPathName.Format(_T("%s%s"), m_pstrDirectory, ptr);
	}
	else
	{
		strPathName.Format(_T("%s\\%s"), m_pstrDirectory, ptr);
	}

	ptr += lstrlen(ptr) + 1;
	  // Found end of list (double nul)
	if ( *ptr )
	{
		pos = reinterpret_cast<POSITION>(ptr);
	}
	else
	{
		pos = 0;
	}
	return strPathName;
}

void CMultiFileDialog::OnSelChange(LPOFNOTIFY /*lpon*/)
{
	TCHAR dummy_buffer;
	  // Get the required size for the 'files' buffer
	int nfiles = this->GetSpec(&dummy_buffer, 1);
	  // Get the required size for the 'folder' buffer
	int nfolder = this->GetFolderPath(&dummy_buffer, 1);

	  // Check if lpstrFile and nMaxFile are large enough
	if ( (unsigned int)(nfiles + nfolder) > m_ofn.nMaxFile )
	{
		  // Needs reparse for GetStartPosition()
		m_bParsed = false;
		if ( m_pstrFiles )
		{
			  // Reallocate only if necessary
			if ( (unsigned int)nfiles + 2 > m_uCurrentFBSize )
			{
				delete[] m_pstrFiles;
				m_pstrFiles = new TCHAR[nfiles + 2];
				m_uCurrentFBSize = (unsigned int)nfiles + 2;
			}
			if ( (unsigned int)nfolder + 1 > m_uCurrentFDSize )
			{
				delete[] m_pstrDirectory;
				m_pstrDirectory = new TCHAR[nfolder + 1];
				m_uCurrentFDSize = (unsigned int)nfolder + 1;
			}
		}
		else
		{
			m_pstrFiles = new TCHAR[nfiles + 2];
			m_uCurrentFBSize = (unsigned int)nfiles + 2;
			m_pstrDirectory = new TCHAR[nfolder + 1];
			m_uCurrentFDSize = (unsigned int)nfolder + 1;
		}
		this->GetSpec(m_pstrFiles, nfiles);
		m_pstrFiles[lstrlen(m_pstrFiles)+1] = _T('\0');  // trailing double \0
		this->GetFolderPath(m_pstrDirectory, nfolder);
	}
	else if ( m_pstrFiles )
	{
		delete[] m_pstrFiles;
		m_pstrFiles = NULL;
		delete[] m_pstrDirectory;
		m_pstrDirectory = NULL;
	}
//	return 0;
}

// Do not use: not systematically called on success
//BOOL CMultiFileDialog::OnFileOK(LPOFNOTIFY lpon)
//{
//	ATLASSERT(lpon->lpOFN);
//	return TRUE;
//}

