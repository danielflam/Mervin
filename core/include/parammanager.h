#ifndef _PARAM_MANAGER_H
#define _PARAM_MANAGER_H

#ifndef _INDICATOR_H
#error This file must be included by indicator.h
#endif

#include "multikey.h"

namespace indicator
{

	class IParamChangeEventSink
	{
	public:
		virtual void onParamsChange() = 0;
	};
	typedef IParamChangeEventSink * PIParamChangeEventSink;

	typedef MultiKey3<int, string, string> ParamNameKey;
	typedef std::map<ParamNameKey, string> ParamStringMap;
	typedef std::map<ParamNameKey, double> ParamDoubleMap;


	class ParamManager
	{
	protected:


	public:
		ParamManager(TradingSession * session_);
		virtual ~ParamManager();

		// for bulk updates
		void beginUpdate();
		void endUpdate();

		void setParam(int symbolID, string section, string name, string value);
//		void setParam(int symbolID, string section, string name, int value);
		void setParam(int symbolID, string section, string name, double value);

		void getParam(int symbolID, string section, string name, string & outval, string defval);
		void getParam(int symbolID, string section, string name, double & outval, double defval);
		void getParamPct(int symbolID, string section, string name, double & outval, double defval);
		void getParam(int symbolID, string section, string name, int & outval, int defval );
		void getParam(int symbolID, string section, string name, bool & outval, bool defval );
		
		void registerSink(PIParamChangeEventSink sink);
		void unregisterSink(PIParamChangeEventSink sink);

	private:
		void InternalEndUpdate();
		void InternalBeginUpdate();
		void NotifySinks();


		bool updating;

//		ParamChangeSinkList sinkList;
		SimpleSinkList<IParamChangeEventSink> sinkList;

		ParamStringMap stringParams;
//		ParamIntMap intParams;
		ParamDoubleMap doubleParams;

		TradingSession * session;
	};
}


#endif
