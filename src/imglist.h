// $Id: imglist.h 9 2005-08-12 17:33:08Z Rasqual $

// *
// *   gryff, a GRF archive management utility, src/imglist.h
// *   Copyright (C) 2003-2005 Rasqual Twilight
// *
// *   This program is distributed in the hope that it will be useful,
// *   but WITHOUT ANY WARRANTY; without even the implied warranty of
// *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// *

// * imglist.h
// * Image list (HIMAGELIST) creation and destruction
// *
// *@author $Author: Rasqual $
// *@version $Revision: 9 $
// *

#if !defined(__IMGLIST_H__)
#define __IMGLIST_H__

#include  <utility>  // pair

class CMyImageList
{
public:
	HIMAGELIST    m_himl;
	HIMAGELIST    m_himlSmall;
	int           m_igrfArchive;
	int           m_iclosedFolder;
	int           m_iopenFolder;
	int           m_idefaultFile;
	int           m_iFirstUserIcon;

	// This member maps file extensions to image lists index, case-insensitively
	// This optimization is proved to be useful in cases where ::SHGetFileInfo
	// would be called several times as profiling highlights this function
	// as a bottleneck.
	ATL::CAtlMap< CString, std::pair<int, CString>, CStringElementTraitsI<CString> > m_ImageMapper;

	//; Shell32 undocumented functions
	//Shell_GetImageLists  @71
	//FileIconInit         @660
	#if WINVER > 0x0501
	#pragma warning CMyImageList: Make sure ordinals map correctly to shell functions for targeted Windows version
	#endif

	typedef BOOL (WINAPI * SHELL_GETIMAGELIST_PROC)(HIMAGELIST *phLarge, HIMAGELIST *phSmall);
	typedef BOOL (WINAPI * FILEICONINIT_PROC)   (BOOL fFullInit);

	CMyImageList() : m_himl(0), m_himlSmall(0), m_igrfArchive(-1), m_iclosedFolder(-1), m_iopenFolder(-1), m_idefaultFile(-1), m_iFirstUserIcon(-1)
	{
		SHELL_GETIMAGELIST_PROC  Shell_GetImageLists;
		FILEICONINIT_PROC        FileIconInit;

		SHFILEINFO sfi;
		  // We do not actually need the directory icon's indes, since using our own
		::SHGetFileInfo(_T(""), FILE_ATTRIBUTE_DIRECTORY, &sfi, sizeof(SHFILEINFO), SHGFI_USEFILEATTRIBUTES/* | SHGFI_SYSICONINDEX*/ | SHGFI_TYPENAME);
		  // Register in cache, with a special key name (extension) that a file cannot get,
		  // according to Windows file naming guidelines (trailing .)
		m_ImageMapper.SetAt(_T("."), std::pair<int, CString>(sfi.iIcon, sfi.szTypeName));

		// Load SHELL32.DLL, if it isn't already
	HMODULE hShell32 = ::GetModuleHandle(_T("SHELL32"));
		if ( hShell32 != 0 )
		{
			//
			// Get Undocumented APIs from Shell32.dll:
			//
			Shell_GetImageLists  = (SHELL_GETIMAGELIST_PROC) ::GetProcAddress(hShell32, (LPCSTR)71);
			FileIconInit         = (FILEICONINIT_PROC)       ::GetProcAddress(hShell32, (LPCSTR)660);

			if ( Shell_GetImageLists && FileIconInit )
			{
				// Initialize imagelist for this process - function not present on win95/98
				FileIconInit(TRUE);
				// Get handles to the large+small system image lists!
				Shell_GetImageLists(&m_himl, &m_himlSmall);
			}
		}
	}

	~CMyImageList()
	{
		if ( m_himl )
		{
			ImageList_Destroy(m_himl);
		}
	}
};

#if !defined(IMAGELIST_STRUCT_ONLY)
#ifndef __ATLGDIX_H__
//#error imglist.h requires <viksoe/atlgdix.h> to be included first // merged into trunk since WTL 7.5 build 5002
#endif

/////////////////////////////////////////////////////////////////////////////
// CreateImageList
//
//  Creates an image list from the resources
//  On success, returns true
//  On error, returns false and sets *pnErrorCode.
//
//  Error codes:
//   1  invalid parameter
//   2  failed to create (memory)
//   3  unable to load from resources
//
/////////////////////////////////////////////////////////////////////////////
bool CreateImageList(CMyImageList **ppImageList, int *pnErrorCode);

/////////////////////////////////////////////////////////////////////////////
// DestroyImageList
//
//  Destroy an image list
//  On success, returns true
//  On error, returns false and sets *pnErrorCode.
//
//  Error codes:
//   1  invalid parameter
//
/////////////////////////////////////////////////////////////////////////////
bool DestroyImageList(CMyImageList **ppImageList, int *pnErrorCode);

#endif  // !IMAGELIST_STRUCT_ONLY


#endif  // !defined(__IMGLIST_H__)
