// $Id: ole_impl.h 9 2005-08-12 17:33:08Z Rasqual $

// *
// *   gryff, a GRF archive management utility, src/MyTreeCtrl.cpp
// *   Copyright (C) 2003, 2004 Rasqual Twilight
// *
// *   This program is distributed in the hope that it will be useful,
// *   but WITHOUT ANY WARRANTY; without even the implied warranty of
// *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// *

// * ole_impl.h
// * OLE interface implementation (class definitions)
// *
// *@author $Author: Rasqual $
// *@version $Revision: 9 $
// *

#ifndef __OLE_IMPL_H__
#define __OLE_IMPL_H__

#pragma once

///////////////////////////////////////////////////////////////////////////////
// Classes in this file:
//
// CDropSource
//   An implementation of IDropSource for dragging entries from documents
//
// CEnumFormatEtc
//   An implementation of IEnumFORMATETC for clipboard formats description
//
// CDataObject
//   An implementation of IDataObject for storing data 
//   (current impl. only allows HGLOBAL data)
//
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////

class CDropSource : public IDropSource
{
	///////////////////////// - CTOR / DTOR - /////////////////////////////
public:
	/**
		Performs initialization.
	**/
	CDropSource() : m_RefCount(1), m_dwLastEffect(DROPEFFECT_NONE) { }

	/**
		Performs cleanup.
	**/
	~CDropSource() { }

	///////////////////////// - data members - /////////////////////////////
protected:
	LONG                 m_RefCount;
	DWORD                m_dwLastEffect;

public:
	//----- Implement IDropSource for drag and drop support -----//
	/**
		IUnknown QueryInterface()
		Supported interfaces are IUnknown + IDropSource
	**/
	STDMETHODIMP  QueryInterface(REFIID riid, LPVOID *ppv);

	/**
		IUnknown AddRef()
		Increase reference count
	**/
    STDMETHODIMP_(ULONG)  AddRef();

	/**
		IUnknown Release()
		Decrease reference count
	**/
	STDMETHODIMP_(ULONG)  Release();

	/**
		IDropSource QueryContinueDrag()
	**/
	STDMETHODIMP  QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState);

	/**
		IDropSource GiveFeedback()
	**/
	STDMETHODIMP  GiveFeedback(DWORD dwEffect);
};

///////////////////////////////////////////////////////////////////////////////

class CEnumFormatEtc : public IEnumFORMATETC
{
	//////////////////////// - static methods - /////////////////////////////
	// DeepCopyFormatEtc()
	//	By J Brown 2004 
	//	www.catch22.net
	//	ENUMFORMAT.CPP from http://www.catch22.net/tuts/dragdrop4.asp
	//
	//	Helper function to perform a "deep" copy of a FORMATETC
	//
	static HRESULT  DeepCopyFormatEtc(FORMATETC *dest, FORMATETC *source)
	{
		// copy the source FORMATETC into dest
		*dest = *source;
		if ( source->ptd )
		{
			// allocate memory for the DVTARGETDEVICE if necessary
			dest->ptd = (DVTARGETDEVICE*)::CoTaskMemAlloc(sizeof(DVTARGETDEVICE));
			if ( !dest->ptd )
			{
				return  E_OUTOFMEMORY;
			}
			// copy the contents of the source DVTARGETDEVICE into dest->ptd
			*(dest->ptd) = *(source->ptd);
		}
		return S_OK;
	}

	///////////////////////// - CTOR / DTOR - /////////////////////////////
public:
	/**
		Performs initialization.
	**/
	CEnumFormatEtc(LPFORMATETC pFE, int numberItems);

	/**
		Performs cleanup.
	**/
	~CEnumFormatEtc();

	///////////////////////// - data members - /////////////////////////////
protected:
	LONG                 m_RefCount;
	LPFORMATETC          m_pStoredFormatEtc;
	DWORD                m_ItemsCount;
	ULONG                m_iCur;

public:
	//----- Implement CDataObject for drag and drop  and clipboard support -----//
	/**
		IUnknown QueryInterface()
		Supported interfaces are IUnknown + IDataObject
	**/
	STDMETHODIMP  QueryInterface(REFIID riid, LPVOID *ppv);

	/**
		IUnknown AddRef()
		Increase reference count
	**/
    STDMETHODIMP_(ULONG)  AddRef();

	/**
		IUnknown Release()
		Decrease reference count
	**/
	STDMETHODIMP_(ULONG)  Release();

	/**
		IEnumFORMATETC Next()
	**/
	STDMETHODIMP  Next(ULONG, LPFORMATETC, ULONG *);

	/**
		IEnumFORMATETC Skip()
	**/
	STDMETHODIMP  Skip(ULONG);

	/**
		IEnumFORMATETC Next()
	**/
	STDMETHODIMP  Reset();

	/**
		IEnumFORMATETC Clone()
	**/
	STDMETHODIMP  Clone(IEnumFORMATETC **);
};

///////////////////////////////////////////////////////////////////////////////


class CDataObject: public IDataObject
{
	///////////////////////// - CTOR / DTOR - /////////////////////////////
public:
	/**
		Performs initialization.
	**/
	CDataObject() : m_RefCount(1), m_pFormatEtc(0), m_pStgMedium(0), m_DataCount(0) { }

	/**
		Performs cleanup.
	**/
	~CDataObject() { }

	///////////////////////// - data members - /////////////////////////////
protected:
	LONG                 m_RefCount;
	LPFORMATETC          m_pFormatEtc;
	LPSTGMEDIUM          m_pStgMedium;
	DWORD                m_DataCount;  // Format count

public:
	//----- Implement CDataObject for drag and drop  and clipboard support -----//
	/**
		IUnknown QueryInterface()
		Supported interfaces are IUnknown + IDataObject
	**/
	STDMETHODIMP  QueryInterface(REFIID riid, LPVOID *ppv);

	/**
		IUnknown AddRef()
		Increase reference count
	**/
    STDMETHODIMP_(ULONG)  AddRef();

	/**
		IUnknown Release()
		Decrease reference count
	**/
	STDMETHODIMP_(ULONG)  Release();

	/**
		IDataObject GetData()
		If specified format(s) are supported, stg medium is set and S_OK is returned;
		Depending on the value of the pUnkForRelease storage medium member,
		it may be the caller's responsibility to Release() it.
	**/
	STDMETHODIMP  GetData(/*[in]*/LPFORMATETC, /*[out]*/LPSTGMEDIUM);

	/**
		IDataObject GetDataHere()
		The LPSTGMEDIUM structure is allocated by the caller, pUnkForRelease is always NULL.
		The medium is also Release()d by the caller.
	**/
	STDMETHODIMP  GetDataHere(LPFORMATETC, /*[in,out]*/LPSTGMEDIUM);

	/**
		IDataObject QueryGetData()
	**/
	STDMETHODIMP  QueryGetData(LPFORMATETC);

	/**
		IDataObject GetCanonicalFormatEtc()
	**/
	STDMETHODIMP  GetCanonicalFormatEtc(LPFORMATETC pFE, LPFORMATETC pCanonFE);

	/**
		IDataObject SetData()
	**/
	STDMETHODIMP  SetData(LPFORMATETC, LPSTGMEDIUM, BOOL release);

	/**
		IDataObject EnumFormatEtc()
		direction is either DATADIR_GET or DATADIR_SET.
	**/
	STDMETHODIMP  EnumFormatEtc(DWORD direction, LPENUMFORMATETC *);

	/**
		IDataObject DAdvise()
	**/
	STDMETHODIMP  DAdvise(LPFORMATETC, DWORD flags, LPADVISESINK, DWORD* pConnection);

	/**
		IDataObject DUnadvise()
	**/
    STDMETHODIMP  DUnadvise(DWORD Connection);

	/**
		IDataObject EnumDAdvise()
	**/
	STDMETHODIMP  EnumDAdvise(LPENUMSTATDATA *);
};

///////////////////////////////////////////////////////////////////////////////


#endif //__OLE_IMPL_H__