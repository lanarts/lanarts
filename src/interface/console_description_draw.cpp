/*
 * content_draw_util.cpp:
 *  Common routines for content descriptions and drawing
 */

#include <ldraw/draw.h>
#include <ldraw/Font.h>

#include <luawrap/luawrap.h>

#include "draw/colour_constants.h"
#include "draw/draw_sprite.h"
#include "draw/SpriteEntry.h"
#include "data/lua_util.h"
#include "gamestate/GameState.h"
#include "objects/PlayerInst.h"

#include "stats/items/EquipmentEntry.h"
#include "stats/items/ProjectileEntry.h"
#include "stats/items/WeaponEntry.h"

#include "stats/SpellEntry.h"

#include "console_description_draw.h"

const char* projectile_speed_description(int speed) {
    if (speed < 1.5) {
        return "Very Slow";
    } else if (speed < 3) {
        return "Slow";
    } else if (speed < 5) {
        return "Medium";
    } else if (speed < 8) {
        return "Fast";
    }
    return "Very Fast";
}

static const char* range_description(int range) {
    if (range < 15) {
        return "Melee";
    } else if (range < 64) {
        return "Close";
    } else if (range < 200) {
        return "Medium";
    }
    return "Long";
}

const char* monster_speed_description(int speed) {
    return NULL;
}

const int WIDTH_THRESHOLD = 600; // Represents an 800 width resolution typically
const int MAX_WIDTH = 800; // Represents an 1000 width resolution typically
class DescriptionBoxHelper {
public:
    DescriptionBoxHelper(const BBox& bbox) :
        bbox(bbox), draw_index(0) {
        cols_per_row = 4;
        if (this->bbox.width() > MAX_WIDTH) {
            this->bbox.x2 = this->bbox.x1 + MAX_WIDTH;
        }
    }

    void draw_sprite_prefix(GameState* gs, sprite_id sprite) {
        Pos pos = get_next_draw_position();
        draw_sprite(sprite, pos.x, pos.y - TILE_SIZE / 2);
        value_draw_pos = Pos(pos.x + TILE_SIZE + 4, pos.y);
    }

    void draw_prefix(GameState* gs, const Colour& col, const char* fmt, ...) {
        Pos pos = get_next_draw_position();
        char buff[512];
        va_list ap;
        va_start(ap, fmt);
        vsnprintf(buff, 512, fmt, ap);
        va_end(ap);
        int offset = gs->font().draw(
            ldraw::DrawOptions(col).origin(ldraw::LEFT_CENTER), pos, buff);
        value_draw_pos = Pos(pos.x + offset, pos.y);
    }

    void draw_value(GameState* gs, const Colour& col, const char* fmt, ...) {
        Pos pos = value_draw_pos;
        char buff[512];
        va_list ap;
        va_start(ap, fmt);
        vsnprintf(buff, 512, fmt, ap);
        va_end(ap);
        gs->font().draw(ldraw::DrawOptions(col).origin(ldraw::LEFT_CENTER), pos,
                        buff);
    }

    int width() const {
        return bbox.width();
    }

    int height() const {
        return bbox.height();
    }

    static const int SX = TILE_SIZE / 2, SY = TILE_SIZE * 3 / 2 + 4;

    Pos get_next_draw_position() {
        int xinterval = bbox.width() / cols_per_row;
        int yinterval = TILE_SIZE;

        int col = draw_index % cols_per_row;
        int row = draw_index / cols_per_row;
        draw_index++;

        return Pos(bbox.x1 + SX + xinterval * col, bbox.y1 + SY + yinterval * row);
    }
    int get_draw_index() {
        return draw_index;
    }

private:
    int cols_per_row;
    /* Members */
    int draw_index;
    BBox bbox;
    Pos value_draw_pos;
};

static void draw_base_entry_overlay(GameState* gs, ResourceEntryBase& entry) {
    GameTextConsole& console = gs->game_console();
    BBox bbox(console.bounding_box());
    int descriptxoff = TILE_SIZE * 1.25;
    int itemoffset = draw_icon_and_name(gs, entry, Colour(), bbox.x1 + 4,
                                        bbox.y1 + 4, TILE_SIZE * 1.25 - 4, TILE_SIZE / 4);
    int typeoffset = gs->font().drawf(
        ldraw::DrawOptions(COL_LIGHT_GRAY).origin(ldraw::LEFT_CENTER),
        Pos(bbox.x1 + descriptxoff, bbox.y1 + TILE_SIZE * .75 + 4),
        entry.entry_type());

    const int x_offset = bbox.width() / 6, y_offset = 4 + TILE_SIZE / 2;
    const int max_width = bbox.width() - x_offset * 2;

    int descript_xoffset = x_offset;
    if (descript_xoffset < WIDTH_THRESHOLD / 6) {
        descript_xoffset = WIDTH_THRESHOLD / 6;
    }
    if (descript_xoffset < itemoffset + descriptxoff + 4) {
        descript_xoffset = itemoffset + descriptxoff + 4;
    }
    if (descript_xoffset < typeoffset + descriptxoff + 4) {
        descript_xoffset = typeoffset + descriptxoff + 4;
    }

    using namespace ldraw;

    gs->font().draw_wrapped(DrawOptions(COL_LIGHT_GRAY).origin(LEFT_CENTER),
                            bbox.left_top() + Pos(descript_xoffset, y_offset), max_width,
                            entry.description);
}

int draw_icon_and_name(GameState* gs, ResourceEntryBase& entry, Colour col, int x,
                       int y, int xoffset, int yoffset) {

    entry.draw(ldraw::DrawOptions(), {x, y});
    ldraw::draw_rectangle_outline(COL_PALE_YELLOW.alpha(50),
                                  BBox(x, y, x + TILE_SIZE, y + TILE_SIZE));
    /* Draw item name */
    return gs->font().draw(ldraw::DrawOptions(col).origin(ldraw::LEFT_CENTER), Pos(x + xoffset, y + yoffset),
                           entry.name);
}

void draw_spell_icon_and_name(GameState* gs, SpellEntry& spl_entry, Colour col,
                              int x, int y) {
    draw_sprite(spl_entry.sprite, x, y);

    ldraw::draw_rectangle_outline(COL_PALE_YELLOW.alpha(50),
                                  BBox(x, y, x + TILE_SIZE, y + TILE_SIZE));
    /* Draw spell name */
    gs->font().draw(ldraw::DrawOptions(col).origin(ldraw::LEFT_CENTER),
                    Pos(x + TILE_SIZE * 1.25, y + TILE_SIZE / 2), spl_entry.name);
}

//Drawn only as part of other draw_console_<something>_description functions
static void draw_value(GameState* gs, DescriptionBoxHelper& dbh,
                       const char* name, int bonus, const Colour& prefixcol = COL_GREEN,
                       const Colour& valuecol = COL_PALE_GREEN, bool optional = true,
                       bool draw_plus_sign = false) {
    if (!optional || bonus != 0) {
        dbh.draw_prefix(gs, prefixcol, "%s", name);
        dbh.draw_value(gs, valuecol,
                       (bonus > 0 && draw_plus_sign) ? "+%d" : "%d", bonus);
    }
}

//Drawn only as part of other draw_console_<something>_description functions
static void draw_bonus(GameState* gs, DescriptionBoxHelper& dbh,
                       const char* name, int bonus, const Colour& prefixcol = COL_GREEN,
                       const Colour& valuecol = COL_PALE_GREEN, bool optional = true) {
    draw_value(gs, dbh, name, bonus, bonus > 0 ? prefixcol : COL_LIGHT_RED,
               bonus > 0 ? valuecol : COL_PALE_RED, optional, true);
}

static void draw_damage_per_second(
    GameState* gs, 
    DescriptionBoxHelper& dbh,
    float cooldown,
    CoreStatMultiplier& damage, 
    CoreStatMultiplier& power, 
    CoreStats& core,
    Colour prefixcol = COL_GREEN, 
    Colour valuecol = COL_PALE_GREEN
) {
    Range damage_range = damage.calculate_range(core);
    Range power_range = power.calculate_range(core);
    float hits_per_second = 60.0f / cooldown;
    float power_bonus = 1 + 0.05f * (power_range.min + power_range.max) / 2.0f;
    float raw_damage_per_second = (damage_range.min + damage_range.max) * hits_per_second / 2.0f;
    dbh.draw_prefix(gs, prefixcol, "Hit/second: ");
    dbh.draw_value(gs, valuecol, "%.1f", raw_damage_per_second * power_bonus);
}

static void draw_statmult(GameState* gs, DescriptionBoxHelper& dbh,
                          const char* name, CoreStatMultiplier& mult, CoreStats& core,
                          Colour prefixcol = COL_GREEN, Colour valuecol = COL_PALE_GREEN,
                          bool optional = true) {
    if (!optional || !mult.is_empty()) {
        Range value_range = mult.calculate_range(core);
        if (value_range.min < 0) {
            prefixcol = COL_LIGHT_RED;
            valuecol = COL_PALE_RED;
        }
        dbh.draw_prefix(gs, prefixcol, "%s", name);
        if (value_range.min == value_range.max) {
            dbh.draw_value(gs, valuecol, "%d", value_range.min);
        } else {
            dbh.draw_value(gs, valuecol, "%d to %d", value_range.min,
                           value_range.max);
        }
    }
}

static void draw_percentage_modifier(GameState* gs, DescriptionBoxHelper& dbh,
                                     float modifier, const char* prefix, const Colour& prefixcol = COL_GREEN,
                                     const Colour& valuecol = COL_PALE_GREEN, bool optional = true) {
    int percentage_mod = round(modifier * 100 - 100);
    bool negative = percentage_mod < 0;
    if (!optional || percentage_mod != 0) {
        dbh.draw_prefix(gs, negative ? COL_LIGHT_RED : prefixcol, "%s", prefix);
        Colour statcol = percentage_mod < 0 ? COL_PALE_RED : valuecol;
        dbh.draw_value(gs, statcol, percentage_mod > 0 ? "+%d%%" : "%d%%",
                       percentage_mod);
    }
}

static void draw_stat_bonuses_overlay(GameState* gs, DescriptionBoxHelper& dbh,
                                      CoreStats& core) {
    draw_bonus(gs, dbh, "Power: ", core.powerfulness);
    draw_bonus(gs, dbh, "Defence: ", core.defence);
    draw_bonus(gs, dbh, "Will: ", core.willpower);
    draw_bonus(gs, dbh, "HP: ", core.max_hp);
    draw_bonus(gs, dbh, "MP: ", core.max_mp);
    if (core.hpregen > 0) {
        dbh.draw_prefix(gs, COL_PALE_GREEN, "+HP/second: ");
        dbh.draw_value(gs, COL_PALE_GREEN, "%.2f", core.hpregen * 60.f);
    } else if (core.hpregen < 0) {
        dbh.draw_prefix(gs, COL_RED, "-HP/second: ");
        dbh.draw_value(gs, COL_RED, "%.1f", -core.hpregen * 60.f);
    }
    if (core.mpregen > 0) {
        dbh.draw_prefix(gs, COL_PALE_BLUE, "+MP/second: ");
        dbh.draw_value(gs, COL_PALE_GREEN, "%.1f", core.mpregen * 60.f);
    } else if (core.mpregen < 0) {
        dbh.draw_prefix(gs, COL_RED, "-MP/second: ");
        dbh.draw_value(gs, COL_RED, "%.1f", -core.mpregen * 60.f);
    }
    draw_percentage_modifier(gs, dbh, core.spell_velocity_multiplier,
                             "Spell Velocity: ");
}

static void draw_cooldown_modifiers_overlay(GameState* gs,
                                            DescriptionBoxHelper& dbh, CooldownModifiers& cooldown) {
    draw_percentage_modifier(gs, dbh, 1.0f / cooldown.melee_cooldown_multiplier,
                             "Melee Rate: ");
    draw_percentage_modifier(gs, dbh, 1.0f / cooldown.spell_cooldown_multiplier,
                             "Casting Rate: ");
    draw_percentage_modifier(gs, dbh,
                             1.0f / cooldown.ranged_cooldown_multiplier, "Firing Rate: ");
    draw_percentage_modifier(gs, dbh, 1.0f / cooldown.rest_cooldown_multiplier,
                             "Resting Cooloff: ");
}

static void draw_defence_bonuses_overlay(GameState* gs,
                                         DescriptionBoxHelper& dbh, ArmourStats& armour) {
    PlayerInst* p = gs->local_player();
    CoreStats& core = p->effective_stats().core;
    const char* dmgresist = "+Defence: ";
    const char* mdmgresist = "+Willpower: ";
    if (dbh.width() < WIDTH_THRESHOLD) {
        dmgresist = "+Def: ";
        mdmgresist = "+Will: ";
    }
    draw_statmult(gs, dbh, dmgresist, armour.resistance, core, COL_GOLD);
    draw_statmult(gs, dbh, mdmgresist, armour.magic_resistance, core,
                  COL_BABY_BLUE);
}

static void draw_damage_bonuses_overlay(GameState* gs,
                                        DescriptionBoxHelper& dbh, DamageStats& damage) {
    PlayerInst* p = gs->local_player();
    CoreStats& core = p->effective_stats().core;
    draw_statmult(gs, dbh, "+Damage: ", damage.damage_stats, core);
    draw_statmult(gs, dbh, "+Power: ", damage.power_stats, core);
}

static void draw_spells_granted_overlay(GameState* gs,
                                        DescriptionBoxHelper& dbh, SpellsKnown& spells_granted) {
    for (int i = 0; i < spells_granted.amount(); i++) {
        SpellEntry& entry = spells_granted.get_entry(i);
        dbh.draw_sprite_prefix(gs, entry.sprite);
        dbh.draw_value(gs, COL_PALE_YELLOW, "Grants \"%s\"", entry.name.c_str());
    }
}

static void call_console_draw_func(const LuaValue& console_draw_func, const LuaValue& context, PlayerInst* player,
                                   DescriptionBoxHelper& dbh) {
    lcall(console_draw_func, context, player, [&dbh]() -> Pos {
        return dbh.get_next_draw_position();
    });
}

static void draw_effect_modifiers_overlay(GameState* gs,
                                          DescriptionBoxHelper& dbh, StatusEffectModifiers& status_effects) {
    for (StatusEffect status_effect : status_effects.status_effects) {
        EffectEntry& entry = game_effect_data.get(status_effect.id);
        call_console_draw_func(entry.console_draw_func, status_effect.args, gs->local_player(), dbh);
    }
}

static void draw_equipment_description_overlay(GameState* gs,
                                               DescriptionBoxHelper& dbh, const Item& item) {
    EquipmentEntry& entry = item.equipment_entry();
    draw_stat_bonuses_overlay(gs, dbh, entry.core_stat_modifier());
    draw_defence_bonuses_overlay(gs, dbh, entry.armour_modifier());
    draw_damage_bonuses_overlay(gs, dbh, entry.damage_modifier());
    draw_cooldown_modifiers_overlay(gs, dbh, entry.cooldown_modifiers);
    draw_spells_granted_overlay(gs, dbh, entry.spells_granted);
    draw_effect_modifiers_overlay(gs, dbh, entry.effect_modifiers);
    //if (dbh.get_draw_index() % 2 == 1) {
    //    dbh.get_next_draw_position();
    //}
    call_console_draw_func(entry.console_draw_func, entry.raw, gs->local_player(), dbh);
}

static void draw_attack_description_overlay(
    GameState* gs,
    DescriptionBoxHelper& dbh, 
    CoreStats& core, 
    CooldownModifiers& modifiers,
    Attack& attack
) {
    // TODO other cooldown multipliers
    
    float cooldown = 
        attack.cooldown * modifiers.melee_cooldown_multiplier * (1.0f - attack.magic_percentage()) +
        attack.cooldown * modifiers.spell_cooldown_multiplier * attack.magic_percentage();
    draw_damage_per_second(
        gs, 
        dbh, 
        cooldown, 
        attack.damage_stats(), 
        attack.power_stats(), 
        core,
        COL_PALE_YELLOW, 
        COL_PALE_GREEN
    );
    // draw_statmult(gs, dbh, "Power: ", attack.power_stats(), core,
    //               COL_PALE_YELLOW, COL_PALE_GREEN, false);

    if (attack.range >= 15) {
        dbh.draw_prefix(gs, COL_PALE_YELLOW, "Range: ");
//                if (p->class_stats().class_entry().name != "Ranger" && p->weapon().weapon_entry().weapon_class == "bows") {
//		    dbh.draw_value(gs, COL_PALE_RED, "Poor (Unproficient)");
//                } else {
        dbh.draw_value(gs, COL_PALE_GREEN, "%s",
                       range_description(attack.range));
//                }
    }
    draw_value(gs, dbh, "Cooldown: ", int(cooldown), COL_PALE_YELLOW,
               COL_PALE_RED);
}

static void draw_console_spell_stats(GameState* gs, DescriptionBoxHelper& dbh, SpellEntry& entry);

static void draw_weapon_description_overlay(GameState* gs,
                                            DescriptionBoxHelper& dbh, const Weapon& weapon) {
    WeaponEntry& entry = weapon.weapon_entry();
    PlayerInst* p = gs->local_player();
    CoreStats& core = p->effective_stats().core;
    CooldownModifiers& modifiers = p->effective_stats().cooldown_modifiers;

    if (entry.attack.alt_spell != -1) {
        draw_console_spell_stats(gs, dbh, entry.attack.alt_spell_entry());
    } else {
        draw_attack_description_overlay(gs, dbh, core, modifiers, entry.attack);
    }
    draw_equipment_description_overlay(gs, dbh, weapon);
}

static void draw_projectile_description_overlay(GameState* gs,
                                                DescriptionBoxHelper& dbh, const Projectile& projectile) {
    ProjectileEntry& entry = projectile.projectile_entry();
    if (entry.is_standalone()) {
        PlayerInst* p = gs->local_player();
        CoreStats& core = p->effective_stats().core;
        CooldownModifiers& modifiers = p->effective_stats().cooldown_modifiers;
        draw_attack_description_overlay(gs, dbh, core, modifiers, entry.attack);
    }
    draw_equipment_description_overlay(gs, dbh, projectile);
}

static void draw_console_spell_stats(GameState* gs, DescriptionBoxHelper& dbh, SpellEntry& entry) {
    GameTextConsole& console = gs->game_console();
    draw_value(gs, dbh, "MP cost: ", entry.mp_cost, COL_PALE_YELLOW,
               COL_PALE_RED);

    if (!entry.projectile.empty()) {
        draw_projectile_description_overlay(gs, dbh, entry.projectile);
    } else {
        draw_value(gs, dbh, "Cooldown: ", std::max(entry.cooldown, entry.spell_cooldown), COL_PALE_YELLOW,
                   COL_PALE_RED);
    }
    lcall(entry.console_draw_func, gs->local_player(), [&dbh]() -> Pos {
        return dbh.get_next_draw_position();
    });
}

void draw_console_spell_description(GameState* gs, SpellEntry& entry) {
    GameTextConsole& console = gs->game_console();
    DescriptionBoxHelper dbh(console.bounding_box());

    if (console.has_content_already()) {
        return;
    }
    console.draw_box(gs);
    draw_base_entry_overlay(gs, entry);

    draw_console_spell_stats(gs, dbh, entry);
}

void draw_console_enemy_description(GameState* gs, EnemyEntry& entry) {
    GameTextConsole& console = gs->game_console();

    if (console.has_content_already()) {
        return;
    }

    console.draw_box(gs);
    draw_base_entry_overlay(gs, entry);
    CoreStats& ecore = entry.basestats.core;

    DescriptionBoxHelper dbh(console.bounding_box());

    lcall(entry.console_draw_func, game_enemy_data.get_raw_data()[entry.name], [&dbh]() -> Pos {
        return dbh.get_next_draw_position();
    });
    draw_value(gs, dbh, "HP: ", ecore.hp, COL_PALE_YELLOW, COL_PALE_RED);
    draw_value(gs, dbh, "Power: ", ecore.powerfulness, COL_PALE_YELLOW,
               COL_PALE_RED);
    draw_value(gs, dbh, "Defence: ", ecore.defence, COL_PALE_YELLOW,
               COL_PALE_RED);
    draw_value(gs, dbh, "Will: ", ecore.willpower, COL_PALE_YELLOW,
               COL_PALE_RED);

    auto core = entry.basestats.effective_stats(gs, NULL).core;
    auto modifiers = entry.basestats.effective_stats(gs, NULL).cooldown_modifiers;
    WeaponEntry& weap = entry.basestats.attacks[0].weapon.weapon_entry();
    draw_attack_description_overlay(gs, dbh, core, modifiers, weap.attack);
}

void draw_console_item_description(GameState* gs, const Item& item,
                                   ItemEntry& entry) {
    GameTextConsole& console = gs->game_console();

    if (console.has_content_already()) {
        return;
    }
    console.draw_box(gs);
    draw_base_entry_overlay(gs, entry);

    // Hackishly position the content where it should be:
    BBox window_region = gs->screens.window_region();
    //gs->game_chat().add_message(format("Window region %d, %d", window_region.x1, window_region.y1),
    //                COL_PALE_RED);
    BBox bbox = console.bounding_box();
    //bbox.translate(window_region.x1, window_region.y1);
    DescriptionBoxHelper dbh(bbox);
    //DescriptionBoxHelper dbh(console.bounding_box());

    if (is_item_projectile(entry)) {
        draw_projectile_description_overlay(gs, dbh, item);
    } else if (is_item_weapon(entry)) {
        draw_weapon_description_overlay(gs, dbh, item);
    } else if (item.is_equipment()) {
        draw_equipment_description_overlay(gs, dbh, item);
    }
}
