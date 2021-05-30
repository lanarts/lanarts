argparse = require "argparse"
EngineInternal = require "core.EngineInternal"
Network = require "core.Network"
ResourceLoading = require "engine.ResourceLoading"
StartEngine = require "engine.StartEngine"
EngineHooks = require "engine.EngineBase"
Settings = require "engine.Settings"
EventLog = require "ui.EventLog"

load_location_is_valid = (load_file) ->
    if not load_file
        return -- No savefile specified:
    if file_exists(load_file)
        return load_file
    else
        error("'#{load_file}' does not exist!")

engine_init = (args) ->
    Settings.settings_save(settings)
    GameState = require("core.GameState")
    -- Player config
    GameState.clear_players()

    EngineInternal.start_connection()
    if settings.connection_type ~= Network.CLIENT
        n_players = (if os.getenv("LANARTS_CONTROLLER") then 0 else 1) + #require("core.Gamepad").ids()
        GameState.register_player(settings.username, settings.class_type, Engine.player_input, true, 0, 0)
        classes = {
            args.class,
            args.class2 or "Fighter",
            args.class3 or "Stormcaller",
            args.class4 or "Pyrocaster"
        }
        for i=2,n_players
            GameState.register_player("Player " .. i, classes[i], Engine.player_input, true, i - 1, 0)

-- dump_module = (mod_name) ->
--     local f
--     to_class_case = (str) ->
--         parts = str\split("_")
--         parts = [part\gsub("^%l", string.upper) for part in *parts]
--         return table.concat parts, ""
--     file_name = mod_name\gsub("%.", "/") .. ".d.ts"
--     interfaces = {}
--     export_interface = (name, value) ->
--         parts = {"export interface #{to_class_case name} {"}
--         if getmetatable(value) and getmetatable(value).__call 
--             append parts, "    (this: void, ...args: any): any;"
--         for k,v in pairs value
--             if type(v) == "function"
--                 append parts, "    #{k}(this: void, ...args: any): any;"
--             elseif type(v) == "table" and #v > 0
--                 append parts, "    #{k}: any[];"
--             elseif type(v) == "table"
--                 export_interface(name .. (to_class_case k), v)
--                 append parts, "    #{k}: #{name .. to_class_case k};"
--             else
--                 append parts, "    #{k}: any;"
--         append parts, "}"
--         append interfaces, table.concat(parts, "\n")

--     flush_interfaces = () ->
--         f\write(table.concat(interfaces, "\n"))
--         f\write("\n")
--         interfaces = {}

--     mod_object = require(mod_name)
--     print(mod_name,type(mod_object))
--     if type(mod_object) ~= "table"
--         return
--     f = io.open(file_name, "w")
--     for k,v in pairs mod_object
--         if type(v) == "function"
--             f\write("export function #{k}(this: void, ...args: any): any\n")
--         elseif type(v) == "table" and #v > 0
--             f\write("export const #{k}: any[];\n")
--         elseif type(v) == "table"
--             export_interface(k, v)
--             flush_interfaces()
--             f\write("export const #{k}: #{to_class_case k};\n")
--         else 
--             f\write("export const #{k}: any;\n")
--     f\close()

game_init = (args, load_file=nil) ->
    log "Initializing GameState object..."
    EngineInternal.init_gamestate()
    GameState = require("core.GameState")
    load_file = load_file or os.getenv("MULTI_LOAD_FILE")
    -- Run require files that custom e.g. scenario
    if args.require
        require(args.require)
    if load_location_is_valid(load_file)
        GameState.load(load_file)
    else
        EngineInternal.start_game()

dump_globals = () ->
    -- TODO make a file?
    setmetatable(_G, nil)
    for k, v in spairs(_G)
        print "      \"#{k}\""
    os.exit()

--Parse lanarts command-line options
run_lanarts = (raw_args) ->
    log "Running Lanarts main function"
    parser = argparse("lanarts", "Run lanarts.")
    parser\option "--debug", "Attach debugger."
    parser\option "--nofilter", "Do not filter Lua error reporting of noise."
    parser\option "--class", "Class to immediately start with.", false
    parser\option "--class2", "Player 2's class.", false
    parser\option "--class3", "Player 3's class.", false
    parser\option "--class4", "Player 4's class.", false
    parser\option "--macro", "Script to run from debug_scripts/"
    parser\option "--require", "Script to run from runtime/"
    parser\option "-C --context", "Amount of lines of Lua error context.", "4"
    parser\option "--settings", "Settings YAML file to use.", "settings.yaml"
    parser\option "--save", "Save file to save to.", false
    parser\option "--load", "Save file to load from.", false
    args = parser\parse(raw_args)
    -- TODO dont use global 'settings' object
    initial_settings = Settings.settings_load(args.settings)

    local entry_point, pregame_start, game_start, game_step
    entry_point = () ->
        -- dump_module("core.World")
        -- dump_module("core.Map")
        -- dump_module("core.SourceMap")
        -- dump_module("tiles.Tilesets")
        -- dump_module("objects.HealingSquare")
        -- dump_module("objects.SpellObjects")
        -- dump_module("input.GamepadInputSource")
        -- dump_module("input.KeyboardDraw")
        -- dump_module("map_descs.PixullochiaDepths")
        -- dump_module("map_descs.OutpostStockroom")
        -- dump_module("map_descs.SnakePitEntrance")
        -- dump_module("map_descs.TempleSanctum")
        -- dump_module("map_descs.Overworld")
        -- dump_module("map_descs.HiveDepths")
        -- dump_module("map_descs.TempleEntrance")
        -- dump_module("map_descs.Hell")
        -- dump_module("map_descs.OutpostEntrance")
        -- dump_module("map_descs.Crypt")
        -- dump_module("map_descs.World")
        -- dump_module("map_descs.TempleChamber")
        -- dump_module("map_descs.SnakePitDepths")
        -- dump_module("map_descs.GraghsLair")
        -- dump_module("map_descs.DebugArena")
        -- dump_module("map_descs.HiveEntrance")
        -- dump_module("map_descs.Underdungeon")
        -- dump_module("map_descs.OgreLair")
        -- dump_module("map_descs.PixullochiaEntrance")
        -- dump_module("maps.MapUtils")
        -- dump_module("maps.OldMaps")
        -- dump_module("maps.ItemUtils")
        -- dump_module("maps.ItemGroups")
        -- dump_module("engine.StartArena")
        -- dump_module("engine.ResourceLoading")
        -- dump_module("engine.StartMonsterEvals")
        -- dump_module("engine.Arena")
        -- dump_module("engine.StartLuaRepl")
        -- dump_module("engine.StartBotTests")
        -- dump_module("engine.StartEngine")
        -- dump_module("engine.StartLanarts")
        -- dump_module("engine.Settings")
        -- dump_module("engine.StartVisualize")
        -- dump_module("engine.InventoryManagement")
        -- dump_module("engine.StartScenario1")
        -- dump_module("menus.GameScenarioMenu")
        -- dump_module("menus.AdventureMode")
        -- dump_module("ai.BotInputSource")
        -- dump_module("OrderedDict")
        -- dump_module("DataWrapped")
        -- dump_module("ui.ConsoleUtils")
        -- dump_module("maps.TemplateGeneration")
        -- dump_module("maps.MapCompilerContext")
        -- dump_module("maps.NewDungeons")
        -- dump_module("maps.GeometryUtils")
        -- dump_module("maps.Vaults")
        -- dump_module("maps.Region1")
        -- dump_module("maps.MapCompiler")
        -- dump_module("maps.DoorGeneration")
        -- dump_module("maps.MapElements")
        -- dump_module("maps.DebugUtils")
        -- dump_module("maps.PolygonShapes")
        -- dump_module("maps.MapRegionShapes")
        -- dump_module("maps.NewMaps")
        -- dump_module("maps.MapLink")
        -- dump_module("maps.B2World")
        -- dump_module("maps.MapRegion")
        -- dump_module("maps.LinearMode")
        -- dump_module("maps.MapDesc")
        -- dump_module("maps.Places")
        -- dump_module("maps.B2Utils")
        -- dump_module("maps.DoorGeneration")
        -- dump_module("maps.MapNodeFills")
        -- dump_module("maps.ItemGroups")
        -- dump_module("maps.MapElements")
        -- dump_module("maps.VaultUtils")
        -- dump_module("maps.GenerateUtils")
        -- dump_module("maps.MapRegionUtils")
        -- dump_module("maps.B2GenerateUtils")
        -- dump_module("maps.B2WorldUtils")
        -- dump_module("interactive.b2proper")
        -- dump_module("interactive.b2onestep")
        -- dump_module("interactive.b2test")
        -- dump_module("maps.TemplateGeneration")
        -- os.exit()
        if args.load
            return pregame_start(args.load)
        else
            return Engine.menu_start(pregame_start)

    pregame_start = (load_file=nil) ->
        engine_init(args)
        if args.class
            settings.class_type = args.class
            return game_start()
        elseif load_file or settings.connection_type ~= 2
            return game_start(load_file)
        else
            return Engine.pregame_menu_start () -> game_start(load_file)

    game_start = (load_file=nil) ->
        return ResourceLoading.ensure_resources_before () ->
            -- (1) Init game
            -- TODO refactor:
            if load_file and args.load
                load_file = args.load
            game_init(args, load_file)

            -- (2) Run macros
            if args.macro
                require("debug_scripts." .. args.macro)

            -- (3) Set up input for first game step
            GameState = require("core.GameState")
            GameState.input_capture()
            if not GameState.input_handle()
                error("Game exited")

            -- (4) Transfer to game loop
            return game_step()

    game_step = () ->
        -- (1) Load dependencies
        Keys = require "core.Keyboard"
        GameLoop = require "GameLoop"
        GameState = require "core.GameState"
        Network = require "core.Network"

        -- (2) Consume network events (TODO this should link to player input sources)
        perf.timing_begin("**Sync Message**")
        Network.sync_message_consume()
        perf.timing_end("**Sync Message**")

        -- (3) Run game loop & and see if game
        -- should be saved + exited
        if not GameLoop.game_step()
            -- Save and exit
            GameState.score_board_store()
            GameState.save(args.save or "saves/savefile.save")
            -- Allocate a fresh GameState
            EngineInternal.init_gamestate_api(initial_settings)
            -- Go back to menu
            return entry_point()
        -- (4) (DEBUG) If F3 is pressed, pause the game
        elseif false and Keys.key_pressed(Keys.F3)
            -- Save game
            GameState.save("saves/tempsave.save")
            -- Allocate a fresh GameState
            -- Clear all loaded 'maps.*' modules
            for k,v in pairs package.loaded
                if k\startswith("maps.") or k\startswith("items.") or k\startswith("spells.") or k\startswith("enemies.") or k\startswith("classes.")
                    package.loaded[k] = nil
            EngineInternal.init_resource_data_sets(true) -- avoid sprite loads
            Engine.resources_load()
            -- Load game
            GameState.load("saves/tempsave.save")
        -- (5) If F4 is pressed, pause the game
        elseif Keys.key_pressed(Keys.F4)
            GameLoop.loop_control.game_is_paused = not GameLoop.loop_control.game_is_paused
        -- (6) If Escape is pressed, tell user about shift+escape
        elseif Keys.key_pressed(Keys.ESCAPE)
            for _ in screens()
                if __EMSCRIPTEN
                    EventLog.add("Press Ctrl + Esc to exit, your progress will be saved.")
                else
                    EventLog.add("Press Shift + Esc to exit, your progress will be saved.")
        -- (7) Backup save every ~5 minutes
        elseif GameState.frame > 0 and GameState.frame % (60 * 60 * 5) == 0  -- Every 5 minutes at 60FPS
            for _ in screens()
                EventLog.add("BACKING UP SAVE!", COL_WHITE)
            if __EMSCRIPTEN
                GameState.save("saves/savefile.save")
            else
                GameState.save((args.save or "saves/savefile.save") .. ".backup." .. GameState.frame)

        -- (8) Loop again
        return game_step

    return StartEngine.start_engine {
        settings: initial_settings,
        :entry_point,
        debug: args.debug,
        nofilter: args.nofilter
        on_exit: Settings.settings_save
    }

return run_lanarts
