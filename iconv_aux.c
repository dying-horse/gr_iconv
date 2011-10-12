#define GR_ICONV_OUTBUFSIZE 32

#include <errno.h>
#include <iconv.h>
#include <lauxlib.h>
#include <lua.h>
#include <stdlib.h>

typedef struct {
 iconv_t iconv;
 char   *outbuf;
 char    outbuf_base[GR_ICONV_OUTBUFSIZE];;
 size_t  inbytesleft;
 size_t  outbytesleft;
} gr_iconv_t;

static int
gr_iconv_open(lua_State *L) {
 gr_iconv_t *iconvstate = lua_newuserdata(L, sizeof(gr_iconv_t));
 luaL_newmetatable(L, "gr_iconv_iconv");
 lua_setmetatable(L, -2);

 iconvstate -> outbuf       = iconvstate -> outbuf_base;
 iconvstate -> outbytesleft = GR_ICONV_OUTBUFSIZE;

 iconvstate -> iconv =
  iconv_open(luaL_checkstring(L, -2), luaL_checkstring(L, -3));
 if   (iconvstate -> iconv == (iconv_t) -1) {
  lua_pop(L, 3);
  switch (errno) {
   case (EMFILE):
    lua_getfield(L, -3, "except_too_many_fd_per_process");
    break;
   case (ENFILE):
    lua_getfield(L, -3, "except_too_many_fd");
    break;
   case (ENOMEM):
    lua_getfield(L, -3, "except_out_of_memory");
    break;
   case (EINVAL): 
    lua_getfield(L, -3, "except_not_supported");
    break;
   default:
    luaL_error(L, "iconv_open: unknown error");
  }
  lua_insert(L, -2);
  lua_call(L, 1, 0);

  lua_pushstring(L, "err");

  return 1;
 }

 lua_remove(L, -2);
 lua_remove(L, -2);

 int ret = iconv(iconvstate -> iconv, NULL, NULL,
  &(iconvstate -> outbuf), &(iconvstate -> outbytesleft));
 lua_setfield(L, -2, "iconvstate");

 lua_pop(L, 1);

 if (ret == (size_t) -1)
  switch (errno) {
   case(E2BIG):
    lua_pushstring(L, "pull");
    break;
   default:
    luaL_error(L, "iconv: unknown error");
    break;
  }
 else
  lua_pushstring(L, "ok");

 return 1;
}

static int
gr_iconv_push(lua_State *L) {
 lua_getfield(L, -2, "iconvstate");
 gr_iconv_t *iconvstate = luaL_checkudata(L, -1, "gr_iconv_iconv");
 lua_getfield(L, -3, "inbuf");

 size_t len_in;
 luaL_Buffer B;
 luaL_buffinit(L, &B);
 luaL_addvalue(&B);
 const char *in = luaL_checklstring(L, -2, &len_in);
 luaL_addlstring(&B, in, len_in);
 luaL_pushresult(&B);

 iconvstate -> inbytesleft = lua_objlen(L, -1);
 lua_setfield(L, -4, "inbuf");
 lua_pop(L, 3);

 return 0;
}

static int
gr_iconv_pull(lua_State *L) {
 lua_getfield(L, -1, "iconvstate");
 gr_iconv_t *iconvstate = luaL_checkudata(L, -1, "gr_iconv_iconv");
 lua_pushlstring(L, iconvstate -> outbuf_base,
  GR_ICONV_OUTBUFSIZE - (iconvstate -> outbytesleft));

 iconvstate -> outbuf = iconvstate -> outbuf_base;
 iconvstate -> outbytesleft = GR_ICONV_OUTBUFSIZE;

 lua_remove(L, -2);
 lua_remove(L, -2);

 return 1;
}

static int
gr_iconv_go(lua_State *L) {
 lua_getfield(L, -1, "iconvstate");
 gr_iconv_t *iconvstate = luaL_checkudata(L, -1, "gr_iconv_iconv");
 lua_getfield(L, -2, "inbuf");
 int inbufsize;
 char *inbuf = (char *) luaL_checklstring(L, -1, &inbufsize);
 inbuf = inbuf + inbufsize - (iconvstate -> inbytesleft);

 size_t ret = iconv(iconvstate -> iconv,
  &inbuf, &(iconvstate -> inbytesleft),
  &(iconvstate -> outbuf), &(iconvstate -> outbytesleft));
 if   (ret == (size_t) -1)
  switch (errno) {
   case(E2BIG):
    lua_pop(L, 3);
    lua_pushstring(L, "pull");
    return 1;
   case(EBADF):
    lua_pop(L, 3);
    luaL_error(L, "iconv: invalid iconv_t");
   case(EILSEQ):
    lua_pop(L, 2);
    lua_getfield(L, -1, "except_invalid_mb");
    lua_insert(L, -2);
    lua_call(L, 1, 0);
    lua_pushstring(L, "err");
    return 1;
   case(EINVAL):
    lua_pushlstring(L, inbuf, iconvstate -> inbytesleft);
    lua_setfield(L, -4, "inbuf");
    lua_pop(L, 3);
    lua_pushstring(L, "push");
    return 1;
  }
 else
  lua_pushstring(L, "");
  lua_setfield(L, -4, "inbuf");
  lua_pop(L, 3);
  lua_pushstring(L, "ok");

 return 1;
}

static int
gr_iconv_except_invalid_mb(lua_State *L) {
 luaL_error(L, "iconv: mb could not be interpreted");
}

static int
gr_iconv_except_too_many_fd_per_process(lua_State *L) {
 luaL_error(L, "iconv: too many open fd per process");
}

static int
gr_iconv_except_too_many_fd(lua_State *L) {
 luaL_error(L, "iconv: too many open fd");
}

static int
gr_iconv_except_out_of_memory(lua_State *L) {
 luaL_error(L, "iconv: out of memory");
}

static int
gr_iconv_except_not_supported(lua_State *L) {
 luaL_error(L, "iconv: transformation not supported");
}

static int
gr_iconv_gc(lua_State *L) {
 lua_getfield(L, -1, "iconvstate");
 gr_iconv_t *iconvstate = luaL_checkudata(L, -1, "gr_iconv_iconv");

 if   (iconv_close(iconvstate -> iconv));
  switch (errno) {
   case (EBADF):
    luaL_error(L, "iconv_close: invalid iconv_t");
   default:
    luaL_error(L, "iconv_close: unknown error");
  }

 lua_pop(L, 2);

 return 0;
}

static int
gr_iconv_new(lua_State *L) {
 lua_createtable(L, 0, 1);

 lua_pushstring(L, "");
 lua_setfield(L, -2, "inbuf");

 lua_createtable(L, 0, 2);

 lua_pushcfunction(L, &gr_iconv_gc);
 lua_setfield(L, -2, "__gc");

 lua_getglobal(L, "iconv");
 lua_setfield(L, -2, "__index");

 lua_setmetatable(L, -2);

 return 1;
}


/* General. */

static luaL_Reg
gr_iconv_funcs[] = {
 { "except_invalid_mb",
   &gr_iconv_except_invalid_mb },
 { "except_not_supported",
   &gr_iconv_except_not_supported },
 { "except_out_of_memory",
   &gr_iconv_except_out_of_memory },
 { "except_too_many_fd",
   &gr_iconv_except_too_many_fd },
 { "except_too_many_fd_per_process",
   &gr_iconv_except_too_many_fd_per_process },
 { "go",   &gr_iconv_go   },
 { "new",  &gr_iconv_new  },
 { "open", &gr_iconv_open },
 { "pull", &gr_iconv_pull },
 { "push", &gr_iconv_push },
 { NULL, NULL }
};

int
luaopen_iconv_aux(lua_State *L) {
 luaL_register(L, "iconv", gr_iconv_funcs);

 return 0;
}
