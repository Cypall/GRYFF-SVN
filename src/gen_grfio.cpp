// $Id: gen_grfio.cpp 9 2005-08-12 17:33:08Z Rasqual $

// *
// *   gryff, a GRF archive management utility, src/gen_grfio.cpp
// *   Copyright (C) 2003, 2004 Rasqual Twilight
// *
// *   This program is distributed in the hope that it will be useful,
// *   but WITHOUT ANY WARRANTY; without even the implied warranty of
// *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// *

// * gen_grfio.cpp
// * grf related functions
// *
// *@author $Author: Rasqual $
// *@version $Revision: 9 $
// *

#include  "stdwtl.h"
#include  <cerrno>
#ifdef WIN32
#include  <windows.h>
#endif /* WIN32 */
#include  <gen_types.h>

#pragma hdrstop
#include  "gen_grfio.h"

#define NO_OFN_DIALOG
#include  "fnmanip.h"

using namespace openkore;

extern LPCTSTR szTempArchivePrefix;
#define GLOBALVAR(e) e

///////////////////////////////////////////////////////////////////////////////

Grf *Gen::GrfOpen(LPCTSTR szGrfFileName, GrfError *pErrorCode)
{
	Grf *pGrf = 0;
	ATLASSERT(pErrorCode);

	char  szFileNameA[MAX_PATH];
	bool bPlanAFailed(false), bPlanBFailed(false);
	CTempFile tmp_file(0);

	{
		CAutoVectorPtr<TCHAR> pShortPathName(0);
		TCHAR dummy_buffer;
		DWORD dwRequiredTchars, dwResult = 0;
		BOOL bDefaultCharUsed(FALSE);
		// May be a Unicode(r) filename. Try Plan A....
		// Plan A: Convert to short path name using thread local ANSI CP
		if ( 0 != (dwRequiredTchars = ::GetShortPathName(szGrfFileName, &dummy_buffer, 1)) )
		{
			// Should EH this
			pShortPathName.Attach(new TCHAR[dwRequiredTchars] );
			dwResult = ::GetShortPathName(szGrfFileName, pShortPathName, dwRequiredTchars);
			// Now, try to convert to the current locale and pray it doesn't blow.
			int nMbCount = ::WideCharToMultiByte(CP_THREAD_ACP, 0, pShortPathName, -1, szFileNameA, MAX_PATH, NULL, &bDefaultCharUsed);
			if ( nMbCount == 0 || bDefaultCharUsed )
			{
				// Uh-oh
				dwResult = 0;
			}
			pShortPathName.Free();
		}

		if ( dwResult == 0 )
		{
			  // Plan A failed, obviously. Let's take it to Plan B.
			bPlanAFailed = true;
			{
				  // Plan B: Create a hardlink in the current directory (to increase chances making a hardlink is possible)
				  // If that fails, try making it in the temporary directory
				  // (this may also fail because NTFS links cannot be spanned over multiple volumes or some other reason)

				  // Create a temporary output file

				  // Retrieve destination filepath components
				CString dn, fn(szGrfFileName);
				::BreakPath(fn, dn, fn);
				TCHAR tmppath[MAX_PATH];
				if ( 0 != ::GetTempFileName(dn, ::szTempArchivePrefix, 0, tmppath) )
				{
					::DeleteFile(tmppath);
					if ( FALSE != ::CreateHardLink(tmppath, szGrfFileName, NULL) )
					{
						tmp_file.Attach(tmppath);
						if ( 0 != (dwRequiredTchars = ::GetShortPathName(tmppath, &dummy_buffer, 1)) )
						{
							// Should EH this
							pShortPathName.Attach(new TCHAR[dwRequiredTchars] );
							dwResult = ::GetShortPathName(tmppath, pShortPathName, dwRequiredTchars);
							// Now, try to convert to the current locale and pray it doesn't blow.
							int nMbCount = ::WideCharToMultiByte(CP_THREAD_ACP, 0, pShortPathName, -1, szFileNameA, MAX_PATH, NULL, &bDefaultCharUsed);
							if ( nMbCount == 0 || bDefaultCharUsed )
							{
								// Uh-oh
								dwResult = 0;
							}
							pShortPathName.Free();
						}
					}
				}

				if ( dwResult == 0 )
				{
					bPlanBFailed = true;
				}

				// There is actually a plan C involving symbolic links, but that feature is undocumented
			}
		}
	}
	if ( bPlanAFailed && bPlanBFailed )
	{
		GRF_SETERR(pErrorCode,GE_INVALID,GrfOpen(): Cannot open file - maybe it does not exist);
		return 0;
	}

	try
	{
		pGrf = grf_open(szFileNameA, "rb", pErrorCode);
		if ( !pGrf && pErrorCode )
		{
			//pStrError->Format(_T("(%d) %s"), GRFERRTYPE(*pErrorCode), CA2T(grf_strerror(*pErrorCode)) );
		}
	}
	catch (...)
	{
		  // Report crash
		if ( pGrf )
		{
			try
			{
				Gen::GrfClose(pGrf);
			}
			catch (...)
			{
			}
		}
		GLOBALVAR(errno) = EFAULT;
		GRF_SETERR(pErrorCode,GE_ERRNO,trycatch);
		//pStrError->Format(_T("(%d) %s"), grferr, CA2T(grf_strerror(grferr)) );
		return 0;
	}
	return pGrf;
}

///////////////////////////////////////////////////////////////////////////////

BOOL Gen::GrfClose(Grf *pGrf)
{
	grf_free(pGrf);
	return true;
}

///////////////////////////////////////////////////////////////////////////////

class GrfFindDescriptor
{
	Grf         *m_pGrf;
	bool        *m_bMatching;
	bool        m_bNoneMatching;
	int         m_nCurrentIndex;

public:
	GrfFindDescriptor(Grf *pInitGrf, LPCTSTR szGlobExp) : m_pGrf(pInitGrf), m_nCurrentIndex(-1)
	{
		m_bNoneMatching = true;
		if ( pInitGrf )
		{
			m_bMatching = new bool[pInitGrf->nfiles];
			for (unsigned int i=0; i<pInitGrf->nfiles; ++i)
			{
				// FIXME: TODO: (x.x) use szGlobExp when searching
				m_bMatching[i] = true;
				m_bNoneMatching = false;
			}
		}
		else
		{
			m_bMatching = 0;
		}
	}

	~GrfFindDescriptor()
	{
		if ( m_bMatching )
			delete m_bMatching;
	}

	Grf *GetGrf()
	{
		return m_pGrf;
	}

	int GrfFindNextMatching()
	{
		if ( m_bMatching && !m_bNoneMatching )
		{
			int nCount = static_cast<int>(m_pGrf->nfiles);
			for (int i=m_nCurrentIndex+1; i<nCount; ++i)
			{
				if ( m_bMatching[i] )
				{
					return (m_nCurrentIndex = i);
				}
			}
		}
		return (m_nCurrentIndex = -1);
	}

	void GrfFindRewind()
	{
		m_nCurrentIndex = -1;
	}

};

///////////////////////////////////////////////////////////////////////////////

POSITION Gen::GrfFindFirstFile(Grf *pGrf, LPCTSTR szGlobExp, GrfFile **ppGrfFile)
{
	int nMatch;
	GrfFindDescriptor *pdesc = new GrfFindDescriptor(pGrf, szGlobExp);
	if ( -1 == (nMatch = pdesc->GrfFindNextMatching()) )
	{
		delete pdesc;
		return 0;
	}
	*ppGrfFile = (pdesc->GetGrf()->files + nMatch);
	return reinterpret_cast<POSITION>(pdesc);
}

///////////////////////////////////////////////////////////////////////////////

BOOL Gen::GrfFindNextFile(POSITION &pos, GrfFile **ppGrfFile)
{
	int nMatch;
	if ( pos == 0 )
	{
		return FALSE;
	}
	GrfFindDescriptor *pdesc = reinterpret_cast<GrfFindDescriptor *>(pos);
	if ( -1 == (nMatch = pdesc->GrfFindNextMatching()) )
	{
		delete pdesc;
		pos = 0;
		return FALSE;
	}
	*ppGrfFile = (pdesc->GetGrf()->files + nMatch);
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////

void Gen::GrfFindClose(POSITION &pos)
{
	if ( pos != 0 )
	{
		GrfFindDescriptor *pdesc = reinterpret_cast<GrfFindDescriptor *>(pos);
		delete pdesc;
		pos = 0;
	}
}

///////////////////////////////////////////////////////////////////////////////
