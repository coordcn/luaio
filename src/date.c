/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @license: MIT license
 * @reference: nginx/src/core/ngx_parse_time.c
 *             lua/src/loslib.c
 */

#include "init.h"
#include "LuaIO.h"

#define LUA_DATE_UTC_SIZE       sizeof("Tue, 10 Nov 2002 23:50:13 GMT") - 1
#define LUA_DATE_LOCAL_SIZE     sizeof("2002-11-10 23:50:13") - 1
#define IS_NUM(c)               ((c) >= '0' && (c) <= '9')

static char *week[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
static char *months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                          "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
static int mday[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

static long int LuaIO_gmtoff;

void LuaIO_date_init() {
  time_t t = time(NULL);
  struct tm tmr, *stm;
  stm = localtime_r(&t, &tmr);

  if (stm == NULL) {
    fprintf(stderr, "LuaIO_date_init() error: can't get GMT offset, date.parseLocalString(time) maybe return an error value\n");
  } else {
    LuaIO_gmtoff = stm->tm_gmtoff;
  }
}

/*Tue, 10 Nov 2002 23:50:13 GMT*/
static time_t LuaIO_date_parse_utc(const char *str, size_t len) {
  uint64_t time;
  char c1, c2, c3, c4;
  int year, month, day, hour, min, sec;
  const char *p; 

  /*day*/
  p = str + 5;
  c1 = *p;
  c2 = *(p + 1);
  if (!IS_NUM(c1) || !IS_NUM(c2)) {
    return -1;
  }
  day = (c1 - '0') * 10 + (c2 - '0');

  /*month*/
  p += 3;
  switch (*p) {
    case 'J':
      month = *(p + 1) == 'a' ? 0 : *(p + 2) == 'n' ? 5 : 6;
      break;

    case 'F':
      month = 1;
      break;

    case 'M':
      month = *(p + 2) == 'r' ? 2 : 4;
      break;

    case 'A':
      month = *(p + 1) == 'p' ? 3 : 7;
      break;

    case 'S':
      month = 8;
      break;

    case 'O':
      month = 9;
      break;

    case 'N':
      month = 10;
      break;

    case 'D':
      month = 11;
      break;

    default:
      return -1;
  }

  /*year*/
  p += 4;
  c1 = *p;
  c2 = *(p + 1);
  c3 = *(p + 2);
  c4 = *(p + 3);
  if (!IS_NUM(c1) || !IS_NUM(c2) || !IS_NUM(c3) || !IS_NUM(c4)) {
    return -1;
  }
  year = (c1 - '0') * 1000 + (c2 - '0') * 100 + (c3 - '0') * 10 + (c4 - '0');

  /*hours*/
  p += 5;
  c1 = *p;
  c2 = *(p + 1);
  if (!IS_NUM(c1) || !IS_NUM(c2)) {
    return -1;
  }
  hour = (c1 - '0') * 10 + (c2 - '0'); 

  /*mimutes*/
  p += 3;
  c1 = *p;
  c2 = *(p + 1);
  if (!IS_NUM(c1) || !IS_NUM(c2)) {
    return -1;
  }
  min = (c1 - '0') * 10 + (c2 - '0');

  /*seconds*/
  p += 3;
  c1 = *p;
  c2 = *(p + 1);
  if (!IS_NUM(c1) || !IS_NUM(c2)) {
    return -1;
  }
  sec = (c1 - '0') * 10 + (c2 - '0');

  if (hour > 23 || min > 59 || sec > 59) {
    return -1;
  }

  if (day == 29 && month == 1) {
    if ((year & 3) || ((year % 100 == 0) && (year % 400) != 0)) {
      return -1;
    }
  } else if (day > mday[month]) {
    return -1;
  }

  /*
   * shift new year to March 1 and start months from 1 (not 0),
   * it is needed for Gauss' formula
   */
  if (--month <= 0) {
    month += 12;
    year -= 1;
  }

  /* Gauss' formula for Gregorian days since March 1, 1 BC */
  time = (uint64_t) (
         /* days in years including leap years since March 1, 1 BC */

         365 * year + year / 4 - year / 100 + year / 400

         /* days before the month */

         + 367 * month / 12 - 30

         /* days before the day */

         + day - 1

         /*
          * 719527 days were between March 1, 1 BC and March 1, 1970,
          * 31 and 28 days were in January and February 1970
          */

         - 719527 + 31 + 28) * 86400 + hour * 3600 + min * 60 + sec;

  return (time_t)time;
}

/*2002-11-10 23:50:13*/
static time_t LuaIO_date_parse_local(const char *str, size_t len) {
  uint64_t time;
  char c1, c2, c3, c4;
  int year, month, day, hour, min, sec;
  const char *p; 

  /*year*/
  p = str;
  c1 = *p;
  c2 = *(p + 1);
  c3 = *(p + 2);
  c4 = *(p + 3);
  if (!IS_NUM(c1) || !IS_NUM(c2) || !IS_NUM(c3) || !IS_NUM(c4)) {
    return -1;
  }
  year = (c1 - '0') * 1000 + (c2 - '0') * 100 + (c3 - '0') * 10 + (c4 - '0');

  /*month*/
  p += 5;
  c1 = *p;
  c2 = *(p + 1);
  if (!IS_NUM(c1) || !IS_NUM(c2)) {
    return -1;
  }
  month = (c1 - '0') * 10 + (c2 - '0');
  if (month < 1 || month > 12) {
    return -1;
  }
  month -= 1;

  /*day*/
  p += 3;
  c1 = *p;
  c2 = *(p + 1);
  if (!IS_NUM(c1) || !IS_NUM(c2)) {
    return -1;
  }
  day = (c1 - '0') * 10 + (c2 - '0');

  /*hours*/
  p += 3;
  c1 = *p;
  c2 = *(p + 1);
  if (!IS_NUM(c1) || !IS_NUM(c2)) {
    return -1;
  }
  hour = (c1 - '0') * 10 + (c2 - '0'); 

  /*mimutes*/
  p += 3;
  c1 = *p;
  c2 = *(p + 1);
  if (!IS_NUM(c1) || !IS_NUM(c2)) {
    return -1;
  }
  min = (c1 - '0') * 10 + (c2 - '0');

  /*seconds*/
  p += 3;
  c1 = *p;
  c2 = *(p + 1);
  if (!IS_NUM(c1) || !IS_NUM(c2)) {
    return -1;
  }
  sec = (c1 - '0') * 10 + (c2 - '0');

  if (hour > 23 || min > 59 || sec > 59) {
    return -1;
  }

  if (day == 29 && month == 1) {
    if ((year & 3) || ((year % 100 == 0) && (year % 400) != 0)) {
      return -1;
    }
  } else if (day > mday[month]) {
    return -1;
  }

  /*
   * shift new year to March 1 and start months from 1 (not 0),
   * it is needed for Gauss' formula
   */
  if (--month <= 0) {
    month += 12;
    year -= 1;
  }

  /* Gauss' formula for Gregorian days since March 1, 1 BC */
  time = (uint64_t) (
         /* days in years including leap years since March 1, 1 BC */

         365 * year + year / 4 - year / 100 + year / 400

         /* days before the month */

         + 367 * month / 12 - 30

         /* days before the day */

         + day - 1

         /*
          * 719527 days were between March 1, 1 BC and March 1, 1970,
          * 31 and 28 days were in January and February 1970
          */

         - 719527 + 31 + 28) * 86400 + hour * 3600 + min * 60 + sec;

  return (time_t)(time - LuaIO_gmtoff);
}

/* @example local now = date.now()
 * @return now {integer}
 */
static int LuaIO_date_now(lua_State *L) {
  time_t t = time(NULL);
  if (t == (time_t)(-1) || t != (time_t)(lua_Integer)t) {
    lua_pushnil(L);
  } else {
    lua_pushinteger(L, (lua_Integer)t);
  }

  return 1;
}

/* @example local ret = date.getUTCDate(time) 
 * @param time {nil|integer}
 * @return ret {table} @date.getUTCDate 
 *    if nil => error
 *    local ret = {
 *      year = {integer}
 *      month = {integer}
 *      day = {integer}
 *      hours = {integer}
 *      minutes = {integer}
 *      seconds = {integer}
 *      weekDay = {integer}
 *      yearDay = {integer}
 *    }
 */
static int LuaIO_date_get_utc_date(lua_State *L) {
  time_t t;
  if (lua_isnoneornil(L, 1)) {
    t = time(NULL);
  } else if (lua_isinteger(L, 1)) {
    t = (time_t)lua_tointeger(L, 1);
  } else {
    return luaL_argerror(L, 1, "date.getUTCDate(time) error: time must be [nil|integer]\n");
  }

  struct tm tmr, *stm;
  stm = gmtime_r(&t, &tmr);
  if (stm == NULL) {
    lua_pushnil(L);
  } else {
    lua_createtable(L, 0, 9);
    LuaIO_setinteger("seconds", stm->tm_sec)
    LuaIO_setinteger("minutes", stm->tm_min)
    LuaIO_setinteger("hours", stm->tm_hour)
    LuaIO_setinteger("day", stm->tm_mday)
    LuaIO_setinteger("month", stm->tm_mon + 1)
    LuaIO_setinteger("year", stm->tm_year + 1900)
    LuaIO_setinteger("weekDay", stm->tm_wday + 1)
    LuaIO_setinteger("yearDay", stm->tm_yday + 1)
  }

  return 1;
}

/* @example local ret = date.getLocalDate(time) 
 * @param time {nil|integer}
 * @return ret {table} @date.getUTCDate 
 */
static int LuaIO_date_get_local_date(lua_State *L) {
  time_t t;
  if (lua_isnoneornil(L, 1)) {
    t = time(NULL);
  } else if (lua_isinteger(L, 1)) {
    t = (time_t)lua_tointeger(L, 1);
  } else {
    return luaL_argerror(L, 1, "date.getLocalDate(time) error: time must be [nil|integer]\n");
  }

  struct tm tmr, *stm;
  stm = localtime_r(&t, &tmr);
  if (stm == NULL) {
    lua_pushnil(L);
  } else {
    lua_createtable(L, 0, 9);
    LuaIO_setinteger("seconds", stm->tm_sec)
    LuaIO_setinteger("minutes", stm->tm_min)
    LuaIO_setinteger("hours", stm->tm_hour)
    LuaIO_setinteger("day", stm->tm_mday)
    LuaIO_setinteger("month", stm->tm_mon + 1)
    LuaIO_setinteger("year", stm->tm_year + 1900)
    LuaIO_setinteger("weekDay", stm->tm_wday + 1)
    LuaIO_setinteger("yearDay", stm->tm_yday + 1)
  }

  return 1;
}

/* @example local ret = date.parseUTCString(time)
 * @param time {nil|string}
 *    Tue, 10 Nov 2002 23:50:13 GMT
 * @return ret {integer} if nil => error
 */
static int LuaIO_date_parse_utc_string(lua_State *L) {
  time_t t;
  if (lua_isnoneornil(L, 1)) {
    t = time(NULL);
  } else if (lua_type(L, 1) == LUA_TSTRING) {
    size_t len;
    const char *str = lua_tolstring(L, 1, &len);
    if(len != LUA_DATE_UTC_SIZE && len != LUA_DATE_LOCAL_SIZE) {
      lua_pushnil(L);
      return 1;
    }
    t = LuaIO_date_parse_utc(str, len);
  } else {
    return luaL_argerror(L, 1, "date.parseUTCString(time) error: time must be [nil|string]\n");
  }

  if (t == (time_t)(-1) || t != (time_t)(lua_Integer)t) {
    lua_pushnil(L);
  } else {
    lua_pushinteger(L, (lua_Integer)t);
  }

  return 1;
}

/* @example local ret = date.parseLocalString(str)
 * @param str {nil|string}
 *    2002-11-10 23:50:13
 * @return ret {integer} if nil => error
 */
static int LuaIO_date_parse_local_string(lua_State *L) {
  time_t t;
  if (lua_isnoneornil(L, 1)) {
    t = time(NULL);
  } else if (lua_type(L, 1) == LUA_TSTRING) {
    size_t len;
    const char *str = lua_tolstring(L, 1, &len);
    if(len != LUA_DATE_UTC_SIZE && len != LUA_DATE_LOCAL_SIZE) {
      lua_pushnil(L);
      return 1;
    }
    t = LuaIO_date_parse_local(str, len);
  } else {
    return luaL_argerror(L, 1, "date.parseLocalString(time) error: time must be [nil|string]\n");
  }

  if (t == (time_t)(-1) || t != (time_t)(lua_Integer)t) {
    lua_pushnil(L);
  } else {
    lua_pushinteger(L, (lua_Integer)t);
  }

  return 1;
}

/* @example local ret = date.getUTCString(time)
 * @param time {nil|integer}
 * @return ret {string} if nil => error
 *    Tue, 10 Nov 2002 23:50:13 GMT
 */
static int LuaIO_date_get_utc_string(lua_State *L) {
  time_t t;
  if (lua_isnoneornil(L, 1)) {
    t = time(NULL);
  } else if (lua_isinteger(L, 1)) {
    t = (time_t)lua_tointeger(L, 1);
  } else {
    return luaL_argerror(L, 1, "date.getUTCString(time) error: time must be [nil|integer]\n");
  }

  struct tm tmr, *stm;
  stm = gmtime_r(&t, &tmr);
  if (stm == NULL) {
    lua_pushnil(L);
  } else {
    char buf[LUA_DATE_UTC_SIZE];
    int year = stm->tm_year + 1900;
    int wday = stm->tm_wday;
    int mday = stm->tm_mday;
    int mon = stm->tm_mon;
    int hour = stm->tm_hour;
    int min = stm->tm_min;
    int sec = stm->tm_sec;
    const char *week_day = week[wday];
    const char *month = months[mon];

    buf[0] = week_day[0];
    buf[1] = week_day[1];
    buf[2] = week_day[2];
    buf[3] = ',';
    buf[4] = ' ';
    buf[5] = (mday / 10) + '0';
    buf[6] = (mday % 10) + '0';
    buf[7] = ' ';
    buf[8] = month[0];
    buf[9] = month[1];
    buf[10] = month[2];
    buf[11] = ' ';
    buf[12] = (year / 1000) +  '0';
    year %= 1000;
    buf[13] = (year / 100) + '0';
    year %= 100;
    buf[14] = (year / 10) + '0';
    buf[15] = (year % 10) + '0';
    buf[16] = ' ';
    buf[17] = (hour / 10) + '0';
    buf[18] = (hour % 10) + '0';
    buf[19] = ':';
    buf[20] = (min / 10) + '0';
    buf[21] = (min % 10) + '0';
    buf[22] = ':';
    buf[23] = (sec / 10) + '0';
    buf[24] = (sec % 10) + '0';
    buf[25] = ' ';
    buf[26] = 'G';
    buf[27] = 'M';
    buf[28] = 'T';

    lua_pushlstring(L, buf, LUA_DATE_UTC_SIZE);
  }

  return 1;
}

/* @example local ret = date.getLocalString(time)
 * @param time {nil|integer}
 * @return ret {string} if nil => error
 *    2002-11-10 23:50:13
 */
static int LuaIO_date_get_local_string(lua_State *L) {
  time_t t;
  if (lua_isnoneornil(L, 1)) {
    t = time(NULL);
  } else if (lua_isinteger(L, 1)) {
    t = (time_t)lua_tointeger(L, 1);
  } else {
    return luaL_argerror(L, 1, "date.getLocalString(time) error: time must be [nil|integer]\n");
  }

  struct tm tmr, *stm;
  stm = localtime_r(&t, &tmr);
  if (stm == NULL) {
    lua_pushnil(L);
  } else {
    char buf[LUA_DATE_LOCAL_SIZE];
    int year = stm->tm_year + 1900;
    int mday = stm->tm_mday;
    int mon = stm->tm_mon + 1;
    int hour = stm->tm_hour;
    int min = stm->tm_min;
    int sec = stm->tm_sec;

    buf[0] = (year / 1000) +  '0';
    year %= 1000;
    buf[1] = (year / 100) + '0';
    year %= 100;
    buf[2] = (year / 10) + '0';
    buf[3] = (year % 10) + '0';
    buf[4] = '-';
    buf[5] = (mon / 10) + '0';
    buf[6] = (mon % 10) + '0';
    buf[7] = '-';
    buf[8] = (mday / 10) + '0';
    buf[9] = (mday % 10) + '0';
    buf[10] = ' ';
    buf[11] = (hour / 10) + '0';
    buf[12] = (hour % 10) + '0';
    buf[13] = ':';
    buf[14] = (min / 10) + '0';
    buf[15] = (min % 10) + '0';
    buf[16] = ':';
    buf[17] = (sec / 10) + '0';
    buf[18] = (sec % 10) + '0';

    lua_pushlstring(L, buf, LUA_DATE_LOCAL_SIZE);
  }

  return 1;
}

int luaopen_date(lua_State *L) {
  luaL_Reg lib[] = {
    { "now", LuaIO_date_now },
    { "getUTCDate", LuaIO_date_get_utc_date },
    { "getLocalDate", LuaIO_date_get_local_date },
    { "parseUTCString", LuaIO_date_parse_utc_string },
    { "parseLocalString", LuaIO_date_parse_local_string },
    { "getUTCString", LuaIO_date_get_utc_string },
    { "getLocalString", LuaIO_date_get_local_string },
    { "__newindex", LuaIO_cannot_change },
    { NULL, NULL }
  };

  lua_createtable(L, 0, 0);

  luaL_newlib(L, lib);
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  lua_pushliteral(L, "metatable is protected.");
  lua_setfield(L, -2, "__metatable");

  lua_setmetatable(L, -2);

  return 1;
}
