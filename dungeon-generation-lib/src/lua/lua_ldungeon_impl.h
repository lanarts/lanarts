/*
 * lua_ldungeon_impl.h
 *
 *  Created on: Jun 18, 2013
 *      Author: adomurad
 */

#ifndef LUA_LDUNGEON_IMPL_H_
#define LUA_LDUNGEON_IMPL_H_

#include "Map.h"

struct lua_State;
class LuaValue;
class MTwist;

namespace ldungeon_gen {
	MTwist& ldungeon_get_rng(lua_State* L);

	Selector lua_selector_get(LuaField args);
	Selector lua_selector_optional_get(LuaField args);

	/* Parses an 'operators' from a table. All fields are optional. */
	Operator lua_operator_get(LuaField args);
	Operator lua_operator_optional_get(LuaField args);

	Square lua_square_get(LuaStackValue args);

	ConditionalOperator lua_conditional_operator_get(LuaField args);
	ConditionalOperator lua_conditional_operator_optional_get(LuaField args);
}

#endif /* LUA_LDUNGEON_IMPL_H_ */
