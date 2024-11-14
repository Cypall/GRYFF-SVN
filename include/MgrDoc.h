// $Id$
#ifndef __MGRDOC_H__
#define __MGRDOC_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __ATLCOLL_H__
   #error MgrDoc.h requires <atlcoll.h> to be included first
#endif

class CDocTemplateBase;

///////////////////////////////////////////////////////////////////////////////
// Classes in this file:
//
// CDocManager
//   Class which manages document templates. It is supposed to be inherited
//   from by the main frame (or the document frame if app is SDI.)
//


template <typename TMainWnd>
class CDocManager
{
public:
	//
	// AddDocTemplate
	// - Registers a document template
	//
	template <class TDocTemplate>
		int AddDocTemplate( TDocTemplate* pDocTemplate )
	{
		_ASSERTE( pDocTemplate != NULL );

		pDocTemplate->m_hWndClient =
			static_cast<TMainWnd*>(this)->m_hWndClient;
		CDocTemplateBase* pDocBase =
			static_cast<CDocTemplateBase*>( pDocTemplate );
		m_aTemplates.Add( pDocBase );

		return m_aTemplates.GetSize() - 1;
	}

	//
	// GetNumTemplates
	// - Returns the number of registered document templates
	//
	int GetNumTemplates() const
	{
		return m_aTemplates.GetSize();
	}

	//
	// GetDocTemplates
	// - Gets a document template by index
	//
	CDocTemplateBase* GetDocTemplate( const int pos )
	{
		_ASSERTE( pos < m_aTemplates.GetSize() );
		return m_aTemplates[ pos ];
	}

protected:
	CSimpleArray<CDocTemplateBase*> m_aTemplates;
};

#endif // __MGRDOC_H__
