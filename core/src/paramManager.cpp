#include "indicator.h"

namespace indicator
{

ParamManager::ParamManager(TradingSession * session_)
: sinkList(&IParamChangeEventSink::onParamsChange)
{
	session = session_;
	updating = false;
}
ParamManager::~ParamManager()
{
}

// for bulk updates
void ParamManager::beginUpdate()
{
	updating = true;
}

void ParamManager::endUpdate()
{
	updating = false;
	NotifySinks();
}

void ParamManager::setParam(int symbolID, string section, string name, string value)
{
	stringParams[ParamNameKey(symbolID, section, name)] = value;
	InternalEndUpdate();
}

/*
void ParamManager::setParam(int symbolID, string section, string name, int value)
{
	intParams[ParamNameKey(section, name)] = value;

	InternalEndUpdate();
}*/

void ParamManager::setParam(int symbolID, string section, string name, double value)
{
	doubleParams[ParamNameKey(symbolID, section, name)] = value;

	InternalEndUpdate();
}

void ParamManager::getParam(int symbolID, string section, string name, string & outval, string defval)
{
	ParamStringMap::const_iterator it = stringParams.find(ParamNameKey(symbolID, section, name));
	if (it == stringParams.end())
	{
		ParamStringMap::const_iterator it1 = stringParams.find(ParamNameKey(-1, section, name));
		outval = (it1 == stringParams.end()) ? defval : (*it1).second;
	}
	else
		outval = (*it).second;
}

void ParamManager::getParam(int symbolID, string section, string name, double & outval, double defval)
{
	ParamDoubleMap::const_iterator it = doubleParams.find(ParamNameKey(symbolID, section, name));
	if (it == doubleParams.end())
	{
		ParamDoubleMap::const_iterator it1 = doubleParams.find(ParamNameKey(-1, section, name));
		outval = (it1 == doubleParams.end()) ? defval : (*it1).second;
	}
	else
	{
		outval = (*it).second;
	}
}

void ParamManager::getParamPct(int symbolID, string section, string name, double & outval, double defval)
{
	ParamDoubleMap::const_iterator it = doubleParams.find(ParamNameKey(symbolID, section, name));
	if (it == doubleParams.end())
	{
		ParamDoubleMap::const_iterator it1 = doubleParams.find(ParamNameKey(-1, section, name));
		outval = (it1 == doubleParams.end()) ? defval : (*it1).second;
	}
	else
	{
		outval = (*it).second;
	}

	outval /= 100.0;
}


void ParamManager::getParam(int symbolID, string section, string name, int & outval, int defval)
{
	double d;
	getParam(symbolID, section, name, d, double(defval));
	outval = (int)d;
}


void ParamManager::getParam(int symbolID, string section, string name, bool & outval, bool defval)
{
	double d = defval ? 1.0 : 0.0;
	getParam(symbolID, section, name, d, d);
	outval = d > 0;
}



void ParamManager::registerSink(PIParamChangeEventSink sink)
{
	sinkList.addSink(sink);
	
}

void ParamManager::unregisterSink(PIParamChangeEventSink sink)
{
	sinkList.removeSink(sink);
}

void ParamManager::InternalBeginUpdate()
{
	// do nothing...	
}

void ParamManager::InternalEndUpdate()
{
	if (!updating)
		NotifySinks();
}


void ParamManager::NotifySinks()
{
	sinkList.sendMessage();
}


}