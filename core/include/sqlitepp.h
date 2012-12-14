#ifndef _SQLITEPP_H
#define _SQLITEPP_H

#include "sqlite3.h"
#include <string>
#include <sstream>

namespace sqlite
{

typedef std::runtime_error ESQLite;


class ISQLitCallback
{

public:	
	virtual int onCallback(
		int nCol,
		char **cols, 
		char **names
		)=0;
};

class SQLiteDB
{

public:
	SQLiteDB()
	{
		hSqlite = NULL;
	}

	SQLiteDB(const char *filename, int flags = SQLITE_OPEN_READWRITE)
	{
		open(filename, flags); 
	}

	~SQLiteDB()
	{
		close();
	}

	inline void close()
	{
		if (hSqlite)
		{
			sqlite3_close(hSqlite);
			hSqlite = NULL;
		}
	}

	inline int open(
		const char *filename,   /* Database filename (UTF-8) */
		int flags = SQLITE_OPEN_READWRITE              /* Flags */
		) 
	{
		int res = sqlite3_open_v2(filename, &hSqlite, flags, NULL); 
		if (res != SQLITE_OK)
		{
			throw ESQLite( sqlite3_errmsg(hSqlite));
		}
		sqlite3_busy_timeout(hSqlite, 4000);

		exec("PRAGMA synchronous = OFF");
		exec("PRAGMA journal_mode = OFF;");
	//	exec("PRAGMA locking_mode = EXCLUSIVE;");
		exec("PRAGMA temp_store = MEMORY;");
		exec("PRAGMA count_changes = OFF;");
		exec("PRAGMA PAGE_SIZE = 4096;");
		exec("PRAGMA default_cache_size=700000;");
		exec("PRAGMA cache_size=700000;");
		exec("PRAGMA compile_options;");

		return res;
	}
	
	inline sqlite3 * handle()
	{
		if (!hSqlite)
		{
			throw ESQLite( "sqlite db not open " AT_FILE_LINE );
		}
		
		return hSqlite;
	}

	void exec(string sql, ISQLitCallback * callback = NULL)
	{
		exec(sql.c_str(), callback);
	}
	
	void exec(const char * sql, ISQLitCallback * callback = NULL)
	{
		char * errormsg;
		int res;

		if (callback)
		{
			res = sqlite3_exec(handle(), sql, callback_func, callback, &errormsg);
		}
		else
		{
			res = sqlite3_exec(handle(), sql, NULL, this, &errormsg);
		}

		if (res != SQLITE_OK)
		{
			std::string s = errormsg;
			sqlite3_free(errormsg);
			s += AT_FILE_LINE;
			throw ESQLite( s );
		}
	}

	std::stringstream & getStream(string sql)
	{
		return getStream(sql.c_str());
	}

	std::stringstream & getStream(const char * sql)
	{
		char * errormsg;
		int res;

		resultStream.str("");

		res = sqlite3_exec(handle(), sql, get_stream_callback_func, this, &errormsg);

		if (res != SQLITE_OK)
		{
			std::string s = errormsg;
			sqlite3_free(errormsg);
			s += AT_FILE_LINE;
			throw ESQLite( s );
		}

		resultStream.seekg(0, std::ios_base::beg);
			
		return resultStream;
	}

private:

	static int callback_func(
		void* obj,
		int nCol,
		char**cols, 
		char**names)
	{
		ISQLitCallback * cb = reinterpret_cast<ISQLitCallback*>(obj);

		return cb->onCallback(nCol, cols, names);
	}

	
	static int get_stream_callback_func(
		void* obj,
		int nCol,
		char**cols, 
		char**names)
	{
		SQLiteDB * db = reinterpret_cast<SQLiteDB*>(obj);
		
		for (int i = 0; i< nCol; i++)
		{
			db->resultStream << (cols[i] ? cols[i] : "")  << "\t";
		}

		return SQLITE_OK;
	}

	private:

	sqlite3 * hSqlite;
	std::stringstream resultStream;
};

/*
class SQLiteStmt;


class SQLiteStmt
{
public:
	SQLiteStmt(SQLiteDB & parent_, ISQLitCallback * callback_ = NULL) :
		parent(parent_),
		callback(callback_)
	{
	}

	static int callback_func(
		void* obj,
		int nCol,
		char**cols, 
		char**names)
	{
		SQLiteStmt * stmt = static_cast<SQLiteStmt*>(obj);

		return stmt->callback->onCallback(nCol, cols, names);
	}

		
	
	void exec(std::string sql)
	{
		exec(sql.c_str());
	}

	void exec(const char * sql)
	{
		char * errormsg;
		int res;

		if (callback)
		{
			res = sqlite3_exec(parent.handle(), sql, &callback_func, this, &errormsg);
		}
		else
		{
			res = sqlite3_exec(parent.handle(), sql, NULL, this, &errormsg);
		}

		if (res != SQLITE_OK)
		{
			std::string s = errormsg;
			sqlite3_free(errormsg);
			s += AT_FILE_LINE;
			throw ESQLite( s );
		}
	}

	SQLiteDB & parent;
	ISQLitCallback * callback;
};
*/

	class SQLiteConfig:
		public ISQLitCallback
	{
	public:
		SQLiteConfig(SQLiteDB & db_)
			: db( db_)
		{
		}

		virtual int onCallback(int nCol,char **cols,char **names)
		{
		   if (cols[0])
			res = cols[0];

		   // most important!
		   return SQLITE_OK;
		}

		void start()
		{

			db.exec(
				"CREATE TABLE if not exists config ( "
				"section VARCHAR( 20 )  NOT NULL,"
				"keyname VARCHAR( 30 )  NOT NULL,"
				"value   VARCHAR( 10 ),"
				"PRIMARY KEY ( section, keyname )  ON CONFLICT REPLACE );"
			);
		}

		void setString( const char * section, const char * key, const char * val)
		{
			std::stringstream sql;

			sql << "replace into config (section, keyname, value) VALUES ('" << 
				section << "','" << key << "','" << val << "');";
			db.exec(sql.str());

		}

		void setFloat( const char * section, const char * key, double val)
		{
			std::stringstream sql;

			sql << "replace into config (section, keyname, value) VALUES ('" << 
				section << "','" << key << "','" << val << "');";
			db.exec(sql.str());
		}

		void setInt( const char * section, const char * key, int val)
		{
			std::stringstream sql;

			sql << "replace into config (section, keyname, value) VALUES ('" << 
				section << "','" << key << "','" << val << "');";
			db.exec(sql.str());
		}

		void setBool(const char * section, const char * key, int val)
		{
			setString( section, key, val? "1" : "0");
		}

		string getString( const char * section, const char * key, const char * defaultVal)
		{
			std::stringstream sql;

			res = defaultVal;
			sql << "select value from config where section='" << section << "' and keyname = '" << key << "'";
			db.exec(sql.str(), this);
			
			return res;
		}

		double getFloat( const char * section, const char * key, double defaultVal)
		{
			std::stringstream s;

			s << defaultVal;

			getString(section,key, s.str().c_str() );

			s.str(res);
			double result;
			s >> result;

			return result;
		}

		bool getBool( const char * section, const char * key, bool defaultVal)
		{
			string s = defaultVal ? "1" : "0";
			
			return getString( section, key, s.c_str() ) == "1";
		}

		int getInt( const char * section, const char * key, int defaultVal)
		{
			std::stringstream s;

			s << defaultVal;

			getString(section,key, s.str().c_str() );

			s.str(res);
			int result;
			s >> result;

			return result;
		}

	private:
		SQLiteDB & db;
		string res;
	};

}

#endif
