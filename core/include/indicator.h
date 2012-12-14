#ifndef _INDICATOR_H
#define _INDICATOR_H

#include <boost/ptr_container/ptr_map.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/ptr_container/ptr_list.hpp>
#include <boost/shared_ptr.hpp>
//#include <boost/functional/factory.hpp>
#include <boost/function.hpp>
#include <boost/utility.hpp>
#include <boost/variant.hpp>
#include <boost/format.hpp>

//#include <memory>
//using std::auto_ptr;

#include <string>
using std::string;

#include <vector>
//using std::vector;

#include <map>
#include <list>

#include "stringexception.h"
#include "managedlist.h"

namespace indicator
{
	// FORWARDS
	class DataStore;
	class TradingObject;
	class DynamicTradingObject;
	class IndicatorSeries;
	class IndicatorSeriesEventBase;
	class ChartScript;
	class ChartScriptFactoryBase; 
	class ChartScriptFactoryManager;
	class TradingSessionObject;
	class TradingSession;
	class TradingSessionManager;
	class ExecutionHandler;
	class OrderManager;
	struct Order;
	class Portfolio;
	struct TradeInfo;
	class IOnCalculateExposure;
}

#include "templatemagic.h"
#include "eventsink.h"
#include "tradingobject.h"
#include "datastore.h"
#include "tick.h"
#include "parammanager.h"
#include "logger.h"
#include "filelogger.h"
#include "IndicatorSeries.h"
#include "position.h"
#include "tradeInfo.h"
#include "order.h"
#include "portfolio.h"
#include "executionHandler.h"
#include "ordermanager.h"
#include "portfoliohistory.h"
#include "ChartScript.h"
#include "ChartScriptManager.h"
#include "symbolmanager.h"
#include "tradingsession.h"


#endif

