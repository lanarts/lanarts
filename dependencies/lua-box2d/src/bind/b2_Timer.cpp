/**
 *
 * MACHINE GENERATED FILE. DO NOT EDIT.
 *
 * Bindings for class b2Timer
 *
 * This file has been generated by dub 2.1.~.
 */
#include "dub/dub.h"
#include "Box2D/Common/b2Timer.h"


/** ~b2Timer()
 * 
 */
static int b2Timer__b2Timer(lua_State *L) {
  try {
    DubUserdata *userdata = ((DubUserdata*)dub_checksdata_d(L, 1, "b2.Timer"));
    if (userdata->gc) {
      b2Timer *self = (b2Timer *)userdata->ptr;
      delete self;
    }
    userdata->gc = false;
    return 0;
  } catch (std::exception &e) {
    lua_pushfstring(L, "~b2Timer: %s", e.what());
  } catch (...) {
    lua_pushfstring(L, "~b2Timer: Unknown exception");
  }
  return dub_error(L);
}

/** b2Timer::b2Timer()
 * vendor/Box2D/Box2D/Common/b2Timer.h:31
 */
static int b2Timer_b2Timer(lua_State *L) {
  try {
    b2Timer *retval__ = new b2Timer();
    dub_pushudata(L, retval__, "b2.Timer", true);
    return 1;
  } catch (std::exception &e) {
    lua_pushfstring(L, "b2Timer: %s", e.what());
  } catch (...) {
    lua_pushfstring(L, "b2Timer: Unknown exception");
  }
  return dub_error(L);
}

/** void b2Timer::Reset()
 * vendor/Box2D/Box2D/Common/b2Timer.h:34
 */
static int b2Timer_Reset(lua_State *L) {
  try {
    b2Timer *self = *((b2Timer **)dub_checksdata(L, 1, "b2.Timer"));
    self->Reset();
    return 0;
  } catch (std::exception &e) {
    lua_pushfstring(L, "Reset: %s", e.what());
  } catch (...) {
    lua_pushfstring(L, "Reset: Unknown exception");
  }
  return dub_error(L);
}

/** float32 b2Timer::GetMilliseconds() const
 * vendor/Box2D/Box2D/Common/b2Timer.h:37
 */
static int b2Timer_GetMilliseconds(lua_State *L) {
  try {
    b2Timer *self = *((b2Timer **)dub_checksdata(L, 1, "b2.Timer"));
    lua_pushnumber(L, self->GetMilliseconds());
    return 1;
  } catch (std::exception &e) {
    lua_pushfstring(L, "GetMilliseconds: %s", e.what());
  } catch (...) {
    lua_pushfstring(L, "GetMilliseconds: Unknown exception");
  }
  return dub_error(L);
}



// --=============================================== __tostring
static int b2Timer___tostring(lua_State *L) {
  b2Timer *self = *((b2Timer **)dub_checksdata_n(L, 1, "b2.Timer"));
  lua_pushfstring(L, "b2.Timer: %p", self);
  
  return 1;
}

// --=============================================== METHODS

static const struct luaL_Reg b2Timer_member_methods[] = {
  { "__gc"         , b2Timer__b2Timer     },
  { "new"          , b2Timer_b2Timer      },
  { "Reset"        , b2Timer_Reset        },
  { "GetMilliseconds", b2Timer_GetMilliseconds },
  { "__tostring"   , b2Timer___tostring   },
  { "deleted"      , dub_isDeleted        },
  { NULL, NULL},
};


extern "C" int luaopen_b2_Timer(lua_State *L)
{
  // Create the metatable which will contain all the member methods
  luaL_newmetatable(L, "b2.Timer");
  // <mt>

  // register member methods
  luaL_register(L, NULL, b2Timer_member_methods);
  // save meta-table in b2
  dub_register(L, "b2", "Timer", "Timer");
  // <mt>
  lua_pop(L, 1);
  return 0;
}