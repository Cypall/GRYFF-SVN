// $Id: commoncontrols.h 9 2005-08-12 17:33:08Z Rasqual $

// *
// *   gryff, a GRF archive management utility, src/commoncontrols.h
// *   Copyright (C) 2003, 2004, 2005 Rasqual Twilight
// *
// *   This program is distributed in the hope that it will be useful,
// *   but WITHOUT ANY WARRANTY; without even the implied warranty of
// *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// *

// * commoncontrols.h
// * Common Controls utilities
// *
// *@author $Author: Rasqual $
// *@version $Revision: 9 $
// *

#ifndef __COMMONCONTROLS_H__
#define __COMMONCONTROLS_H__

#pragma once

namespace
{
int  ComCtlVersion = -1;
}


namespace commoncontrols
{

DWORD  GetComCtlVersion()
{
	if ( ComCtlVersion != -1 )
	{
		return  ComCtlVersion;
	}

HMODULE  hModule = ::GetModuleHandleA("Comctl32.dll");
	ATLASSERT(hModule != NULL);
DLLGETVERSIONPROC  proc = reinterpret_cast<DLLGETVERSIONPROC>(::GetProcAddress(hModule, "DllGetVersion"));
DWORD  dwVersion = MAKELONG(0, 4);

	if ( proc != NULL )
	{
	DLLVERSIONINFO  dvi = { sizeof(DLLVERSIONINFO), 0, 0, 0, 0 };
	HRESULT  hr = (*proc)(&dvi);
		if ( SUCCEEDED(hr) )
		{
			ATLASSERT(dvi.dwMajorVersion <= 0xFFFF);
			ATLASSERT(dvi.dwMinorVersion <= 0xFFFF);
			dwVersion = MAKELONG(dvi.dwMinorVersion, dvi.dwMajorVersion);
		}
	}
	ComCtlVersion = dwVersion;
	return dwVersion;
}

} // EON


#endif  // ndef __COMMONCONTROLS_H__
