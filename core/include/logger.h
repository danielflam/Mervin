#ifndef LOGGER_H
#define LOGGER_H

#ifndef _INDICATOR_H
#error This file must be included by indicator.h
#endif


namespace indicator
{

#define LOG if (logger->enabled()) logger->log

		enum LogType {
			LT_NONE = 0,
			LT_ERROR = 1,
			LT_WARNING = 2,
			LT_INFO = 3,
			LT_DEBUG = 4
		};

		enum LOGGER_ID { LID_GLOBAL = -1 };

	class ILoggerStartEvent
	{
		friend class Logger;
	protected:
		virtual void onLoggerStart(Logger * logger) = 0;
	};
	

	class Logger
	{
	public:

		Logger(TradingSession * session_, ILoggerStartEvent * onInit);
		virtual ~Logger();

		virtual void log(LogType logtype, const char * left, const char * fmt) = 0;

		virtual void log(const char * left, const char * fmt) = 0;

		virtual bool enabled() = 0;
		virtual void enable() = 0;
		virtual void disable() = 0;
		virtual void setStatus(bool enable_) = 0;
		//virtual void open(const char * fn_, int uniqueID, const char * filetype_) = 0;
		virtual void setSession(TradingSession * session_){session = session_;}

		template <typename T>
		void log(const char * left, T t)
		{
			res = boost::lexical_cast<string, T>t;
			log(logtype, left,res.c_str());
		}
				

//		template < MAGIC_PARAM_SEQUENCIALIZE_COMMA_1(typename P ) >
//		void log(LogType logtype, const char * left, const char * fmt, MAGIC_PARAM_SEQUENCIALIZE_P2_1( P , p , MAGIC_COMMA ) )
//		{
//			res = boost::str( boost::format( fmt ) % MAGIC_PARAM_SEQUENCIALIZE_1( p , MAGIC_COMMA ) );
//			log(logtype, left,res.c_str());
//		}


#define	LOGFUNC_DEF(NNNN) \
	template < MAGIC_PARAM_SEQUENCIALIZE_COMMA_##NNNN##( typename P ) > \
		void log(LogType logtype, const char * left, const char * fmt, \
			MAGIC_PARAM_SEQUENCIALIZE_P2_##NNNN##( P , p , MAGIC_COMMA) ) \
		{ \
			res = boost::str( boost::format( fmt ) % MAGIC_PARAM_SEQUENCIALIZE_##NNNN##( p , % ) ); \
			log(logtype, left,res.c_str()); \
		} 

MAGIC_PARAM_INSTANTIATE_22( LOGFUNC_DEF )


/*
		template <MAGIC_3PARAM_TEMPLATE>
		void log(LogType logtype, const char * left, const char * fmt, MAGIC_2PARAM_PARAMLIST_)
		{
			res = boost::str( boost::format( fmt ) MAGIC_3PARAM_INVOKE_OP(%));
			log(logtype, left,res.c_str());
		}

		template <MAGIC_4PARAM_TEMPLATE>
		void log(LogType logtype, const char * left, const char * fmt, MAGIC_4PARAM_PARAMLIST_)
		{
			res = boost::str( boost::format( fmt ) MAGIC_4PARAM_INVOKE_OP(%));
			log(logtype, left,res.c_str());
		}

		template <class T1, typename T2, typename T3, typename T4, typename T5>
		void log(LogType logtype, const char * left, const char * fmt, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5)
		{
			res = boost::str( boost::format( fmt ) % t1 %t2 %t3 % t4 % t5 );
			log(logtype, left,res.c_str());
		}
*/
	protected:
		void onStart()
		{
			if (onStartEvent)
				onStartEvent->onLoggerStart(this);
		}

		string res;
		ILoggerStartEvent * onStartEvent;
		TradingSession * session;
	};

}
#endif