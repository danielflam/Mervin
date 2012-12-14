#ifndef _PORTFOLIO_H
#define _PORTFOLIO_H

#ifndef _INDICATOR_H
#error This file must be included by <indicator.h>
#endif

namespace indicator
{

	struct PortfolioInfo
	{
		double whenDate;
		double investedVal;
		double cashVal;
		bool lastOne;
	};

	class IPortfolioChangeEventSink
	{
	public:
		virtual void onPortfolioChange(Portfolio * portfolio, PortfolioInfo * info) = 0;
	};

/*
	class IUpdatePortfolioEventSink
	{
	public:
		virtual void onUpdatePortfolio(OrderManager * orderManager, 
			double WhenDate, 
			double investedVal, 
			double cashVal, 
			bool lasttick
		) = 0;

	};
	typedef IUpdatePortfolioEventSink * PIUpdatePortfolioEventSink;

	typedef std::list<PIUpdatePortfolioEventSink> UpdatePortfolioEventSinkList;
*/

	class Portfolio : 
		public TradingObject,
		public IParamChangeEventSink,
		public IOrderExecuteEventSink
	{
	public:
		Portfolio(TradingSession * session_) : session(session_){}
		virtual ~Portfolio(){}

		virtual void processTrade( Order * order ) = 0;

		virtual void start( ) = 0;
		virtual void stop( ) = 0;

		virtual DateTime lastTradeTime() = 0;

		virtual void Update( const Tick * tick ) = 0;
		virtual int positionCount() = 0;
		const virtual Position * getPosition(int idx) = 0;
		virtual PositionList & getPositionBySymbol(int symbolID) = 0;
		virtual double getPositionSizeBySymbol(int symbolID) = 0;
		virtual double openValue() = 0;		
		virtual double cashValue() = 0;
		virtual void setCashValue(double val) = 0;
		virtual double getStartingCash() = 0;

		virtual void registerPortfolioChangeSink( IPortfolioChangeEventSink * ) = 0;
		virtual void unregisterPortfolioChangeSink( IPortfolioChangeEventSink * ) = 0;

		virtual void onOrderExecute(indicator::TradingObject *obj,indicator::Order *order) = 0;
		
	protected:
		TradingSession * session;

	};


}
#endif