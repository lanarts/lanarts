Data.equipment_create {
    name = "Leather Armour",
    description = "A durable, flexible armour.",
    type = "armour",

    shop_cost = {15,25},

    spr_item = "leather armour",
    randart_weight = 100,


    stat_bonuses = { 
        spell_cooldown_multiplier = 1.1,
        defence = 1,
    },
}

Data.equipment_create {
    name = "Chainmail",
    description = "A strong armour with limited flexibility.",
    type = "armour",

    shop_cost = {45,60},
    randart_weight = 20,

    spr_item = "spr_armour.chain_mail1",

    stat_bonuses = { 
        defence = 1.5,
        spell_cooldown_multiplier = 1.2
    },
}

Data.equipment_create {
    name = "Crystalline Plate",
    description = "An armour made of heavy but magically unburdensome crystals.",
    type = "armour",

    shop_cost = {295,305},

    spr_item = "spr_armour.crystal_plate2",

    stat_bonuses = { 
        defence = 2
    }
}

Data.equipment_create {
    name = "Platemail",
    description = "A strong, heavy armour.",
    type = "armour",

    shop_cost = {95,105},

    spr_item = "platemail",

    stat_bonuses = { 
        defence = 2,
        spell_cooldown_multiplier = 1.3
    },
}

Data.equipment_create {
     name = "Robe",
     description = "A lightly protective, uninhibited robe.",
     type = "armour",
 
     shop_cost = {10,20},
 
     spr_item = "robe",
 
     stat_bonuses = { 
         willpower = 1,
     },
}

Data.equipment_create {
    name = "Power Robe",
    description = "An enchanted robe of power.",
    type = "armour",

    shop_cost = {75,90},

    spr_item = "enchanted robe",

    stat_bonuses = {
        powerfulness = 1
    }
}

Data.equipment_create {
    name = "Runed Robe",
    description = "A heavily enchanted robe of protection and magical power.",
    type = "armour",

    shop_cost = {200,335},

    spr_item = "runed robe",

    stat_bonuses = {
        defence = 1, 
        willpower = 1
    }
}

Data.equipment_create {
    name = "Robe of Mana",
    description = "A robe of great magical capacity.",
    type = "armour",

    shop_cost = {200,245},

    spr_item = "robe of mana",

    stat_bonuses = {
        mp = 35 
    }
}

Data.equipment_create {
    name = "Robe of Vitality",
    description = "A robe that magically grants prolonged life to the user.",
    type = "armour",

    shop_cost = {150,290},

    spr_item = "spr_armour.robe_of_health",

    stat_bonuses = {
        hp = 35
    }
}

Data.equipment_create {
    name = "Red Dragonplate",
    description = "A great armour crafted from dragon scales. Grants mana on every kill.",
    type = "armour",

    shop_cost = {395,605},

    spr_item = "spr_armour.red_dragon_scale_mail",
    stat_bonuses = {
        hp = 10,
        mp = 10,
        defence = 1,
        willpower = 1,
    },
    effects_granted = {"ArmourManaGainOnKill"}
}
