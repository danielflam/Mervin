#ifndef BOOST_FORMAT_HELPER_H
#define BOOST_FORMAT_HELPER_H

#include "boost/format.hpp"

//////////////////////////////////////////
//
// strong type format functions 
// String return type
//
/////////////////////////////////////////

inline const char * fmtString(const char * fmt)
{
	static string res;
	res = fmt;
	return res.c_str();
}

template <class T1>
inline const char * fmtString(const char * fmt, T1 t1)
{
	static string res;
	res = boost::str( boost::format( fmt ) % t1 );
	return res.c_str();
}

template <class T1, class T2>
inline const char * fmtString(const char * fmt, T1 t1, T2 t2)
{
	static string res;
	res = boost::str( boost::format( fmt ) % t1 %t2 );
	return res.c_str();
}

template <class T1, class T2, class T3>
inline const char * fmtString(const char * fmt, T1 t1, T2 t2, T3 t3)
{
	static string res;
	res = boost::str( boost::format( fmt ) % t1 % t2 %t3 );
	return res.c_str();
}

template <class T1, class T2, class T3, class T4>
inline const char * fmtString(const char * fmt, T1 t1, T2 t2, T3 t3, T4 t4)
{
	static string res;
	res = boost::str( boost::format( fmt ) % t1 % t2 % t3 % t4);
	return res.c_str();
}

template <class T1, class T2, class T3, class T4, class T5>
inline const char * fmtString(const char * fmt, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5)
{
	static string res;
	res = boost::str( boost::format( fmt ) % t1 % t2 % t3 % t4 % t5);
	return res.c_str();
}

template <class T1, class T2, class T3, class T4, class T5, class T6>
inline const char * fmtString(const char * fmt, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6)
{
	static string res;
	res = boost::str( boost::format( fmt ) % t1 % t2 % t3 % t4 % t5 %t6 );
	return res.c_str();
}

template <class T1, class T2, class T3, class T4, class T5, class T6, class T7>
inline const char * fmtString(const char * fmt, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7)
{
	
	static string res;
	res = boost::str( boost::format( fmt ) % t1 % t2 % t3 % t4 % t5 % t6 % t7);
	return res.c_str();
}

#endif