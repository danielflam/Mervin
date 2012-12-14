#ifndef _INDICATOR_H
#error This file must be included by <indicator.h>
#endif

#ifndef IND_DATASTORE_H
#define IND_DATASTORE_H

namespace indicator
{

	typedef std::runtime_error EDataStore;

	typedef std::vector<string> DataStoreRow;
	typedef std::vector<DataStoreRow> DataStoreTable;

	class DataStoreINI;

	class DataStoreConfig
	{
	public:
		virtual ~DataStoreConfig(){}

		virtual bool isValid() = 0;
	};

	class DataStore
	{
	public:
		DataStore(DataStoreConfig * config_)
			: config(config_)
		{

		}
		virtual ~DataStore(){}
		
		virtual DataStoreConfig * getConfig(){return config;}

		virtual void execSQL(const char * sql) = 0;
		virtual void execSQL(const std::string sql) = 0;

		virtual void readTable(const char * sql, DataStoreTable & table) = 0;

		virtual bool connect() = 0;
		virtual bool disconnect() = 0;

		virtual bool connected() = 0;
		
		virtual DataStoreINI & getIniStore(const char * tablename) = 0;

	protected:
		DataStoreConfig * config;
	};

	class DataStoreINI : public TradingObject
	{
	public:
		DataStoreINI(DataStore * store_, const char * tablename_) :
		  store(store_), tablename(tablename_)
		{
		}

		virtual ~DataStoreINI(){}

	    virtual void writeData(const char * section, const char * key, void * buffer, int size) = 0;
		virtual void writeString( const char * section, const char * key, const char * val) = 0;
		virtual void writeString( const char * section, const char * key, std::string val) = 0;
		virtual void writeFloat( const char * section, const char * key, double val) = 0;
		virtual void writeFloat( const char * section, const char * key, float val) = 0;
		virtual void writeInt( const char * section, const char * key, int val) = 0;
		virtual void writeBool( const char * section, const char * key, bool val) = 0;
	
	    virtual int readData(const char * section, const char * key, void * buffer, int size) = 0;
		virtual std::string readString( const char * section, const char * key, const char * defVal) = 0;
		virtual double readFloat( const char * section, const char * key, double defVal) = 0;
		virtual float readFloat( const char * section, const char * key, float defVal) = 0;
		virtual int readInt( const char * section, const char * key, int defVal) = 0;
		virtual bool readBool( const char * section, const char * key, bool defVal) = 0;

	protected:
		DataStore * store;
		const char * tablename;
	};


	// Really Really Really simple class 

	class DataFieldBase
	{
	public:
		DataFieldBase(const char * fieldname_, int index_)
			: fieldname(fieldname_), index(index_)
		{
		}

		virtual ~DataFieldBase(){}

		inline const char * getFieldName() const {return fieldname;} 
		inline const int getIndex() const { return index; }
		
		virtual bool isNull() = 0;
		virtual std::string asString() = 0;
		virtual int asInteger() = 0;
		virtual double asFloat() = 0;

	protected:
		const char * fieldname;
		const int index;
	};

	class DataTable
	{
	public:
		DataTable(DataStore * store_, const char * tablename_);
		~DataTable();

		virtual void attach(DataFieldBase & field) = 0;

		virtual void open() = 0;
		virtual void close() = 0;
		virtual bool active() = 0;
		virtual void begin() = 0;
		virtual void end() = 0;
		virtual void next() = 0;
		virtual bool eof() = 0;

	protected:
		DataStore * store;
		const char * tablename;

	};
}


#endif