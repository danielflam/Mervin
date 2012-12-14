#include "indicator.h"
#include "executionSimulator.h"

namespace indicator
{

	//////////////////////////////////
	//
	// ExecutionSimulatorImpl
	//
	//////////////////////////////////

	class ExecutionSimulatorImpl : public PImpl
	{
	public:
		ExecutionSimulatorImpl(ExecutionSimulator * parent_, TradingSession * session_) 
		{
			session = session_;
			parent = parent_;
			tmpQueueSize = 0;
			queueSize = 0;
		}

		~ExecutionSimulatorImpl()
		{

		}

		int state(){ return 0;}

		void registerExecuteEventSink(IOrderExecuteEventSink * sink)
		{
			//			onOrderExecuteSinkList.addSink(sink);
		}

		void unregisterExecuteEventSink(IOrderExecuteEventSink * sink)
		{
			//			onOrderExecuteSinkList.removeSink(sink);
		}

		void executeOrder(Order * order)
		{
			TradeInfo & tradeInfo = order->tradeInfo;

			tradeInfo.orderID = order->orderID;
			tradeInfo.limit = order->limit;
			tradeInfo.symbolID = order->symbolID;
			tradeInfo.tickID = order->currentTickID;
			//			tradeInfo.entryPrice = order->currentAsk; // filled by event handlers
			tradeInfo.executionPrice = 
				order->type == OT_BuyLong || order->type == OT_CoverShort ?
				order->currentAsk : order->currentBid; 
			tradeInfo.tradeTime = lastTickTime;
			tradeInfo.tickSize = order->tickSize;
			tradeInfo.type = order->type;
			order->status = OS_Closed;
			tradeInfo.size = order->size;
			tradeInfo.limit = order->limit;
			tradeInfo.limitPrice = order->limitPrice;
			tradeInfo.result = OR_SUCCESS;
			order->onOrderExecuteSinkList.sendMessage(parent, order);
		}

		void copyOrdersFromTmpQueue()
		{
			for (TmpOrderList::iterator it = tmpOrderQueue.begin(); it != tmpOrderQueue.end(); it++)
			{
				orderQueue.push_back(*it);
			}
			tmpOrderQueue.clear();
			tmpQueueSize = 0;
			queueSize = orderQueue.size();
		}

		//void clearClosedTrades()
		//{
		//			OrderMapIndex::iterator iter = orderMapIndex.begin();
		//			OrderMapIndex::iterator nextIter = orderMapIndex.begin();
		//			OrderMapIndex::iterator endIter = orderMapIndex.end();

		//			while (iter != endIter)
		//			{
		//			if (nextIter!=endIter)
		//			nextIter++;
		//			if( (*iter).second->status == OS_Closed )
		//			{
		//			orderMapIndex.erase(iter);
		//			iter = nextIter;
		//			if (nextIter != endIter)
		//			nextIter++;
		//			}
		//			else
		//			{
		//			++iter;
		//			}
		//			}
		//}

		void processAllOrders()
		{
			OrderList::iterator it = orderQueue.begin();
			while ( it != orderQueue.end())
			{
				Order * order = (*it);

				Tick & tick = session->getSymbolManager()->getCurrentTick(order->symbolID);

				if (order->status != OS_Closed)
				{
					order->currentBid = tick.bid;
					order->currentAsk = tick.ask;
					order->currentTickID = tick.ID;
					order->tradeInfo.tradeTime = tick.time;
					executeOrder(order);
				}

				if (order->status == OS_Closed)
				{
					orderMapIndex.erase(order->orderID);
					it = orderQueue.remove(it);
				}
				else
				{
					it++;
				}
			}
			queueSize = orderQueue.size();
		}

		void processOrderQueue(const Tick * tick)
		{
			OrderList::iterator it = orderQueue.begin();
			while ( it != orderQueue.end())
			{
				Order * order = (*it);

				if (order->status != OS_Closed)
				{
					if (order->symbolID == tick->symbolID)
					{
						order->currentBid = tick->bid;
						order->currentAsk = tick->ask;
						order->currentTickID = tick->ID;
						order->tradeInfo.tradeTime = tick->time;
						executeOrder(order);
					}
					else if (tick->lastone)
					{
						Tick & tick1 = session->getSymbolManager()->getCurrentTick(order->symbolID);

						if (order->status != OS_Closed)
						{
							order->currentBid = tick1.bid;
							order->currentAsk = tick1.ask;
							order->currentTickID = tick1.ID;
							order->tradeInfo.tradeTime = tick1.time;
							executeOrder(order);
						}
					}

				}

				if (order->status == OS_Closed)
				{
					orderMapIndex.erase(order->orderID);
					it = orderQueue.remove(it);
				}
				else
				{
					it++;
				}
			}
			queueSize = orderQueue.size();

		}

		void update(const Tick * tick)
		{
			lastTickTime = tick->time;

			if (tmpQueueSize > 0)
			{
				copyOrdersFromTmpQueue();
			}

			if (queueSize > 0)
			{
				processOrderQueue(tick);
			}
		}

		void submitOrder(Order * order)
		{
			tmpQueueSize++;
			tmpOrderQueue.push_back(order);
			orderMapIndex[order->orderID] = order;
		}

	public:

		TradingSession * session;
		TmpOrderList tmpOrderQueue;
		OrderList orderQueue;
		OrderMapIndex orderMapIndex;		
		//		OrderExecuteSinkList onOrderExecuteSinkList;
		int tmpQueueSize;
		int queueSize;
		ExecutionSimulator * parent;
		double lastTickTime;
	};

	//////////////////////////////////
	//
	// ExecutionSimulator
	//
	//////////////////////////////////

	ExecutionSimulator::ExecutionSimulator(TradingSession * session_)
		: ExecutionHandler(session_)
	{
		pImpl = new ExecutionSimulatorImpl(this, session_);
	}

	ExecutionSimulator::~ExecutionSimulator()
	{
		delete pImpl;
	}

	void ExecutionSimulator::submitOrder( Order * order)
	{
		ExecutionSimulatorImpl * impl = dynamic_cast<ExecutionSimulatorImpl*>(pImpl);

		impl->submitOrder(order);
	}

	void ExecutionSimulator::onExecute(const TradeInfo & tradeInfo)
	{
		ExecutionSimulatorImpl * impl = dynamic_cast<ExecutionSimulatorImpl*>(pImpl);

		//Order * order = impl->orderMapIndex[tradeInfo.orderID]

		OrderMapIndex::iterator it = impl->orderMapIndex.find(tradeInfo.orderID);

		if (it != impl->orderMapIndex.end())
		{
			Order * order = (*it).second;
			order->tradeInfo = tradeInfo;

			impl->orderMapIndex.erase(it);

			order->onOrderExecuteSinkList.sendMessage(this, order);

			impl->orderQueue.remove(order);
		}
	}

	void ExecutionSimulator::onParamsChange()
	{
		//ExecutionSimulatorImpl * impl = dynamic_cast<ExecutionSimulatorImpl*>(pImpl);

		// NOP
	}

	bool ExecutionSimulator::orderExists(const int orderID)
	{
		ExecutionSimulatorImpl * impl = dynamic_cast<ExecutionSimulatorImpl*>(pImpl);

		return impl->orderMapIndex.find(orderID) !=
			impl->orderMapIndex.end();
	}

	int ExecutionSimulator::openOrderCount()
	{
		ExecutionSimulatorImpl * impl = dynamic_cast<ExecutionSimulatorImpl*>(pImpl);

		return impl->orderQueue.size();
	}

	const Order * ExecutionSimulator::getOrderByIndex(const int idx)
	{
		ExecutionSimulatorImpl * impl = dynamic_cast<ExecutionSimulatorImpl*>(pImpl);
		return impl->orderQueue[idx];
	}

	const Order * ExecutionSimulator::getOrderByID(const int orderID)
	{
		ExecutionSimulatorImpl * impl = dynamic_cast<ExecutionSimulatorImpl*>(pImpl);

		OrderMapIndex::iterator it = impl->orderMapIndex.find(orderID);

		return (it != impl->orderMapIndex.end()) ? it->second : 0;
	}

	const int ExecutionSimulator::indexOfOrder(const int orderID)
	{
		ExecutionSimulatorImpl * impl = dynamic_cast<ExecutionSimulatorImpl*>(pImpl);

		for (int i=0 ; i < impl->orderQueue.size(); i++)
		{
			if (impl->orderQueue[i]->orderID == orderID)
				return i;
		}
		return -1;
	}

	void ExecutionSimulator::Update(const Tick * tick)
	{
		ExecutionSimulatorImpl * impl = dynamic_cast<ExecutionSimulatorImpl*>(pImpl);

		impl->update(tick);

	}

	void ExecutionSimulator::cancelOrder(int orderID)
	{

	}

	void ExecutionSimulator::cancelAllOpenOrders()
	{

	}

	void ExecutionSimulator::changeOrder(Order * order)
	{

	}

	void ExecutionSimulator::registerExecuteEventSink(IOrderExecuteEventSink * sink)
	{
		ExecutionSimulatorImpl * impl = dynamic_cast<ExecutionSimulatorImpl*>(pImpl);

		impl->registerExecuteEventSink(sink);
	}

	void ExecutionSimulator::unregisterExecuteEventSink(IOrderExecuteEventSink * sink)
	{
		ExecutionSimulatorImpl * impl = dynamic_cast<ExecutionSimulatorImpl*>(pImpl);

		impl->unregisterExecuteEventSink(sink);
	}

}