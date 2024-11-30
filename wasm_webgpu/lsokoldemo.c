#include "emscripten.h"

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include "hellotriangle.h"

static int demo(lua_State *L)
{
    // String
    char demo[32] = "";

    // Call the function
    int res = run_hello_triangle();

    // Put number in the string
    sprintf(demo, "%d", res);

    // lua_pushnumber(L, 0);
    lua_pushstring(L, demo);

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