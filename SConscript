# RT-Thread building script for bridge

import os
from building import *

cwd = GetCurrentDir()
path = [cwd]
objs = []

# Verify package need
if not GetDepend('PKG_USING_NRF24L01'):
    Return('objs')


# Source
src = []
src	= src + Glob('core/src/*.c')
src = src + Glob('platform/depimpl/*.c')
incdirs = [
    'core/src', 
    'platform', 
    'platform/depimpl', 
]
    

group = DefineGroup('nRF24L01', src, depend = [''], CPPDEFINES=['NRF24L01_ENABLE_EXT_SOURCE'],  CPPPATH = [os.path.join(cwd, x) for x in incdirs])
objs = objs + group

# Cmd
objs = objs + DefineGroup('nRF24L01', Glob('platform/cmd/*.c'), depend = ['PKG_NRF24L01_ENABLE_SHELL_CMD'], CPPPATH = [os.path.join(cwd, x) for x in ['core/utils/cmd/incc']])

# Demo
objs = objs + SConscript(os.path.join(os.path.join(cwd, 'demo'), 'SConscript'))

Return('objs')