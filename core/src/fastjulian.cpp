#include "fastjulian.h"
#include "stdio.h"
#include <string>

namespace fastjulian
{

	const int STARTDATE = 2415019 ; // 30/12/1899

//	extern const char * juliandatecache[];
//	extern const int fastjulian::juliandaycache[];
//	extern const int julianmonthcache[];
//	extern const int julianyearcache[];

#define DATE_MIN -657434



	/* Convert a VT_DATE value to a Julian Date */
	int JulianFromDate(int dateIn)
	{
		int julianDays = dateIn;

		julianDays -= DATE_MIN; /* Convert to + days from 1 Jan 100 AD */
		julianDays += 1757585;  /* Convert to + days from 23 Nov 4713 BC (Julian) */
		return julianDays;
	}

	int DateFromJulian(int dateIn)
	{
		int julianDays = dateIn;

		julianDays -= 1757585;  /* Convert to + days from 1 Jan 100 AD */
		julianDays += DATE_MIN; /* Convert to +/- days from 1 Jan 1899 AD */
		return julianDays;
	}

	void DMYFromDateTime(int jd, 
		unsigned short &year, 
		unsigned short &month, 
		unsigned short &day
		)
	{
//		jd = JulianFromDate(jd);
//		DMYFromJulian(jd, year, month, day);
//			jd += 363;
			day = getjuliandaycache(jd);
			month = getjulianmonthcache(jd);
			year = getjulianyearcache(jd);

	}

	double DateTimeFromDMY(
		unsigned short year, 
		unsigned short month, 
		unsigned short day)
	{
		double res = JulianFromDMY(year, month, day);

		return DateFromJulian(int(res));
	}


	int JulianFromDMY(
		unsigned short year, 
		unsigned short month, 
		unsigned short day
		)
	{
		int m12 = month <= 2 ? -1 : 0; //(month - 14) / 12;

		return (1461 * (year + 4800 + m12)) / 4 + (367 * (month - 2 - 12 * m12)) / 12 -
			(3 * ((year + 4900 + m12) / 100)) / 4 + day - 32075;

	}

	void DMYFromJulian(
		int jd, 
		unsigned short &year, 
		unsigned short &month, 
		unsigned short &day
		)
	{
		jd -= STARTDATE;

//		if ((jd >= 0) && (jd <= 65534))
//		{
//			jd += 363;
			day = getjuliandaycache(jd);
			month = getjulianmonthcache(jd);
			year = getjulianyearcache(jd);
//		}
/*		else
		{
			int j, i, l, n;

			jd += STARTDATE;

			l = jd + 68569;
			n = (l * 4) / 146097;
			l = l - (n * 146097 + 3) / 4;
			i = (4000 * (l + 1)) / 1461001;
			l = l - (i * 1461) / 4 + 31 ;
			j = (l * 80) / 2447;
			day = l - (j * 2447) / 80;
			l = j / 11;
			month = (j + 2) - (12 * l);
			year = 100 * (n - 49) + i + l;
		}
	*/
	}

	const char * formatOleDate(double jd)
	{
		static char buf[50];

		//jd -= STARTDATE; 

		//jd += 363;

//		if ((jd >= 0) && (jd <= 65534))
//		{
			return getjuliandatecache()[int(jd)-1];
//		}
//		else
//		{
//			unsigned short d, m, y;
//			jd -= 363;
//			jd += STARTDATE;
//			DMYFromJulian( int(jd), y,m,d);

//			sprintf_s(buf, 50, "%2d/%2d/%4d", m,d,y);

//			return buf;
//		}
	}

	std::string formatOleDateTime(double datetime)
	{
		char buf[50];

		int YMD = int(datetime);


		double HMS = datetime - YMD;


		int S = int(HMS * 86400);


		int H = S / 3600;

		S = S % 3600;
		int M = S / 60;
		S = S % 60;

		sprintf_s(buf, 50, "%s-%2d:%2d:%2d",getjuliandatecache()[int(YMD)-1], H,M,S); 

		return std::string(buf);
	}
}