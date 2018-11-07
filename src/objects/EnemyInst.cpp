/*
 * EnemyInst.cpp:
 *  Represents an AI controlled combat entity
 */

#include <cmath>
#include <typeinfo>

#include <luawrap/LuaValue.h>
#include <luawrap/calls.h>
#include <lcommon/SerializeBuffer.h>

#include <ldraw/draw.h>
#include <ldraw/Font.h>
#include <ldraw/DrawOptions.h>

#include "lua_api/lua_api.h"

#include "draw/colour_constants.h"
#include "draw/draw_sprite.h"
#include "draw/SpriteEntry.h"

#include "gamestate/GameState.h"
#include "interface/console_description_draw.h"

#include "stats/items/WeaponEntry.h"
#include "stats/stat_formulas.h"

#include <lcommon/math_util.h>
#include "objects/PlayerInst.h"

#include "objects/AnimatedInst.h"

#include "objects/ItemInst.h"

#include "objects/ProjectileInst.h"
#include "objects/collision_filters.h"

#include "objects/EnemyInst.h"
#include "objects/EnemyEntry.h"

//draw depth, also determines what order objects evaluate in
static const int DEPTH = 50;

static EnemyEntry& __E(enemy_id enemytype) {
	return game_enemy_data.get(enemytype);
}

float monster_difficulty_multiplier(GameState* gs, EnemyEntry& etype) {
	size_t size = gs->player_data().all_players().size();
	float mult = (size - 1) / 4.0f;//NB: natural log, base e ~ 2.718...
	if (etype.unique) {
		return 1 + mult * 2; // Can reasonably expect all players to be part of a boss fight
	}
	return 1 + mult;
}

EnemyInst::EnemyInst(int enemytype, int x, int y, int team) :
				CombatGameInst(__E(enemytype).basestats,
						__E(enemytype).enemy_sprite, Pos(x, y), team,
						__E(enemytype).radius, true, DEPTH) {
	this->seen = false;
	this->xpgain = __E(enemytype).xpaward;
    this->vision_radius = __E(enemytype).vision_radius;
	this->enemytype = enemytype;
	this->enemy_regen_cooloff = 0;
}

EnemyInst::~EnemyInst() {

}

static void combine_hash(unsigned int& hash, unsigned int val1, unsigned val2) {
	hash ^= (hash >> 11) * val1;
	hash ^= val1;
	hash ^= (hash >> 11) * val2;
	hash ^= val2;
	hash ^= hash << 11;
}

void EnemyInst::signal_attacked_successfully() {
	eb.randomization.successful_hit_timer = 0;
}

unsigned int EnemyInst::integrity_hash() {
	unsigned int hash = CombatGameInst::integrity_hash();
	combine_hash(hash, eb.current_node, eb.path_steps);
	combine_hash(hash, eb.path_start.x, eb.path_start.y);
// TODO: hash the content of the simulation object, the id is a bad thing to hash
//	combine_hash(hash, collision_simulation_id(), eb.current_action);
	return hash;
}

void EnemyInst::serialize(GameState* gs, SerializeBuffer& serializer) {
	CombatGameInst::serialize(gs, serializer);
	serializer.write(seen);
	serializer.write_int(enemytype);
	eb.serialize(gs, serializer);
	serializer.write_int(enemy_regen_cooloff);
//	ai_state.serialize(gs, serializer);
}

void EnemyInst::deserialize(GameState* gs, SerializeBuffer& serializer) {
	CombatGameInst::deserialize(gs, serializer);
	serializer.read(seen);
	serializer.read_int(enemytype);
	eb.deserialize(gs, serializer);
	serializer.read_int(enemy_regen_cooloff);
        if (!destroyed) {
            // We care about the details of destroyed objects
            // but they cant add themselves to collision avoidance.
            CollisionAvoidance& coll_avoid = gs->collision_avoidance();
            collision_simulation_id() = coll_avoid.add_active_object(ipos(),
                            target_radius, effective_stats().movespeed);
        }
//	ai_state.deserialize(gs, serializer);
}

bool EnemyInst::damage(GameState* gs, float dmg, CombatGameInst* attacker) {
	eb.damage_was_taken();
	enemy_regen_cooloff += dmg;
	return CombatGameInst::damage(gs, dmg, attacker);
}

EnemyEntry& EnemyInst::etype() {
	return game_enemy_data.get(enemytype);
}

void EnemyInst::init(GameState* gs) {
	CombatGameInst::init(gs);
	MonsterController& mc = gs->monster_controller();
	mc.register_enemy(this);

	CollisionAvoidance& coll_avoid = gs->collision_avoidance();
	collision_simulation_id() = coll_avoid.add_active_object(ipos(),
			target_radius, effective_stats().movespeed);

	lua_State* L = gs->luastate();

	float diffmult = monster_difficulty_multiplier(gs, etype());
	core_stats().hp = static_cast<int>(ceil(core_stats().hp * diffmult));
	core_stats().max_hp = static_cast<int>(ceil(core_stats().max_hp * diffmult));

	lcall(etype().init_event, this);
	lua_api::event_monster_init(L, this);
}
static bool starts_with_vowel(const std::string& name) {
	char c = tolower(name[0]);
	return (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u');
}
static void show_appear_message(GameChat& chat, EnemyEntry& e) {
	if (e.appear_msg.empty()) {
		char buff[100];
		const char* a_or_an = starts_with_vowel(e.name) ? "An " : "A ";
		snprintf(buff, 100, "%s%s appears!", a_or_an, e.name.c_str());
		chat.add_message(buff, COL_PALE_RED);
	} else {
		chat.add_message(e.appear_msg.c_str(), COL_PALE_RED);
	}
}
static void show_defeat_message(GameChat& chat, EnemyEntry& e) {
	if (!e.defeat_msg.empty()) {
		chat.add_message(e.defeat_msg, COL_MUTED_GREEN);
	}
}

void EnemyInst::step(GameState* gs) {
	//Much of the monster implementation resides in MonsterController
        if (has_paths_data()) {
            paths_to_object().fill_paths_in_radius(ipos(), PLAYER_PATHING_RADIUS);
        }

        if (field_of_view) {
            int sx = x / TILE_SIZE;
            int sy = y / TILE_SIZE;
            field_of_view->calculate(gs, vision_radius, sx, sy);
        }
// XXX: Make the monster health absorbing way less hackish and more general
	int hp_before = stats().core.hp;

	CombatGameInst::step(gs);

    // Regenerate much quicker if the player left the level:
	if (!gs->player_data().level_has_player(gs->get_level_id())) {
	    core_stats().heal_hp(effective_stats().core.hpregen * 7, effective_stats().core.max_hp);
	}
    // Absorb health regen if recently damaged
	int hp_gain = std::max(0, stats().core.hp - hp_before);
	int hp_cooloff = std::min(enemy_regen_cooloff, hp_gain);
	enemy_regen_cooloff -= hp_cooloff;
	stats().core.hp -= hp_cooloff;
	effective_stats().core.hp = stats().core.hp;

	update_position();

	gs->for_screens( [&]() {
		if (!seen && team == MONSTER_TEAM && gs->object_visible_test(this, gs->local_player())) {
			seen = true;
			gs->enemies_seen().mark_as_seen(enemytype);
					if (gs->local_player()->current_floor == current_floor) {
						play(etype().unique ? "sound/boss_appears.ogg" : "sound/see_monster.ogg");
					}
			show_appear_message(gs->game_chat(), etype());
		}
	});
}
void EnemyInst::post_draw(GameState* gs) {
	CombatGameInst::post_draw(gs);
	lcall(etype().on_post_draw, this);
}
void EnemyInst::draw(GameState* gs) {
	GameView& view = gs->view();
	ldraw::Drawable& spr = res::sprite(etype().enemy_sprite);

	if (gs->game_settings().draw_diagnostics) {
		char statbuff[255];
		snprintf(statbuff, 255,
				"simid=%d nvx=%f vy=%f\n chasetime=%d \n mdef=%d pdef=%d", // \n act=%d, path_steps = %d\npath_cooldown = %d\n",
				simulation_id, vx, vy, eb.chase_timeout,
				(int) effective_stats().core.willpower,
				(int) effective_stats().core.defence);
		//eb.current_action,
		//eb.path_steps, eb.path_cooldown);
		gs->font().draw(COL_WHITE, Pos(x - radius - view.x, y - 70 - view.y),
				statbuff);
	}

	int w = spr.size().w, h = spr.size().h;
	int xx = x - w / 2, yy = y - h / 2;

	if (!view.within_view(xx, yy, w, h))
		return;
	if (!gs->object_visible_test(this))
		return;

	float frame = gs->frame();
	if (etype().name == "Hydra") {
		frame = floor(core_stats().hp / float(core_stats().max_hp) * 4);
		if (frame >= 4)
			frame = 4;
	}
	CombatGameInst::draw(gs, frame);

	lcall(etype().draw_event, this);

	if (team == PLAYER_TEAM && !effects.has("DiesOnEndOrFirstAttack")) {
            res::sprite("spr_effects.friendly").draw(on_screen(gs, PosF {x-16, y-16}));
            res::sprite("spr_amulets.i-faith").draw(on_screen(gs, PosF {x-23, y-23}));
	}
}

EnemyInst* EnemyInst::clone() const {
	return new EnemyInst(*this);
}

bool EnemyInst::within_field_of_view(const Pos & pos) {
	return distance_between(Pos(x, y), pos) <= 100;
}

const double deg2rad = 3.14159265 / 180.0;

void EnemyInst::die(GameState* gs) {
    auto* curr_level = gs->get_level();
    gs->set_level(get_map(gs));
    if (!destroyed) {
		lcall(etype().death_event, this);
        lua_api::event_monster_death(gs->luastate(), this);
        gs->player_data().n_enemy_killed(enemytype)++;
        AnimatedInst* anim = new AnimatedInst(ipos(), etype().enemy_sprite, 20);
        anim->frame(0);
        gs->add_instance(anim);
        gs->remove_instance(this);
        if (team != PLAYER_TEAM) {
            if (gs->object_visible_test(this)) {
                play("sound/paind.ogg");
            }
        }

        MTwist& mt = gs->rng();
        CollisionAvoidance& coll_avoid = gs->collision_avoidance();
        coll_avoid.remove_object(collision_simulation_id());

        gs->for_screens([&]() {
            show_defeat_message(gs->game_chat(), etype());
        });
        if (etype().death_sprite > -1) {
            const int DEATH_SPRITE_TIMEOUT = 1600;
            gs->add_instance<AnimatedInst>(
                    ipos(), etype().death_sprite,
                    DEATH_SPRITE_TIMEOUT, PosF(), PosF(), ItemInst::DEPTH);
        }
    }
    gs->set_level(curr_level);
}

void EnemyInst::copy_to(GameInst *inst) const {
	LANARTS_ASSERT(typeid(*this) == typeid(*inst));
	*(EnemyInst*) inst = *this;
}

std::vector<StatusEffect> EnemyInst::base_status_effects(
        GameState* gs) {
    return etype().effect_modifiers.status_effects;
}
