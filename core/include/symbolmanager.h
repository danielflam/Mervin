#ifndef _SYMBOL_MANAGER_H
#define _SYMBOL_MANAGER_H

#ifndef _INDICATOR_H
#error This file must be included by <indicator.h>
#endif

namespace indicator
{

/*
	struct Symbol
	{
		int ID;
		double CurrentValue;
	};
*/
	typedef std::runtime_error ESymbolManager;

	// right now AS SIMPLE AS IT GETS 
	// Later symbol manager will drive app.

	typedef std::map<int, Tick> SymbolTickMap;
	typedef std::map<int, ChartScriptPtrList> ChartScriptPtrListMap;
	typedef std::map<string, int> StringIntMap;
	typedef std::map<int, string> IntStringMap;

	class SymbolManager
		: public IParamChangeEventSink
	{
	private:

	public:
		SymbolManager(indicator::TradingSession * session_);
		virtual ~SymbolManager();
	
		Tick & getCurrentTick(int ID)
		{
			return tickMap[ID];
		}

//		void setCurrentValue(int ID, double Value)
//		{
//			valueMap[ID] = Value;
//		}

		double getTickSize(int ID)
		{
			return tickMap[ID].size;
		}

		double getTickValueFactor(int ID)
		{
			return tickMap[ID].valueFactor;
		}

		void setTickSize(int ID, double Value)
		{
			Tick & tick = tickMap[ID];
			tick.size = Value;
		}

		int registerSymbol(std::string symbolname, int symbolID);
		int getSymbolId(std::string symbolname);
		int findSymbolId(std::string symbolname); // returns -1 on error - instead of exception
		std::string getSymbolName(int symbolID);

		int getSymbolCount();

		void registerChartScript(ChartScript * chartScript);

//		void unregisterChartScript(ChartScript * chartScript)
//		{
//
//		}

		void updateTick(Tick * tick);

		void onParamsChange();

		double totalVolatility();
	private:

		SymbolTickMap tickMap;
		ChartScriptPtrListMap symbol2ChartScriptMap;
		StringIntMap symbolNames;		
		IntStringMap symbolIndex;
		

		TradingSession * session;
		int lastSymbolID;

	};
}

#endif
