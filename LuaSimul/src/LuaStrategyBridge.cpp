////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	License
//	-------
//
//	Mervin is made available under both the GPL v2 license and a BSD (3-point) style license. 
//  You can select which one you wish to use the DataTables code under.
//
//	Copyright (c) 2008-2012, Daniel Flam of NewYorkBrass.com
//	All rights reserved.
//
//	Redistribution and use in source and binary forms, with or without modification, 
//  are permitted provided that the following conditions are met:
//
//  Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
//  Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the 
//  following disclaimer in the documentation and/or other materials provided with the distribution.
//  Neither the name of Daniel Flam nor NewYorkBrass.com may be used to endorse or promote products derived from this software 
//  without specific prior written permission. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS "AS IS" AND ANY EXPRESS OR 
//  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
//  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, 
//  OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
//  OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
//  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// LuaStrategyBridge.cpp : Defines the exported functions for the DLL application.
//
//
// This file is an example of a standalone app that drives ticks into a user defined system
// It also interfaces with LUA allowing the system to read parameters and be controlled by a
// LUA script
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//#include "vld.h" 
#include <lua_dyn.h>
#include "luaxlib_dyn.h"
//#include <lualib.h>


#include "indicator.h"
#include "fastjulian.h"

#include <time.h>

//#include "tracetool.h" 

#include <fstream>
#include <sstream>
//#include "boost/format.hpp"
//#include "formatHelper.h"
#include <boost/algorithm/string.hpp>
#include "executionSimulator.h"
#include "sqlitePortfolio.h"
#include "sqliteDataStore.h"


using namespace indicator;

// This registers and initializes your trading script
#include "MyTradingScript.h"
static indicator::RegisterClassScript<indicator::MyTradingScript> script("MyTradingScript");


static Logger * logger;

TradingSession * LuaTradingSession;

std::list<string> symbolnames;

extern "C" {

#define LUALIB_EXPORT __declspec(dllexport)
#define MY_LUALIB_API static

	MY_LUALIB_API int lua_getSymbolID(lua_State *L)
	{
		string symbolname = luaL_checkstring(L, 1);
				

		lua_pushnumber(L, LuaTradingSession->getSymbolManager()->getSymbolId(symbolname) );

		return 1;
	}

	MY_LUALIB_API int lua_registerSymbol(lua_State *L)
	{
		string symbolname = luaL_checkstring(L, 1);
		int symbolID = luaL_checkint(L, 2);
				

		lua_pushnumber(L, LuaTradingSession->getSymbolManager()->registerSymbol(symbolname,symbolID) );

		return 1;
	}

	MY_LUALIB_API int lua_getScriptInstance(lua_State *L)
	{
		ChartScriptFactoryManager & manager = ChartScriptFactoryManager::getInstance();

		long id = (long)luaL_checknumber(L, 1);

		//	string msg = "Preparing instance ";
		//	msg  += itoa(id, );

		//string msg = boost::str( boost::format( "Looking up instance of '%i'" ) % id );


		//	log("BRIDGE","Looking up instance of '%i'", id );

		if (id < 0)
		{
			string scriptname = luaL_checkstring(L, 2);
			string symbolname = luaL_checkstring(L, 3);

			LOG(LT_DEBUG, "BRIDGE","Not found, requesting new instance of '%s' on symbol '%s'", scriptname, symbolname );

			ChartScript * p = manager.createScriptInstance(scriptname, LuaTradingSession);
			if (p)
			{
				p->setSymbol(symbolname);

				id = p->getInstanceID();
			}
		}
		lua_pushnumber(L, id);

		return 1;
	}

	MY_LUALIB_API int lua_releaseScriptInstance(lua_State *L)
	{

		int id = (int)luaL_checknumber(L, 1);

		LuaTradingSession->removeScript(id);

		return 0;
	}

	MY_LUALIB_API int lua_updateParameter(lua_State *L)
	{
		//ChartScriptRunner & runner = ChartScriptRunner::getInstance();

		long id = luaL_checkinteger(L, 1);

		//	log("BRIDGE", "Looking up instance of '%i'", id );
		//	indicator::ChartScriptPtr script = runner.getScriptInstance(id);

		string section = luaL_checkstring(L, 2);
		string name = luaL_checkstring(L, 3);
		//	long paramtype = luaL_checkinteger(L, 3);

		double dval;
		string sval;


		ParamManager & paramManager = *LuaTradingSession->getParamManager();

		switch (lua_type(L, 4)) 
		{
		case LUA_TNUMBER: 
			dval = luaL_checknumber(L, 4);
			paramManager.setParam(id, section, name, dval);
			//log("BRIDGE", "Updateing '%d,%s,%s'='%f'", paramname, dval );
			//script->updateParam(paramname, dval);
			break;

		case LUA_TSTRING:
			sval = luaL_checkstring(L, 4);
			paramManager.setParam(id, section, name, sval);
			//log("BRIDGE", "Updateing '%s,%s'='%s'", paramname, sval );
			//script->updateParam(paramname, sval);
			break;

		case LUA_TBOOLEAN: 
			dval = lua_toboolean(L, 4) > 0 ? 1.0 : 0.0;
			paramManager.setParam(id, section, name, dval);
			break;

			//	case LUA_TBOOLEAN:
			//		luaL_
		default:
			LOG(LT_DEBUG, "BRIDGE", "Param '%s': Unknown param type '%d'", name, lua_type(L, 4)  );
		}

		return 0;
	}

	MY_LUALIB_API int lua_parameterBeginUpdate(lua_State *L)
	{

		ParamManager & paramManager = *LuaTradingSession->getParamManager();
		paramManager.beginUpdate();

		return 0;
	}

	MY_LUALIB_API int lua_parameterEndUpdate(lua_State *L)
	{

		ParamManager & paramManager = *LuaTradingSession->getParamManager();
		paramManager.endUpdate();

		return 0;
	}

	MY_LUALIB_API int lua_updateTick(lua_State *L)
	{
		//ChartScriptRunner & runner = ChartScriptRunner::getInstance();

		long id = luaL_checkinteger(L, 1);

		//log("BRIDGE", "Looking up instance of '%i'", id );
		ChartScript * script = LuaTradingSession->getScript(id);

		//log("BRIDGE", "Filling in tick data" );

		Tick tick;

		tick.time = luaL_checknumber(L, 2);
		tick.bid = luaL_checknumber(L, 3);
		tick.ask= luaL_checknumber(L, 4);

		tick.close = (tick.bid + tick.ask)/2 ;
		tick.open = tick.close;
		tick.high = tick.close;
		tick.low = tick.close;


		tick.volume = 1;

		//	log("BRIDGE", "Executing script %i", id );

		script->updateTick(&tick);

		return 0;
	}

	struct TickCacheHeader
	{
		unsigned char ID[8];
		time_t cachetime;
	};

	struct TickCacheData
	{
		int symbolID;
		double when;
		double bid;
		double ask;
	};

	class BidAskStreams 
	{
	public:

		BidAskStreams(int symbolID_, string bidfn_, string askfn_, string cachefn_, bool bypasscache)
		{
			symbolID = symbolID_;
			currentTick.symbolID = symbolID_;
			refcount = 1;
			validtick = false;
			startdate = 0;
			enddate = 999999999;

			tickcacheout  = 0;
			bidfs =0;
			askfs = 0;

			bidwhen = 0;
			bidval = std::numeric_limits<double>::quiet_NaN();
			askwhen = 0;
			askval = std::numeric_limits<double>::quiet_NaN();
			//bypasscache = true;
			tickcache = bypasscache ? 0 : new std::ifstream(cachefn_.c_str(), std::ifstream::binary);
			if (!tickcache || tickcache->fail())
			{
				if (tickcache)
				{
					delete tickcache;
					tickcache = 0;
				}
				tickcacheout = new std::ofstream(cachefn_.c_str(), std::ofstream::binary);
				
				bidfs = new std::ifstream(bidfn_.c_str(), std::ifstream::binary);
		//		askfs = new std::ifstream(askfn_.c_str(), std::ifstream::binary);
			}
			eof = ! ( tickcache || (tickcacheout && bidfs));
		}

		~BidAskStreams()
		{
			close();
		}

		void close()
		{
			if (bidfs) 
				delete bidfs;
			if (askfs) 
				delete askfs;
			if (tickcache) 
				delete tickcache;
			if (tickcacheout) 
				delete tickcacheout;
		}

		void release()
		{
			--refcount;
			if (refcount == 0)
				delete this;
		}

		bool ReadTick(int & symbolID_, double & when_, double & bidval_, double & askval_)
		{
			static double lastticktime = 0;
			if (tickcache)
			{
				eof = tickcache->eof();
				if (eof)
				{
					symbolID = -1;
					when_ =   std::numeric_limits<double>::quiet_NaN();
					bidval_ = std::numeric_limits<double>::quiet_NaN();
					askval_ = std::numeric_limits<double>::quiet_NaN();
				}
				else
				{
					TickCacheData tick;
					tickcache->read((char*)&tick,   sizeof(TickCacheData));				

					symbolID_ = tick.symbolID;
					when_ = tick.when;
					bidwhen = when_;
					askwhen = when_;

					bidval_ = tick.bid;
					bidval=bidval_;
					askval_ = tick.ask;
					askval = askval_;
				}

				return eof;
			}


			eof = bidfs->eof();

			if (eof)
			{
				symbolID = -1;
				when_ =   std::numeric_limits<double>::quiet_NaN();
				bidval_ = std::numeric_limits<double>::quiet_NaN();
				askval_ = std::numeric_limits<double>::quiet_NaN();
			}
			else
			{
				
				std::string sym;
				double i= 2455701, j, v1, v2;
				when_ = lastticktime; 
				bool found = false;
				char comma;
				while (!eof && !found)
				{
					std::getline(*bidfs, sym, ',');
					*bidfs >> i >> comma >> j >> comma >> bidval_ >> comma >> v1 >> comma >> askval_ >> comma >> v2;

					if (i < 2455700 && bidval_ > 0 && askval_ > 0)
					{
	//					i -= 2415019;

						when_= i + j/86400000.0;
						if (when_ >= lastticktime)
						{
							boost::trim(sym);
							symbolID_ = LuaTradingSession->getSymbolManager()->findSymbolId(sym);
							if (symbolID_ > -1)
							{
								lastticktime = when_;

								found = true;
							}
						}
					}
					else
					{
						eof = bidfs->eof();
					}
				}


			}

			if (!eof)
			{
				TickCacheData tick;
				tick.symbolID = symbolID_;
				tick.when = when_;
				tick.bid = bidval_;
				tick.ask = askval_;
				// if you want to cache the tick data for faster retrieval
				//	tickcacheout->write((char*)&tick, sizeof(TickCacheData));
			}

			return eof;
		}


		bool getNextValidTick()
		{
			double when;
			double bid;
			double ask;
			
			validtick = false;
			while (!validtick)
			{
				if (eof)
				return false;

				prevTick = currentTick;

				validtick = !ReadTick(symbolID, when, bid, ask) && when <= enddate;
				if (validtick)
				{
					if (when >= startdate)
					{
						currentTick.newId();
						currentTick.symbolID =  symbolID;
						currentTick.time = when;
						currentTick.bid = bid;
						currentTick.ask= ask;
						currentTick.close = (currentTick.bid + currentTick.ask)/2 ;
						currentTick.open = currentTick.close;
						currentTick.high = currentTick.close;
						currentTick.low = currentTick.close;
						currentTick.volume = 1;
					}
					else 
						validtick = false;
				}

				if (!validtick)
				{
					prevTick.lastone = true;
				}
				
			}

			return validtick;
		}

		Tick prevTick;
		Tick currentTick;
		bool validtick;
		double startdate, enddate;

		int refcount;
		bool eof;
		double bidwhen;
		double bidval;
		double askwhen;
		double askval;
		int symbolID;

		string cachefn;

		std::ifstream * bidfs;
		std::ifstream * askfs;
		std::ifstream * tickcache;
		std::ofstream * tickcacheout;

	};

	managedList<BidAskStreams> streamList;


	MY_LUALIB_API int lua_opentickstream (lua_State *L) 
	{

		string symbolname = luaL_checkstring(L, 1);
		string bidfn = luaL_checkstring(L, 2);
		string askfn = luaL_checkstring(L, 3);
		string tickcachefn = luaL_checkstring(L, 4);
		bool bypasscache = (lua_type(L, 5) == LUA_TBOOLEAN) && lua_toboolean(L, 5);

//		int symbolID = LuaTradingSession->getSymbolManager()->getSymbolId( symbolname );

		BidAskStreams * streams = new BidAskStreams(-1, bidfn, askfn, tickcachefn, bypasscache);
		streamList.push_back(streams);

		int ID = (int)streams;
		//return handle to file as int
		lua_pushinteger(L,  ID);



		return 1;

	}

	MY_LUALIB_API int lua_readtick (lua_State *L) {

		BidAskStreams * streams = (BidAskStreams *)luaL_checkinteger(L, 1);

		bool eof;

		int symbol;
		double when;
		double bid, ask;

		eof = streams->ReadTick(symbol,when, bid, ask);

		lua_pushboolean(L, eof);
		lua_pushnumber (L, symbol);
		lua_pushnumber (L, when);
		lua_pushnumber (L, bid);
		lua_pushnumber (L, ask);

		return 5;
	}

	MY_LUALIB_API int lua_closetickstream (lua_State *L) 
	{

		BidAskStreams * streams = (BidAskStreams *)luaL_checkinteger(L, 1);
		streamList.remove(streams);

		return 0;
	}


	MY_LUALIB_API int lua_split (lua_State *L) {
		const char *s = luaL_checkstring(L, 1);
		const char *sep = luaL_checkstring(L, 2);
		const char *e;
		int i = 1;

		lua_newtable(L);  /* result */

		/* repeat for each separator */
		while ((e = strchr(s, *sep)) != NULL) {
			lua_pushlstring(L, s, e-s);  /* push substring */
			lua_rawseti(L, -2, i++);
			s = e + 1;  /* skip separator */
		}

		/* push last substring */
		lua_pushstring(L, s);
		lua_rawseti(L, -2, i);

		return 1;  /* return the table */
	}


	MY_LUALIB_API int lua_init (lua_State *L) 
	{
		LuaTradingSession = new DefaultTradingSession();
		// This must be called as first logger instance!
		//LuaTradingSession->setLogger(new indicator::FileLogger(LuaTradingSession, "alllogs", -1, ".txt"));
		LuaTradingSession->setLogger(new indicator::FileLogger(LuaTradingSession, new OnFileLoggerStartEvent("alllogs")));
		LuaTradingSession->setExecutionHandler( new indicator::ExecutionSimulator(LuaTradingSession) );
		LuaTradingSession->setPortfolio( new indicator::SQLitePortfolio(LuaTradingSession));

		logger = new indicator::FileLogger(LuaTradingSession, new OnFileLoggerStartEvent("luabridge"));
		LuaTradingSession->setDataStore( 
			new indicator::SqliteDataStore(
				new indicator::SqliteDataStoreConfig(
					"trader.sqlite", 
					6 /*SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE*/, 
					4000
				)
			)
		);


		return 0;
	}

	MY_LUALIB_API int lua_cleanup (lua_State *L) 
	{
		if (logger)
		{
			delete logger;
			logger = 0;
		}
		if (LuaTradingSession)
		{
			delete LuaTradingSession;
			LuaTradingSession = 0;
		}
		streamList.clear();
		return 0;
	}


	MY_LUALIB_API int lua_runstrategy(lua_State *L) 
	{
		double startdate = lua_type(L, 1) == LUA_TNUMBER ?  luaL_checknumber(L,1) : 0;
		double enddate = lua_type(L, 2) == LUA_TNUMBER ?  luaL_checknumber(L,2) : 999999999; // extinction of humanity bug.

		std::stringstream ss1;
		ss1 << "Starting run from " << startdate << " to " << enddate;
		LOG(LT_DEBUG, "BRIDGE", "%s", ss1.str());



		for (
			std::vector<BidAskStreams*>::iterator it = streamList.list.begin();
			it != streamList.list.end();
		it++)
		{
			(*it)->startdate = startdate;
			(*it)->enddate = enddate;
		}

		
		LuaTradingSession->start();

		LuaTradingSession->getLogger()->disable();
		
		BidAskStreams * s = streamList.list.front();

		s->getNextValidTick();

		int tickcount = 0;
		time_t start,end;
		time(&start);
		do
		{
			s->getNextValidTick();
			tickcount++;

			LuaTradingSession->updateTick(&s->prevTick);
		}
		while (!s->prevTick.lastone);
		time(&end);

		std::stringstream ss;
		double dif = difftime (end,start);
		ss << "Completed " << tickcount << " ticks in " << dif << " seconds, processing " << dif/double(tickcount);
		
		LOG(LT_DEBUG, "BRIDGE", "%s", ss.str());

		return 0;
	}

	MY_LUALIB_API int lua_DMY2DateTime(lua_State *L) 
	{
		unsigned short y = (unsigned short )luaL_checknumber(L, 1);
		unsigned short m = (unsigned short )luaL_checknumber(L, 2);
		unsigned short d = (unsigned short )luaL_checknumber(L, 3);

		lua_pushnumber(L,  fastjulian::DateTimeFromDMY(y, m, d));

		return 1;
	}

	/////////////////////////////////////////////////////////////
	//
	//
	// LUA Register
	//
	//
	/////////////////////////////////////////////////////////////

	static const luaL_Reg register_module[] = 
	{
		{ "init", lua_init },
		{ "cleanup", lua_cleanup },
		{ "getScriptInstance",		lua_getScriptInstance },
		{ "releaseScriptInstance",		lua_releaseScriptInstance },
		{ "updateTick", lua_updateTick },
		{ "parameterUpdate", lua_updateParameter },
		{ "parameterBeginUpdate", lua_parameterBeginUpdate },
		{ "parameterEndUpdate", lua_parameterEndUpdate },
		{ "openTickStream", lua_opentickstream},
		{ "closeTickStream", lua_closetickstream},
		{ "readTickStream", lua_readtick},
		{ "runStrategy", lua_runstrategy},
		{ "split", lua_split },
		{ "DMY2DateTime", lua_DMY2DateTime},
		{ "getSymbolID", lua_getSymbolID},
		{ "registerSymbol", lua_registerSymbol},

		{ NULL,				NULL	}
	};

	int LUALIB_EXPORT luaopen_LuaSimul(lua_State *l)
	{

		luaXlibLink();


		luaL_register(l, "LuaSimul", register_module);


		return 1;

	}



}
