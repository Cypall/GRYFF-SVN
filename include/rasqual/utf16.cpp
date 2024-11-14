// $Id: utf16.cpp 9 2005-08-12 17:33:08Z Rasqual $
#include "utf16.h"

//derivative from IBM ICU - X-derived license
//http://oss.software.ibm.com/cvs/icu/~checkout~/icu/license.html
//Copyright (c) 2004 Rasqual Twilight

//ICU License - ICU 1.8.1 and later
//COPYRIGHT AND PERMISSION NOTICE

//Copyright (c) 1995-2003 International Business Machines Corporation and others
//All rights reserved.

//Permission is hereby granted, free of charge, to any person obtaining a
//copy of this software and associated documentation files (the
//"Software"), to deal in the Software without restriction, including
//without limitation the rights to use, copy, modify, merge, publish,
//distribute, and/or sell copies of the Software, and to permit persons
//to whom the Software is furnished to do so, provided that the above
//copyright notice(s) and this permission notice appear in all copies of
//the Software and that both the above copyright notice(s) and this
//permission notice appear in supporting documentation.

//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
//OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT
//OF THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
//HOLDERS INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL
//INDIRECT OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING
//FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
//NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
//WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

//Except as contained in this notice, the name of a copyright holder
//shall not be used in advertising or otherwise to promote the sale, use
//or other dealings in this Software without prior written authorization
//of the copyright holder.

//--------------------------------------------------------------------------------
//All trademarks and registered trademarks mentioned herein are the property of their respective owners.


/**
	Extract a single UTF-32 value from a string.
	If a validity check is required, use UTF16::IsLegal() on the return value.
	If the char retrieved is part of a surrogate pair, its supplementary
	character will be returned. If a complete supplementary character is
	not found the incomplete character will be returned.
**/
int UTF16::CharAt(const UTF16_t source[], int offset16)
{
	int single = source[offset16];
	if (single < LEAD_SURROGATE_MIN_VALUE ||
	    single > TRAIL_SURROGATE_MAX_VALUE)
	{
		return single;
	}
	// Convert the UTF-16 surrogate pair if necessary.
	// For simplicity in usage, and because the frequency of pairs is
	// low, look both directions.

	if (single <= LEAD_SURROGATE_MAX_VALUE)
	{
		// Lead surrogate, is there a matching trail surrogate?
		UTF16_t trail = source[++offset16];  // note: may be end of string, too
		if (IsTrailSurrogate(trail))
		{
			return (single << LEAD_SURROGATE_SHIFT_) + trail + SURROGATE_OFFSET_;
		}
	}
	else
	{
		--offset16;
		if (offset16 >= 0)
		{
			// single is a trail surrogate so
			UTF16_t lead = source[offset16];
			if (IsLeadSurrogate(lead)) {
				return (lead << LEAD_SURROGATE_SHIFT_) + single + SURROGATE_OFFSET_;
			}
		}
	}
	return single;  // return unmatched surrogate
}

/**
	Returns the number of UTF16_t occupied by a character (single or surrogate pair)
**/
unsigned int UTF16::GetCharCount(int char32)
{
	if (char32 < SUPPLEMENTARY_MIN_VALUE) {
		return 1U;
	}
	return 2U;
}

/**
	A code point is illegal if and only if
	 . Out of bounds, less than 0 or greater than MAX_VALUE
	 . A surrogate value, 0xD800 to 0xDFFF
	 . Not-a-character, having the form 0x xxFFFF or 0x xxFFFE
	Note: legal does not mean that it is assigned in this version of Unicode.
**/
bool UTF16::IsLegal(int char32)
{
	if (char32 < MIN_VALUE) {
		return false;
	}
	if (char32 < SURROGATE_MIN_VALUE) {
		return true;
	}
	if (char32 <= SURROGATE_MAX_VALUE) {
		return false;
	}
	if (IsNonCharacter(char32)) {
		return false;
	}
	return (char32 <= MAX_VALUE);
}


/**
	Tests whether a UTF16 string is valid, that is to say it does not contain
	any unmatched surrogate pair element (unpaired surrogate) and
	no noncharacters that can be used internally by an implementation
	(BOM + FDD0... ~ FDEF...)
	str is supposed to be non null and terminated by a nul character.
**/
bool UTF16::IsValidString(const UTF16_t str[])
{
	unsigned int maxbound = sizeof(str) / sizeof(str[0]);
	int ch;
	for (unsigned int i = 0; i < maxbound; i += UTF16::GetCharCount(ch) )
	{
		ch = UTF16::CharAt(str, i);  // Get the character or pair
		if ( !UTF16::IsLegal(ch) )
		{
			return false;
		}
	}
	return true;
}

/**
	Tests whether a character is the lead part of a surrogate pair
	(high surrogate U+d800..U+dbff)
**/
bool UTF16::IsLeadSurrogate(const UTF16_t char16)
{
	return ((char16 & 0xfffffc00) == 0xd800);
}

/**
	Tests whether a character is the trail part of a surrogate pair
	(low surrogate U+dc00..U+dfff)
**/
bool UTF16::IsTrailSurrogate(const UTF16_t char16)
{
	return ((char16 & 0xfffffc00) == 0xdc00);
}

/**
	Tests whether a character is part of a surrogate pair
	(surrogate U+d800..U+dfff)
**/
bool UTF16::IsSurrogate(const UTF16_t char16)
{
	return ((char16 & 0xfffff800) == 0xd800);
}

/**
	Determines if codepoint is a non character
**/
bool UTF16::IsNonCharacter(int char32)
{
	if ((char32 & NON_CHARACTER_SUFFIX_MIN_3_0_) ==
	                                NON_CHARACTER_SUFFIX_MIN_3_0_)
	{
		return true;
	}

	return char32 >= NON_CHARACTER_MIN_3_1_ && char32 <=  NON_CHARACTER_MAX_3_1_;
}

