#include "indicator.h"

#include <sstream>

namespace indicator
{

	SymbolManager::SymbolManager(indicator::TradingSession * session_)
	{
		lastSymbolID = 0;
		session = session_;

		session->getParamManager()->registerSink(this);
	}

	SymbolManager::~SymbolManager()
	{
		session->getParamManager()->unregisterSink(this);
	}

	int SymbolManager::registerSymbol(std::string symbolname, int symbolID)
	{
		
		IntStringMap::iterator it1 = symbolIndex.find(symbolID);
		if (it1 != symbolIndex.end() )
		{
			std::stringstream s;
			s << "Symbol ID " << symbolID ;
			s << " is already registered for " << it1->second << AT_FILE_LINE;

			throw ETradingSession ( s.str() );
		}

		StringIntMap::iterator it = symbolNames.find(symbolname);
		if (it != symbolNames.end() )
		{
			std::string s;
			s += "Symbol ";
			s += symbolname;
			s += " is already registered " AT_FILE_LINE;

			throw ETradingSession ( s );
		}

		symbolNames[symbolname] = symbolID;
		symbolIndex[symbolID] = symbolname;
		Tick & currTick = tickMap[symbolID];
		currTick.ID = symbolID;
		currTick.volatility = 0;
		currTick.meanPct = 0;
		return symbolID;
	}
	
	std::string SymbolManager::getSymbolName(int symbolID)
	{
		IntStringMap::iterator it = symbolIndex.find(symbolID);
		if (it == symbolIndex.end() )
		{
			std::stringstream s;
			s << "Symbol ID " << symbolID << " not found " AT_FILE_LINE;

			throw ETradingSession ( s.str() );
		}
		return it->second;
	}

	void SymbolManager::updateTick(Tick * tick)
	{
	//	tickMap[tick->symbolID] = (*tick);

		Tick & currTick = tickMap[tick->symbolID];

		if (currTick.bid + currTick.ask > 0)
		{
			currTick.meanPct=  
				0.999*currTick.meanPct + 0.001*( (tick->bid + tick->ask)/(currTick.bid + currTick.ask) - 1.0 );
			currTick.volatility =  0.999*currTick.volatility + 0.001*currTick.meanPct*currTick.meanPct;
		}

		currTick.ID = tick->ID;
		currTick.bid = tick->bid;
		currTick.ask = tick->ask;
		currTick.time = tick->time;

	}

	void SymbolManager::registerChartScript(ChartScript * chartScript)
	{
		int id = getSymbolId(chartScript->getSymbol());
		chartScript->setSymbolID(id);
		symbol2ChartScriptMap[id].push_back(chartScript);
	}

	int SymbolManager::findSymbolId(std::string symbolname)
	{
		StringIntMap::iterator it = symbolNames.find(symbolname);
		if (it != symbolNames.end())
			return it->second;
		else
			return -1;
	}

	int SymbolManager::getSymbolId(std::string symbolname)
	{
		
		StringIntMap::iterator it = symbolNames.find(symbolname);
		if (it != symbolNames.end())
			return it->second;
		else
		{
			std::string s;
			s += "Symbol ";
			s += symbolname;
			s += " is not registered " AT_FILE_LINE;

			throw ETradingSession ( s );
		}

		//symbolNames[symbolname] = lastSymbolID;
		//return (++lastSymbolID) - 1;
	}

	void SymbolManager::onParamsChange()
	{
		ParamManager * paramManager = session->getParamManager();

		for (
			SymbolTickMap::iterator it = tickMap.begin();
			it != tickMap.end(); it++)
		{
			paramManager->getParam(it->first, "symbol", "tick size" , it->second.size , 0.0001);
			paramManager->getParam(it->first, "symbol", "value factor" , it->second.valueFactor , 1);
		}
	}

	double SymbolManager::totalVolatility()
	{
		double result = 0;

		for (
			SymbolTickMap::iterator it = tickMap.begin();
			it != tickMap.end(); it++)
		{
			if (it->second.volatility > 0)
				result += sqrt(it->second.volatility);
		}		

		return result;
	}

	int SymbolManager::getSymbolCount()
	{
		return symbolIndex.size();
	}
}
