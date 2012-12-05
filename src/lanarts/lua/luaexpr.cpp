/*
 * luaexpr.cpp:
 *  Function for calling a function and storing the result.
 */

#include <cstdio>
#include <common/lua/LuaValue.h>
#include <common/perf_timer.h>

#include "../lanarts_defines.h"
#include "luaexpr.h"

extern "C" {
#include <lua/lua.h>
}

void luavalue_call_and_store(lua_State* L, LuaValue& eval) {
	perf_timer_begin(FUNCNAME);

	if (eval.empty()) {
		perf_timer_end(FUNCNAME);
		return;
	}

	eval.push(L);

	if (lua_pcall(L, 0, 1, 0 /*no errfunc*/)) {
		printf("Error while evaluating lua expression:\n%s\n",
				lua_tostring(L, -1));
		exit(0);
	}

	eval.pop(L);
	perf_timer_end(FUNCNAME);
}