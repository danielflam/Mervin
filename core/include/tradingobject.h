#ifndef _TRADINGOBJECT_H
#define _TRADINGOBJECT_H

#ifndef _INDICATOR_H
#error This file must be included by <indicator.h>
#endif

namespace indicator
{

	// All implementation class are derived from this one
	class PImpl
	{
	public:
		virtual ~PImpl(){}
				
		virtual int state() = 0;
	};

	// all classes are derived from class "TradingObject"
	// this allows to do garbage collection the easy way.

	// A Trading session object is dynamically created 
	// once passed into session it is owned and freed by the session; 

class TradingObject 
{
protected:
	virtual ~TradingObject(){}

public:
	TradingObject()
	{
		refcount = 1;
	}

	unsigned int addRef()
	{
		return ++refcount;
	}

	void release()
	{
		if (--refcount == 0)
		{
			delete this;
		}
	}

private:
	unsigned int refcount;
};


class TradingSessionObject : public TradingObject
{
protected:
	virtual ~TradingSessionObject(){}

public:
	TradingSessionObject(TradingSession * session_);


	void setSession(TradingSession * session_);
	inline TradingSession * getSession(){return session;}

protected:
	TradingSession * session;

};
	

}

#endif