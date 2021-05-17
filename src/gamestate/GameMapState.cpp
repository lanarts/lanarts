/*
 * GameMapState.cpp:
 *  Contains game map (level) state information
 */

#include "objects/PlayerInst.h"

#include <lcommon/SerializeBuffer.h>

#include "GameMapState.h"

#include "GameState.h"
#include "GameLogger.h"

GameMapState::GameMapState(int levelid, ldungeon_gen::MapPtr source_map, const Size& size,
		bool wandering_flag, bool is_simulation) :
		_levelid(levelid),
		_steps_left(0),
		_source_map(source_map),
		_size(size),
		_tiles(Size(size.w / TILE_SIZE, size.h / TILE_SIZE)),
		_inst_set(size.w, size.h),
		_monster_controller(wandering_flag),
		_is_simulation(is_simulation) {
	_attraction_map.init(tiles().solidity_map());
}

GameMapState::~GameMapState() {
}

void GameMapState::copy_to(GameMapState & room) const {
	room.entrances = this->entrances; //Copy exits&entrances just in case
	room.exits = this->exits; //However we will typically copy_to just to synch
//	this->inst_set.copy_to(level.inst_set);
//	level.is_simulation = this->is_simulation;
//	tiles.copy_to(level.tiles);
//	this->mc.partial_copy_to(level.mc);
//	level.mc.finish_copy(&level);
//	level.is_simulation = this->is_simulation;
//	level.steps_left = this->steps_left;
}

GameMapState* GameMapState::clone() const {
	GameMapState* state = new GameMapState(_levelid, ldungeon_gen::MapPtr(),_size, _is_simulation);
	copy_to(*state);
	return state;
}

static void update_player_fields_of_view(GameState* gs) {
	perf_timer_begin(FUNCNAME);
	std::vector<PlayerInst*> players = gs->players_in_level();
	for (int i = 0; i < players.size(); i++) {
		players[i]->update_field_of_view(gs);
	}
	perf_timer_end(FUNCNAME);
}

void GameMapState::serialize(GameState* gs, SerializeBuffer& serializer) {
	serializer.write_container(exits);
	serializer.write_container(entrances);
    _source_map->serialize(serializer);
	serializer.write(_label);
	serializer.write(_levelid);
	serializer.write(_steps_left);
	serializer.write(_size);
	serializer.write(_is_simulation);
    serializer.write_container(attraction_map().attraction_map._internal_vector());
	tiles().serialize(serializer);
	game_inst_set().serialize(gs, serializer);
	monster_controller().serialize(serializer);
}

void GameMapState::deserialize(GameState* gs, SerializeBuffer& serializer) {
	serializer.read_container(exits);
	serializer.read_container(entrances);
	_source_map = ldungeon_gen::MapPtr(new ldungeon_gen::Map());
	_source_map->deserialize(serializer);
	serializer.read(_label);
	serializer.read(_levelid);
	serializer.read(_steps_left);
	serializer.read(_size);
	serializer.read(_is_simulation);
    serializer.read_container(attraction_map().attraction_map._internal_vector());

	tiles().deserialize(serializer);
	collision_avoidance().clear();
	game_inst_set().deserialize(gs, serializer);
	monster_controller().deserialize(serializer);
}

// Does an add_instance in this map
obj_id GameMapState::add_instance(GameState* gs, GameInst* inst) {
	GameMapState* current_level = gs->game_world().get_current_level();

	// 'Awake' this map
	gs->game_world().set_current_level(this);
	obj_id id = game_inst_set().add_instance(inst);
	inst->init(gs);
	event_log("Adding instance id: %d x: %f y: %f target_radius: %f depth %d",
			inst->id, inst->x, inst->y, inst->target_radius, inst->depth);
	gs->game_world().set_current_level(current_level);

	return id;
}

void GameMapState::step(GameState* gs, bool simulate_monsters) {
	const int STEPS_TO_SIMULATE = 1200; // 20 seconds

	bool game_has_players = !gs->player_data().all_players().empty();
	bool has_player_in_level = gs->player_data().level_has_player(id()) || !game_has_players;

	if (has_player_in_level) {
		_steps_left = STEPS_TO_SIMULATE;
	}
	if (_steps_left <= 0) {
		return;
	}
	perf_timer_begin(FUNCNAME);

	GameMapState* previous_level = gs->get_level();
	gs->set_level(this);

	update_player_fields_of_view(gs);
	if (simulate_monsters) {
		monster_controller().pre_step(gs);
	}
	game_inst_set().step(gs);
	tiles().step(gs);
	_steps_left--;

	gs->set_level(previous_level);

	perf_timer_end(FUNCNAME);
}

// Only for Lua use at the moment!
void GameMapState::draw(GameState* gs, bool reveal_all) {
	perf_timer_begin(FUNCNAME);

	tiles().pre_draw(gs, reveal_all);
	// Become current level
	GameMapState* previous_level = gs->get_level();
	gs->set_level(this);

	std::vector<GameInst*> safe_copy = game_inst_set().to_vector();
	for (size_t i = 0; i < safe_copy.size(); i++) {
		safe_copy[i]->draw(gs);
	}

	for (size_t i = 0; i < safe_copy.size(); i++) {
		safe_copy[i]->post_draw(gs);
	}

	monster_controller().post_draw(gs);
	if (!reveal_all) {
		tiles().post_draw(gs);
	}
	gs->set_level(previous_level);

	perf_timer_end(FUNCNAME);
}
