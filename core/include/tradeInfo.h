#ifndef _TRADEINFO_H
#define _TRADEINFO_H

#ifndef _INDICATOR_H
#error This file must be included by <indicator.h>
#endif

namespace indicator
{
	enum OrderState
	{
		ORDER_TYPE_INIT,
		ORDER_TYPE_PENDING,
		ORDER_TYPE_COMPLETE,
		ORDER_TYPE_REJECTED
	};

	enum OrderType
	{
		OT_Unknown = 0,
		OT_BuyLong = 1,
		OT_SellShort = 2,
		OT_ExitLong = 3,
		OT_CoverShort = 4,
	};

	enum OrderLimit
	{
		OL_Unknown = -1,
		OL_Market = 1,
		OL_Stop = 2,
		OL_Limit = 3
	};

	enum OrderStatus
	{
		OS_Open = 1,
		OS_Closed = 2
	};

	enum OrderResult
	{
		OR_SUCCESS = 0,
		OR_REJECTED = 1,
		OR_PARTIAL = 2,
		OR_ERROR = 3
	};

	struct TradeInfo
	{
		int orderID;
		int symbolID;
		indicator::DateTime openTime;
		indicator::DateTime tradeTime;
		OrderType type;
		OrderLimit limit;
		OrderResult result;
		int tickID; // TickID at moment of execution
		double PnL;
		double PnLinTicksPerUnit;
		double PnLpct;
		double limitPrice;
		double executionPrice;
		double entryPrice;
		double MAE; //Max Adverse Excursion
		double MPE; // Max Positive Excursion
		double tickSize;
		double size;
		std::string errcode;
	};
}

#endif