#ifndef _EXECUTION_SIMULATOR_H
#define _EXECUTION_SIMULATOR_H

namespace indicator
{

	class ExecutionSimulator : 
		public ExecutionHandler
	{
		public:
			ExecutionSimulator(TradingSession * session_);
			virtual ~ExecutionSimulator();

			void submitOrder( Order * order);
			void onExecute(const TradeInfo & tradeInfo);
			void onParamsChange();

			int openOrderCount();


			void cancelOrder(int orderID);
			void cancelAllOpenOrders();
			void changeOrder(Order * order);

			bool orderExists(const int orderID);
			const Order * getOrderByIndex(const int idx);
			const Order * getOrderByID(const int orderID);
			const int indexOfOrder(const int orderID);

			// specific for the simulator
			void Update(const Tick * tick);  

			void registerExecuteEventSink(IOrderExecuteEventSink * sink);
			void unregisterExecuteEventSink(IOrderExecuteEventSink * sink);

		private:

			PImpl * pImpl;
				
	
	};

}

#endif