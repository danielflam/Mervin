
#include "indicator.h"


namespace indicator
{


	/////////////////////////////////////////////////////////
	//
	//  TradingSessionImpl
	//
	//
	/////////////////////////////////////////////////////////


	class TradingSessionImpl : public PImpl
	{
	public:

		TradingSessionImpl(TradingSession * parent_)
		{
			active = false;
			parent = parent_;
			logger = 0;
			chartScripts = new managedList<ChartScript>;
			paramManager = 0; //new ParamManager(parent_);
			symbolManager = 0; //new SymbolManager(parent_);
			orderManager = 0; //new OrderManager(parent_);
			executionHandler = 0;
			portfolio = 0;
		}

		virtual ~TradingSessionImpl()
		{

			delete chartScripts;

			if (executionHandler)
				delete executionHandler;
			if (portfolio)
				delete portfolio;
			if (orderManager) 
				delete orderManager;
			if (symbolManager)
				delete symbolManager;
			if (paramManager)
				delete paramManager;
			if (logger)
				delete logger;
			if (dataStore)
				delete dataStore;

		}

		int state(){return active;}

		void checkCanStart()
		{
			checkNotStarted();

			if (!orderManager)
				throw ETradingSession ( std::string("orderManager is NULL" AT_FILE_LINE) );
			if (!symbolManager)
				throw ETradingSession ( std::string("symbolManager is NULL" AT_FILE_LINE) );
			if (!paramManager)
				throw ETradingSession ( std::string("paramManager is NULL" AT_FILE_LINE) );
			if (!logger)
				throw ETradingSession ( std::string("logger is NULL" AT_FILE_LINE) );
			if (!executionHandler)
				throw ETradingSession ( std::string("executionHandler is NULL" AT_FILE_LINE) );
			if (!portfolio)
				throw ETradingSession ( std::string("portfolio is NULL" AT_FILE_LINE) );
			if (!dataStore)
				throw ETradingSession ( std::string("dataStore is NULL" AT_FILE_LINE) );


		}

		void checkNotStarted()
		{
			if (active)
				//		_RAISE_EXCEPTION(ETradingSession, "Session started");
				throw ETradingSession ( std::string("Session active" AT_FILE_LINE) );
		}

		void checkStarted()
		{
			if (!active)
				//		_RAISE_EXCEPTION(ETradingSession, "Session started");
				throw ETradingSession ( std::string("Session not active" AT_FILE_LINE) );
		}

	public:
		OrderManager * orderManager;
		ExecutionHandler * executionHandler;
		//	ChartScriptFactoryManager * scriptManager;
		SymbolManager * symbolManager;
		ParamManager * paramManager;
		TradingSession * parent;
		Logger * logger;
		Portfolio * portfolio;

		managedList<TradingSessionObject> sessionObjects;
		managedList<ChartScript> * chartScripts;
		ChartScriptPtrIndex chartScriptIndexBySymbolID;
		ChartScriptPtrMap chartScriptIndexByRunnerID;
		//	ChartScriptPtrMap chartScriptIndexBySymbolID;
		bool active;
		DataStore * dataStore;
	};

	typedef TradingSessionImpl * PTradingSessionImpl;


	/////////////////////////////////////////////////////////
	//
	//  TradingSessionObject
	//
	//
	/////////////////////////////////////////////////////////

	TradingSessionObject::TradingSessionObject(TradingSession * session_)
	{
		session = 0;

		setSession(session_);
	}

	void TradingSessionObject::setSession(TradingSession * session_)
	{
		session = session_;
	}

	/////////////////////////////////////////////////////////
	//
	//  TradingSession
	//
	//
	/////////////////////////////////////////////////////////

	TradingSession::TradingSession()
	{
		TradingSessionImpl * impl = new TradingSessionImpl(this);
		pImpl = impl;
	}

	TradingSession::~TradingSession()
	{
		delete pImpl;
	}

	OrderManager * TradingSession::getOrderManager()
	{
		return dynamic_cast<PTradingSessionImpl>(pImpl)->orderManager;
	}


	SymbolManager * TradingSession::getSymbolManager()
	{
		return dynamic_cast<PTradingSessionImpl>(pImpl)->symbolManager;
	}

	ExecutionHandler * TradingSession::getExecutionHandler()
	{
		return dynamic_cast<PTradingSessionImpl>(pImpl)->executionHandler;
	}

	ParamManager * TradingSession::getParamManager()
	{
		return dynamic_cast<PTradingSessionImpl>(pImpl)->paramManager;
	}

	Logger * TradingSession::getLogger()
	{
		return dynamic_cast<PTradingSessionImpl>(pImpl)->logger;
	}

	Portfolio * TradingSession::getPortfolio()
	{
		PTradingSessionImpl impl = dynamic_cast<PTradingSessionImpl>(pImpl);
		
		return impl->portfolio;
	}

	void TradingSession::registerTradingSessionObject(TradingSessionObject * obj)
	{
		dynamic_cast<PTradingSessionImpl>(pImpl)->sessionObjects.push_back(obj);
	}

	void TradingSession::unregisterTradingSessionObject(TradingSessionObject * obj)
	{
		dynamic_cast<PTradingSessionImpl>(pImpl)->sessionObjects.remove(obj);
	}

	void TradingSession::setOrderManager(OrderManager * orderManager)
	{
		PTradingSessionImpl impl = dynamic_cast<PTradingSessionImpl>(pImpl);

		impl->checkNotStarted();
		impl->orderManager = orderManager;
	}

	void TradingSession::setExecutionHandler(ExecutionHandler * executionHandler)
	{
		PTradingSessionImpl impl = dynamic_cast<PTradingSessionImpl>(pImpl);

		impl->checkNotStarted();
		impl->executionHandler = executionHandler;
	}


	void TradingSession::setSymbolManager(indicator::SymbolManager * symbolManager)
	{
		PTradingSessionImpl impl = dynamic_cast<PTradingSessionImpl>(pImpl);

		impl->checkNotStarted();
		impl->symbolManager = symbolManager;
	}


	void TradingSession::setParamManager(ParamManager * paramManager)
	{
		PTradingSessionImpl impl = dynamic_cast<PTradingSessionImpl>(pImpl);

		impl->checkNotStarted();
		impl->paramManager = paramManager;
	}


	void TradingSession::setLogger(Logger * logger)
	{
		PTradingSessionImpl impl = dynamic_cast<PTradingSessionImpl>(pImpl);

		impl->checkNotStarted();
		logger->setSession(this);
		impl->logger = logger;
	}

	void TradingSession::setPortfolio(Portfolio * portfolio)
	{
		PTradingSessionImpl impl = dynamic_cast<PTradingSessionImpl>(pImpl);

		impl->checkNotStarted();
		impl->portfolio = portfolio;
	}

	void TradingSession::start()
	{
		PTradingSessionImpl impl = dynamic_cast<PTradingSessionImpl>(pImpl);
		impl->checkCanStart();
		impl->dataStore->connect();
		TradingSessionManager::Instance().onNewSession(this);
		impl->portfolio->start();
		impl->executionHandler->start();
		impl->active = true;
		

		for (
			std::vector<ChartScript*>::iterator it = impl->chartScripts->list.begin();
			it != impl->chartScripts->list.end(); it++)
		{
			if ( (*it)->hasRunnerOptions(ChartScript::CSO_AUTOSTART) )
				(*it)->start();
		}	
	}

	void TradingSession::stop()
	{
		PTradingSessionImpl impl = dynamic_cast<PTradingSessionImpl>(pImpl);

		impl->portfolio->stop();

		impl->active = false;
	}


	bool TradingSession::active()
	{
		return dynamic_cast<PTradingSessionImpl>(pImpl)->active;
	}

	void TradingSession::insertScript(ChartScript * script)
	{
		if (getScript(script->getInstanceID()))
			return;

		PTradingSessionImpl impl = dynamic_cast<PTradingSessionImpl>(pImpl);
		impl->chartScripts->push_back(script);
		impl->chartScriptIndexBySymbolID[script->getSymbolID()].push_back(script);
		impl->chartScriptIndexByRunnerID[script->getInstanceID()] = script;
		//impl->chartScriptIndexBySymbolID[script->getSymbolID()] = script;
	}

	ChartScript * TradingSession::getScript(int RunnerID)
	{
		PTradingSessionImpl impl = dynamic_cast<PTradingSessionImpl>(pImpl);
		ChartScriptPtrMap::iterator it = impl->chartScriptIndexByRunnerID.find(RunnerID);
		return it == impl->chartScriptIndexByRunnerID.end() ? 0 : it->second;
	}

	void TradingSession::removeScript(int RunnerID)
	{
		PTradingSessionImpl impl = dynamic_cast<PTradingSessionImpl>(pImpl);
		ChartScriptPtrMap::iterator it = impl->chartScriptIndexByRunnerID.find(RunnerID);
		if (it != impl->chartScriptIndexByRunnerID.end())
		{
			ChartScriptPtrList & list = impl->chartScriptIndexBySymbolID[it->second->getSymbolID()];
			list.remove(it->second);
			impl->chartScripts->remove(it->second);
			impl->chartScriptIndexByRunnerID.erase(it);
		}
	}

	void TradingSession::updateTick(Tick * tick)
	{

		PTradingSessionImpl impl = dynamic_cast<PTradingSessionImpl>(pImpl);

		if (tick->ID < 0 || tick->bid < 0 || tick->ask < 0)
			return;

		// An event might be in place - although this might be more efficient(faster)
		impl->symbolManager->updateTick( tick );
		impl->portfolio->Update( tick );
		impl->executionHandler->Update( tick );
		impl->orderManager->Update(	tick );

		ChartScriptPtrList & list = impl->chartScriptIndexBySymbolID[tick->symbolID];
		for ( ChartScriptPtrList::iterator it = list.begin(); it != list.end(); it++)
		{
			(*it)->updateTick(tick);
			(*it)->onOrderManagerUpdated(tick);
		}

		/*
		for (
		std::vector<ChartScript*>::iterator it = impl->chartScripts->list.begin();
		it != impl->chartScripts->list.end(); it++)
		{
		}
		*/


	}

	void TradingSession::onChartScriptSymbolIDChanged(ChartScript *sender, int oldID, int newID)
	{
		PTradingSessionImpl impl = dynamic_cast<PTradingSessionImpl>(pImpl);

		ChartScriptPtrList & listOld = impl->chartScriptIndexBySymbolID[oldID];

		listOld.remove(sender);

		ChartScriptPtrList & listNew = impl->chartScriptIndexBySymbolID[newID];

		listNew.push_back(sender);

	}


	DataStore * TradingSession::getDataStore()
	{
		return dynamic_cast<PTradingSessionImpl>(pImpl)->dataStore;
	}
	void TradingSession::setDataStore(DataStore * dataStore)
	{
		dynamic_cast<PTradingSessionImpl>(pImpl)->dataStore = dataStore;
	}



	/////////////////////////////////////////////////////////
	//
	//  DefaultTradingSession
	//
	//
	/////////////////////////////////////////////////////////

	DefaultTradingSession::DefaultTradingSession()
		: TradingSession()
	{	
		PTradingSessionImpl impl = dynamic_cast<PTradingSessionImpl>(pImpl);

		impl->paramManager = new ParamManager(this);
		impl->symbolManager = new SymbolManager(this);
		impl->orderManager = new OrderManager(this);
		impl->executionHandler = NULL;	
	}

	/////////////////////////////////////////////////////////
	//
	//  CustomTradingSession
	//
	//
	/////////////////////////////////////////////////////////

	CustomTradingSession::CustomTradingSession(
		OrderManager * orderManager_,
		ExecutionHandler * executionHandler,
		ParamManager * paramManager_, 
		SymbolManager * symbolManager_
		)  : TradingSession()
	{

		PTradingSessionImpl impl = dynamic_cast<PTradingSessionImpl>(pImpl);

		impl->paramManager = 
			paramManager_ ? 
paramManager_ : 
		new ParamManager(this);

		impl->executionHandler =
			executionHandler;

		impl->symbolManager = 
			symbolManager_  ? 
symbolManager_  : 
		new SymbolManager(this);

		impl->orderManager = 
			orderManager_ ? 
orderManager_ : 
		new OrderManager(this);

		// must supply a portfolio manager
		// currently: InMem and MySQL versions
	}



	/////////////////////////////////////////////////////////
	//
	//  TradingSessionManager
	//
	//
	/////////////////////////////////////////////////////////


	TradingSessionManager & TradingSessionManager::Instance()
	{
		static TradingSessionManager instance;

		return instance;
	}

	void TradingSessionManager::onNewSession(TradingSession * session_)
	{
		for (
			NewSessionEventList::const_iterator it = newSessionEventList.begin();
			it != newSessionEventList.end(); 
		it++)
		{
			(*it)->OnNewSession(session_);
		}
	}

	void TradingSessionManager::registerNewSessionCallback(INewSessionEvent * ev)
	{
		newSessionEventList.push_back(ev);
	}





}