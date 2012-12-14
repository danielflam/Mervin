


#include "Indicator.h"

namespace indicator 
{

	//////////////////////////////////////////////
	//
	//  IndicatorSeriesEvent
	//
	//
	//////////////////////////////////////////////



	//////////////////////////////////////////////
	//
	//  Indicator
	//
	//
	//////////////////////////////////////////////



	IndicatorSeries::IndicatorSeries(ChartScript * parent_)
		:  parent(0), session(0)
	{
		setParent(parent_);
		paramsDirty = true;
	}

	IndicatorSeries::~IndicatorSeries()
	{	
		session->getParamManager()->unregisterSink(this);
	}

	void IndicatorSeries::setParent(ChartScript * parent_)
	{
		if (parent == parent_)
			return;

		if (parent)
		{
			parent->unregisterIndicatorSeries(this);
			session = 0;
		}

		parent = parent_;

		if (parent)
		{
			parent->registerIndicatorSeries(this);
			session = parent->session;
		}
	}


	void IndicatorSeries::start()
	{
		paramsDirty = true;
		
		session  = parent->getSession();
		
		internalStart();

		session->getParamManager()->registerSink(this);
	}

	void IndicatorSeries::reset()
	{
		paramsDirty = true;
		internalReset();
		//			ParamManager::Instance().unregisterSink(this);
	}

	void IndicatorSeries::recalc(){
		internalRecalc();
	}


	void IndicatorSeries::onParamsChange(){paramsDirty = true;}

	void IndicatorSeries::update()
	{
		if (paramsDirty)
		{
			ParamManager & paramManager = *session->getParamManager();

			onReadParams(paramManager);

			paramsDirty = false;
		}

		if (onUpdate())
		{
			//				triggerOnChange();
		}

	}


}