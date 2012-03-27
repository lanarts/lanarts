/*
 * load_data.h
 *
 *  Created on: Mar 25, 2012
 *      Author: 100397561
 */

#ifndef LOAD_DATA_H_
#define LOAD_DATA_H_


#include "../../world/GameSettings.h"

void load_tile_data(const char* filename);
void load_sprite_data(const char* filename);

void load_enemy_data(const char* filename);
void load_dungeon_data(const char* filename);
GameSettings load_settings_data(const char* filename);

#endif /* LOAD_DATA_H_ */