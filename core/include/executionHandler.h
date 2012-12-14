#ifndef _INDICATOR_H
#error This file must be included by <indicator.h>
#endif

#ifndef _EXECUTION_HANDLER_H
#define _EXECUTION_HANDLER_H


namespace indicator
{

///////////////////////////////////
///
///   Execution Handler
///
///////////////////////////////////

	class ExecutionHandler : 
		public TradingObject,
		public IParamChangeEventSink
	{
	public:
		ExecutionHandler(TradingSession * session_ = NULL);
		virtual ~ExecutionHandler();

		virtual void submitOrder( Order * order) = 0;
		virtual void cancelOrder(int orderID) = 0;
		virtual void cancelAllOpenOrders() = 0;
		virtual void changeOrder(Order * order) = 0;

		virtual void start();
		virtual void onExecute(const TradeInfo & tradeInfo) = 0;
		virtual void onParamsChange(){}

		virtual bool orderExists(const int orderID) = 0;
		virtual int openOrderCount() = 0;
		virtual const Order * getOrderByIndex(const int idx) = 0;
		virtual const Order * getOrderByID(const int orderID) = 0;
		virtual const int indexOfOrder(const int orderID) = 0;
		inline const Order * operator[](const int idx){return getOrderByIndex(idx);}
		
		virtual void Update(const Tick * tick) = 0; 

//		virtual void registerExecuteEventSink(IOrderExecuteEventSink * sink) = 0;
//		virtual void unregisterExecuteEventSink(IOrderExecuteEventSink * sink) = 0;


	protected:
		TradingSession * session;
	};

}

#endif
