#ifndef _ORDER_MANAGER_H
#define _ORDER_MANAGER_H

#ifndef _INDICATOR_H
#error This file must be included by <indicator.h>
#endif

//#include "multikey.h"


namespace indicator
{

	class IOnCalculateExposure	{
	public:
			virtual void onCalculateExposure(TradingObject * source, double & exposure_) = 0;
	};
	typedef IOnCalculateExposure * PIOnCalculateExposure;

	class IOnTradingSuspended{
	public:
			virtual void onTradingSuspended () = 0;
	};

	class IOnTradingResumed{
	public:
			virtual void onTradingResumed() = 0;
	};


	class OrderManager : public TradingObject
	{ 
		friend class PortfolioHistoryLogger;
	protected:

	public:

		enum RiskMode
		{
			RM_ViaOrder = 0,
			RM_VolatilityAdustedPercentOfEquity = 1,
			RM_PercentOfEquity = 2
		};


	public:
//		static OrderManager & Instance();
		OrderManager(TradingSession * session_);
		virtual ~OrderManager();
		
		void Update(const Tick * tick);  


		Order * submitOrder(Order * order);

		Order * applyRisk(Order * order);

		Order * createOrder(
			int SymbolID, 
			OrderType type, 
			IOrderChangeEventSink * onOrderChange = 0,
			IPositionChangeEventSink * onPositionChange = 0
		);

		Order * createOrder(
			int SymbolID, 
			OrderType type, 
			double size,
			IOrderChangeEventSink * onOrderChange = 0,
			IPositionChangeEventSink * onPositionChange = 0
		);

/*		int Buy(
			int symbolID, 
			int shares = ORDER_SIZE_AUTO, 
			OrderLimit = OL_Market, 
			double limitPrice = 0, 
			PIPositionUpdateEventSink sink = 0
		);
		int Buy(int symbolID, PIPositionUpdateEventSink sink)
		{
			return Buy(symbolID,ORDER_SIZE_AUTO,OL_Market, 0,sink); 
		}
		int Sell(
			int symbolID, 
			int shares = ORDER_SIZE_AUTO, 
			OrderLimit = OL_Market, 
			double limitPrice = 0, 
			PIPositionUpdateEventSink sink = 0
		);

		int Sell(int symbolID, PIPositionUpdateEventSink sink)
		{
			return Sell(symbolID,ORDER_SIZE_AUTO,OL_Market, 0,sink); 
		}

		int Short(
			int symbolID, 
			int shares = ORDER_SIZE_AUTO, 
			OrderLimit = OL_Market, 
			double limitPrice = 0, 
			PIPositionUpdateEventSink sink = 0
		);
		int Short(int symbolID, PIPositionUpdateEventSink sink)
		{
			return Short(symbolID,ORDER_SIZE_AUTO,OL_Market, 0,sink); 
		}

		int Cover(
			int symbolID, 
			int shares = ORDER_SIZE_AUTO, 
			OrderLimit = OL_Market, 
			double limitPrice = 0, 
			PIPositionUpdateEventSink sink = 0
		);
		int Cover(int symbolID, PIPositionUpdateEventSink sink)
		{
			return Cover(symbolID,ORDER_SIZE_AUTO,OL_Market, 0,sink); 
		}
*/

		virtual void registerTradingSuspendedSink(IOnTradingSuspended * sink);
		virtual void unregisterTradingSuspendedSink(IOnTradingSuspended * sink);
		virtual void registerTradingResumedSink(IOnTradingResumed * sink);
		virtual void unregisterTradingResumedSink(IOnTradingResumed * sink);
		virtual bool tradingSuspended();
		virtual void suspendTrading();
		virtual void resumeTrading();

		void CancelOrders(int symbolID);
		void CancelOrder(int orderID);
		void UpdateOrderLimit(int orderID, double NewStopOrLimit);

		void registerTradeEventSink(IOrderChangeEventSink * sink);
		void unregisterTradeEventSink(IOrderChangeEventSink * sink);

		void registerOrderSubmitEventSink(IOrderSubmitEventSink * sink);
		void unregisterOrderSubmitEventSink(IOrderSubmitEventSink * sink);

		
	protected:


		PImpl * pImpl;
	};

}

#endif