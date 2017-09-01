#!/usr/bin/env python

from distutils.core import setup,Extension
import os

qtinclude = os.getenv('QTDIR')+'/include'
qtlibs    = os.getenv('QTDIR')+'/lib'
qwtinclude = os.getenv('QWTDIR')+'/include'
qwtlib    = os.getenv('QWTDIR')+'/lib/libqwt.so'
qwtlibs    = os.getenv('QWTLIB')

setup( name='AdOpt',
       version='2.0',
       description='AdOpt python modules',
       author='Arcetri',
       package_dir = {'AdOpt': '.',
		      'AdOpt.WfsGUI': 'WfsGUI',
                      'AdOpt.QtDesigner': 'QtDesigner',
                      'AdOpt.widgets': 'widgets'},
       packages=['AdOpt.WfsGUI', 'AdOpt.QtDesigner', 'AdOpt.widgets'],
       py_modules = ['AdOpt.AOprocess'],

       ext_package='AdOpt.WfsGUI',
       ext_modules=[

#            Extension('pixelview2', ['WfsGUI/pixelview2.cpp', '../lib/base/build_time.c'],
#                      include_dirs = ['../lib', qtinclude],
#                      extra_objects = ['../lib/base/thrdlib.a', '../lib/Reorder.o', '../lib/Paths.o', '../lib/bcu_diag.o', '../lib/diagBuffer.o', '../lib/iolib.o', '../contrib/cfitsio/libcfitsio.a'],
#                      library_dirs = [qtlibs],
#                      libraries = ['rt', 'stdc++', 'qt-mt']),

           Extension('pixelview', ['WfsGUI/pixelview.cpp', '../lib/base/build_time.c'],
                      include_dirs = ['../lib', qtinclude, qwtinclude],
                      extra_objects = ['../lib/drawlib/libdrawlib.a', '../lib/Reorder.o', '../lib/Paths.o', '../lib/bcu_diag.o', '../lib/diagBuffer.o', '../lib/iolib.o', '../lib/Utils.o', '../contrib/cfitsio/libcfitsio.a', '../lib/Logger.o', '../lib/base/thrdlib.a', qwtlib],
                      library_dirs = [qtlibs],
                      libraries = ['rt', 'stdc++', 'qt-mt']),

       # package_data = {'AdOpt.WfsGUI' : ['images/*.png']}     -- needs python >= 2.4
     ])
