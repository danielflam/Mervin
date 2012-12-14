#include "indicator.h"

namespace indicator 
{


//////////////////////////////////////////////
//
//  ChartScriptFactoryManager
//
//
//////////////////////////////////////////////


	ChartScriptFactoryManager::ChartScriptFactoryManager() 
	{
	}

	ChartScriptFactoryManager::~ChartScriptFactoryManager()
	{
	}

	ChartScriptFactoryManager & ChartScriptFactoryManager::getInstance()
	{
		static ChartScriptFactoryManager instance;

		return instance;
	}

	void ChartScriptFactoryManager::registerScript( string scriptName, ChartScriptFactoryBase * factory )
	{
		ChartScriptFactoryBasePtr p(factory);
		chartScriptFactories[scriptName] = p;
	}

	ChartScript * ChartScriptFactoryManager::createScriptInstance(string scriptName, 
		TradingSession * session, 		
		ChartScript::ChartScriptOptions options)
	{
		ChartScriptFactoryBasePtr factory = chartScriptFactories[scriptName];
		ChartScript * script(factory->create(session, *this));

		script->setRunnerOptions(options);
		//if ( script->hasRunnerOptions(ChartScript::CSO_AUTOSTART) )
		//	script->start();

		return script;
	}

}