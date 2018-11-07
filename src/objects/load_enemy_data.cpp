#include <fstream>
#include <yaml-cpp/yaml.h>
#include <luawrap/luawrap.h>

#include "data/game_data.h"

#include "data/parse.h"

#include "lua_api/lua_yaml.h"
#include "stats/stats.h"
#include "stats/load_stats.h"

#include "EnemyEntry.h"

using namespace std;


std::vector<StatusEffect> load_statuses(const LuaValue& effects) {
    if (effects.isnil()) {
        return {};
    }
    std::vector<StatusEffect> ret;
    for (int i = 1; i <= effects.objlen(); i++) {
        if (effects[i].is<const char*>()) {
            effect_id id = get_effect_by_name(effects[i].to_str());
            ret.push_back({id, LuaValue(effects.luastate())});
        } else {
            effect_id id = get_effect_by_name(effects[i][1].to_str());
            LuaValue args = effects[i][2];
            ret.push_back({id, args});
        }
    }
    return ret;
}

static EnemyEntry parse_enemy_type(const LuaStackValue& table) {
    using namespace luawrap;
    lua_State* L = table.luastate();
    EnemyEntry entry;

    entry.name = table["name"].to_str();
    entry.description = defaulted(table, "description", std::string());

    entry.appear_msg = defaulted(table, "appear_message", std::string());
    entry.defeat_msg = defaulted(table, "defeat_message", std::string());

    if (!table["sprite"].isnil()) {
        entry.enemy_sprite = res::sprite_id(table["sprite"].to_str());
    }
    if (!table["death_sprite"].isnil()) {
        entry.death_sprite = res::sprite_id(table["death_sprite"].to_str());
    }
    entry.radius = defaulted(table, "radius", 12);
    entry.xpaward = table["xpaward"].to_num();

    entry.basestats = parse_combat_stats(table["stats"]);
    entry.unique = defaulted(table, "unique", false);
    entry.vision_radius = defaulted(table, "vision_radius", 7);

    entry.init_event = table["init_func"];
    entry.step_event = table["step_func"];
    entry.draw_event = table["draw_func"];
    entry.death_event = table["death_func"];
    entry.on_post_draw = table["on_post_draw"];
    entry.console_draw_func = table["console_draw_func"];
    entry.target_draw_func = table["target_draw_func"];

    entry.effect_modifiers.status_effects = load_statuses(table["effects_active"]);
    entry.raw_table = table;
    return entry;
}

void lapi_data_create_enemy(const LuaStackValue& table) {
    EnemyEntry entry = parse_enemy_type(table);
    game_enemy_data.new_entry(entry.name, entry, table);
}

