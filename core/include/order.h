#ifndef _ORDER_H
#define _ORDER_H

#ifndef _INDICATOR_H
#error This file must be included by <indicator.h>
#endif

namespace indicator
{

#define ORDER_SIZE_AUTO	-1
#define ORDER_SIZE_UPDATE_STOP  -2


	class IOrderExecuteEventSink
	{
	public:
		virtual void onOrderExecute(TradingObject * source, Order * order) = 0;
	};
	typedef IOrderExecuteEventSink * PIOrderExecuteEventSink;
	typedef SinkList<TradingObject, IOrderExecuteEventSink, Order>
		OrderExecuteSinkList;

	class IOrderChangeEventSink
	{
	public:
		virtual void onOrderChange(TradingObject * source, Order * order) = 0;
	};
	typedef IOrderChangeEventSink * PIOrderChangeEventSink;
	typedef SinkList<TradingObject, IOrderChangeEventSink, Order> 
		OrderChangeSinkList;
	//	typedef std::list<PIOrderChangeEventSink> OrderChangeSinkList;

	class IOrderSubmitEventSink
	{
	public:
		virtual void onOrderSubmit(TradingObject * source, Order * order) = 0;
	};
	typedef IOrderSubmitEventSink * PIOrderSubmitEventSink;
	typedef SinkList<TradingObject, IOrderSubmitEventSink, Order> 
		OrderSubmitSinkList;


	struct Order : public TradingObject
	{
		Order(TradingSession * session_, int existingID = -1) :
		session(session_),
		onOrderChangeSinkList(&IOrderChangeEventSink::onOrderChange),
		onPositionChangeSinkList(&IPositionChangeEventSink::onPositionChange),
		onOrderExecuteSinkList(&IOrderExecuteEventSink::onOrderExecute)
	{
		// needs to be initialized and saved to/from a file
		if (existingID == -1)
			orderID = newOrderID();
		symbolID = -1;
		type = OT_Unknown;
		size = -1;
		limit = OL_Unknown;
		limitPrice = -1;
		status = OS_Open;				
		calculateExposure = NULL;
		risk = -1;
	}

	Order(const Order & o) :
		onOrderChangeSinkList(o.onOrderChangeSinkList),
		onPositionChangeSinkList(o.onPositionChangeSinkList),
		onOrderExecuteSinkList(o.onOrderExecuteSinkList)
	{
		orderID = o.orderID;
		session = o.session;
		symbolID = o.symbolID;
		type = o.type;
		limit = o.limit;
		limitPrice = o.limitPrice;
		currentBid = o.currentBid;
		currentAsk = o.currentAsk;
		currentTickID = o.currentTickID;
		size = o.size;
		status = o.status;
		tickSize = o.tickSize;
		calculateExposure = o.calculateExposure;
		tradeInfo = o.tradeInfo;
		risk = o.risk;
	}

	int newOrderID();

	void update(Tick * tick)
	{
		currentBid = tick ->bid;
		currentAsk = tick ->ask;
		currentTickID = tick->ID;
	}

	int orderID;
	int symbolID;
	OrderType type;
	OrderLimit limit;
	double limitPrice;
	//double currentPrice;
	double currentBid;
	double currentAsk;
	double risk;
	int currentTickID;
	double size;
	OrderStatus status;
	double tickSize;
	// these are the events that will be transfered to the newly created position
	PositionChangeSinkList onPositionChangeSinkList;
	// These are the events that are sent from the order manager
	OrderChangeSinkList onOrderChangeSinkList;
	// These are the events that are sent from the execution manager
	OrderExecuteSinkList onOrderExecuteSinkList;
	// we can send in a exposure calculation rig
	IOnCalculateExposure * calculateExposure;
	TradeInfo tradeInfo;
	TradingSession * session;
	};

	typedef Order * OrderPtr;
	typedef managedList<Order> OrderList;
	typedef std::list<Order *> TmpOrderList;
	typedef std::map<int, OrderPtr> OrderMapIndex;
}

#endif