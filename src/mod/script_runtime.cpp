#include "stoneforge/mod/script_runtime.hpp"

#ifdef STONEFORGE_HAS_LUA
extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}
#endif

namespace stoneforge::mod {

ScriptRuntime::ScriptRuntime() = default;

ScriptRuntime::~ScriptRuntime() {
#ifdef STONEFORGE_HAS_LUA
    if(state_ != nullptr) {
        lua_close(static_cast<lua_State*>(state_));
        state_ = nullptr;
    }
#endif
}

bool ScriptRuntime::initialize() {
#ifdef STONEFORGE_HAS_LUA
    if(state_ != nullptr) {
        return true;
    }

    lua_State* lua = luaL_newstate();
    if(lua == nullptr) {
        lastError_ = "failed to initialize Lua runtime";
        return false;
    }

    luaL_requiref(lua, "_G", luaopen_base, 1);
    lua_pop(lua, 1);
    luaL_requiref(lua, LUA_TABLIBNAME, luaopen_table, 1);
    lua_pop(lua, 1);
    luaL_requiref(lua, LUA_STRLIBNAME, luaopen_string, 1);
    lua_pop(lua, 1);
    luaL_requiref(lua, LUA_MATHLIBNAME, luaopen_math, 1);
    lua_pop(lua, 1);

    state_ = lua;
    enabled_ = true;
    return true;
#else
    enabled_ = false;
    lastError_ = "Lua support is not available in this build";
    return false;
#endif
}

void ScriptRuntime::loadScripts(const std::vector<LoadedModInfo>& mods) {
#ifdef STONEFORGE_HAS_LUA
    if(!enabled_ || state_ == nullptr) {
        return;
    }

    auto* lua = static_cast<lua_State*>(state_);
    for(const auto& mod : mods) {
        for(const auto& scriptRel : mod.scripts) {
            const auto scriptPath = (mod.rootPath / scriptRel).string();
            if(luaL_dofile(lua, scriptPath.c_str()) != LUA_OK) {
                const char* msg = lua_tostring(lua, -1);
                lastError_ = msg != nullptr ? msg : "unknown lua script load error";
                lua_pop(lua, 1);
            }
        }
    }
#else
    (void)mods;
#endif
}

void ScriptRuntime::emitEvent(const std::string& eventName, const std::unordered_map<std::string, std::string>& payload) {
#ifdef STONEFORGE_HAS_LUA
    if(!enabled_ || state_ == nullptr) {
        return;
    }

    auto* lua = static_cast<lua_State*>(state_);
    lua_getglobal(lua, eventName.c_str());
    if(!lua_isfunction(lua, -1)) {
        lua_pop(lua, 1);
        return;
    }

    lua_newtable(lua);
    for(const auto& [key, value] : payload) {
        lua_pushlstring(lua, key.c_str(), key.size());
        lua_pushlstring(lua, value.c_str(), value.size());
        lua_settable(lua, -3);
    }

    if(lua_pcall(lua, 1, 0, 0) != LUA_OK) {
        const char* msg = lua_tostring(lua, -1);
        lastError_ = msg != nullptr ? msg : "unknown lua runtime error";
        lua_pop(lua, 1);
    }
#else
    (void)eventName;
    (void)payload;
#endif
}

bool ScriptRuntime::enabled() const {
    return enabled_;
}

const std::string& ScriptRuntime::lastError() const {
    return lastError_;
}

}  // namespace stoneforge::mod
