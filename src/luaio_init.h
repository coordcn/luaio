/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @license: MIT license
 * @overview: 
 */

#ifndef LUAIO_INIT_H
#define LUAIO_INIT_H

#include "uv.h"
#include "lua.h"
#include "lauxlib.h"
#include "luaio_config.h"

int luaio_init(lua_State *L, int argc, char *argv[]);

lua_State *luaio_get_main_thread();
uint64_t luaio_get_start_time();

int luaopen_errno(lua_State *L);
int luaopen_system(lua_State *L);
int luaopen_signal(lua_State *L);
int luaopen_process(lua_State *L);

int luaopen_strlib(lua_State *L);
void luaio_date_init(); 
int luaopen_date(lua_State *L);

int luaopen_read_buffer(lua_State *L);
int luaopen_write_buffer(lua_State *L);

int luaio_parse_socket_address(lua_State *L, struct sockaddr_storage *addr);
void luaio_dns_init(lua_State *L);
int luaopen_dns(lua_State *L);
int luaopen_tcp(lua_State *L);
int luaopen_http(lua_State *L);
int luaopen_fs(lua_State *L);

#endif /* LUAIO_INIT_H */
