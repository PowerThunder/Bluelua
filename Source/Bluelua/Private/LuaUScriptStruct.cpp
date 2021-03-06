#include "LuaUScriptStruct.h"

#include "UObject/Class.h"
#include "UObject/UnrealType.h"

#include "Bluelua.h"
#include "lua.hpp"
#include "LuaState.h"
#include "LuaUStruct.h"

DECLARE_CYCLE_STAT(TEXT("ScriptStructPush"), STAT_ScriptStructPush, STATGROUP_Bluelua);
DECLARE_CYCLE_STAT(TEXT("ScriptStructConstruct"), STAT_ScriptStructConstruct, STATGROUP_Bluelua);

const char* FLuaUScriptStruct::USCRIPTSTRUCT_METATABLE = "UScriptStruct_Metatable";

FLuaUScriptStruct::FLuaUScriptStruct(UScriptStruct* InSource)
	: Source(InSource)
{

}

FLuaUScriptStruct::~FLuaUScriptStruct()
{

}

int FLuaUScriptStruct::Push(lua_State* L, UScriptStruct* InSource)
{
	SCOPE_CYCLE_COUNTER(STAT_ScriptStructPush);

	if (!InSource)
	{
		lua_pushnil(L);
		return 1;
	}

	FLuaState* LuaStateWrapper = FLuaState::GetStateWrapper(L);
	if (LuaStateWrapper && LuaStateWrapper->GetFromCache(InSource))
	{
		return 1;
	}

	void* Buffer = lua_newuserdata(L, sizeof(FLuaUScriptStruct));
	FLuaUScriptStruct* LuaUScriptStruct = new(Buffer) FLuaUScriptStruct(InSource);

	if (luaL_newmetatable(L, USCRIPTSTRUCT_METATABLE))
	{
		static struct luaL_Reg Metamethods[] =
		{
			{ "__call", Construct },
			{ "__tostring", ToString },
			{ NULL, NULL },
		};

		luaL_setfuncs(L, Metamethods, 0);
	}

	lua_setmetatable(L, -2);

	if (LuaStateWrapper)
	{
		LuaStateWrapper->AddToCache(InSource);
	}

	return 1;
}

int FLuaUScriptStruct::Construct(lua_State* L)
{
	SCOPE_CYCLE_COUNTER(STAT_ScriptStructConstruct);

	FLuaUScriptStruct* LuaUScriptStruct = (FLuaUScriptStruct*)luaL_checkudata(L, 1, USCRIPTSTRUCT_METATABLE);
	if (!LuaUScriptStruct->Source.IsValid())
	{
		return 0;
	}

	return FLuaUStruct::Push(L, LuaUScriptStruct->Source.Get());
}

int FLuaUScriptStruct::ToString(lua_State* L)
{
	FLuaUScriptStruct* LuaUScriptStruct = (FLuaUScriptStruct*)luaL_checkudata(L, 1, USCRIPTSTRUCT_METATABLE);

	lua_pushstring(L, TCHAR_TO_UTF8(*FString::Printf(TEXT("UScriptStruct[%s][%x]"), LuaUScriptStruct->Source.IsValid() ? *(LuaUScriptStruct->Source->GetName()) : TEXT("null"), LuaUScriptStruct->Source.Get())));

	return 1;
}
