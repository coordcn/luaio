/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @license: MIT license
 * @overview: 
 */

#ifndef LUAIO_INIT_H
#define LUAIO_INIT_H

#include "timer.h"
#include "lua.h"
#include "lauxlib.h"
#include "uv.h"

int LuaIO_init(lua_State *L, int argc, char* argv[]);
uint64_t LuaIO_get_start_time();

int luaopen_errno(lua_State* L);
int luaopen_system(lua_State* L);
int luaopen_process(lua_State* L);

void LuaIO_date_init(); 
int luaopen_date(lua_State *L);

int luaopen_read_buffer(lua_State *L);
int luaopen_write_buffer(lua_State *L);

void LuaIO_dns_init(lua_State* L);
int luaopen_dns(lua_State* L);

void LuaIO_tcp_connect_req_pool_init(size_t max_free_chunks);
void LuaIO_tcp_write_req_pool_init(size_t max_free_chunks);
void LuaIO_tcp_shutdown_req_pool_init(size_t max_free_chunks);
int luaopen_tcp(lua_State* L);

void LuaIO_fs_req_pool_init(size_t max_free_chunks);
int luaopen_fs(lua_State* L);

#endif /* LUAIO_INIT_H */
