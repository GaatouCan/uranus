#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <iostream>

int main() {
    lua_State *L = luaL_newstate();
    if (!L) {
        std::cerr << "Failed to create Lua state" << std::endl;
        return 1;
    }
    luaL_openlibs(L);

    if (luaL_dofile(L, "../uranus-src/lua/main.lua") != LUA_OK) {
        const char *error = lua_tostring(L, -1);
        std::cerr << "Error: " << error << std::endl;
        lua_pop(L, 1);
    }

    lua_close(L);
    return 0;
}