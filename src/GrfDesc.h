// $Id: GrfDesc.h 9 2005-08-12 17:33:08Z Rasqual $

// *
// *   gryff, a GRF archive management utility, src/GrfDesc.h
// *   Copyright (C) 2003, 2004 Rasqual Twilight
// *
// *   This program is distributed in the hope that it will be useful,
// *   but WITHOUT ANY WARRANTY; without even the implied warranty of
// *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// *

// * GrfDesc.h
// * Internal structures for representing a Grf's elements
// *
// *@author $Author: Rasqual $
// *@version $Revision: 9 $
// *

#ifndef __GRFDESC_H__
#define __GRFDESC_H__


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __ATLCOLL_H__
   #error GrfDesc.h requires <atlcoll.h> to be included first
#endif

#include  "StringTraits.h"

///////////////////////////////////////////////////////////////////////////////
// Structs in this file:
//
// CGrfFileEntry
//   Node Element of a Grf document
//   There are two types of nodes:
//    . Files originating from an existing Grf file
//    . Files loaded from the filesystem
// CGrfDirectoryEntry
//   Directory node in a GRF document


struct CGrfFileEntry;
struct CGrfDirectoryEntry;

typedef ATL::CRBMap< CString, CGrfFileEntry*, CStringElementTraitsGrfName<CString> > SortedGrfFileMap;
typedef ATL::CRBMap< CString, CGrfDirectoryEntry*, CStringElementTraitsGrfName<CString> > SortedGrfDirMap;

struct CGrfDirectoryEntry
{
	CString           dir;   // Not trailing bslash : "data", "data\\model", etc.
	SortedGrfFileMap  files;

	CGrfDirectoryEntry(const CString& insert_dir) : dir(insert_dir)
	{
	}
	// Note: the destructor does not delete() any CGrfFileEntry. If allocated data
	// has been set, it is up to the user to delete() it.
};


// Note: the actual file name used for the entry is defined by the map key
// of the files member of CGrfDirectoryEntry, to avoid having too much redundancy

struct CGrfFileEntry
{
	static enum FILEORIG { FROMGRF=1, FROMFS };
	static enum { GRF_MAX_PATH = MAX_PATH };  // maximal size of the complete path in the grf. The filename shall of course be less than this.

	FILEORIG           origin;     // GRF or FS
	  // original source (abspath inside GRF / FS abspath) because file may be renamed/moved. Ex: "data\\model\\foo.gr2" "c:\\Documents and Settings\\Bamboo\\Desktop\\sample.xml"
	CString            realpath;   // FS: real file path
	char               in_grf_path[GRF_MAX_PATH];  // GRF: file path in multibyte form
	CString            *grf_src;   // FROMGRF only, 0 otherwise.
	DWORD              fullsize;   // uncompressed size
	DWORD              compsize;   // compressed size (FROMGRF), static_cast<DWORD>(-1) otherwise

	CGrfFileEntry(LPCTSTR fn, FILEORIG from=FROMFS, LPCTSTR gsrc=0) : origin(from), realpath(fn), fullsize(0), compsize(from == FROMGRF? 0 : static_cast<DWORD>(-1))
	{
		grf_src = gsrc != 0 ? new CString(gsrc) : 0;
		if ( from == CGrfFileEntry::FROMGRF )
		{
			::WideCharToMultiByte(0x3B5, 0, fn, -1, this->in_grf_path, CGrfFileEntry::GRF_MAX_PATH, NULL, NULL);
		}

	}

	CGrfFileEntry(const CGrfFileEntry &copy) : origin(copy.origin), realpath(copy.realpath), fullsize(copy.fullsize), compsize(copy.compsize)
	{
		grf_src = copy.grf_src != 0 ? new CString(*(copy.grf_src)) : 0;
		strncpy(in_grf_path, copy.in_grf_path, GRF_MAX_PATH);
		in_grf_path[GRF_MAX_PATH-1] = 0;
	}

	~CGrfFileEntry()
	{
		if ( grf_src )
		{
			delete grf_src;
			grf_src = 0;
		}
	}


	void  setSizes(DWORD f, DWORD c = static_cast<DWORD>(-1))
	{
		fullsize = f;
		compsize = c;
	}
};


#endif // __GRFDESC_H__
