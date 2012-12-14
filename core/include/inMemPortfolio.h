#ifndef _MEMORY_PORTFOLIO_H
#define _MEMORY_PORTFOLIO_H

namespace indicator
{

	class InMemPortfolio :
		public Portfolio
	{
	public:
		InMemPortfolio (TradingSession * session_);
		~InMemPortfolio ();
		
		void onParamsChange();

		void processTrade( Order * order );

		void Update( const Tick * tick );

		int positionCount();
		const Position * getPosition(int idx);
		PositionList & getPositionBySymbol(int symbolID);
		double getPositionSizeBySymbol(int symbolID);

		void CloseAllPositions();

		DateTime lastTradeTime();

		void start();
		void stop();

		double openValue();		
		double cashValue();
		void setCashValue(double val);
		double getStartingCash();
		
		void registerPortfolioChangeSink( IPortfolioChangeEventSink * sink);
		void unregisterPortfolioChangeSink( IPortfolioChangeEventSink * sink);

		void onOrderExecute(indicator::TradingObject *obj,indicator::Order *order);

	private:
		PImpl * pImpl;
	};



}


#endif