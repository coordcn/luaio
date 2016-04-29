/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @license: MIT license
 * @overview: 
 */

#include "luaio.h"
#include "luaio_init.h"
#include "luaio_check_data.h"

typedef struct {
  uv_fs_t         req;
  lua_State       *current_thread;
  luaio_buffer_t  *read_buffer;
  size_t          bytes;
  int             write_data_ref;
} luaio_fs_req_t;

static int luaio_fs_parse_open_flags(lua_State *L, const char *flags, size_t len) {
  if (luaio_strcaseeq(flags, "r", 1)) return O_RDONLY;
  if (luaio_strcaseeq(flags, "w", 1)) return O_TRUNC | O_CREAT | O_WRONLY;
  if (luaio_strcaseeq(flags, "a", 1)) return O_APPEND | O_CREAT | O_WRONLY;
  if (luaio_strcaseeq(flags, "r+", 2)) return O_RDWR;
  if (luaio_strcaseeq(flags, "w+", 2)) return O_TRUNC | O_CREAT | O_RDWR;
  if (luaio_strcaseeq(flags, "a+", 2)) return O_APPEND | O_CREAT | O_RDWR;

  if (luaio_strcaseeq(flags, "wx", 2) || luaio_strcaseeq(flags, "xw", 2)) {
    return O_TRUNC | O_CREAT | O_WRONLY | O_EXCL;
  }

  if (luaio_strcaseeq(flags, "ax", 2) || luaio_strcaseeq(flags, "xa", 2)) {
    return O_APPEND | O_CREAT | O_WRONLY | O_EXCL;
  }

  if (luaio_strcaseeq(flags, "wx+", 3) || luaio_strcaseeq(flags, "xw+", 3)) {
    return O_TRUNC | O_CREAT | O_RDWR | O_EXCL;
  }

  if (luaio_strcaseeq(flags, "ax+", 3) || luaio_strcaseeq(flags, "xa+", 3)) {
    return O_APPEND | O_CREAT | O_RDWR |O_EXCL;
  }

  return luaL_error(L, "fs_native.open(path, flags, mode) error: unknown file open flag[%s]", flags);
}

static int luaio_fs_parse_access_mode(lua_State *L, const char *amode, int n) {
  int mode = 0;
  for (int i = 0; i < n; ++i) {
    switch (amode[i]) {
      case 'r':
      case 'R':
        mode |= R_OK;
        break;
      case 'w':
      case 'W':
        mode |= W_OK;
        break;
      case 'x':
      case 'X':
        mode |= X_OK;
        break;
      default:
        return luaL_error(L, "fs_native.access(path, mode) error: unknown file access mode[%s]", amode);
    }
  }

  return mode;
}

#define CREATE_REQ1() \
  luaio_fs_req_t *req = luaio_palloc(sizeof(luaio_fs_req_t)); \
  if (req == NULL) { \
    lua_pushinteger(L, UV_ENOMEM); \
    return 1; \
  } \
  req->current_thread = L;

#define CREATE_REQ2() \
  luaio_fs_req_t *req = luaio_palloc(sizeof(luaio_fs_req_t)); \
  if (req == NULL) { \
    lua_pushnil(L); \
    lua_pushinteger(L, UV_ENOMEM); \
    return 1; \
  } \
  req->current_thread = L;

#define FS_CALL1(name, req, ...) \
  int ret = uv_fs_##name(uv_default_loop(), &req->req, __VA_ARGS__, luaio_fs_callback); \
  if (ret < 0) { \
    luaio_pfree(req); \
    lua_pushinteger(L, ret); \
    return 1; \
  } \
  return lua_yield(L, 0);

#define FS_CALL2(name, req, ...) \
  int ret = uv_fs_##name(uv_default_loop(), &req->req, __VA_ARGS__, luaio_fs_callback); \
  if (ret < 0) { \
    luaio_pfree(req); \
    lua_pushnil(L); \
    lua_pushinteger(L, ret); \
    return 2; \
  } \
  return lua_yield(L, 0);

static void luaio_fs_create_stat(lua_State *L, const uv_stat_t *s) {
#define X(name) \
  lua_pushinteger(L, s->st_##name); \
  lua_setfield(L, -2, #name);

  lua_createtable(L, 0, 23);
  X(dev)
  X(ino)
  X(mode)
  X(nlink)
  X(uid)
  X(gid)
  X(rdev)
  X(size)
  X(blksize)
  X(blocks)

#undef X

  lua_pushinteger(L, s->st_atim.tv_sec);
  lua_setfield(L, -2, "atime");
  lua_pushinteger(L, s->st_mtim.tv_sec);
  lua_setfield(L, -2, "mtime");
  lua_pushinteger(L, s->st_ctim.tv_sec);
  lua_setfield(L, -2, "ctime");
  lua_pushinteger(L, s->st_birthtim.tv_sec);
  lua_setfield(L, -2, "birthtime");

  lua_pushinteger(L, s->st_mode & S_IFMT);
  lua_setfield(L, -2, "type");
}

static void luaio_fs_callback(uv_fs_t *req) {
  luaio_fs_req_t *fs_req = container_of(req, luaio_fs_req_t, req);
  lua_State *L = fs_req->current_thread;
  int result = req->result;

  switch (req->fs_type) {
    case UV_FS_ACCESS:
    case UV_FS_CLOSE:
    case UV_FS_RENAME:
    case UV_FS_UNLINK:
    case UV_FS_RMDIR:
    case UV_FS_MKDIR:
    case UV_FS_FTRUNCATE:
    case UV_FS_FSYNC:
    case UV_FS_FDATASYNC:
    case UV_FS_LINK:
    case UV_FS_SYMLINK:
    case UV_FS_CHMOD:
    case UV_FS_FCHMOD:
    case UV_FS_CHOWN:
    case UV_FS_FCHOWN:
    case UV_FS_UTIME:
    case UV_FS_FUTIME:
    case UV_FS_OPEN:
    case UV_FS_SENDFILE:
      lua_pushinteger(L, result);
      luaio_pfree(fs_req);
      luaio_resume(L, 1);
      return;

    case UV_FS_READ:
      if (result > 0) {
        fs_req->read_buffer->write_pos += result;
      }

      lua_pushinteger(L, result);
      luaio_pfree(fs_req);
      luaio_resume(L, 1);
      return;

    case UV_FS_WRITE:
      if (result < 0) {
        lua_pushinteger(L, result);
      } else {
        lua_pushinteger(L, fs_req->bytes);
      }

      luaL_unref(L, LUA_REGISTRYINDEX, fs_req->write_data_ref);
      luaio_pfree(fs_req);
      luaio_resume(L, 1);
      return;

    case UV_FS_STAT:
    case UV_FS_LSTAT:
    case UV_FS_FSTAT:
      if (result < 0) {
        lua_pushnil(L);
        lua_pushinteger(L, result);
      } else {
        luaio_fs_create_stat(L, &req->statbuf);
        lua_pushinteger(L, 0);
      }

      luaio_pfree(fs_req);
      luaio_resume(L, 2);
      return;

    case UV_FS_MKDTEMP:
      if (result < 0) {
        lua_pushnil(L);
        lua_pushinteger(L, result);
      } else {
        lua_pushstring(L, req->path);
        lua_pushinteger(L, 0);
      }

      luaio_pfree(fs_req);
      luaio_resume(L, 2);
      return;

    case UV_FS_READLINK:
      if (result < 0) {
        lua_pushnil(L);
        lua_pushinteger(L, result);
      } else {
        lua_pushstring(L, (char*)req->ptr);
        lua_pushinteger(L, 0);
      }

      luaio_pfree(fs_req);
      luaio_resume(L, 2);
      return;

    case UV_FS_SCANDIR:
      if (result < 0) {
        lua_pushnil(L);
        lua_pushinteger(L, result);
      } else {
        int ret;
        lua_createtable(L, 0, 0);

        for (int i = 1; ; i++) {
          uv_dirent_t ent;
          ret = uv_fs_scandir_next(req, &ent);
          if (ret) break;

          lua_pushstring(L, ent.name);
          lua_rawseti(L, -2, i);
        }

        if (ret && ret != UV_EOF) {
          lua_pop(L, 1);
          lua_pushnil(L);
          lua_pushinteger(L, ret);
        } else {
          lua_pushinteger(L, 0);
        }
      }

      luaio_pfree(fs_req);
      luaio_resume(L, 2);
      return;

    default:
      luaL_error(L, "fs_native module error: unknown fs type(%d)", req->fs_type); 
  }
}

/* local ret = fs.access(path, mode)
 * ret < 0 => errno
 * ret == 0 => ok
 */
static int luaio_fs_access(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);

  int mode;
  size_t len;
  const char *amode;
  if (lua_isinteger(L, 2)) {
    mode = lua_tointeger(L, 2);
  } else if (lua_isstring(L, 2)) {
    amode = lua_tolstring(L, 2, &len);
    mode = luaio_fs_parse_access_mode(L, amode, len); 
  } else {
    return luaL_argerror(L, 2, "fs_native.access(path, mode) error: mode must be [string|integer]\n"); 
  }
 
  CREATE_REQ1();
  FS_CALL1(access, req, path, mode);
}

/* local ret = fs.open(path, flags, mode)
 * ret < 0 => errno
 * ret >= 0 => fd
 */
static int luaio_fs_open(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);

  size_t len;
  const char *flags = luaL_checklstring(L, 2, &len);
  int flag = luaio_fs_parse_open_flags(L, flags, len);
  int mode = luaL_checkinteger(L, 3);

  CREATE_REQ1();
  FS_CALL1(open, req, path, flag, mode);
}

/*local err = fs.close(fd)*/
static int luaio_fs_close(lua_State *L) {
  int fd = luaL_checkinteger(L, 1);
  CREATE_REQ1();
  FS_CALL1(close, req, fd);
}

/* local ret = fs.read(fd, buffer, pos)
 * ret < 0 => errno
 * ret >= 0 => read bytes
 */
static int luaio_fs_read(lua_State *L) {
  int fd = luaL_checkinteger(L, 1);

  luaio_buffer_t *buffer = lua_touserdata(L, 2);
  if (buffer == NULL || buffer->type != LUAIO_TYPE_READ_BUFFER) {
    return luaL_argerror(L, 2, "fs_native.read(fd, buffer, pos) error: buffer must be [read_buffer]\n");
  }

  int pos = luaL_checkinteger(L, 3);

  CREATE_REQ1();
  if (buffer->capacity == 0) {
    size_t buffer_size = buffer->size;
    char *start = luaio_palloc(buffer_size);
    size_t capacity = luaio_pmemory_size(start, buffer_size);
    luaio_pmemory_set_used(start, capacity);
    if (start == NULL) {
      luaio_pfree(req);
      lua_pushinteger(L, UV_ENOMEM);
      return 1;
    }

    buffer->capacity = capacity;
    buffer->start = start;
    buffer->read_pos = start;
    /*buffer->parse_pos = start;*/
    buffer->write_pos = start;
    buffer->end = start + capacity;
  }

  uv_buf_t buf;
  char *write_pos = buffer->write_pos;
  buf.base = write_pos;
  buf.len = buffer->end - write_pos;
  req->read_buffer = buffer;

  FS_CALL1(read, req, fd, &buf, 1, pos);
}

/* local ret = fs.write(fd, data, pos)
 * ret < 0 => errno
 * ret >= 0 => write bytes
 */
static int luaio_fs_write(lua_State *L) {
  int fd = luaL_checkinteger(L, 1);
  /*common.h*/
  luaio_check_data(L, 2, fs_native:write(fd, data, pos));
  int pos = luaL_checkinteger(L, 3);

  CREATE_REQ1();
  req->bytes = bytes;
  int err = uv_fs_write(uv_default_loop(), 
                        &req->req,
                        fd,
                        bufs,
                        count,
                        pos,
                        luaio_fs_callback);
  if (err) {
    if(tmp != NULL) {
      luaio_pfree(tmp);
    }

    luaio_pfree(req);
    lua_pushinteger(L, err);
    return 1;
  }

  lua_pushvalue(L, 2);
  req->write_data_ref = luaL_ref(L, LUA_REGISTRYINDEX);
  if(tmp != NULL) {
    luaio_pfree(tmp);
  }

  return lua_yield(L, 0);
}

/*local err = fs.unlink(path)*/
static int luaio_fs_unlink(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  CREATE_REQ1();
  FS_CALL1(unlink, req, path);
}

/*local err = fs.rename(path, newpath)*/
static int luaio_fs_rename(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  const char *newpath = luaL_checkstring(L, 2);
  CREATE_REQ1();
  FS_CALL1(rename, req, path, newpath);
}

/*local err = fs.rmdir(path)*/
static int luaio_fs_rmdir(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  CREATE_REQ1();
  FS_CALL1(rmdir, req, path);
}

/*local err = fs.mkdir(path, mode)*/
static int luaio_fs_mkdir(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  int mode = luaL_checkinteger(L, 2);
  CREATE_REQ1();
  FS_CALL1(mkdir, req, path, mode);
}

/*local err = fs.mkdtemp(temp)*/
static int luaio_fs_mkdtemp(lua_State *L) {
  const char *temp = luaL_checkstring(L, 1);
  CREATE_REQ2();
  FS_CALL2(mkdtemp, req, temp);
}

/*local files, err = fs.readdir(path)*/
static int luaio_fs_readdir(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  CREATE_REQ2();
  FS_CALL2(scandir, req, path, 0);
}

/*local err = fs.sendfile(outfd, infd, offset, length)*/
static int luaio_fs_sendfile(lua_State *L) {
  int outfd = luaL_checkinteger(L, 1);
  int infd = luaL_checkinteger(L, 2);
  int offset = luaL_checkinteger(L, 3);
  int length = luaL_checkinteger(L, 4);
  CREATE_REQ1();
  FS_CALL1(sendfile, req, outfd, infd, offset, length);
}

/*local stat, err = fs.stat(path)*/
static int luaio_fs_stat(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  CREATE_REQ2();
  FS_CALL2(stat, req, path);
}

/*local stat, err = fs.fstat(fd)*/
static int luaio_fs_fstat(lua_State *L) {
  int fd = luaL_checkinteger(L, 1);
  CREATE_REQ2();
  FS_CALL2(fstat, req, fd);
}

/*local stat, err = fs.lstat(path)*/
static int luaio_fs_lstat(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  CREATE_REQ2();
  FS_CALL2(lstat, req, path);
}

/*local err = fs.link(path, newpath)*/
static int luaio_fs_link(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  const char *newpath = luaL_checkstring(L, 2);
  CREATE_REQ1();
  FS_CALL1(link, req, path, newpath);
}

/*local err = fs.symlink(path, newpath, flags)*/
static int luaio_fs_symlink(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  const char *newpath = luaL_checkstring(L, 2);

  int flags = 0;
  if (lua_type(L, 3) == LUA_TTABLE) {
    lua_getfield(L, 3, "dir");
    if (lua_toboolean(L, -1)) flags |= UV_FS_SYMLINK_DIR;
    lua_pop(L, 1);
    lua_getfield(L, 3, "junction");
    if (lua_toboolean(L, -1)) flags |= UV_FS_SYMLINK_JUNCTION;
    lua_pop(L, 1);
  }

  CREATE_REQ1();
  FS_CALL1(symlink, req, path, newpath, flags);
}

/*local link, err = fs.readlink(path)*/
static int luaio_fs_readlink(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  CREATE_REQ2();
  FS_CALL2(readlink, req, path);
}

/*local err = fs.chmod(path, mode)*/
static int luaio_fs_chmod(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  int mode = luaL_checkinteger(L, 2);
  CREATE_REQ1();
  FS_CALL1(chmod, req, path, mode);
}

/*local err = fs.fchmod(fd, mode)*/
static int luaio_fs_fchmod(lua_State *L) {
  int fd = luaL_checkinteger(L, 1);
  int mode = luaL_checkinteger(L, 2);
  CREATE_REQ1();
  FS_CALL1(fchmod, req, fd, mode);
}

/*local err = fs.chown(path, uid, gid)*/
static int luaio_fs_chown(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  uv_uid_t uid = luaL_checkinteger(L, 2);
  uv_gid_t gid = luaL_checkinteger(L, 3);
  CREATE_REQ1();
  FS_CALL1(chown, req, path, uid, gid);
}

/*local err = fs.fchown(fd, uid, gid)*/
static int luaio_fs_fchown(lua_State *L) {
  int fd = luaL_checkinteger(L, 1);
  uv_uid_t uid = luaL_checkinteger(L, 2);
  uv_gid_t gid = luaL_checkinteger(L, 3);
  CREATE_REQ1();
  FS_CALL1(fchown, req, fd, uid, gid);
}

/*local err = fs.utime(path, atime, mtime) */
static int luaio_fs_utime(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  time_t atime = luaL_checkinteger(L, 2);
  time_t mtime = luaL_checkinteger(L, 3);
  CREATE_REQ1();
  FS_CALL1(utime, req, path, atime, mtime);
}

/*local err = fs.futime(fd, atime, mtime) */
static int luaio_fs_futime(lua_State *L) {
  int fd = luaL_checkinteger(L, 1);
  time_t atime = luaL_checkinteger(L, 2);
  time_t mtime = luaL_checkinteger(L, 3);
  CREATE_REQ1();
  FS_CALL1(futime, req, fd, atime, mtime);
}

/*local err = fs.ftruncate(fd, length)*/
static int luaio_fs_ftruncate(lua_State *L) {
  int fd = luaL_checkinteger(L, 1);
  int64_t length = luaL_checkinteger(L, 2);
  CREATE_REQ1();
  FS_CALL1(ftruncate, req, fd, length);
}

/*local err = fs.fsync(fd)*/
static int luaio_fs_fsync(lua_State *L) {
  int fd = luaL_checkinteger(L, 1);
  CREATE_REQ1();
  FS_CALL1(fsync, req, fd);
}

/*local err = fs.fdatasync(fd)*/
static int luaio_fs_fdatasync(lua_State *L) {
  int fd = luaL_checkinteger(L, 1);
  CREATE_REQ1();
  FS_CALL1(fdatasync, req, fd);
}

static void luaio_fs_setup_constants(lua_State *L) {
  luaio_setinteger("FILE", S_IFREG)
  luaio_setinteger("DIR", S_IFDIR)
  luaio_setinteger("LINK", S_IFLNK)
  luaio_setinteger("FIFO", S_IFIFO)
  luaio_setinteger("SOCKET", S_IFSOCK)
  luaio_setinteger("CHAR", S_IFCHR)
  luaio_setinteger("BLOCK", S_IFBLK)
}

int luaopen_fs(lua_State *L) {
  luaL_Reg lib[] = {
    { "access", luaio_fs_access },
    { "open", luaio_fs_open },
    { "close", luaio_fs_close },
    { "read", luaio_fs_read },
    { "write", luaio_fs_write },
    { "unlink", luaio_fs_unlink },
    { "rename", luaio_fs_rename },
    { "rmdir", luaio_fs_rmdir },
    { "mkdir", luaio_fs_mkdir },
    { "mkdtemp", luaio_fs_mkdtemp },
    { "readdir", luaio_fs_readdir },
    { "sendfile", luaio_fs_sendfile },
    { "stat", luaio_fs_stat },
    { "lstat", luaio_fs_lstat },
    { "fstat", luaio_fs_fstat },
    { "link", luaio_fs_link },
    { "symlink", luaio_fs_symlink },
    { "readlink", luaio_fs_readlink },
    { "chmod", luaio_fs_chmod }, 
    { "fchmod", luaio_fs_fchmod },
    { "chown", luaio_fs_chown },
    { "fchown", luaio_fs_fchown },
    { "utime", luaio_fs_utime },
    { "futime", luaio_fs_futime },
    { "ftruncate", luaio_fs_ftruncate },
    { "fsync", luaio_fs_fsync },
    { "fdatasync", luaio_fs_fdatasync },
    { "__newindex", luaio_cannot_change },
    { NULL, NULL }
  };

  lua_createtable(L, 0, 0);

  luaL_newlib(L, lib);
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  lua_pushliteral(L, "metatable is protected.");
  lua_setfield(L, -2, "__metatable");
  luaio_fs_setup_constants(L);

  lua_setmetatable(L, -2);

  return 1;
}
