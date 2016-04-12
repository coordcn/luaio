/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @license: MIT license
 * @overview: 
 */

#include "luaio_setaffinity.h"

int luaio_setaffinity(int pid, int cpu_id) {
#if defined(__linux__)
  cpu_set_t mask;
  CPU_ZERO(&mask);
  CPU_SET(cpu_id, &mask);
  return sched_setaffinity(pid, sizeof(cpu_set_t), &mask); 
#elif defined(__FreeBSD__)
  cpuset_t mask;
  CPU_ZERO(&mask);
  CPU_SET(cpu_id, &mask);
  return cpuset_setaffinity(CPU_LEVEL_WHICH,
                            CPU_WHICH_PID,
                            pid.
                            sizeof(cpuset_t),
                            &mask);
#else
  return -1;
#endif
}
