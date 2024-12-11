#include "emscripten.h"

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include "helloboat.h"

int len = 94410;

static int demo(lua_State *L)
{
    // Call the function
    unsigned char * png = run_hello_boat();

    // lua_pushnumber(L, 0);
    lua_pushlstring(L, png, len);

    return 1;
}

// Library registration function
static const struct luaL_Reg lsokoldemo_funcs[] = {
    {"demo", demo},
    {NULL, NULL} /* Sentinel */
};

// Initialization function
int luaopen_lsokoldemo(lua_State *L)
{
    luaL_newlib(L, lsokoldemo_funcs);
    return 1;
}