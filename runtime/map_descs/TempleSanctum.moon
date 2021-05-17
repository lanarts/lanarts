OldMaps = require "maps.OldMaps"
Tilesets = require "tiles.Tilesets"
Vaults = require "maps.Vaults"
ItemUtils = require "maps.ItemUtils"
ItemGroups = require "maps.ItemGroups"
MapUtils = require "maps.MapUtils"
{:place_vault} = require "maps.VaultUtils"

return OldMaps.create_map_desc table.merge OldMaps.Dungeon2[3], {
    tileset: Tilesets.temple
    label: "Temple Sanctum"
    on_generate: (map) ->
        ---------------------------------------------------------------------
        -- Place key vault, along with 2 scrolls of experience and a randartsay --
        for type in *{"Azurite Key", "Scroll of Experience", "Scroll of Experience", ItemUtils.randart_generate(1).type}
            if not place_vault map, Vaults.small_item_vault {
                rng: map.rng
                item_placer: (map_, xy) ->
                    amount = 1
                    -- if not type
                    --     {:type, :amount} = ItemUtils.item_generate ItemGroups.enchanted_items
                    MapUtils.spawn_item(map_, type, amount, xy)
                tileset: Tilesets.snake
            }
                return false
        return true
}
