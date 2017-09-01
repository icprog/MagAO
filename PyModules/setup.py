#!/usr/bin/env python

from distutils.core import setup,Extension
import os
import sys

qtinclude = os.getenv('QTDIR')+'/include' 
qtlibs    = os.getenv('QTDIR')+'/lib'

py_ver='python'+sys.version[0:3]

setup( name='AdOpt',
       version='2.0',
       description='AdOpt python modules',
       author='Arcetri',
       packages=['AdOpt','AdOpt.I4D', 'AdOpt.hwctrl', 'AdOpt.wrappers'],
       ext_package='AdOpt.wrappers',
       ext_modules=[ 
#            Extension('reorder', ['AdOpt/wrappers/reorder.cpp', '../lib/base/build_time.c'],
#                       include_dirs = ['../lib'],
#                       extra_objects = ['../lib/Reorder.o', '../lib/Paths.o', '../lib/bcu_diag.o'],
#                       libraries = ['rt', 'stdc++']),
            Extension('diagbuf_wrapper', ['AdOpt/wrappers/diagbuf_wrapper.cpp', '../lib/base/build_time.c', '../lib/bcu_diag.cpp'],
                       extra_objects = ['../lib/diagBuffer.o', '../lib/Paths.o', '../lib/Reorder.o', '../lib/iolib.o', '../lib/Utils.o', '../lib/Logger.o', '../lib/base/thrdlib.a'],
                       include_dirs = ['../lib', '/usr/lib/%s/site-packages/numpy/numarray'%py_ver, '/usr/lib/%s/site-packages/numpy/core/include'%py_ver],
                       libraries = ['rt', 'stdc++', 'cfitsio'],
                       library_dirs = ['../contrib/cfitsio'],
                       define_macros = [('NO_OPTLOOP', 1)]),
            Extension('msglib', ['AdOpt/wrappers/msglib.c', '../lib/base/build_time.c'],
                       include_dirs = ['../lib'],
                       extra_objects = ['../lib/base/msglib.a'],
                       libraries = ['rt']),
            Extension('thrdlib', ['AdOpt/wrappers/thrdlib.c', '../lib/base/build_time.c'],
                       include_dirs = ['../lib'],
                       extra_objects = ['../lib/base/thrdlib.a'],
                       libraries = ['rt']),
            Extension('logger', ['AdOpt/wrappers/logger.cpp', '../lib/base/build_time.c'],
                       include_dirs = ['../lib'],
                       extra_objects = ['../lib/Logger.o', '../lib/base/msglib.a', '../lib/Utils.o'],
                       libraries = ['rt', 'stdc++']),
            Extension('drawlib', ['AdOpt/wrappers/drawlib.cpp', '../lib/base/build_time.c'],
                       include_dirs = ['../lib', qtinclude],
                       extra_objects = ['../lib/drawlib/libdrawlib.a', '../lib/base/msglib.a'],
                       libraries = ['rt', 'stdc++', 'qt-mt'],
                       library_dirs = [qtlibs]),

            ],
    )
