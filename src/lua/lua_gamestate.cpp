#include <lua/lunar.h>

#include "lua_api.h"

#include "../world/GameState.h"
#include "../world/GameLevelState.h"

#include "../world/objects/EnemyInst.h"
#include "../world/objects/PlayerInst.h"
#include "../world/objects/ProjectileInst.h"

static Stats* get_stats(GameInst* inst) {
	PlayerInst* p;
	EnemyInst* e;
//	if ((p = dynamic_cast<CombatGameInst*>(inst))) {
//		return &p->stats();
//	if ((e = dynamic_cast<EnemyInst*>(inst))) {
//		return &e->oldstats();
//	} else {
//		return NULL;
//	}
	return NULL;
}

class GameStateLuaBinding {
public:
	static const char className[];
	static Lunar<GameStateLuaBinding>::RegType methods[];

	GameStateLuaBinding(GameState* gs) :
			gs(gs) {
	}

	/*Takes GameInst object*/
	int remove_object(lua_State* L) {
		obj_id id = lua_gameinst_arg(L, 1);
		if (id > 0) {
			GameInst* inst = gs->get_instance(id);
			if (inst)
				gs->remove_instance(inst);
		}
		return 0;
	}
	int create_projectile(lua_State* L) {
		int nargs = lua_gettop(L);
		obj_id origin_id = lua_gameinst_arg(L, 1);

		lua_pushstring(L, "name");
		lua_gettable(L, 4);
		sprite_id sprite = get_sprite_by_name(lua_tostring(L, lua_gettop(L)));
		lua_pop(L, 1);


		bool bounce = nargs < 8 ? false : lua_toboolean(L, 8);
		int hits = nargs < 9 ? 1 : lua_tonumber(L, 9);
		obj_id target = nargs < 10 ? 0 : lua_gameinst_arg(L, 10);

		obj_id projectile_id = 0;
		GameInst* origin_obj = gs->get_instance(origin_id);
		Stats* s = get_stats(origin_obj);

		if (s != NULL) {
//			ProjectileInst(sprite_id sprite, obj_id originator, float speed, int range,
//					int damage, int x, int y, int tx, int ty, bool bounce = false,
//					int hits = 1, obj_id target = NONE);
			GameInst* inst = new ProjectileInst(sprite, origin_id, lua_tonumber(L, 5),
					lua_tonumber(L, 6), lua_tonumber(L, 7), origin_obj->x, origin_obj->y,
					lua_tonumber(L, 2), lua_tonumber(L, 3), bounce,
					hits, target);
			projectile_id = gs->add_instance(inst);
		}

		lua_push_gameinst(L, projectile_id);
		return 0;
	}

	int players_in_room(lua_State* L) {
		PlayerController& pc = gs->player_controller();
		lua_createtable(L, 0, 0);
		int table = lua_gettop(L);
		for (int i = 0; i < pc.player_ids().size(); i++) {
			lua_push_gameinst(L, pc.player_ids()[i]);
			//  lua_pushnumber(L, 2);
			lua_rawseti(L, table, i + 1);
		}
		return 1;
	}

	int rand_range(lua_State* L) {
		int nargs = lua_gettop(L);

		lua_pushnumber(L, 1.0);
		lua_rawget(L, 1);
		int min = lua_tonumber(L,lua_gettop(L));
		int max = min;
		lua_pop(L, 1);

		if (nargs > 1){
			lua_pushnumber(L, 2.0);
			lua_rawget(L, 1);
			max = lua_tonumber(L,lua_gettop(L));
			lua_pop(L, 1);
		}

		lua_pushnumber(L, gs->rng().rand(min, max+1));
		return 1;
	}
	GameState* game_state() {
		return gs;
	}
private:
	GameState* gs;
};

typedef GameStateLuaBinding bind_t;
typedef Lunar<GameStateLuaBinding> lunar_t;
typedef lunar_t::RegType meth_t;
#define LUA_DEF(m) meth_t(#m, &bind_t:: m)

static int lua_member_lookup(lua_State* L) {
#define IFLUA_NUM_MEMB_LOOKUP(n, m) \
		if (strncmp(cstr, n, sizeof(n))==0){\
		lua_pushnumber(L, m );\
	}

	bind_t* state = lunar_t::check(L, 1);
	GameState* gs = state->game_state();
	const char* cstr = lua_tostring(L, 2);
	GameView& view = gs->window_view();

	IFLUA_NUM_MEMB_LOOKUP("width", gs->width())
	else IFLUA_NUM_MEMB_LOOKUP("height", gs->height())
	else IFLUA_NUM_MEMB_LOOKUP("mouse_x", gs->mouse_x() + view.x)
	else IFLUA_NUM_MEMB_LOOKUP("mouse_y", gs->mouse_y() + view.y)
	else IFLUA_NUM_MEMB_LOOKUP("frame_number", gs->frame())
	else IFLUA_NUM_MEMB_LOOKUP("level_number", gs->level()->level_number)
	else IFLUA_NUM_MEMB_LOOKUP("monster_num", gs->monster_controller().number_monsters())
	else {
		lua_getglobal(L, bind_t::className);
		int tableind = lua_gettop(L);
		lua_pushvalue(L, 2);
		lua_gettable(L, tableind);
	}
	return 1;
}

meth_t bind_t::methods[] = {
		LUA_DEF(remove_object),
		LUA_DEF(create_projectile),
		LUA_DEF(players_in_room),
		LUA_DEF(rand_range),
		meth_t(0, 0) };

GameState* lua_get_gamestate(lua_State* L) {
	lua_getglobal(L, "world");
	int idx = lua_gettop(L);
	bind_t* obj = lunar_t::check(L, idx);
	lua_pop(L, 1);
	if (!obj)
		return NULL;
	return obj->game_state();
}

void lua_gamestate_bindings(GameState* gs, lua_State* L) {
	lunar_t::Register(L);

	luaL_getmetatable(L, bind_t::className);
//
	int tableind = lua_gettop(L);

	lua_pushstring(L, "__index");
	lua_pushcfunction(L, lua_member_lookup);
	lua_settable(L, tableind);

	lunar_t::push(L, new bind_t(gs), true);

	lua_setglobal(L, "world");
}
const char GameStateLuaBinding::className[] = "GameWorld";
