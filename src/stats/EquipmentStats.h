/*
 * EquipmentStats.h:
 *  Represents all the possessions and equipped items of a player
 */

#ifndef EQUIPMENTSTATS_H_
#define EQUIPMENTSTATS_H_

#include "../lanarts_defines.h"

#include "Inventory.h"

#include "items/items.h"

class EquipmentStats {
public:
	EquipmentStats() :
			money(0) {
	}
	bool valid_to_use_projectile(const Item& proj);
	bool valid_to_use(const Item& item);
	void equip(const Item& item);
	void deequip_projectiles();
	void deequip_weapon();
	void deequip_armour();
	void deequip(int equipment_type);

	void use_ammo(int amnt = 1);

	bool has_weapon() {
		return weapon.id > NO_ITEM;
	}
	bool has_armour() {
		return armour.id > NO_ITEM;
	}
	bool has_projectile() {
		return projectile.id > NO_ITEM;
	}

	void serialize(SerializeBuffer& serializer);
	void deserialize(SerializeBuffer& serializer);

	Inventory inventory;
	Weapon weapon;
	Projectile projectile;
	Item armour;
	money_t money;
};

#endif /* EQUIPMENTSTATS_H_ */