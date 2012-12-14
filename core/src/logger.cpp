

//#include "windows.h"
//#include "tracetool.h"

#include "indicator.h"
#include <fstream>
#include <sstream>
#include <algorithm>

#define SYSTEM_PARAM_SECTION "system"
#define SYSTEM_PARAM_LOG_DIRECTORY "log directory"
#define RENKO_PARAM_LOG_ALL "consolidate logs"

#include <stdio.h>
#include <stdlib.h>


namespace indicator
{
	//	class AnyBase
	//	{
	//	};

	//	class 
	//	{

	//	};

	class streamable
	{
	public:
		streamable()
		{
		}

		friend std::ostream &operator<<(std::ostream &o, streamable &s);
		
		virtual std::ostream & stream(std::ostream & o){return o;}
	};


	template <class T>
	class Any : public streamable
	{
	public:
		Any(T t_) : t(t_)
		{	

		}

		std::ostream & stream(std::ostream & o)
		{
			o << t;
			return o;
		}

		T & t;
	};

	enum ParserState
	{
		PS_TEXT,
		PS_IDENT,
		//			PS_FORMAT,
		PS_ERROR,
		PS_ESCAPE
	};



	std::string csharpformat(std::string fmt, std::vector<streamable> args)
	{
		std::ostringstream str;
		char identbuf[100];
		int n = 0;
		ParserState state = PS_TEXT;
		const char * p1 = fmt.c_str();
		char * pident = identbuf;

		while(p1)
		{
			switch (state)
			{
			case PS_TEXT:
				if (*p1=='{')
				{
					pident = identbuf;
					state = PS_IDENT;
				}
				else if (*p1=='\\')
				{
					state = PS_ESCAPE;
				}
				else
				{
					str << *p1;
				}
				break;
			case PS_IDENT:
				if (*p1=='}')
				{
					*pident = '\0';
					int ident = atoi(identbuf);
					args[ident].stream(str);
					state = PS_TEXT;
				}
				else if (*p1>'0' && *p1<'9')
				{
					*pident = *p1;
					pident++;
				}
				else 
				{
					state = PS_TEXT;
					str << '?';
				}
				break;
				//				case PS_FORMAT:
				//					break;
			case PS_ESCAPE:
				switch (*p1)
				{
				case 'n': str << '\n'; break;
				case 't': str << '\t'; break;
				default: str << *p1;
				}
				state = PS_TEXT;
				break;
			}
			p1++;
		}
		return str.str();
	}

	template <typename T1>
	std::string csharpformat(std::string fmt, T1 t1)
	{
		std::vector<streamable> args;
		args.push_back(Any(t1));

		csharpformat(fmt, arga);
	}

	template <typename T1, typename T2>
	std::string csharpformat(std::string fmt, T1 t1, T2 t2)
	{
		std::vector<streamable> args;
		args.push_back(Any<T1>(t1));
		args.push_back(Any<T2>(t2));

		return csharpformat(fmt, args);
	}

	void test()
	{
		std::string s = "asdasd{1}sdfsdfsdf{2}asdfsdsdf{1}";
			
		csharpformat(s, 1,"3333");

	}

	template <typename T1, typename T2, typename T3>
	std::string csharpformat(std::string fmt, T1 t1, T2 t2, T3 t3)
	{
		std::vector<streamable> args;
		args.push_back(t1);
		args.push_back(t2);
		args.push_back(t3);

		csharpformat(fmt, arga);
	}

	Logger::Logger(TradingSession * session_, ILoggerStartEvent * onStart_)
	{
		setSession(session_);
		onStartEvent = onStart_;
	}

	Logger::~Logger()
	{
		{
			if (onStartEvent)
				delete onStartEvent;
		}
	}

	class FileLoggerImpl : public PImpl
	{
		friend class FileLogger;
	public: 
		FileLoggerImpl()
		{
			shouldlog = true;
			isGlobalLogger = false; 
			consolidate = false;
			fs = 0;
			ID = -1;
			filetype = ".txt";
		}

		~FileLoggerImpl()
		{
			if (fs)
				delete fs;
		}


		int state ()
		{
			return 0;
		}


		string fn;
		string res;
		string filetype;
		int ID;

		void * pImpl;

		bool consolidate;
		bool shouldlog;
		bool isGlobalLogger;
		std::ofstream * fs;

	};

	void remove_filename_chars(std::string & s)
	{
		std::replace(s.begin(), s.end(), '/','_');
		std::replace(s.begin(), s.end(), '"','_');
		std::replace(s.begin(), s.end(), '?','_');
		std::replace(s.begin(), s.end(), '-','_');
	}

	OnFileLoggerStartEvent::OnFileLoggerStartEvent(const char * filename_, const char * filetype_ , int ID_ )
	{
		filename = filename_;
		remove_filename_chars(filename);

		filetype = filetype_;
		remove_filename_chars(filetype);

		ID = ID_;
	}

	OnFileLoggerStartEvent::OnFileLoggerStartEvent(string filename_, const char * filetype_ , int ID_ )
	{
		filename = filename_;
		remove_filename_chars(filename);

		filetype = filetype_;
		remove_filename_chars(filetype);
		ID = ID_;
	}

	void OnFileLoggerStartEvent::onLoggerStart(Logger * logger)
	{
		FileLogger* fileLogger = dynamic_cast<FileLogger*>(logger);
		fileLogger->setFilename(filename.c_str());
		fileLogger->setFiletype(filetype.c_str());
		fileLogger->setID(ID);
	}

	FileLogger::FileLogger(TradingSession * session_, ILoggerStartEvent * onStart_)
		: Logger(session_, onStart_)
	{
		pImpl = new FileLoggerImpl;
		FileLoggerImpl* impl = dynamic_cast<FileLoggerImpl*>(pImpl);
	}


	FileLogger::~FileLogger()
	{
		delete dynamic_cast<FileLoggerImpl*>(pImpl); 
	}


	void FileLogger::log(const char * left, const char * fmt)
	{
		log(LT_DEBUG, left, fmt);
	}


	void FileLogger::log(LogType logtype,const char * left,const char * right)
	{

		FileLoggerImpl * impl = dynamic_cast<FileLoggerImpl*>(pImpl);


		if (!impl->fs)
		{
			onStart();

			impl->isGlobalLogger = session->getLogger() == this;

			std::stringstream s;

			string logdir;
			string logpath;
			session->getParamManager()->getParam(-1, SYSTEM_PARAM_SECTION, SYSTEM_PARAM_LOG_DIRECTORY, logdir, "");
			if (logdir.size() > 0)
			{
				s << logdir;
				s << "\\";
			}
			s << impl->fn;
			if (impl->ID > 0)
			{
				s << "_";
				s << impl->ID;
			}
			s << impl->filetype;
			logpath = s.str();

			impl->fs = new std::ofstream(logpath.c_str());
			if (impl->isGlobalLogger)
			{
				session->getParamManager()->getParam(-1, SYSTEM_PARAM_SECTION, RENKO_PARAM_LOG_ALL, impl->shouldlog, true);
				impl->consolidate = false;
			}
			else
			{
				impl->consolidate = session->getLogger()->enabled();					
			}
		}

		switch (logtype)
		{
		case LT_ERROR: 
			(*impl->fs) << "ERROR:\t"; 
			break;
		case LT_WARNING:
			*impl->fs << "WARNING:\t"; 
			break;
		case LT_DEBUG: 
			*impl->fs << "DEBUG:\t"; 
			break;

		case LT_NONE:
			break;			
		default: 
			*impl->fs << "DEBUG:\t"; 
			break;

		}

		if (logtype == LT_NONE)
		{
			*impl->fs << left << right << "\n";
		}
		else
		{
			*impl->fs << left << "\t" << right << "\n";
		}

		impl->fs->flush();

		if (impl->consolidate)
			session->getLogger()->log(logtype,left,right);
	}

	/*
	void FileLogger::open(const char * fn_, int uniqueID_, const char * filetype_)
	{
	FileLoggerImpl * impl = dynamic_cast<FileLoggerImpl*>(pImpl);
	impl->setfn(fn_);
	impl->ID = uniqueID_;
	impl->filetype = filetype_;
	}
	*/


	bool FileLogger::enabled() 
	{
		return dynamic_cast<FileLoggerImpl*>(pImpl)->shouldlog;
	}

	void FileLogger::enable()
	{
		dynamic_cast<FileLoggerImpl*>(pImpl)->shouldlog = true;
	}

	void FileLogger::disable()
	{
		dynamic_cast<FileLoggerImpl*>(pImpl)->shouldlog = false;
	}

	void FileLogger::setStatus(bool enable_)
	{
		dynamic_cast<FileLoggerImpl*>(pImpl)->shouldlog = enable_;
	}

	void FileLogger::setSession(TradingSession * session_)
	{
		Logger::setSession(session_);
		dynamic_cast<FileLoggerImpl*>(pImpl)->isGlobalLogger = session_ && session_->getLogger() == this;
	}

	void FileLogger::setFilename(const char * fn)
	{
		dynamic_cast<FileLoggerImpl*>(pImpl)->fn = fn;
		remove_filename_chars(dynamic_cast<FileLoggerImpl*>(pImpl)->fn);

	}

	void FileLogger::setFiletype(const char * ft)
	{
		dynamic_cast<FileLoggerImpl*>(pImpl)->filetype = ft;
		remove_filename_chars(dynamic_cast<FileLoggerImpl*>(pImpl)->filetype);
	}

	void FileLogger::setID(int ID )
	{
		dynamic_cast<FileLoggerImpl*>(pImpl)->ID = ID;
	}


	/*
	enum {LOGARG_NONE=0,LOGARG_STRING,LOGARG_INT,LOGARG_DOUBLE};
	void FileLogger::args_clear()
	{
	for (int i=0;i<arg_count;i++)
	{
	arg_type[i]=LOGARG_NONE;
	}
	arg_count = 0;

	}

	void FileLogger::args_push(int arg)
	{
	arg_type[arg_count] = LOGARG_INT;
	arg_int[arg_count] = arg;
	++arg_count;
	}

	void FileLogger::args_push(string arg)
	{
	arg_type[arg_count] = LOGARG_STRING;
	arg_string[arg_count] = arg;
	++arg_count;
	}

	void FileLogger::args_push(double arg)
	{
	arg_type[arg_count] = LOGARG_DOUBLE;
	arg_double[arg_count] = arg;
	++arg_count;
	}

	enum PARSESTATE {PS_CHAR, PS_PCT, PS_LB, PS_RB, PS_ARGNO, PS_ESCAPE };
	const char * FileLogger::parse(const char * format)
	{
	PARSESTATE state = PS_CHAR;
	int positional_param;
	char buf[1024];
	char * pbuf = &buf[0];
	char * plast = &buf[1023];

	for (char * p = res.c_str(); p != 0; p++)
	{
	switch(state)
	{
	case PS_CHAR:
	switch(*p)
	{
	case '\\': state=PS_ESCAPE; break;
	case '%': state=PS_PCT; break;
	case '{': state=PS_LB; break;
	default if (pbuf < plast){*pbuf=*p; pbuf++;};break;
	}
	break;
	case PS_PCT:
	switch(*p)
	{
	case '%': 
	state=PS_CHAR;
	if (pbuf < plast){*pbuf=*p; pbuf++;};break;
	case 'd': state=PS_LB; break;
	default 
	if (pbuf < plast){
	*pbuf=*p; pbuf++;
	}
	}

	break;
	case PS_LB:

	break;
	case PS_RB:

	break;
	case PS_ARGNO:

	break; 
	case PS_ESCAPE:

	break;
	}
	return 0;
	}
	*/

	/*
	LoggerTraceTool::LoggerTraceTool()
	: Logger()
	{

	}
	*/

	/*
	LoggerTraceTool & LoggerTraceTool::Instance()
	{
	static LoggerTraceTool inst;

	return inst;
	}

	LoggerTraceTool::~LoggerTraceTool()
	{

	}

	void LoggerTraceTool::log(const char * left, const char * fmt)
	{
	log(LT_DEBUG, left, fmt);
	}


	void LoggerTraceTool::log(LogType logtype,const char * left,const char * right)
	{
	TraceToSend* sender;
	switch (logtype)
	{
	case LT_ERROR: 
	sender = TTrace::Error();
	break;
	case LT_WARNING:
	sender = TTrace::Warning();
	break;
	case LT_DEBUG: 
	sender = TTrace::Debug();
	break;
	default: 
	sender = TTrace::Debug();
	break;
	}

	sender->Send(left,right );
	}
	*/
}

