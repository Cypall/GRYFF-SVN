// $Id: gryff.cpp 9 2005-08-12 17:33:08Z Rasqual $

// *
// *   gryff, a GRF archive management utility, src/gryff.cpp
// *   Copyright (C) 2003-2005 Rasqual Twilight
// *
// *   This program is distributed in the hope that it will be useful,
// *   but WITHOUT ANY WARRANTY; without even the implied warranty of
// *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// *

// * gryff.cpp
// * Contains the program's entry point.
// * Initializes the framework and creates the main window.
// *
// *@author $Author: Rasqual $
// *@version $Revision: 9 $
// *

#include  "stdwtl.h"
#include  "imglist.h"
#pragma hdrstop
#include  "resource.h"
#include  "WndMain.h"

#include  "DlgWelcome.h"

///////////////////////////////////////////////////////////////////////////////

CAppModule    _Module;
CMyImageList  *g_pImageList = 0;
LPCTSTR       szTempArchivePrefix = _T("GrfA");
CString       strGryffRegKey;
UINT  UWM_ARE_YOU_ME = ::RegisterWindowMessage(UWM_ARE_YOU_ME_MSG);
UINT  UCF_GRYFF_ENTRIES = ::RegisterClipboardFormat(UCF_GRYFF_ENTRIES_FMT);
UINT  UCF_GRYFF_ENTRIES_INTERNAL = ::RegisterClipboardFormat(UCF_GRYFF_ENTRIES_INTERNAL_FMT);

#define BUFFERS_TCHAR_COUNT  256U

INT_PTR  Run(LPTSTR lpCmdLine = NULL, int nCmdShow = SW_SHOWDEFAULT);

namespace
{
inline HRESULT INITIALIZE_COM()
{
	return ::OleInitialize(NULL);
}
inline void UNINITIALIZE_COM()
{
	::OleUninitialize();
}
}

///////////////////////////////////////////////////////////////////////////////


// Avoiding Multiple Instances of an Application
// By Joseph M. Newcomer
// http://www.codeproject.com/cpp/avoidmultinstance.asp
// Source Code is provided as is and with no warranties.

BOOL CALLBACK  InstanceSearcher(HWND hWnd, LPARAM lParam)
{
	DWORD result;
	LRESULT ok = ::SendMessageTimeout(hWnd,
	                                  UWM_ARE_YOU_ME,
	                                  0, 0,
	                                  SMTO_BLOCK | SMTO_ABORTIFHUNG | SMTO_NOTIMEOUTIFNOTHUNG,
	                                  200,  // 200ms timeout
	                                  &result);
	if ( ok == 0 )
	{
		  // timeout; ignore this and continue
		return TRUE;
	}
	if ( result == UWM_ARE_YOU_ME )
	{
		*(reinterpret_cast<HWND *>(lParam)) = hWnd;
		  // stop search
		return FALSE;
	}
	  // continue search
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////

extern "C"
int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/,
                       LPTSTR lpCmdLine, int nCmdShow)
{
	  // Make sure only one instance of the application is running for this user
	  // FIXME: TODO: (1.2) User option: "single instance"
	  // FIXME: TODO: (x.x) Detect CLI batch invocations
	if ( 1 )
	{
	CString mutexName;
	HDESK desktop = ::GetThreadDesktop(::GetCurrentThreadId());
	DWORD len = 0;
	BOOL result = ::GetUserObjectInformation(desktop, UOI_NAME, NULL, 0, &len);
		if (result == 0 && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
		{
			  // temporary buffer
			LPBYTE info = new BYTE[len];
			::GetUserObjectInformation(desktop, UOI_NAME, reinterpret_cast<PVOID>(info), len, &len);
			mutexName.Format(_T("Single-Gryff-%s"), reinterpret_cast<LPCTSTR>(info));
			delete []info;
		}
		else
		{
			mutexName = _T("Single-Gryff");
		}
		  // Try to create the single-instance mutex.
		::CreateMutex(NULL, FALSE, mutexName);
		if ( ::GetLastError() == ERROR_ALREADY_EXISTS ||
		     ::GetLastError() == ERROR_ACCESS_DENIED )
		{
		HWND hWndInstance = 0;
			::EnumWindows(InstanceSearcher, (LPARAM)&hWndInstance);
			if ( hWndInstance != 0 )
			{
				if ( ::IsIconic(hWndInstance) )
				{
					::ShowWindow(hWndInstance, SW_RESTORE);
				}
				::SetForegroundWindow( hWndInstance );  // The target cannot do it itself
			COPYDATASTRUCT cds = {0};
				  // Pass parameters to existing instance
				if ( *lpCmdLine != _T('\0') )
				{
				TCHAR cwd[MAX_PATH];
					memset(cwd, 0, sizeof(cwd) / sizeof(cwd[0]));
				CString strCmdLine;
					strCmdLine.Format(_T("*\"%s\" %s"), _tgetcwd(cwd, MAX_PATH), lpCmdLine);
					cds.dwData = 27015;
					cds.cbData = static_cast<DWORD>((strCmdLine.GetLength() + 1)*sizeof(TCHAR));
					cds.lpData = reinterpret_cast<LPVOID>(strCmdLine.GetBuffer());
					::SendMessage(hWndInstance, WM_COPYDATA, 0, (LPARAM)&cds);
				}
				return 0;
			}
		}
	}  // end "single app" check

LPTSTR   pTitle = _T("Gryff initialization error");
TCHAR    szErrorTitle[BUFFERS_TCHAR_COUNT];
	if ( ::LoadString(::GetModuleHandle(NULL), IDS_INIT_ERROR_MSGBOX_TITLE,
					  szErrorTitle, BUFFERS_TCHAR_COUNT) != 0 )
	{
		pTitle = szErrorTitle;
	}

	  // Initialize COM
	  // Use OleInitialize() instead to enable Drag and drop functionality
HRESULT hRes  = INITIALIZE_COM();
	if ( FAILED(hRes) )
	{
		::MessageBox(HWND_DESKTOP, CString(MAKEINTRESOURCE(IDS_INIT_ERROR_COINIT_FAILED)),
		             pTitle, MB_ICONERROR | MB_OK);
		return  1;
	}

	  // this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
	::DefWindowProc(NULL, 0, 0, 0L);

	if ( FALSE == ::AtlInitCommonControls(ICC_COOL_CLASSES | ICC_PROGRESS_CLASS | ICC_BAR_CLASSES | ICC_TREEVIEW_CLASSES | ICC_LISTVIEW_CLASSES) )
	{
		::MessageBox(HWND_DESKTOP, CString(MAKEINTRESOURCE(IDS_INIT_ERROR_INITCC_FAILED)),
		             pTitle, MB_ICONERROR | MB_OK);
		UNINITIALIZE_COM();
		return  2;
	}

	hRes = _Module.Init(NULL, hInstance);
	if ( FAILED(hRes) )
	{
		::MessageBox(HWND_DESKTOP, CString(MAKEINTRESOURCE(IDS_INIT_ERROR_MODULE_FAILED)),
		             pTitle, MB_ICONERROR | MB_OK);
		UNINITIALIZE_COM();
		return  3;
	}

INT_PTR  nRet = ::Run(lpCmdLine, nCmdShow);

	_Module.Term();
	UNINITIALIZE_COM();

	return nRet > 255 ? 255 : (int)nRet;
}

///////////////////////////////////////////////////////////////////////////////

void  Welcome()
{
static const unsigned char  vd_tab[] = { 237, 216, 211, 204, 204 };
TCHAR  str[sizeof(vd_tab)+1];
int i;
	for ( i = 0; i < sizeof(vd_tab); ++i )
	{
		str[i] = (TCHAR)(vd_tab[i] ^ 0xaa);
	}
	str[i] = 0;

ATL::CRegKey  appRegKey;
	appRegKey.Create(HKEY_CURRENT_USER, ::strGryffRegKey);

DWORD  seenWelcome = 0;
	appRegKey.QueryDWORDValue(str, seenWelcome);
	if ( !seenWelcome )
	{
	CWelcomeDialog  dlg;
		dlg.DoModal();
		appRegKey.SetDWORDValue(str, 1);
	}
}

///////////////////////////////////////////////////////////////////////////////

class CodePageChecker
{
public:
	void  CheckSupport()
	{
	ATL::CRegKey  appRegKey;
		appRegKey.Create(HKEY_CURRENT_USER, ::strGryffRegKey);

	DWORD  alreadyChecked = 0;
		appRegKey.QueryDWORDValue(_T("CheckedInstalledCodePage"), alreadyChecked);
		if ( !alreadyChecked )
		{
		bool  foundCodePage(false);
			win::EnumSystemCodePages(this, &CodePageChecker::CheckSupport_callback, CP_INSTALLED, reinterpret_cast<LPARAM>(&foundCodePage));
			if ( !foundCodePage )
			{
				::MessageBox(HWND_DESKTOP, CString(MAKEINTRESOURCE(IDS_INIT_WARNING_CODEPAGE_NOT_INSTALLED)),
				  CString(MAKEINTRESOURCE(IDS_INIT_ERROR_MSGBOX_TITLE)), MB_ICONWARNING | MB_OK);
			}
			appRegKey.SetDWORDValue(_T("CheckedInstalledCodePage"), 1);
		}
	}
protected:
	BOOL  CheckSupport_callback(LPTSTR lpCodePage, LPARAM lParam)
	{
		errno = 0;
	UINT uiCodePage = static_cast<UINT>(_tcstol(lpCodePage, NULL, 10) & UINT_MAX);
		if ( errno == 0 && uiCodePage == 0x3B5 )  // CP-949
		{
			*(reinterpret_cast<bool *>(lParam)) = true;  // code page is supported
			return FALSE;
		}
		return TRUE;
	}
};



///////////////////////////////////////////////////////////////////////////////

INT_PTR  Run(LPTSTR lpCmdLine, int nCmdShow)
{
CString  strErrorTitle(MAKEINTRESOURCE(IDS_INIT_ERROR_MSGBOX_TITLE));
INT_PTR  nErrorCode;
	::strGryffRegKey.LoadString(IDS_APPLICATION_REGISTRY_KEY);

	::Welcome();
CodePageChecker checker;
	checker.CheckSupport();

class ImageListUnwind
{
public:
	ImageListUnwind(bool &isSuccessful, int &nErrorCode)
	{
		isSuccessful = ::CreateImageList(&g_pImageList, &nErrorCode);
	}
	~ImageListUnwind()
	{
		if ( g_pImageList )
		{
			int nErrorCode;
			// Ignore failure
			::DestroyImageList(&g_pImageList, &nErrorCode);
		}
	}
};
bool successful;

	ImageListUnwind ILManager(successful, nErrorCode);
	  // Create a image list that may be used throughout the application.
	  // It shall only be modified in this function
	if ( !successful )
	{
	CString  strError;
		strError.Format(CString(MAKEINTRESOURCE(IDS_INIT_ERROR_IMGLIST_FAILED)), nErrorCode);
		::MessageBox(HWND_DESKTOP, strError, strErrorTitle, MB_ICONERROR | MB_OK);
		return  4;
	}

CMessageLoop theLoop;
	if ( FALSE == _Module.AddMessageLoop(&theLoop) )
	{
		::MessageBox(HWND_DESKTOP, CString(MAKEINTRESOURCE(IDS_INIT_ERROR_MSGLOOP_FAILED)),
		             strErrorTitle, MB_ICONERROR | MB_OK);
		return  5;
	}

CMainWindow wnd;
INT_PTR nRet;
	  // Create original window
	if ( NULL == wnd.CreateEx() )
	{
		::MessageBox(HWND_DESKTOP, CString(MAKEINTRESOURCE(IDS_INIT_ERROR_CREATE_MAIN_WINDOW_FAILED)),
		             strErrorTitle, MB_ICONERROR | MB_OK);
		nRet = 6;
	}
	else
	{
		wnd.ShowWindow(nCmdShow);
		wnd.UpdateWindow();

		  // Enter the message processing loop
		nRet = theLoop.Run();
	}
	_Module.RemoveMessageLoop();
	return nRet;
}

///////////////////////////////////////////////////////////////////////////////

CString  GetWindowMenuLabel()
{
	// FIXME: TODO (1.8) Return label corresponding to active language
#if 1
	return  CString(MAKEINTRESOURCE(IDS_MENU_WINDOW));
#endif
}
