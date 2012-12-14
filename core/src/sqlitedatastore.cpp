#include "indicator.h"
#include "sqliteDataStore.h"
#include "sqlite3.h"

#include <sstream>

namespace indicator
{		

	//////////////////
	//
	// SqliteDataStore
	//
	/////////////////

	SqliteDataStore::SqliteDataStore(DataStoreConfig * config_)
		: DataStore(config_)
	{
		hdb = 0;
	}

	SqliteDataStore::~SqliteDataStore(){
		disconnect();
		if (config)
			delete config;
	}

	sqlite3 * SqliteDataStore::handle()
	{
		if (!hdb)
		{
			throw EDataStore( "sqlite db not open " AT_FILE_LINE );
		}

		return hdb;
	}

	void SqliteDataStore::execSQL(const char * sql)
	{
		char * errormsg;
		int res;

		res = sqlite3_exec(handle(), sql, NULL, this, &errormsg);

		if (res != SQLITE_OK)
		{
			std::string s = errormsg;
			sqlite3_free(errormsg);
			s += AT_FILE_LINE;
			throw EDataStore( s );
		}
	}

	void SqliteDataStore::execSQL(const std::string sql) 
	{
		execSQL(sql.c_str());
	}

	void SqliteDataStore::readTable(const char * sql, DataStoreTable & table)
	{
		//get_table_callback_func
	}

	bool SqliteDataStore::connect() 
	{
		SqliteDataStoreConfig * conf = dynamic_cast<SqliteDataStoreConfig *>(config);
		if (!conf)
			throw EDataStore("SqliteDataStoreConfig is null " AT_FILE_LINE );

		int res = sqlite3_open_v2(conf->getFilename().c_str(), &hdb, conf->getFlags(),NULL);
		if (res != SQLITE_OK)
		{
			throw EDataStore( sqlite3_errmsg(hdb));
		}
		sqlite3_busy_timeout(hdb, conf->getTimeout());

		return true;
	}

	bool SqliteDataStore::disconnect()
	{
		if (hdb)
		{
			sqlite3_close(hdb);
			hdb = NULL;
		}

		return true;
	}

	bool SqliteDataStore::connected()
	{
		return hdb != 0;	
	}


	DataStoreINI & SqliteDataStore::getIniStore(const char * tablename)
	{
		std::map<std::string, DataStoreINI *>::iterator it = dataStoreMap.find(tablename);
		if (it == dataStoreMap.end())
		{
			DataStoreINI * ini = new SqliteDataStoreINI(this, tablename);
			dataStoreMap[tablename] = ini;
			dataStoreList.push_back(ini);

			return *ini;
		}

		return *it->second;
	}

	//////////////////
	//
	// SqliteDataStoreINI
	//
	/////////////////

	SqliteDataStoreINI::SqliteDataStoreINI(DataStore * store_, const char * tablename_)
		: DataStoreINI(store_, tablename_)
	{
		std::stringstream s;

		s << "CREATE TABLE if not exists " << tablename << " ( ";
		s << "section VARCHAR( 20 )  NOT NULL,";
		s << "keyname VARCHAR( 30 )  NOT NULL,";
		s << "value   VARCHAR( 10 ),";
		s << "bvalue BLOB,";
		s << "PRIMARY KEY ( section, keyname )  ON CONFLICT REPLACE );";

		dynamic_cast<SqliteDataStore*>(store)->execSQL(s.str());
	}

	void SqliteDataStoreINI::writeData(const char * section, const char * key, void * buffer, int size)
	{
		sqlite3_stmt * stmt;
		SqliteDataStore* sqstore = dynamic_cast<SqliteDataStore*>(store);

		std::stringstream sql;

		sql << "replace into " << tablename << " (section, keyname, bvalue) VALUES ('" << 
			section << "','" << key << "',?);";

		if ( sqlite3_prepare_v2(
			sqstore->handle(),
			sql.str().c_str(),
			sql.str().size() + 1,&stmt,NULL) != SQLITE_OK)
		{
			throwException();
		}

		if (sqlite3_bind_blob(stmt, 1, buffer, size, SQLITE_STATIC) != SQLITE_OK)
		{
			sqlite3_finalize(stmt);
			throwException();
		}

		int stepres = sqlite3_step(stmt);
		if (stepres != SQLITE_DONE)
		{
			sqlite3_finalize(stmt);
			throwException();
		}

		sqlite3_finalize(stmt);
	}

	void SqliteDataStoreINI::writeString( const char * section, const char * key, const char * val)
	{
		std::stringstream sql;

		sql << "replace into " << tablename << " (section, keyname, value) VALUES ('" << 
			section << "','" << key << "','" << val << "');";
		dynamic_cast<SqliteDataStore*>(store)->execSQL(sql.str());
	}

	void SqliteDataStoreINI::writeString( const char * section, const char * key, std::string val)
	{
		std::stringstream sql;

		sql << "replace into " << tablename << " (section, keyname, value) VALUES ('" << 
			section << "','" << key << "','" << val << "');";
		dynamic_cast<SqliteDataStore*>(store)->execSQL(sql.str());
	}

	void SqliteDataStoreINI::writeFloat( const char * section, const char * key, double val)
	{
		std::stringstream sql;

		sql << "replace into " << tablename << " (section, keyname, value) VALUES ('" << 
			section << "','" << key << "','" << val << "');";
		dynamic_cast<SqliteDataStore*>(store)->execSQL(sql.str());
	}

	void SqliteDataStoreINI::writeFloat( const char * section, const char * key, float val)
	{
		std::stringstream sql;

		sql << "replace into " << tablename << " (section, keyname, value) VALUES ('" << 
			section << "','" << key << "','" << val << "');";
		dynamic_cast<SqliteDataStore*>(store)->execSQL(sql.str());
	}

	void SqliteDataStoreINI::writeInt( const char * section, const char * key, int val)
	{

		std::stringstream sql;

		sql << "replace into " << tablename << " (section, keyname, value) VALUES ('" << 
			section << "','" << key << "','" << val << "');";
		dynamic_cast<SqliteDataStore*>(store)->execSQL(sql.str());
	}

	void SqliteDataStoreINI::writeBool( const char * section, const char * key, bool val)
	{
		std::stringstream sql;

		sql << "replace into " << tablename << " (section, keyname, value) VALUES ('" << 
			section << "','" << key << "','" << (val ? 1 : 0) << "');";
		dynamic_cast<SqliteDataStore*>(store)->execSQL(sql.str());
	}

	int SqliteDataStoreINI::readData(const char * section, const char * key, void * buffer, int size)
	{
		sqlite3_stmt * stmt;
		SqliteDataStore* sqstore = dynamic_cast<SqliteDataStore*>(store);

		std::stringstream sql;

		sql << "select bvalue from " << tablename << 
			" where section = '" << section << 
			"' and keyname = '" << key << "';";

		if ( sqlite3_prepare_v2(
			sqstore->handle(),
			sql.str().c_str(),
			sql.str().size() + 1,&stmt,NULL) != SQLITE_OK)
		{
			throwException();
		}

		int byteres = 0;

		int stepres = sqlite3_step(stmt);
		if (stepres == SQLITE_ROW)
		{
			byteres = sqlite3_column_bytes(stmt, 0);
			if (byteres > size)
				byteres = size;

			memcpy(buffer, sqlite3_column_blob(stmt, 0), byteres);
		}
		else if (stepres != SQLITE_DONE)
		{
			sqlite3_finalize(stmt);
			throwException();
		}

		sqlite3_finalize(stmt);

		return byteres;
	}

	std::string SqliteDataStoreINI::readString( const char * section, const char * key, const char * defVal)
	{
		sqlite3_stmt * stmt;
		SqliteDataStore* sqstore = dynamic_cast<SqliteDataStore*>(store);

		std::stringstream sql;

		sql << "select value from " << tablename << 
			" where section = '" << section << 
			"' and keyname = '" << key << "';";

		if ( sqlite3_prepare_v2(
			sqstore->handle(),
			sql.str().c_str(),
			sql.str().size() + 1,&stmt,NULL) != SQLITE_OK)
		{
			throwException();
		}

		string res = defVal;

		int stepres = sqlite3_step(stmt);
		if (stepres == SQLITE_ROW)
		{
			res = (const char *)sqlite3_column_text(stmt,0);
		}
		else if (stepres != SQLITE_DONE)
		{
			sqlite3_finalize(stmt);
			throwException();
		}

		sqlite3_finalize(stmt);

		return res;
	}

	double SqliteDataStoreINI::readFloat( const char * section, const char * key, double defVal)
	{
		sqlite3_stmt * stmt;
		SqliteDataStore* sqstore = dynamic_cast<SqliteDataStore*>(store);

		std::stringstream sql;

		sql << "select value from " << tablename << 
			" where section = '" << section << 
			"' and keyname = '" << key << "';";

		if ( sqlite3_prepare_v2(
			sqstore->handle(),
			sql.str().c_str(),
			sql.str().size() + 1,&stmt,NULL) != SQLITE_OK)
		{
			throwException();
		}

		double res = defVal;

		int stepres = sqlite3_step(stmt);
		if (stepres == SQLITE_ROW)
		{
			res = sqlite3_column_double(stmt,0);
		}
		else if (stepres != SQLITE_DONE)
		{
			sqlite3_finalize(stmt);
			throwException();
		}

		sqlite3_finalize(stmt);

		return res;
	}

	float SqliteDataStoreINI::readFloat( const char * section, const char * key, float defVal)
	{
		sqlite3_stmt * stmt;
		SqliteDataStore* sqstore = dynamic_cast<SqliteDataStore*>(store);

		std::stringstream sql;

		sql << "select value from " << tablename << 
			" where section = '" << section << 
			"' and keyname = '" << key << "';";

		if ( sqlite3_prepare_v2(
			sqstore->handle(),
			sql.str().c_str(),
			sql.str().size() + 1,&stmt,NULL) != SQLITE_OK)
		{
			throwException();
		}

		float res = defVal;

		int stepres = sqlite3_step(stmt);
		if (stepres == SQLITE_ROW)
		{
			res = float(sqlite3_column_double(stmt,0));
		}
		else if (stepres != SQLITE_DONE)
		{
			sqlite3_finalize(stmt);
			throwException();
		}

		sqlite3_finalize(stmt);

		return res;
	}

	int SqliteDataStoreINI::readInt( const char * section, const char * key, int defVal)
	{	
		sqlite3_stmt * stmt;
		SqliteDataStore* sqstore = dynamic_cast<SqliteDataStore*>(store);

		std::stringstream sql;

		sql << "select value from " << tablename << 
			" where section = '" << section << 
			"' and keyname = '" << key << "';";

		if ( sqlite3_prepare_v2(
			sqstore->handle(),
			sql.str().c_str(),
			sql.str().size() + 1,&stmt,NULL) != SQLITE_OK)
		{
			throwException();
		}

		int res = defVal;

		int stepres = sqlite3_step(stmt);
		if (stepres == SQLITE_ROW)
		{
			res = sqlite3_column_int(stmt,0);
		}
		else if (stepres != SQLITE_DONE)
		{
			sqlite3_finalize(stmt);
			throwException();
		}

		sqlite3_finalize(stmt);

		return res;
	}

	bool SqliteDataStoreINI::readBool( const char * section, const char * key, bool defVal)
	{
		sqlite3_stmt * stmt;
		SqliteDataStore* sqstore = dynamic_cast<SqliteDataStore*>(store);

		std::stringstream sql;

		sql << "select value from " << tablename << 
			" where section = '" << section << 
			"' and keyname = '" << key << "';";

		if ( sqlite3_prepare_v2(
			sqstore->handle(),
			sql.str().c_str(),
			sql.str().size() + 1,&stmt,NULL) != SQLITE_OK)
		{
			throwException();
		}

		bool res = defVal;

		int stepres = sqlite3_step(stmt);
		if (stepres == SQLITE_ROW)
		{
			res = sqlite3_column_int(stmt,0) == 1;
		}
		else if (stepres != SQLITE_DONE)
		{
			sqlite3_finalize(stmt);
			throwException();
		}

		sqlite3_finalize(stmt);

		return res;
	}

	void SqliteDataStoreINI::throwException()
	{
		sqlite3 * hdb = dynamic_cast<SqliteDataStore*>(store)->handle();
		throw EDataStore(sqlite3_errmsg(hdb));
	}
}