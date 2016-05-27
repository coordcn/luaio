return {
  name = 'redis',
  description = 'redis',
  keywords = {'redis', 'luaio'},
  version = '0.1.0',
  license = 'MIT license',
  package_servers = {
    'localhost',
    'http://third_party.org'
  },
  maintainers = {
    {
      name = 'coordcn',
      email = 'coordcn@163.com'
    },
    {
      name = 'someone',
      email = 'someone@any.com'
    }
  },
  repository = {
  },
  bugs = {
  },
  deps = {
    lib1: '1',    -- < 2.0.0  nil -> max
    lib2: '1.0',  -- < 1.1.0  minor == 1 => developing version
    lib3: '1.0.2' -- = 1.0.2
  },
  scripts = {
  }
}
