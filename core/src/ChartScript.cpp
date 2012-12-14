#include "indicator.h"
//#include <boost/lexical_cast.hpp>
#include "symbolmanager.h"
#include "ordermanager.h"

namespace indicator 
{


	//////////////////////////////////////////////
	//
	//  ChartScriptImpl
	//
	//
	//////////////////////////////////////////////

	class ChartScriptImpl : public PImpl
	{
	public:
		ChartScriptImpl( TradingSession * session, ChartScriptFactoryManager & parent_ ) : 
		  parent(parent_)
		{
			instanceID = newInstanceID();
			enabled = true;
			started = false;
			symbolID = 0;
			paramsDirty = true;
		}
 
		int state(){return 1;}

		int newInstanceID()
		{
			static int ID = 0;

			return ++ID;
		}

		int instanceID;
		int symbolID;
		std::string symbol;
		ChartScript::ChartScriptOptions runnerOptions;


		//		PriceVector times;
		//		PriceVector opens;
		//		PriceVector closes;
		//		PriceVector highs;
		//		PriceVector lows;
		//		PriceVector volumes;

		ChartScriptFactoryManager & parent;
		IndicatorSeriesIndex indicatorList;
		bool paramsDirty;
		bool started;
		bool enabled;
	};

	//////////////////////////////////////////////
	//
	//  ChartScript
	//
	//
	//////////////////////////////////////////////

	ChartScript::ChartScript(TradingSession * session, ChartScriptFactoryManager & parent_) :
	TradingSessionObject(session), pImpl(new ChartScriptImpl(session, parent_))
	{
//		ChartScriptImpl * p = dynamic_cast<ChartScriptImpl *>(pImpl);

		lastbarID = -1;
		prevbarID = -1;

		session->insertScript(this);
	}

	ChartScript::~ChartScript()
	{
		delete pImpl;
	}

	void ChartScript::updateTick(Tick * tick)
	{
		internalUpdateTick(tick);
	}

	void ChartScript::internalUpdateTick(Tick * tick)
	{
		ChartScriptImpl * p = dynamic_cast<ChartScriptImpl *>(pImpl);

		if ( p->symbolID != tick->symbolID )
			return;		

		if (p->paramsDirty)
		{
			ParamManager & paramManager = *session->getParamManager();

			onReadParams(paramManager);

			p->paramsDirty = false;
		}

		//symbol_manager::SymbolManager::Instance().setCurrentValue(scriptID(), tick->open);
//		session->getOrderManager()->Update(	getSymbolID(), tick );

	}
	
	void ChartScript::onOrderManagerUpdated(Tick * tick)
	{
		//times.push_back(tick->time);
		//opens.push_back(tick->open);
		//highs.push_back(tick->high);
		//lows.push_back(tick->low);
		//closes.push_back(tick->close);
		//volumes.push_back(tick->volume);

		ChartScriptImpl * p = dynamic_cast<ChartScriptImpl *>(pImpl);

		if ( p->symbolID != tick->symbolID )
			return;		

		prevbarID = lastbarID;
		lastbarID = tick->ID;

		prevBar = curBar;

		curBar = *tick;
		if (updateIndicators(tick) == 0)
			executeScript();

	}

	void ChartScript::start()
	{
		if (!enabled())
			return;

		internalOnStart();
//		ChartScriptImpl * impl = dynamic_cast<ChartScriptImpl *>(pImpl);
		 
//		for (IndicatorSeriesIndex::iterator it = impl->indicatorList.begin();
//			it != impl->indicatorList.end(); it++)
//		{		
//			(*it)->start();
//		}

		session->getParamManager()->registerSink(this);


		dynamic_cast<ChartScriptImpl *>(pImpl)->started = true;
	}

	void ChartScript::stop()
	{

		session->getParamManager()->unregisterSink(this);

		internalOnStop();

		dynamic_cast<ChartScriptImpl *>(pImpl)->started = false;

	}

	void ChartScript::reset()
	{
		internalOnReset();
	}

	void ChartScript::setSymbolID(int ID)
	{
		ChartScriptImpl * p = dynamic_cast<ChartScriptImpl *>(pImpl);

		int oldID = p->symbolID;
		p->symbolID = ID;

		session->onChartScriptSymbolIDChanged(this, oldID, ID);
	}

	int ChartScript::getInstanceID()
	{
		return dynamic_cast<ChartScriptImpl *>(pImpl)->instanceID;
	}
	
	int ChartScript::getSymbolID()
	{
		return dynamic_cast<ChartScriptImpl *>(pImpl)->symbolID;
	}
	
	string ChartScript::getSymbol()
	{
		return dynamic_cast<ChartScriptImpl *>(pImpl)->symbol;
	}
	
	void ChartScript::setSymbol(string s)
	{
		dynamic_cast<ChartScriptImpl *>(pImpl)->symbol = s;

		setSymbolID(session->getSymbolManager()->getSymbolId(s));
	}

	ChartScript::ChartScriptOptions ChartScript::getRunnerOptions() 
	{
		return dynamic_cast<ChartScriptImpl *>(pImpl)->runnerOptions;
	}
	
	bool ChartScript::hasRunnerOptions(ChartScriptOptions options) 
	{
		return (dynamic_cast<ChartScriptImpl *>(pImpl)->runnerOptions & options) == options;
	}

	void ChartScript::setRunnerOptions(ChartScriptOptions options) 
	{
		dynamic_cast<ChartScriptImpl *>(pImpl)->runnerOptions = options;
	}

	void ChartScript::onParamsChange()
	{
		dynamic_cast<ChartScriptImpl *>(pImpl)->paramsDirty = true;
	}

	void ChartScript::registerIndicatorSeries(IndicatorSeries *indicator)
	{
		dynamic_cast<ChartScriptImpl *>(pImpl)->indicatorList.push_back(indicator);
	}

	void ChartScript::unregisterIndicatorSeries(IndicatorSeries * indicator)
	{
		dynamic_cast<ChartScriptImpl *>(pImpl)->indicatorList.remove(indicator);
	}

	bool ChartScript::active()
	{
		return dynamic_cast<ChartScriptImpl *>(pImpl)->started;
	}

	bool ChartScript::enabled()
	{
		return dynamic_cast<ChartScriptImpl *>(pImpl)->enabled;
	}
	
	void ChartScript::enable()
	{
		dynamic_cast<ChartScriptImpl *>(pImpl)->enabled = true;
	}
	
	void ChartScript::disable()
	{
		dynamic_cast<ChartScriptImpl *>(pImpl)->enabled = false;
	}
}