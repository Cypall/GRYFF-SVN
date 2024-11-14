// $Id: url.h 42 2005-06-02 12:47:47Z Rasqual $
// Open a URL in a new window
// Author: Robert Edward Caldecott
//         http://www.codeproject.com/internet/urlnewwindow.asp
// Modifications by toshiya (http://www.codeproject.com/internet/urlnewwindow.asp?df=100&forumid=16169&exp=0&select=964593#xx964593xx)
#pragma once

class CURL
{
private:
	// The default browser
	CString m_strBrowser;
public:
	void Open(LPCTSTR lpszURL, bool bNewWindow = true)
	{
		if (bNewWindow)
		{
			GetBrowser();
		}
		if (!bNewWindow || m_strBrowser.IsEmpty())
		{
			::ShellExecute(NULL, NULL, lpszURL, NULL, NULL, SW_SHOWNORMAL);
		}
		else
		{
			::ShellExecute(NULL, NULL, m_strBrowser, lpszURL, NULL, SW_SHOWNORMAL);
		}
	}

	LPCTSTR GetBrowser(void)
	{
		// Do we have the default browser yet?
		if (m_strBrowser.IsEmpty())
		{
			// Get the default browser from HKCR\http\shell\open\command
			HKEY hKey = NULL;
			// Open the registry
			if (::RegOpenKeyEx(HKEY_CLASSES_ROOT, _T("http\\shell\\open\\command"), 0,
					KEY_READ, &hKey) == ERROR_SUCCESS)
			{
				// Data size
				DWORD cbData = 0;
				// Get the default value
				if (::RegQueryValueEx(hKey, NULL, NULL, NULL, NULL, &cbData) == ERROR_SUCCESS && cbData > 0)
				{
					// Allocate a suitable buffer
					TCHAR* psz = new TCHAR [cbData];
					// Success?
					if (psz != NULL)
					{
						if (::RegQueryValueEx(hKey, NULL, NULL,
							NULL, (LPBYTE)psz, &cbData) ==
							ERROR_SUCCESS)
						{
							// Success!
							m_strBrowser = psz;
						}
						delete [] psz;
					}
				}
				::RegCloseKey(hKey);
				// Do we have the browser?
				if (!m_strBrowser.IsEmpty())
				{
					int nStart = 0;
					int nEnd = m_strBrowser.GetLength();
					if (m_strBrowser[0] == _T('\"'))
					{
						++nStart;
						if ( -1 == (nEnd = m_strBrowser.Find(_T('\"'), 1)) )
						{
							nEnd = m_strBrowser.GetLength();
						}
					}
					else
					{
						if ( -1 == (nEnd = m_strBrowser.Find(_T(' '))) )
						{
							nEnd = m_strBrowser.GetLength();
						}
					}
					// Get the full path
					m_strBrowser = m_strBrowser.Mid(nStart, nEnd - nStart);
				}
			}
		}
		// Done
		return m_strBrowser;
	}
};
