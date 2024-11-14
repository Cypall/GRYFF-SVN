// $Id$
#ifndef __GEN_TYPES_H__
#define __GEN_TYPES_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


/* Integer types */
#ifdef WIN32
	/* Supposing <windows.h> header correctly defines _INC_WINDOWS */
	#if !defined(_INC_WINDOWS)
	#error <windows.h> is required for <gen_types.h>
	#endif
	#if !defined(uinttypes_def)
		typedef UINT32 uint32_t;
		typedef UINT16 uint16_t;
		typedef UINT8 uint8_t;
		#define uinttypes_def
	#endif
#else /* WIN32 */
	#include <inttypes.h>
#endif /* WIN32 */

#endif  /* __GEN_TYPES_H__ */
