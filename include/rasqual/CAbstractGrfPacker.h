// $Id: CAbstractGrfPacker.h 9 2005-08-12 17:33:08Z Rasqual $
#if !defined(__CABSTRACTGRFPACKER_H__)
#define __CABSTRACTGRFPACKER_H__

  // Interface
#include  "IGrfPacker"

class CAbstractGrfPacker : public IGrfPacker
{
public:
	CAbstractGrfPacker() {}
	virtual ~CAbstractGrfPacker() {}

public:
	/**
		Implements IGrfPacker::PackFile()
		Call CAbstractGrfPacker::PackFile() in subclasses to detect
		GPNullPointerException and GPInvalidSequenceException.
	**/
	virtual int PackFile(const UTF16_t utf16le_encoded_filepath[])
	  throw ( GPNullPointerException, GPInvalidSequenceException, GPIoException, GPUnlogicalException, GPIllegalStateException, GPUnsupportedOperationException );

	/**
		Implements IGrfPacker::SetVersion()
	**/
	virtual uint16_t SetVersion(uint16_t that_version)
	  throw ( GPInvalidParameterException )
	{
		if ( !IsSupportedVersion(that_version) )
		{
			throw GPInvalidParameterException("SetVersion: unsupported version");
		}
		uint16_t ver = this->version;
		this->version = that_version;
		return ver;
	}
	/**
		Implements IGrfPacker::GetVersion()
	**/
	virtual uint16_t GetVersion() const
	{
		return this->version;
	}

protected:
	uint16_t version;

private:
	CAbstractGrfPacker(const CAbstractGrfPacker&);
	CAbstractGrfPacker &operator=(const CAbstractGrfPacker&);
};

#endif  // !defined(__CABSTRACTGRFPACKER_H__)
