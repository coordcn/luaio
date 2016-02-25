{
  'targets': [
    {
      'target_name': 'libLuaIO',
      'type': 'static_library',
      'dependencies': [
        'deps/lua/lua.gyp:liblua',
        'deps/cares/cares.gyp:cares',
        'deps/uv/uv.gyp:libuv',
        'deps/zlib/zlib.gyp:zlib',
        'deps/openssl/openssl.gyp:openssl',
      ],
      'export_dependent_settings': [
        'deps/lua/lua.gyp:liblua',
        'deps/cares/cares.gyp:cares',
        'deps/uv/uv.gyp:libuv',
        'deps/zlib/zlib.gyp:zlib',
        'deps/openssl/openssl.gyp:openssl',
      ],
      'conditions': [
        ['OS=="linux" or OS=="freebsd" or OS=="openbsd" or OS=="solaris"', {
          'cflags': [ '--std=c99' ],
          'defines': [ '_GNU_SOURCE' ]
        }]
      ],
      'include_dirs': [
        'src',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          'src',
        ]
      },
      'sources': [
        'src/alloc.c',
        'src/palloc.c',
        'src/pmemory.c',
        'src/setaffinity.c',
        'src/timer.c',
        'src/hash.c',
        'src/common.c',
        'src/http_parser.c',
        'src/init.c',
        'src/errno.c',
        'src/system.c',
        'src/signal.c',
        'src/process.c',
        'src/date.c',
        'src/buffer.c',
        'src/read_buffer.c',
        'src/write_buffer.c',
        'src/dns.c',
        'src/tcp.c',
        'src/http.c',
        'src/fs.c',
      ],
    },
    {
      'target_name': 'LuaIO',
      'type': 'executable',
      'dependencies': [
        'libLuaIO',
      ],
      'include_dirs': [
        'src',
      ],
      'direct_dependent_settings': {
       'include_dirs': [
         'src',
       ]
      },
      'sources': [
        'src/main.c',
      ],
      'msvs-settings': {
        'VCLinkerTool': {
          'SubSystem': 1, # /subsystem:console
        },
      },
      'conditions': [
        ['OS == "win"', {
          'libraries': [
            '-lgdi32.lib',
            '-luser32.lib'
          ],
        }],
        ['OS == "linux"', {
          'libraries': [ '-ldl' ],
        }],
        ['OS=="linux" or OS=="freebsd" or OS=="openbsd" or OS=="solaris"', {
          'cflags': [ '--std=c99' ],
          'defines': [ '_GNU_SOURCE' ]
        }],
      ],
      'defines': [ 'BUNDLE=1' ]
    },
    {
      'target_name': 'copy',
      'type': 'none',
      'dependencies': [
        'LuaIO',
      ],
      'copies': [
        {
          'destination': 'test/',
          'files': [
            'out/Debug/LuaIO',
            'lib/',
          ]
        },
        {
          'destination': 'example/',
          'files': [
            'out/Debug/LuaIO',
            'lib/',
          ]
        },
      ],
    },
  ],
}
