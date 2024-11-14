// $Id: CWinApiFileIoManager.h 9 2005-08-12 17:33:08Z Rasqual $
#if !defined(__CWINAPIFILEIOMANAGER_H__)
#define __CWINAPIFILEIOMANAGER_H__

#include  <windows.h>

  // Interface
#include  "IFileIoManager"

class CWinApiFileIoManager : public IFileIoManager
{
protected:
	HANDLE file;
public:
	CWinApiFileIoManager() : file(INVALID_HANDLE_VALUE)
	{}
	virtual ~CWinApiFileIoManager();

	/**
		Implements IFileIoManager::AcquireOpenWrite()
	**/
	virtual void AcquireOpenWrite(const UTF16_t strFilePath[])
	  throw ( GPNullPointerException, GPInvalidSequenceException, GPIoException, GPUnlogicalException );

	/**
		Implements IFileIoManager::AcquireOpenRead()
	**/
	virtual void AcquireOpenRead(const UTF16_t strFilePath[])
	  throw ( GPNullPointerException, GPInvalidSequenceException, GPIoException, GPUnlogicalException );

	/**
		Implements IFileIoManager::Seek()
	**/
	virtual	int64_t Seek(const int64_t &offset, int whence)
	  throw ( GPUnlogicalException, GPIoException, GPIllegalStateException );

	/**
		Implements IFileIoManager::WriteBytes()
	**/
	virtual	void WriteBytes(const uint8_t data[], unsigned int len)
	  throw ( GPIoException, GPIllegalStateException );

	/**
		Implements IFileIoManager::ReadBytes()
	**/
	virtual	unsigned int ReadBytes(uint8_t dest[], unsigned int len)
	  throw ( GPInvalidParameterException, GPIoException, GPIllegalStateException );

	/**
		Implements IFileIoManager::ForceClose()
	**/
	virtual void ForceClose() throw ( );

	/**
		Implements IFileIoManager::Move()
	**/
	static void Move(const UTF16_t strSourceFilePath[], const UTF16_t strDestinationFilePath[])
	  throw ( GPNullPointerException, GPInvalidSequenceException, GPIoException, GPUnlogicalException );
};


#endif  // !defined(__CWINAPIFILEIOMANAGER_H__)
