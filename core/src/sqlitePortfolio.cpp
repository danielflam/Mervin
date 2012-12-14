#include "indicator.h"
#include "sqlitepp.h"
#include "SQLitePortfolio.h"

using namespace sqlite;

namespace indicator
{

	//////////////////////////////////
	//
	// SQLitePortfolioImpl
	//
	//////////////////////////////////

#define PARAM_PORTFOLIO_SECTION "portfolio"
#define PARAM_PORTFOLIO_STARTING_CASH "starting cash"
#define PARAM_PORTFOLIO_SHOULD_LOG "logging enabled"
#define PARAM_SIMUL_MODE "simulation mode"

	typedef std::vector<Position *> PositionRefVector;
	typedef std::map<int, PositionList> PositionListMap;
	typedef std::map<int, PositionType> SymbolPositionTypeMap;

	typedef SinkListWith1Params<Portfolio, IPortfolioChangeEventSink, PortfolioInfo *> PortfolioChangeSinkList;  

	class SQLitePortfolioImpl;


	class SQPositionUpdater :
		public ISQLitCallback
	{
	public:
		SQPositionUpdater (SQLiteDB & db_, PositionListMap & positionListMap_, TradingSession * session_)
			: db( db_), positionListMap(positionListMap_)
		{
			session = session_;
		}

		virtual int onCallback(int nCol,char **cols,char **names)
		{


			Position * p = new Position(
				session->getSymbolManager()->getSymbolId(cols[1]), 
				atof(cols[2]),
				PositionType(atoi(cols[3])),
				atoi(cols[4]),
				atof(cols[5]) 
			);

           p->positionID = atoi(cols[0]);
		   p->unitCurrentPrice = cols[6] ? atof(cols[6]) : atof(cols[5]);
		   p->unitPnL = 0; //cols[7] ? atof(cols[7]) : 0 ;
		   p->unitMaxPrice = cols[8] ? atof(cols[8]) : atof(cols[5]);
		   p->unitMinPrice = cols[9] ? atof(cols[9]) : atof(cols[5]);
		   p->valueFactor = session->getSymbolManager()->getTickValueFactor(p->SymbolID);

		   PositionList & positionList = positionListMap[p->SymbolID];
		   positionList.push_back(p);

		   // most important!
		   return SQLITE_OK;
		}

		void start()
		{
			  db.exec(
				  "CREATE TABLE if not exists positions ( "
				  "positionID       INT   PRIMARY KEY,"
				  "Symbol           VARCHAR( 7 ),"
				  "openDate         FLOAT,"
				  "type             INT,"
				  "size             INT,"
				  "unitEntryPrice   FLOAT,"
				  "unitCurrentPrice FLOAT,"
				  "unitPnL          FLOAT,"
				  "unitMaxPrice     FLOAT,"
				  "unitMinPrice     FLOAT "
				  " ); "
				  );
		}

		void writePosition(Position & position)
		{
			  std::stringstream sql;

			  sql << "insert or replace into positions";
			  sql << "( positionID, ";
			  sql << " Symbol, openDate, type, size, unitEntryPrice, unitCurrentPrice,";
			  sql << "unitPnL,";
			  sql << "unitMaxPrice,";
			  sql << "unitMinPrice";
			  sql << ") VALUES (";
			  sql << position.positionID << ",";
			  sql << "'" << session->getSymbolManager()->getSymbolName(position.SymbolID) << "',";
			  sql << position.openDate << ",";
			  sql << position.type << ",";
			  sql << position.size << ",";
			  sql << position.unitEntryPrice << ",";
			  sql << position.unitCurrentPrice << ",";
			  sql << position.unitPnL << ",";
			  sql << position.unitMaxPrice << ",";
			  sql << position.unitMinPrice << ")";
			
			  db.exec(sql.str());
		}

		void readPositions()
		{
			std::stringstream sql;

			  sql << "select positionID, ";
			  sql << " Symbol, openDate, type, size, unitEntryPrice, unitCurrentPrice,";
			  sql << "unitPnL,";
			  sql << "unitMaxPrice,";
			  sql << "unitMinPrice";
			  sql << " from positions;";

			  db.exec(sql.str(), this);
		}

		void removePosition(Position & position)
		{
			std::stringstream sql;

			sql << "delete from positions where positionID = " << position.positionID;

			db.exec(sql.str());
		}

		void clear()
		{
			std::stringstream sql;

			sql << "delete from positions;";

			db.exec(sql.str());
		}

	private:
		SQLiteDB & db;
		PositionListMap & positionListMap;
		TradingSession * session;
	};

	
	class SQLitePortfolioImpl : public PImpl
	{
	public:
		SQLitePortfolioImpl(SQLitePortfolio * parent_, TradingSession * session_) :
		  portfolioChangeSinkList(&IPortfolioChangeEventSink::onPortfolioChange),
			  updater(db, positionListMap, session_) 
		  {
			  parent = parent_;
			  session = session_;
			  session->getParamManager()->registerSink(parent);
			  portfolioChangeSinkList.setSource(parent);
			  startCash =  100000;
			  cashOnHand = 100000;
			  dirty = true;
			  cachedValue = std::numeric_limits<double>::quiet_NaN();
			  openValue();
			  logger = new FileLogger(session, new OnFileLoggerStartEvent("PortfolioInMem"));
		  }

		  void start()
		  {
			  db.open("trader.sqlite", SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);

			  config = & session->getDataStore()->getIniStore("config");
			
			  updater.start();

			  if (!simulationMode)
			  {
				  // read current cash value of portfolio - its static - unlike synamic values of open positions

				  // read all positions

				  updater.readPositions();
				  cashOnHand = config->readFloat("portfolio","cash",100000.0);
				  startCash = config->readFloat("portfolio","startCash",100000.0);
				  lastTrade = config->readFloat("portfolio","lastTrade", 0.0);

				  config->writeFloat("portfolio","cash",cashOnHand);
				  config->writeFloat("portfolio","startCash",startCash);
				  config->writeFloat("portfolio","lastTrade",lastTrade);

			  }
			  else
			  {
				  updater.clear();
			  }

			  config->writeFloat("portfolio","cash",cashOnHand);
			  config->writeFloat("portfolio","startCash",startCash);
		      config->writeFloat("portfolio","lastTrade",lastTrade);
			  dirty = true;
		  }

		  void stop()
		  {
			  db.close();
		  }

		  ~SQLitePortfolioImpl()
		  {
			  db.close();
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

			  int ID = config->readInt("portfolio","Last ID", 0) + 1;
			  p->positionID = ID;
			  config->writeInt("portfolio","Last ID", ID);

			  p->valueFactor = session->getSymbolManager()->getTickValueFactor(p->SymbolID);

			  p->onPositionChangeSinkList.assign( order.onPositionChangeSinkList );
			  p->risk = order.risk;

			  PositionList & positionList = positionListMap[order.symbolID];
			  positionList.push_back(p);


			  p->setUnitPrice(tradeInfo.executionPrice, tradeInfo.executionPrice);
			  cashOnHand -= p->currentValue();
			  cachedValue += p->currentValue();

			  updater.writePosition(*p);
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
					  cachedValue -= p.currentValue();
					  tradeInfo.PnL += p.currentPnL();
					  tradeInfo.PnLpct += p.currentValuePct()*p.size;
					  tradeInfo.entryPrice += p.unitEntryPrice *  p.size;
					  tradeInfo.MAE += p.currentMAE();
					  tradeInfo.MPE += p.currentMPE();

					  if (order.size == p.size) 
					  {
						  order.size = 0;
					  }
					  updater.removePosition(p);
					  positionList.pop_front();
				  }
				  else if (order.size < p.size)
				  {
					  // keep bucket make smaller
					  cashOnHand += p.currentValue(order.size);
					  cachedValue -= p.currentValue(order.size);
					  tradeInfo.PnL += p.currentPnL(order.size);
					  tradeInfo.PnLpct += p.currentValuePct()*order.size;
					  tradeInfo.entryPrice += p.unitEntryPrice *order.size;
					  tradeInfo.MAE += p.currentMAE(order.size);
					  tradeInfo.MPE += p.currentMPE(order.size);
					  tradeInfo.size += order.size;
					  p.size -= order.size;
					  order.size = 0; // we are done
					  cachedValue += p.currentValue();
					  updater.writePosition(p);
				  }
				  else // shares > p.size
				  {
					  // remove and continue
					  cashOnHand += p.currentValue();
					  cachedValue -= p.currentValue();
					  tradeInfo.PnL += p.currentPnL();
					  tradeInfo.PnLpct += p.currentValuePct()*p.size;
					  tradeInfo.entryPrice += p.unitEntryPrice *p.size;
					  tradeInfo.MAE += p.currentMAE();
					  tradeInfo.MPE += p.currentMPE();
					  tradeInfo.size += p.size;
					  order.size -= p.size;

					  updater.removePosition(p);
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

			  int ID = config->readInt("portfolio","Last ID", 0) + 1;
			  p->positionID = ID;
			  config->writeInt("portfolio","Last ID", ID);

			  p->valueFactor = session->getSymbolManager()->getTickValueFactor(p->SymbolID);

			  p->risk = order.risk;

			  p->onPositionChangeSinkList.assign( order.onPositionChangeSinkList );

			  positionListMap[order.symbolID].push_back(p);

			  p->setUnitPrice(order.currentBid, order.currentAsk);
				  

			  order.status = OS_Closed;
			  symbolStatus[order.symbolID] = PT_Short;
			  //	dirty = true;

			  p->setUnitPrice(order.currentBid, order.currentAsk);
			  cashOnHand -= p->currentValue();
			  cachedValue += p->currentValue();
			  
			  updater.writePosition(*p);

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

				  if (order.size == -1 || order.size == p.size)
				  {
					  // remove and quit
					  tradeInfo.size  += p.size;
					  cashOnHand += p.currentValue();
					  cachedValue -= p.currentValue();
					  tradeInfo.PnLpct += p.currentValuePct()*p.size;
					  tradeInfo.PnL += p.currentPnL();
					  tradeInfo.entryPrice += p.unitEntryPrice * p.size;
					  tradeInfo.MAE += p.currentMAE();
					  tradeInfo.MPE += p.currentMPE();
					  if (order.size == p.size) 
					  {
						  order.size = 0;
					  }

					  updater.removePosition(p);
					  positionList.pop_front();

				  }
				  else if (order.size < p.size)
				  {
					  // keep bucket make smaller
					  cashOnHand += p.currentValue(order.size);
					  cachedValue -= p.currentValue(order.size);
					  tradeInfo.PnL += p.currentPnL(order.size);
					  tradeInfo.PnLpct += p.currentValuePct()*order.size;
					  tradeInfo.entryPrice += p.unitEntryPrice *order.size;
					  tradeInfo.MAE += p.currentMAE(order.size);
					  tradeInfo.MPE += p.currentMPE(order.size);
					  tradeInfo.size  += order.size;
					  p.size -= order.size;
					  order.size = 0; // we are done
					  cachedValue += p.currentValue();
					  updater.writePosition(p);
				  }
				  else // shares > p.size
				  {
					  // remove and continue
					  cashOnHand += p.currentValue();
					  cachedValue -= p.currentValue();
					  tradeInfo.PnLpct += p.currentValuePct()*p.size;
					  tradeInfo.PnL += p.currentPnL();
					  tradeInfo.entryPrice += p.unitEntryPrice * p.size;
					  order.size -= p.size;
					  tradeInfo.size  += p.size;
					  tradeInfo.MAE += p.currentMAE();
					  tradeInfo.MPE += p.currentMPE();

					  updater.removePosition(p);
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

			  lastTrade = order->tradeInfo.tradeTime;
			  config->writeFloat("portfolio","cash",cashOnHand);
			  config->writeFloat("portfolio","lastTrade",order->tradeInfo.tradeTime);


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
			  config->writeFloat("portfolio","cash",cashOnHand);
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

			  paramManager.getParam(-1,
				  "system",
				  PARAM_SIMUL_MODE,
				  simulationMode, false);

			  bool shouldlog;
			  paramManager.getParam(-1, PARAM_PORTFOLIO_SECTION, PARAM_PORTFOLIO_SHOULD_LOG ,shouldlog , true);
			  logger->setStatus(shouldlog);

			  if (simulationMode && startcashTmp != startCash)
			  {
				  //startCash =  500000;
				 startCash = startCash;

				 cashOnHand = startCash;
			     dirty = true;
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

// when we load the positions we will need to update the prices
//				  updater.writePosition(*(*it));

				  (*it)->notifyAfterUpdate();
			  }

			  PortfolioInfo pi;
			  pi.cashVal = cashOnHand;
			  pi.investedVal = cachedValue;
			  pi.lastOne = tick->lastone;
			  pi.whenDate = tick->time;
			  portfolioChangeSinkList.sendMessage(&pi);

			  if (tick->lastone && simulationMode)
			  {
				  config->writeFloat("portfolio","lastTrade",0.0);
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
								  position->size
								  )
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

		SQLitePortfolio * parent;

		PositionListMap positionListMap;

		bool dirty;
		double startCash;
		double cashOnHand;
		double cachedValue;
		//double currentTime;
		Logger * logger;
		TradingSession * session;
		SymbolPositionTypeMap symbolStatus;
		PortfolioChangeSinkList portfolioChangeSinkList;

		DataStoreINI * config;
		SQLiteDB db;
		SQPositionUpdater updater;
		bool simulationMode;
		DateTime lastTrade;
	};

	//////////////////////////////////
	//
	// SQLitePortfolio
	//
	//////////////////////////////////

	SQLitePortfolio::SQLitePortfolio (TradingSession * session_)
		: Portfolio(session_)
	{
		pImpl = new SQLitePortfolioImpl(this, session_);

		session->getParamManager()->registerSink(this);
	}

	SQLitePortfolio::~SQLitePortfolio()
	{
		session->getParamManager()->unregisterSink(this);
		delete pImpl;
	}

	void SQLitePortfolio::processTrade( Order * order )
	{
		SQLitePortfolioImpl * impl = dynamic_cast<SQLitePortfolioImpl *>(pImpl);

		impl->processTrade(order);
	}


	void SQLitePortfolio::Update( const Tick * tick )
	{
		SQLitePortfolioImpl * impl = dynamic_cast<SQLitePortfolioImpl *>(pImpl);

		impl->update(tick);
	}

	void SQLitePortfolio::CloseAllPositions()
	{
		SQLitePortfolioImpl * impl = dynamic_cast<SQLitePortfolioImpl *>(pImpl);

		impl->CloseAllPositions();
	}

	int SQLitePortfolio::positionCount()
	{
		SQLitePortfolioImpl * impl = dynamic_cast<SQLitePortfolioImpl *>(pImpl);

		return impl->positionCount();
	}

	const Position * SQLitePortfolio::getPosition(int idx)
	{
		SQLitePortfolioImpl * impl = dynamic_cast<SQLitePortfolioImpl *>(pImpl);

		return impl->getPosition(idx);
	}

	void SQLitePortfolio::onParamsChange()
	{
		SQLitePortfolioImpl * impl = dynamic_cast<SQLitePortfolioImpl *>(pImpl);

		return impl->onParamsChange();
	}

	PositionList & SQLitePortfolio::getPositionBySymbol(int symbolID)
	{
		SQLitePortfolioImpl * impl = dynamic_cast<SQLitePortfolioImpl *>(pImpl);

		return impl->positionListMap[symbolID];
	}

	double SQLitePortfolio::getPositionSizeBySymbol(int symbolID)
	{
		SQLitePortfolioImpl * impl = dynamic_cast<SQLitePortfolioImpl *>(pImpl);

		return impl->getPositionSizeBySymbol(symbolID);

	}

	double SQLitePortfolio::openValue()
	{
		SQLitePortfolioImpl * impl = dynamic_cast<SQLitePortfolioImpl *>(pImpl);

		return impl->openValue();
	}

	double SQLitePortfolio::cashValue()
	{
		SQLitePortfolioImpl * impl = dynamic_cast<SQLitePortfolioImpl *>(pImpl);

		return impl->cashValue();

	}


	void SQLitePortfolio::setCashValue(double val)
	{
		SQLitePortfolioImpl * impl = dynamic_cast<SQLitePortfolioImpl *>(pImpl);

		impl->setCashValue( val );

	}

	void SQLitePortfolio::registerPortfolioChangeSink( IPortfolioChangeEventSink * sink)
	{
		SQLitePortfolioImpl * impl = dynamic_cast<SQLitePortfolioImpl *>(pImpl);

		impl->registerPortfolioChangeSink( sink );
	}

	void SQLitePortfolio::unregisterPortfolioChangeSink( IPortfolioChangeEventSink * sink) 
	{
		SQLitePortfolioImpl * impl = dynamic_cast<SQLitePortfolioImpl *>(pImpl);

		impl->unregisterPortfolioChangeSink( sink );
	}

	double SQLitePortfolio::getStartingCash()
	{
		SQLitePortfolioImpl * impl = dynamic_cast<SQLitePortfolioImpl *>(pImpl);

		return impl->getStartingCash();
	}

	void SQLitePortfolio::onOrderExecute(indicator::TradingObject *obj,indicator::Order *order)
	{
		SQLitePortfolioImpl * impl = dynamic_cast<SQLitePortfolioImpl *>(pImpl);

		impl->processTrade(order);
	}

	void SQLitePortfolio::start( )
	{
		SQLitePortfolioImpl * impl = dynamic_cast<SQLitePortfolioImpl *>(pImpl);

		impl->start();
	}

	void SQLitePortfolio::stop( ) 
	{
		SQLitePortfolioImpl * impl = dynamic_cast<SQLitePortfolioImpl *>(pImpl);

		impl->stop();
	}

	DateTime SQLitePortfolio::lastTradeTime()
	{
		SQLitePortfolioImpl * impl = dynamic_cast<SQLitePortfolioImpl *>(pImpl);

		return impl->lastTrade;

	}


}