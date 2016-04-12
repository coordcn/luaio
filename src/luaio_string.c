/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @license: MIT license
 * @overview: 
 */

#include "luaio_string.h"

#define LOWER8    0x20
#define LOWER16   0x2020
#define LOWER32   0x20202020
#define LOWER64   0x2020202020202020

int luaio_streq_32(const char *s1, const char *s2, size_t n) {
  if (n <= LUAIO_SHORT_STRING_LENGTH) {
    if (s1 == s2) return 1;

    const char *ss1 = s1 + n;
    const char *ss2 = s2 + n;

    switch (n) {
      case 32:
        if (*(uint32_t*)(ss1 - 32) != *(uint32_t*)(ss2 - 32)) return 0;
      case 28:
        if (*(uint32_t*)(ss1 - 28) != *(uint32_t*)(ss2 - 28)) return 0;
      case 24:
        if (*(uint32_t*)(ss1 - 24) != *(uint32_t*)(ss2 - 24)) return 0;
      case 20:
        if (*(uint32_t*)(ss1 - 20) != *(uint32_t*)(ss2 - 20)) return 0;
      case 16:
        if (*(uint32_t*)(ss1 - 16) != *(uint32_t*)(ss2 - 16)) return 0;
      case 12:
        if (*(uint32_t*)(ss1 - 12) != *(uint32_t*)(ss2 - 12)) return 0;
      case 8:
        if (*(uint32_t*)(ss1 - 8) != *(uint32_t*)(ss2 - 8)) return 0;
      case 4:
        if (*(uint32_t*)(ss1 - 4) != *(uint32_t*)(ss2 - 4)) return 0;
      case 0:
        return 1;

      case 31:
        if (*(uint32_t*)(ss1 - 31) != *(uint32_t*)(ss2 - 31)) return 0;
      case 27:
        if (*(uint32_t*)(ss1 - 27) != *(uint32_t*)(ss2 - 27)) return 0;
      case 23:
        if (*(uint32_t*)(ss1 - 23) != *(uint32_t*)(ss2 - 23)) return 0;
      case 19:
        if (*(uint32_t*)(ss1 - 19) != *(uint32_t*)(ss2 - 19)) return 0;
      case 15:
        if (*(uint32_t*)(ss1 - 15) != *(uint32_t*)(ss2 - 15)) return 0;
      case 11:
        if (*(uint32_t*)(ss1 - 11) != *(uint32_t*)(ss2 - 11)) return 0;
      case 7:
        if (*(uint32_t*)(ss1 - 7) != *(uint32_t*)(ss2 - 7)) return 0;
      case 3:
        if (*(uint16_t*)(ss1 - 3) != *(uint16_t*)(ss2 - 3)) return 0;
        return *(ss1 - 1) == *(ss2 - 1);

      case 30:
        if (*(uint32_t*)(ss1 - 30) != *(uint32_t*)(ss2 - 30)) return 0;
      case 26:
        if (*(uint32_t*)(ss1 - 26) != *(uint32_t*)(ss2 - 26)) return 0;
      case 22:
        if (*(uint32_t*)(ss1 - 22) != *(uint32_t*)(ss2 - 22)) return 0;
      case 18:
        if (*(uint32_t*)(ss1 - 18) != *(uint32_t*)(ss2 - 18)) return 0;
      case 14:
        if (*(uint32_t*)(ss1 - 14) != *(uint32_t*)(ss2 - 14)) return 0;
      case 10:
        if (*(uint32_t*)(ss1 - 10) != *(uint32_t*)(ss2 - 10)) return 0;
      case 6:
        if (*(uint32_t*)(ss1 - 6) != *(uint32_t*)(ss2 - 6)) return 0;
      case 2:
        return *(uint16_t*)(ss1 - 2) == *(uint16_t*)(ss2 - 2);

      case 29:
        if (*(uint32_t*)(ss1 - 29) != *(uint32_t*)(ss2 - 29)) return 0;
      case 25:
        if (*(uint32_t*)(ss1 - 25) != *(uint32_t*)(ss2 - 25)) return 0;
      case 21:
        if (*(uint32_t*)(ss1 - 21) != *(uint32_t*)(ss2 - 21)) return 0;
      case 17:
        if (*(uint32_t*)(ss1 - 17) != *(uint32_t*)(ss2 - 17)) return 0;
      case 13:
        if (*(uint32_t*)(ss1 - 13) != *(uint32_t*)(ss2 - 13)) return 0;
      case 9:
        if (*(uint32_t*)(ss1 - 9) != *(uint32_t*)(ss2 - 9)) return 0;
      case 5:
        if (*(uint32_t*)(ss1 - 5) != *(uint32_t*)(ss2 - 5)) return 0;
      case 1:
        return *(ss1 - 1) == *(ss2 - 1);

      default: return 0;
    }
  } else {
    int ret = luaio_strncmp(s1, s2, n);
    return ret ? 0 : 1;
  }
}

int luaio_strcaseeq_32(const char *s1, const char *s2, size_t n) {
  if (n <= LUAIO_SHORT_STRING_LENGTH) {
    if (s1 == s2) return 1;

    const char *ss1 = s1 + n;
    const char *ss2 = s2 + n;

    switch (n) {
      case 32:
        if ((*(uint32_t*)(ss1 - 32) | LOWER32) != (*(uint32_t*)(ss2 - 32) | LOWER32)) return 0;
      case 28:
        if ((*(uint32_t*)(ss1 - 28) | LOWER32) != (*(uint32_t*)(ss2 - 28) | LOWER32)) return 0;
      case 24:
        if ((*(uint32_t*)(ss1 - 24) | LOWER32) != (*(uint32_t*)(ss2 - 24) | LOWER32)) return 0;
      case 20:
        if ((*(uint32_t*)(ss1 - 20) | LOWER32) != (*(uint32_t*)(ss2 - 20) | LOWER32)) return 0;
      case 16:
        if ((*(uint32_t*)(ss1 - 16) | LOWER32) != (*(uint32_t*)(ss2 - 16) | LOWER32)) return 0;
      case 12:
        if ((*(uint32_t*)(ss1 - 12) | LOWER32) != (*(uint32_t*)(ss2 - 12) | LOWER32)) return 0;
      case 8:
        if ((*(uint32_t*)(ss1 - 8) | LOWER32) != (*(uint32_t*)(ss2 - 8) | LOWER32)) return 0;
      case 4:
        if ((*(uint32_t*)(ss1 - 4) | LOWER32) != (*(uint32_t*)(ss2 - 4) | LOWER32)) return 0;
      case 0:
        return 1;

      case 31:
        if ((*(uint32_t*)(ss1 - 31) | LOWER32) != (*(uint32_t*)(ss2 - 31) | LOWER32)) return 0;
      case 27:
        if ((*(uint32_t*)(ss1 - 27) | LOWER32) != (*(uint32_t*)(ss2 - 27) | LOWER32)) return 0;
      case 23:
        if ((*(uint32_t*)(ss1 - 23) | LOWER32) != (*(uint32_t*)(ss2 - 23) | LOWER32)) return 0;
      case 19:
        if ((*(uint32_t*)(ss1 - 19) | LOWER32) != (*(uint32_t*)(ss2 - 19) | LOWER32)) return 0;
      case 15:
        if ((*(uint32_t*)(ss1 - 15) | LOWER32) != (*(uint32_t*)(ss2 - 15) | LOWER32)) return 0;
      case 11:
        if ((*(uint32_t*)(ss1 - 11) | LOWER32) != (*(uint32_t*)(ss2 - 11) | LOWER32)) return 0;
      case 7:
        if ((*(uint32_t*)(ss1 - 7) | LOWER32) != (*(uint32_t*)(ss2 - 7) | LOWER32)) return 0;
      case 3:
        if ((*(uint16_t*)(ss1 - 3) | LOWER16) != (*(uint16_t*)(ss2 - 3) | LOWER16)) return 0;
        return (*(ss1 - 1) | LOWER8) == (*(ss2 - 1) | LOWER8);

      case 30:
        if ((*(uint32_t*)(ss1 - 30) | LOWER32) != (*(uint32_t*)(ss2 - 30) | LOWER32)) return 0;
      case 26:
        if ((*(uint32_t*)(ss1 - 26) | LOWER32) != (*(uint32_t*)(ss2 - 26) | LOWER32)) return 0;
      case 22:
        if ((*(uint32_t*)(ss1 - 22) | LOWER32) != (*(uint32_t*)(ss2 - 22) | LOWER32)) return 0;
      case 18:
        if ((*(uint32_t*)(ss1 - 18) | LOWER32) != (*(uint32_t*)(ss2 - 18) | LOWER32)) return 0;
      case 14:
        if ((*(uint32_t*)(ss1 - 14) | LOWER32) != (*(uint32_t*)(ss2 - 14) | LOWER32)) return 0;
      case 10:
        if ((*(uint32_t*)(ss1 - 10) | LOWER32) != (*(uint32_t*)(ss2 - 10) | LOWER32)) return 0;
      case 6:
        if ((*(uint32_t*)(ss1 - 6) | LOWER32) != (*(uint32_t*)(ss2 - 6) | LOWER32)) return 0;
      case 2:
        return (*(uint16_t*)(ss1 - 2) | LOWER16) == (*(uint16_t*)(ss2 - 2) | LOWER16);

      case 29:
        if ((*(uint32_t*)(ss1 - 29) | LOWER32) != (*(uint32_t*)(ss2 - 29) | LOWER32)) return 0;
      case 25:
        if ((*(uint32_t*)(ss1 - 25) | LOWER32) != (*(uint32_t*)(ss2 - 25) | LOWER32)) return 0;
      case 21:
        if ((*(uint32_t*)(ss1 - 21) | LOWER32) != (*(uint32_t*)(ss2 - 21) | LOWER32)) return 0;
      case 17:
        if ((*(uint32_t*)(ss1 - 17) | LOWER32) != (*(uint32_t*)(ss2 - 17) | LOWER32)) return 0;
      case 13:
        if ((*(uint32_t*)(ss1 - 13) | LOWER32) != (*(uint32_t*)(ss2 - 13) | LOWER32)) return 0;
      case 9:
        if ((*(uint32_t*)(ss1 - 9) | LOWER32) != (*(uint32_t*)(ss2 - 9) | LOWER32)) return 0;
      case 5:
        if ((*(uint32_t*)(ss1 - 5) | LOWER32) != (*(uint32_t*)(ss2 - 5) | LOWER32)) return 0;
      case 1:
        return (*(ss1 - 1) | LOWER8) == (*(ss2 - 1) | LOWER8);

      default: return 0;
    }
  } else {
    int ret = luaio_strncasecmp(s1, s2, n);
    return ret ? 0 : 1;
  }
}

int luaio_streq_64(const char *s1, const char *s2, size_t n) {
  if (n <= LUAIO_SHORT_STRING_LENGTH) {
    if (s1 == s2) return 1;

    const char *ss1 = s1 + n;
    const char *ss2 = s2 + n;

    switch (n) {
      case 64:
        if (*(uint64_t*)(ss1 - 64) != *(uint64_t*)(ss2 - 64)) return 0;
      case 56:
        if (*(uint64_t*)(ss1 - 56) != *(uint64_t*)(ss2 - 56)) return 0;
      case 48:
        if (*(uint64_t*)(ss1 - 48) != *(uint64_t*)(ss2 - 48)) return 0;
      case 40:
        if (*(uint64_t*)(ss1 - 40) != *(uint64_t*)(ss2 - 40)) return 0;
      case 32:
        if (*(uint64_t*)(ss1 - 32) != *(uint64_t*)(ss2 - 32)) return 0;
      case 24:
        if (*(uint64_t*)(ss1 - 24) != *(uint64_t*)(ss2 - 24)) return 0;
      case 16:
        if (*(uint64_t*)(ss1 - 16) != *(uint64_t*)(ss2 - 16)) return 0;
      case 8:
        if (*(uint64_t*)(ss1 - 8) != *(uint64_t*)(ss2 - 8)) return 0;
      case 0:
        return 1;

      case 63:
        if (*(uint64_t*)(ss1 - 63) != *(uint64_t*)(ss2 - 63)) return 0;
      case 55:
        if (*(uint64_t*)(ss1 - 55) != *(uint64_t*)(ss2 - 55)) return 0;
      case 47:
        if (*(uint64_t*)(ss1 - 47) != *(uint64_t*)(ss2 - 47)) return 0;
      case 39:
        if (*(uint64_t*)(ss1 - 39) != *(uint64_t*)(ss2 - 39)) return 0;
      case 31:
        if (*(uint64_t*)(ss1 - 31) != *(uint64_t*)(ss2 - 31)) return 0;
      case 23:
        if (*(uint64_t*)(ss1 - 23) != *(uint64_t*)(ss2 - 23)) return 0;
      case 15:
        if (*(uint64_t*)(ss1 - 15) != *(uint64_t*)(ss2 - 15)) return 0;
      case 7:
        if (*(uint32_t*)(ss1 - 7) != *(uint32_t*)(ss2 - 7)) return 0;
        return *(uint32_t*)(ss1 - 4) == *(uint32_t*)(ss2 - 4);

      case 62:
        if (*(uint64_t*)(ss1 - 62) != *(uint64_t*)(ss2 - 62)) return 0;
      case 54:
        if (*(uint64_t*)(ss1 - 54) != *(uint64_t*)(ss2 - 54)) return 0;
      case 46:
        if (*(uint64_t*)(ss1 - 46) != *(uint64_t*)(ss2 - 46)) return 0;
      case 38:
        if (*(uint64_t*)(ss1 - 38) != *(uint64_t*)(ss2 - 38)) return 0;
      case 30:
        if (*(uint64_t*)(ss1 - 30) != *(uint64_t*)(ss2 - 30)) return 0;
      case 22:
        if (*(uint64_t*)(ss1 - 22) != *(uint64_t*)(ss2 - 22)) return 0;
      case 14:
        if (*(uint64_t*)(ss1 - 14) != *(uint64_t*)(ss2 - 14)) return 0;
      case 6:
        if (*(uint32_t*)(ss1 - 6) != *(uint32_t*)(ss2 - 6)) return 0;
        return *(uint16_t*)(ss1 - 2) == *(uint16_t*)(ss2 - 2);

      case 61:
        if (*(uint64_t*)(ss1 - 61) != *(uint64_t*)(ss2 - 61)) return 0;
      case 53:
        if (*(uint64_t*)(ss1 - 53) != *(uint64_t*)(ss2 - 53)) return 0;
      case 45:
        if (*(uint64_t*)(ss1 - 45) != *(uint64_t*)(ss2 - 45)) return 0;
      case 37:
        if (*(uint64_t*)(ss1 - 37) != *(uint64_t*)(ss2 - 37)) return 0;
      case 29:
        if (*(uint64_t*)(ss1 - 29) != *(uint64_t*)(ss2 - 29)) return 0;
      case 21:
        if (*(uint64_t*)(ss1 - 21) != *(uint64_t*)(ss2 - 21)) return 0;
      case 13:
        if (*(uint64_t*)(ss1 - 13) != *(uint64_t*)(ss2 - 13)) return 0;
      case 5:
        if (*(uint32_t*)(ss1 - 5) != *(uint32_t*)(ss2 - 5)) return 0;
        return *(ss1 - 1) == *(ss2 - 1);

      case 60:
        if (*(uint64_t*)(ss1 - 60) != *(uint64_t*)(ss2 - 60)) return 0;
      case 52:
        if (*(uint64_t*)(ss1 - 52) != *(uint64_t*)(ss2 - 52)) return 0;
      case 44:
        if (*(uint64_t*)(ss1 - 44) != *(uint64_t*)(ss2 - 44)) return 0;
      case 36:
        if (*(uint64_t*)(ss1 - 36) != *(uint64_t*)(ss2 - 36)) return 0;
      case 28:
        if (*(uint64_t*)(ss1 - 28) != *(uint64_t*)(ss2 - 28)) return 0;
      case 20:
        if (*(uint64_t*)(ss1 - 20) != *(uint64_t*)(ss2 - 20)) return 0;
      case 12:
        if (*(uint64_t*)(ss1 - 12) != *(uint64_t*)(ss2 - 12)) return 0;
      case 4:
        return *(uint32_t*)(ss1 - 4) == *(uint32_t*)(ss2 - 4);

      case 59:
        if (*(uint64_t*)(ss1 - 59) != *(uint64_t*)(ss2 - 59)) return 0;
      case 51:
        if (*(uint64_t*)(ss1 - 51) != *(uint64_t*)(ss2 - 51)) return 0;
      case 43:
        if (*(uint64_t*)(ss1 - 43) != *(uint64_t*)(ss2 - 43)) return 0;
      case 35:
        if (*(uint64_t*)(ss1 - 35) != *(uint64_t*)(ss2 - 35)) return 0;
      case 27:
        if (*(uint64_t*)(ss1 - 27) != *(uint64_t*)(ss2 - 27)) return 0;
      case 19:
        if (*(uint64_t*)(ss1 - 19) != *(uint64_t*)(ss2 - 19)) return 0;
      case 11:
        if (*(uint64_t*)(ss1 - 11) != *(uint64_t*)(ss2 - 11)) return 0;
      case 3:
        if (*(uint16_t*)(ss1 - 3) != *(uint16_t*)(ss2 - 3)) return 0;
        return *(ss1 - 1) == *(ss2 - 1);

      case 58:
        if (*(uint64_t*)(ss1 - 58) != *(uint64_t*)(ss2 - 58)) return 0;
      case 50:
        if (*(uint64_t*)(ss1 - 50) != *(uint64_t*)(ss2 - 50)) return 0;
      case 42:
        if (*(uint64_t*)(ss1 - 42) != *(uint64_t*)(ss2 - 42)) return 0;
      case 34:
        if (*(uint64_t*)(ss1 - 34) != *(uint64_t*)(ss2 - 34)) return 0;
      case 26:
        if (*(uint64_t*)(ss1 - 26) != *(uint64_t*)(ss2 - 26)) return 0;
      case 18:
        if (*(uint64_t*)(ss1 - 18) != *(uint64_t*)(ss2 - 18)) return 0;
      case 10:
        if (*(uint64_t*)(ss1 - 10) != *(uint64_t*)(ss2 - 10)) return 0;
      case 2:
        return *(uint16_t*)(ss1 - 2) == *(uint16_t*)(ss2 - 2);

      case 57:
        if (*(uint64_t*)(ss1 - 57) != *(uint64_t*)(ss2 - 57)) return 0;
      case 49:
        if (*(uint64_t*)(ss1 - 49) != *(uint64_t*)(ss2 - 49)) return 0;
      case 41:
        if (*(uint64_t*)(ss1 - 41) != *(uint64_t*)(ss2 - 41)) return 0;
      case 33:
        if (*(uint64_t*)(ss1 - 33) != *(uint64_t*)(ss2 - 33)) return 0;
      case 25:
        if (*(uint64_t*)(ss1 - 25) != *(uint64_t*)(ss2 - 25)) return 0;
      case 17:
        if (*(uint64_t*)(ss1 - 17) != *(uint64_t*)(ss2 - 17)) return 0;
      case 9:
        if (*(uint64_t*)(ss1 - 9) != *(uint64_t*)(ss2 - 9)) return 0;
      case 1:
        return *(ss1 - 1) == *(ss2 - 1);

      default: return 0;
    }
  } else {
    int ret = luaio_strncmp(s1, s2, n);
    return ret ? 0 : 1;
  }
}

int luaio_strcaseeq_64(const char *s1, const char *s2, size_t n) {
  if (n <= LUAIO_SHORT_STRING_LENGTH) {
    if (s1 == s2) return 1;

    const char *ss1 = s1 + n;
    const char *ss2 = s2 + n;

    switch (n) {
      case 64:
        if ((*(uint64_t*)(ss1 - 64) | LOWER64) != (*(uint64_t*)(ss2 - 64) | LOWER64)) return 0;
      case 56:
        if ((*(uint64_t*)(ss1 - 56) | LOWER64) != (*(uint64_t*)(ss2 - 56) | LOWER64)) return 0;
      case 48:
        if ((*(uint64_t*)(ss1 - 48) | LOWER64) != (*(uint64_t*)(ss2 - 48) | LOWER64)) return 0;
      case 40:
        if ((*(uint64_t*)(ss1 - 40) | LOWER64) != (*(uint64_t*)(ss2 - 40) | LOWER64)) return 0;
      case 32:
        if ((*(uint64_t*)(ss1 - 32) | LOWER64) != (*(uint64_t*)(ss2 - 32) | LOWER64)) return 0;
      case 24:
        if ((*(uint64_t*)(ss1 - 24) | LOWER64) != (*(uint64_t*)(ss2 - 24) | LOWER64)) return 0;
      case 16:
        if ((*(uint64_t*)(ss1 - 16) | LOWER64) != (*(uint64_t*)(ss2 - 16) | LOWER64)) return 0;
      case 8:
        if ((*(uint64_t*)(ss1 - 8) | LOWER64) != (*(uint64_t*)(ss2 - 8) | LOWER64)) return 0;
      case 0:
        return 1;

      case 63:
        if ((*(uint64_t*)(ss1 - 63) | LOWER64) != (*(uint64_t*)(ss2 - 63) | LOWER64)) return 0;
      case 55:
        if ((*(uint64_t*)(ss1 - 55) | LOWER64) != (*(uint64_t*)(ss2 - 55) | LOWER64)) return 0;
      case 47:
        if ((*(uint64_t*)(ss1 - 47) | LOWER64) != (*(uint64_t*)(ss2 - 47) | LOWER64)) return 0;
      case 39:
        if ((*(uint64_t*)(ss1 - 39) | LOWER64) != (*(uint64_t*)(ss2 - 39) | LOWER64)) return 0;
      case 31:
        if ((*(uint64_t*)(ss1 - 31) | LOWER64) != (*(uint64_t*)(ss2 - 31) | LOWER64)) return 0;
      case 23:
        if ((*(uint64_t*)(ss1 - 23) | LOWER64) != (*(uint64_t*)(ss2 - 23) | LOWER64)) return 0;
      case 15:
        if ((*(uint64_t*)(ss1 - 15) | LOWER64) != (*(uint64_t*)(ss2 - 15) | LOWER64)) return 0;
      case 7:
        if ((*(uint32_t*)(ss1 - 7) | LOWER32) != (*(uint32_t*)(ss2 - 7) | LOWER32)) return 0;
        return (*(uint32_t*)(ss1 - 4) | LOWER32) == (*(uint32_t*)(ss2 - 4) | LOWER32);

      case 62:
        if ((*(uint64_t*)(ss1 - 62) | LOWER64) != (*(uint64_t*)(ss2 - 62) | LOWER64)) return 0;
      case 54:
        if ((*(uint64_t*)(ss1 - 54) | LOWER64) != (*(uint64_t*)(ss2 - 54) | LOWER64)) return 0;
      case 46:
        if ((*(uint64_t*)(ss1 - 46) | LOWER64) != (*(uint64_t*)(ss2 - 46) | LOWER64)) return 0;
      case 38:
        if ((*(uint64_t*)(ss1 - 38) | LOWER64) != (*(uint64_t*)(ss2 - 38) | LOWER64)) return 0;
      case 30:
        if ((*(uint64_t*)(ss1 - 30) | LOWER64) != (*(uint64_t*)(ss2 - 30) | LOWER64)) return 0;
      case 22:
        if ((*(uint64_t*)(ss1 - 22) | LOWER64) != (*(uint64_t*)(ss2 - 22) | LOWER64)) return 0;
      case 14:
        if ((*(uint64_t*)(ss1 - 14) | LOWER64) != (*(uint64_t*)(ss2 - 14) | LOWER64)) return 0;
      case 6:
        if ((*(uint32_t*)(ss1 - 6) | LOWER32) != (*(uint32_t*)(ss2 - 6) | LOWER32)) return 0;
        return (*(uint16_t*)(ss1 - 2) | LOWER16) == (*(uint16_t*)(ss2 - 2) | LOWER16);

      case 61:
        if ((*(uint64_t*)(ss1 - 61) | LOWER64) != (*(uint64_t*)(ss2 - 61) | LOWER64)) return 0;
      case 53:
        if ((*(uint64_t*)(ss1 - 53) | LOWER64) != (*(uint64_t*)(ss2 - 53) | LOWER64)) return 0;
      case 45:
        if ((*(uint64_t*)(ss1 - 45) | LOWER64) != (*(uint64_t*)(ss2 - 45) | LOWER64)) return 0;
      case 37:
        if ((*(uint64_t*)(ss1 - 37) | LOWER64) != (*(uint64_t*)(ss2 - 37) | LOWER64)) return 0;
      case 29:
        if ((*(uint64_t*)(ss1 - 29) | LOWER64) != (*(uint64_t*)(ss2 - 29) | LOWER64)) return 0;
      case 21:
        if ((*(uint64_t*)(ss1 - 21) | LOWER64) != (*(uint64_t*)(ss2 - 21) | LOWER64)) return 0;
      case 13:
        if ((*(uint64_t*)(ss1 - 13) | LOWER64) != (*(uint64_t*)(ss2 - 13) | LOWER64)) return 0;
      case 5:
        if ((*(uint32_t*)(ss1 - 5) | LOWER32) != (*(uint32_t*)(ss2 - 5) | LOWER32)) return 0;
        return (*(ss1 - 1) | LOWER8) == (*(ss2 - 1) | LOWER8);

      case 60:
        if ((*(uint64_t*)(ss1 - 60) | LOWER64) != (*(uint64_t*)(ss2 - 60) | LOWER64)) return 0;
      case 52:
        if ((*(uint64_t*)(ss1 - 52) | LOWER64) != (*(uint64_t*)(ss2 - 52) | LOWER64)) return 0;
      case 44:
        if ((*(uint64_t*)(ss1 - 44) | LOWER64) != (*(uint64_t*)(ss2 - 44) | LOWER64)) return 0;
      case 36:
        if ((*(uint64_t*)(ss1 - 36) | LOWER64) != (*(uint64_t*)(ss2 - 36) | LOWER64)) return 0;
      case 28:
        if ((*(uint64_t*)(ss1 - 28) | LOWER64) != (*(uint64_t*)(ss2 - 28) | LOWER64)) return 0;
      case 20:
        if ((*(uint64_t*)(ss1 - 20) | LOWER64) != (*(uint64_t*)(ss2 - 20) | LOWER64)) return 0;
      case 12:
        if ((*(uint64_t*)(ss1 - 12) | LOWER64) != (*(uint64_t*)(ss2 - 12) | LOWER64)) return 0;
      case 4:
        return (*(uint32_t*)(ss1 - 4) | LOWER32) == (*(uint32_t*)(ss2 - 4) | LOWER32);

      case 59:
        if ((*(uint64_t*)(ss1 - 59) | LOWER64) != (*(uint64_t*)(ss2 - 59) | LOWER64)) return 0;
      case 51:
        if ((*(uint64_t*)(ss1 - 51) | LOWER64) != (*(uint64_t*)(ss2 - 51) | LOWER64)) return 0;
      case 43:
        if ((*(uint64_t*)(ss1 - 43) | LOWER64) != (*(uint64_t*)(ss2 - 43) | LOWER64)) return 0;
      case 35:
        if ((*(uint64_t*)(ss1 - 35) | LOWER64) != (*(uint64_t*)(ss2 - 35) | LOWER64)) return 0;
      case 27:
        if ((*(uint64_t*)(ss1 - 27) | LOWER64) != (*(uint64_t*)(ss2 - 27) | LOWER64)) return 0;
      case 19:
        if ((*(uint64_t*)(ss1 - 19) | LOWER64) != (*(uint64_t*)(ss2 - 19) | LOWER64)) return 0;
      case 11:
        if ((*(uint64_t*)(ss1 - 11) | LOWER64) != (*(uint64_t*)(ss2 - 11) | LOWER64)) return 0;
      case 3:
        if ((*(uint16_t*)(ss1 - 3) | LOWER16) != (*(uint16_t*)(ss2 - 3) | LOWER16)) return 0;
        return (*(ss1 - 1) | LOWER8) == (*(ss2 - 1) | LOWER8);

      case 58:
        if ((*(uint64_t*)(ss1 - 58) | LOWER64) != (*(uint64_t*)(ss2 - 58) | LOWER64)) return 0;
      case 50:
        if ((*(uint64_t*)(ss1 - 50) | LOWER64) != (*(uint64_t*)(ss2 - 50) | LOWER64)) return 0;
      case 42:
        if ((*(uint64_t*)(ss1 - 42) | LOWER64) != (*(uint64_t*)(ss2 - 42) | LOWER64)) return 0;
      case 34:
        if ((*(uint64_t*)(ss1 - 34) | LOWER64) != (*(uint64_t*)(ss2 - 34) | LOWER64)) return 0;
      case 26:
        if ((*(uint64_t*)(ss1 - 26) | LOWER64) != (*(uint64_t*)(ss2 - 26) | LOWER64)) return 0;
      case 18:
        if ((*(uint64_t*)(ss1 - 18) | LOWER64) != (*(uint64_t*)(ss2 - 18) | LOWER64)) return 0;
      case 10:
        if ((*(uint64_t*)(ss1 - 10) | LOWER64) != (*(uint64_t*)(ss2 - 10) | LOWER64)) return 0;
      case 2:
        return (*(uint16_t*)(ss1 - 2) | LOWER16) == (*(uint16_t*)(ss2 - 2) | LOWER16);

      case 57:
        if ((*(uint64_t*)(ss1 - 57) | LOWER64) != (*(uint64_t*)(ss2 - 57) | LOWER64)) return 0;
      case 49:
        if ((*(uint64_t*)(ss1 - 49) | LOWER64) != (*(uint64_t*)(ss2 - 49) | LOWER64)) return 0;
      case 41:
        if ((*(uint64_t*)(ss1 - 41) | LOWER64) != (*(uint64_t*)(ss2 - 41) | LOWER64)) return 0;
      case 33:
        if ((*(uint64_t*)(ss1 - 33) | LOWER64) != (*(uint64_t*)(ss2 - 33) | LOWER64)) return 0;
      case 25:
        if ((*(uint64_t*)(ss1 - 25) | LOWER64) != (*(uint64_t*)(ss2 - 25) | LOWER64)) return 0;
      case 17:
        if ((*(uint64_t*)(ss1 - 17) | LOWER64) != (*(uint64_t*)(ss2 - 17) | LOWER64)) return 0;
      case 9:
        if ((*(uint64_t*)(ss1 - 9) | LOWER64) != (*(uint64_t*)(ss2 - 9) | LOWER64)) return 0;
      case 1:
        return (*(ss1 - 1) | LOWER8) == (*(ss2 - 1) | LOWER8);
      
      default: return 0;
    }
  } else {
    int ret = luaio_strncasecmp(s1, s2, n);
    return ret ? 0 : 1;
  }
}
