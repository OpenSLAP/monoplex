import os
import re

Import('global_env')
env = global_env.Clone()

# we hold ourselves to a higher standard ;)
env.Append(CFLAGS = env['STRICTFLAGS'])
env.Append(CXXFLAGS = env['STRICTFLAGS'])


sources = ['monoplex.cpp']
lib = env.StaticLibrary(env['PROJ_NAME'], [env.Object(s) for s in sources])

Return('lib')
