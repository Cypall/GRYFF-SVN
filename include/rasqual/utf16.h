// $Id: utf16.h 9 2005-08-12 17:33:08Z Rasqual $
//derivative from IBM ICU - X-derived license
//http://oss.software.ibm.com/cvs/icu/~checkout~/icu/license.html
//Copyright (c) 2004 Rasqual Twilight

#if !defined(__UTF16_H__)
#define __UTF16_H__

#if defined(WIN32)
# include <windows.h>
  typedef UINT16 UTF16_t;
#else             // !defined(WIN32)
# include <inttypes.h>
  typedef uint16_t UTF16_t;
#endif            // defined(WIN32)


namespace UTF16
{
	enum
	{
		LEAD_SURROGATE_MIN_VALUE = 0xD800,
		TRAIL_SURROGATE_MIN_VALUE = 0xDC00,
		LEAD_SURROGATE_MAX_VALUE = 0xDBFF,
		TRAIL_SURROGATE_MAX_VALUE = 0xDFFF,
		SURROGATE_MIN_VALUE = LEAD_SURROGATE_MIN_VALUE,
		SURROGATE_MAX_VALUE = TRAIL_SURROGATE_MAX_VALUE,

		SUPPLEMENTARY_MIN_VALUE  = 0x10000,
		CODEPOINT_MIN_VALUE = 0,
		CODEPOINT_MAX_VALUE = 0X10FFFF,
		MIN_VALUE = CODEPOINT_MIN_VALUE,
		MAX_VALUE = CODEPOINT_MAX_VALUE,

		NON_CHARACTER_SUFFIX_MIN_3_0_ = 0xFFFE,
		NON_CHARACTER_MIN_3_1_ = 0xFDD0,
		NON_CHARACTER_MAX_3_1_ = 0xFDEF,

		LEAD_SURROGATE_SHIFT_ = 10U,
		SURROGATE_OFFSET_ =
                           SUPPLEMENTARY_MIN_VALUE -
                           (SURROGATE_MIN_VALUE <<
                           LEAD_SURROGATE_SHIFT_) -
                           TRAIL_SURROGATE_MIN_VALUE

	};

	/**
		Extract a single UTF-32 value from a string.
		If a validity check is required, use UTF16::IsLegal() on the return value.
		If the char retrieved is part of a surrogate pair, its supplementary
		character will be returned. If a complete supplementary character is
		not found the incomplete character will be returned.
	**/
	int CharAt(const UTF16_t source[], int offset16);

	/**
		Returns the number of UTF16_t occupied by a character (single or surrogate pair)
	**/
	unsigned int GetCharCount(int char32);

	/**
		A code point is illegal if and only if
		 . Out of bounds, less than 0 or greater than MAX_VALUE
		 . A surrogate value, 0xD800 to 0xDFFF
		 . Not-a-character, having the form 0x xxFFFF or 0x xxFFFE
		Note: legal does not mean that it is assigned in this version of Unicode.
	**/
	bool IsLegal(int char32);

	/**
		Tests whether a UTF16 string is valid, that is to say it does not contain
		any unmatched surrogate pair element (unpaired surrogate) and
		no noncharacters that can be used internally by an implementation
		(BOM + FDD0... ~ FDEF...)
		str is supposed to be non null and terminated by a nul character.
	**/
	bool IsValidString(const UTF16_t str[]);

	/**
		Tests whether a character is the lead part of a surrogate pair
		(high surrogate U+d800..U+dbff)
	**/
	bool IsLeadSurrogate(const UTF16_t);

	/**
		Tests whether a character is the trail part of a surrogate pair
		(low surrogate U+dc00..U+dfff)
	**/
	bool IsTrailSurrogate(const UTF16_t);

	/**
		Tests whether a character is part of a surrogate pair
		(surrogate U+d800..U+dfff)
	**/
	bool IsSurrogate(const UTF16_t char16);

	/**
		Determines if codepoint is a non character
	**/
	bool IsNonCharacter(int char32);
};


#endif  // !defined(__UTF16_H__)
