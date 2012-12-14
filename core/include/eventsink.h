#ifndef _EVENT_SINK_H
#define _EVENT_SINK_H

#ifndef _INDICATOR_H
#error This file must be included by <indicator.h>
#endif

namespace indicator
{

	class MessageBase
	{
	public:
		virtual ~MessageBase(){}

	};

#define MESSAGE_FUNCDEF(NNNN) \
	template < MAGIC_PARAM_SEQUENCIALIZE_COMMA_##NNNN##( typename P ) > \
	class MessageParam##NNNN## : public MessageBase \
	{ \
	public: \
		MessageParam##NNNN##(MAGIC_PARAM_SEQUENCIALIZE_P2_##NNNN##( P , _p , MAGIC_COMMA)) \
		{\
		MAGIC_PARAM_SEQUENCIALIZE_P2_##NNNN##( p , =_p , ; ); \
		}\
		MAGIC_PARAM_SEQUENCIALIZE_P2_##NNNN##( P , _p , ; );\
	};

/*	template <class T1, class T2>
	class MessageParam2 : public MessageBase
	{
	public:
		MessageParam2(T1 t1_, T2 t2_)
		{
			t1=t1_;
			t2=t2_;
		}

		T1 t1;
		T2 t2;
	};
*/

	// interface defining a class that can handle messages
	// message can be anythings

	template <class SOURCE, class SINK, class MESSAGE>
	class SinkList
	{
	public:
		SinkList(SOURCE * source_, void (SINK::*callbackFunction_)(SOURCE * sender, MESSAGE * message))
		{
			callbackFunction = callbackFunction_;
			source = source_;
		}

		SinkList(void (SINK::*callbackFunction_)(SOURCE * sender, MESSAGE * message))
		{
			callbackFunction = callbackFunction_;
			source = 0;
		}

		SinkList(const SinkList & other)
		{
			callbackFunction = other.callbackFunction;
			source = other.source;
			sinkList.assign(other.sinkList.begin(),other.sinkList.end());
		}

		void addSink(SINK * sink){sinkList.push_back(sink);}
		void removeSink(SINK * sink){sinkList.remove(sink);}
		void assign(const SinkList & other){ sinkList.assign(other.sinkList.begin(), other.sinkList.end()); }
		void append(const SinkList & other){ sinkList.insert(sinkList.end(),other.sinkList.begin(), other.sinkList.end()); }

		void sendMessage(MESSAGE * message = 0)
		{
			typename std::list<SINK *>::iterator end = sinkList.end();
			for (std::list<SINK *>::iterator it = sinkList.begin();
				it != end; it++)
			{
				((*it)->*callbackFunction)(source, message);
			}
		}

		void sendMessage(SOURCE * sender_, MESSAGE * message = 0)
		{
			typename std::list<SINK *>::iterator end = sinkList.end();

			for (std::list<SINK *>::iterator it = sinkList.begin();
				it != end; it++)
			{
				((*it)->*callbackFunction)(sender_, message);
			}
		}

	private:
		SOURCE * source;
//		SINK * sink;
		std::list<SINK *> sinkList;
		void (SINK::*callbackFunction)(SOURCE * sender, MESSAGE * message); 
	};

	// A one to many messaging class with no message
	template <class SINK>
	class SimpleSinkList
	{
	public:
		SimpleSinkList(void (SINK::*callbackFunction_)())
		{
			callbackFunction = callbackFunction_;
			end = sinkList.end();
		}
		
		void addSink(SINK * sink){sinkList.push_back(sink);}
		void removeSink(SINK * sink){sinkList.remove(sink);}

		void sendMessage()
		{
			for (std::list<SINK *>::iterator it = sinkList.begin();
				it != end; it++)
			{
				((*it)->*callbackFunction)();
			}
		}

	private:
		SINK * sink;
		std::list<SINK *> sinkList;
		typename std::list<SINK *>::iterator end;
		void (SINK::*callbackFunction)(); 
	};

	/*
	template <class SOURCE, class SINK, MAGIC_PARAM_SEQUENCIALIZE_COMMA_2( typename P ) > 
	class SinkListWith2Params 
	{ 
	public: 
		SinkListWith2Params( 
			void (SINK::*callbackFunction_)(SOURCE * sender, MAGIC_PARAM_SEQUENCIALIZE_P2_2(P, p, MAGIC_COMMA) ) 
		) 
		{ 
			callbackFunction = callbackFunction_; 
		}
		void setSource(SOURCE source_){	source = source_; }
		void addSink(SINK * sink){sinkList.push_back(sink);}
		void removeSink(SINK * sink){sinkList.remove(sink);}
		void sendMessage(MAGIC_PARAM_SEQUENCIALIZE_P2_2(P, p, MAGIC_COMMA))
		{
			static std::list<SINK *>::iterator end = sinkList.end();
			for (std::list<SINK *>::iterator it = sinkList.begin();
				it != end; it++)
			{
				((*it)->*callbackFunction)(source, MAGIC_PARAM_SEQUENCIALIZE_COMMA_2(p));
			}
		}
	private:
		SOURCE * source;
		SINK * sink;
		std::list<SINK *> sinkList;
		void (SINK::*callbackFunction)(SOURCE * sender, MAGIC_PARAM_SEQUENCIALIZE_P2_2(P, p, MAGIC_COMMA));
	};
	*/

#define DEFINE_SINKLISTWITHPARAM(MMMM)	\
	template <class SOURCE, class SINK, MAGIC_PARAM_SEQUENCIALIZE_COMMA_##MMMM(typename P) > \
	class SinkListWith##MMMM##Params \
	{ \
	public: \
		SinkListWith##MMMM##Params( \
			void (SINK::*callbackFunction_)(SOURCE * sender, MAGIC_PARAM_SEQUENCIALIZE_P2_##MMMM(P, p, MAGIC_COMMA) ) \
		) \
		{ \
			callbackFunction = callbackFunction_; \
		}\
		SinkListWith##MMMM##Params( const SinkListWith##MMMM##Params & other) \
		{\
		   source = other.source;\
		   callbackFunction = other.callbackFunction;\
		   sinkList.assign(other.sinkList.begin(), other.sinkList.end());\
		}\
		void assign(const SinkListWith##MMMM##Params & other){ sinkList.assign(other.sinkList.begin(), other.sinkList.end()); }\
		void append(const SinkListWith##MMMM##Params & other){ sinkList.insert(sinkList.end(),other.sinkList.begin(), other.sinkList.end()); }\
		void setSource(SOURCE * source_){	source = source_; }\
		void addSink(SINK * sink){sinkList.push_back(sink);}\
		void removeSink(SINK * sink){sinkList.remove(sink);}\
		void sendMessage(MAGIC_PARAM_SEQUENCIALIZE_P2_##MMMM(P, p, MAGIC_COMMA))\
		{\
			typename std::list<SINK *>::iterator end = sinkList.end();\
			for (std::list<SINK *>::iterator it = sinkList.begin();\
				it != end; it++)\
			{\
				((*it)->*callbackFunction)(source, MAGIC_PARAM_SEQUENCIALIZE_COMMA_##MMMM(p));\
			}\
		}\
	private:\
		SOURCE * source;\
		std::list<SINK *> sinkList;\
		void (SINK::*callbackFunction)(SOURCE * sender, MAGIC_PARAM_SEQUENCIALIZE_P2_##MMMM(P, p, MAGIC_COMMA));\
	};


// SinkListWithXParam(source, sink, param1, param2....)
// sinkList.sendMessage(param1, param2...)
DEFINE_SINKLISTWITHPARAM( 1 )
DEFINE_SINKLISTWITHPARAM( 2 )
DEFINE_SINKLISTWITHPARAM( 3 )
DEFINE_SINKLISTWITHPARAM( 4 )



}

#endif
