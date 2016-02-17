/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @license: MIT license
 * @overview: 
 */

#ifndef LUAIO_SETAFFINITY_H
#define LUAIO_SETAFFINITY_H

#if defined(__linux__)
#include <sched.h>
#elif defined(__FreeBSD__)
#include <sys/cpuset.h>
#endif

int LuaIO_setaffinity(int pid, int cpu_id);

#endif /* LUAIO_SETAFFINITY_H */
