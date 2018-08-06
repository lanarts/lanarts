-- {:draw_console_text, :draw_console_effect} = require 'ui.ConsoleUtils'
-- Display = require "core.Display"
-- DataW = require "DataWrapped"
--
-- BONUSES = OrderedDict()
-- define_bonus = (bonus) ->
--     BONUSES[bonus.name] = bonus
--
--     if bonus.stat_func
--         DataW.additive_effect_create {
--             name: "#{bonus.name}Effect"
--             key: "amount" -- Additive effect, accessed with @value().
--             stat_func: (obj, old, new) =>
--                 bonus\stat_func(@value(), obj, old, new)
--         }
--         bonus.effects_granted = {{"#{bonus.name}Effect", {amount: 1}}}
--
-- -- +1/+1 COLOR BONUSES
-- define_color_bonus = (name, classes) ->
--     define_bonus {
--         :name
--         sprite: tosprite("spr_bonuses.#{name\lower()}")
--         console_draw_func: (obj, get_next) =>
--             draw_console_effect get_next(), @sprite, {
--                 {COL_LIGHT_GRAY, "#{@name}: "}
--                 {COL_WHITE, "#{@name} classes get +1 Strength, +1 Magic."}
--             }
--             get_next()
--         stat_func: (amount, obj, old, new) =>
--             -- TODO dont hardcode class lists
--             for klass in *classes
--                 if obj\has_effect(klass)
--                     new.strength += amount
--                     new.magic += amount
--                     return
--         item_draw_func: (options, x, y) =>
--             @sprite\draw options, {x, y}
--     }
--
-- define_color_bonus "Black", {"Necromancer"}
-- define_color_bonus "White", {"Stormcaller"}
-- define_color_bonus "Red", {"Pyrocaster"}
-- define_color_bonus "Green", {"Fighter"}
-- define_color_bonus "Blue", {}
--
-- -- ITEM COST BONUSES
-- -- TODO NEVER PUT LUXURY IN STORES
--
-- define_bonus {
--     name: "Luxury"
--     appears_in_stores: false -- TODO use
--     sprite: tosprite("spr_bonuses.luxury")
--     console_draw_func: (obj, get_next) =>
--         draw_console_effect get_next(), @sprite, {
--             {COL_GOLD, "Luxury: "}
--             -- TODO is this just like finding money?
--             {COL_WHITE, "Sells for 3x."}
--         }
--     item_draw_func: (options, x, y) =>
--         @sprite\draw options, {x, y}
-- }
--
-- -- STUN BONUS
-- -- TODO only put on weapons that go boof
--
-- define_bonus {
--     name: "Knockback"
--     sprite: tosprite "spr_effects.fleeing"
--     effects_granted: {{"KnockbackWeapon", 1}}
--     item_draw_func: (options, x, y) =>
--         options = table.merge options, {scale: {1,1}}
--         @sprite\draw options, {x - 20, y}
-- }
--
-- -- STUN BONUS
-- -- TODO only put on weapons that go boof
--
--
-- BONUS_LOCS = {
--     {0,16}
--     {16,16}
--     {0,0}
--     {16,0}
-- }
--
-- equipment_derive = (entry) ->
--     adjective = table.concat entry.bonuses, " "
--     bonus_objs = (for bonus in *entry.bonuses do BONUSES[bonus])
--
--     -- Call 'apply' on each bonus object
--     for bonus in *bonus_objs
--         if bonus.apply
--             bonus\apply(entry)
--
--     -- Compute name
--     entry.name =  "#{adjective} #{entry.name}"
--
--     -- Draw console description
--     entry.console_draw_func = (obj, get_next) =>
--         for bonus in *bonus_objs
--             if bonus.console_draw_func
--                 bonus\console_draw_func(obj, get_next)
--
--     -- Calculate auto_equip status
--     if entry.auto_equip == nil
--         entry.auto_equip = true
--     for {:auto_equip} in *bonus_objs
--         if auto_equip == false
--             entry.auto_equip = false
--
--     -- Draw item minisprite overlays
--     entry.item_draw_func = (options, x, y) =>
--         options = {color: options.color, scale: {0.5, 0.5}}
--         for i, bonus in ipairs bonus_objs
--             if bonus.item_draw_func and BONUS_LOCS[i]
--                 {bx, by} = BONUS_LOCS[i]
--                 bonus\item_draw_func(options, x + bx, y + by)
--
--     entry.effects_granted or= {}
--     for {:effects_granted} in *bonus_objs
--         if effects_granted
--             entry.effects_granted = table.tconcat(entry.effects_granted, effects_granted)
--     return entry
--
-- -- TODO properly calculate shop cost
-- -- TODO -- piggy back into randarts system - use new tech to enhance RandartsBonuses
-- for bonus1 in *BONUSES.__keys
--     for bonus2 in *BONUSES.__keys
--         Data.equipment_create equipment_derive {
--             name: "Ring"
--             description: ""
--             type: "ring"
--             bonuses: {bonus1, bonus2}
--             shop_cost: {30, 90}
--             spr_item: "spr_rings.ivory"
--         }
require "items.BonusesAll"
