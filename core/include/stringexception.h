#ifndef _stringexception_h
#define _stringexception_h

#ifndef _INDICATOR_H
#error This file must be included by <indicator.h>
#endif

#include <stdexcept>

namespace indicator 
{

#define _STRINGIFY(X) #X
#define _TOSTRING(x) _STRINGIFY(x)
#define AT_FILE_LINE " at " __FILE__ ":" _TOSTRING(__LINE__) 
#define _RAISE_EXCEPTION( X , msg ) throw X ( std::string(msg AT_FILE_LINE) )

/*	template <class T>
	class ERuntimeException : public std::runtime_error
	{
	public: 
		ERuntimeException(const string & s) : std::runtime_error(s){}
	};*/


};

#endif

