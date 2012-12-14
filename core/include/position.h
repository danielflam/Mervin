#ifndef _POSITION_H
#define _POSITION

#ifndef _INDICATOR_H
#error This file must be included by <indicator.h>
#endif


namespace indicator
{

	enum PositionType
	{
		PT_None = 0,
		PT_Long = 1,
		PT_Short = 2,
	};

	struct Position;

	class IPositionChangeEventSink
	{
	public:
		virtual void onPositionChange(TradingObject * , Position * position) = 0;
	};
	typedef IPositionChangeEventSink * PPositionChangeEventSink;
	typedef SinkListWith1Params<TradingObject, IPositionChangeEventSink, Position *> PositionChangeSinkList;


	struct Position : public TradingObject
	{ 
		Position(
			int SymbolID_,
			double openDate_,
			PositionType type_,
			double size_ ,
			double unitEntryPrice_) :

				onPositionChangeSinkList(&IPositionChangeEventSink::onPositionChange)
			
		{
			positionID = newID();
			SymbolID = SymbolID_;
			openDate = openDate_; 
			type = type_;
			size = size_;
			unitEntryPrice = unitEntryPrice_;
			unitCurrentPrice = unitEntryPrice_;
			unitMinPrice = unitEntryPrice_+100000;
			unitMaxPrice = unitEntryPrice_-100000;
			valueFactor = 1.0;
		}

		// this is more efficient than using a default value
		double currentValue() const 
		{
			return currentUnitValue() * size;
		}

		double currentValue(double size_) const
		{
			return currentUnitValue() * size_;
		}

		double currentUnitValue() const
		{
			return type == PT_Long ? 
				unitCurrentPrice*valueFactor : 
				(2*unitEntryPrice - unitCurrentPrice) * valueFactor; 
		}

		double currentPnL() const 
		{
			return currentUnitPnL() * size;
		}

		double currentPnL(double size_) const
		{
			return currentUnitPnL() * size_;
		}

		double currentUnitPnL() const
		{
			return type == PT_Long ? 
				(unitCurrentPrice - unitEntryPrice)*valueFactor: 
				(unitEntryPrice - unitCurrentPrice)*valueFactor;
		}

		double currentMAE() const
		{
			return currentUnitMAE() * size;
		}
		
		double currentMAE(double size_) const
		{
			return currentUnitMAE() * size_;
		}

		double currentUnitMAE() const
		{
			return type == PT_Long ? 
				(unitMinPrice - unitEntryPrice)*valueFactor: 
				(unitEntryPrice - unitMaxPrice)*valueFactor;
		}

		double currentMPE() const
		{
			return currentUnitMPE() * size;
		}
		
		double currentMPE(double size_) const
		{
			return currentUnitMPE() * size_;
		}

		double currentUnitMPE() const
		{
			return type == PT_Long ? 
				(unitMaxPrice - unitEntryPrice)*valueFactor: 
				(unitEntryPrice - unitMinPrice)*valueFactor;
		}

		double currentValuePct() const
		{
			return currentUnitValue()/(valueFactor*unitEntryPrice);
		}

		void setUnitPrice(double bid, double ask)
		{
			unitCurrentPrice = type == PT_Long ? bid : ask;

			if (unitCurrentPrice > unitMaxPrice)
				unitMaxPrice = unitCurrentPrice;

			if (unitCurrentPrice < unitMinPrice)
				unitMinPrice = unitCurrentPrice;
		}

		void notifyAfterUpdate()
		{
			onPositionChangeSinkList.sendMessage(this);
		}

		int newID()
		{
			static int ID = 0;

			return ID++; 			
		}

		int positionID;
		int SymbolID;
		double openDate; 
		PositionType type;
		double size;
		double unitEntryPrice;
		double unitCurrentPrice;
		double unitPnL;
//		double unitMaxAdverseExcursion;
//		double unitMaxPositiveExcursion;
		double unitMaxPrice;
		double unitMinPrice;
		double valueFactor;
		PositionChangeSinkList onPositionChangeSinkList;
		double risk;
	};
	typedef Position * PositionPtr;

	typedef managedList<Position> PositionList;
}

#endif