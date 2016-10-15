/**
 *
 * MACHINE GENERATED FILE. DO NOT EDIT.
 *
 * Bindings for class b2ClipVertex
 *
 * This file has been generated by dub 2.1.~.
 */
#include "dub/dub.h"
#include "Box2D/Collision/b2Collision.h"


/** Set attributes (key, value)
 * 
 */
static int b2ClipVertex__set_(lua_State *L) {

  b2ClipVertex *self = *((b2ClipVertex **)dub_checksdata_n(L, 1, "b2.ClipVertex"));
  const char *key = luaL_checkstring(L, 2);
  int key_h = dub_hash(key, 2);
  switch(key_h) {
    case 0: {
      if (DUB_ASSERT_KEY(key, "v")) break;
      self->v = **((b2Vec2 **)dub_checksdata_n(L, 3, "b2.Vec2"));
      return 0;
    }
    case 1: {
      if (DUB_ASSERT_KEY(key, "id")) break;
      self->id = **((b2ContactID **)dub_checksdata_n(L, 3, "b2.ContactID"));
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
static int b2ClipVertex__get_(lua_State *L) {

  b2ClipVertex *self = *((b2ClipVertex **)dub_checksdata_n(L, 1, "b2.ClipVertex", true));
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
      if (DUB_ASSERT_KEY(key, "v")) break;
      dub_pushudata(L, &self->v, "b2.Vec2", false);
      return 1;
    }
    case 1: {
      if (DUB_ASSERT_KEY(key, "id")) break;
      dub_pushudata(L, &self->id, "b2.ContactID", false);
      return 1;
    }
  }
  return 0;
}

/** b2ClipVertex()
 * 
 */
static int b2ClipVertex_b2ClipVertex(lua_State *L) {
  try {
    b2ClipVertex *retval__ = new b2ClipVertex();
    dub_pushudata(L, retval__, "b2.ClipVertex", true);
    return 1;
  } catch (std::exception &e) {
    lua_pushfstring(L, "b2ClipVertex: %s", e.what());
  } catch (...) {
    lua_pushfstring(L, "b2ClipVertex: Unknown exception");
  }
  return dub_error(L);
}



// --=============================================== __tostring
static int b2ClipVertex___tostring(lua_State *L) {
  b2ClipVertex *self = *((b2ClipVertex **)dub_checksdata_n(L, 1, "b2.ClipVertex"));
  lua_pushfstring(L, "b2.ClipVertex: %p", self);
  
  return 1;
}

// --=============================================== METHODS

static const struct luaL_Reg b2ClipVertex_member_methods[] = {
  { "__newindex"   , b2ClipVertex__set_   },
  { "__index"      , b2ClipVertex__get_   },
  { "new"          , b2ClipVertex_b2ClipVertex },
  { "__tostring"   , b2ClipVertex___tostring },
  { "deleted"      , dub_isDeleted        },
  { NULL, NULL},
};


extern "C" int luaopen_b2_ClipVertex(lua_State *L)
{
  // Create the metatable which will contain all the member methods
  luaL_newmetatable(L, "b2.ClipVertex");
  // <mt>

  // register member methods
  luaL_register(L, NULL, b2ClipVertex_member_methods);
  // save meta-table in b2
  dub_register(L, "b2", "ClipVertex", "ClipVertex");
  // <mt>
  lua_pop(L, 1);
  return 0;
}
