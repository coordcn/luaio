{
  'targets': [
    {
      'target_name': 'liblua',
      'type': 'static_library',
      'sources': [
        'src/lapi.c',
        'src/lauxlib.c',
        'src/lbaselib.c',
        'src/lbitlib.c',
        'src/lcode.c',
        'src/lcorolib.c',
        'src/lctype.c',
        'src/ldblib.c',
        'src/ldebug.c',
        'src/ldo.c',
        'src/ldump.c',
        'src/lfunc.c',
        'src/lgc.c',
        'src/linit.c',
        'src/liolib.c',
        'src/llex.c',
        'src/lmathlib.c',
        'src/lmem.c',
        'src/loadlib.c',
        'src/lobject.c',
        'src/lopcodes.c',
        'src/loslib.c',
        'src/lparser.c',
        'src/lstate.c',
        'src/lstring.c',
        'src/lstrlib.c',
        'src/ltable.c',
        'src/ltablib.c',
        'src/ltm.c',
        'src/lundump.c',
        'src/lutf8lib.c',
        'src/lvm.c',
        'src/lzio.c',
      ],
      'include_dirs': [
        'src',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          'src',
        ],
      },
    },
    {
      'target_name': 'lua',
      'type': 'executable',
      'sources': [
        'src/lua.c'
      ],
      'dependencies': [
        'liblua',
      ],
      'conditions': [
        ['OS == "linux"', {
          'defines': [ 'LUA_USE_DLOPEN'],
          'ldflags': ['-ldl'],
        }]
      ]
    },
    {
      'target_name': 'luac',
      'type': 'executable',
      'sources': [
        'src/luac.c',
      ],
      'dependencies': [
        'liblua',
      ],
    },
  ],
}