/*
 * GameMapState.h:
 *  Contains game map (level) data
 */

#ifndef GAMEMAPSTATE_H_
#define GAMEMAPSTATE_H_

#include <vector>

#include <lcommon/geometry.h>
#include <ldungeon_gen/Map.h>
#include <pathfind/AttractionMap.h>

#include "collision_avoidance/CollisionAvoidance.h"
#include "objects/MonsterController.h"
#include "pathfind/WanderMap.h"

#include "LuaDrawableQueue.h"
#include "GameInstSet.h"

#include "GameTiles.h"

#include "PlayerData.h"

class GameState;
class SerializeBuffer;

struct GameRoomPortal {
	Pos entrancesqr;
	Pos exitsqr; //0,0 if undecided
	GameRoomPortal(const Pos& entrance = Pos(), const Pos& exit = Pos()) :
					entrancesqr(entrance),
					exitsqr(exit) {
	}
};

class GameMapState {
public:
	GameMapState(level_id levelid, ldungeon_gen::MapPtr source_map, const Size& size,
			bool wandering_flag = true, bool is_simulation = false);
	~GameMapState();

	void copy_to(GameMapState & level) const;
	GameMapState* clone() const;

	int width() const {
		return _size.w;
	}
	int height() const {
		return _size.h;
	}

	Size size() const {
		return _size;
	}
	GameTiles& tiles() {
		return _tiles;
	}

	GameInstSet& game_inst_set() {
		return _inst_set;
	}
	int tile_width() const {
		return _size.w / TILE_SIZE;
	}

	int tile_height() const {
		return _size.h / TILE_SIZE;
	}

	level_id id() const {
                LANARTS_ASSERT(_levelid >= 0 && _levelid < (1<<22));
		return _levelid;
	}

	MonsterController& monster_controller() {
		return _monster_controller;
	}

	CollisionAvoidance& collision_avoidance() {
		return _collision_avoidance;
	}

	LuaDrawableQueue& drawable_queue() {
		return _drawable_queue;
	}

	obj_id add_instance(GameState* gs, GameInst* inst);
	template <typename T, typename ...Args>
	T* add_instance(GameState* gs, Args... args) {
		T* inst = new T(args...);
		add_instance(gs, inst);
		return inst;
	};

	void serialize(GameState* gs, SerializeBuffer& serializer);
	void deserialize(GameState* gs, SerializeBuffer& serializer);

	void step(GameState* gs, bool simulate_monsters = true);
	void draw(GameState* gs, bool reveal_all = false);

	std::string& label() {
		return _label;
	}

	// TODO: De-duplicate this (same logic in GameState, for now)

	bool tile_radius_test(int x, int y, int rad, bool issolid = true,
			int ttype = -1, Pos* hitloc = NULL) {
		return tiles().radius_test(Pos(x, y), rad, issolid, ttype, hitloc);
	}

	int object_radius_test(GameInst* obj, GameInst** objs, int obj_cap,
			col_filterf f, int x, int y, int radius) {
		return game_inst_set().object_radius_test(obj, objs, obj_cap, f, x, y, radius);
	}

	bool solid_test(GameInst* obj, int x, int y, int radius = -1) {
		return solid_test(obj, NULL, 0, NULL, x, y, radius);
	}

	bool solid_test(GameInst* obj, GameInst** objs = NULL, int obj_cap = 0,
			col_filterf f = NULL, int x = -1, int y = -1, int radius = -1) {
		int lx = (x == -1 ? obj->x : x), ly = (y == -1 ? obj->y : y);
		return tile_radius_test(lx, ly, radius == -1 ? obj->radius : radius)
				|| object_radius_test(obj, objs, obj_cap, f, x, y, radius);
	}
	ldungeon_gen::MapPtr source_map() {
	    return _source_map;
	}

    AttractionMap& attraction_map() {
        return _attraction_map;
    }

    int& vision_radius() {
        return _vision_radius;
    }

    template <typename T, typename Func>
    bool for_each(const Func& func) {
        return _inst_set.for_each<T>(func);
	}

public:
	std::vector<GameRoomPortal> exits, entrances;
private:
	std::string _label;
	level_id _levelid;
	int _steps_left;
	Size _size;
	GameTiles _tiles;
	GameInstSet _inst_set;
	AttractionMap _attraction_map;
	ldungeon_gen::MapPtr _source_map;
	MonsterController _monster_controller;
	CollisionAvoidance _collision_avoidance;
	/* Used to store dynamic drawable information */
	LuaDrawableQueue _drawable_queue;
    int _vision_radius = 7;
	bool _is_simulation;
};

#endif /* GAMEMAPSTATE_H_ */
