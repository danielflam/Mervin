#ifndef CHARTSCRIPT_H
#define CHARTSCRIPT_H

#ifndef _INDICATOR_H
#error This file must be included by <indicator.h>
#endif


namespace indicator
{


//////////////////////////////////////////////
//
//  ChartScript
//
//////////////////////////////////////////////

	// derive from this class to run the script
	class ChartScript : 
		public boost::noncopyable, 
		public TradingSessionObject, 
		public IParamChangeEventSink
	{
		// allow sharing of the data baseline data
		friend class IndicatorSeries;
		friend class ChartScriptFactoryManager;
	protected:
		// only runner can instantiate it!
		ChartScript(TradingSession * session,  ChartScriptFactoryManager & parent);
		
	public:
		enum CalcStatus {
			CS_OK = 0, 
			CS_ERROR = 1,
			CS_SKIP = 2
		};

		enum ChartScriptOptions { 
			CSO_NONE=0, 
			CSO_AUTOSTART=1, 
			CSO_AUTOSTOP=2,
			CSO_DEFAULT=3
		};

	public:

		virtual ~ChartScript();

		// override update to implement the strategy
		
		void start();
		void stop();
		void reset();
		bool active();

		bool enabled();
		void enable();
		void disable();


		void updateTick(Tick * tick);
		void onOrderManagerUpdated(Tick * tick);

		int getInstanceID();
		int getSymbolID();
		void setSymbolID(int ID); 
		string getSymbol();
		void setSymbol(string s);

		
		ChartScriptOptions getRunnerOptions();
		bool hasRunnerOptions(ChartScriptOptions options);
		void setRunnerOptions(ChartScriptOptions options);
		
		Tick * getCurBar(){return &curBar;}
		Tick * getPrevBar(){return &prevBar;}

		void onParamsChange();

	protected: 
		virtual void internalOnStart()=0;
		virtual void internalOnStop()=0;
		virtual void internalOnReset(){}
		virtual void internalUpdateTick(Tick * tick);
		virtual CalcStatus updateIndicators(Tick * tick) = 0;
		virtual void executeScript() = 0;
		virtual void onReadParams(ParamManager & paramManager) = 0;

		void registerIndicatorSeries(IndicatorSeries *);
		void unregisterIndicatorSeries(IndicatorSeries *);

		int lastbarID;
		int prevbarID;

		Tick curBar;
		Tick prevBar;

	private:
		PImpl * pImpl;

	};

	typedef ChartScript * ChartScriptPtr;

	typedef std::map<int, ChartScript *> ChartScriptPtrMap;
	typedef std::list<ChartScript *> ChartScriptPtrList;
	typedef std::map<int, ChartScriptPtrList> ChartScriptPtrIndex;
	


//////////////////////////////////////////////
//
//  ChartScriptFactory
//
//
//////////////////////////////////////////////

	//typedef boost::shared_ptr<ChartScript> ChartScriptPtr;
	//typedef std::map<int, ChartScriptPtr> ChartScriptMap;

	class ChartScriptFactoryBase
	{
	public:
		virtual ChartScript * create(TradingSession * session, ChartScriptFactoryManager &) = 0;
		virtual ~ChartScriptFactoryBase(){}
	};

	typedef boost::shared_ptr <ChartScriptFactoryBase> ChartScriptFactoryBasePtr;
	typedef std::map<string,ChartScriptFactoryBasePtr> ChartScriptFactoryMap;

	template <class T>
	class ChartScriptFactory : public ChartScriptFactoryBase
	{
	public:
		virtual ChartScript * create(TradingSession * session, ChartScriptFactoryManager & runner)
		{
			return new T(session, runner);
		}
		virtual ~ChartScriptFactory(){}
	};

}

#endif