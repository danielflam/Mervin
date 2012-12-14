#include "indicator.h"

#include "fastjulian.h"

namespace indicator
{

	class PortfolioHistoryLogger : 
		public TradingSessionObject, 
		public IPortfolioChangeEventSink
{
public:
	PortfolioHistoryLogger(indicator::TradingSession * session_ );
	~PortfolioHistoryLogger();

	void onPortfolioChange(Portfolio * portfolio, PortfolioInfo * info);

protected:

	double calculate_sharpe(double CurrentTotal, double PrevTotal, double StartCash);

	virtual void registerLogger();
	virtual bool isNewInterval(double WhenDate) = 0;
	

	HistoryNodePtrList investedList;
	HistoryNodePtrList cashList;
	HistoryNodePtrList totalList;
	indicator::Logger * logger;
	indicator::Logger * csvlogger;
		
	HistoryNode * currentOpen;
	HistoryNode * currentCash;
	HistoryNode * currentTotal;


	bool initialized;
	double maxval;
	double period_maxval;
	double minval;
	double drawdown;

	double interval, riskFreeInterest;
	double M2, mean;
	
};


//////////////////////////////////
//
// PortfolioHistoryLogger 
//
//////////////////////////////////

	PortfolioHistoryLogger::PortfolioHistoryLogger(indicator::TradingSession * session_ )
	: TradingSessionObject(session_)
{
	M2 = 0;
	mean = 0;
	
	currentOpen = 0;
	currentCash = 0;
	currentTotal = 0;
	initialized = false;

	registerLogger();
}

PortfolioHistoryLogger::~PortfolioHistoryLogger()
{
	delete logger;
}

void PortfolioHistoryLogger::registerLogger()
{
	session->getPortfolio()->registerPortfolioChangeSink(this);
}

void PortfolioHistoryLogger::onPortfolioChange(Portfolio * portfolio, PortfolioInfo * info)
{
	if (!initialized || isNewInterval(info->whenDate) || info->lastOne)
	{
		if (initialized)
		{
		//	HistoryNode & currentOpen = investedList.back();
		//	HistoryNode & currentCash = cashList.back();
		//	HistoryNode & currentTotal = totalList.back();
			
			double sharpe = calculate_sharpe(
				info->investedVal+info->cashVal,
				currentTotal->close, 
				portfolio->getStartingCash()
			); 
			
			if (currentTotal->close> period_maxval)
				period_maxval = currentTotal->close;

			LOG(indicator::LT_DEBUG,
				"PORTVAL", 
				"%d-%d-%d: %d ticks. invested = %2.2f cash = %2.2f total(O-H-L-C): %2.2f-%2.2f-%2.2f-%2.2f current = %2.2f (%2.2f%%) dd:%2.2f%% sharpe: %f dd_max:%2.2f%% ",
				currentOpen->month,
				currentOpen->day,
				currentOpen->year,
				currentTotal->count,
				currentOpen->close,
				currentCash->close,
				currentTotal->open,
				currentTotal->high,
				currentTotal->low,
				currentTotal->close,
				info->investedVal+info->cashVal,
				100*((info->investedVal+info->cashVal)/portfolio->getStartingCash() - 1),
				100*( currentTotal->close/period_maxval - 1),
				sharpe,
				100*( currentTotal->low/maxval - 1)
			);

			csvlogger->log(indicator::LT_NONE,"", 
				"%s,%f,%f,%f,%f,%f",
				fastjulian::formatOleDateTime(currentOpen->when),
				currentOpen->when, 				
				currentTotal->open,
				currentTotal->high,
				currentTotal->low,
				currentTotal->close
			);

			//investedList.back().close = investedVal;
			//cashList.back().close = cashVal;
		}
		else
		{
			initialized = true;
			period_maxval = info->investedVal+info->cashVal;
			maxval = info->investedVal+info->cashVal;
			minval = info->investedVal+info->cashVal;
			csvlogger->log(indicator::LT_NONE,"","when,open,high,low,close");
		}

		HistoryNodePtr p = new HistoryNode;
		p->when = info->whenDate;
		p->count = 1;
		p->open = info->investedVal;
		p->high = info->investedVal;
		p->low = info->investedVal;
		p->close = info->investedVal;
		fastjulian::DMYFromDateTime(int(info->whenDate),p->year,p->month,p->day);
		investedList.push_back(p);
		currentOpen = p;

		HistoryNodePtr p1 = new HistoryNode ;
		p1->when = info->whenDate;
		p1->count = 1;
		p1->open = info->cashVal;
		p1->high = info->cashVal;
		p1->low = info->cashVal;
		p1->close = info->cashVal;
		p1->year = p->year;
		p1->month = p->month;
		p1->day = p->day;
		cashList.push_back(p1);
		currentCash = p1;

		HistoryNodePtr p2 = new HistoryNode ;
		p2->when = info->whenDate;
		p2->count = 1;
		p2->open = info->cashVal+info->investedVal;
		p2->high = p2->open;
		p2->low = p2->open;
		p2->close = p2->open;
		p2->year = p->year;
		p2->month = p->month;
		p2->day = p->day;
		totalList.push_back(p2);
		currentTotal = p2;

		return;
	}
	


	currentOpen->close = info->investedVal;
	currentOpen->count++;

	currentCash->close = info->cashVal;
	currentCash->count++;
	
	currentTotal->close = info->cashVal + info->investedVal;
	if (currentTotal->high < currentTotal->close) 
		currentTotal->high = currentTotal->close;
	else if (currentTotal->low  > currentTotal->close)
		currentTotal->low  = currentTotal->close;
	currentTotal->count++;

	if (currentTotal->high > maxval)
		maxval = currentTotal->high;

	if (currentTotal->low < minval)
		minval = currentTotal->low;

}

double PortfolioHistoryLogger::calculate_sharpe(double CurrentTotal, double PrevTotal, double StartCash)
{
	double x = CurrentTotal/StartCash;
	double n = totalList.size();
	double delta = x - mean;
	mean = mean + delta/n;
	M2 = M2 + delta*(x - mean);  //# This expression uses the new value of mean
 
	//double variance_n = M2/n
	double variance = n > 1 ? M2/(n - 1) : M2;
	
	double riskFreeReturns = pow(1 + riskFreeInterest/interval, n);
	double strategyReturns = CurrentTotal/StartCash ;

	return (strategyReturns - riskFreeReturns)/sqrt(variance);
}


//////////////////////////////////
//
// PortfolioHistoryMonthly 
//
//////////////////////////////////


class PortfolioHistoryMonthly : public PortfolioHistoryLogger
{
public:
	PortfolioHistoryMonthly(indicator::TradingSession * session_);
	~PortfolioHistoryMonthly();

	virtual bool isNewInterval(double WhenDate);

protected:
	int lastday;
	int lastmonth;
	int lastyear;
protected:

	double riskFree;

};
	

PortfolioHistoryMonthly::PortfolioHistoryMonthly(indicator::TradingSession * session_)
 : PortfolioHistoryLogger (session_) 
{
	riskFreeInterest = 0.001;
	interval = 12;
	logger = new indicator::FileLogger(session_,new indicator::OnFileLoggerStartEvent("HistoryMonthly", ".txt"));
	logger->enable();
	csvlogger = new indicator::FileLogger(session_,new indicator::OnFileLoggerStartEvent("historymonthly", ".csv"));
	logger->enable();

}

PortfolioHistoryMonthly::~PortfolioHistoryMonthly()
{
	delete csvlogger;
}

bool PortfolioHistoryMonthly::isNewInterval(double WhenDate)
{
	unsigned short year; 
	unsigned short month; 
	unsigned short day;

	if (!currentOpen)
		return false;

	if (int(currentOpen->when) >= int(WhenDate))
		return false;

	fastjulian::DMYFromDateTime(int(WhenDate), year, month, day);		

	return month != currentOpen->month;
}


//////////////////////////////////
//
// PortfolioHistoryDaily 
//
//////////////////////////////////

class PortfolioHistoryDaily : public PortfolioHistoryLogger
{
  public:
	  PortfolioHistoryDaily(indicator::TradingSession * session_ )
		: PortfolioHistoryLogger(session_)
	{
		riskFreeInterest = 0.035;
		interval = 365;
		logger = new indicator::FileLogger(session_,new indicator::OnFileLoggerStartEvent("HistoryDaily", ".txt"));
		logger->enable();
		csvlogger = new indicator::FileLogger(session_,new indicator::OnFileLoggerStartEvent("HistoryDaily", ".csv"));
	}
	
	~PortfolioHistoryDaily() 
	{
	  delete csvlogger;
	}

	virtual bool isNewInterval(double WhenDate)
	{
		return currentOpen && ((int(WhenDate) - int(currentOpen->when)) > 0);
	}

  protected:
};


void portfolioHistoryLoggerInit()
{
	//static indicator::SessionObjectFactory<PortfolioHistoryMonthly> portfolioHistoryMonthlyFactory;
	static indicator::SessionObjectFactory<PortfolioHistoryDaily> portfolioHistoryDailyFactory;
}

}