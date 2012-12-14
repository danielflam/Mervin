#include "indicator.h"


namespace indicator
{

#define DATE_MIN -657434

/* Convert a VT_DATE value to a Julian Date */
//static int JulianFromDate(int dateIn)
//{
//  int julianDays = dateIn;
//
//  julianDays -= DATE_MIN; /* Convert to + days from 1 Jan 100 AD */
//  julianDays += 1757585;  /* Convert to + days from 23 Nov 4713 BC (Julian) */
//  return julianDays;
//}
//
//static int DateFromJulian(int dateIn)
//{
//  int julianDays = dateIn;
//
//  julianDays -= 1757585;  /* Convert to + days from 1 Jan 100 AD */
//  julianDays += DATE_MIN; /* Convert to +/- days from 1 Jan 1899 AD */
//  return julianDays;
//}
//


/* Convert Day/Month/Year to a Julian date - from PostgreSQL */
/*
static double JulianFromDMY(unsigned short year, unsigned short month, unsigned short day)
{
	int m12 = month <= 2 ? -1 : 0; //(month - 14) / 12;

  return (1461 * (year + 4800 + m12)) / 4 + (367 * (month - 2 - 12 * m12)) / 12 -
           (3 * ((year + 4900 + m12) / 100)) / 4 + day - 32075;
}
*/



/* Convert a Julian date to Day/Month/Year - from PostgreSQL */
/*static void DMYFromJulian(
	int jd, 
	unsigned short &year, 
	unsigned short &month, 
	unsigned short &day
)
{
  int j, i, l, n;

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
/*
static void DMYFromDateTime(int jd, 
	unsigned short &year, 
	unsigned short &month, 
	unsigned short &day
	)
{
   jd = JulianFromDate(jd);
   DMYFromJulian(jd, year, month, day);
}

double DateTimeFromDMY(
	unsigned short year, 
	unsigned short month, 
	unsigned short day)
{
	double res = JulianFromDMY(year, month, day);

	return DateFromJulian(res);
}

	const char * formatOleTime(double time)
	{
		static string res;
		unsigned short y, m, d;

		DMYFromDateTime(int(time), y, m , d);
		int YMD = int(time);


		double HMS = time - YMD;
		

		int S = int(HMS * 86400);


		int H = S / 3600;

		S = S % 3600;
		int M = S / 60;
		S = S % 60;

		res = boost::str( boost::format( "%d/%d/%d-%2d:%2d:%2d" ) % y %m %d %H %M % S );

		return res.c_str();


	}
	*/
}