/*
 * itemgen.cpp
 *
 *  Created on: Mar 19, 2012
 *      Author: 100397561
 */

#include "itemgen.h"
#include "../world/GameState.h"
#include "../world/objects/ItemInst.h"
#include "../data/item_data.h"


void generate_items(const ItemGenSettings& is, MTwist& mt, GeneratedLevel& level, GameState* gs){
	GameTiles& tiles = gs->tile_grid();
	int start_x = (tiles.tile_width()-level.width())/2;
	int start_y = (tiles.tile_height()-level.height())/2;

	int amount = mt.rand(is.min_items, is.max_items+1);

	//generate gold
	for (int i = 0; i < amount; i++) {
		Pos ipos = generate_location(mt, level);
		int ix = (ipos.x+start_x) * 32 + 16;
		int iy = (ipos.y+start_y) * 32 + 16;
		gs->add_instance(new ItemInst(ITEM_GOLD, ix,iy));
		level.at(ipos).has_instance = true;
	}
	
	amount = mt.rand(3, 5);
	//generate potions
	for(int i = 0; i < amount; i++){
		Pos ipos = generate_location(mt, level);
		int ix = (ipos.x+start_x) * 32 + 16;
		int iy = (ipos.y+start_y) * 32 + 16;
		gs->add_instance(new ItemInst(ITEM_POTION, ix,iy));
		level.at(ipos).has_instance = true;
	}
}
