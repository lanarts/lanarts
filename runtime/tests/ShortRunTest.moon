GameState = require "core.GameState"
GlobalData = require "core.GlobalData"
Map = require "core.Map"

TEST_SEED = 123456789

return table.merge require("tests.TestBase"), {
    overworld_create: () =>
        random_seed(TEST_SEED)
        O = require("maps.01_Overworld")
        return O.overworld_create()
}