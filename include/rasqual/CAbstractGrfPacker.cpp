// $Id: CAbstractGrfPacker.cpp 9 2005-08-12 17:33:08Z Rasqual $
#ifdef WIN32
#include  <windows.h>
#endif /* WIN32 */

#include  <gen_types.h>
#include  "CAbstractGrfPacker.h"


/**
	Implements IGrfPacker::PackFile()
	Call CAbstractGrfPacker::PackFile() in subclasses to detect
	GPNullPointerException and GPInvalidSequenceException.
**/
int CAbstractGrfPacker::PackFile(const UTF16_t utf16le_encoded_filepath[])
  throw ( GPNullPointerException, GPInvalidSequenceException, GPIoException, GPUnlogicalException, GPUnsupportedOperationException )
{
	if ( utf16le_encoded_filepath == 0 )
	{
		throw GPNullPointerException("PackFile: GPNullPointerException");
	}

	  // Unicode FAQ - UTF and BOM
	  // Q: Are there any paired surrogates that are invalid?
	  // http://www.unicode.org/faq/utf_bom.html#16
	if ( !UTF16::IsValidString(utf16le_encoded_filepath) )
	{
		throw GPInvalidSequenceException("PackFile: GPInvalidSequenceException");
	}
	return 0;
}
