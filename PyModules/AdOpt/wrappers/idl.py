#######################################################################
#                                                                     #
# Python-IDL, copyright (c) 2002 Andrew McMurry ; all rights reserved #
#                                                                     #
# see the LICENSE file for license details                            #
#                                                                     #
#######################################################################

import _idl
import numpy

## class IDLArray (numarray.NDArray):
##     "Interface to an IDL array"
    
##     def __init__(self, _idlarr):
##         numarray.NDArray.__init__(self, _idlarr.shape(),
##                                   _idlarr.itemsize())
##         self._arr = _idlarr

##     def _getitem(self, key):
##         o = self._getByteOffset(key)
##         return self._arr.getitem(o / self._itemsize)

##     def _setitem(self, key, v):
##         o = self._getByteOffset(key)
##         self._arr.setitem(o / self._itemsize, v)

class IDLArray (numpy.ndarray):
    "Interface to an IDL array"

    pass

def make_IDLArray(_idlarr):
    if hasattr(_idlarr,'dtype'):
        t = _idlarr.dtype
    else:
        t = _idlarr.type()
    self = IDLArray(_idlarr.shape(),
                    _types[t],
                    buffer(_idlarr))
    self._arr = _idlarr
    return self

class IDLReference:
    "Interface to an IDL variable for passing by reference"

    count = 0

    def __init__(self, value = None):
        if value is None:
            self.name = '_Py_Ref_%d' % (self.count,)
            self.count += 1
        elif isinstance(value, IDLArray):
            self.name = value._arr.name()
        elif isinstance(value, numpy.ndarray):
            self.idla = _idl._IDLArray(value._data, _types[value._type],
                                       value._shape)
            self.name = self.idla.name()
        elif isinstance(value, IDLReference):
            self.name = value.name
        else:
            self.name = '_Py_Ref_%d' % (self.count,)
            self.count += 1
            _idl.ex('%s = %s' % (self.name, repr(value)))

    def __call__(self):
        return _idl.getvar(self.name)

class IDLVariables:
    "Interface to IDL variables"

    def __init__(self):
        global idlarraytype
        _idl.ex('_Py_FnReturn = [1,2]')
        idlarraytype = type(_idl.getvar('_Py_FnReturn'))

    def __getattr__(self, name):
        v = _idl.getvar(name)
        if type(v) is idlarraytype:
            return make_IDLArray(v)
        return v

    def __setattr__(self, name, v):
        _idl.setvar(name, v)

class IDLProc:
    def __init__(self, name, func = 0):
        self.name = name
        self.func = func

    def arg_to_string(self, arg):
        if isinstance(arg, IDLArray):
            return arg._arr.name()
        elif isinstance(arg, numpy.ndarray):
            na = _idl._IDLArray(arg._data, _types[arg._type], arg._shape)
            self.reserve.append(na)
            return na.name()
        elif isinstance(arg, IDLReference):
            return arg.name
        return repr(arg)

    def make_call_string(self, args, kwargs):
        argv = [self.name]
        for arg in args:
            argv.append(self.arg_to_string(arg))
        for (key,arg) in kwargs.iteritems():
            argv.append(key+"="+self.arg_to_string(arg))
        if self.func:
            cmd = "_Py_FnReturn = "+self.name+"("+", ".join(argv[1:])+")"
        else:
            cmd = ", ".join(argv)
        return cmd

    def __call__(self, *args, **kwargs):
        self.reserve = []
        _idl.ex(self.make_call_string(args,kwargs))
        self.reserve = []
        if self.func:
            return var._Py_FnReturn

class IDLFunctions:
    "Interface to IDL functions"
    def __getattr__(self, name):
        return IDLProc(name, 1)

class IDLProcedures:
    "Interface to IDL procedures"
    def __getattr__(self, name):
        return IDLProc(name)

_types = _idl.gettypes(numpy)
var = IDLVariables()
ex = _idl.ex
pro = IDLProcedures()
fn = IDLFunctions()
idlprint = IDLProc('print')
