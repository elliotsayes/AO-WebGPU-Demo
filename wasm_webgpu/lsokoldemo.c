#include "emscripten.h"

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#define BOAT

#ifdef BOAT
#include "helloboat.h"
// int run_hello_boat(unsigned char *png);
#else
#include "hellotriangle.h"
#endif // BOAT

static int demo(lua_State *L)
{
#ifdef BOAT
    int len;
    unsigned char *png = run_hello_boat(&len);
#else
    int len = 331412;
    unsigned char *png = run_hello_triangle();
#endif // BOAT

    // lua_pushnumber(L, 0);
    lua_pushlstring(L, (const char *)png, len);

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