// $Id: CGrfEntry.h 9 2005-08-12 17:33:08Z Rasqual $
#if !defined(__CGRFENTRY_H__)
#define __CGRFENTRY_H__

#include  "IGrfEntry"
#include  <openkore/libgrf>
#include  <libz>
#include  <climits>

#include  <system.h>


/////
class CGrfBasicEntry : public IGrfEntry  // virtual base class
{
public:
	enum { ENTRY_MAX_PATH = MAX_PATH };
protected:
	char      m_EntryName[ENTRY_MAX_PATH];   // CP-949 encoded file or directory name
	uint8_t   m_OriginalFlags;
public:
	/**
		constructor
	**/
	CGrfBasicEntry(const char korean_oem_grffile_path[], int originalFlags)
	  throw ( GPNullPointerException, GPSizeTooLongException ) : m_OriginalFlags(originalFlags)
	{
		if ( korean_oem_grffile_path == 0 )
		{
			throw  GPNullPointerException("CGrfBasicEntry: GPNullPointerException");
		}
#if defined(__HAVE_STRNLEN__)
		if ( strnlen(korean_oem_grffile_path, ENTRY_MAX_PATH) == ENTRY_MAX_PATH )
#else  // !defined(__HAVE_STRNLEN__)
		if ( strlen(korean_oem_grffile_path) >= ENTRY_MAX_PATH )
#endif  // defined(__HAVE_STRNLEN__)
		{
			throw  GPSizeTooLongException("CGrfBasicEntry: GPSizeTooLongException");
		}
		strncpy(m_EntryName, korean_oem_grffile_path, ENTRY_MAX_PATH);
	}
	/**
		destructor
	**/
	virtual ~CGrfBasicEntry() { }

	virtual const char * GetEntryName() const
	{
		return m_EntryName;
	}

	virtual uint8_t GetOriginalFlags() const
	{
		return m_OriginalFlags;
	}

	virtual unsigned int GetExtendedInfo(int /*nFlags*/) const
	{
		return 0;
	}
	virtual void SetExtendedInfo(int /*nFlags*/, unsigned int /*value*/) { }

	virtual void Dispose() { }
};


/////
class CGrfDirEntry : public CGrfBasicEntry
{
public:
	/**
		constructor
	**/
	CGrfDirEntry(const char korean_oem_grffile_path[], int flags = 0)
	  throw ( GPNullPointerException, GPSizeTooLongException ) : CGrfBasicEntry(korean_oem_grffile_path, flags)
	{
	}
	/**
		destructor
	**/
	virtual ~CGrfDirEntry() {}

	virtual int GetEntryType() const
	{
		return DIRECTORY_ENTRY;
	}

	virtual std::pair<const uint8_t*, size_t> GetData(ICancelPoller *)
	{
		throw GPUnsupportedOperationException("GetData: unsupported");
	}
};



class CGrfCompressedEntry : public CGrfBasicEntry
{
protected:
	openkore::Grf   *m_pGrf;
	int             m_index;
	HANDLE          m_hMapping;
	LPVOID          m_pView;
	uint32_t        m_size, m_zsize, m_zsize_aligned;  // uncompressed and compressed sizes
public:
	/**
		CGrfCompressedEntry constructor
	**/
	CGrfCompressedEntry(const char korean_oem_grffile_path[], openkore::Grf *pGrf, uint32_t index, HANDLE hMapping, int flags = 0)
	  throw ( GPNullPointerException, GPSizeTooLongException, GPUnlogicalException ) : CGrfBasicEntry(korean_oem_grffile_path, flags), m_pGrf(pGrf), m_index(index), m_hMapping(hMapping), m_pView(0)
	{
		if ( pGrf == 0 )
		{
			throw GPNullPointerException("CGrfCompressedEntry: GPNullPointerException");
		}
		if ( index >= pGrf->nfiles )
		{
			throw GPUnlogicalException("CGrfCompressedEntry: index is out of bounds");
		}
		m_size = m_pGrf->files[index].real_len;
		m_zsize = m_pGrf->files[index].compressed_len;
		m_zsize_aligned = m_pGrf->files[index].compressed_len_aligned;
	}

	/**
		CGrfCompressedEntry destructor
	**/
	virtual ~CGrfCompressedEntry()
	{
		Dispose();
	}

	virtual int GetEntryType() const
	{
		return FILE_COMPRESSED_ENTRY;
	}

	virtual std::pair<const uint8_t*, size_t> GetData(ICancelPoller *pPoller)
	{
		// If not encrypted, return pointer to desired data (avoid double malloc that has big impact on large files)
		if ( 0 == (m_pGrf->files[m_index].flags & (GRFFILE_FLAG_MIXCRYPT |GRFFILE_FLAG_0x14_DES)) )
		{
			uintptr_t memAddress = Memory::RoundDown( m_pGrf->files[m_index].pos );
			if ( !m_pView )
			{
				// Aligned begin: file offset rounded down
			DWORD begin_h = static_cast<DWORD>((INT64(memAddress)>>32U) & 0xFFFFFFFFU);
			DWORD begin_l = static_cast<DWORD>(memAddress & 0xFFFFFFFFU);
			SIZE_T mappedLength = m_pGrf->files[m_index].pos - memAddress + m_pGrf->files[m_index].compressed_len_aligned;
				m_pView = ::MapViewOfFile(m_hMapping, FILE_MAP_READ, begin_h, begin_l, mappedLength);
			}
			if ( !m_pView )
			{
				return std::pair<const uint8_t*, size_t>(0, 0);
			}
		const uint8_t * tmp = (uint8_t *)m_pView;
			return std::pair<const uint8_t*, size_t>(tmp + m_pGrf->files[m_index].pos - memAddress, m_zsize_aligned);
		}
	uint32_t size, zsize;
	openkore::GrfError error;
		(*pPoller)();
	const uint8_t * tmp = (uint8_t *)openkore::grf_index_get_z(m_pGrf, m_index, &zsize, &size, &error);
		(*pPoller)();
		return std::pair<const uint8_t*, size_t>(tmp, m_zsize_aligned);
	}

	virtual unsigned int GetExtendedInfo(int nFlags) const
	{
		if ( nFlags & XNFO_UNCOMP_SIZE )
		{
			return m_size;
		}
		else if(nFlags & XNFO_COMP_SIZE_RAW )
		{
			return m_zsize;
		}
		return 0;
	}

	virtual void SetExtendedInfo(int /*nFlags*/, unsigned int /*value*/) { }

	virtual void  Dispose()
	{
		if ( m_pView )
		{
			::UnmapViewOfFile(reinterpret_cast<LPCVOID>(Memory::RoundDown(reinterpret_cast<uintptr_t>(m_pView))));
			m_pView = 0;
		}
	}

};


/////
template <typename TFileIoManager /* : public IFileIoManager */>
class CGrfEntry : public CGrfBasicEntry
{
protected:
	TFileIoManager    *m_IoManager;
private:
	int       m_UseDelayLoad;
	wchar_t   *m_pFilePath;   // for delay load
	bool      m_Loaded;       //     ''
	CHeapPtr<uint8_t>   m_pData;
	size_t    m_DataLength;
	uint32_t  m_zsize;  // compressed sizes

public:
	/**
		CGrfEntry constructor
	**/
	CGrfEntry(const char korean_oem_grffile_path[], const wchar_t utf16le_encoded_file_path[], TFileIoManager *pt_manager, int flags = 0, bool UseDelayLoad = true)
	  throw ( GPNullPointerException, GPSizeTooLongException, GPInvalidSequenceException, GPIoException, GPUnlogicalException, GPUnsupportedOperationException  ) : CGrfBasicEntry(korean_oem_grffile_path, flags), m_IoManager(pt_manager), m_UseDelayLoad(UseDelayLoad), m_pFilePath(0), m_Loaded(!UseDelayLoad), m_DataLength(0), m_zsize(0)
	{
		// precheck
		if ( utf16le_encoded_file_path == 0 )
		{
			throw GPNullPointerException("CGrfEntry: GPNullPointerException");
		}
		if ( !UTF16::IsValidString((UTF16_t*)utf16le_encoded_file_path) )
		{
			throw GPInvalidSequenceException("CGrfEntry: GPInvalidSequenceException");
		}

		if ( !UseDelayLoad )
		{
			ReadData_(utf16le_encoded_file_path);
		}
		else
		{
			size_t elt_count = 0U;
			wchar_t *ptr = const_cast<wchar_t *>(utf16le_encoded_file_path);
			while ( *ptr != L'\0' )
			{
				++elt_count;
				++ptr;
			}
			m_pFilePath = new wchar_t[elt_count+1];
			memcpy(m_pFilePath, utf16le_encoded_file_path, (elt_count+1) * sizeof(wchar_t));
		}
	}

	/**
		CGrfEntry destructor
	**/
	virtual ~CGrfEntry()
	{
	}

	virtual int GetEntryType() const
	{
		return m_Loaded? FILE_COMPRESSED_ENTRY : FILE_DELAYLOAD_ENTRY;
	}

	virtual std::pair<const uint8_t*, size_t> GetData(ICancelPoller *pPoller)
	{
		if ( !m_Loaded )
		{
			ATLASSERT(m_pFilePath != 0);

			(*pPoller)();
			ReadData_(m_pFilePath);
			(*pPoller)();
			m_Loaded = true;
		}
		return 	std::pair<const uint8_t*, size_t>(m_pData, m_zsize);
	}

private:
	std::pair<uint8_t*,size_t>  ReadData_(const wchar_t utf16le_encoded_file_path[])
	{
		USES_ATL_SAFE_ALLOCA;
		const size_t BufferElementsCount = 65536;
		CONST SIZE_T AllocByteCount = BufferElementsCount * sizeof(uint8_t); 

		m_IoManager->AcquireOpenRead((UTF16_t*)utf16le_encoded_file_path);
		int64_t offset = 0;
		int64_t filesize = m_IoManager->Seek(offset, IFileIoManager::FPTR_END);
		if ( filesize - UINT_MAX > 0 )
		{
			throw  GPSizeTooLongException("ReadData_: Cannot handle files whose size cannot fit in a 32-bit variable");
		}
		m_DataLength = static_cast<size_t>(filesize & 0xffffffffui32);
		m_IoManager->Seek(offset, IFileIoManager::FPTR_SET);

		uint8_t * tmpBuffer = reinterpret_cast<uint8_t *>(
		  _ATL_SAFE_ALLOCA(AllocByteCount, AllocByteCount));
		MyCompress_(uint32_t(m_DataLength), tmpBuffer, BufferElementsCount);
		m_IoManager->ForceClose();
		return std::pair<uint8_t*, size_t>(m_pData, m_zsize);
	}

	int  MyCompress_(const uint32_t &streamSize, uint8_t *helperBuffer, size_t bufferSize)
	{
		zlib::uLongf  comp_len = zlib::compressBound(static_cast<zlib::uLong>(streamSize));  // Determine how large the destination might be
		int err = Z_OK;
		CHeapPtr<zlib::Bytef> comp_dat;
		if ( !comp_dat.Allocate(comp_len) )
		{
			throw GPMemAllocateException("MyCompress_: GPMemAllocateException");
		}
		if ( streamSize == 0 )
		{
			err = zlib::compress(comp_dat, &comp_len, helperBuffer, static_cast<zlib::uLong>(0));
			if (err != Z_OK) return err;
			m_zsize = comp_len;
			m_pData = comp_dat;  // transfer ownership
		}

		m_zsize = 0;  // Set offset
		unsigned int  uoffset = 0;  // uncompressed read data

		zlib::z_stream stream;
		{
			stream.zalloc = (zlib::alloc_func)0;
			stream.zfree = (zlib::free_func)0;
			stream.opaque = (zlib::voidpf)0;
		}

		stream.next_out = comp_dat.operator zlib::Bytef *() /*+ m_zsize*/;  // comp_dat with offset
		stream.avail_out = (zlib::uInt)comp_len /*- m_zsize*/;

		// Initialize deflate with stream
		err = zlib::deflateInit(&stream, Z_DEFAULT_COMPRESSION);
		if (err != Z_OK) return err;

		while ( streamSize - uoffset > 0 )  // Read all data
		{
			stream.next_in = (zlib::Bytef*)helperBuffer;  // buffer where uncompressed data read from io manager is put
		const unsigned int readBytesCount = m_IoManager->ReadBytes(helperBuffer, static_cast<unsigned int>(bufferSize));
			uoffset += readBytesCount;
			stream.avail_in = (zlib::uInt)readBytesCount;

			// Now compress that input chunk
			do
			{
				err = zlib::deflate(&stream, Z_NO_FLUSH);
			} while ( stream.avail_in != 0 || stream.avail_out == 0 );  // since we used compressBound we should in theory never loop
			// end compress chunk, branch to while() beginning for reading more data
		}
		stream.avail_in = 0;
		err = zlib::deflate(&stream, Z_FINISH);
		err = zlib::deflateEnd(&stream);
		if (err != Z_OK) return err;
		m_zsize = stream.total_out;
		comp_dat.ReallocateBytes(m_zsize);
		m_pData = comp_dat;  // transfer ownership
		return Z_OK;
	}

public:
	virtual unsigned int GetExtendedInfo(int nFlags) const
	{
		if ( nFlags & XNFO_UNCOMP_SIZE )
		{
			return static_cast<unsigned int>(m_DataLength);
		}
		else if(nFlags & XNFO_COMP_SIZE_RAW )
		{
			return m_zsize;
		}
		return 0;
	}

	virtual void SetExtendedInfo(int /*nFlags*/, unsigned int /*value*/) { }

	virtual void  Dispose()
	{
		if ( m_UseDelayLoad && m_Loaded )
		{
			m_pData.Free();
			m_Loaded = false;
		}
	}
};

#endif  // !defined(__CGRFENTRY_H__)
