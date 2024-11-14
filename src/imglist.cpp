// $Id: imglist.cpp 9 2005-08-12 17:33:08Z Rasqual $

// *
// *   gryff, a GRF archive management utility, src/imglist.cpp
// *   Copyright (C) 2003-2005 Rasqual Twilight
// *
// *   This program is distributed in the hope that it will be useful,
// *   but WITHOUT ANY WARRANTY; without even the implied warranty of
// *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// *

// * imglist.cpp
// * Image list creation and destruction
// *
// *@author $Author: Rasqual $
// *@version $Revision: 9 $
// *

#include  "stdwtl.h"
#pragma hdrstop
#include  "resource.h"
#include  "imglist.h"


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
bool CreateImageList(CMyImageList **ppImageList, int *pnErrorCode)
{
	if ( ppImageList == 0 || *ppImageList != 0 )
	{
		*pnErrorCode = 1;
		return false;
	}
	try
	{
		*ppImageList = new CMyImageList;
		if ( *ppImageList == 0 )
		{
			throw 0;
		}

#define USER_ICONS_COUNT  4
		  // Create the image list.
//		if ( NULL == ((*ppImageList)->m_himl = ImageList_Create(16,16,ILC_MASK|ILC_COLOR32,USER_ICONS_COUNT,4)) )
//		{
//			throw 0;
//		}
	}
	catch (...)
	{
		if ( *ppImageList )
		{
			delete *ppImageList;
			*ppImageList = 0;
		}
		*pnErrorCode = 2;
		return false;
	}
	(*ppImageList)->m_iFirstUserIcon = ImageList_GetImageCount((*ppImageList)->m_himl);

	  // Load icons
	{
		CIcon hGRFFileIcon;
		hGRFFileIcon.LoadIcon(IDC_GRYFFCHILD);
		(*ppImageList)->m_igrfArchive = ImageList_AddIcon((*ppImageList)->m_himl, hGRFFileIcon);
		ImageList_AddIcon((*ppImageList)->m_himlSmall, hGRFFileIcon);

		CIcon hFolderIcon;
		hFolderIcon.LoadIcon(IDI_CLOSEDFOLDER);
		(*ppImageList)->m_iclosedFolder = ImageList_AddIcon((*ppImageList)->m_himl, hFolderIcon);
		ImageList_AddIcon((*ppImageList)->m_himlSmall, hFolderIcon);

		CIcon hOpenFolderIcon;
		hOpenFolderIcon.LoadIcon(IDI_OPENFOLDER);
		(*ppImageList)->m_iopenFolder = ImageList_AddIcon((*ppImageList)->m_himl, hOpenFolderIcon);
		ImageList_AddIcon((*ppImageList)->m_himlSmall, hOpenFolderIcon);

		CIcon hFileIcon;
		hFileIcon.LoadIcon(IDI_FILEICON);
		(*ppImageList)->m_idefaultFile = ImageList_AddIcon((*ppImageList)->m_himl, hFileIcon);
		ImageList_AddIcon((*ppImageList)->m_himlSmall, hFileIcon);
	}

	if ( ImageList_GetImageCount((*ppImageList)->m_himl) - (*ppImageList)->m_iFirstUserIcon < USER_ICONS_COUNT )
	{
		delete *ppImageList;
		*ppImageList = 0;
		*pnErrorCode = 3;
		return false;
	}

	return true;
}

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
bool DestroyImageList(CMyImageList **ppImageList, int *pnErrorCode)
{
	if ( ppImageList == 0 )
	{
		*pnErrorCode = 1;
		return false;
	}

	delete *ppImageList;
	*ppImageList = 0;
	return true;
}

