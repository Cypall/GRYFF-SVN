// $Id: grfexceptions.h 9 2005-08-12 17:33:08Z Rasqual $
//Copyright (c) 2004 Rasqual Twilight

#if !defined(__GRFEXCEPTIONS_H__)
#define __GRFEXCEPTIONS_H__

#include  <string>
#include  <stdexcept>

  // Base exception
class GPException : public std::logic_error
{
public:
	GPException(const std::string &arg) : logic_error(arg) {}
};
	  // Input/Output exception : failed to open, seek, or read/write a file.
	class GPIoException : public GPException
	{
	public:
		GPIoException(const std::string &arg) : GPException(arg) {}
		void SetErrno(const int &error_no) { m_errno = error_no; }
		const int &GetErrno() const { return m_errno; }
	protected:
		int m_errno;
	};
	  // Tried to perform an operation while conditions are not suitable
	class GPIllegalStateException : public GPException
	{
	public:
		GPIllegalStateException(const std::string &arg) : GPException(arg) {}
	};

	  // Invalid parameter exception: unexpected data
	class GPInvalidParameterException : public GPException
	{
	public:
		GPInvalidParameterException(const std::string &arg) : GPException(arg) {}
	};
		  // a parameter was the null pointer; a valid pointer was expected
		class GPNullPointerException : public GPInvalidParameterException
		{
		public:
			GPNullPointerException(const std::string &arg) : GPInvalidParameterException(arg) {}
		};

		  // Parameter has an unlogical value
		class GPUnlogicalException : public GPInvalidParameterException
		{
		public:
			GPUnlogicalException(const std::string &arg) : GPInvalidParameterException(arg) {}
		};

		  // Bad data stream
		class GPInvalidSequenceException : public GPInvalidParameterException
		{
		public:
			GPInvalidSequenceException(const std::string &arg) : GPInvalidParameterException(arg) {}
		};

		  // Size overflow
		class GPSizeTooLongException : public GPInvalidParameterException
		{
		public:
			GPSizeTooLongException(const std::string &arg) : GPInvalidParameterException(arg) {}
		};

	  // Does not support this operation
	class GPUnsupportedOperationException : public GPException
	{
	public:
		GPUnsupportedOperationException(const std::string &arg) : GPException(arg) {}
	};

	  // Memory allocation exception
	class GPMemAllocateException : public GPException
	{
	public:
		GPMemAllocateException(const std::string &arg) : GPException(arg) {}
	};

	  // Cancelling by request
	class GPCancelOperationException : public GPException
	{
	public:
		GPCancelOperationException(const std::string &arg) : GPException(arg) {}
	};

#endif  // !defined(__GRFEXCEPTIONS_H__)
