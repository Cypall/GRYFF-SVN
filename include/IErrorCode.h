// $Id$
#ifndef __IERRORCODE_H__
#define __IERRORCODE_H__

template <typename T>
class IErrorCode
{
public:
	virtual void SetErrorCode(const T&) = 0;
	virtual const T &GetErrorCode() const = 0;
};

#endif //__IERRORCODE_H__
