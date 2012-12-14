//#include "stdafx.h"


#include "luaxlib_dyn.h"
#include <windows.h>

lua_All_functions LuaFunctions; 

static int linked = 0;

int luaXlibLink()
{

	HMODULE module = GetModuleHandleA("lua5.1.dll");
	if(module == NULL)
	{
		module = GetModuleHandleA("Lua51.dll");
	}
	if(module == NULL)
	{
		module = GetModuleHandleA("Lua.dll");
	}
	if(module == NULL)
	{
		module = GetModuleHandleA(NULL);
	}

	if (module == NULL)
	{
		return 1;
	}

	
	// Xlib functions
	//luaL_register= (luaL_register_t)  GetProcAddress(module, "luaL_register");
	//luaL_checknumber = (luaL_checknumber_t) GetProcAddress(module, "luaL_checknumber") ;

	luaL_loadfunctions(module, &LuaFunctions, sizeof(lua_All_functions));

	return 0;
}