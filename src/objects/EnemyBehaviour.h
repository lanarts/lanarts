#ifndef ENEMYBEHAVIOUR_H_
#define ENEMYBEHAVIOUR_H_

#include <vector>

#include "lanarts_defines.h"

#include "objects/GameInst.h"

class SerializeBuffer;

struct EnemyRandomization {
	enum {
		SUCCESSFUL_HIT_MAXTIME = 75, DAMAGE_TAKEN_TIMETHRESHOLD = 200
	};
	EnemyRandomization() :
			successful_hit_timer(0), damage_taken_timer(0), random_walk_timer(
					0), vx(0), vy(0) {

	}
	//XXX: This attempts to randomize enemy movement
	//If they have been hit but have not hit anything in some time
	int successful_hit_timer;
	int damage_taken_timer;
	int random_walk_timer;
	float vx, vy;

	void step() {
		if (successful_hit_timer < SUCCESSFUL_HIT_MAXTIME) {
			successful_hit_timer++;
		}
		if (damage_taken_timer < DAMAGE_TAKEN_TIMETHRESHOLD) {
			damage_taken_timer++;
		}
		if (random_walk_timer > 0) {
			random_walk_timer--;
			if (random_walk_timer == 0) {
				successful_hit_timer = 0;
				damage_taken_timer = 0;
			}
		}
	}
	bool has_random_goal() {
		return random_walk_timer > 0;
	}
	bool should_randomize_movement() {
		return random_walk_timer > 0
				|| (successful_hit_timer >= SUCCESSFUL_HIT_MAXTIME
						&& damage_taken_timer < DAMAGE_TAKEN_TIMETHRESHOLD);
	}
};

struct EnemyBehaviour {
	enum {
		RANDOM_WALK_COOLDOWN = 3000
	};
	enum Action {
		INACTIVE = 0, FOLLOWING_PATH = 1, CHASING_PLAYER = 2
	};

	std::vector<Pos> path;
	Action current_action;

	int current_node;
	int path_cooldown;

	float force_x, force_y;
	int chase_timeout;
	obj_id chasing_actor;

	EnemyRandomization randomization;

	Pos path_start;
	int path_steps;
	bool movement_decided;
	EnemyBehaviour() :
			current_action(INACTIVE), current_node(0), path_cooldown(0), force_x(
					0), force_y(0), chase_timeout(0), chasing_actor(0), path_start(
					0, 0), path_steps(0), movement_decided(false) {
	}
	void step() {
		cooldown_step(chase_timeout);
		randomization.step();
	}

	void damage_was_taken() {
		randomization.damage_taken_timer = 0;
	}

	void serialize(GameState* gs, SerializeBuffer& serializer);
	void deserialize(GameState* gs, SerializeBuffer& serializer);
};

#endif /* ENEMYBEHAVIOUR_H_ */
