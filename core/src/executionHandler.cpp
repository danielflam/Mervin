#include "indicator.h"

namespace indicator
{

	ExecutionHandler::ExecutionHandler(TradingSession * session_)
		:	TradingObject(), 
		session(session_)
	{
//		ParamManager & paramManager = *session->getParamManager();
//		paramManager.registerSink(this);
	}

	void ExecutionHandler::start()
	{
		ParamManager & paramManager = *session->getParamManager();
		paramManager.registerSink(this);
	}

	ExecutionHandler::~ExecutionHandler()
	{
//		ParamManager & paramManager = *session->getParamManager();
//		paramManager.unregisterSink(this);
	}


}