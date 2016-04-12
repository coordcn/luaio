/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @license: MIT license
 * @overview: 
 */

#include "luaio_timer.h"

static luaio_timer_pool_t luaio_timer_pool;

void luaio_timer_init(size_t max_free_timers) {
  luaio_list_init(&(luaio_timer_pool.free_list));
  luaio_timer_pool.max_free_timers = max_free_timers;
  luaio_timer_pool.free_timers = 0;
}

void luaio_timer_set_max_free_timers(size_t max_free_timers) {
  luaio_timer_pool.max_free_timers = max_free_timers;
}

uv_timer_t *luaio_timer_alloc() {
  luaio_list_t *free_list = &(luaio_timer_pool.free_list);
  luaio_timer_t *luaio_timer;
  uv_timer_t *timer;

  if (!luaio_list_is_empty(free_list)) {
    luaio_list_t *list = free_list->next;
    luaio_timer = luaio_list_entry(list, luaio_timer_t, list);
    timer = &luaio_timer->timer;
    luaio_list_remove(list);
    luaio_timer_pool.free_timers--;  
  } else {
    luaio_timer = luaio_memalign(LUAIO_TIMER_ALIGNMENT, sizeof(luaio_timer_t));
    if (luaio_timer == NULL) return NULL;

    timer = &luaio_timer->timer;
    uv_timer_init(uv_default_loop(), timer);
  }

  return timer;
}

static void luaio_timer_onclose(uv_handle_t *handle) {
  luaio_timer_t *luaio_timer = container_of(handle, luaio_timer_t, timer);
  luaio_free(luaio_timer);
}

void luaio_timer_free(uv_timer_t *timer) {
  if (timer != NULL) {
    luaio_timer_t *luaio_timer = luaio_list_entry(timer, luaio_timer_t, timer);

    if (luaio_timer_pool.free_timers < luaio_timer_pool.max_free_timers) {
      luaio_list_insert_head(&luaio_timer->list, &(luaio_timer_pool.free_list));
      luaio_timer_pool.free_timers++;
    } else {
      uv_close((uv_handle_t*)timer, luaio_timer_onclose);
    }
  }
}
