/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @license: MIT license
 * @overview: 
 */

#include "init.h"
#include "LuaIO.h"

static int LuaIO_system_cpuinfo(lua_State *L) {
  uv_cpu_info_t *cpu_infos;
  int count;

  int err = uv_cpu_info(&cpu_infos, &count);
  if (err < 0) return luaL_error(L, "system.cpuinfo() error: %s", uv_strerror(err));

  lua_createtable(L, count, 0);
  for (int i = 0; i < count; i++) {
    uv_cpu_info_t* ci = cpu_infos + i;

    lua_createtable(L, 0, 3);
    lua_pushstring(L, ci->model);
    lua_setfield(L, -2, "model");
    lua_pushinteger(L, ci->speed);
    lua_setfield(L, -2, "speed");

    lua_createtable(L, 0, 5);
    lua_pushinteger(L, ci->cpu_times.user);
    lua_setfield(L, -2, "user");
    lua_pushinteger(L, ci->cpu_times.nice);
    lua_setfield(L, -2, "nice");
    lua_pushinteger(L, ci->cpu_times.sys);
    lua_setfield(L, -2, "sys");
    lua_pushinteger(L, ci->cpu_times.idle);
    lua_setfield(L, -2, "idle");
    lua_pushinteger(L, ci->cpu_times.irq);
    lua_setfield(L, -2, "irq");

    lua_setfield(L, -2, "times");

    lua_rawseti(L, -2, i + 1);
  }

  uv_free_cpu_info(cpu_infos, count);

  return 1;
}

static int LuaIO_system_meminfo(lua_State *L) {
  uint64_t total = uv_get_total_memory();
  uint64_t free = uv_get_free_memory();
  
  lua_createtable(L, 0, 2);
  lua_pushinteger(L, total);
  lua_setfield(L, -2, "total");
  lua_pushinteger(L, free);
  lua_setfield(L, -2, "free");

  return 1;
}

static int LuaIO_system_loadavg(lua_State *L) {
  double loadavg[3];
  uv_loadavg(loadavg);
  
  lua_createtable(L, 3, 0);
  lua_pushnumber(L, loadavg[0]);
  lua_rawseti(L, -2, 1);
  lua_pushnumber(L, loadavg[1]);
  lua_rawseti(L, -2, 2);
  lua_pushnumber(L, loadavg[2]);
  lua_rawseti(L, -2, 3);

  return 1;
}

/*nanoseconds*/
static int LuaIO_system_hrtime(lua_State *L) {
  uint64_t hrtime = uv_hrtime();
  lua_pushinteger(L, hrtime);
  return 1;
}

/*seconds ?*/
static int LuaIO_system_uptime(lua_State *L) {
  double uptime;
  int err = uv_uptime(&uptime);
  if (err < 0)return luaL_error(L, "system.uptime() error: %s", uv_strerror(err));
  
  lua_pushnumber(L, uptime);

  return 1;
}

int luaopen_system(lua_State *L) {
  const char *type;
  const char *release;
  const char *endian;
#define test 1
#ifdef test
  struct utsname info;
  if (uname(&info) < 0) {
    type = "Unknown";
    release = "Unknown";
  }

  type = info.sysname;
  release = info.release;
#else
  type = "Windows";

  OSVERSIONINFOW info;
  info.dwOSVersionInfoSize = sizeof(info);

  // Don't complain that GetVersionEx is deprecated; there is no alternative.
  #pragma warning(suppress : 4996)
  if (GetVersionExW(&info) == 0) {
    release = "Unknown";
  } else {
    char buf[256];
    snprintf(buf,
             sizeof(buf),
             "%d.%d.%d",
             (int)info.dwMajorVersion,
             (int)info.dwMinorVersion,
             (int)info.dwBuildNumber);
    release = buf;
  }
#endif

  union {
    uint32_t i;
    uint8_t c;
  } u;
  u.i = 1;
  endian = (u.c == 1) ? "LE" : "BE";

  luaL_Reg lib[] = {
    { "cpuinfo", LuaIO_system_cpuinfo },
    { "meminfo", LuaIO_system_meminfo },
    { "loadavg", LuaIO_system_loadavg },
    { "hrtime", LuaIO_system_hrtime },
    { "uptime", LuaIO_system_uptime },
    { "__newindex", LuaIO_cannot_change },
    { NULL, NULL }
  };

  lua_createtable(L, 0, 0);

  luaL_newlib(L, lib);
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  lua_pushliteral(L, "metatable is protected.");
  lua_setfield(L, -2, "__metatable");

  lua_pushlstring(L, type, strlen(type));
  lua_setfield(L, -2, "type");
  lua_pushlstring(L, release, strlen(release));
  lua_setfield(L, -2, "release");
  lua_pushlstring(L, endian, 2);
  lua_setfield(L, -2, "endian");

  lua_setmetatable(L, -2);

  return 1;
}
