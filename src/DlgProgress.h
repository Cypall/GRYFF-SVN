// $Id: DlgProgress.h 9 2005-08-12 17:33:08Z Rasqual $

// *
// *   gryff, a GRF archive management utility, src/DlgProgress.h
// *   Copyright (C) 2003, 2004 Rasqual Twilight
// *
// *   This program is distributed in the hope that it will be useful,
// *   but WITHOUT ANY WARRANTY; without even the implied warranty of
// *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// *

// * DlgProgress.h
// * Wrapper for the IDD_PROGRESS dialog
// *
// *@author $Author: Rasqual $
// *@version $Revision: 9 $
// *

#if !defined(__DLGPROGRESS_H__)
#define __DLGPROGRESS_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __ATLFRAME_H__
   #error DlgProgress.h requires <atlframe.h> to be included first
#endif

///////////////////////////////////////////////////////////////////////////////
// Classes in this file:
//
// CProgressDialog
//   Class instantiating a dialog with a progress bar.
//   used when performing along operation.
//   Dialog title, description, caption and progress control provide the feedback.
//   A cancel button allows to abort the operation.
//
// Command
//   Abstract base class for commands
//
// SaveCommand
//   Command invoked when saving a grf document

///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////

class CProgressDialog : public CDialogImpl< CProgressDialog >
{
public:
	enum { IDD = IDD_PROGRESS, ID_TIMER_ELAPSED = 222 };

	typedef void (*InitializeCallback) ( LPARAM lParam );

public:
	static void  initializeCallbackNull(LPARAM)
	{}

public:
	CStatic        m_ctlDesc;      // Description of the current task
	CStatic        m_ctlCaption;   // Caption for current task (e.g. filename)
	CProgressBarCtrl  m_ctlProgress;  // Gauge
	unsigned int   m_CaptionWidth;
protected:
	CString        m_strBaseTitle;
	UINT_PTR       m_TimerElapsed;
	int            m_Elapsed;
	HANDLE         m_hThread;  // used only as an identifier, handle is closed indeed
	bool           m_Cancelling;
private:
	InitializeCallback m_InitializeCallback;
	LPARAM         m_lParam;

	///////////////////////////////////////////////////////////////////////////////

public:
	CProgressDialog(InitializeCallback initializeCallback = 0, LPARAM lParam = 0) : CDialogImpl<CProgressDialog>(), m_TimerElapsed(ID_TIMER_ELAPSED), m_Elapsed(0), m_hThread(0), m_Cancelling(false), m_InitializeCallback(initializeCallback ? initializeCallback : initializeCallbackNull), m_lParam(lParam)
	{}

	~CProgressDialog()
	{
		if ( m_TimerElapsed != 0 )
		{
			::KillTimer(m_hWnd, m_TimerElapsed);
			m_TimerElapsed = 0;
		}
	}
protected:
	LRESULT OnInitDialog(UINT, WPARAM, LPARAM lParam, BOOL&)
	{
		m_ctlDesc = this->GetDlgItem(IDC_PROGRESS_STATIC);
		m_ctlCaption = this->GetDlgItem(IDC_PROGRESS_CAPTION);
		m_ctlProgress = this->GetDlgItem(IDC_PROGRESS_CONTROL);
	CRect rc;
		m_ctlCaption.GetClientRect(&rc);
	CSize sz;
		m_CaptionWidth = rc.Width();

		m_TimerElapsed = ::SetTimer(m_hWnd, m_TimerElapsed, 1000 /*milliseconds*/, (TIMERPROC)0);
		m_InitializeCallback(m_lParam);

		return TRUE;
	}

	///////////////////////////////////////////////////////////////////////////////

public:
	void  SetBaseTitle(LPCTSTR str)
	{
		m_strBaseTitle = str;
		this->SetWindowText(m_strBaseTitle);
	}

	void  SetWindowCaption(LPCTSTR str)
	{
		this->SetWindowText(m_strBaseTitle + _T(" - ") + str);
	}

	void  SetRange(int minimum, int maximum)
	{
		if ( minimum == -1 && maximum == -1 )
		{
			this->EndDialog(IDCANCEL);
		}
		else
		{
			m_ctlProgress.SetRange(minimum, maximum);
		}
	}

	void  SetPos(int position)
	{
		m_ctlProgress.SetPos(position);
	}

	void  SetStatus1(LPCTSTR str)
	{
		m_ctlDesc.SetWindowText(str);
	}

	void  SetStatus2(LPCTSTR str)
	{
		m_ctlCaption.SetWindowText(str);
	}

	bool  IsCancelling()
	{
		return m_Cancelling;
	}

	///////////////////////////////////////////////////////////////////////////////

protected:
	void  OnTimer(UINT TimerId, TIMERPROC)
	{
		if ( TimerId == m_TimerElapsed )
		{
			this->m_Elapsed += 1;
		}
		else
		{
			SetMsgHandled(FALSE);
		}
	}

	///////////////////////////////////////////////////////////////////////////////

	LRESULT OnCancel(WORD, WORD wID, HWND, BOOL&)
	{
	CMenuHandle hSysMenu(this->GetSystemMenu(FALSE));
		hSysMenu.EnableMenuItem(SC_CLOSE, MF_BYCOMMAND | MF_GRAYED);
		::EnableWindow(this->GetDlgItem(IDCANCEL), FALSE);
		m_Cancelling = true;
		this->SetWindowCaption(_T("Aborting, please wait..."));
		return 0;
	}


	BEGIN_MSG_MAP(CProgressDialog)
	  MSG_WM_TIMER(OnTimer)
	  MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	  COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	END_MSG_MAP()
};

///////////////////////////////////////////////////////////////////////////////

template <class TYPE_FRAME>
class Command
{
protected:
	TYPE_FRAME     *m_pFrame;
	LPARAM         m_lParam;
protected:
	/**
		Abstract constructor to prevent instantiation (use inheritance)
	**/
	Command(TYPE_FRAME *pFrame, LPARAM lParam) : m_pFrame(pFrame), m_lParam(lParam)
	{
	}
};

///////////////////////////////////////////////////////////////////////////////

template <class TYPE_FRAME>
class SaveCommand : Command<TYPE_FRAME>
{
	friend TYPE_FRAME;
public:
	/**
		Delegates for the pending operation
	**/
	typedef void (*InitializeCallback) ( TYPE_FRAME *pFrame, LPARAM lParam, SaveCommand<TYPE_FRAME> *pThisCommand );
	typedef void (*FinishCallback) ( TYPE_FRAME *pFrame, LPARAM lParam, SaveCommand<TYPE_FRAME> *pThisCommand );

	/**
		Delegates for the progress dialog
	**/
	/// <summary>
	/// Delegate for a method which allows the progress range to be reset
	/// </summary>
	typedef void (*InitializeProgressCallback) ( TYPE_FRAME *pThis, int minimum, int maximum );
	/// <summary>
	/// Delegate for a method which allows the progress to be updated
	/// </summary>
	typedef void (*ProgressCallback) ( TYPE_FRAME *pThis, int progress );
	/// <summary>
	/// Delegate for a method which allows the status text to be updated
	/// </summary>
	typedef void (*StatusCallback) ( TYPE_FRAME *pThis, const CString &statusText );
	/// <summary>
	/// Delegate for a worker method which provides additional callbacks
	/// </summary>
	struct WorkInvoker
	{
		InitializeCallback initializeCallback;
		InitializeProgressCallback initializeProgressCallback;
		ProgressCallback progressCallback;
		StatusCallback status1Callback;
		StatusCallback status2Callback;
		FinishCallback finishCallback;
	};

public:
	static void  initializeCallbackNull(TYPE_FRAME *pFrame, LPARAM lParam, SaveCommand<TYPE_FRAME> *pThisCommand)
	{}
	static void  finishCallbackNull(TYPE_FRAME *pFrame, LPARAM lParam, SaveCommand<TYPE_FRAME> *pThisCommand)
	{}
	static void  initializeProgressCallbackNull(TYPE_FRAME *pFrame, int minimum, int maximum)
	{}
	static void  progressCallbackNull(TYPE_FRAME *pFrame, int progress)
	{}
	static void  status1CallbackNull(TYPE_FRAME *pFrame, const CString &statusText)
	{}
	static void  status2CallbackNull(TYPE_FRAME *pFrame, const CString &statusText)
	{}

	template <class TYPE_FRAME>
	class EndInvoker
	{
		SaveCommand<TYPE_FRAME> &this_ref;
		LPARAM  m_lParam;
	public:
		EndInvoker(SaveCommand<TYPE_FRAME> &ref, LPARAM lParam) : this_ref(ref), m_lParam(lParam)
		{}
		~EndInvoker()
		{
			this_ref.OnEndInvoke(m_lParam);
		}
	};

	class SaveCommandCancelChecker : public ICancelPoller
	{
	public:
		SaveCommandCancelChecker(SaveCommand &command) : parentCommand(command)
		{}
	public:
		virtual void operator()()
		{
			if ( parentCommand.IsCancelling() )
			{
				throw GPCancelOperationException(std::string());
			}
		}
	protected:
		SaveCommand &parentCommand;
	};

private:
	WorkInvoker    m_WorkInvoker;
	volatile bool  m_Cancelling;
	SaveCommandCancelChecker m_CancelChecker;
public:
	SaveCommand(TYPE_FRAME *pFrame, LPARAM lParam, const WorkInvoker &workInvoker) : Command<TYPE_FRAME>(pFrame, lParam), m_WorkInvoker(workInvoker), m_Cancelling(false), m_CancelChecker(*this)
	{
		if ( !m_WorkInvoker.initializeCallback )
		{
			m_WorkInvoker.initializeCallback = initializeCallbackNull;
		}
		if ( !m_WorkInvoker.finishCallback )
		{
			m_WorkInvoker.finishCallback = finishCallbackNull;
		}
		if ( !m_WorkInvoker.initializeProgressCallback )
		{
			m_WorkInvoker.initializeProgressCallback = initializeProgressCallbackNull;
		}
		if ( !m_WorkInvoker.progressCallback )
		{
			m_WorkInvoker.progressCallback = progressCallbackNull;
		}
		if ( !m_WorkInvoker.status1Callback )
		{
			m_WorkInvoker.status1Callback = status1CallbackNull;
		}
		if ( !m_WorkInvoker.status2Callback )
		{
			m_WorkInvoker.status2Callback = status2CallbackNull;
		}
	}

	/**
		Call backs the initialization proc and starts a worker thread
	**/
	HANDLE BeginInvoke()
	{
		m_WorkInvoker.initializeCallback(m_pFrame, m_lParam, this);
		return win::beginthreadex(this, &SaveCommand<TYPE_FRAME>::OnBeginInvoke, m_lParam);
	}

	/**
		
	**/
	DWORD OnBeginInvoke(LPARAM lParam)
	{
		// Make sure OnEndInvoke is called when task is complete
	EndInvoker<TYPE_FRAME> endInvoker(*this, lParam);
	CDocumentSaverData * pSaveData = reinterpret_cast<CDocumentSaverData *>(lParam);
	SaveCommand<TYPE_FRAME> * pCommand = pSaveData->pCmd;
	const int maxProgress = static_cast<int>(pSaveData->pEntries.GetCount());
		pCommand->m_WorkInvoker.initializeProgressCallback(m_pFrame, 0, maxProgress);
		pSaveData->pPak->SetCancelStatePoller(&m_CancelChecker);
		try
		{
			for ( int i = 0; i < maxProgress; ++i )
			{
				m_WorkInvoker.progressCallback(m_pFrame, i);
			IGrfEntry *pEntry = pSaveData->pEntries[i];
			WCHAR EntryName[CGrfFileEntry::GRF_MAX_PATH];
				::MultiByteToWideChar(0x3B5, 0, pEntry->GetEntryName(), -1, EntryName, CGrfFileEntry::GRF_MAX_PATH);
				if ( ::PathCompactPathW(CClientDC(pSaveData->pDlg->m_hWnd), EntryName, pSaveData->pDlg->m_CaptionWidth) )
				{
					m_WorkInvoker.status2Callback(m_pFrame, EntryName);
				}
				pSaveData->pPak->AddEntry(pSaveData->pEntries[i]);
				if ( m_Cancelling )
				{
					throw GPCancelOperationException(std::string());
				}
			}
		}
		catch (const GPCancelOperationException&)
		{
			m_WorkInvoker.initializeProgressCallback(m_pFrame, -1, -1);
			::DeleteFile(pSaveData->tmppath);
			return IDCANCEL;
		}
		m_WorkInvoker.progressCallback(m_pFrame, maxProgress);
		m_WorkInvoker.status1Callback(m_pFrame, _T("Finalizing archive"));
		m_WorkInvoker.status2Callback(m_pFrame, pSaveData->path);
		return pSaveData->pPak->PackFile(reinterpret_cast<const UTF16_t*>(pSaveData->path));
	}

	void  Cancel()
	{
		m_Cancelling = true;
	}

	bool IsCancelling()
	{
	CDocumentSaverData * pSaveData = reinterpret_cast<CDocumentSaverData *>(m_lParam);
		if ( pSaveData->pDlg->IsCancelling() )
		{
			this->Cancel();
		}
		return m_Cancelling;
	}

	void OnEndInvoke(LPARAM)
	{
		m_WorkInvoker.finishCallback(m_pFrame, m_lParam, this);
	}
};

#endif   // !defined(__DLGINVALIDENTRIES_H__)
