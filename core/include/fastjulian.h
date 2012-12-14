#ifndef _FASTJULIAN_H
#define _FASTJULIAN_H

#include <string>

namespace fastjulian
{
	const double OLE_SECOND = 1/86400.0;

	int JulianFromDate(int dateIn);
	int DateFromJulian(int dateIn);


	void DMYFromDateTime(
		int jd, 
		unsigned short &year, 
		unsigned short &month, 
		unsigned short &day
		);

	double DateTimeFromDMY(
		unsigned short year, 
		unsigned short month, 
		unsigned short day
		);

	int JulianFromDMY(	
		unsigned short year, 
		unsigned short month, 
		unsigned short day
		);

	void DMYFromJulian(int jd, 
		unsigned short &year, 
		unsigned short &month, 
		unsigned short &day
		);

	const char * formatOleDate(double date);
	std::string formatOleDateTime(double datetime);
	const char ** getjuliandatecache();
	const int getjuliandaycache(int i);
	const int getjulianmonthcache(int i);
	const int getjulianyearcache(int i);

}
#endif
