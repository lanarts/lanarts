export enemy_init
export enemy_step
export enemy_berserker_init
export enemy_berserker_step

Map = require "core.Map"
ObjectUtils = require "objects.ObjectUtils"
EventLog = require "ui.EventLog"

ally_list = (inst) ->
    if not inst.is_enemy
        return Map.players_list() or {}
    else
        return Map.monsters_list() or {}

enemy_list = (inst) ->
    if inst.is_enemy
        return Map.players_list() or {}
    else
        return Map.monsters_list() or {}

enemy_init = (enemy) -> nil

enemy_step = (enemy) -> nil

enemy_berserker_init = (enemy) ->
    enemy.berserk_time = 500

enemy_berserker_step = (enemy) ->
    enemy.berserk_time = enemy.berserk_time - 1
    if enemy.berserk_time <= 0 and Map.object_visible(enemy)
        enemy\add_effect("Berserk", 300)
        enemy.berserk_time = 500

-- UNDEAD ENEMIES --

Data.enemy_create {
    name: "Crypt Keeper" 
    sprite: "spr_enemies.undead.lich"
    radius: 11
    xpaward: 75
    appear_message: "A frightful Crypt Keeper emerges!"
    defeat_message: "The Crypt Keeper has been destroyed."
    stats: {
        attacks: {
            {weapon: "Basic Melee"}
        }
        hp: 50
        hpregen: 0.03
        movespeed: 2
        -- base stats:
        strength: 15
        defence: 8
        magic: 5
        willpower: 8
    }
    effects_active: {"Pain Aura"}
}

Data.enemy_create {
    name: "Gaseous Ghost" 
    sprite: "spr_enemies.undead.hungry_ghost"
    radius: 11
    xpaward: 75
    appear_message: "A gaseous ghost appears!"
    defeat_message: "The vile ghost is gone."
    stats: {
        attacks: {
            {weapon: "Basic Melee"}
            {projectile: "Mephitize"}
        }
        hp: 50
        hpregen: 0.03
        movespeed: 1
        -- base stats:
        strength: 15
        defence: 5
        magic: 30
        willpower: 8
    }
    effects_active: {"PoisonedWeapon"}
}

Data.enemy_create {
    name: "Mummy" 
    sprite: "spr_enemies.undead.mummy"
    radius: 11
    xpaward: 10
    appear_message: "A summoned mummy appears!"
    defeat_message: "The summoned mummy is put to rest."
    stats: {
        attacks: {
            {weapon: "Basic Melee"}
        }
        hp: 10
        hpregen: 0
        movespeed: 5
        -- base stats:
        strength: 15
        defence: 0
        magic: 15
        willpower: 0
    }
    effects_active: {"PoisonedWeapon"}
}


Data.enemy_create {
    name: "Mummoner" 
    sprite: "spr_enemies.undead.greater_mummy"
    radius: 11
    xpaward: 75
    appear_message: "You find a frightful Mummoner!"
    defeat_message: "The mummoner has been destroyed."
    stats: {
        attacks: {
            {weapon: "Basic Melee"}
        }
        hp: 75
        hpregen: 0.03
        movespeed: 1
        -- base stats:
        strength: 15
        defence: 8
        magic: 10
        willpower: 8
    }
    init_func: () =>
        @n_steps = 0
        @summon_rate = 60
        @summoned = {}
        @n_summons = 0
    step_func: () =>
        @n_summons = 0
        for mon, time in pairs @summoned
            if time > 250
                mon\direct_damage(mon.stats.hp + 1)
            if mon.destroyed
                @summoned[mon] = nil
            else
                @summoned[mon] += 1
                @n_summons += 1
        if Map.object_visible(@) and not (@has_effect "Summoning") and @n_summons < 2
            if #Map.players_list() == 0
                return
            if @n_steps > @summon_rate
                @add_effect("Summoning", 20).monster = "Mummy" 
                @n_steps = 0
            else 
                @n_steps += 1
}

-- SNAKE PIT ENEMIES

Data.enemy_create {
    name: "Black Mamba"
    sprite: "spr_enemies.animals.black_mamba"
    death_sprite: "blood"
    radius: 10
    xpaward: 10
    appear_message: "A poisonous black mamba slithers onto the scene!"
    defeat_message: "The black mamba is dead."
    stats: {
        attacks: {{weapon: "Fast Melee"}}
        hp: 40
        hpregen: 0.04
        movespeed: 3.5
        strength: 15
        defence: 0
        willpower: 5
    }
    effects_active: {"PoisonedWeapon"}
}
 
Data.enemy_create {
    name: "Mouther" 
    sprite: "spr_enemies.animals.tyrant_leech"
    radius: 11
    xpaward: 15
    appear_message: "You hear a mouther start screeching!"
    defeat_message: "The mouther has been shut."
    stats: {
        attacks: {
            {weapon: "Basic Melee"}
            {projectile: "Mephitize"}
        }
        hp: 40
        hpregen: 0.03
        movespeed: 1
        -- base stats:
        strength: 5
        defence: 0
        magic: 2
        willpower: 0
    }
    effects_active: {"PoisonedWeapon"}
}


-- enemy_charge = (caster) ->
--     for target in *enemy_list(caster)
--         if vector_distance({target.x, target.y}, {caster.x, caster.y}) < target.target_radius + caster.target_radius + 30
--             str_diff = math.max(0, caster.stats.strength - target.stats.strength)
--             thrown = target\add_effect("Thrown", 10)
--             thrown.angle = vector_direction({caster.x, caster.y}, {target.x, target.y})
--             if not caster.is_enemy and caster\is_local_player() 
--                 EventLog.add("The " .. target.name .." is thrown back!", {200,200,255})
--             elseif not target.is_enemy and target\is_local_player()
--                 EventLog.add("You are thrown back!", {200,200,255})

Data.effect_create {
    name: "Charging"
    stat_func: (mon, old, new) =>
        new.speed *= 4
        if @n_steps > @n_ramp
            new.melee_cooldown_multiplier *= 0.25
        new.hpregen *= 2
    effected_sprite: "spr_effects.i-loudness"
    effected_colour: {255,0,0}
    fade_out: 15
    init_func: (mon) =>
        @n_steps = 0
        @n_ramp = 10
    step_func: () =>
        if @n_steps % 60 == 0
            play_sound "sound/wavy.ogg"
        @n_steps += 1
    on_melee_func: (mon, defender, damage, attack_stats) =>
        if @n_steps > @n_ramp
            thrown = defender\add_effect("Thrown", 10)
            thrown.angle = vector_direction({mon.x, mon.y}, {defender.x, defender.y})
            if not mon.is_enemy and mon\is_local_player() 
                EventLog.add("The " .. defender.name .." is thrown back!", {200,200,255})
            elseif not defender.is_enemy and defender\is_local_player()
                EventLog.add("You are thrown back!", {200,200,255})
            -- mon\remove_effect("Charging")
}

Data.effect_create {
    name: "Enraging"
    init_func: (mon) =>
        @damage_tracker = 0
        @damage_interval = mon.stats.max_hp / 3
    on_damage_func: (mon, dmg) =>
        @damage_tracker += dmg
        if @damage_tracker > @damage_interval
            EventLog.add((if mon.unique then "" else "The ") .. mon.name .. " gets mad!", {255,255,255})
            mon\add_effect("Charging", 100)
            @damage_tracker = 0
        return dmg
}

Data.enemy_create {
    name: "Elephant"
    sprite: "spr_enemies.animals.elephant"
    death_sprite: "blood"
    radius: 15
    xpaward: 50
    appear_message: "A loud elephant!"
    defeat_message: "The elephant is dead."
    stats: {
        attacks: {{weapon: "Slow Melee"}}
        hp: 70
        hpregen: 0.05
        movespeed: 2
        strength: 15
        defence: 8
        willpower: 8
    }
    effects_active: {"Enraging"}
}
 
Data.enemy_create {
    name: "Red Dragon"
    appear_message: "A frighteningly large red dragon comes into view!"
    defeat_message: "You have slain the red dragon!"
    sprite: "red dragon"
    radius: 27
    xpaward: 150
    unique: true
    stats: {
        attacks: { {weapon: "Basic Melee"}, { projectile: "Large Fire Ball"} }
        hp: 220
        hpregen: 0.1
        movespeed: 2.5
        strength: 25
        magic: 25
        defence: 15
        willpower: 15
    }
    death_func: () =>
        ObjectUtils.spawn_item_near(@, "Red Dragonplate", 1)
}

Data.enemy_create {
    name: "Hell Warrior"
    appear_message: "A Hell Warrior commands you to die!"
    defeat_message: "You have rebuked the Hell Warrior!"
    sprite: "hell warrior"
    radius: 16
    xpaward: 150
    unique: true
    init_func: enemy_berserker_init 
    step_func: enemy_berserker_step
    stats: {
        attacks: {{weapon: "Basic Melee"}}
        hp: 200
        hpregen: 0.125
        movespeed: 5
        strength: 25
        magic: 10
        defence: 12
        willpower: 8
    }
    death_func: () =>
        item = random_choice {"Will Scroll", "Strength Scroll", "Defence Scroll", "Magic Scroll"}
        ObjectUtils.spawn_item_near(@, item, 1)
    init_func: () =>
        @n_steps = 0
        @summon_rate = 60
        @summoned = {}
        @n_summons = 0
    step_func: () =>
        @n_summons = 0
        for mon, time in pairs @summoned
            if time > 600
                mon\direct_damage(mon.stats.hp + 1)
            if mon.destroyed
                @summoned[mon] = nil
            else
                @summoned[mon] += 1
                @n_summons += 1
        if Map.object_visible(@) and not (@has_effect "Summoning") and @n_summons < 5
            if #Map.players_list() == 0
                return
            if @n_steps > @summon_rate
                @add_effect("Summoning", 20).monster = "Imp"
                @n_steps = 0
            else 
                @n_steps += 1
}

Data.enemy_create {
    name: "Gragh"
    appear_message: "Gragh greets you uproariously!"
    defeat_message: "Gragh yells goodbye!"
    sprite: "spr_enemies.bosses.gragh"
    radius: 18
    xpaward: 200
    unique: true
    stats: {
        attacks: {{weapon: "Basic Melee"}}
        hp: 200
        hpregen: 0.25
        movespeed: 3
        strength: 25
        magic: 10
        defence: 8
        willpower: 8
    }
    death_func: () =>
        item = random_choice {"Gragh's Club"}
        ObjectUtils.spawn_item_near(@, item, 1)
    effects_active: {"Enraging"}
}
