// $Id: ThreadIntercom.h 9 2005-08-12 17:33:08Z Rasqual $

// *
// *   gryff, a GRF archive management utility, src/ThreadIntercom.h
// *   Copyright (C) 2003, 2004 Rasqual Twilight
// *
// *   This program is distributed in the hope that it will be useful,
// *   but WITHOUT ANY WARRANTY; without even the implied warranty of
// *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// *

// * ThreadIntercom.h
// * Thread communications
// *
// *@author $Author: Rasqual $
// *@version $Revision: 9 $
// *

#if !defined(__THREADINTERCOM_H__)
#define __THREADINTERCOM_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __ATLFRAME_H__
   #error DlgProgress.h requires <atlframe.h> to be included first
#endif

class CGrfFrame;
class CProgressDialog;
template <typename T> class SaveCommand;

///////////////////////////////////////////////////////////////////////////////
// Classes in this file:
//
// GryffJobException
//   Exceptions thrown by the job manager.
//   Can be derived from and override ToString() for more explicit information
// JOBDATA
//   Struct containing data shared between threads
// CDocumentSaverData
//   Information needed by the thread function when saving a document
// ClassifiedAds
//   Job manager that launches tasks to perform via its GetJobDone static method


///////////////////////////////////////////////////////////////////////////////
// Exceptions logged by available jobs

// Derive from GryffJobException to implement more specific debugging
class GryffJobException
{
	CString  m_Message;
public:
	GryffJobException(const CString &message) : m_Message(message)
	{}
	virtual ~GryffJobException()
	{}
	virtual CString  ToString() const
	{
		return m_Message;
	}
};

///////////////////////////////////////////////////////////////////////////////
// Utility macros for  ClassifiedAds class

#define BEGIN_JOB_HANDLER_MAP() \
public: \
	static bool  ProcessJobData(struct JOBDATA& jobData, LRESULT& lResult) \
	{ \
		static struct \
		{ \
			int                 JobId; \
			LRESULT             (*JobProc)(struct JOBDATA&); \
		} JobProcessing[] = \
		{


#define	JOB_HANDLER(jobid, static_proc) \
			{ jobid, static_proc },

#define END_JOB_HANDLER_MAP() \
		}; \
		for ( int i = 0; i < sizeof(JobProcessing) / sizeof(JobProcessing[0]); ++i ) \
		{ \
			if ( JobProcessing[i].JobId == jobData.JobId ) \
			{ \
				lResult = JobProcessing[i].JobProc(jobData); \
				return true; \
			} \
		} \
		return false; \
	}

///////////////////////////////////////////////////////////////////////////////
// Job identifiers used in JOBDATA

#define JOBID_FIRST                       0x300

#define JOBID_IMPORTDOC                   JOBID_FIRST
#define JOBID_IMPORT_FS                   JOBID_FIRST+1
#define JOBID_SAVEDOC                     JOBID_FIRST+2
#define JOBID_EXTRACT                     JOBID_FIRST+3

#define JOBID_LAST JOBID_EXTRACT

///////////////////////////////////////////////////////////////////////////////

struct JOBDATA
{
	HWND                hWndMessagePumper;
	int                 JobId;
	LPARAM              JobData;
	GryffJobException   *pException;
	~JOBDATA()
	{
		delete pException;
	}
};

///////////////////////////////////////////////////////////////////////////////

// The following data is used to exchange data between threads
struct CDocumentSaverData
{
	/**
		Pointer to the originating frame
	**/
	CGrfFrame                                      *pThis;
	CProgressDialog                                *pDlg;
	SaveCommand<CGrfFrame>                         *pCmd;

	/**
	   entries to be removed afterwards if successful
	**/
	const CAtlList< std::pair<uint8_t, CString> >  *pInvalidEntries;
	/**
	   unresolved entries when preparing the document (either missing GRF/file or entry not found)
	**/
	CAtlList< std::pair<uint8_t, CString> >        *pErroneousEntries;
	/**
		Path to the temporary output file
	**/
	TCHAR                                          tmppath[MAX_PATH];
	/**
		Final target name
	**/
	TCHAR                                          path[MAX_PATH];
	/**
		Factory for grfs referenced in the being-saved document
	**/
	CGrfCache                                      *pCache;
	/**
		Array of auto-ptr'd entries (deleted when the struct is deleted)
		Number of entries in saved document
	**/
	CAutoPtrArray<IGrfEntry>                       pEntries;

	IGrfPacker                                     *pPak;
	IFileIoManager                                 *pIoManager;

public:
	/**
		Starts thread saving the document
		Returns IDOK if document was saved
		IDCANCEL if user cancelled the operation
		IDABORT if an error occured
	**/
	static LRESULT  BeginJob(struct JOBDATA&);
};



///////////////////////////////////////////////////////////////////////////////
// Job manager

class ClassifiedAds
{
BEGIN_JOB_HANDLER_MAP()
	JOB_HANDLER(JOBID_SAVEDOC, &CDocumentSaverData::BeginJob)
END_JOB_HANDLER_MAP()

public:
	// Launches a job task specified by its jobData parameter
	// Returns 0L if job is not handled
	static LRESULT  GetJobDone(struct JOBDATA& jobData)
	{
	LRESULT  lResult;
		if ( ProcessJobData(jobData, lResult) )
		{
			return lResult;
		}
		return 0L;  // Job not handled
	}
};

///////////////////////////////////////////////////////////////////////////////

LRESULT  CDocumentSaverData::BeginJob(struct JOBDATA& jobData)
{
CDocumentSaverData *pData = reinterpret_cast<CDocumentSaverData *>(jobData.JobData);
	class HandleCloser
	{
		HANDLE m_h;
	public:
		HandleCloser(HANDLE h) : m_h(h) {}
		~HandleCloser()
		{
			if ( m_h )	{ ::CloseHandle( m_h ); }
		}
		operator HANDLE()	{ return m_h; }
	};
	// TODO: begin job in an asynchronous context
	return 0;
}

///////////////////////////////////////////////////////////////////////////////


#endif   // !defined(__THREADINTERCOM_H__)
