#!/usr/bin/env python
#
import types
from AOConstants import AOVarCode

tipo2PyType   = {'INT_VARIABLE':types.IntType, 'REAL_VARIABLE':types.FloatType, 'CHAR_VARIABLE':types.StringType,\
                'BIT8_VARIABLE':types.IntType, 'BIT16_VARIABLE':types.IntType, 'BIT32_VARIABLE': types.LongType,\
                'BIT64_VARIABLE':types.LongType}

tipo2CfgType = {'INT_VARIABLE':'int', 'REAL_VARIABLE': 'float', 'CHAR_VARIABLE':'string'}

class AOVar:
    
    def __init__(self,name='',tipo='INT_VARIABLE',value=0,owner='',mtime=-1,type=None):
        self.SetName(name)
        if type: 
          self.SetType(type)
        else:
          self.SetType(tipo)
        self.SetValue(value)
        self.SetOwner(owner)
        self.SetLastNotifier('')
        self.SetMTime(mtime)
        #self.Check()

    #def Copy(self):
    #    return AOVar(self.name, self.tipo, self.value, self.owner)

    def Copy(self,var):
        self.SetName(var.name)
        self.SetType(var.tipo)
        self.SetValue(var.value)
        self.SetOwner(var.owner)
        self.SetLastNotifier(var.lastNotifier)
        self.SetMTime(var.mtime)

#TODO aggiungere overriding per copia per valore di var f=g 

    def __eq__(self, var):
        if isinstance(var,AOVar) == False:
            return self.value == var
        if self.name == var.name and self.tipo == var.tipo and \
           self.value == var.value and self.owner == var.owner:
            return True
        else:
            return False

    def __ne__(self, var):
        if isinstance(var,AOVar) == False:
            return self.value != var
        if self.name != var.name or self.tipo != var.tipo or \
           self.value != var.value or self.owner != var.owner:
            return True

            return False

    def __ge__(self,var):
        if isinstance(var,AOVar) == False:
            return self.value >= var
        return self.Value() >= var.Value() 

    def __gt__(self,var):
        if isinstance(var,AOVar) == False:
            return self.value > var
        return self.Value() > var.Value()

    def __le__(self,var):
        if isinstance(var,AOVar) == False:
            return self.value <= var
        return self.Value() <= var.Value() 

    def __lt__(self,var):
        if isinstance(var,AOVar) == False:
            return self.value < var
        return self.Value() < var.Value()

    def __len__(self):
        if   type(self.value) == types.TupleType:
            return len(self.value)
        elif self.tipo == 'CHAR_VARIABLE':
            return len(self.value)  
        else:
            return 1

    def __repr__(self):
        stringa = "%25s %10s %3d %15s %4s mtime:%10d" %  \
            (self.name, self.tipo, len(self), self.owner, self.value, self.mtime)
        return stringa 

    def cfgStr(self):
        if self.tipo == 'INT_VARIABLE':
            fmt = '%-30s int    %d'
        elif self.tipo == 'REAL_VARIABLE':
            fmt = '%-30s float  %7.3f'
        elif self.tipo == 'CHAR_VARIABLE':
            fmt = '%-30s string "%s"'
        else:
            return None

        return fmt % (self.name, self.value)

    def SetName(self,name):
        self.name=name;

    def Name(self):
        return self.name
    
    def SetType(self,tipo):
        errstr = 'Unsupported type %s. Allowed types: %s' % (tipo, AOVarCode.keys())
        if tipo not in AOVarCode.keys():
            raise TypeError, (errstr)
        self.tipo=tipo;

    def Type(self):
        return self.tipo
    
    def SetValue(self,value):
        self.value=value;
        self.Check()

    def Value(self):
        return self.value

    def SetOwner(self,owner):
        self.owner=owner or 'PUBLIC'

    def Owner(self):
        return self.owner

    def SetLastNotifier(self,client):
        self.lastNotifier = client

    def LastNotifier(self):
        if self.lastNotifier:
            return self.lastNotifier 

    def SetMTime(self, mtime):
        self.mtime = mtime or -1

    def MTime(self):
        return self.mtime

    def Check(self):
        # if Value is None, is ok: this means that the var is not initialized, but
        # it isn't an error. I want unable SetValue(None) to reset a variable.
        if self.value != None:
            # se tipo disaccorda con value raise ValueError
            if (type(self.value)==types.TupleType) or (type(self.value)==types.ListType):
                for item in self.value:
                    if tipo2PyType[self.tipo] != type(item):
                        errstr = 'Invalid type value %s for declared type %s' % \
                            (type(item), self.tipo)
                        raise ValueError, (errstr)
                    # if bit* check value is in the range
                    if self.tipo == 'BIT8_VARIABLE' and (item<0 or item>255):
                        errstr = 'Invalid value %s for declared type %s' % (str(item), self.tipo)
                        raise ValueError, (errstr)
                    if self.tipo == 'BIT16_VARIABLE' and (item<0 or item>65535):
                        errstr = 'Invalid value %s for declared type %s' % (str(item), self.tipo)
                        raise ValueError, (errstr)
                    if self.tipo == 'BIT32_VARIABLE' and (item<0 or item>4294967295):
                        errstr = 'Invalid value %s for declared type %s' % (str(item), self.tipo)
                        raise ValueError, (errstr)
                    if self.tipo == 'BIT64_VARIABLE' and (item<0 or item>18446744073709551615):
                        errstr = 'Invalid value %s for declared type %s' % (str(item), self.tipo)
                        raise ValueError, (errstr)
                    if self.tipo == 'INT_VARIABLE' and (item<-2147483648 or item>2147483647):
                        errstr = 'Invalid value %s for declared type %s' % (str(item), self.tipo)
                        raise ValueError, (errstr)
            
            else:
                if tipo2PyType[self.tipo] != type(self.value):
                    errstr = 'Invalid type value %s for declared type %s' % \
                        (type(self.value), self.tipo)
                    raise ValueError, (errstr)
                # if bit* check value is in the range
                if self.tipo == 'BIT8_VARIABLE' and (self.value<0 or self.value>255):
                    errstr = 'Invalid value %s for declared type %s' % (str(self.value), self.tipo)
                    raise ValueError, (errstr)
                if self.tipo == 'BIT16_VARIABLE' and (self.value<0 or self.value>65535):
                    errstr = 'Invalid value %s for declared type %s' % (str(self.value), self.tipo)
                    raise ValueError, (errstr)
                if self.tipo == 'BIT32_VARIABLE' and (self.value<0 or self.value>4294967295):
                    errstr = 'Invalid value %s for declared type %s' % (str(self.value), self.tipo)
                    raise ValueError, (errstr)
                if self.tipo == 'BIT64_VARIABLE' and (self.value<0 or self.value>18446744073709551615):
                    errstr = 'Invalid value %s for declared type %s' % (str(self.value), self.tipo)
                    raise ValueError, (errstr)
                if self.tipo == 'INT_VARIABLE' and (self.value<-2147483648 or self.value>2147483647):
                    errstr = 'Invalid value %s for declared type %s' % (str(self.value), self.tipo)
                    raise ValueError, (errstr)
   
        

def AllowedTypes():
    return AOVarCode.keys()


def _mappa(func,value):
    if type(value) == types.TupleType:
        return tuple(map(func,value))
    elif type(value) == types.IntType or \
         type(value) == types.FloatType or \
         type(value) == types.LongType or \
         type(value) == types.StringType:
        return func(value)
    else:
        errstr = 'Invalid type value %s' % (type(value))
        raise ValueError, (errstr)


def AOint(name='',value=0,owner=''):
    return AOVar(name,'INT_VARIABLE',value,owner)

def AOreal(name='',value=0,owner=''):
    return AOVar(name,'REAL_VARIABLE',_mappa(float,value),owner)

def AOchar(name='',value='',owner=''):
    return AOVar(name,'CHAR_VARIABLE',_mappa(str,value),owner)

def AObit8(name='',value=0,owner=''):
    return AOVar(name,'BIT8_VARIABLE',value,owner)

def AObit16(name='',value=0,owner=''):
    return AOVar(name,'BIT16_VARIABLE',value,owner)

def AObit32(name='',value=0,owner=''):
    return AOVar(name,'BIT32_VARIABLE',_mappa(long,value),owner)

def AObit64(name='',value=0,owner=''):
    return AOVar(name,'BIT64_VARIABLE',_mappa(long,value),owner)


def createVar(name, value, owner='PUBLIC'):
    '''
    Creates an AOVar from an ordinary Pyhton value,
    or None if the conversion fails
    Only scalar values are supported.
    '''

    vtype = type(value)
    if vtype == types.IntType:
        return AOVar(name, 'INT_VARIABLE', value, owner)
    if vtype == types.FloatType:
        return AOVar( name, 'REAL_VARIABLE', value, owner)
    if vtype == types.StringType:
        return AOVar( name, 'CHAR_VARIABLE', value, owner)

    return None
 
