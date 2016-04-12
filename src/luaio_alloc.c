/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @overview: alloc, calloc, realloc, free, memalign from nginx
 * @reference: ngx_alloc.c
 */

#include "luaio_alloc.h"

void *luaio_malloc(size_t size) {
  void *p = malloc(size);
  if (p == NULL) {
    fprintf(stderr, "malloc(size: %" PRId64 ") failed.\n", size);
  }

  return p;
}

void *luaio_realloc(void *p, size_t size) {
  void *new = realloc(p, size);
  if (new == NULL) {
    fprintf(stderr, "realloc(pointer: %p, size: %" PRId64 ") failed.\n", p, size);
  }

  return new;
}

#if (HAVE_POSIX_MEMALIGN)

void *luaio_memalign(size_t align, size_t size) {
  void *p;

  int err = posix_memalign(&p, align, size);
  if (err) {
    fprintf(stderr, "memalign(align: %" PRId64 ", size: %" PRId64 ") failed.\n", align, size);
    p = NULL;
  }

  return p;
}

#elif (HAVE_MEMALIGN)

void *luaio_memalign(size_t align, size_t size) {
  void *p = memalign(align, size);
  if (p == NULL) {
    fprintf(stderr, "memalign(align: %" PRId64 ", size: %" PRId64 ") failed.\n", align, size);
  }

  return p;
}

#endif
