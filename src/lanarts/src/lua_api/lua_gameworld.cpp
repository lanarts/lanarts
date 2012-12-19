/*
 * lua_gameworld.cpp:
 *  Query the state of the game world.
 */

#include <lua.hpp>
#include <luawrap/luawrap.h>
#include <luawrap/functions.h>
#include <luawrap/types.h>

#include <SDL.h>

#include "gamestate/GameState.h"
#include "gamestate/GameLevelState.h"
#include "gamestate/GameSettings.h"

#include "lua_newapi.h"
#include "lua_api.h"

static int level_regenerate(lua_State* L) {
	GameState* gs = lua_api::gamestate(L);
	int levelid = gs->get_level()->id();

	if (lua_gettop(L) > 0) {
		levelid = lua_tonumber(L, 1);
	}

	gs->game_world().regen_level(levelid);
	return 0;
}

static int objects(lua_State* L) {
	GameState* gs = lua_api::gamestate(L);
	gs->get_level()->game_inst_set().to_vector();
	return 0;

}

namespace lua_api {
	void register_gameworld_api(lua_State* L) {
		LuaValue globals = luawrap::globals(L);
		luawrap::install_type<GameInst*>();

		LuaValue world = globals["world"].ensure_table();
		LuaValue level = globals["level"].ensure_table();

		level["regenerate"].bind_function(level_regenerate);
	}
}
