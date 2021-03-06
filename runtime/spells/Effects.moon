EventLog = require "ui.EventLog"
GameObject = require "core.GameObject"
GameState = require "core.GameState"
Map = require "core.Map"
World = require "core.World"
Bresenham = require "core.Bresenham"
Display = require "core.Display"
SpellObjects = require "objects.SpellObjects"
DataW = require "DataWrapped"
TypeEffectUtils = require "spells.TypeEffectUtils"
BonusesUtils = require "items.BonusesUtils"

DataW.effect_create {
    name: "Poison"
    effected_sprite: "spr_effects.poison"
    poison_rate: 35
    effected_colour: {100, 255, 100}
    fade_out: 4
    can_use_rest: false
    stat_func: (obj, old, new) =>
        new.defence = math.max(0, new.defence - 5)
        new.willpower = math.max(0, new.willpower - 3)
        if not obj.is_enemy
            new.speed /= 2
    apply_func: (obj, args) =>
        @steps = 0
        @attacker = assert args.attacker
        @damage, @power = assert(args.damage), assert(args.power)
        @magic_percentage = args.magic_percentage or 0.0
        @poison_rate = assert args.poison_rate
    step_func: (obj) =>
        if @steps < @poison_rate
            @steps += 1
        else
            @steps = 0
            obj\damage(@damage / 2, @power, @magic_percentage, @attacker, (TypeEffectUtils.get_resistance obj, "Green"))
}

DataW.effect_create {
    name: "Berserk"
    exhausted_duration: 300
    can_use_rest: false
    can_use_spells: true
    can_use_stairs: true
    fade_out: 5
    effected_colour: {255,160,160}
    effected_sprite: "berserk_effected"
    on_melee_func: (attacker, defender, damage) =>
        -- Give some extra time for simply hitting enemies in melee
        -- Favours high melee speed??
        if attacker.stats.level >= 2
            EventLog.add("Your rage grows from combat...", {200,200,255})
            @time_left = math.min(@max_time * 1.5, @time_left + 5)
        
        if chance(.1)
            defender\add_effect("Stun", 20 + attacker.stats.level)
        return damage
    stat_func: (obj, old, new) =>
        -- 2 Power points during berserk for each levelup
        new.powerfulness += obj.stats.level * 2
        -- Greater defences during berserk
        -- 3 defence points during berserk for each levelup
        new.defence = math.max(0, new.defence + obj.stats.level * 3)
        new.willpower = math.max(0, new.willpower + obj.stats.level * 3)
        new.melee_cooldown_multiplier /= 1.6
        hp_regen_bonus = (10 + 4 * obj.stats.level) / 60 -- 10 + 4 * levelup per second
        new.hpregen += hp_regen_bonus
        if obj\has_effect("AmuletBerserker")
            new.hpregen += hp_regen_bonus
        if new.speed < 6
            new.speed = math.max(new.speed * 1.25, 6)
        obj\reset_rest_cooldown()
    apply_func: (obj, time_left) =>
        @kill_tracker = obj.kills
        @max_time = math.max(@max_time or 0, time_left)
        @extensions = 0
    remove_func: (obj) =>
        obj\add_effect("Exhausted", @exhausted_duration)
        for _ in screens()
            if obj\is_local_player()
                play_sound "sound/exhausted.ogg"
                EventLog.add("You are now exhausted...", {255,200,200})
    step_func: (obj) =>
        if @extensions >= 10
            for _ in screens()
                if obj\is_local_player()
                    EventLog.add("Your rage is tickled... but cannot grow further.", {200,200,255})
            return
        diff = math.max(obj.kills - @kill_tracker, 0)
        for i=1,diff
            @time_left = math.min(@max_time * 1.5, @time_left + 45)
            for _ in screens()
                if obj\is_local_player()
                    EventLog.add("Your rage grows from killing...", {200,200,255})
                    play_sound "sound/swish-11.wav"
                if settings.verbose_output
                    EventLog.add("Killed Enemy, berserk time_left = " .. @time_left)
            if obj\has_effect("AmuletBerserker") and chance(0.05) then
                play_sound "sound/summon.ogg"
                eff = obj\add_effect("Summoning", 20)
                eff.monster = "Centaur Hunter"
                eff.duration = 5
            --stats = obj\effective_stats()
            -- TODO moving towards HP regen++ instead of hp heal
            --obj\heal_hp(10 + stats.defence * 2)
            @extensions += 1
        @kill_tracker = obj.kills
}

DataW.effect_create {
    name: "Exhausted"
    can_use_rest: false
    effected_colour: {120,120,120}
    effected_sprite: "exhausted_effected"
    stat_func: (obj, old, new) =>
        new.speed = new.speed / 2
        new.defence = math.max(0, new.defence - 3)
        new.willpower = math.max(0, new.willpower - 3)
        new.cooldown_mult = new.cooldown_mult * 1.25
        obj\reset_rest_cooldown()
    remove_func: (obj) =>
        for _ in screens()
            if obj\is_local_player()
                EventLog.add("You are no longer exhausted.", {200,200,255})
}

-- May 16 
-- DataW.effect_create {
--     name: "Expedited"
--     stat_func: () =>
--         if new.speed < 6
--             new.speed = math.max(new.speed * 1.5, 6)
--         new.range_cooldown_multiplier /= 1.5
--     can_use_rest: false
--     finish_func: (obj) =>
--         for _ in screens()
--             if obj\is_local_player()
--                 EventLog.add("You are no longer expedited.", {255,200,200})
--     effected_colour: {220,220,255}
--     effected_sprite: "haste effected"
-- }

DataW.effect_create {
    name: "Charge"
    can_use_spells: false
    stat_func: (obj, old, new) =>
        new.defence += 5
        new.powerfulness += 5
        new.speed = 0
    remove_func: (obj) =>
        @callback(obj)
    effected_colour: {255,0,0}
    effected_sprite: "charge_effected"
    fade_out: 0
}

DataW.effect_create {
    name: "Haste"
    effected_colour: {160,160,255,240}
    stat_func: (obj, old, new) =>
        new.speed += 2
        new.powerfulness += 2
        new.defence += 4
        new.willpower += 4
        new.cooldown_mult /= 1.25
}

DataW.effect_create {
    name: "Stun"
    effected_sprite: "spr_effects.pain_mirror"
    effected_colour: {200,200,200}
    duration: 15
    stat_func: (obj, old, new) =>
        new.speed = 0
        obj.stats.attack_cooldown = 2
    fade_out: 5
}

DataW.effect_create {
    name: "Thrown"
    effected_sprite: "spr_effects.pain_mirror"
    effected_colour: {200,200,200}
    duration: 15
    apply_func: (obj) =>
        @steps = 0
    step_func: (obj) =>
        increment = @duration - @steps
        if @steps <= @duration
            x = math.floor(obj.x + math.cos(@angle) * increment )
            y = math.floor(obj.y + math.sin(@angle) * increment )
            if not Map.object_solid_check(obj, {x,y})
                obj.x, obj.y = x, y
        @steps += 1
    stat_func: (obj, old, new) =>
        new.speed = 0
        obj.stats.attack_cooldown = 2
    fade_out: 5
}

--# TODO: Asleep enemies dont attack or move until woke
--#    name: "Asleep"
--#    effected_sprite: spr_effects.sleeping
--#    fade_out: 5

DataW.effect_create {
    name: "Pained"
    effected_sprite: "spr_spells.pain"
    effected_colour: {100,100,100}
    fade_out: 15
}

DataW.effect_create {
    name: "Sapped"
    effected_sprite: "spr_effects.manatake"
    fade_out: 15
}

DataW.effect_create {
    name: "Dazed"
    effected_sprite: "spr_effects.may_stab_brand"
    fade_out: 5
    -- Effect in CPP
}


DataW.effect_create {
    name: "AlwaysAttack"
    -- Effect in CPP
}

DataW.effect_create {
    name: "Slowed"
    effected_sprite: "spr_effects.slowed"
    fade_out: 5
    stat_func: (obj, old, new) =>
        new.melee_cooldown_multiplier *= 2
        new.spell_cooldown_multiplier *= 2
        new.speed /= 2
}

DataW.effect_create {
    name: "Regeneration"
    effected_sprite: "spr_amulets.i-regeneration"
    effected_colour: {200,255,200} --RGB
    -- Rest of effect in CPP code
    stat_func: (obj, old, new) =>
        if obj\has_effect("AmuletGreatPain")
            new.hpregen *= 30
        else
            new.hpregen *= 15
        if obj.stats.hp == new.max_hp
            obj\remove_effect("Regeneration")
    fade_out: 55
}

DataW.effect_create {
    name: "Fear"
    effected_sprite: "spr_effects.fleeing"
    effected_colour: {200,255,200}
    -- Rest of effect in CPP code
    stat_func: (obj, old, new) =>
        new.speed *= 1.25
    fade_out: 55
}

--DataW.weapon_poison = tosprite("spr_effects.weapon_poison")
--DataW.weapon_confusion = tosprite("spr_effects.weapon_confusion")
--DataW.weapon_vampirism = tosprite("spr_effects.weapon_vampirism")

M = nilprotect {}
M._poison = tosprite("spr_weapons.i-venom")

M._fire_resist = tosprite("spr_effects.i-r-fire-big")
M._ice_resist = tosprite("spr_effects.i-r-cold-big")
M._generic_resist = tosprite("spr_effects.i-protection-big")
M._poison_resist = tosprite("spr_effects.i-r-poison-big")
M._black_resist = tosprite("spr_effects.black-resist")
M._storm_resist = tosprite("spr_effects.shock-resist")

M._fire_power = tosprite("spr_effects.i-fire-big")
M._ice_power = tosprite("spr_effects.i-cold-big")
M._black_power = tosprite("spr_effects.black-power")
M._storm_power = tosprite("spr_effects.shock")
M._generic_power = tosprite("spr_effects.sword")
M._poison_power = tosprite("spr_effects.i-poison-big")


M._confusion = tosprite("spr_weapons.i-confusion")
M._vampirism = tosprite("spr_weapons.i-vampirism")
M._fleeing = tosprite("spr_effects.fleeing")
M._fear = tosprite("spr_effects.i-life-protection")

draw_console_effect = (sprite, text, xy, color = COL_PALE_YELLOW) ->
    sprite\draw {
        origin: Display.LEFT_CENTER
    }, {xy[1], xy[2] + 4}
    font_cached_load(settings.font, 10)\draw {
        :color
        origin: Display.LEFT_CENTER
    }, {xy[1] + Map.TILE_SIZE + 4, xy[2]}, text

draw_weapon_console_effect = (player, sprite, text, xy, color = COL_PALE_YELLOW) ->
    player.weapon_sprite\draw {
        origin: Display.LEFT_CENTER
    }, {xy[1], xy[2] + 4}
    draw_console_effect(sprite, text, xy, color)

DataW.effect_create {
    name: "FearWeapon"
    console_draw_func: (player, get_next) =>
        draw_weapon_console_effect(player, M._fear, "+10% chance fear", get_next())
    on_melee_func: (attacker, defender, damage) =>
        if defender\has_effect("Fear")
            return damage
        if chance(.1)
            eff = defender\add_effect("Fear", 150)
        return damage
}

DataW.effect_create {
    name: "ConfusingWeapon"
    console_draw_func: (player, get_next) =>
        draw_weapon_console_effect(player, M._confusion, "+10% chance daze", get_next())
    on_melee_func: (attacker, defender, damage) =>
        if defender\has_effect("Dazed")
            return damage
        if chance(.1 * @n_derived)
            eff = defender\add_effect("Dazed", 70)
        return damage
}


DataW.effect_create {
    name: "StopOnceInRange"
    -- For centaurs predominantly, coded in C++
}

DataW.effect_create {
    name: "Encumbered"
    stat_func: (obj, old, new) =>
        new.speed -= 1
}

DataW.effect_create {
    name: "Baleful Regeneration",
    effected_sprite: "spr_amulets.i-regeneration"
    effected_colour: {200,255,200}
    step_func: (caster) =>
        {:hp, :max_hp, :hpregen} = caster\effective_stats()
        if hp >= max_hp
            caster\remove_effect("Baleful Regeneration")
        else
            regen_rate = hpregen * (if caster\has_effect("AmuletGreatPain") then 29 else 14)
            caster\heal_hp(regen_rate)
}

for {resist_sprite, power_sprite, type, color} in *{
    {M._poison_resist, M._poison_power, "Green", {226, 66, 244}}
    {M._fire_resist, M._fire_power, "Red", {244, 72, 66}}
    {M._ice_resist, M._ice_power, "Blue", COL_WHITE}
    {M._storm_resist, M._storm_power, "White", COL_BLUE}
    {M._black_resist, M._black_power, "Black", COL_BLUE}
    -- Swords
    {M._generic_resist, M._generic_power, "Slashing", {66, 222, 188}}
    {M._generic_resist, M._generic_power, "Bludgeon", {66, 244, 194}}
    --
    {M._generic_resist, M._generic_power, "Piercing", {66, 244, 214}}
} do
    color = COL_WHITE -- OVERRIDE HACK FOR NOW
    -- Simply a stat variable:
    DataW.additive_effect_create {
        name: "#{type}Resist"
        key: "resist" -- Additive effect, accessed with @value().
        console_draw_func: (player, get_next) =>
            text = "#{type} Resist"
            res = if @resist < 0 then @resist else "+"..@resist
            draw_console_effect(resist_sprite, "#{res} #{text}", get_next(), if @resist >= 1 then color else COL_PALE_RED)
    }
    -- Adds power specifically to attacks of this type
    -- Subform of
    -- TODO separate sprite
    DataW.additive_effect_create {
        name: "#{type}Power"
        key: "power" -- Additive effect, accessed with @value().
        console_draw_func: (player, get_next) =>
            text = "#{type} Power"
            res = if @power < 0 then @power else "+"..@power
            draw_console_effect(power_sprite, "#{res} #{text}", get_next(), if @power >= 0 then color else COL_PALE_RED)
    }
DataW.additive_effect_create {
    name: "PoisonedWeapon"
    key: "poison_percentage" -- Additive effect, accessed with @value().
    console_draw_func: (player, get_next) =>
        draw_weapon_console_effect(player, M._poison, "+#{math.floor(@poison_percentage * 100)}% chance poison", get_next())
    on_melee_func: (attacker, defender, damage) =>
        if defender\has_effect("Poison")
            return damage
        power = TypeEffectUtils.get_power(attacker, "Green")
        resist = TypeEffectUtils.get_resistance(defender, "Green")
        poison_chance = @value()
        if chance(poison_chance * resist)
            eff = defender\add_effect "Poison", {
                time_left: 100
                poison_rate: 25
                :attacker
                damage:  damage * (1 + power/5)
                power: attacker\effective_stats().powerfulness + power
                type_resistance: resist
                magic_percentage: 1.0
            }
        return damage
}

DataW.additive_effect_create {
    name: "Spiky"
    key: "recoil_percentage" -- Additive effect, accessed with @value().
    console_draw_func: (player, get_next) =>
        draw_console_effect(tosprite("spr_spells.spectral_weapon"), "+#{math.floor(@recoil_percentage * 100)}% melee recoil damage", get_next())
    on_receive_melee_func: (attacker, defender, damage) =>
        percentage_recoil = @value()
        attacker\direct_damage(damage * percentage_recoil, defender)
        for _ in screens()
            if defender.is_local_player and defender\is_local_player()
                EventLog.add("You strike back with spikes!", COL_PALE_BLUE)
}

-- These all are implemented by checks in the code:
DataW.effect_create {
    name: "AmuletBerserker"
}

DataW.effect_create {
    name: "AmuletGreatPain"
}

DataW.effect_create {
    name: "AmuletGreaterFire"
}

DataW.effect_create {
    name: "DiesOutsideOfSummonerRange"
    init_func: (summon) =>
        @summoner = false -- TODO think about allowing initial state to be passed to effects!
        -- TODO but OTOH I like the current system.
    step_func: (summon) =>
        assert @summoner, "No summoner set on DiesOutsideOfSummonerRange!"
        if not Map.object_visible summon, summon.xy, @summoner
            summon\direct_damage(10)
}

DataW.effect_create {
    -- Dies at the end of this effect, or on the first attack.
    name: "DiesOnEndOrFirstAttack"
    init_func: () =>
        @n_animations = rand_range(0, 16)
    die: (obj) =>
        BonusesUtils.create_animation @, obj, "spr_bonuses.ghost", 0.5
        obj\die()
    remove_func: (obj) =>
        @die obj
    on_melee_func: (obj, defender, damage) =>
        @die obj
        return damage
    on_projectile_func: (obj, defender, xy) =>
        @die obj
}


DataW.effect_create {
    name: "Summoner"
    init_func: (caster) =>
        @n_summons = 0
    step_func: (caster) =>
        caster.summoned or= OrderedDict()
        @n_summons = 0
        for mon, time in strictpairs caster.summoned
            if not mon.destroyed
                event_log("Desummoning monster (%s, id=%d) at (%.2f, %.2f)", mon.name, math.max(0, mon.id), mon.x, mon.y)
                time_out = (if mon.name == "Spectral Beast" then (time > 600) else (time > 1200)) -- All others are permanents
                diff_floor = (caster.map ~= mon.map)
                if time_out or diff_floor
                    mon\direct_damage(mon.stats.hp + 1)
            if mon.destroyed
                caster.summoned[mon] = nil
            else
                caster.summoned[mon] += 1
                @n_summons += 1
}

-- TODO flip effect of fortification -- TODO whut?
DataW.effect_create {
    name: "Fortification"
    console_draw_func: (player, get_next) =>
        draw_console_effect(tosprite("spr_spells.statue_form"), "Gain defence when hit", get_next())
    init_func: (caster) =>
        @active_bonuses = OrderedDict()
        @duration = 320
    stat_func: (obj, old, new) =>
        for k, v in strictpairs @active_bonuses -- Abuse that stat_func is called every frame
            if v == 0
                for _ in screens()
                    EventLog.add("Your defence falls back down...", COL_PALE_BLUE)
                @active_bonuses[k] = nil
            else
                new.defence += 2
                @active_bonuses[k] -= 1
    on_receive_damage_func: (attacker, defender, damage) =>
        if not @active_bonuses[attacker.id]
            for _ in screens()
                EventLog.add("Your defence rises due to getting hit!", COL_PALE_BLUE)
            @active_bonuses[attacker.id] = @duration
        elseif @active_bonuses[attacker.id] < @duration
            @active_bonuses[attacker.id] = @duration
        return damage

}

DataW.effect_create {
    name: "Ludaze"
    effected_sprite: "spr_spells.ludaze"
    can_use_rest: false
    effected_colour: {200, 200, 255, 200}
    fade_out: 10
}

DataW.effect_create {
    name: "Ice Form"
    effected_sprite: "spr_spells.iceform"
    can_use_rest: false
    can_use_spells: false
    can_use_stairs: true -- Important!!
    can_use_weapons: false
    effected_colour: {200, 200, 255}
    stat_func: (effect, obj, old, new) ->
        new.resistance += 100
        new.magic_resistance += 100
        new.speed = math.min(new.speed, 0.5)
    fade_out: 55
}

-------  START EQUIPMENT SLOT STACKED EFFECTS --------
-- Hack because effects are not currently easily stacked otherwise:
for equip_slot in *{"", "Armour", "Amulet", "Ring", "Belt", "Weapon", "Legwear"}
    DataW.effect_create {
        name: "#{equip_slot}HealthGainOnKill"
        category: "EquipEffect"
        init_func: (caster) =>
            @kill_tracker = caster.kills
        step_func: (caster) =>
            while caster.kills > @kill_tracker
                for _ in screens()
                    if caster\is_local_player()
                        EventLog.add("You regain health for killing!", COL_PALE_BLUE)
                caster\heal_hp(5 + caster.stats.level)
                play_sound "sound/swish-10.wav"
                BonusesUtils.create_animation {n_animations: 0}, caster, "spr_effects.health", 0.5
                @kill_tracker += 1
    }
    DataW.effect_create {
        name: "#{equip_slot}ManaGainOnKill"
        category: "EquipEffect"
        init_func: (caster) =>
            @kill_tracker = caster.kills
        step_func: (caster) =>
            while caster.kills > @kill_tracker
                for _ in screens()
                    if caster\is_local_player()
                        EventLog.add("You regain mana for killing!", COL_PALE_BLUE)
                caster\heal_mp(5 + caster.stats.level)
                play_sound "sound/swish-10.wav"
                BonusesUtils.create_animation {n_animations: 0}, caster, "spr_effects.mana", 0.5
                @kill_tracker += 1
    }

DataW.effect_create {
    name: "PossiblySummonCentaurOnKill"
    --console_draw_func: (player, get_next) =>
    --    draw_console_effect(tosprite("spr_enemies.humanoid.centaur"), "Can summon after kill", get_next())
    category: "EquipEffect"
    init_func: (caster) =>
        @kill_tracker = caster.kills
    step_func: (caster) =>
        if caster\has_effect "Summoning"
            return
        while caster.kills > @kill_tracker
            if chance(0.03 * @n_derived)
                for _ in screens()
                    EventLog.add("A creature is summoned due to your graceful killing!!", COL_PALE_BLUE)
                play_sound "sound/summon.ogg"
                eff = caster\add_effect("Summoning", 5)
                eff.monster = "Centaur Hunter"
                eff.duration = 5
            @kill_tracker += 1
}

DataW.effect_create {
    name: "SummonMummyOnKill"
    --console_draw_func: (player, get_next) =>
    --    draw_console_effect(tosprite("spr_enemies.undead.mummy"), "Summons help every kill", get_next())
    category: "EquipEffect"
    init_func: (caster) =>
        @kill_tracker = caster.kills
    step_func: (caster) =>
        if caster\has_effect "Summoning"
            return
        while caster.kills > @kill_tracker
            for _ in screens()
                EventLog.add("A mummy is summoned due to your graceful killing!!", COL_PALE_BLUE)
            play_sound "sound/summon.ogg"
            eff = caster\add_effect("Summoning", 5)
            eff.monster = "Mummy"
            eff.duration = 5
            @kill_tracker += 1
}

DataW.effect_create {
    name: "PossiblySummonStormElementalOnKill"
    --console_draw_func: (player, get_next) =>
    --    draw_console_effect(tosprite("storm elemental"), "Can appear after a kill", get_next())
    category: "EquipEffect"
    init_func: (caster) =>
        @kill_tracker = caster.kills
    step_func: (caster) =>
        if caster\has_effect "Summoning"
            return
        while caster.kills > @kill_tracker
            if chance(0.05 * @n_derived)
                for _ in screens()
                    EventLog.add("A creature is summoned due to your graceful killing!!", COL_PALE_BLUE)
                play_sound "sound/summon.ogg"
                monster = "Storm Elemental"
                eff = caster\add_effect("Summoning", 5)
                eff.monster = (if type(monster) == "string" then monster else random_choice(monster))
                eff.duration = 5
            @kill_tracker += 1
}

DataW.effect_create {
    name: "PossiblySummonGolemOnKill"
    --console_draw_func: (player, get_next) =>
    --    draw_console_effect(tosprite("golem"), "Can appear after a kill", get_next())
    category: "EquipEffect"
    init_func: (caster) =>
        @kill_tracker = caster.kills
    step_func: (caster) =>
        while caster.kills > @kill_tracker
            if chance(0.03 * @n_derived)
                for _ in screens()
                    EventLog.add("A creature is summoned due to your graceful killing!!", COL_PALE_BLUE)
                play_sound "sound/summon.ogg"
                monster = "Golem"
                if not (caster\has_effect "Summoning")
                    eff = caster\add_effect("Summoning", 20)
                    eff.monster = (if type(monster) == "string" then monster else random_choice(monster))
                    eff.duration = 5
            @kill_tracker += 1
}
-------  STOP EQUIPMENT SLOT STACKED EFFECTS --------

DataW.effect_create {
    name: "Summoning"
    category: "Summon"
    effected_colour: COL_PALE_RED
    fade_out: 10
    --stat_func: (effect, obj, old, new) ->
    --    new.speed *= 2
    init_func: (caster) =>
        @n_steps = 0
        @delay = 1
        @duration = @time_left
        @on_summon = do_nothing
        @summon_xy = false
        @start_map = caster.map
        caster.summoned or= OrderedDict()
    step_func: (caster) =>
        @n_steps += 1
        if @n_steps == @delay
            if caster.map == @start_map
                ability = SpellObjects.SummonAbility.create {monster: @monster, :caster, xy: @summon_xy or caster.xy, duration: @duration, on_summon: @on_summon}
                GameObject.add_to_level(ability)
}

DataW.effect_create {
    name: "Reviving"
    remove_func: (player) =>
        player.is_ghost = false
        player.stats.hp = player\effective_stats().max_hp -- / 2 TODO should ghosts have half HP?
    draw_func: (player, top_left_x, top_left_y) =>
        xy = Display.to_screen_xy player.xy
        Fonts.small\draw({origin: Display.CENTER}, xy, "#{math.ceil @time_left / 60}")
}

-- Methods for implementing Aura's.
-- No extension mechanism, just called explicitly.
AuraBase = {
    init: () =>
        @n_steps = 0
        @total_time = @time_left
        @max_alpha = 0.15
        @n_ramp = 25
    step: () =>
        assert @range ~= nil, "'range' must be assigned to Aura effects."
    draw: (inner_col, outer_col, x, y) =>
        @n_steps += 1
        min = math.min(@n_ramp, if @n_steps > @total_time / 2 then math.abs(@n_steps - @total_time) else @n_steps)
        alpha = (min / @n_ramp)
        alpha = math.max(0.2, math.min(alpha, 1))
        alpha *= @max_alpha
        xy = Display.to_screen_xy {x, y}
        Display.draw_circle(with_alpha(inner_col, alpha), xy, @range)
        Display.draw_circle_outline(with_alpha(outer_col, alpha), xy, @range, 2)
}

DataW.effect_create {
    name: "Fear Aura"
    category: "Aura"
    effected_colour: {200, 200, 255}
    fade_out: 100
    effected_sprite: "spr_spells.cause_fear"
    init_func: (caster) =>
        AuraBase.init(@, caster)
        @max_alpha = 0.4
    step_func: (caster) =>
        AuraBase.step(@, caster)
        for mon in *(Map.enemies_list caster)
            if mon\has_effect("Fear")
                continue
            dist = vector_distance({mon.x, mon.y}, {caster.x, caster.y})
            if dist < @range
                mon\add_effect("Fear", 300)
    draw_func: (caster, top_left_x, top_left_y) =>
        AuraBase.draw(@, COL_GRAY, COL_BLACK, caster.x, caster.y)
}

DataW.effect_create {
    name: "Healing"
    category: "Aura"
    effected_sprite: "spr_amulets.healing"
    fade_out: 25
    step_func: (obj) =>
        {:hpregen} = obj\effective_stats()
        obj\heal_hp(hpregen * 20)
}

DataW.effect_create {
    name: "Healing Aura"
    category: "Aura"
    effected_sprite: "spr_amulets.healing"
    fade_out: 100
    init_func: (caster) =>
        AuraBase.init(@, caster)
        @max_alpha = 0.4
    step_func: (caster) =>
        AuraBase.step(@, caster)
        for ally in *Map.allies_list(caster)
            if ally\has_effect("Healing")
                continue
            dist = vector_distance({ally.x, ally.y}, {caster.x, caster.y})
            if dist < @range
                ally\add_effect("Healing", 200)
    draw_func: (caster, top_left_x, top_left_y) =>
        AuraBase.draw(@, COL_GRAY, COL_WHITE, caster.x, caster.y)
}

DataW.effect_create {
    name: "Daze Aura"
    category: "Aura"
    effected_sprite: "spr_amulets.light"
    fade_out: 100
    init_func: (caster) =>
        AuraBase.init(@, caster)
    step_func: (caster) =>
        AuraBase.step(@, caster)
        for mon in *(Map.enemies_list caster)
            if mon\has_effect("Dazed")
                continue
            dist = vector_distance({mon.x, mon.y}, {caster.x, caster.y})
            if dist < @range
                mon\add_effect("Dazed", 100)
    draw_func: (caster, top_left_x, top_left_y) =>
        AuraBase.draw(@, COL_PALE_YELLOW, COL_YELLOW, caster.x, caster.y)
}

DataW.effect_create {
    name: "Sap Aura"
    category: "Aura"
    fade_out: 100
    init_func: (caster) =>
        AuraBase.init(@, caster)
        if caster.is_enemy
            @range = 90
    step_func: (caster) =>
        AuraBase.step(@, caster)
        if caster.is_enemy and not Map.object_visible(caster)
            return
        for mon in *(Map.enemies_list caster)
            if mon\has_effect("Sapped")
                continue
            dist = vector_distance({mon.x, mon.y}, {caster.x, caster.y})
            if dist < @range
                mon\add_effect("Sapped", 35)
                mon.stats.mp = math.max(0, mon.stats.mp - 10)
                for _ in screens()
                    if mon\is_local_player()
                        EventLog.add("Your MP is drained!", {200,200,255})
                    elseif not mon.is_enemy
                        EventLog.add(mon.name .. "'s MP is drained!", {200,200,255})
    draw_func: (caster, top_left_x, top_left_y) =>
        @max_alpha = 0.35
        AuraBase.draw(@, COL_PALE_BLUE, COL_BLUE, caster.x, caster.y)
}

DataW.effect_create {
    name: "Pain Aura Anim Hack"
    category: "Aura"
    effected_sprite: "spr_spells.greaterpain"
    fade_out: 50
    init_func: (caster) =>
        AuraBase.init(@, caster)
        @mp_gain = 10
        if caster.is_enemy
            @range = 90
    step_func: (caster) =>
        AuraBase.step(@, caster)
        if @animation_only
            return
    draw_func: (caster, top_left_x, top_left_y) =>
        @max_alpha = 0.35
        if @animation_only
            @max_alpha /= 4
        elseif not caster.is_enemy and not caster\has_effect "Pained"
            @max_alpha /= 2
        AuraBase.draw(@, COL_PALE_RED, COL_RED, caster.x, caster.y)
}

DataW.effect_create {
    name: "Pain Aura"
    category: "Aura"
    effected_sprite: "spr_spells.greaterpain"
    fade_out: 50
    init_func: (caster) =>
        AuraBase.init(@, caster)
    apply_func: (caster, args) =>
        @mp_gain = 10
        @damage = args.damage
        @range = args.range
    step_func: (caster) =>
        AuraBase.step(@, caster)
        if @animation_only
            return
        if caster.is_enemy and not Map.object_visible(caster)
            return
        for mon in *(Map.enemies_list caster)
            if mon\has_effect("Pained")
                continue
            dist = vector_distance({mon.x, mon.y}, {caster.x, caster.y})
            if dist < @range
                mon\add_effect("Pained", 50)
                play_pained_sound()
                caster\add_effect("Pained", 50)
                stats = caster\effective_stats()
                damage, power = @damage, random(2,5) + stats.powerfulness
                power = power + TypeEffectUtils.get_power(caster, "Black")
                type_multiplier = TypeEffectUtils.get_resistance(mon, "Black")
                if mon\damage(damage, power, 1, caster, type_multiplier) then
                    {:stats} = caster
                    {:max_hp} = mon\effective_stats()
                    @damage += 5 -- Damage creep
                    @time_left += 100 -- Effect creep
                    if caster\has_effect("AmuletGreatPain")
                        caster\heal_hp(max_hp * 2/ 16)
                    else
                        caster\heal_hp(max_hp / 16)
                    --stats.mp = math.min(stats.max_mp, stats.mp + @mp_gain)
                    ---- Summon zombies by probability!?
                    --if caster\is_local_player()
                    --    EventLog.add("You drain the enemy's life force as MP!", {200,200,255})
                    --elseif not caster.is_enemy
                    --    EventLog.add(caster.name .. " drains the enemy's life force as MP!", {200,200,255})
    draw_func: (caster, top_left_x, top_left_y) =>
        @max_alpha = 0.35
        if @animation_only
            @max_alpha /= 4
        elseif not caster.is_enemy and not caster\has_effect "Pained"
            @max_alpha /= 2
        AuraBase.draw(@, COL_PALE_RED, COL_RED, caster.x, caster.y)
}

-- TODO separate into ItemEffects.moon

DataW.effect_create {
    name: "VampiricWeapon"
    console_draw_func: (player, get_next) =>
        draw_weapon_console_effect(player, M._vampirism, "Heal +25% dealt", get_next())
    on_melee_func: (attacker, defender, damage) =>
        for _ in screens()
            if attacker\is_local_player()
                EventLog.add("You steal the enemy's life!", {200,200,255})
        attacker\heal_hp(damage / 4 * @n_derived)
        return damage
}


-- TODO new effect
-- DataW.effect_create {
--     name: "Abolishment"
--     stat_func: (effect, obj, old, new) ->
--         new.powerfulness += math.ceil(new.powerfulness / 3)
--         new.powerfulness = 0
-- }

DataW.effect_create {
    name: "KnockbackWeapon"
    console_draw_func: (player, get_next) =>
        draw_weapon_console_effect(player, M._fleeing, "+10% chance of knockback", get_next())
    on_melee_func: (attacker, defender, damage) =>
        if defender\has_effect("Thrown")
            return damage
        if chance(.1 * @n_derived)
            thrown = defender\add_effect("Thrown", 20)
            thrown.angle = vector_direction({attacker.x, attacker.y}, {defender.x, defender.y})
        return damage
}

DataW.effect_create {
    name: "StunWeapon"
    console_draw_func: (player, get_next) =>
        draw_weapon_console_effect(player, M._fleeing, "+10% chance of stun", get_next())
    on_melee_func: (attacker, defender, damage) =>
        if defender\has_effect("Stun")
            return damage
        if chance(.1 * @n_derived)
            thrown = defender\add_effect("Stun", 20)
            thrown.angle = vector_direction({attacker.x, attacker.y}, {defender.x, defender.y})
        return damage
}

share_damage = (target, damage, min_health) ->
    -- TODO
    min_health = 0
    if target.stats.hp - damage < min_health
        damage = math.max(0, target.stats.hp - min_health)
    target.stats.hp -= damage

LAST_WARNING = GameVar.create("LAST_WARNING", -math.huge)
for name in *{"Ranger", "Fighter", "Templar", "Rogue", "Green Mage", "Black Mage", "Necromancer", "Stormcaller", "Pyrocaster", "Blue Mage"}
    DataW.effect_create {
        :name
        stat_func: (obj, old, new) =>
            if name == "Fighter"
                new.spell_cooldown_multiplier *= 2
            else
                new.melee_cooldown_multiplier *= 2
            
        init_func: (caster) =>
            @kill_tracker = caster.kills
            @links = {}
            @n_animations = 0
            LAST_WARNING\set -math.huge -- HACK
        step_func: (caster) =>
            -- Keep state for doing summons in a stateful effect:
            if caster\has_effect "Summoner"
                caster\add_effect "Summoner", 2 -- Keep effect from dying
            else
                eff = caster\add_effect "Summoner", 2
                eff.duration = 30
            while caster.kills > @kill_tracker
                for {:instance, :class_name} in *World.players
                    if class_name == "Necromancer"
                        for _ in screens()
                            if instance\is_local_player()
                                EventLog.add("You feed on the dying monster's energy!", COL_PALE_BLUE)
                        istats = instance.stats
                        if istats.mp < istats.max_mp
                            BonusesUtils.create_animation @, instance, "spr_bonuses.mana", 0.5
                        instance\heal_mp(10)
                @kill_tracker += 1
    }

-- Move in a direction, attacking everyone along the way
DataW.effect_create {
    name: "Dash Attack"
    effected_sprite: "spr_effects.i-stealth"
    can_use_rest: false
    effected_colour: {200, 200, 255, 255}
    fade_out: 10
    init_func: (caster) =>
        if Map.object_visible(caster)
            play_sound "sound/swish-12.wav"
        @steps = 0
        @n_hits = 0
        @attacked = {} -- No one attacked at first
    apply_func: (caster, args) =>
        @dir_vector = {math.cos(args.angle) * 16, math.sin(args.angle) * 16}
    step_func: (caster) =>
        @steps += 1
        -- Move forward best as possible:
        dxy = @dir_vector
        if caster.configure_dir
            dxy = caster\configure_dir(dxy)
        if dxy[1] == @dir_vector[1] and dxy[2] == @dir_vector[2]
            @dir_vector = GameObject.simulate_bounce(caster, @dir_vector)
            dxy = @dir_vector
        xy = {caster.x + dxy[1], caster.y + dxy[2]}
        if Map.object_tile_check(caster, xy)
            if Map.object_visible(caster)
                play_sound "sound/door.ogg"
            caster\remove_effect "Dash Attack"
            return
        caster.xy = xy
        if caster\has_ranged_weapon()
            return
        -- Attack nearby monsters:
        for mon in *Map.enemies_list(caster)
            dist = vector_distance(mon.xy, caster.xy)
            if dist < caster.target_radius + mon.target_radius --+ caster.weapon_range
                if @attacked[mon.id]
                    continue
                @attacked[mon.id] = true
                for _ in screens()
                    if caster\is_local_player()
                        EventLog.add("You strike as you pass!", {200,200,255})
                @n_hits += 1
                caster\melee(mon, 1.0) -- math.max(0.1, 1.0 / @n_hits))
    draw_func: (caster) =>
        for i=1,math.min(@steps, 12)
            xy = {
                caster.x - @dir_vector[1] * i
                caster.y - @dir_vector[2] * i
            }
            screen_xy = Display.to_screen_xy(xy)
            caster.sprite\draw({color: {255,255,255, 200 - 30 * i}, origin: Display.CENTER}, screen_xy)
    stat_func: (caster, old, new) =>
        new.speed = 0
        --caster.stats.attack_cooldown = 2
        new.powerfulness += caster.stats.level
        new.defence += caster.stats.level * 5
        new.willpower += caster.stats.level * 5
}

enemy_init = (enemy) -> nil

enemy_step = (enemy) -> nil

enemy_berserker_init = (enemy) ->
    enemy.berserk_time = 500

enemy_berserker_step = (enemy) ->
    enemy.berserk_time = enemy.berserk_time - 1
    if enemy.berserk_time <= 0 and Map.object_visible(enemy)
        enemy\add_effect("Berserk", 300)
        enemy.berserk_time = 500

DataW.effect_create {
    name: "Enraging"
    effected_sprite: "spr_effects.i-loudness-transparent"
    apply_func: (mon) =>
        @damage_tracker = 0
        -- Change May 16, make interval 66% of hp instead of 33%
        @damage_interval = mon.stats.max_hp * 2 / 3
        @next_hp_threshold = mon.stats.max_hp - @damage_interval
    on_receive_damage_func: (attacker, mon, dmg) =>
        @damage_tracker += dmg
        new_hp = mon.stats.hp - dmg
        if new_hp < @next_hp_threshold
            for _ in screens()
                EventLog.add((if mon.unique then "" else "The ") .. mon.name .. " gets mad!", {255,255,255})
            mon\add_effect("Charging", 100)
            @next_hp_threshold -= @damage_interval
        return dmg
}

DataW.effect_create {
    name: "Charging"
    stat_func: (mon, old, new) =>
        new.speed *= 4
        if @n_steps > @n_ramp
            new.melee_cooldown_multiplier *= 0.5
        if mon.name ~= "Gragh"
            new.hpregen *= 2
    effected_sprite: "spr_effects.i-loudness"
    effected_colour: {255,0,0}
    fade_out: 15
    apply_func: (mon) =>
        @n_steps = 0
        @n_ramp = 10
    step_func: (mon) =>
        if @n_steps % 60 == 0
            if Map.object_visible(mon)
                play_sound "sound/wavy.ogg"
        @n_steps += 1
    on_melee_func: (mon, defender, damage) =>
        do return nil
        if @n_steps > @n_ramp
            thrown = defender\add_effect("Thrown", 10)
            thrown.angle = vector_direction({mon.x, mon.y}, {defender.x, defender.y})
            for _ in screens()
                if not mon.is_enemy and mon\is_local_player()
                    EventLog.add("The " .. defender.name .." is thrown back!", {200,200,255})
                elseif not defender.is_enemy and defender\is_local_player()
                    EventLog.add("You are thrown back!", {200,200,255})
            -- mon\remove_effect("Charging")
        return damage
}

DataW.effect_create {
    name: "DebugMonsterEvalEffect"
    init_func: () =>
        @defender_count = 0
        @attacker_count = 0
        @melee_count = 0
        @projectile_count = 0
        @receive_melee_count = 0
    on_defend_func: () =>
        @defender_count += 1
    on_attack_func: () =>
        @attacker_count += 1
    --stat_func: () =>
    --attack_stat_func: () =>
    --step_func: () ->

    --console_draw_func,
    --remove_func,
    --remove_derived_func,
    --apply_derived_func,
    --apply_buff_func,
    on_melee_func: () =>
        @melee_count += 1
    on_projectile_func: () =>
        @projectile_count += 1
    on_receive_melee_func: () =>
        @receive_melee_count += 1
    --on_equip_func,
    --on_identify_func,
    --on_uncurse_func,
    --on_gain_spell_func,
}

return M
