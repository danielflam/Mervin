#ifndef SQLITE_DATASTORE_H
#define SQLITE_DATASTORE_H

typedef struct sqlite3 sqlite3;

namespace indicator
{


	class SqliteDataStoreConfig : public DataStoreConfig
	{
	public:
		SqliteDataStoreConfig(const char * filename_, int flags_, int timeout_ = 4000)
		{
			filename = filename_;
			flags = flags_;
			timeout = timeout_;
		}

		SqliteDataStoreConfig(std::string filename_, int flags_)
		{
			filename = filename_;
			flags = flags_;
		}

		std::string getFilename(){return filename;}
		int getFlags(){return flags;}
		int getTimeout(){return timeout;}

		virtual bool isValid(){return true;};

	protected:
		std::string filename;
		int flags;
		int timeout;
	};

	class SqliteDataStoreINI;
	
	class SqliteDataStore : public DataStore
	{
	public:
		SqliteDataStore(DataStoreConfig * config_);
		virtual ~SqliteDataStore();

		sqlite3 * handle();

		void execSQL(const char * sql);
		void execSQL(const std::string sql);
		void readTable(const char * sql, DataStoreTable & table);
		bool connect();
		bool disconnect();
		bool connected();
		DataStoreINI & getIniStore(const char * tablename);

	private:
		//static int get_table_callback_func(
		//	void* obj,
		//	int nCol,
		//	char**cols, 
		//	char**names)
		//{
//			DataStoreTable * table = reinterpret_cast<DataStoreTable*>(obj);

//			for (int i = 0; i< nCol; i++)
//			{

//			}

		//	return SQLITE_OK;
		//}

	private:
		sqlite3 * hdb;
		std::string errmsg;

		managedList<DataStoreINI> dataStoreList;
		std::map<std::string, DataStoreINI *> dataStoreMap;
	};

	class SqliteDataStoreINI : public DataStoreINI
	{
	public:
		SqliteDataStoreINI(DataStore * store_, const char * tablename_);

		void writeData(const char * section, const char * key, void * buffer, int size);
		virtual void writeString( const char * section, const char * key, const char * val);
		virtual void writeString( const char * section, const char * key, std::string val);
		virtual void writeFloat( const char * section, const char * key, double val);
		virtual void writeFloat( const char * section, const char * key, float val);
		virtual void writeInt( const char * section, const char * key, int val);
		virtual void writeBool( const char * section, const char * key, bool val);

		virtual int readData(const char * section, const char * key, void * buffer, int size);
		virtual std::string readString( const char * section, const char * key, const char * defVal);
		virtual double readFloat( const char * section, const char * key, double defVal);
		virtual float readFloat( const char * section, const char * key, float defVal);
		virtual int readInt( const char * section, const char * key, int defVal);
		virtual bool readBool( const char * section, const char * key, bool defVal);

	protected:
		void throwException();
	};


}

#endif