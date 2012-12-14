#include "indicator.h"
#include "fastjulian.h"

namespace indicator
{
	class TradeLogger : 
		public IOrderExecuteEventSink,
		public IOrderSubmitEventSink,
		public TradingSessionObject 
	{
	public: 
		TradeLogger(indicator::TradingSession * session_)
			: indicator::TradingSessionObject(session_)
		{
			session->getOrderManager()->registerOrderSubmitEventSink(this);

			logger = new indicator::FileLogger(session, new indicator::OnFileLoggerStartEvent("trades"));

			avgPctPerTrade = -100000;
	 		volPerTrade = 0; // we start with a high value 
			avgOfVolPerTrade = 0;
			sdOfVolPerTrade = 0.005;
			avgPnLPerTrade = -100000;
			logger->enable();
		}

		virtual ~TradeLogger()
		{
			delete logger;
		}
	
		void onOrderSubmit(TradingObject * source, Order * order)
		{
			order->onOrderExecuteSinkList.addSink(this);
		}

		void onOrderExecute(TradingObject * source, Order * order)
		{	
			string orderType;
			TradeInfo & tradeInfo = order->tradeInfo;

			switch (tradeInfo.type)
			{
				case OT_BuyLong : orderType = "BUY_LONG"; break;
				case OT_SellShort : orderType = "SELL_SHORT"; break;
				case OT_ExitLong : orderType = "EXIT_LONG"; break;
				case OT_CoverShort : orderType = "COVER_SHORT"; break;
			}
			
			if (tradeInfo.type == OT_BuyLong || tradeInfo.type == OT_SellShort)
			{
				LOG(
					indicator::LT_DEBUG, "Order","FILLED %s '%d' %d X %d @%s (%d)",
					orderType,
					session->getSymbolManager()->getSymbolName(tradeInfo.symbolID), 
					tradeInfo.size, 
					tradeInfo.executionPrice, 
					fastjulian::formatOleDateTime(tradeInfo.tradeTime), 
					tradeInfo.tickID				
				);
			} 
			else
			{
				avgPctPerTrade = avgPctPerTrade < -99999 ? tradeInfo.PnLpct : (20 * avgPctPerTrade + tradeInfo.PnLpct)/21.0;
				avgPnLPerTrade = avgPnLPerTrade < -99999 ? 
					tradeInfo.PnLinTicksPerUnit : (20 * avgPnLPerTrade + tradeInfo.PnLinTicksPerUnit) / 21.0;
				double delta = tradeInfo.PnLpct - avgPctPerTrade;
				volPerTrade = volPerTrade == 0? delta*delta : 
					(20*volPerTrade + delta*delta)/21.0 ; 


				LOG(indicator::LT_DEBUG,"Order",
					"FILLED %s '%d' %d X %d @%s(%d) (%2.2f%% | %d). avg is %2.3f%% (%d) volatiliy is %2.2f%%",
					orderType,
					session->getSymbolManager()->getSymbolName(tradeInfo.symbolID), 
					tradeInfo.size, 
					tradeInfo.executionPrice, 
					fastjulian::formatOleDateTime(tradeInfo.tradeTime),
					tradeInfo.tickID,
					100*(tradeInfo.PnLpct-1),
					tradeInfo.PnLinTicksPerUnit,
					100*(avgPctPerTrade-1),
					avgPnLPerTrade,
					100*sqrt(volPerTrade)
				);
			}
		}

	private:
		double avgPctPerTrade;
		double avgPnLPerTrade;
		double volPerTrade; // == stddev(pctpertrade)
		double avgOfVolPerTrade;
		double sdOfVolPerTrade;

		indicator::Logger * logger;
	};
	
	

	class TradeCsvLogger : 
		public IOrderExecuteEventSink, 
		public IOrderSubmitEventSink,
		public TradingSessionObject,
		public IPortfolioChangeEventSink
	{
	public: 
		TradeCsvLogger(indicator::TradingSession * session_)
			: indicator::TradingSessionObject(session_)
		{
			logger = new indicator::FileLogger(session, new indicator::OnFileLoggerStartEvent("trades", ".csv"));

			session->getPortfolio()->registerPortfolioChangeSink(this);
			session->getOrderManager()->registerOrderSubmitEventSink(this);


			avgPctPerTrade = -100000;
			volPerTrade = 0; // we start with a high value 
			avgOfVolPerTrade = 0;
			sdOfVolPerTrade = 0.005;
			avgPnLPerTrade = -100000;
			logger->enable();
			init = false;
		}
	
		virtual ~TradeCsvLogger()
		{
			delete logger;
		}

		void onPortfolioChange(Portfolio * portfolio, PortfolioInfo * info)
		{
			currentInvested = info->investedVal;
			currentCash = info->cashVal;
			currentPortfolioVal = info->investedVal + info->cashVal ;
		}

		void onOrderSubmit(TradingObject * source, Order * order)
		{
			order->onOrderExecuteSinkList.addSink(this);
		}

		void onOrderExecute(TradingObject * source, Order * order)
		{	
			TradeInfo & tradeInfo = order->tradeInfo;
			string orderType;

			if (!init)
			{
				init = true;
				LOG(indicator::LT_NONE,"Order",
				"stype,orderType,symbolID,Jentrydate,entrydate,Jexitdate,exitdate,tickID,size,avgEntryPrice,exitPrice,PnLpct,PnLperTickPerUnit,PnL,MAE,MPE,Days,Invested,Cash,Total");
			}

			switch (tradeInfo.type)
			{
				case OT_BuyLong : orderType = "BUY_LONG"; break;
				case OT_SellShort : orderType = "SELL_SHORT"; break;
				case OT_ExitLong : orderType = "EXIT_LONG"; break;
				case OT_CoverShort : orderType = "COVER_SHORT"; break;
			}
			
			if (tradeInfo.type == OT_BuyLong || tradeInfo.type == OT_SellShort)
			{
			}
			else
			{
				avgPctPerTrade = avgPctPerTrade < -99999 ? tradeInfo.PnLpct : (20 * avgPctPerTrade + tradeInfo.PnLpct)/21.0;
				avgPnLPerTrade = avgPnLPerTrade < -99999 ? 
					tradeInfo.PnLinTicksPerUnit : (20 * avgPnLPerTrade + tradeInfo.PnLinTicksPerUnit) / 21.0;
				double delta = tradeInfo.PnLpct - avgPctPerTrade;
				volPerTrade = volPerTrade == 0? delta*delta : 
					(20*volPerTrade + delta*delta)/21.0 ; 


				LOG(indicator::LT_NONE,"",
					"%s,%d,%d,%f,%s,%f,%s,%f,%d,%f,%f,%2.5f,%f,%f,%f,%f,%f,%f,%f,%f",
					orderType,
					tradeInfo.type,
					tradeInfo.symbolID, 
					tradeInfo.openTime,
					fastjulian::formatOleDateTime(tradeInfo.openTime),
					tradeInfo.tradeTime,
					fastjulian::formatOleDateTime(tradeInfo.tradeTime),
					tradeInfo.tickID,
					tradeInfo.size, 
					tradeInfo.entryPrice,
					tradeInfo.executionPrice, 
					100*(tradeInfo.PnLpct-1),
					tradeInfo.PnLinTicksPerUnit,
					tradeInfo.PnL,
					tradeInfo.MAE/(tradeInfo.size*tradeInfo.tickSize),
					tradeInfo.MPE/(tradeInfo.size*tradeInfo.tickSize),
					tradeInfo.tradeTime - tradeInfo.openTime,
					currentInvested,
					currentCash,
					currentPortfolioVal
				);
			}
		}
	private:
		double avgPctPerTrade;
		double avgPnLPerTrade;
		double volPerTrade; // == stddev(pctpertrade)
		double avgOfVolPerTrade;
		double sdOfVolPerTrade;
		double currentInvested;
		double currentCash;
		double currentPortfolioVal;
		bool init;

		indicator::Logger * logger;
	};


	//static TradeCsvLogger tradeCsvLogger;
	void orderLoggerInit()
	{
			static indicator::SessionObjectFactory<TradeCsvLogger> TradeCsvLoggerFactory;
			static indicator::SessionObjectFactory<TradeLogger> tradeLoggerFactory;
	}
	
}