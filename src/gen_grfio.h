// $Id: gen_grfio.h 9 2005-08-12 17:33:08Z Rasqual $

// *
// *   gryff, a GRF archive management utility, src/gen_grfio.cpp
// *   Copyright (C) 2003, 2004 Rasqual Twilight
// *
// *   This program is distributed in the hope that it will be useful,
// *   but WITHOUT ANY WARRANTY; without even the implied warranty of
// *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// *

// * gen_grfio.cpp
// * grf related functions and structure definitions
// *
// *@author $Author: Rasqual $
// *@version $Revision: 9 $
// *

#ifndef __GENGRFIO_H__
#define __GENGRFIO_H__

#pragma once
#include  <openkore/libgrf>

// Gen::GrfImportError
//   thrown structure on import error

namespace Gen
{
	// Reading
	openkore::Grf *GrfOpen(LPCTSTR szGrfFileName, openkore::GrfError * = 0);
	BOOL GrfClose(openkore::Grf *pGrf);
	//

	// finding data
	POSITION GrfFindFirstFile(openkore::Grf *pGrf, LPCTSTR szGlobExp, openkore::GrfFile **ppGrfFile);
	BOOL GrfFindNextFile(POSITION &pos, openkore::GrfFile **ppGrfFile);
	void GrfFindClose(POSITION&);

	struct GrfImportError
	{
		int        code;
		int        subcode;
		CString    message;
		enum GIECodes
		{
			LIBRARY_ERROR = 1,   // failed to open, library returned a descriptive error (subcode, message)
			LIBRARY_FAILURE = 2  // library crashed
		};
	};


}  // Gen::


class CGrfCache
{
private:
	struct FileMappingInfo
	{
		HANDLE hFile;
		HANDLE hFileMapping;
	};
public:
	CGrfCache() {}
	~CGrfCache() throw()
	{
		for ( POSITION pos = m.GetStartPosition(); pos; m.GetNext(pos) )
		{
			try
			{
				Gen::GrfClose(m.GetValueAt(pos));
			}
			catch(...)
			{
				bool caught = true;
			}
		}
		for ( POSITION pos = m_FileMappings.GetStartPosition(); pos; m_FileMappings.GetNext(pos) )
		{
			FileMappingInfo &mappingInfo = m_FileMappings.GetValueAt(pos);
			::CloseHandle(mappingInfo.hFileMapping);
			::CloseHandle(mappingInfo.hFile);
		}

	}

	openkore::Grf *get(const CString& name) throw(Gen::GrfImportError, openkore::GrfError)
	{
		openkore::Grf *pGrf;
		openkore::GrfError errorcode;
		if ( m.Lookup(name, pGrf) )
		{
			return pGrf;
		}
		try
		{
			pGrf = Gen::GrfOpen(name, &errorcode);
		}
		catch(...)
		{
			Gen::GrfImportError gie;
			gie.code = Gen::GrfImportError::LIBRARY_FAILURE;
			throw gie;
		}
		if ( pGrf == 0 )
		{
			throw errorcode;
		}
		m.SetAt( name, pGrf );
		return pGrf;
	}

	HANDLE getFileMapping(const CString& name)
	{
		FileMappingInfo mappingInfo;
		if ( m_FileMappings.Lookup(name, mappingInfo) )
		{
			return mappingInfo.hFileMapping;
		}
		mappingInfo.hFile = ::CreateFile(name, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if ( mappingInfo.hFile == INVALID_HANDLE_VALUE)
		{
			return 0;
		}
		DWORD dwFileSize = ::GetFileSize(mappingInfo.hFile, NULL);
		if ( dwFileSize == 0 )
		{
			::CloseHandle(mappingInfo.hFile);
			return 0;
		}
			// NULL ACL token
		mappingInfo.hFileMapping = ::CreateFileMapping(mappingInfo.hFile, NULL, PAGE_READONLY, 0, dwFileSize, NULL /*name*/);
		if ( !mappingInfo.hFileMapping )
		{
			::CloseHandle(mappingInfo.hFile);
			return 0;
		}
		m_FileMappings.SetAt( name, mappingInfo );
		return mappingInfo.hFileMapping;
	}


private:
	ATL::CAtlMap< CString, openkore::Grf * > m;
	ATL::CAtlMap< CString, FileMappingInfo > m_FileMappings;
private:
	CGrfCache(const CGrfCache&) {}
	CGrfCache& operator=(const CGrfCache&) {}
};

#endif // __GENGRFIO_H__
