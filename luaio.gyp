{
  'targets': [
    {
      'target_name': 'libluaio',
      'type': 'static_library',
      'dependencies': [
        #'deps/lua/lua.gyp:liblua',
        'deps/cares/cares.gyp:cares',
        'deps/luajit/luajit.gyp:luajit',
        'deps/luajit/luajit.gyp:libluajit',
        'deps/uv/uv.gyp:libuv',
        'deps/zlib/zlib.gyp:zlib',
        'deps/openssl/openssl.gyp:openssl',
      ],
      'export_dependent_settings': [
        #'deps/lua/lua.gyp:liblua',
        'deps/luajit/luajit.gyp:luajit',
        'deps/luajit/luajit.gyp:libluajit',
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
        'src/luaio_buffer.c',
        'src/luaio_date.c',
        'src/luaio_dns.c',
        'src/luaio_errno.c',
        'src/luaio_fs.c',
        'src/luaio_hash.c',
        'src/luaio_http.c',
        'src/luaio_http_parser.c',
        'src/luaio_init.c',
        'src/luaio_pmemory.c',
        'src/luaio_process.c',
        'src/luaio_read_buffer.c',
        'src/luaio_setaffinity.c',
        'src/luaio_signal.c',
        'src/luaio_strlib.c',
        'src/luaio_string.c',
        'src/luaio_system.c',
        'src/luaio_tcp.c',
        'src/luaio_timer.c',
        'src/luaio_util.c',
        'src/luaio_write_buffer.c',
      ],
     'rules': [
       {
         'rule_name': 'jit_lua',
         'extension': 'lua',
         'outputs': [
           '<(SHARED_INTERMEDIATE_DIR)/generated/<(RULE_INPUT_ROOT)_jit.c'
         ],
         'action': [
           '<(PRODUCT_DIR)/luajit',
           '-b', '-g', '<(RULE_INPUT_PATH)',
           '<(SHARED_INTERMEDIATE_DIR)/generated/<(RULE_INPUT_ROOT)_jit.c',
         ],
         'process_outputs_as_sources': 1,
         'message': 'luajit <(RULE_INPUT_PATH)'
       }
     ],
    },
    {
      'target_name': 'luaio',
      'type': 'executable',
      'dependencies': [
        'libluaio',
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
        'src/luaio.c',
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
        'luaio',
      ],
      'copies': [
        {
          'destination': './',
          'files': [
            'out/Debug/luaio',
          ]
        },
      ],
    },
  ],
}
