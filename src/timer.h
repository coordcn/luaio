/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @license: MIT license
 * @overview: 
 */

#ifndef LUAIO_TIMER_H
#define LUAIO_TIMER_H

#include "list.h"
#include "alloc.h"
#include "uv.h"

#define LUAIO_TIMER_ALIGNMENT              64

typedef struct {
  LuaIO_list_t  free_list;
  size_t        max_free_timers;
  size_t        free_timers;
} LuaIO_timer_pool_t;

typedef struct {
  LuaIO_list_t  list;
  uv_timer_t    timer;
} LuaIO_timer_t;

void LuaIO_timer_init(size_t max_free_timers);
void LuaIO_timer_set_max_free_timers(size_t max_free_timers);
uv_timer_t *LuaIO_timer_alloc();
void LuaIO_timer_free(uv_timer_t *timer);

#endif /* LUAIO_TIMER_H */
