// $Id: ole_impl.cpp 9 2005-08-12 17:33:08Z Rasqual $

// *
// *   gryff, a GRF archive management utility, src/MyTreeCtrl.cpp
// *   Copyright (C) 2003, 2004 Rasqual Twilight
// *
// *   This program is distributed in the hope that it will be useful,
// *   but WITHOUT ANY WARRANTY; without even the implied warranty of
// *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// *

// * ole_impl.cpp
// * OLE interface implementation (implementation)
// *
// *@author $Author: Rasqual $
// *@version $Revision: 9 $
// *

#include  "stdwtl.h"
#pragma hdrstop
#include  "ole_impl.h"

///////////////////////////////////////////////////////////////////////////////
// Classes in this file:
//
// CDropSource implementation
// CEnumFormatEtc implementation
// CDataObject implementation


///////////////////////////////////////////////////////////////////////////////

HRESULT  CDropSource::QueryInterface(REFIID riid, LPVOID *ppvObject)
{
	if ( ppvObject == 0 )
	{
		return E_INVALIDARG;
	}
	if ( IsEqualIID(riid, IID_IUnknown) )
	{
		this->AddRef();
		*ppvObject = reinterpret_cast<IUnknown *>(this);
		return S_OK;
	}
	if ( IsEqualIID(riid, IID_IDropSource) )
	{
		this->AddRef();
		*ppvObject = reinterpret_cast<IDropSource *>(this);
		return S_OK;
	}
	*ppvObject = 0;
	return E_NOINTERFACE;
}

///////////////////////////////////////////////////////////////////////////////

ULONG  CDropSource::AddRef()
{
	return  static_cast<ULONG>(::InterlockedIncrement(&m_RefCount));
}

///////////////////////////////////////////////////////////////////////////////

ULONG  CDropSource::Release()
{
	ULONG ulVal = static_cast<ULONG>(InterlockedDecrement(&m_RefCount));
	if ( ulVal == 0 )
	{
		// Since the object is only meant to implement IDropSource, delete it when not needed anymore
		delete this;
	}
	return ulVal;
}

///////////////////////////////////////////////////////////////////////////////

HRESULT  CDropSource::QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState)
{
    if ( fEscapePressed )
	{
		return DRAGDROP_S_CANCEL;
	}

    if ( !(grfKeyState & (MK_LBUTTON /*| MK_RBUTTON*/)) )
	{
		// If the last effect returned was NONE, then we don't do anything
		// here because the target rejected the drag.
		if ( DROPEFFECT_NONE == m_dwLastEffect )
		{
			return DRAGDROP_S_CANCEL;
		}
		// Actually do the file copying/extraction here to the temp dir if target is shell
		return DRAGDROP_S_DROP;
	}

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

HRESULT  CDropSource::GiveFeedback(DWORD dwEffect)
{
	m_dwLastEffect = dwEffect;
	return DRAGDROP_S_USEDEFAULTCURSORS;
}

///////////////////////////////////////////////////////////////////////////////

CEnumFormatEtc::CEnumFormatEtc(LPFORMATETC pFE, int nItems) : m_RefCount(1), m_iCur(0)
{
	m_pStoredFormatEtc = new FORMATETC[nItems];

	if ( m_pStoredFormatEtc )
    {
		// Make sure pointers are zero
		::ZeroMemory(m_pStoredFormatEtc, sizeof(FORMATETC) * nItems);
		bool FailedAlloc(false);
		for ( int i = 0; i < nItems; ++i )
		{
			if ( FAILED(DeepCopyFormatEtc(&m_pStoredFormatEtc[i], &pFE[i])) )
			{
				FailedAlloc = true;
				break;
			}
		}
		if ( FailedAlloc )
		{
			for ( int i = 0; i < nItems; ++i )
			{
				if ( m_pStoredFormatEtc[i].ptd )  // non-zero pointers are freed
				{
					::CoTaskMemFree(m_pStoredFormatEtc[i].ptd);
				}
			}
			delete[] m_pStoredFormatEtc;
			m_pStoredFormatEtc = 0;
			m_ItemsCount = 0;
		}
		else
		{
			m_ItemsCount = nItems;
		}
    }
	else
    {
		m_ItemsCount = 0;
    }
}

///////////////////////////////////////////////////////////////////////////////

CEnumFormatEtc::~CEnumFormatEtc()
{
    for( ULONG i = 0; i < m_ItemsCount; ++i )
    {
        if ( m_pStoredFormatEtc[i].ptd )
		{
			::CoTaskMemFree(m_pStoredFormatEtc[i].ptd);
		}
    }
	delete[] m_pStoredFormatEtc;
}

///////////////////////////////////////////////////////////////////////////////

HRESULT  CEnumFormatEtc::QueryInterface(REFIID riid, LPVOID *ppvObject)
{
	if ( ppvObject == 0 )
	{
		return E_INVALIDARG;
	}
	if ( IsEqualIID(riid, IID_IUnknown) )
	{
		this->AddRef();
		*ppvObject = reinterpret_cast<IUnknown *>(this);
		return S_OK;
	}
	if ( IsEqualIID(riid, IID_IEnumFORMATETC) )
	{
		this->AddRef();
		*ppvObject = reinterpret_cast<IEnumFORMATETC *>(this);
		return S_OK;
	}
	*ppvObject = 0;
	return E_NOINTERFACE;
}

///////////////////////////////////////////////////////////////////////////////

ULONG  CEnumFormatEtc::AddRef()
{
	return  static_cast<ULONG>(::InterlockedIncrement(&m_RefCount));
}

///////////////////////////////////////////////////////////////////////////////

ULONG  CEnumFormatEtc::Release()
{
	ULONG ulVal = static_cast<ULONG>(InterlockedDecrement(&m_RefCount));
	if ( ulVal == 0 )
	{
		// Since the object is only meant to implement IEnumFORMATETC, delete it when not needed anymore
		delete this;
	}
	return ulVal;
}

///////////////////////////////////////////////////////////////////////////////

STDMETHODIMP CEnumFormatEtc::Next(ULONG celt, LPFORMATETC pFE, ULONG *puFetched)
{
    ULONG cReturn = 0L;
	if ( celt <= 0 || pFE == NULL || m_iCur >= m_ItemsCount )
	{
		return S_FALSE;
	}
	if ( puFetched == NULL && celt != 1 )
	{
        return S_FALSE;
	}
	if ( puFetched != NULL )
	{
		*puFetched = 0;
	}
	while ( m_iCur < m_ItemsCount && celt > 0 )
	{
		DeepCopyFormatEtc(pFE++, &m_pStoredFormatEtc[m_iCur++]);
		++cReturn;
		--celt;
	}
	if ( puFetched != NULL )
	{
		*puFetched = (cReturn - celt);
	}
	return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

STDMETHODIMP  CEnumFormatEtc::Skip(ULONG celt)
{
	if ( (m_iCur + celt) >= m_ItemsCount )
	{
		return S_FALSE;
	}

	m_iCur += celt;
	return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

STDMETHODIMP  CEnumFormatEtc::Reset()
{
	m_iCur = 0;
	return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

STDMETHODIMP  CEnumFormatEtc::Clone(IEnumFORMATETC **ppCloneEnumFormatEtc)
{
CEnumFormatEtc *newEnum;

	if ( NULL == ppCloneEnumFormatEtc )
	{
		return E_INVALIDARG;
	}     
	newEnum = new CEnumFormatEtc(m_pStoredFormatEtc, m_ItemsCount);
	if ( newEnum == NULL )
	{
		return E_OUTOFMEMORY;
	}
	newEnum->AddRef();
	newEnum->m_iCur = m_iCur;
	*ppCloneEnumFormatEtc = newEnum;

	return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

HRESULT  CDataObject::QueryInterface(REFIID riid, LPVOID *ppvObject)
{
	if ( ppvObject == 0 )
	{
		return E_INVALIDARG;
	}
	if ( IsEqualIID(riid, IID_IUnknown) )
	{
		this->AddRef();
		*ppvObject = reinterpret_cast<IUnknown *>(this);
		return S_OK;
	}
	if ( IsEqualIID(riid, IID_IDataObject) )
	{
		this->AddRef();
		*ppvObject = reinterpret_cast<IDataObject *>(this);
		return S_OK;
	}
	*ppvObject = 0;
	return E_NOINTERFACE;
}

///////////////////////////////////////////////////////////////////////////////

ULONG  CDataObject::AddRef()
{
	return  static_cast<ULONG>(::InterlockedIncrement(&m_RefCount));
}

///////////////////////////////////////////////////////////////////////////////

ULONG  CDataObject::Release()
{
	ULONG ulVal = static_cast<ULONG>(InterlockedDecrement(&m_RefCount));
	if ( ulVal == 0 )
	{
		// Since the object is only meant to implement IDataObject, delete it when not needed anymore
		delete this;
	}
	return ulVal;
}

///////////////////////////////////////////////////////////////////////////////

STDMETHODIMP  CDataObject::GetData(LPFORMATETC pFE, LPSTGMEDIUM pSM)
{
	if ( pFE == NULL || pSM == NULL )
	{
		return E_INVALIDARG;
	}
	::ZeroMemory(pSM, sizeof(STGMEDIUM));

	  // Find matching format
	for ( DWORD i = 0; i < m_DataCount; ++i )
	{
		if ( (pFE->tymed & m_pFormatEtc[i].tymed) && 
		     (pFE->dwAspect == m_pFormatEtc[i].dwAspect) && 
		     (pFE->cfFormat == m_pFormatEtc[i].cfFormat) &&
		     (pFE->lindex == m_pFormatEtc[i].lindex) )
		{
			// Copy is left-to-right...
		HRESULT  hr = ::CopyStgMedium(&m_pStgMedium[i], pSM);
			if ( SUCCEEDED(hr) && pSM->tymed == TYMED_HGLOBAL )
			{
				// this tells the caller not to free the global memory
				this->QueryInterface(IID_IUnknown, (LPVOID*)&pSM->pUnkForRelease);
			}
			return hr;
		}
	}

	return DV_E_FORMATETC;
}

///////////////////////////////////////////////////////////////////////////////

STDMETHODIMP  CDataObject::GetDataHere(LPFORMATETC, LPSTGMEDIUM)
{
	// This implementation only supports HGLOBAL data
	return DATA_E_FORMATETC;
}

///////////////////////////////////////////////////////////////////////////////

STDMETHODIMP CDataObject::QueryGetData(LPFORMATETC pFE)
{ 
	if ( pFE == NULL )
	{
		return DV_E_FORMATETC;
	}
	if ( !(DVASPECT_CONTENT & pFE->dwAspect) )
	{
		return DV_E_DVASPECT;
	}
	// now check for an appropriate TYMED (type of medium).
	HRESULT  hr(DV_E_TYMED);
	for ( DWORD i = 0; i < m_DataCount; ++i )
	{
		if ( (pFE->tymed & m_pFormatEtc[i].tymed)
		  && pFE->cfFormat == m_pFormatEtc[i].cfFormat
		  && pFE->lindex == m_pFormatEtc[i].lindex )
		{
			if ( pFE->dwAspect == m_pFormatEtc[i].dwAspect )
			{
				return S_OK;
			}
			else
			{
				hr = DV_E_DVASPECT;  // unless found another aspect w/ same TYMED & CF, spec. error
			}
		}
	}
	return hr;
}

///////////////////////////////////////////////////////////////////////////////

STDMETHODIMP  CDataObject::GetCanonicalFormatEtc(LPFORMATETC lpIn, LPFORMATETC lpOut)
{
	if ( lpOut == NULL )
	{
		return E_INVALIDARG; 
	}
	::CopyMemory(lpOut, lpIn, sizeof(FORMATETC));
	lpOut->ptd = NULL;
	return DATA_S_SAMEFORMATETC;
}

///////////////////////////////////////////////////////////////////////////////

STDMETHODIMP  CDataObject::SetData(LPFORMATETC pFE, LPSTGMEDIUM pSM, BOOL fRelease)
{
LPFORMATETC pfeNew;
LPSTGMEDIUM psmNew;

	++m_DataCount;
	// FIXME: TODO: (x.x) Uguuu~ use vector-like objects
	pfeNew = new FORMATETC[m_DataCount];
	psmNew = new STGMEDIUM[m_DataCount];

	if ( !pfeNew || !psmNew )
	{
		--m_DataCount;
		delete [] pfeNew;
		delete [] psmNew;
		return E_OUTOFMEMORY;
	}
	::ZeroMemory(pfeNew, sizeof(FORMATETC) * m_DataCount);
	::ZeroMemory(psmNew, sizeof(STGMEDIUM) * m_DataCount);

	// copy the existing data
	if ( m_pFormatEtc )
	{
		::CopyMemory(pfeNew, m_pFormatEtc, sizeof(FORMATETC) * (m_DataCount - 1));
	}
	if ( m_pStgMedium )
	{
		::CopyMemory(psmNew, m_pStgMedium, sizeof(STGMEDIUM) * (m_DataCount - 1));
	}

	//add the new data
	pfeNew[m_DataCount - 1] = *pFE;
	if ( fRelease )
	{
		psmNew[m_DataCount - 1] = *pSM;
	}
	else
	{
		// Left-to-right
		::CopyStgMedium(pSM, &psmNew[m_DataCount - 1]);
	}

	// swap and delete old arrays
LPFORMATETC pfeTemp(m_pFormatEtc);
	m_pFormatEtc = pfeNew;
LPSTGMEDIUM psmTemp(m_pStgMedium);
	m_pStgMedium = psmNew;
	delete [] pfeTemp;
	delete [] psmTemp;

	return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

STDMETHODIMP  CDataObject::EnumFormatEtc(DWORD Direction, LPENUMFORMATETC *ppEnum)
{
	if ( ppEnum == NULL )
	{
		return E_INVALIDARG; 
	}
    *ppEnum = NULL;

	if ( Direction == DATADIR_GET )
	{
		*ppEnum =  new CEnumFormatEtc(m_pFormatEtc, m_DataCount);
		if ( *ppEnum )
		{
			return S_OK;
		}
		return E_OUTOFMEMORY;
	}   
	return OLE_S_USEREG;
}

///////////////////////////////////////////////////////////////////////////////

STDMETHODIMP  CDataObject::DAdvise(LPFORMATETC, DWORD flags, LPADVISESINK, DWORD* pConnection)
{
	return OLE_E_ADVISENOTSUPPORTED;
}

///////////////////////////////////////////////////////////////////////////////

STDMETHODIMP  CDataObject::DUnadvise(DWORD Connection)
{
	return OLE_E_ADVISENOTSUPPORTED;
}

///////////////////////////////////////////////////////////////////////////////

STDMETHODIMP  CDataObject::EnumDAdvise(LPENUMSTATDATA *)
{
	return OLE_E_ADVISENOTSUPPORTED;
}

///////////////////////////////////////////////////////////////////////////////

