/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @license: MIT license
 * @overview: 
 */

#include "timer.h"

static LuaIO_timer_pool_t LuaIO_timer_pool;

void LuaIO_timer_init(size_t max_free_timers) {
  LuaIO_list_init(&(LuaIO_timer_pool.free_list));
  LuaIO_timer_pool.max_free_timers = max_free_timers;
  LuaIO_timer_pool.free_timers = 0;
}

void LuaIO_timer_set_max_free_timers(size_t max_free_timers) {
  LuaIO_timer_pool.max_free_timers = max_free_timers;
}

uv_timer_t* LuaIO_timer_alloc() {
  LuaIO_list_t* free_list = &(LuaIO_timer_pool.free_list);
  LuaIO_timer_t* LuaIO_timer;
  uv_timer_t* timer;

  if (!LuaIO_list_is_empty(free_list)) {
    LuaIO_list_t* list = free_list->next;
    LuaIO_timer = LuaIO_list_entry(list, LuaIO_timer_t, list);
    timer = &LuaIO_timer->timer;
    LuaIO_list_remove(list);
    LuaIO_timer_pool.free_timers--;  
  } else {
    LuaIO_timer = LuaIO_memalign(LUAIO_TIMER_ALIGNMENT, sizeof(LuaIO_timer_t));
    if (LuaIO_timer == NULL) return NULL;

    timer = &LuaIO_timer->timer;
    uv_timer_init(uv_default_loop(), timer);
  }

  return timer;
}

void LuaIO_timer_free(uv_timer_t* timer) {
  if (timer != NULL) {
    LuaIO_timer_t* LuaIO_timer = LuaIO_list_entry(timer, LuaIO_timer_t, timer);

    if (LuaIO_timer_pool.free_timers < LuaIO_timer_pool.max_free_timers) {
      LuaIO_list_insert_head(&LuaIO_timer->list, &(LuaIO_timer_pool.free_list));
      LuaIO_timer_pool.free_timers++;
    } else {
      LuaIO_free(LuaIO_timer);
    }
  }
}
