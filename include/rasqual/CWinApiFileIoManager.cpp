// $Id: CWinApiFileIoManager.cpp 9 2005-08-12 17:33:08Z Rasqual $
#ifdef WIN32
#include  <windows.h>
#endif /* WIN32 */

#include  <gen_types.h>
#include  "CWinApiFileIoManager.h"

CWinApiFileIoManager::~CWinApiFileIoManager()
{
	if ( file != INVALID_HANDLE_VALUE )
	{
		::CloseHandle(file);
		file = INVALID_HANDLE_VALUE;
	}
}

/**
	Implements IFileIoManager::AcquireOpenWrite()
**/
void CWinApiFileIoManager::AcquireOpenWrite(const UTF16_t strFilePath[])
  throw ( GPIoException, GPUnlogicalException )
{
	ForceClose();
	if ( strFilePath == 0 )
	{
		throw GPNullPointerException("AcquireOpenWrite: GPNullPointerException");
	}

	if ( !UTF16::IsValidString(strFilePath) )
	{
		throw GPInvalidSequenceException("AcquireOpenWrite: GPInvalidSequenceException");
	}
	WIN32_FILE_ATTRIBUTE_DATA attributes;
	if ( ::GetFileAttributesExW((LPCWSTR)strFilePath, GetFileExInfoStandard, &attributes) )
	{
		if ( (attributes.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 )
		{
			throw GPUnlogicalException(std::string("AcquireOpenWrite: cannot acquire directory"));
		}
	}

	file = ::CreateFileW((LPCWSTR)strFilePath, GENERIC_WRITE, 0 /* file share */, NULL /* inherit ACL */, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL /* template handle */);
	if ( file == INVALID_HANDLE_VALUE )
	{
		GPIoException e(std::string("AcquireOpenWrite: acquire failed"));
		e.SetErrno(::GetLastError());
		throw e;
	}
}

/**
	Implements IFileIoManager::AcquireOpenRead()
**/
void CWinApiFileIoManager::AcquireOpenRead(const UTF16_t strFilePath[])
  throw ( GPIoException, GPUnlogicalException )
{
	ForceClose();
	if ( strFilePath == 0 )
	{
		throw GPNullPointerException("AcquireOpenRead: GPNullPointerException");
	}

	if ( !UTF16::IsValidString(strFilePath) )
	{
		throw GPInvalidSequenceException("AcquireOpenRead: GPInvalidSequenceException");
	}	WIN32_FILE_ATTRIBUTE_DATA attributes;
	if ( ::GetFileAttributesExW((LPCWSTR)strFilePath, GetFileExInfoStandard, &attributes) )
	{
		if ( (attributes.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 )
		{
			throw GPUnlogicalException("AcquireOpenRead: cannot acquire directory");
		}
	}

	file = ::CreateFileW((LPCWSTR)strFilePath, GENERIC_READ, FILE_SHARE_READ, NULL /* inherit ACL */, OPEN_EXISTING, 0 /* FlagsAndAttributes */, NULL /* template handle */);
	if ( file == INVALID_HANDLE_VALUE )
	{
		GPIoException e("AcquireOpenRead: acquire failed");
		e.SetErrno(::GetLastError());
		throw e;
	}
}

/**
	Implements IFileIoManager::Seek()
**/
int64_t CWinApiFileIoManager::Seek(const int64_t &offset, int whence)
  throw ( GPUnlogicalException, GPIoException, GPIllegalStateException )
{
	LARGE_INTEGER li, UpdatedOffset;
	li.QuadPart = offset;
	static const DWORD mode[] = { FILE_BEGIN, FILE_CURRENT, FILE_END };
	if ( file == INVALID_HANDLE_VALUE )
	{
		throw GPIllegalStateException("Seek: no acquired resource");
	}
	if ( whence < FPTR_MODE_MIN_ || whence > FPTR_MODE_MAX_ )
	{
		throw GPUnlogicalException("Seek: invalid whence flag");
	}
	if ( !::SetFilePointerEx(file, li, &UpdatedOffset, mode[whence]) )
	{
		GPIoException e("Seek: failed");
		e.SetErrno(::GetLastError());
		throw e;
	}
	return UpdatedOffset.QuadPart;
}

/**
	Implements IFileIoManager::WriteBytes()
**/
void CWinApiFileIoManager::WriteBytes(const uint8_t data[], unsigned int len)
  throw ( GPIoException, GPIllegalStateException )
{
	if ( file == INVALID_HANDLE_VALUE )
	{
		throw GPIllegalStateException("WriteBytes: no acquired resource");
	}
	if ( len )
	{
		if ( data == 0 )
		{
			// nothing to write actually
			return;
		}
		DWORD WrittenBytes;
		if ( !::WriteFile(file, data, len, &WrittenBytes, NULL /* Overlapped */) )
		{
			GPIoException e("WriteBytes: failed");
			e.SetErrno(::GetLastError());
			throw e;
		}
	}
}

/**
	Implements IFileIoManager::ReadBytes()
**/
unsigned int CWinApiFileIoManager::ReadBytes(uint8_t dest[], unsigned int len)
  throw ( GPInvalidParameterException, GPIoException, GPIllegalStateException )
{
	if ( file == INVALID_HANDLE_VALUE )
	{
		throw GPIllegalStateException("ReadBytes: no acquired resource");
	}
	if ( len )
	{
		if ( dest == 0 )
		{
			throw GPInvalidParameterException("ReadBytes: bad parameter");
		}
		DWORD ReadBytes;
		if ( !::ReadFile(file, dest, len, &ReadBytes, NULL /* Overlapped */) )
		{
			GPIoException e("ReadBytes: failed");
			e.SetErrno(::GetLastError());
			throw e;
		}
		return static_cast<unsigned int>(ReadBytes);
	}
	return 0;
}

/**
	Implements IFileIoManager::ForceClose()
**/
void CWinApiFileIoManager::ForceClose() throw ( )
{
	if ( file != INVALID_HANDLE_VALUE )
	{
		::CloseHandle(file);
		file = INVALID_HANDLE_VALUE;
	}
}


/**
	Implements IFileIoManager::Move()
**/
void CWinApiFileIoManager::Move(const UTF16_t strSourceFilePath[], const UTF16_t strDestinationFilePath[])
	throw ( GPNullPointerException, GPInvalidSequenceException, GPIoException, GPUnlogicalException )
{
	if ( strSourceFilePath == 0 || strDestinationFilePath == 0 )
	{
		throw GPNullPointerException("Move: GPNullPointerException");
	}

	if ( !UTF16::IsValidString(strSourceFilePath) || !UTF16::IsValidString(strDestinationFilePath) )
	{
		throw GPInvalidSequenceException("Move: GPInvalidSequenceException");
	}
	WIN32_FILE_ATTRIBUTE_DATA attributes;
	if ( GetFileAttributesExW((LPCWSTR)strSourceFilePath, GetFileExInfoStandard, &attributes) )
	{
		if ( (attributes.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 )
		{
			throw GPUnlogicalException("Move: cannot move directory");
		}
	}
	if ( FALSE == ::MoveFileExW((LPCWSTR)strSourceFilePath, (LPCWSTR)strDestinationFilePath, MOVEFILE_REPLACE_EXISTING) )
	{
	}

}
