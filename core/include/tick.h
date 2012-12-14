#ifndef _TICK_H
#define _TICK_H

#ifndef _INDICATOR_H
#error This file must be included by <indicator.h>
#endif

namespace indicator
{

	template < typename T >
	T sign( T const &value )
	{
		return value < 0 ? -1 : 1;
	}

	typedef double DateTime;

	struct Tick
	{
		Tick()
		{
			ID = newId();
			lastone = false;
			valueFactor = 1.0;
		}

		unsigned int newId()
		{
			static unsigned int id_ = 0;
			return ID = ++id_;	
		}
/*	
		Tick (const & Tick other)
		{
			time = other.time;
			open = other.open;
			high = other.high;
			low = other.low;
			close = other.close;
			volume = volume.close;
		}

		const Tick & operator=(const & Tick other)
		{
			time = other.time;
			open = other.open;
			high = other.high;
			low = other.low;
			close = other.close;
			volume = volume.close;
			return *this;
		}
*/

		// lets use the internal memcpy - more efficient for this struct

		DateTime time;
		int symbolID;
		unsigned int ID;
		double bid;
		double ask;
		double open;
		double high;
		double low;
		double close;
		double volume;
		bool lastone;
		double size;
		double valueFactor;
		double meanPct;
		double volatility;

	};
	typedef std::vector<double> PriceVector;
//	typedef boost::shared_ptr<Tick> TickPtr;

//////////////////////////////////////////////
//
//  ParameterField
//
//////////////////////////////////////////////


typedef std::map<string,string> ParamMap;
typedef std::map<string,double * > ParamMapDoubleRef;

}

#endif