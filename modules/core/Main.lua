local nilprotect_meta = {__index = function(self, k)
    error( ("Key '%s' does not exist in table!"):format(k) )
end}    
-- Set to a metatable that does not allow nil accesses
function nilprotect(t)
    return setmetatable(t, nilprotect_meta)
end

-- Ensure undefined global variable access results in an error
nilprotect(_G)

local globals_subpackage = "core.globals"
local modules_submodule = globals_subpackage .. ".Modules"
-- Note: 'import' is not defined until Modules.lua is ran.
dofile(_ROOT_FOLDER .. "/" .. modules_submodule:gsub("%.", "/") .. ".lua")
import_all(globals_subpackage, "*", true, function(name) 
    return name ~= modules_submodule
end)