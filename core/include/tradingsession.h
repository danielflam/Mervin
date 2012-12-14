#ifndef _TRADING_SESSION_H
#define _TRADING_SESSION_H

#ifndef _INDICATOR_H
#error This file must be included by <indicator.h>
#endif


namespace indicator {

//	class ETradingSession : public std::runtime_error {};
	typedef std::runtime_error ETradingSession;


	class TradingSession : public TradingObject
	{
	public:
		TradingSession();

		// enables supplying a custom logger

		virtual ~TradingSession();

		virtual OrderManager * getOrderManager();
		virtual ExecutionHandler * getExecutionHandler();
		virtual SymbolManager * getSymbolManager(); 
		virtual ParamManager * getParamManager();
		virtual Logger * getLogger();
		virtual Portfolio * getPortfolio();
		virtual DataStore * getDataStore();

		virtual void setOrderManager(OrderManager * orderManage);
		virtual void setExecutionHandler(ExecutionHandler * executionHandler);
		virtual void setSymbolManager(indicator::SymbolManager * symbolManager); 
		virtual void setParamManager(ParamManager * paramManager);
		virtual void setLogger(Logger * logger);
		virtual void setPortfolio(Portfolio * portfolio);
		virtual void setDataStore(DataStore * store);

		virtual void start();
		virtual void stop();
		virtual bool active();

		void registerTradingSessionObject(TradingSessionObject * obj);
		void unregisterTradingSessionObject(TradingSessionObject * obj);

		void insertScript(ChartScript * script);
		ChartScript * getScript(int RunnerID);
		void removeScript(int RunnerID);

		void updateTick(Tick * tick);
		

	protected:
		friend void ChartScript::setSymbolID(int ID);
		virtual void onChartScriptSymbolIDChanged(ChartScript *sender, int oldID, int newID);


		PImpl * pImpl;

	};


	class INewSessionEvent
	{
	public:
		virtual void OnNewSession(TradingSession * session_) = 0;
	};


	typedef INewSessionEvent * PINewSessionEvent;
	typedef std::list<PINewSessionEvent> NewSessionEventList;

	class TradingSessionManager
	{
		friend class TradingSession;
	private:
		TradingSessionManager(){}

	public:

		static TradingSessionManager & Instance();

		void registerNewSessionCallback(INewSessionEvent * ev);


	protected:
		void onNewSession(TradingSession * session_);

	protected:
		NewSessionEventList newSessionEventList;
	};

	// Creates a child object when session is started

	template <class T> 
	class SessionObjectFactory : public INewSessionEvent
	{
	public:
		SessionObjectFactory()
		{
			TradingSessionManager::Instance().registerNewSessionCallback(this);
		}

		void OnNewSession(indicator::TradingSession * session_)
		{
			session_->registerTradingSessionObject(new T(session_));
		}
	};

	class DefaultTradingSession : public TradingSession
	{
	public:
		DefaultTradingSession();


	};

	class CustomTradingSession : public TradingSession
	{
	public:
		CustomTradingSession(
			OrderManager * orderManager_ = NULL,
			ExecutionHandler * executionHandler = NULL,
			ParamManager * paramManager_ = NULL, 
			SymbolManager * symbolManager_ = NULL
		);
	};


}


#endif