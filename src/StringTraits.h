// $Id: StringTraits.h 9 2005-08-12 17:33:08Z Rasqual $

// *
// *   gryff, a GRF archive management utility, src/StringTraits.h
// *   Copyright (C) 2003-2005 Rasqual Twilight
// *
// *   This program is distributed in the hope that it will be useful,
// *   but WITHOUT ANY WARRANTY; without even the implied warranty of
// *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// *

// * StringTraits.h
// * Shell function helpers
// *
// *@author $Author: Rasqual $
// *@version $Revision: 9 $
// *

#ifndef __STRINGTRAITS_H__
#define __STRINGTRAITS_H__

#pragma once


///////////////////////////////////////////////////////////////////////////////
// Classes in this file:
//
// CStringElementTraitsGrfName
//   A templated traits class to take into account equivalent names in Grf files
//   (Comparable to case-insensitive traits, see CStringElementTraitsI<T, CharTraits>
// CompareHelper
//  ::CompareNaturalCStringsClass
//   Class containing natural string compare functor
///////////////////////////////////////////////////////////////////////////////

#if 0
template< typename T, class CharTraits = ATL::CDefaultCharTraits<T::XCHAR> >
class CStringElementTraitsGrfName :
	public CElementTraitsBase< T >
{
public:
	typedef typename T::PCXSTR INARGTYPE;
	typedef T& OUTARGTYPE;

	static ULONG Hash( INARGTYPE str )
	{
	uint32_t  tmp = 0x1505;
	const T::XCHAR* pch = str;
		ATLENSURE( pch != NULL );
		for (size_t i = strlen(name); i > 0; --i)
		{
			tmp += (tmp << 5);
			tmp += CharTraits::CharToUpper(*pch++);
		}
		return ULONG(tmp);
	}

	static bool CompareElements( INARGTYPE str1, INARGTYPE str2 ) throw()
	{
		return( T::StrTraits::StringCompareIgnore( str1, str2 ) == 0 );
	}

	static int CompareElementsOrdered( INARGTYPE str1, INARGTYPE str2 ) throw()
	{
		return( T::StrTraits::StringCompareIgnore( str1, str2 ) );
	}
};
#else
template< typename T, class CharTraits = ATL::CDefaultCharTraits<T::XCHAR> >
class CStringElementTraitsGrfName : public CStringElementTraitsI<T, CharTraits>
{
};
#endif


class CompareHelper
{
public:
	// Class containing natural string compare functor
	class CompareNaturalCStringsClass
	{
		/**
		 * CompareNaturalCStringsClass Natural order string comparison for CStringT-based strings.
		 * I doubt this would work for MBCS codepages, however.
		 * Copyright (C) 2005 Rasqual Twilight (C -> Java ->) **C++ port**
		 * Based on Java code, with the following LICENSE information.
		 *
		 * NaturalOrderComparator.java -- Perform 'natural order' comparisons of strings in Java.
		 * Copyright (C) 2003 by Pierre-Luc Paour <natorder@paour.com>
		 *
		 * Based on the C version by Martin Pool, of which this is more or less a straight conversion.
		 * Copyright (C) 2000 by Martin Pool <mbp@humbug.org.au>
		 *
		 * This software is provided 'as-is', without any express or implied
		 * warranty.  In no event will the authors be held liable for any damages
		 * arising from the use of this software.
		 *
		 * Permission is granted to anyone to use this software for any purpose,
		 * including commercial applications, and to alter it and redistribute it
		 * freely, subject to the following restrictions:
		 *
		 * 1. The origin of this software must not be misrepresented; you must not
		 * claim that you wrote the original software. If you use this software
		 * in a product, an acknowledgment in the product documentation would be
		 * appreciated but is not required.
		 * 2. Altered source versions must be plainly marked as such, and must not be
		 * misrepresented as being the original software.
		 * 3. This notice may not be removed or altered from any source distribution.
		 *
		 */

	public:
		typedef ChTraitsCRT<CString::XCHAR> CharTraits;
		typedef CDefaultCharTraits<CString::XCHAR> DefaultCharTraits;
		typedef CString::XCHAR XCHAR;

		int  operator ()(CString str1, CString str2) const
		{
			int ia = 0, ib = 0;
			int result;
			while (true) {
				// only count the number of zeroes leading the last number compared
				int nza = 0, nzb = 0;
				XCHAR ca = ia<str1.GetLength()? str1[ia] : _T('\0');
				XCHAR cb = ib<str2.GetLength()? str2[ib] : _T('\0');

				// skip over leading spaces or zeros
				while (CharTraits::IsSpace(ca) || ca == _T('0')) {
					if (ca == _T('0')) {
						++nza;
					} else {
						// only count consecutive zeroes
						nza = 0;
					}

					++ia;
					ca = ia<str1.GetLength()? str1[ia] : _T('\0');
				}

				while (CharTraits::IsSpace(cb) || cb == _T('0')) {
					if (cb == _T('0')) {
						++nzb;
					} else {
						// only count consecutive zeroes
						nzb = 0;
					}

					++ib;
					cb = ib<str1.GetLength()? str1[ib] : _T('\0');
				}

				// process run of digits
				if (CharTraits::IsDigit(ca) && CharTraits::IsDigit(cb)) {
					if ((result = CompareRight(str1.Right(str1.GetLength() - ia), str2.Right(str2.GetLength() - ib))) != 0) {
						return result;
					}
				}

				if (ca == _T('\0') && cb == _T('\0')) {
					// The strings compare the same (e.g. 0001 == 01).
					// Perhaps the caller will want to call strcmp to break the tie.
					return nza - nzb;
				}

				XCHAR uppera = DefaultCharTraits::CharToUpper(ca);
				XCHAR upperb = DefaultCharTraits::CharToUpper(cb);
				if (uppera < upperb) {
					return -1;
				} else if (uppera > upperb) {
					return +1;
				}
				++ia; ++ib;
			}
		}

	private:
		/**
			Helper comprator used by the natural order comparison function
		**/
		int  CompareRight(CString str1, CString str2) const
		{
			int bias = 0;
			int ia = 0;
			int ib = 0;

			// The longest run of digits wins.  That aside, the greatest
			// value wins, but we can't know that it will until we've scanned
			// both numbers to know that they have the same magnitude, so we
			// remember it in BIAS.
			for (; ; ia++, ib++)
			{
				XCHAR ca = ia<str1.GetLength()? str1[ia] : _T('\0');
				XCHAR cb = ib<str2.GetLength()? str2[ib] : _T('\0');

				if (!CharTraits::IsDigit(ca)
					&& !CharTraits::IsDigit(cb))
				{
					return bias;
				} else if (!CharTraits::IsDigit(ca)) {
					return -1;
				} else if (!CharTraits::IsDigit(cb)) {
					return +1;
				} else if (ca < cb) {
					if (bias == 0) {
						bias = -1;
					}
				} else if (ca > cb) {
					if (bias == 0) {
						bias = +1;
					}
				} else if (ca == _T('\0') && cb == _T('\0')) {
					return bias;
				}
			}
		}
	};
};


///////////////////////////////////////////////////////////////////////////////

#endif  // __STRINGTRAITS_H__
