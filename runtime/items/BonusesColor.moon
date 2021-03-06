{:draw_console_text, :draw_console_effect} = require 'ui.ConsoleUtils'
{:define_bonus} = require "items.Bonuses"
{:of_color, :draw_color_bound_effect} = require "items.BonusesUtils"

-- +1/+1 COLOR BONUSES
define_color_bonus = (name) ->
    local BONUS
    BONUS = define_bonus {
        :name
        shop_cost: {50, 100}
        sprite: tosprite("spr_bonuses.#{name\lower()}")
        effect: {
            console_draw_func: (obj, get_next) =>
                draw_color_bound_effect BONUS, obj, get_next, {name},
                    "+1 Power, Improved Spells."
                get_next()
            stat_func: (obj, old, new) =>
                if of_color obj, name
                    new.powerfulness += @n_derived
        }
        item_draw_func: (options, x, y) =>
            @sprite\draw options, {x, y}
    }

define_color_bonus "Black"
define_color_bonus "White"
define_color_bonus "Red"
define_color_bonus "Green"
define_color_bonus "Blue"
