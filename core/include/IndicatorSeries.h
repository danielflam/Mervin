#ifndef INDICATORSERIES_H
#define INDICATORSERIES_H



namespace indicator
{



//////////////////////////////////////////////
//
//  IndicatorSeries
//
//////////////////////////////////////////////

	class IndicatorSeries : public boost::noncopyable, IParamChangeEventSink
	{
		friend class ChartScript;


	public:
		IndicatorSeries(ChartScript * parent_ = 0);
		virtual ~IndicatorSeries();

		virtual double value(int index = -1) = 0;
		virtual int size() = 0;

		double operator[](int index){return value(index);}

		void start();		
		void reset();
		void recalc();

		void setParent(ChartScript * parent_);

		void onParamsChange();

		void update();

	protected:
		virtual void internalStart() = 0;
		virtual void internalReset() = 0;
		virtual void internalRecalc(){}

		virtual bool onUpdate() = 0;

		virtual void onReadParams(ParamManager & paramManager) = 0;

		ChartScript * parent;
		TradingSession * session;
		bool paramsDirty;

	};

	typedef IndicatorSeries * IndicatorSeriesPtr;
	typedef std::list<IndicatorSeriesPtr> IndicatorSeriesIndex;

}

#endif