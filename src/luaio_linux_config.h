/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @license: MIT license
 * @overview: 
 */

#ifndef LUAIO_LINUX_CONFIG_H
#define LUAIO_LINUX_CONFIG_H

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <assert.h>

/*PATH_MAX*/
#include <limits.h>
/*memalgin*/
#include <malloc.h>

#include <signal.h>
#include <sched.h>
#include <sys/resource.h>
/*uname*/
#include <sys/utsname.h>

/*dns*/
#include <arpa/nameser.h>
#include <arpa/inet.h>

/*buffer*/
#include <endian.h>

#endif /* LUAIO_LINUX_CONFIG_H */
