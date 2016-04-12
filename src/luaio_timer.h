/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @license: MIT license
 * @overview: 
 */

#ifndef LUAIO_TIMER_H
#define LUAIO_TIMER_H

#include "uv.h"
#include "luaio_list.h"
#include "luaio_alloc.h"

#define LUAIO_TIMER_ALIGNMENT 64

typedef struct {
  luaio_list_t  free_list;
  size_t        max_free_timers;
  size_t        free_timers;
} luaio_timer_pool_t;

typedef struct {
  luaio_list_t  list;
  uv_timer_t    timer;
} luaio_timer_t;

void luaio_timer_init(size_t max_free_timers);
void luaio_timer_set_max_free_timers(size_t max_free_timers);
uv_timer_t *luaio_timer_alloc();
void luaio_timer_free(uv_timer_t *timer);

#endif /* LUAIO_TIMER_H */
