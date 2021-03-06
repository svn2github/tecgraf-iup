/** \file
 * \brief pplot binding for Lua 5.
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdlib.h>

#include <lua.h>
#include <lauxlib.h>

#include "iup.h"
#include "iup_pplot.h"

#include <cd.h>
#include <cdlua.h>

#include "iuplua.h"
#include "iuplua_pplot.h"
#include "il.h"


static int pplot_edit_cb(Ihandle *self, int p0, int p1, float p2, float p3, float *p4, float *p5)
{
  int ret;
  lua_State *L = iuplua_call_start(self, "edit_cb");
  lua_pushinteger(L, p0);
  lua_pushinteger(L, p1);
  lua_pushnumber(L, p2);
  lua_pushnumber(L, p3);
  ret = iuplua_call_raw(L, 4+2, LUA_MULTRET);  /* 4 args + 2 args(errormsg, handle), variable number of returns */
  if (ret || lua_isnil(L, -1))
    return IUP_DEFAULT;
  ret = lua_tointeger(L,-1);

  if (ret == IUP_IGNORE) 
  {
    lua_pop(L, 1);
    return IUP_IGNORE;
  }

  *p4 = (float)lua_tonumber(L, -3); 
  *p5 = (float)lua_tonumber(L, -2); 
  lua_pop(L, 1);
  return IUP_DEFAULT;
}

static int pplot_postdraw_cb(Ihandle *self, cdCanvas* cnv)
{
  lua_State *L = iuplua_call_start(self, "postdraw_cb");
  cdlua_pushcanvas(L, cnv);
  return iuplua_call(L, 1);
}

static int pplot_predraw_cb(Ihandle *self, cdCanvas* cnv)
{
  lua_State *L = iuplua_call_start(self, "predraw_cb");
  cdlua_pushcanvas(L, cnv);
  return iuplua_call(L, 1);
}

static int PPlotBegin(lua_State *L)
{
  Ihandle *ih = iuplua_checkihandle(L,1);
  IupPPlotBegin(ih, luaL_checkint(L,2));
  return 0;
}

static int PPlotAdd(lua_State *L)
{
  Ihandle *ih = iuplua_checkihandle(L,1);
  IupPPlotAdd(ih, (float)luaL_checknumber(L,2), (float)luaL_checknumber(L,3));
  return 0;
}

static int PPlotAddStr(lua_State *L)
{
  Ihandle *ih = iuplua_checkihandle(L,1);
  IupPPlotAddStr(ih, luaL_checkstring(L,2), (float)luaL_checknumber(L,3));
  return 0;
}

static int PPlotEnd(lua_State *L)
{
  Ihandle *ih = iuplua_checkihandle(L,1);
  int ret = IupPPlotEnd(ih);
  lua_pushinteger(L, ret);
  return 1;
}

static int PPlotInsertStr(lua_State *L)
{
  Ihandle *ih = iuplua_checkihandle(L,1);
  IupPPlotInsertStr(ih, luaL_checkint(L,2), luaL_checkint(L,3), luaL_checkstring(L,4), (float)luaL_checknumber(L,5));
  return 0;
}

static int PPlotInsert(lua_State *L)
{
  Ihandle *ih = iuplua_checkihandle(L,1);
  IupPPlotInsert(ih, luaL_checkint(L,2), luaL_checkint(L,3), (float)luaL_checknumber(L,4), (float)luaL_checknumber(L,5));
  return 0;
}

static int PPlotInsertPoints(lua_State *L)
{
  float *px, *py;
  int count = luaL_checkinteger(L, 6);
  px = iuplua_checkfloat_array(L, 4, count);
  py = iuplua_checkfloat_array(L, 5, count);
  IupPPlotInsertPoints(iuplua_checkihandle(L,1), luaL_checkint(L,2), luaL_checkint(L,3), px, py, count);
  free(px);
  free(py);
  return 0;
}

static int PPlotInsertStrPoints(lua_State *L)
{
  float *py;
  char* *px;
  int count = luaL_checkinteger(L, 6);
  px = iuplua_checkstring_array(L, 4, count);
  py = iuplua_checkfloat_array(L, 5, count);
  IupPPlotInsertStrPoints(iuplua_checkihandle(L,1), luaL_checkint(L,2), luaL_checkint(L,3), px, py, count);
  free(px);
  free(py);
  return 0;
}

static int PPlotAddPoints(lua_State *L)
{
  float *px, *py;
  int count = luaL_checkinteger(L, 5);
  px = iuplua_checkfloat_array(L, 3, count);
  py = iuplua_checkfloat_array(L, 4, count);
  IupPPlotAddPoints(iuplua_checkihandle(L,1), luaL_checkint(L,2), px, py, count);
  free(px);
  free(py);
  return 0;
}

static int PPlotAddStrPoints(lua_State *L)
{
  float *py;
  char* *px;
  int count = luaL_checkinteger(L, 5);
  px = iuplua_checkstring_array(L, 3, count);
  py = iuplua_checkfloat_array(L, 4, count);
  IupPPlotAddStrPoints(iuplua_checkihandle(L,1), luaL_checkint(L,2), px, py, count);
  free(px);
  free(py);
  return 0;
}

static int PPlotTransform(lua_State *L)
{
  Ihandle *ih = iuplua_checkihandle(L,1);
  int ix, iy;
  IupPPlotTransform(ih, (float)luaL_checknumber(L,2), (float)luaL_checknumber(L,3), &ix, &iy);
  lua_pushinteger(L, ix);
  lua_pushinteger(L, iy);
  return 2;
}

static int PPlotPaintTo(lua_State *L)
{
  Ihandle *ih = iuplua_checkihandle(L,1);
  IupPPlotPaintTo(ih, cdlua_checkcanvas(L,2));
  return 0;
}

void iuplua_pplotfuncs_open (lua_State *L)
{
  iuplua_register_cb(L, "EDIT_CB", (lua_CFunction)pplot_edit_cb, "pplot");
  iuplua_register_cb(L, "PREDRAW_CB", (lua_CFunction)pplot_predraw_cb, NULL);
  iuplua_register_cb(L, "POSTDRAW_CB", (lua_CFunction)pplot_postdraw_cb, NULL);

  iuplua_register(L, PPlotBegin       ,"PPlotBegin");
  iuplua_register(L, PPlotAdd         ,"PPlotAdd");
  iuplua_register(L, PPlotAddStr      ,"PPlotAddStr");
  iuplua_register(L, PPlotEnd         ,"PPlotEnd");
  iuplua_register(L, PPlotInsertStr   ,"PPlotInsertStr");
  iuplua_register(L, PPlotInsert      ,"PPlotInsert");
  iuplua_register(L, PPlotInsertPoints    ,"PPlotInsertPoints");
  iuplua_register(L, PPlotInsertStrPoints ,"PPlotInsertStrPoints");
  iuplua_register(L, PPlotAddPoints       ,"PPlotAddPoints");
  iuplua_register(L, PPlotAddStrPoints    ,"PPlotAddStrPoints");

  iuplua_register(L, PPlotTransform   ,"PPlotTransform");
  iuplua_register(L, PPlotPaintTo     ,"PPlotPaintTo");
}

int iuppplotlua_open(lua_State * L);

int iup_pplotlua_open(lua_State * L)
{
  if (iuplua_opencall_internal(L))
    IupPPlotOpen();

  iuplua_get_env(L);
  iuppplotlua_open(L);
  return 0;
}

/* obligatory to use require"iuplua_pplot" */
int luaopen_iuplua_pplot(lua_State* L)
{
  return iup_pplotlua_open(L);
}

