/*
 * MonsterControllerActions.cpp:
 *  Handles implementation of monster behaviours
 */

#include <algorithm>
#include <cmath>

#include <rvo2/RVO.h>

#include "draw/colour_constants.h"
#include "draw/TileEntry.h"
#include "gamestate/GameState.h"

#include "gamestate/PlayerData.h"

#include "stats/items/ProjectileEntry.h"
#include "stats/items/WeaponEntry.h"
#include "stats/effect_data.h"

#include <lcommon/math_util.h>

#include "objects/PlayerInst.h"

#include "objects/collision_filters.h"
#include "EnemyInst.h"
#include "MonsterController.h"

const int HUGE_DISTANCE = 1000000;
const int OFFSCREEN_CHASE_TIME = 300;

static bool choose_random_direction(GameState* gs, EnemyInst* e, float& vx,
		float& vy) {
	const float deg2rad = 3.14159265f / 180.0f;
	int MAX_ATTEMPTS = 10;
	float movespeed = e->effective_stats().movespeed;
	MTwist& mt = gs->rng();

	for (int attempts = 0; attempts < MAX_ATTEMPTS; attempts++) {
		float direction = mt.rand(360) * deg2rad;
//        vx = cos(direction) * movespeed * (MAX_ATTEMPTS - attempts + 1), vy = sin(direction) *  (MAX_ATTEMPTS - attempts + 1);
        float raw_x = cos(direction), raw_y = sin(direction);
        vx = raw_x * movespeed, vy = raw_y * movespeed;
		int nx = iround(e->x + vx * TILE_SIZE);
		int ny = iround(e->y + vy * TILE_SIZE);
		bool solid = gs->tile_radius_test(nx, ny, TILE_SIZE);
		if (!solid) {
			return true;
		}
	}
	return false;
}

bool has_ranged_attack(EnemyInst* e) {
	CombatStats& stats = e->stats();
	for (int i = 0; i < stats.attacks.size(); i++) {
		if (stats.attacks[i].is_ranged()) {
			return true;
		}
	}
	return false;
}
bool potentially_randomize_movement(GameState* gs, EnemyInst* e) {
	EnemyRandomization& er = e->behaviour().randomization;

    bool should_randomize_movement = er.should_randomize_movement();
    if (e->etype().name == "Giant Bee" || e->etype().name == "Queen Bee" || e->etype().name == "Waxanarian" || e->etype().name == "Adder" || e->etype().name == "Ogre Mage" || e->etype().name == "Black Mamba" || e->etype().name == "Cloud Elemental") { // TODO model with effect, at least
        should_randomize_movement = true; // Always random
    } else if (!has_ranged_attack(e) && gs->get_level()->label() != "Plain Valley") {
		//Only enable this behaviour for ranged enemies for now, unless in overworld
        should_randomize_movement = false;
	} else if (!has_ranged_attack(e) && e->effective_stats().movespeed <= 3) {
        // Want it on fast melee units only
        should_randomize_movement = false;
    } else if (e->effective_stats().movespeed <= 1.5) {
        // Dont want it on really slow units
        should_randomize_movement = false;
    }

	bool randomized = false;
	if (should_randomize_movement) {
		if (er.has_random_goal()) {
			randomized = true;
		}
		if (!randomized && gs->rng().rand(32) == 0
				&& choose_random_direction(gs, e, er.vx, er.vy)) {
			event_log("Enemy id=%d going (x=%.2f, y=%.2f) => (vx=%.2f, vy=%.2f)", std::max(0, e->id), e->x, e->y, er.vx, er.vy);
			er.random_walk_timer = gs->rng().rand(TILE_SIZE, TILE_SIZE * 4) / e->effective_stats().movespeed;
			randomized = true;
		}
		if (randomized) {
			int nx = iround(e->x + er.vx), ny = iround(e->y + er.vy);
			bool solid = gs->tile_radius_test(nx, ny, e->radius);
			if (!solid) {
				e->vx = er.vx, e->vy = er.vy;
			} else {
				er.random_walk_timer = 0;
				er.successful_hit_timer = 0;
				er.damage_taken_timer = 0;
				randomized = false;
			}
		}
	}
	return randomized;
}

static bool same_target_and_moved_colfilter(GameInst* self, GameInst* other) {
	EnemyInst* e1 = (EnemyInst*)self;
	EnemyInst* e2 = dynamic_cast<EnemyInst*>(other);
	if (!e2) {
		return false;
	}
	if (e1->behaviour().chasing_actor != e2->behaviour().chasing_actor) {
		return false;
	}
	return (e2->behaviour().movement_decided);
}

const int TOO_LARGE_RANGE = 99999;
static bool attack_ai_choice(GameState* gs, CombatGameInst* inst,
		CombatGameInst* target, AttackStats& attack) {
	CombatStats& stats = inst->stats();
        EffectiveStats& estats = inst->effective_stats();
	std::vector<AttackStats>& attacks = stats.attacks;

	int attack_id = -1;
	int smallest_range = TOO_LARGE_RANGE;
	float dist = distance_between(Pos(inst->x, inst->y),
								  Pos(target->x, target->y));
	int radii = inst->target_radius + target->target_radius;

	for (int i = 0; i < attacks.size(); i++) {
		WeaponEntry& wentry = attacks[i].weapon.weapon_entry();
		int range = wentry.range();
		if (!attacks[i].projectile.empty()) {
			ProjectileEntry& pentry = attacks[i].projectile_entry();
			range = std::max(range, pentry.range());
			if (!estats.allowed_actions.can_use_spells) {
				continue;
			}
		} else if (!estats.allowed_actions.can_use_weapons) {
			continue;
		}
		event_log("attack_ai_choice radii=%d range=%d smallrange=%d ", radii, range, smallest_range);
		if (radii + range >= dist && range < smallest_range) {
			attack_id = i;
			smallest_range = range;
		}
	}

	if (attack_id > -1) {
		attack = attacks[attack_id];
		return true;
	}
	return false;
}
bool go_towards_if_free_in_direction(GameState* gs, CombatGameInst* inst,
		float vx, float vy) {
	float tx = vx, ty = vy;
	normalize(tx, ty, TILE_SIZE);
	normalize(vx, vy, inst->effective_stats().movespeed);
	float nx = inst->x + tx, ny = inst->y + ty;
	if (!gs->solid_test(inst, round(nx), round(ny), inst->radius)) {
		inst->vx = vx, inst->vy = vy;
		return true;
	}
	return false;
}

void MonsterController::set_monster_headings(GameState* gs,
		std::vector<EnemyOfInterest>& eois) {
	perf_timer_begin(FUNCNAME);

	std::sort(eois.begin(), eois.end());
	for (int i = 0; i < eois.size(); i++) {
		eois[i].e->behaviour().movement_decided = false;
	}
	for (int i = 0; i < eois.size(); i++) {
		EnemyInst* e = eois[i].e;
		float movespeed = e->effective_stats().movespeed;
		int actor_id = eois[i].actor_id;
        event_log("Enemy id=%d has move speed %f chasing actor id=%d", std::max(0, e->id), movespeed, actor_id);
		CombatGameInst* p = gs->get_instance<CombatGameInst>(actor_id);
        if (p == NULL) {
            continue;
        }
		EnemyBehaviour& eb = e->behaviour();

		eb.current_action = EnemyBehaviour::CHASING_PLAYER;
		eb.path.clear();
		if (is_visible(gs, e, p)) {
			eb.chase_timeout = OFFSCREEN_CHASE_TIME;
			eb.chasing_actor = p->id;
		}

		if (!potentially_randomize_movement(gs, e)) {
            if (e->has_paths_data()) {
                // TODO break out allied monster code into its own thing:
                e->vx = 0, e->vy = 0;
                if (e->has_paths_data()) {
                    Pos p = e->direction_towards_enemy(gs);
                    e->vx = p.x, e->vy = p.y;
                    float speed = e->effective_stats().movespeed;
                    normalize(e->vx, e->vy, speed);
                    if (e->vx == 0 && e->vy == 0) {
                        GameInst* inst = get_nearest_ally(gs, e);
                        if (inst) {
                            if (distance_between(e->ipos(), inst->ipos()) > TILE_SIZE) {
                                Pos p = e->direction_towards_ally_player(gs);
                                e->vx = p.x, e->vy = p.y;
                                float speed = e->effective_stats().movespeed;
                                normalize(e->vx, e->vy, speed);
                            }
                        }
                    }
                }
                event_log("set_monster_headings id=%d randomizing movement, vx=%d vy=%d", std::max(0, e->id), e->vx, e->vy);
            } else {
                PosF heading = get_direction_towards(gs, e, p, movespeed);
                e->vx = heading.x;
                e->vy = heading.y;
            }
		}

		// Compare position to enemy of interest:
		float pdist = distance_between(Pos(e->x, e->y), Pos(p->x, p->y));

		AttackStats attack;
		bool viable_attack = attack_ai_choice(gs, e, p, attack);
		WeaponEntry& wentry = attack.weapon_entry();
		bool hasproj = attack.projectile.id != NO_ITEM;

		// Part of: Implement a bunch of new status effects.
        bool has_fear = e->effects.has("Fear");
		if (pdist < e->target_radius + p->target_radius && !has_fear) {
			e->vx = 0, e->vy = 0;
		}

		if (viable_attack) {
			int mindist = wentry.range() + p->target_radius + e->target_radius
					- TILE_SIZE / 8;
			if (hasproj) {
				mindist = attack.projectile_entry().range();
			}
			if (has_fear) {
			    // Don't stop moving near if have fear.
			} else if (!attack.is_ranged() && attack.weapon_entry().range() < 100) {
			    // TODO clean up mess of 100 range clause
				e->vx = 0, e->vy = 0;
			} else {
				int close = 40;
                bool stop_once_in_range = e->effects.has("StopOnceInRange");
				if (stop_once_in_range) {
					close = 150;
				}
				if (pdist < std::min(mindist, close)) {
					e->vx = 0, e->vy = 0;
				}
			}

			if (!wentry.attack.alt_action.isnil()) {
				lcall(wentry.attack.alt_action, e, p->pos(), p);
			} else {
				e->attack(gs, p, attack);
			}

		}
		if (gs->tile_radius_test(e->x, e->y, TILE_SIZE / 2 + 4)) {
			if (gs->object_radius_test(e, NULL, 0,
					same_target_and_moved_colfilter, e->x, e->y,
					e->target_radius - e->effective_stats().movespeed - 2)) {
				float dx = p->x - e->x, dy = p->y - e->y;
				if (!go_towards_if_free_in_direction(gs, e, -dy, dx)
						&& !go_towards_if_free_in_direction(gs, e, dy, -dx)) {
					e->vx = 0, e->vy = 0;
				}
			}
		}

		eb.movement_decided = true;
		e->vx = round(e->vx * 4096.0f) / 4096.0f;
		e->vy = round(e->vy * 4096.0f) / 4096.0f;
                event_log("set_monster_headings id=%d vx=%f vy=%f", std::max(0, e->id), e->vx, e->vy);
	}

	perf_timer_end(FUNCNAME);
}

//returns true if will be exactly on target
static bool move_towards(GameState* gs, EnemyInst* e, const Pos& p) {
	EnemyBehaviour& eb = e->behaviour();

	float speed = e->effective_stats().movespeed;
	float dx = p.x - e->x, dy = p.y - e->y;
	float mag = distance_between(p, e->ipos());

	if (mag <= speed / 2) {
		e->vx = dx;
		e->vy = dy;
		return true;
	}

	eb.path_steps++;
	e->vx = dx / mag * speed;
	e->vy = dy / mag * speed;
    if (gs->tile_radius_test(iround(e->x + e->vx), iround(e->y + e->vy), TILE_SIZE)) {
        return true;
    }
	// Ensure floating point differences do not occur
	e->vx = round(e->vx * 4096.0f) / 4096.0f;
	e->vy = round(e->vy * 4096.0f) / 4096.0f;

	return false;
}

static Pos monster_wander_position(GameState* gs, EnemyInst* e);

void MonsterController::monster_follow_path(GameState* gs, EnemyInst* e) {
	EnemyBehaviour& eb = e->behaviour();
    EnemyRandomization& er = eb.randomization;
    if (er.random_walk_timer > 0) {
//        int nx = iround(e->x + er.vx), ny = iround(e->y + er.vy);
//            e->vx = er.vx, e->vy = er.vy;
        e->use_move(gs, {er.vx, er.vy}, false);
        er.random_walk_timer -= 1;
        return;

//        bool solid = gs->tile_radius_test(nx, ny, TILE_SIZE);
//        if (!solid) {
//            return;
//        } else {
//            er.random_walk_timer = 0;
//            er.successful_hit_timer = 0;
//            er.damage_taken_timer = 0;
//        }

    }

    Pos xy = monster_wander_position(gs, e);
	float movespeed = e->effective_stats().movespeed;
    if (xy != Pos()) {
        PosF dxy = xy.scaled(TILE_SIZE) + Pos(TILE_SIZE/2, TILE_SIZE/2) - e->pos();
        float mag = sqrtf(dxy.x*dxy.x + dxy.y*dxy.y);
        er.vx = dxy.x * movespeed / std::max(mag, 1.0f);
        er.vy = dxy.y * movespeed / std::max(mag, 1.0f);
        e->use_move(gs, {er.vx, er.vy}, false);
        er.random_walk_timer = 600;
    }
//    monster_wander_position
//    if (choose_random_direction(gs, e, er.vx, er.vy)) {
//        e->use_move(gs, {er.vx, er.vy}, false);
//        er.random_walk_timer = 600;
//        return;
//    }
//    eb.path.clear();
//    eb.current_action = EnemyBehaviour::INACTIVE;
//	MTwist& mt = gs->rng();
//	EnemyBehaviour& eb = e->behaviour();
//
//	const int PATH_CHECK_INTERVAL = 600; //~10seconds
//	float path_progress_threshold = movespeed / 50.0f;
//	float progress = distance_between(eb.path_start, e->ipos());
//
//	if (eb.path_steps > PATH_CHECK_INTERVAL * 5
//			&& progress / eb.path_steps < path_progress_threshold) {
//		eb.path.clear();
//        eb.current_action = EnemyBehaviour::INACTIVE;
//		return;
//	}
//	// Walk randomization
//	EnemyRandomization& er = e->behaviour().randomization;
//	if (er.random_walk_timer > 0) {
//		int nx = iround(e->x + er.vx), ny = iround(e->y + er.vy);
//
//		bool solid = gs->tile_radius_test(nx, ny, TILE_SIZE);
//		if (!solid) {
//			e->vx = er.vx, e->vy = er.vy;
//			return;
//		} else {
//			er.random_walk_timer = 0;
//			er.successful_hit_timer = 0;
//			er.damage_taken_timer = 0;
//		}
//
//	}
//	if (mt.rand(12000) == 0) {
//        if (choose_random_direction(gs, e, er.vx, er.vy)) {
//			e->vx = er.vx, e->vy = er.vy;
//			er.random_walk_timer = 60;
//            return;
//        }
//    }
//
//	if (eb.current_node < eb.path.size()) {
//		if (move_towards(gs, e, eb.path[eb.current_node])) {
//            eb.current_node++;
//        }
//	} else {
//        eb.path.clear();
//        eb.current_action = EnemyBehaviour::INACTIVE;
//	}
}

static Pos monster_wander_position(GameState* gs, EnemyInst* e) {
	GameTiles& tile = gs->tiles();
	MTwist& mt = gs->rng();
	auto& map = e->get_map(gs)->attraction_map();
	Pos exy = e->ipos().divided(TILE_SIZE);
	Pos target;
	// Complex stairs-avoidance logic follows
	int n_considered = 0;
	float score_to_beat = -10000;
	for (int i = 0; i < 20; i++) {
		Pos candidate = {
			squish(exy.x + mt.rand(-2, 3), 0, tile.tile_width() - 1),
			squish(exy.y + mt.rand(-2, 3), 0, tile.tile_height() - 1)
		};
		if (tile.is_solid(candidate)) {
			continue;
		}
		// Favour big distances
		float dist = distance_between(candidate, exy);
		float score = map.attraction_map[candidate] + dist;
		if (score >= score_to_beat) {
			n_considered++;
			target = candidate;
		}
		if (n_considered >= 6) {
			break;
		}
	}
	if (n_considered == 0) {
		return Pos();
	}
    map.attraction_map[target] -= 1;
	return target;
}

void MonsterController::monster_wandering(GameState* gs, EnemyInst* e) {
	MTwist& mt = gs->rng();
	EnemyBehaviour& eb = e->behaviour();
	e->vx = 0, e->vy = 0;

        // Part of: Implement status effects.
    bool forced_wander = e->effects.has("Dazed");

	if (!forced_wander && !monsters_wandering_flag) {
		return;
	}
	PERF_TIMER();
//	bool is_fullpath = false;
	if (eb.path_cooldown > 0) {
		eb.path_cooldown--;
//		is_fullpath = false;
	}
    Pos exy = e->ipos().divided(TILE_SIZE);

	if (gs->rng().rand(32) == 0) {
        // Avoid pathological cases of mass path finding on one step
		return;
	}

	auto try_wander = [&](Pos target, bool full_path) {
        BBox region = AStarPath::surrounding_region(exy, target, Size(6, 6));
        if (full_path) {
            region = BBox({0,0}, gs->get_level()->size());
        }
        eb.path = astarcontext.calculate_AStar_path(gs, exy, target, region);
        if (eb.path.size() <= 1) {
            return false;
        }
        eb.current_node = 0;
//	if (is_fullpath) {
        eb.path_cooldown = EnemyBehaviour::RANDOM_WALK_COOLDOWN/2 + mt.rand(EnemyBehaviour::RANDOM_WALK_COOLDOWN);
//	}
        eb.current_action = EnemyBehaviour::FOLLOWING_PATH;

        eb.path_steps = 0;
        eb.path_start = exy;

        event_log("Path for instance id: %d, (%d path steps), x: %f y: %f target_radius: %f depth %d",
                  std::max(0, e->id), (int)eb.path.size(), e->x, e->y, e->target_radius, e->depth);
        return true;
	};
	bool wandered = false;
    if (!e->lua_variables.empty()) {
        Pos target = lcall_def(Pos(), e->lua_variables["monster_wander_position"], e);
        if (target != Pos()) {
            wandered |= try_wander(target, true);
        }
    }
    if (!wandered) {
        Pos target = monster_wander_position(gs, e);
        if (target != Pos()) {
            wandered |= try_wander(target, false);
        }
    }
}
