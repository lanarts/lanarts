/*
 * luawrap.h:
 *  Flexible wrapping library.
 *  Revolves around three operations for each type:
 *   - push: push on lua stack
 *   - get: get from lua index
 *   - check: check that index is desired object
 *
 *  Basic idea -> const T& and T are value types and operate on callbacks bound to T
 *  T* and T& are reference types and operate on callbacks bound to T*
 *
 */

#ifndef LUAWRAP_LUAWRAP_H_
#define LUAWRAP_LUAWRAP_H_

#include <cstdio>

#include <lua.hpp>

#include <luawrap/config.h>

#include <luawrap/LuaValue.h>
#include <luawrap/LuaStackValue.h>

// Include internal header
#include "../../src/pushget_helper.h"

namespace luawrap {
	namespace _private {
		template<class C>
		struct TypeImpl;

		//Use RAII to pop the lua stack after return:
		struct PopHack {
			lua_State* L;
			PopHack(lua_State* L) :
					L(L) {
			}
			~PopHack() {
				lua_pop(L, 1);
			}
		};
	}

}

namespace luawrap {

	template<typename T>
	inline void push(lua_State* L, const T& val) {
		_private::PushGetCheckWrap<T>::push(L, val);
	}

	template<typename T>
	inline typename _private::PushGetCheckWrap<T>::RetType get(lua_State* L,
			int idx) {
		return _private::PushGetCheckWrap<T>::get(L, idx);
	}

	template<typename T>
	inline bool check(lua_State* L, int idx) {
		return _private::PushGetCheckWrap<T>::check(L, idx);
	}

	// Utility operators:
	template<typename T>
	inline void get(lua_State* L, int idx, T& val) {
		val = get<T>(L, idx);
	}

	template<typename T>
	inline T pop(lua_State* L) {
		_private::PopHack delayedpop(L);
		return _private::TypeImpl<T>::get(L, -1);
	}

	template<typename T>
	inline LuaValue value(lua_State* L, const T& val) {
		push<T>(L, val);
		LuaValue lval;
		lval.pop(L);
		return lval;
	}

	template<typename T>
	inline void setfield(lua_State* L, int idx, const char* key, const T& val) {
		push<T>(L, val);
		lua_setfield(L, idx, key);
	}

	template<typename T>
	inline T getfield(lua_State* L, int idx, const char* key) {
		lua_getfield(L, idx, key);
		return pop<T>(L);
	}

	template<typename T>
	inline void getfield(lua_State* L, int idx, const char* key, T& val) {
		val = getfield<T>(L, idx, key);
	}

	// Get field if value is not nil
	template<typename T>
	inline void getoptfield(lua_State* L, int idx, const char* key, T& val) {
		lua_getfield(L, idx, key);
		if (!lua_isnil(L, -1)) {
			val = pop<T>(L);
		} else {
			lua_pop(L, 1);
		}
	}
}

// LuaValue related
namespace _luawrap_private {

	template<typename T>
	inline _LuaField::operator T() {
		luawrap::_private::PopHack delayedpop(L);
		luafield_push(L, value, key);
		return luawrap::get<T>(L, -1);
	}

	template<typename T>
	inline void _LuaField::operator=(const T& t) {
		luawrap::push<T>(L, t);
		luafield_pop(L, value, key);
	}

	template<typename T>
	inline _LuaStackField::operator T() {
		lua_State* L = value.luastate();
		luawrap::_private::PopHack delayedpop(L);
		lua_getfield(L, value.index(), key);
		return luawrap::get<T>(L, -1);
	}

	template<typename T>
	inline void _LuaStackField::operator=(const T& t) {
		lua_State* L = value.luastate();
		luawrap::push<T>(L, t);
		lua_setfield(L, value.index(), key);
	}
}

#include "../../src/predefined_helper.h"

#endif /* LUAWRAP_LUAWRAP_H_ */