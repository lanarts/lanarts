/**
 *
 * MACHINE GENERATED FILE. DO NOT EDIT.
 *
 * Bindings for class b2SimplexCache
 *
 * This file has been generated by dub 2.1.~.
 */
#include "dub/dub.h"
#include "Box2D/Collision/b2Distance.h"


/** Set attributes (key, value)
 * 
 */
static int b2SimplexCache__set_(lua_State *L) {

  b2SimplexCache *self = *((b2SimplexCache **)dub_checksdata_n(L, 1, "b2.SimplexCache"));
  const char *key = luaL_checkstring(L, 2);
  int key_h = dub_hash(key, 2);
  switch(key_h) {
    case 0: {
      if (DUB_ASSERT_KEY(key, "metric")) break;
      self->metric = luaL_checknumber(L, 3);
      return 0;
    }
    case 1: {
      if (DUB_ASSERT_KEY(key, "count")) break;
      self->count = luaL_checkint(L, 3);
      return 0;
    }
  }
  if (lua_istable(L, 1)) {
    lua_rawset(L, 1);
  } else {
    luaL_error(L, KEY_EXCEPTION_MSG, key);
  }
  return 0;
}

/** Get attributes (key)
 * 
 */
static int b2SimplexCache__get_(lua_State *L) {

  b2SimplexCache *self = *((b2SimplexCache **)dub_checksdata_n(L, 1, "b2.SimplexCache", true));
  const char *key = luaL_checkstring(L, 2);
  // <self> "key" <mt>
  // rawget(mt, key)
  lua_pushvalue(L, 2);
  // <self> "key" <mt> "key"
  lua_rawget(L, -2);
  if (!lua_isnil(L, -1)) {
    // Found method.
    return 1;
  } else {
    // Not in mt = attribute access.
    lua_pop(L, 2);
  }
  int key_h = dub_hash(key, 2);
  switch(key_h) {
    case 0: {
      if (DUB_ASSERT_KEY(key, "metric")) break;
      lua_pushnumber(L, self->metric);
      return 1;
    }
    case 1: {
      if (DUB_ASSERT_KEY(key, "count")) break;
      lua_pushnumber(L, self->count);
      return 1;
    }
  }
  return 0;
}

/** b2SimplexCache()
 * 
 */
static int b2SimplexCache_b2SimplexCache(lua_State *L) {
  try {
    b2SimplexCache *retval__ = new b2SimplexCache();
    dub_pushudata(L, retval__, "b2.SimplexCache", true);
    return 1;
  } catch (std::exception &e) {
    lua_pushfstring(L, "b2SimplexCache: %s", e.what());
  } catch (...) {
    lua_pushfstring(L, "b2SimplexCache: Unknown exception");
  }
  return dub_error(L);
}

/** Read indexA(size_t i)
 * 
 */
static int b2SimplexCache_indexA(lua_State *L) {
  try {
    b2SimplexCache *self = *((b2SimplexCache **)dub_checksdata(L, 1, "b2.SimplexCache"));
    int top__ = lua_gettop(L);
    if (top__ >= 3) {
      size_t i = dub_checkint(L, 2);
      uint8 v = dub_checkint(L, 3);
      if (!i || i > 3) return 0;
      self->indexA[i-1] = v;
      return 0;
    } else {
      size_t i = dub_checkint(L, 2);
      if (!i || i > 3) return 0;
      lua_pushnumber(L, self->indexA[i-1]);
      return 1;
    }
  } catch (std::exception &e) {
    lua_pushfstring(L, "indexA: %s", e.what());
  } catch (...) {
    lua_pushfstring(L, "indexA: Unknown exception");
  }
  return dub_error(L);
}

/** Read indexB(size_t i)
 * 
 */
static int b2SimplexCache_indexB(lua_State *L) {
  try {
    b2SimplexCache *self = *((b2SimplexCache **)dub_checksdata(L, 1, "b2.SimplexCache"));
    int top__ = lua_gettop(L);
    if (top__ >= 3) {
      size_t i = dub_checkint(L, 2);
      uint8 v = dub_checkint(L, 3);
      if (!i || i > 3) return 0;
      self->indexB[i-1] = v;
      return 0;
    } else {
      size_t i = dub_checkint(L, 2);
      if (!i || i > 3) return 0;
      lua_pushnumber(L, self->indexB[i-1]);
      return 1;
    }
  } catch (std::exception &e) {
    lua_pushfstring(L, "indexB: %s", e.what());
  } catch (...) {
    lua_pushfstring(L, "indexB: Unknown exception");
  }
  return dub_error(L);
}



// --=============================================== __tostring
static int b2SimplexCache___tostring(lua_State *L) {
  b2SimplexCache *self = *((b2SimplexCache **)dub_checksdata_n(L, 1, "b2.SimplexCache"));
  lua_pushfstring(L, "b2.SimplexCache: %p", self);
  
  return 1;
}

// --=============================================== METHODS

static const struct luaL_Reg b2SimplexCache_member_methods[] = {
  { "__newindex"   , b2SimplexCache__set_ },
  { "__index"      , b2SimplexCache__get_ },
  { "new"          , b2SimplexCache_b2SimplexCache },
  { "indexA"       , b2SimplexCache_indexA },
  { "indexB"       , b2SimplexCache_indexB },
  { "__tostring"   , b2SimplexCache___tostring },
  { "deleted"      , dub_isDeleted        },
  { NULL, NULL},
};


extern "C" int luaopen_b2_SimplexCache(lua_State *L)
{
  // Create the metatable which will contain all the member methods
  luaL_newmetatable(L, "b2.SimplexCache");
  // <mt>

  // register member methods
  luaL_register(L, NULL, b2SimplexCache_member_methods);
  // save meta-table in b2
  dub_register(L, "b2", "SimplexCache", "SimplexCache");
  // <mt>
  lua_pop(L, 1);
  return 0;
}
