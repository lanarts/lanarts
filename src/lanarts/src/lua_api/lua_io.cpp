/*
 * lua_io.cpp:
 *  Bindings for checking IO states
 */

#include <lua.hpp>
#include <luawrap/luawrap.h>
#include <luawrap/functions.h>

#include <SDL.h>

#include "gamestate/GameState.h"

#include "lua_newapi.h"

// Lets meet lua half-way, we accept LuaStackValue which is an efficient way
// of saying we take any lua value. This also lets us access our LuaState!

static int lua_tokeycode(const LuaStackValue& value) {
	lua_State* L = value.luastate();
	int idx = value.index();

	if (lua_isnumber(L,idx)) {
		return lua_tonumber(L, idx);
	}

	if (lua_isstring(L, idx)) {
		size_t size;
		const char* str = lua_tolstring(L, idx, &size);
		if (size != -1) {
			luaL_error(L, "Expected key number or one-character string for keycode, but got \"%s\".", str);
			return 0;
		}
		return tolower(str[0]);
	}

	luaL_error(L, "Expected key number or one-character string for keycode, but got %s.", lua_typename(L, idx));
	return 0;
}

static bool key_pressed(const LuaStackValue& value) {
	GameState* gs = lua_api::gamestate(value.luastate());
	return gs->key_press_state(lua_tokeycode(value));
}
static bool key_held(const LuaStackValue& value) {
	GameState* gs = lua_api::gamestate(value.luastate());
	return gs->key_down_state(lua_tokeycode(value));
}

namespace lua_api {
	void register_io_api(lua_State* L) {
		LuaValue globals = luawrap::globals(L);

		globals["key_pressed"].bind_function(key_pressed);
		globals["key_held"].bind_function(key_held);

		LuaValue keys = globals["keys"].ensure_table();

		keys["ENTER"] = (int)SDLK_RETURN;
		keys["ESCAPE"] = (int)SDLK_ESCAPE;
		keys["SPACE"] = (int)SDLK_SPACE;
		keys["DELETE"] = (int)SDLK_DELETE;
		keys["UP"] = (int)SDLK_UP;
		keys["DOWN"] = (int)SDLK_DOWN;
		keys["RIGHT"] = (int)SDLK_RIGHT;
		keys["LEFT"] = (int)SDLK_LEFT;
		keys["INSERT"] = (int)SDLK_INSERT;
		keys["HOME"] = (int)SDLK_HOME;
		keys["END"] = (int)SDLK_END;
		keys["PAGEUP"] = (int)SDLK_PAGEUP;
		keys["PAGEDOWN"] = (int)SDLK_PAGEDOWN;
		keys["F1"] = (int)SDLK_F1;
		keys["F2"] = (int)SDLK_F2;
		keys["F3"] = (int)SDLK_F3;
		keys["F4"] = (int)SDLK_F4;
		keys["F5"] = (int)SDLK_F5;
		keys["F6"] = (int)SDLK_F6;
		keys["F7"] = (int)SDLK_F7;
		keys["F8"] = (int)SDLK_F8;
		keys["F9"] = (int)SDLK_F9;
		keys["F10"] = (int)SDLK_F10;
		keys["F11"] = (int)SDLK_F11;
		keys["F12"] = (int)SDLK_F12;
		keys["F13"] = (int)SDLK_F13;
		keys["F14"] = (int)SDLK_F14;
		keys["F15"] = (int)SDLK_F15;
	}
}