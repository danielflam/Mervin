#ifndef __SCAN_DATA_H
#define __SCAN_DATA_H

#define MAXSYMLEN 20
#define MAXARGS 6

#include <istream>

struct TickDataStruct
{
	TickDataStruct()
	{
		reset();
	}

	void reset()
	{
	  nArgs = 0;
	  cmd[0] = '\0'; 
	}

	char sym[MAXSYMLEN+1];
	union {
		double asArray[MAXARGS];
		struct {
			double date;
			double time;
			double bid;
			double bidSize;
			double ask;
			double askSize;
		} byName;
	} args;
	int nArgs;
};

typedef void (*OnDataReady)(TickDataStruct * tickDataStruct);
void scanData(istream & instream, OnDataReady * onDataReady);


#endif