//#include "tracetool.h"
#include "indicator.h"

//#include "ordermanager.h"
#include "symbolmanager.h"
//include "renko_params.h"
#include "fastjulian.h"

#define PARAM_ORDER_MANAGER_SECTION "order manager"
//#define PARAM_ORDER_MANAGER_STARTING_CASH "starting cash"
#define PARAM_ORDER_MANAGER_MAXCASHPERPOSITION "maximum pct cash per position"
#define PARAM_ORDER_MANAGER_MAXMARGIN "maximum margin"
#define PARAM_ORDER_MANAGER_MINORDERSIZE "minimum order size"
#define PARAM_ORDER_MANAGER_MAXKELLYRISK "maximum pct of kelly risk"
#define PARAM_ORDER_MANAGER_SHOULD_LOG "logging enabled"
#define PARAM_ORDER_MANAGER_RISK_MODE "risk mode"

namespace indicator
{
	using fastjulian::formatOleDateTime;

	void orderLoggerInit();
	void portfolioHistoryLoggerInit();

	class OrderManagerImpl : public PImpl,
		public IParamChangeEventSink,
		public IOrderExecuteEventSink
	{
	public:
		OrderManagerImpl(OrderManager * parent_, TradingSession * session_) :
		  parent( parent_ ),
			  session(session_),
			  onOrderChangeSinkList(&IOrderChangeEventSink::onOrderChange),
			  onOrderSubmitSinkList(&IOrderSubmitEventSink::onOrderSubmit),
			  onTradingSuspendedSinkList(&IOnTradingSuspended::onTradingSuspended),
			  onTradingResumedSinkList(&IOnTradingResumed::onTradingResumed)
		  {
			  tradingSuspended = false;
			  session->getParamManager()->registerSink(this);

			  logger = new FileLogger(session, new OnFileLoggerStartEvent("OrderManager"));
			  started = false;	

			  orderLoggerInit();
			  portfolioHistoryLoggerInit();

		  }

		  ~OrderManagerImpl()
		  {
			  session->getParamManager()->unregisterSink(this);

			  delete logger;

		  }

		  void init()
		  {
			  if (!started){
				  ParamManager & paramManager = *session->getParamManager();


				  bool shouldlog;
				  paramManager.getParam(-1, PARAM_ORDER_MANAGER_SECTION, PARAM_ORDER_MANAGER_SHOULD_LOG ,shouldlog , true);
				  logger->setStatus(shouldlog);

				  started = true;	
			  }
		  }

		  void onParamsChange()
		  {
			  ParamManager & paramManager = *session->getParamManager();

			  paramManager.getParamPct(-1, 
				  PARAM_ORDER_MANAGER_SECTION, 
				  PARAM_ORDER_MANAGER_MAXCASHPERPOSITION,
				  maxpositionpct,
				  97);


			  paramManager.getParam(-1, 
				  PARAM_ORDER_MANAGER_SECTION, 
				  PARAM_ORDER_MANAGER_MAXMARGIN,
				  margin ,
				  2.0);

			  paramManager.getParam(-1, 
				  PARAM_ORDER_MANAGER_SECTION, 
				  PARAM_ORDER_MANAGER_MINORDERSIZE,
				  minOrderSize ,
				  10000.0);

			  paramManager.getParam(-1, 
				  PARAM_ORDER_MANAGER_SECTION, 
				  PARAM_ORDER_MANAGER_RISK_MODE,
				  (int&)riskMode ,
				  0);

			  paramManager.getParamPct(-1, 
				  PARAM_ORDER_MANAGER_SECTION, 
				  PARAM_ORDER_MANAGER_MAXKELLYRISK ,
				  maxKellyRisk ,
				  100.0);


			  bool shouldlog;
			  paramManager.getParam(-1, PARAM_ORDER_MANAGER_SECTION, PARAM_ORDER_MANAGER_SHOULD_LOG ,shouldlog , true);
			  logger->setStatus(shouldlog);
		  }

		  void onOrderExecute(TradingObject *,Order * order)
		  {
			  // perhaps more bookeeping here?
			  // partial fills etc...
			  // we just pass this on to the caller
			  order->onOrderChangeSinkList.sendMessage(parent, order);
		  }

		  int state()
		  {
			  return 0;
		  }

		  Order * createOrder(
			  int SymbolID, 
			  OrderType type, 
			  IOrderChangeEventSink * onOrderChange,
			  IPositionChangeEventSink * onPositionChange
			  )
		  {
			  Order * order = new Order(session);

			  order->symbolID = SymbolID;
			  order->type = type;
			  order->limit = OL_Market;
			  order->limitPrice = 0;
			  order->size = -1;
			  if (onOrderChange)
				  order->onOrderChangeSinkList.addSink(onOrderChange);
			  if (onPositionChange)
				  order->onPositionChangeSinkList.addSink(onPositionChange);

			  return order;
		  }

		  Order * createOrder(
			  int SymbolID, 
			  OrderType type, 
			  double size,
			  IOrderChangeEventSink * onOrderChange,
			  IPositionChangeEventSink * onPositionChange
			  )
		  {
			  Order * order = new Order(session);

			  order->symbolID = SymbolID;
			  order->limit = OL_Market;
			  order->limitPrice = 0;
			  order->type = type;
			  order->size = size;
			  if (onOrderChange)
				  order->onOrderChangeSinkList.addSink(onOrderChange);
			  if (onPositionChange)
				  order->onPositionChangeSinkList.addSink(onPositionChange);

			  return order;
		  }


		  Order * applyRisk(Order * order)
		  {

			  if (order->size == -1)
			  {
				  Portfolio * portfolio = session->getPortfolio();

				  LOG(indicator::LT_DEBUG, "OrderManager","Automatic order size initiated...");

				  if (order->type == OT_BuyLong || order->type == OT_SellShort)
				  {
					  Tick & currTick = session->getSymbolManager()->getCurrentTick(order->symbolID);
					  double valueFactor = session->getSymbolManager()->getTickValueFactor(order->symbolID);

					  order->currentBid = currTick.bid;
					  order->currentAsk = currTick.ask;
					  order->currentTickID = currTick.ID;



					  double totalValue = portfolio->openValue() + portfolio->cashValue();
					  double totalMarginValue = margin * totalValue;
					  double maxInvestable = totalMarginValue -  portfolio->openValue();

					  double  exposure = 1.0;

					  if ( order->calculateExposure )  
					  {
						  order->calculateExposure->onCalculateExposure( parent, exposure );
					  }

					  double positionRisk;

					  switch (riskMode)
					  {
					  case OrderManager::RM_ViaOrder:
						  if (order->risk > 0)
						  {
							  positionRisk = maxKellyRisk*order->risk;
							  if (positionRisk  > maxpositionpct)
								  positionRisk = maxpositionpct;
						  }
						  else 
						  {
							  positionRisk = maxpositionpct;
						  }
						  break;

					  case OrderManager::RM_VolatilityAdustedPercentOfEquity:
						  {				
							  double symbolCount = double(session->getSymbolManager()->getSymbolCount());
							  double avgVol = session->getSymbolManager()->totalVolatility()/symbolCount;
							  double volFactor = avgVol/sqrt(currTick.volatility);
								
							  positionRisk = volFactor * maxpositionpct; 
							  //if (positionRisk  > maxpositionpct)
							  //{
							  //  positionRisk = maxpositionpct;
							  // }
						  }
						  break;

					  case OrderManager::RM_PercentOfEquity:
						  double symbolCount = double(session->getSymbolManager()->getSymbolCount());
						  positionRisk = maxpositionpct;
						  break;
					 }
					 
					 double maxPositionVal = exposure * positionRisk * totalMarginValue;
					 if ( maxPositionVal > maxInvestable )
						  maxPositionVal = maxInvestable;


					  order->size = order->type == OT_BuyLong ?
						  int(maxPositionVal/(order->currentAsk*valueFactor)) :
					  int(maxPositionVal/(order->currentBid*valueFactor) ) ;

					  LOG(indicator::LT_DEBUG, "OrderManager","%s@(tick %d) Calculating size: totalValue=%f, "
						  "openValue=%f, cashValue=%f,totalMarginValue=%f, margin=%f, maxInvestable=%f, exposure=%f"
						  ",maxPositionVal=%f,orderSize=%f",
						  session->getSymbolManager()->getSymbolName(order->symbolID),
						  order->currentTickID,
						  totalValue,portfolio->openValue(), portfolio->cashValue(),
						  totalMarginValue, margin, maxInvestable, exposure, maxPositionVal,order->size
						  );

					  if (order->size < 1)
					  {
						  // Log that the order has been closed!
						  LOG(indicator::LT_DEBUG, "OrderManager","%s@(tick %d): Could not calculate size - closing order",
							  session->getSymbolManager()->getSymbolName(order->symbolID),
							  order->currentTickID);

						  order->status = OS_Closed;
					  }
					  if (order->size < minOrderSize)
					  {
						  LOG(indicator::LT_DEBUG, "OrderManager","%s@(tick %d) Order size is %f < minimum "
							  "order size %f. closing.", 				  
							  session->getSymbolManager()->getSymbolName(order->symbolID),
							  order->currentTickID,
							  order->size , minOrderSize);
						  order->size = minOrderSize;
						  order->status = OS_Closed;
						  //						  return order;

					  }
					  else 
					  {
						  LOG(indicator::LT_DEBUG, "OrderManager","Order size is %f.", order->size);
					  }
				  }
				  else if (order->type == OT_ExitLong || order->type == OT_CoverShort)
				  {
					  Tick & currTick = session->getSymbolManager()->getCurrentTick(order->symbolID);

					  order->currentTickID = currTick.ID;

					  order->size = portfolio->getPositionSizeBySymbol(order->symbolID);
					  if (order->size < 1)
					  {
						  LOG(indicator::LT_DEBUG, "OrderManager","Order size is 0 - closing order");

						  order->status = OS_Closed;
						  //						  return order;
					  }

				  }
			  }

			  return order;
		  }

		  Order * submitOrder(Order * order)
		  {
			  if (order->status == OS_Closed)
			  {
				  LOG(indicator::LT_DEBUG, "OrderManager","%s@(tick %d) ERROR: attempting to submit a closed order. ", 							  session->getSymbolManager()->getSymbolName(order->symbolID),
					  order->currentTickID);

				  return order;
			  }

			  if (tradingSuspended)
			  {
				  LOG(indicator::LT_DEBUG, "OrderManager","%s@(tick %d) Trading Suspended - closing order. ", 		
					  session->getSymbolManager()->getSymbolName(order->symbolID),
					  order->currentTickID);
				  order->status = OS_Closed;
				  return order;
			  }
			  order->tickSize = session->getSymbolManager()->getTickSize(order->symbolID);

			  order->onOrderExecuteSinkList.addSink(session->getPortfolio());
			  order->onOrderExecuteSinkList.addSink(this);
			  order->onOrderChangeSinkList.append(onOrderChangeSinkList);

			  // allow clients to add handlers
			  onOrderSubmitSinkList.sendMessage(parent, order);

			  LOG(indicator::LT_DEBUG, "OrderManager","%s@(tick %d) Submitting order %d for execution: %f %f %f %f",
				  session->getSymbolManager()->getSymbolName(order->symbolID),
				  order->currentTickID,
				  order->orderID,
				  order->type,
				  order->size,
				  order->limit,
				  order->limitPrice);

			  session->getExecutionHandler()->submitOrder(order);

			  return order;
		  }

		  void Update(const Tick * tick)
		  {
			  init();


			  currentTime = tick->time;

		  }

		  void registerTradingSuspendedSink(IOnTradingSuspended * sink)
		  {
			  onTradingSuspendedSinkList.addSink(sink);
		  }

		  void unregisterTradingSuspendedSink(IOnTradingSuspended * sink)
		  {
			 onTradingSuspendedSinkList.removeSink(sink);
		  }

		  void registerTradingResumedSink(IOnTradingResumed * sink)
		  {
			  onTradingResumedSinkList.addSink(sink);
		  }

		  void unregisterTradingResumedSink(IOnTradingResumed * sink)
		  {
			 onTradingResumedSinkList.removeSink(sink);
		  }

	public:

		OrderManager * parent;
		TradingSession * session;

		bool started;

		bool tradingSuspended;

		double maxpositionpct;
		double margin;
		double maxKellyRisk;

		DateTime currentTime;
		SimpleSinkList<IOnTradingSuspended> onTradingSuspendedSinkList;
		SimpleSinkList<IOnTradingResumed> onTradingResumedSinkList;
		OrderChangeSinkList onOrderChangeSinkList;
		OrderSubmitSinkList onOrderSubmitSinkList;
		Logger * logger;
		double minOrderSize;
		OrderManager::RiskMode riskMode;
	};


	//////////////////////////////////
	//
	// OrderManager
	//
	//////////////////////////////////

	OrderManager::OrderManager(TradingSession * session_)
		:   TradingObject()
	{
		pImpl = new OrderManagerImpl(this, session_);
	}


	OrderManager::~OrderManager()
	{
		delete pImpl;
	}

	/*
	int OrderManager::Buy(
	int symbolID, 
	int shares, 
	OrderLimit limit, 
	double limitPrice,
	PIPositionUpdateEventSink sink
	)
	{
	OrderPtr p = new Order(symbolID, OT_BuyLong, shares, limit, limitPrice, sink);
	p->tickSize = session->getSymbolManager()->getTickSize(symbolID);
	p->sinkList.push_back(this);
	session->getExecutionHandler()->submitOrder(p);

	return p->orderID;
	}

	int OrderManager::Sell(
	int symbolID, 
	int shares, 
	OrderLimit limit, 
	double limitPrice, 
	PIPositionUpdateEventSink sink
	)
	{
	OrderPtr p = new Order(symbolID, OT_ExitLong, shares, limit, limitPrice);
	p->tickSize = session->getSymbolManager()->getTickSize(symbolID);
	p->onOrderExecuteSinkList.push_back(this);
	if (sink)
	p->onOrderChangeSinkList(sink);

	session->getExecutionHandler()->submitOrder(p);

	return p->orderID;
	}

	int OrderManager::Short(
	int symbolID, 
	int shares, 
	OrderLimit limit, 
	double limitPrice, 
	PIPositionUpdateEventSink sink
	)
	{
	OrderPtr p = new Order(symbolID, OT_SellShort, shares, limit, limitPrice,sink);
	p->tickSize = session->getSymbolManager()->getTickSize(symbolID);
	p->sinkList.push_back(this);
	session->getExecutionHandler()->submitOrder(p);

	return p->orderID;
	}

	int OrderManager::Cover(
	int symbolID, 
	int shares, 
	OrderLimit limit,
	double limitPrice, 
	PIPositionUpdateEventSink sink
	)
	{
	OrderPtr p = new Order(symbolID, OT_CoverShort, shares, limit, limitPrice,sink);
	p->tickSize = session->getSymbolManager()->getTickSize(symbolID);
	p->sinkList.push_back(this);
	session->getExecutionHandler()->submitOrder(p);

	return p->orderID;
	}

	*/

	Order * OrderManager::submitOrder(Order * order)
	{
		OrderManagerImpl * impl = dynamic_cast<OrderManagerImpl*>(pImpl);

		return impl->submitOrder(order);
	}

	//	double OrderManager::calculateExposure()
	//	{
	//		return 1.0;
	/*
	double sdVol = sqrt(avgOfVolPerTrade);
	double maxvol = 1.5*sdVol+0.000000001;
	double minvol = 1.1*sdVol-0.000000001; 
	double volatility = sqrt(volPerTrade);
	double volFactor = 	1 - ( (volatility-minvol)/(maxvol-minvol) );
	if (volFactor < 0.2) 
	volFactor = 0.2;
	if (volFactor > 1)
	volFactor = 1;

	return volFactor;*/
	//	}


	void OrderManager::Update(const Tick * tick)
	{
		OrderManagerImpl * impl = dynamic_cast<OrderManagerImpl*>(pImpl);

		impl->Update(tick);

	}


	void OrderManager::CancelOrder(int orderID)
	{
		/*
		OrderMapIndex::iterator it = orderMapIndex.find(orderID);
		if (it != orderMapIndex.end())
		{
		(*it).second->status = OS_Closed;
		}
		*/

		OrderManagerImpl * impl = dynamic_cast<OrderManagerImpl*>(pImpl);
		impl->session->getExecutionHandler()->cancelOrder(orderID);

		// send cancel cancel to execution manager
	}


	void OrderManager::UpdateOrderLimit(int orderID, double NewStopOrLimit)
	{
		/*	OrderMapIndex::iterator it = orderMapIndex.find(orderID);
		if (it != orderMapIndex.end())
		{
		(*it).second->limitPrice = NewStopOrLimit;
		}*/

		//	session->getExecutionHandler()->changeOrder(
		// go to execution manager
	}


	void OrderManager::registerTradeEventSink(IOrderChangeEventSink * sink)
	{
		OrderManagerImpl * impl = dynamic_cast<OrderManagerImpl*>(pImpl);

		impl->onOrderChangeSinkList.addSink(sink);
	}

	void OrderManager::unregisterTradeEventSink(IOrderChangeEventSink * sink)
	{
		OrderManagerImpl * impl = dynamic_cast<OrderManagerImpl*>(pImpl);

		impl->onOrderChangeSinkList.removeSink(sink);
	}

	void OrderManager::registerOrderSubmitEventSink(IOrderSubmitEventSink * sink)
	{
		OrderManagerImpl * impl = dynamic_cast<OrderManagerImpl*>(pImpl);

		impl->onOrderSubmitSinkList.addSink(sink);
	}

	void OrderManager::unregisterOrderSubmitEventSink(IOrderSubmitEventSink * sink)
	{
		OrderManagerImpl * impl = dynamic_cast<OrderManagerImpl*>(pImpl);

		impl->onOrderSubmitSinkList.removeSink(sink);
	}

	Order * OrderManager::createOrder(
		int SymbolID, 
		OrderType type, 
		IOrderChangeEventSink * onOrderChange,
		IPositionChangeEventSink * onPositionChange
		)
	{
		OrderManagerImpl * impl = dynamic_cast<OrderManagerImpl*>(pImpl);

		return impl->createOrder(SymbolID, type, onOrderChange, onPositionChange);
	}

	Order * OrderManager::createOrder(
		int SymbolID, 
		OrderType type, 
		double size,
		IOrderChangeEventSink * onOrderChange,
		IPositionChangeEventSink * onPositionChange
		)
	{
		OrderManagerImpl * impl = dynamic_cast<OrderManagerImpl*>(pImpl);

		return impl->createOrder(SymbolID, type, size, onOrderChange, onPositionChange);
	}

	Order * OrderManager::applyRisk(Order * order)
	{
		OrderManagerImpl * impl = dynamic_cast<OrderManagerImpl*>(pImpl);

		return impl->applyRisk(order);
	}

	bool OrderManager::tradingSuspended()
	{
		OrderManagerImpl * impl = dynamic_cast<OrderManagerImpl*>(pImpl);

		return impl->tradingSuspended;
	}

	void OrderManager::resumeTrading()
	{
		OrderManagerImpl * impl = dynamic_cast<OrderManagerImpl*>(pImpl);

		impl->tradingSuspended = false;
		
		impl->onTradingResumedSinkList.sendMessage();
	}

	void OrderManager::suspendTrading()
	{
		OrderManagerImpl * impl = dynamic_cast<OrderManagerImpl*>(pImpl);

		impl->tradingSuspended = true;
		
		impl->onTradingSuspendedSinkList.sendMessage();
	}

	void OrderManager::registerTradingSuspendedSink(IOnTradingSuspended * sink)
	{
		OrderManagerImpl * impl = dynamic_cast<OrderManagerImpl*>(pImpl);

		impl->onTradingSuspendedSinkList.addSink(sink);
	}

	void OrderManager::unregisterTradingSuspendedSink(IOnTradingSuspended * sink)
	{
		OrderManagerImpl * impl = dynamic_cast<OrderManagerImpl*>(pImpl);
		
		impl->onTradingSuspendedSinkList.removeSink(sink);

	}

	void OrderManager::registerTradingResumedSink(IOnTradingResumed * sink)
	{

		OrderManagerImpl * impl = dynamic_cast<OrderManagerImpl*>(pImpl);

		impl->onTradingResumedSinkList.addSink(sink);


	}	

	void OrderManager::unregisterTradingResumedSink(IOnTradingResumed * sink)
	{
		OrderManagerImpl * impl = dynamic_cast<OrderManagerImpl*>(pImpl);

		impl->onTradingResumedSinkList.removeSink(sink);

	}


}