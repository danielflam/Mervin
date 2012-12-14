#ifndef _FILELOGGER_H
#define _FILELOGGER_H

#ifndef _INDICATOR_H
#error This file must be included by indicator.h
#endif

namespace indicator
{
	class OnFileLoggerStartEvent : public ILoggerStartEvent
	{
	public:
		OnFileLoggerStartEvent(const char * filename_, const char * filetype_ = ".txt", int ID_ = -1);
		OnFileLoggerStartEvent(string filename_, const char * filetype_ = ".txt", int ID_ = -1);

	protected:
		virtual void onLoggerStart(Logger * logger);
		string filename;
		string filetype;
		int ID;
	};


	class FileLogger : public Logger
	{
	public:

		FileLogger(TradingSession * session, ILoggerStartEvent * onStart_);
		virtual ~FileLogger();
		
		virtual void log(LogType logtype, const char * left, const char * fmt);
		virtual void log(const char * left, const char * fmt);

		virtual bool enabled();
		virtual void enable();
		virtual void disable();
		virtual void setStatus(bool enable_);
		virtual void setSession(TradingSession * session_);
		
		virtual void setFilename(const char * );
		virtual void setFiletype(const char * );
		virtual void setID(int ID );


	protected:


		PImpl * pImpl;

	};

}
#endif