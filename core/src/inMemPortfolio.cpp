#include "indicator.h"
#include "inMemPortfolio.h"
namespace indicator
{

	//////////////////////////////////
	//
	// InMemPortfolioImpl
	//
	//////////////////////////////////

#define PARAM_PORTFOLIO_SECTION "portfolio"
#define PARAM_PORTFOLIO_STARTING_CASH "starting cash"
#define PARAM_PORTFOLIO_MAXCASHPERPOSITION "maximum pct cash per position"
#define PARAM_PORTFOLIO_SHOULD_LOG "logging enabled"

	typedef std::vector<Position *> PositionRefVector;
	typedef std::map<int, PositionList> PositionListMap;
	typedef std::map<int, PositionType> SymbolPositionTypeMap;

	typedef SinkListWith1Params<Portfolio, IPortfolioChangeEventSink, PortfolioInfo *> PortfolioChangeSinkList;  
//	typedef SinkList<TradingObject, IPortfolioChangeEventSink, PortfolioInfo> PortfolioChangeSinkList;

	class InMemPortfolioImpl : public PImpl
	{
	public:
		InMemPortfolioImpl(InMemPortfolio * parent_, TradingSession * session_) :
		  portfolioChangeSinkList(&IPortfolioChangeEventSink::onPortfolioChange)
		  {
			  parent = parent_;
			  session = session_;
			  session->getParamManager()->registerSink(parent);
			  portfolioChangeSinkList.setSource(parent);
			  startCash =  500000;
			  cashOnHand = 500000;
			  dirty = true;
			  cachedValue = std::numeric_limits<double>::quiet_NaN();
			  openValue();
			  logger = new FileLogger(session, new OnFileLoggerStartEvent("PortfolioInMem"));
		  }

		  ~InMemPortfolioImpl()
		  {
			  session->getParamManager()->unregisterSink(parent);
			  delete logger;

		  }

		  int state(){return 0;}

		  static bool positionErasePred(const Position * p)
		  {
			  return p->size == 0;
		  }

		  void InternalBuy(Order & order)
		  {

			  TradeInfo & tradeInfo = order.tradeInfo;

			  tradeInfo.openTime = tradeInfo.tradeTime;
			  tradeInfo.entryPrice = tradeInfo.executionPrice;

			  PositionPtr p = new Position(
				  tradeInfo.symbolID, 
				  tradeInfo.tradeTime,
				  PT_Long,
				  tradeInfo.size, tradeInfo.executionPrice
				  );

			  p->onPositionChangeSinkList.assign( order.onPositionChangeSinkList );

			  PositionList & positionList = positionListMap[order.symbolID];
			  positionList.push_back(p);
			  
			  cashOnHand -= order.size*order.currentAsk;

			  p->setUnitPrice(tradeInfo.executionPrice, tradeInfo.executionPrice);
			  cachedValue += p->currentValue();
		  }

		  void InternalSell(Order & order)
		  {

			  TradeInfo & tradeInfo = order.tradeInfo;

			  tradeInfo.openTime = 0;
			  tradeInfo.PnLpct = 0;
			  tradeInfo.PnL = 0;
			  tradeInfo.MAE = 0;
			  tradeInfo.MPE = 0;
			  tradeInfo.entryPrice = 0;
			  tradeInfo.size = 0;

			  PositionList & positionList = positionListMap[order.symbolID];
			  while (order.size != 0 && positionList.size() > 0)
			  {
				  Position & p = positionList.front();

				  p.setUnitPrice(tradeInfo.executionPrice, tradeInfo.executionPrice);

				  cachedValue -= p.currentValue();

				  if (tradeInfo.openTime == 0 || 
					  (tradeInfo.openTime > p.openDate && p.openDate > 0))
				  {
					  tradeInfo.openTime = p.openDate;
				  }

				  if (order.size == -1 || order.size == p.size)
				  {
					  // remove and quit
					  tradeInfo.size += p.size;
					  cashOnHand += p.currentValue();
					  tradeInfo.PnL += p.currentPnL();
					  tradeInfo.PnLpct += p.currentValuePct()*p.size;
					  tradeInfo.entryPrice += p.unitEntryPrice *  p.size;
					  tradeInfo.MAE += p.currentMAE();
					  tradeInfo.MPE += p.currentMPE();
					  if (order.size == p.size) 
					  {
						  order.size = 0;
					  }
					  positionList.pop_front();
				  }
				  else if (order.size < p.size)
				  {
					  // keep bucket make smaller
					  cashOnHand += p.currentValue(order.size);
					  tradeInfo.PnL += p.currentPnL(order.size);
					  tradeInfo.PnLpct += p.currentValuePct()*order.size;
					  tradeInfo.entryPrice += p.unitEntryPrice *order.size;
					  tradeInfo.MAE += p.currentMAE(order.size);
					  tradeInfo.MPE += p.currentMPE(order.size);
					  tradeInfo.size += order.size;
					  p.size -= order.size;
					  order.size = 0; // we are done
					  cachedValue += p.currentValue();
				  }
				  else // shares > p.size
				  {
					  // remove and continue
					  cashOnHand += p.currentValue();
					  tradeInfo.PnL += p.currentPnL();
					  tradeInfo.PnLpct += p.currentValuePct()*p.size;
					  tradeInfo.entryPrice += p.unitEntryPrice *p.size;
					  tradeInfo.MAE += p.currentMAE();
					  tradeInfo.MPE += p.currentMPE();
					  tradeInfo.size += p.size;
					  order.size -= p.size;
					  positionList.pop_front();
				  }
			  }

			  tradeInfo.PnLpct = tradeInfo.PnLpct/tradeInfo.size;
			  tradeInfo.PnLinTicksPerUnit = tradeInfo.PnL / (tradeInfo.size*order.tickSize);
			  tradeInfo.entryPrice /= tradeInfo.size;
		  }

		  void InternalShort(Order & order)
		  {
			  TradeInfo & tradeInfo = order.tradeInfo;

			  tradeInfo.openTime = tradeInfo.tradeTime;
			  tradeInfo.entryPrice = tradeInfo.executionPrice;

			  PositionPtr p( new Position(
				  order.symbolID, 
				  tradeInfo.tradeTime, 
				  PT_Short,
				  tradeInfo.size, tradeInfo.executionPrice
				  ));

			  p->onPositionChangeSinkList.assign( order.onPositionChangeSinkList );

			  positionListMap[order.symbolID].push_back(p);

			  cashOnHand -= order.size*order.currentBid;
			  order.status = OS_Closed;
			  symbolStatus[order.symbolID] = PT_Short;
			  //	dirty = true;

			  p->setUnitPrice(order.currentBid, order.currentAsk);
			  cachedValue += p->currentValue();

		  }

		  void InternalCover( Order & order)
		  {
			  TradeInfo & tradeInfo = order.tradeInfo;

			  tradeInfo.openTime = 0;
			  tradeInfo.PnLpct = 0;
			  tradeInfo.PnL = 0;
			  tradeInfo.MAE = 0;
			  tradeInfo.MPE = 0;
			  tradeInfo.entryPrice = 0;
			  tradeInfo.size = 0; 

			  PositionList & positionList = positionListMap[order.symbolID];

			  while (order.size != 0 && positionList.size() > 0)
			  {
				  Position & p = positionList.front();
				  p.setUnitPrice(tradeInfo.executionPrice, tradeInfo.executionPrice);

				  //p.setUnitPrice(order.currentBid, order.currentAsk);

				  if (tradeInfo.openTime == 0 || 
					  (tradeInfo.openTime > p.openDate && p.openDate > 0))
					  tradeInfo.openTime = p.openDate;

				  cachedValue -= p.currentValue();

				  if (order.size == -1 || order.size == p.size)
				  {
					  // remove and quit
					  tradeInfo.size  += p.size;
					  cashOnHand += p.currentValue();
					  tradeInfo.PnLpct += p.currentValuePct()*p.size;
					  tradeInfo.PnL += p.currentPnL();
					  tradeInfo.entryPrice += p.unitEntryPrice * p.size;
					  tradeInfo.MAE += p.currentMAE();
					  tradeInfo.MPE += p.currentMPE();
					  if (order.size == p.size) 
					  {
						  order.size = 0;
					  }
					  positionList.pop_front();
				  }
				  else if (order.size < p.size)
				  {
					  // keep bucket make smaller
					  cashOnHand += p.currentValue(order.size);
					  tradeInfo.PnL += p.currentPnL(order.size);
					  tradeInfo.PnLpct += p.currentValuePct()*order.size;
					  tradeInfo.entryPrice += p.unitEntryPrice *order.size;
					  tradeInfo.MAE += p.currentMAE(order.size);
					  tradeInfo.MPE += p.currentMPE(order.size);
					  tradeInfo.size  += order.size;
					  p.size -= order.size;
					  order.size = 0; // we are done
					  cachedValue += p.currentValue();
				  }
				  else // shares > p.size
				  {
					  // remove and continue
					  cashOnHand += p.currentValue();
					  tradeInfo.PnLpct += p.currentValuePct()*p.size;
					  tradeInfo.PnL += p.currentPnL();
					  tradeInfo.entryPrice += p.unitEntryPrice * p.size;
					  order.size -= p.size;
					  tradeInfo.size  += p.size;
					  tradeInfo.MAE += p.currentMAE();
					  tradeInfo.MPE += p.currentMPE();

					  positionList.pop_front();
				  }
			  }

			  tradeInfo.PnLpct = tradeInfo.PnLpct/tradeInfo.size;
			  tradeInfo.PnLinTicksPerUnit = tradeInfo.PnL / (tradeInfo.size*order.tickSize);
			  tradeInfo.entryPrice /= tradeInfo.size;
		  }


		  void processTrade( Order * order )
		  {
			  if (order->tradeInfo.result == OR_REJECTED || order->tradeInfo.result == OR_ERROR)
			  {
				  // No changes
				  return;
			  }

			  switch (order->type)
			  {
			  case OT_BuyLong: 
				  InternalBuy(*order);
				  break;
			  case OT_ExitLong: 
				  InternalSell(*order);
				  break;
			  case OT_SellShort:
				  InternalShort(*order);
				  break;
			  case OT_CoverShort:
				  InternalCover(*order);
				  break;
			  }

			  PortfolioInfo pi;
			  pi.cashVal = cashOnHand; //cashValue();
			  pi.investedVal = cachedValue; //openValue();
			  pi.lastOne = false;
			  pi.whenDate = order->tradeInfo.tradeTime;
			  portfolioChangeSinkList.sendMessage(&pi);
		  }

		  double openValue()
		  {
			  if (!dirty)
				  return cachedValue;

			  cachedValue = 0;
			  for (PositionListMap::const_iterator it1 = positionListMap.begin();
				  it1 != positionListMap.end(); it1++)
			  {
				  const PositionList & positionList = (*it1).second;

				  for (std::vector<PositionPtr>::const_iterator it = positionList.list.begin();
					  it != positionList.list.end(); 
					  it++)
				  {
					  cachedValue += (*it)->currentValue();
				  }
			  }

			  dirty = false;

			  return cachedValue;

		  }

		  double cashValue()
		  {
			  return cashOnHand;
		  }

		  double getStartingCash()
		  {
			  return startCash;
		  }


		  void setCashValue(double val){
			  cashOnHand = val;
		  }

		  void onParamsChange()
		  {
			  ParamManager & paramManager = *session->getParamManager();

			  double startcashTmp;
			  paramManager.getParam(-1, 
				  PARAM_PORTFOLIO_SECTION, 
				  PARAM_PORTFOLIO_STARTING_CASH,
				  startcashTmp, 
				  500000
				  );

			  paramManager.getParamPct(-1, 
				  PARAM_PORTFOLIO_SECTION, 
				  PARAM_PORTFOLIO_MAXCASHPERPOSITION,
				  maxpositionpct,
				  97);

			  bool shouldlog;
			  paramManager.getParam(-1, PARAM_PORTFOLIO_SECTION, PARAM_PORTFOLIO_SHOULD_LOG ,shouldlog , true);
			  logger->setStatus(shouldlog);

			  if (startcashTmp != startCash)
			  {
				  //startCash =  500000;
				  startCash = startCash;
				  cashOnHand = startCash;
			  }

		  }


		  void update(const Tick * tick)
		  {
//			  currentTime = tick->time;

			  PositionList & pl = positionListMap[tick->symbolID];
			  for (PositionList::const_iterator it = pl.list.begin();
				  it != pl.list.end(); it++)
			  {
				  cachedValue -= (*it)->currentValue();
				  (*it)->setUnitPrice(tick->bid, tick->ask);
				  cachedValue += (*it)->currentValue();
				  (*it)->notifyAfterUpdate();
			  }

			  PortfolioInfo pi;
			  pi.cashVal = cashOnHand;
			  pi.investedVal = cachedValue;
			  pi.lastOne = tick->lastone;
			  pi.whenDate = tick->time;
			  portfolioChangeSinkList.sendMessage(&pi);

			  if (tick->lastone)
			  {
				  CloseAllPositions();
			  }
		  }

		  void CloseAllPositions()
		  {
			  PositionListMap::iterator iter = positionListMap.begin();
			  PositionListMap::iterator endIter = positionListMap.end();

			  for (; iter != endIter; iter++)
			  {
				  PositionList & pl = (*iter).second;
				  if (pl.size() > 0)
				  {
					  for (
						  PositionList::iterator it = pl.begin();
						  it != pl.end(); it++)
					  {
						  Position * position = *it;

						  if (position->size > 0)
						  {
							  session->getOrderManager()->submitOrder(
								  session->getOrderManager()->createOrder(
								  position->SymbolID,
								  position->type == PT_Long ? OT_ExitLong : OT_CoverShort,
								  position->size)
							  );
						  }

						  /*
						  create order and sell
						  if (pl.front().type == PT_Long)
						  {
						  InternalSell( *(*iter).first );
						  }
						  else if (pl.front().type == PT_Short)
						  {
						  InternalCover( *(*iter).first );
						  }
						  */
					  }
				  }
			  }
		  }

		  int positionCount()
		  {
			  return positionListMap.size();
		  }

		  const Position * getPosition(int idx)
		  {
			  return 0; 
		  }

		  void registerPortfolioChangeSink( IPortfolioChangeEventSink * sink)
		  {
			  portfolioChangeSinkList.addSink(sink);
		  }

		  void unregisterPortfolioChangeSink( IPortfolioChangeEventSink * sink) 
		  {
			  portfolioChangeSinkList.removeSink(sink);
		  }

		  double getPositionSizeBySymbol(int symbolID)
		  {
			    double result = 0;

				PositionList & positionList = positionListMap[symbolID];
				PositionList::const_iterator end = positionList.end();
				for (PositionList::const_iterator it = positionList.begin(); it != end; it++)
				{
					result += (*it)->size;
				}

				return result;
		  }


	public:

		InMemPortfolio * parent;

		PositionListMap positionListMap;

		bool dirty;
		double startCash;
		double cashOnHand;
		double cachedValue;
		//double currentTime;
		double maxpositionpct;
		Logger * logger;
		TradingSession * session;
		SymbolPositionTypeMap symbolStatus;
		PortfolioChangeSinkList portfolioChangeSinkList;
		DateTime lastTrade;

	};

	//////////////////////////////////
	//
	// InMemPortfolio
	//
	//////////////////////////////////

	InMemPortfolio::InMemPortfolio (TradingSession * session_)
		: Portfolio(session_)
	{
		pImpl = new InMemPortfolioImpl(this, session_);

		session->getParamManager()->registerSink(this);
	}

	InMemPortfolio::~InMemPortfolio()
	{
		session->getParamManager()->unregisterSink(this);
		delete pImpl;
	}

	void InMemPortfolio::processTrade( Order * order )
	{
		InMemPortfolioImpl * impl = dynamic_cast<InMemPortfolioImpl *>(pImpl);

		impl->processTrade(order);
	}


	void InMemPortfolio::Update( const Tick * tick )
	{
		InMemPortfolioImpl * impl = dynamic_cast<InMemPortfolioImpl *>(pImpl);

		impl->update(tick);
	}

	void InMemPortfolio::CloseAllPositions()
	{
		InMemPortfolioImpl * impl = dynamic_cast<InMemPortfolioImpl *>(pImpl);

		impl->CloseAllPositions();
	}

	int InMemPortfolio::positionCount()
	{
		InMemPortfolioImpl * impl = dynamic_cast<InMemPortfolioImpl *>(pImpl);

		return impl->positionCount();
	}

	const Position * InMemPortfolio::getPosition(int idx)
	{
		InMemPortfolioImpl * impl = dynamic_cast<InMemPortfolioImpl *>(pImpl);

		return impl->getPosition(idx);
	}

	void InMemPortfolio::onParamsChange()
	{
		InMemPortfolioImpl * impl = dynamic_cast<InMemPortfolioImpl *>(pImpl);

		return impl->onParamsChange();
	}

	PositionList & InMemPortfolio::getPositionBySymbol(int symbolID)
	{
		InMemPortfolioImpl * impl = dynamic_cast<InMemPortfolioImpl *>(pImpl);

		return impl->positionListMap[symbolID];
	}

	double InMemPortfolio::getPositionSizeBySymbol(int symbolID)
	{
		InMemPortfolioImpl * impl = dynamic_cast<InMemPortfolioImpl *>(pImpl);

		return impl->getPositionSizeBySymbol(symbolID);

	}

	double InMemPortfolio::openValue()
	{
		InMemPortfolioImpl * impl = dynamic_cast<InMemPortfolioImpl *>(pImpl);

		return impl->openValue();
	}

	double InMemPortfolio::cashValue()
	{
		InMemPortfolioImpl * impl = dynamic_cast<InMemPortfolioImpl *>(pImpl);

		return impl->cashValue();

	}


	void InMemPortfolio::setCashValue(double val)
	{
		InMemPortfolioImpl * impl = dynamic_cast<InMemPortfolioImpl *>(pImpl);

		impl->setCashValue( val );

	}

	void InMemPortfolio::registerPortfolioChangeSink( IPortfolioChangeEventSink * sink)
	{
		InMemPortfolioImpl * impl = dynamic_cast<InMemPortfolioImpl *>(pImpl);

		impl->registerPortfolioChangeSink( sink );
	}

	void InMemPortfolio::unregisterPortfolioChangeSink( IPortfolioChangeEventSink * sink) 
	{
		InMemPortfolioImpl * impl = dynamic_cast<InMemPortfolioImpl *>(pImpl);

		impl->unregisterPortfolioChangeSink( sink );
	}

	double InMemPortfolio::getStartingCash()
	{
		InMemPortfolioImpl * impl = dynamic_cast<InMemPortfolioImpl *>(pImpl);

		return impl->getStartingCash();
	}

	void InMemPortfolio::onOrderExecute(indicator::TradingObject *obj,indicator::Order *order)
	{
		InMemPortfolioImpl * impl = dynamic_cast<InMemPortfolioImpl *>(pImpl);

		impl->processTrade(order);
	}

	void InMemPortfolio::start( )
	{

	}

	void InMemPortfolio::stop( ) 
	{
	}

	DateTime InMemPortfolio::lastTradeTime()
	{
		InMemPortfolioImpl * impl = dynamic_cast<InMemPortfolioImpl *>(pImpl);

		return impl->lastTrade;

	}


}