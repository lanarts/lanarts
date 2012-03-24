/*
 * enemy_data.cpp
 *
 *  Created on: Mar 20, 2012
 *      Author: 100397561
 */

#ifndef ENEMY_DATA_CPP_
#define ENEMY_DATA_CPP_

#include "enemy_data.h"
#include "sprite_data.h"

//Stats constructor:
//	Stats(movespeed, hp, mp, can_melee, can_range, melee_dmg, range_dmg,
//			cooldown, mreach, range, bulletspeed = 7)
//EnemyType constructor:
//   EnemyType(name, rad, xpaward, spriten, stats)
Attack weakmelee(true,  10, 25, 70);
Attack medmelee(true,  15, 25, 70);
Attack strongmelee(true,  25, 25, 70);
Attack strongshortrange_storm(true, 25, 100, 70, SPR_STORMBOLT, 4 );
Attack strongshortrange_fire(true, 25, 100, 70, SPR_FIREBOLT, 4 );

EnemyType game_enemy_data[] = {
		EnemyType("Chicken", 14, 5, SPR_CHICKEN,
				Stats(4, 10,0, weakmelee, Attack())),
		EnemyType("Skeleton", 11, 10, SPR_SKELETON,
				Stats(2, 30,0, medmelee, Attack())),
		EnemyType("Zombie", 14, 15, SPR_ZOMBIE,
				Stats(2, 40,0, medmelee, Attack())),
		EnemyType("Storm Elemental", 14, 20, SPR_STORM_ELE,
				Stats(1, 40,0, Attack(), strongshortrange_storm)),
		EnemyType("Grue", 14, 25, SPR_GRUE,
				Stats(0.5, 80,0, strongmelee, Attack())),
		EnemyType("Super Chicken", 14, 15, SPR_SUPERCHICKEN,
				Stats(4, 30,0, medmelee, Attack())),
		EnemyType("Ciribot", 14, 30, SPR_CIRIBOT,
				Stats(2, 40,0, strongmelee, Attack())),
		EnemyType("Jester", 14, 35, SPR_JESTER,
				Stats(3, 100,0, weakmelee, Attack())),
		EnemyType("Hell Forged", 14, 35, SPR_HELLFORGED,
				Stats(2, 40,0, Attack(), strongshortrange_fire)),
};

size_t game_enemy_n = sizeof(game_enemy_data)/sizeof(EnemyType);



#endif /* ENEMY_DATA_CPP_ */
