#ifndef _PORTFOLIO_HISTORY_H
#define _PORTFOLIO_HISTORY_H

#ifndef _INDICATOR_H
#error This file must be included by <indicator.h>
#endif

namespace indicator
{

	struct HistoryNode
	{
		indicator::DateTime when;
		double open, high, low, close;
		int count;
		unsigned short year; 
		unsigned short month; 
		unsigned short day;
	};

	typedef HistoryNode * HistoryNodePtr;

	typedef boost::ptr_list<HistoryNode> HistoryNodePtrList;


}


#endif
