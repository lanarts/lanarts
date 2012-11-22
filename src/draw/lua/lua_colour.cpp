/*
 * lua_colour.cpp:
 *  Bindings for Colour
 */

#include <common/lua/LuaValue.h>
#include <common/lua/luacpp.h>

#include "lua_colour.h"

#include "../colour_constants.h"

LUACPP_WRAP_AS_NUMARRAY2_IMPL(Colour, int, 1 /*allowed missing*/, 255 /*default*/);

void ldraw::lua_register_colour_constants(lua_State *L, const LuaValue& module) {
#define BIND_COLOUR(col) \
	module.get(L, #col) = col

	BIND_COLOUR(COL_GOLD);
	BIND_COLOUR(COL_YELLOW);
	BIND_COLOUR(COL_MUTED_YELLOW);
	BIND_COLOUR(COL_PALE_YELLOW);

	BIND_COLOUR(COL_LIGHT_RED);
	BIND_COLOUR(COL_PALE_RED);
	BIND_COLOUR(COL_RED);

	BIND_COLOUR(COL_MUTED_GREEN);
	BIND_COLOUR(COL_PALE_GREEN);
	BIND_COLOUR(COL_GREEN);

	BIND_COLOUR(COL_LIGHT_BLUE);
	BIND_COLOUR(COL_BLUE);
	BIND_COLOUR(COL_BABY_BLUE);
	BIND_COLOUR(COL_PALE_BLUE);

	BIND_COLOUR(COL_MEDIUM_PURPLE);

	BIND_COLOUR(COL_BLACK);
	BIND_COLOUR(COL_DARKER_GRAY);
	BIND_COLOUR(COL_DARK_GRAY);
	BIND_COLOUR(COL_GRAY);
	BIND_COLOUR(COL_MID_GRAY);
	BIND_COLOUR(COL_LIGHT_GRAY);
	BIND_COLOUR(COL_WHITE);

}