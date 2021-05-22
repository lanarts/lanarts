Map = require "core.Map"
ObjectUtils = require "objects.ObjectUtils"
DataW = require "DataWrapped"
GameObject = require "core.GameObject"
World = require "core.World"
GameState = require "core.GameState"
Display = require "core.Display"
SpellUtils = require "spells.SpellUtils"
TypeEffectUtils = require "spells.TypeEffectUtils"

M = nilprotect {} -- submodule

M.MOLD = {
    tosprite("spr_effects.mold_glowing1"),
    tosprite("spr_effects.mold_glowing2"),
    tosprite("spr_effects.mold_glowing3"),
    tosprite("spr_effects.mold_glowing4")
-- }

-- M.MOLD = {
    -- tosprite("spr_effects.mold_small1"),
    -- tosprite("spr_effects.mold_small2"),
    -- tosprite("spr_effects.mold_small3"),
    -- tosprite("spr_effects.mold_small4")
}

DataW.effect_create {
    name: "SlimeSlow"
    effected_sprite: "spr_effects.slow"
    effected_colour: {100, 255, 100}
    fade_out: 4
    stat_func: (obj, old, new) =>
        new.speed /= 2
}

M.SLIME_MOLD = SpellUtils.spell_object_type {
    types: {"Green"}
    init: (args) =>
        @sprite_index = random(1, #M.MOLD)
    on_step: () =>
        for obj in *Map.radius_collision_check(@map, 15, @xy)
            if (obj.team == nil) or obj.team == @caster.team
                continue
            if obj.destroyed
                continue
            --play_sound "sound/ringfire-hit.ogg"
            obj\add_effect "SlimeSlow", {time_left: 1}
    on_draw: () =>
        if not Map.object_visible @
            return
        alpha = 1.0
        if @in_time_slice(0.95, 1)
            alpha = 0
        elseif @in_time_slice(0.0, 0.1)
            alpha = @get_progress(0.0, 0.1)
        elseif @in_time_slice(0.9, 1)
            alpha = 1.0 - @get_progress(0.9, 1)
        frame = @time_passed()
        ObjectUtils.screen_draw(M.MOLD[@sprite_index], @xy, alpha / 2, frame)
}

return M
