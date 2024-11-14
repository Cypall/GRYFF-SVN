// $Id: CGrfPacker.h 9 2005-08-12 17:33:08Z Rasqual $
#if !defined(__CGRFPACKER_H__)
#define __CGRFPACKER_H__

#include  <vector>

#include  <atlcore.h>
#include  <libz>

#include  "CAbstractGrfPacker.h"

/* Headers */
#define GRF_HEADER		"Master of Magic"
#define GRF_HEADER_LEN		(sizeof(GRF_HEADER)-1)	/* -1 to strip
                                                     * null terminator
                                                     */
#define GRF_HEADER_MID_LEN	(sizeof(GRF_HEADER)+0xE)	/* -1 + 0xF */
#define GRF_HEADER_FULL_LEN	(sizeof(GRF_HEADER)+0x1E)	/* -1 + 0x1F */

inline uint32_t HostToLittleEndian32(const uint32_t &val)
{
#ifdef _WIN32
	return val;
#endif
}

template <typename TFileIoManager /* : public IFileIoManager */>
class CGrfPacker : public CAbstractGrfPacker
{
	typedef int (*CALLBACK_FUNCTION_PTR)(int, IGrfEntry *, LPARAM);

public:
	static ICancelPoller NoCancel;

public:
	/**
		callback_next is a function called before processing each entry taking param as
		a user-defined parameter. It should return 0 if processing may continue, 1 if
		it should be put on hold and -1 if the task has been cancelled.
	**/
	CGrfPacker(int informativeSize = 0, CALLBACK_FUNCTION_PTR callback_next = 0, LPARAM lParam = 0, const UTF16_t utf16le_encoded_temp_filepath[] = 0)
		throw ( GPNullPointerException, GPInvalidSequenceException, GPIoException, GPUnlogicalException, GPUnsupportedOperationException ) : CAbstractGrfPacker(), IoManager(new TFileIoManager()), m_CurrentTableOffset(0U), m_EntryCount(0), m_FinalizedImage(false), m_pCallback(callback_next), m_CallbackParam(lParam), m_pCancelEmitter(&CGrfPacker::NoCancel)
	{
		this->version = this->GetMaxVersion();
		if ( utf16le_encoded_temp_filepath != 0 )
		{
			IoManager->AcquireOpenWrite(utf16le_encoded_temp_filepath);
			memcpy(m_TempFileName, utf16le_encoded_temp_filepath, MAX_PATH * sizeof(UTF16_t));
		}
		else
		{
			IoManager->AcquireOpenWrite(reinterpret_cast<const UTF16_t *>(CGrfPacker<TFileIoManager>::TEMP_FILE_NAME));
			memcpy(m_TempFileName, CGrfPacker<TFileIoManager>::TEMP_FILE_NAME, MAX_PATH * sizeof(UTF16_t));
		}
		this->WriteHeader_(IoManager);
		informativeSize = informativeSize == 0 ? INITIAL_ENTRIES_REGISTRY_SIZE : informativeSize;
		zlib::uLongf  decomp_len = static_cast<zlib::uLongf>(informativeSize * ( GRF_NAMELEN * sizeof(char) + sizeof(EntryCompressInfo) ));
		if ( !m_FileInfoTable.Allocate(decomp_len) )
		{
			throw GPMemAllocateException("CGrfPacker: GPMemAllocateException (temp uncompressed entry table)");
		}
	}
	virtual ~CGrfPacker()
	{
		if ( IoManager )
		{
			IoManager->ForceClose();
			delete IoManager;
		}
	}

private:
	struct EntryCompressInfo {
		uint32_t  c_len;    // comp len
		uint32_t  c_len_a;  // comp len, 8-aligned (for block enc)
		uint32_t  d_len;    // decomp len
		uint8_t   e_flags;   // "type"
		uint32_t  g_off;    // grf offset
	};

public:
	virtual void SetCancelStatePoller(ICancelPoller *pCancelEmitter)
	{
		m_pCancelEmitter = pCancelEmitter;
	}
	virtual ICancelPoller *GetCancelStatePoller() const
	{
		return m_pCancelEmitter;
	}


public:
	/**
		Implements IGrfPacker::AddEntry
		Files are written as they are fed to the packer
		For efficiency, all added entries are supposed to be unique.
	**/
	virtual IGrfEntry * AddEntry(IGrfEntry *pEntry)
	  throw ( GPNullPointerException, GPCancelOperationException )
	{
		if ( pEntry == 0 )
		{
			throw GPNullPointerException("AddEntry: GPNullPointerException");
		}
		if ( m_FinalizedImage )
		{
			throw GPIllegalStateException("AddEntry: document has already been finalized!");
		}
		// IoManager cannot be 0 because: not finalized yet
		// Next, attempt to flush entry to disk without caching anything
		  // First, invoke the callback function
		if ( m_pCallback != 0 )
		{
			int CallbackRetValue = m_pCallback(m_EntryCount, pEntry, m_CallbackParam);
			if ( CallbackRetValue != 0 )
			{
				  // Aborting
				if ( CallbackRetValue == -1 )
				{
					IoManager->ForceClose();
					return 0;
				}
				if ( CallbackRetValue == 1 )
				{
					// FIXME: TODO: (x.x) should wait until allowed to resume task
				}
				else
				{
					//? Should not be here
					return 0;
				}
			}
		}
		(*m_pCancelEmitter)();
		this->WriteEntry_(IoManager, pEntry);
		(*m_pCancelEmitter)();
		return pEntry;
	}

	virtual void RemoveEntry(const IGrfEntry *pEntry)
	  throw ( GPNullPointerException, GPUnsupportedOperationException )
	{
		if ( pEntry == 0 )
		{
			throw GPNullPointerException("RemoveEntry: GPNullPointerException");
		}
		throw GPUnsupportedOperationException("RemoveEntry: unsupported by CGrfPacker");
	}

	/**
		Implements IGrfPacker::PackFile()
		Finalizes the document, nothing can be written afterwards
	**/
	virtual int PackFile(const UTF16_t utf16le_encoded_filepath[])
	  throw ( GPNullPointerException, GPInvalidSequenceException, GPIoException, GPUnlogicalException, GPIllegalStateException, GPUnsupportedOperationException )
	{
		if (!IoManager)
		{
			throw GPIllegalStateException("AddEntry: document has already been finalized!");
		}
		// Finalize temp.
		this->WriteHeader_(IoManager, true);
		IoManager->ForceClose();

		int  BaseRetVal = CAbstractGrfPacker::PackFile(utf16le_encoded_filepath);
		if ( BaseRetVal != 0 )
		{
			return BaseRetVal;
		}
		TFileIoManager::Move(m_TempFileName, utf16le_encoded_filepath);
		delete IoManager;
		IoManager = 0;
		return 0;
	}

private:
	void WriteHeader_(IFileIoManager *mgr, bool Finalize = false)
	  throw ( GPIoException, GPUnlogicalException, GPIllegalStateException, GPUnsupportedOperationException, GPMemAllocateException )
	{
		if (m_FinalizedImage)
		{
			return;
		}
		if ( mgr == 0 )
		{
			throw GPIllegalStateException("WriteHeader_: manager not initialized");
		}

		if ( !Finalize )  // first pass
		{
			int64_t reloffset(0);
			mgr->Seek(reloffset, IFileIoManager::FPTR_SET);

			uint8_t  buf[GRF_HEADER_FULL_LEN], *ptrbuf = buf + GRF_HEADER_LEN;

			  // Copy the Master of Magic signature
			memcpy(buf, GRF_HEADER, GRF_HEADER_LEN);
			for ( unsigned int ui = 0; ui < (GRF_HEADER_MID_LEN - GRF_HEADER_LEN); ++ui )
			{
				*ptrbuf = static_cast<uint8_t>(ui & 0xFF);
				++ptrbuf;
			}
			  // Skip "entry offset", "seed" and "number of entries". "version" is set hereafter.
			memset(ptrbuf, 0, GRF_HEADER_FULL_LEN - GRF_HEADER_MID_LEN);
			uint32_t *ptrVersion = reinterpret_cast<uint32_t *>(buf + GRF_HEADER_FULL_LEN - 4);
			*ptrVersion = ::HostToLittleEndian32(version);

			mgr->WriteBytes(buf, sizeof(buf));
		}
		else  // second pass
		{
			CHeapPtr<zlib::Bytef> comp_dat;
			zlib::uLongf  comp_len;
			  // Initialize with initial, upper size
			comp_len = zlib::compressBound(static_cast<zlib::uLongf>(m_CurrentTableOffset));
			if ( !comp_dat.Allocate(comp_len) )
			{
				throw GPMemAllocateException("WriteHeader_: GPMemAllocateException (compressed entry table)");
			}
			  // After done: comp_len is set to exact length
			zlib::compress(comp_dat, &comp_len, m_FileInfoTable, static_cast<zlib::uLong>(m_CurrentTableOffset));
			m_FileInfoTable.Free();

			int64_t reloffset(0);
			int64_t position = mgr->Seek(reloffset, IFileIoManager::FPTR_CUR);
			position -= GRF_HEADER_FULL_LEN;  // corrects offset to header
			uint32_t  comp_len32 = ::HostToLittleEndian32(comp_len);
			uint32_t  byte_count32 = ::HostToLittleEndian32(m_CurrentTableOffset);

			// size of compressed block
			mgr->WriteBytes(reinterpret_cast<const uint8_t*>(&comp_len32), sizeof(uint32_t));
			// size of uncompressed block
			mgr->WriteBytes(reinterpret_cast<const uint8_t*>(&byte_count32), sizeof(uint32_t));
			// compressed block
			mgr->WriteBytes(comp_dat, comp_len);

			// Set the correct values at the beginning
			reloffset = GRF_HEADER_MID_LEN;
			mgr->Seek(reloffset, IFileIoManager::FPTR_SET);
			uint32_t  pos32 = ::HostToLittleEndian32(static_cast<uint32_t>(position & 0xFFFFFFFF));
			uint32_t  dummy_seed = 0;
			uint32_t  entry_count32 = ::HostToLittleEndian32( static_cast<uint32_t>(m_EntryCount + 7) );
			mgr->WriteBytes(reinterpret_cast<const uint8_t*>(&pos32), sizeof(uint32_t));
			mgr->WriteBytes(reinterpret_cast<const uint8_t*>(&dummy_seed), sizeof(uint32_t));
			mgr->WriteBytes(reinterpret_cast<const uint8_t*>(&entry_count32), sizeof(uint32_t));
			m_FinalizedImage = true;
		}
	}

private:
	void WriteEntry_(IFileIoManager *mgr, IGrfEntry *pEntry)
	  throw ( GPInvalidParameterException, GPIoException, GPIllegalStateException, GPMemAllocateException, GPCancelOperationException )
	{
		if ( m_FinalizedImage )
		{
			throw GPIllegalStateException("WriteEntry_: document has already been finalized!");
		}
		EntryCompressInfo  cinfo = {0};

		if ( pEntry->GetEntryType() & IGrfEntry::FILE_ENTRY )  // binary AND
		{
			std::pair<const uint8_t*, size_t> p(pEntry->GetData(m_pCancelEmitter));
			int64_t reloffset(0);
			int64_t position = mgr->Seek(reloffset, IFileIoManager::FPTR_CUR);
			cinfo.e_flags   = version == pEntry->GetExtendedInfo(IGrfEntry::XNFO_ORIG_VERSION) ? pEntry->GetOriginalFlags() : GRFFILE_FLAG_FILE;
			// Set the position before writing
			cinfo.g_off    = static_cast<uint32_t>(position);

			if ( p.second != 0 )
			{
				if ( pEntry->GetEntryType() != IGrfEntry::FILE_COMPRESSED_ENTRY )
				{
					zlib::uLongf  comp_len = zlib::compressBound(static_cast<zlib::uLong>(p.second));  // Determine how large the destination might be

					CHeapPtr<zlib::Bytef> comp_dat;
					if ( !comp_dat.Allocate(comp_len) )
					{
						throw GPMemAllocateException("WriteEntry_: GPMemAllocateException");
					}

					(*m_pCancelEmitter)();
					zlib::compress(comp_dat, &comp_len, p.first, static_cast<zlib::uLong>(p.second));

					const uint8_t *pPtr(comp_dat);
					const unsigned int CHUNK_SIZE = 4096U;
					for ( unsigned int writtenBytes = 0; writtenBytes < comp_len; )
					{
						unsigned int plannedBytes = CHUNK_SIZE;
						if ( CHUNK_SIZE + writtenBytes > comp_len )
						{
							plannedBytes = comp_len - writtenBytes;
						}
						(*m_pCancelEmitter)();
						mgr->WriteBytes(pPtr, plannedBytes);
						pPtr += plannedBytes;
						writtenBytes += plannedBytes;
					}
					mgr->WriteBytes(comp_dat, comp_len);
					cinfo.c_len    = comp_len;
					cinfo.c_len_a  = comp_len;
					cinfo.d_len    = static_cast<uint32_t>(p.second);
				}
				else  // pre-compressed
				{
					const uint8_t *pPtr(p.first);
					const unsigned int CHUNK_SIZE = 4096U;
					for ( unsigned int writtenBytes = 0; writtenBytes < static_cast<unsigned int>(p.second); )
					{
						unsigned int plannedBytes = CHUNK_SIZE;
						if ( CHUNK_SIZE + writtenBytes > static_cast<unsigned int>(p.second) )
						{
							plannedBytes = static_cast<unsigned int>(p.second) - writtenBytes;
						}
						(*m_pCancelEmitter)();
						mgr->WriteBytes(pPtr, plannedBytes);
						pPtr += plannedBytes;
						writtenBytes += plannedBytes;
					}
					cinfo.c_len    = pEntry->GetExtendedInfo(IGrfEntry::XNFO_COMP_SIZE_RAW);
					cinfo.c_len_a  = static_cast<uint32_t>(p.second);
					cinfo.d_len    = pEntry->GetExtendedInfo(IGrfEntry::XNFO_UNCOMP_SIZE);
				}
				(*m_pCancelEmitter)();
				pEntry->Dispose();
			}
			else  // empty file
			{
				cinfo.c_len    = 0;
				cinfo.c_len_a  = 0;
				cinfo.d_len    = 0;
			}
		}
		else  // Dir
		{
			cinfo.c_len    = GRFFILE_DIR_SZSMALL;// 	0x0449	compressed_len for directory entries
			cinfo.c_len_a  = GRFFILE_DIR_SZFILE; // 	0x0714	compressed_len_aligned for directory entries
			cinfo.d_len    = GRFFILE_DIR_SZORIG; // 	0x055C	real_len for directory entries
			cinfo.e_flags  = 2;                  // GRFFILE_FLAG_FILE not set
			cinfo.g_off    = GRFFILE_DIR_OFFSET; // 	0x058A	pos value for directory entries
		}
		(*m_pCancelEmitter)();

		// Write to entries table in memory, in that order:
		// CP949 filename\0, compressed length, compresed length aligned, uncompressed length, flags, entry offset to header
		{
			const char *entryName = pEntry->GetEntryName();
			size_t namelen = strlen(entryName);
			strncpy((char*)((zlib::Bytef*)m_FileInfoTable)+m_CurrentTableOffset, entryName, namelen+1);
			m_CurrentTableOffset += namelen+1;
		}
		{
			*(uint32_t*)(((zlib::Bytef*)m_FileInfoTable)+m_CurrentTableOffset) = ::HostToLittleEndian32(cinfo.c_len);
			m_CurrentTableOffset += sizeof(uint32_t);
		}
		{
			*(uint32_t*)(((zlib::Bytef*)m_FileInfoTable)+m_CurrentTableOffset) = ::HostToLittleEndian32(cinfo.c_len_a);
			m_CurrentTableOffset += sizeof(uint32_t);
		}
		{
			*(uint32_t*)(((zlib::Bytef*)m_FileInfoTable)+m_CurrentTableOffset) = ::HostToLittleEndian32(cinfo.d_len);
			m_CurrentTableOffset += sizeof(uint32_t);
		}
		{
			*((zlib::Bytef*)m_FileInfoTable+m_CurrentTableOffset) = cinfo.e_flags;
			m_CurrentTableOffset += sizeof(zlib::Bytef);
		}
		{
			*(uint32_t*)(((zlib::Bytef*)m_FileInfoTable)+m_CurrentTableOffset) = ::HostToLittleEndian32(cinfo.g_off - GRF_HEADER_FULL_LEN);
			m_CurrentTableOffset += sizeof(uint32_t);
		}
		++m_EntryCount;
	}


public:
	virtual bool IsSupportedVersion(uint16_t that_version) const
	{
		unsigned int i = 0, uMax = sizeof(SupportedVersions) / sizeof(SupportedVersions[0]);
		while ( i < uMax )
		{
			if ( SupportedVersions[i] == that_version )
			{
				return true;
			}
			++i;
		}
		return false;
	}
	virtual uint16_t GetMinVersion() const
	{
		return SupportedVersions[0];
	}
	virtual uint16_t GetMaxVersion() const
	{
		return SupportedVersions[sizeof(SupportedVersions) / sizeof(SupportedVersions[0]) - 1];
	}
	virtual std::list<uint16_t> GetSupportedVersions() const
	{
		std::list<uint16_t> list_v;
		unsigned int i = 0, uMax = sizeof(SupportedVersions) / sizeof(SupportedVersions[0]);
		while ( i < uMax )
		{
			list_v.push_back( SupportedVersions[i] );
			++i;
		}
		return list_v;
	}

protected:
	IFileIoManager                 *IoManager;
	CHeapPtr<zlib::Bytef>           m_FileInfoTable;
	size_t                          m_CurrentTableOffset;
	int                             m_EntryCount;
	bool                            m_FinalizedImage;
	CALLBACK_FUNCTION_PTR           m_pCallback;
	LPARAM                          m_CallbackParam;
	UTF16_t                         m_TempFileName[MAX_PATH];
	ICancelPoller                  *m_pCancelEmitter;

private:
	/**
		Size of the SupportedVersions[] private.
	**/
	enum { SUPPORTED_VERSION_COUNT = 1 };

	enum { INITIAL_ENTRIES_REGISTRY_SIZE = 30000 };

	static const TCHAR TEMP_FILE_NAME[MAX_PATH];

private:
 	static const uint16_t SupportedVersions[SUPPORTED_VERSION_COUNT];

private:
	CGrfPacker(const CGrfPacker&);
	CGrfPacker &operator=(const CGrfPacker&);
};

template <typename TFileIoManager>
const uint16_t CGrfPacker<TFileIoManager>::SupportedVersions[] = { 0x0200 };
template <typename TFileIoManager>
const TCHAR CGrfPacker<TFileIoManager>::TEMP_FILE_NAME[] = _T("tmpgryff.grf");
template <typename TFileIoManager>
ICancelPoller CGrfPacker<TFileIoManager>::NoCancel;

#endif  // !defined(__CGRFPACKER_H__)
