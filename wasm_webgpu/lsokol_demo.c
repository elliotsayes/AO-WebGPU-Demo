#include <emscripten.h>
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include <stdio.h>
#define SOKOL_IMPL
#define SOKOL_WGPU
#include "sokol_gfx.h"

// Run the demo
char *run_demo()
{
    // return dummy output
    return "Hello from Sokol!";
}

static int demo(lua_State *L)
{
    // Call the run_demo function
    char *output = run_demo();

    // lua_pushnumber(L, 0);
    lua_pushstring(L, output);

    return 1;
}


// Library registration function
static const struct luaL_Reg lsokol_demo_funcs[] = {
    {"demo", demo},
    {NULL, NULL} /* Sentinel */
};

// Initialization function
int luaopen_lsokol_demo(lua_State *L)
{
    luaL_newlib(L, lsokol_demo_funcs);
    return 1;
}