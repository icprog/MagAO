#!/usr/bin/env python
#@File: AOConfig.py
#
# Python library to parse AdOpt configuration files
#
# This library reads text files containing the definition 
# of an array or a structure defined as in IDL routines 
# read_ascii_structure/array.pro.
# 
# The function read_config(filename) return a dictionary. For each item in 
# the input file, the corresponding AOVar is added to the return dictionary, 
# using a key of the form filename.itemname
# 
# Type conversion are: 
#     IDL                               Python               MsgD
#   float, double                       float                real(C double)
#   byte, int, uint, long               int (C long)         int (C long)
#   ulong, long64, ulong64              long                 bit64
#   string                              str                  char
#   complex, dcomplex                   not supported yet    NS
# 
# The dimensions of an array "file.namearray" are written in
# a field named "file.namearray.sz" as a tuple in the format of
# the SIZE function in IDL.
# Example: the array defined in elec.txt as
#
# dummy_act          array  
#        int 3
#        7 23 39
# end
#
# is converted as:
#
# elec.dummy_act.sz               (1, 3, 2, 3)              int
# elec.dummy_act                  (7, 23, 39)               int
# 
#
# History: 
# ????     Alfio Puglisi
# 8/9/2005 Lorenzo Busoni: conforming to file format standard
#                          as described in read_ascii_structure/array.pro
# 1/2/2006 Alfio Puglisi: returning a dictionary instead of a list
# 2/2/2006 LB:            returning a dictionary of AOVar, structname keyword added 
#
# Bugs/TODO:
#  -  fits files not supported yet
#  -  ' and " are not allowed in strings
#

from string import *
from types import *
import os
import re
import AOVar

class Error(Exception):
    """Base class for exceptions in this module."""
    pass

class ParseError(Error):
    """Exception raised for errors in parsing config file.
    Attributes:
    expression -- input expression in which the error occurred
    message -- explanation of the error
    """
    def __init__(self, expression, message):
        self.expression = expression
        self.message = message

def write_config( filename, dataDict, header= None):
    '''
    Writes an AdOpt configuration file.
    Only scalar values are supported. Values may be both AOVars or python scalar values.
    '''

    f = file(filename, 'w')
    if header:
        f.write(header)

    for k in dataDict.keys():

        v = dataDict[k]
        if not isinstance(v, AOVar.AOVar):
            v = AOVar.createVar( k, dataDict[k])
        f.write( v.cfgStr()+'\n')
    f.close()

def read_config(filename, structname=None, add_side = False, useAOVar = True):
    '''
    Read an AdOpt configuration file and return a dictionary of
    configuration variables.
    If structname is defined keys are in the form 'structname.varname',
    otherwise keys are named 'filename.varname'.
    Values are the corresponding AOVar.
    if <useAOVar> is False, values are not converted to AOVars but kept as simple values.
    '''
    expectEnd = False
    
    filename   =  filename.strip()
    if structname == None:
        structname =  ''
        if structname.rfind('.') != -1:
            cutExt = structname.rfind('.') 
        else:
            cutExt = len(filename)
        structname = structname[:cutExt]
    dic={}
    res= {}

    try:
        try:
            lines = file(filename).read().splitlines()
        except IOError:
            # Retry adding $ADOPT_ROOT in front
            adopt_root = os.environ['ADOPT_ROOT']
            if filename[0] != '/':
                adopt_root += '/'
            try:
                lines = file(adopt_root + filename).read().splitlines()
            except IOError, (errno, strerror):
                #print "I/O error(%s): %s - %s" % (errno, strerror, filename)
                #print "(even tried to add $ADOPT_ROOT)"
                raise IOError( errno, 'No such file or directory')

        for l in lines:
            l = skip_comments(l)
            words = l.split()
            if len(words)<1:
                continue
            if words[0].lower() in ['byte', 'int', 'uint', 'long', 'ulong', \
                'long64', 'ulong64', 'float', 'double', 'complex', \
                'dcomplex', 'string']:
                res = parse_array(lines, expectEnd)
	    else:
                 res = parse_structure(lines, expectEnd, filename)
        dump(res,dic, structname, useAOVar)
    except IOError, (errno, strerror):
        #print "I/O error(%s): %s - %s" % (errno, strerror, filename)
        raise
    except:
        raise

    if add_side:
        dic.update( dict({'mySide': createSide(filename)}))

    return dic
    
def read_ascii_structure(filename):
    lines = file(filename).read().splitlines()
    expectEnd = False
    return parse_structure(lines, expectEnd, filename)

def read_ascii_array(filename):
    lines = file(filename).read().splitlines()
    expectEnd = False
    return parse_array(lines, expectEnd)

def parse_structure(lines, expectEnd, filename):
    res = {}

    while len(lines) > 0:
        l = lines.pop(0)
        l = skip_comments(l)

        words = l.split()
        if len(words)<1:
            continue

        if expectEnd==True:
            if lower(words[0]) == 'end':
                return res

        name = words[0]
        tipo = words[1].lower()
        value = ' '.join(words[2:])

        if tipo == "array":
            if len(value) < 1: # in-line array
                res[name] = parse_array(lines, True)

            else :
                if value[0] == '"' or value[0] == '\'' :
                    value = value[1:]
                if value[-1] == '"' or value[-1] == '\'' :
                    value = value[:-1]
		if value[0] == "[":
			# Unsupported in-line array
			pass
		else:
                	res[name] = read_ascii_array(value)
                # TODO: implementare i file fits

        elif tipo == "byte":
            # convert 1B -> 1
            value = replace(value,'b','')
            value = replace(value,'B','')
            res[name] = int(value)

        elif tipo == "uint" or tipo == "int" or tipo == "long":
            value = replace(value,'l','')
            value = replace(value,'L','')
            base = 10
            if value[0] == 'x' or value[0] == 'X':
                value = '0'+value
                base = 16
            res[name] = int(value, base)
        
        elif tipo == "ulong" or tipo == "ulong64" or tipo == "long64":
            value = replace(value,'l','')
            value = replace(value,'L','')
            base = 10
            if value[0] == 'x' or value[0]=='X':
                value = '0'+value
                base = 16
            res[name] = long(value, base)
        
        elif tipo == "float" or tipo == "double" or tipo == "float32":
            # convert 2.5d3 -> 2.5e3
            value = replace(value,'d','e')
            value = replace(value,'D','e')
            res[name] = float(value)

        elif tipo == "complex" or tipo == "dcomplex":
            stringa = 'Unsupported type '+ tipo +' (line: '+l+ ')'
            raise TypeError, (stringa)

        elif tipo == "string":
            if value[0] == '"' or value[0] == '\'' :
                value = value[1:]
            if value[-1] == '"' or value[-1] == '\'' :
                value = value[:-1]
            res[name] = value

        elif tipo == "structure":
            if len(value) < 1: # in-line structure
                res[name] = parse_structure(lines, True, filename)
            else :
                if value[0] == '"' or value[0] == '\'' :
                    value = value[1:]
                if value[-1] == '"' or value[-1] == '\'' :
                    value = value[:-1]

                if os.path.dirname(filename) != '':
                    f = os.path.dirname(filename)+'/'+value
                else:
                    f = './'+value

                res[name] = read_ascii_structure(f)
        else:
            stringa = 'Unsupported type '+ tipo +' (line: '+l+ ')'
            raise TypeError, (stringa)

    if expectEnd==True:
        raise ParseError, (l, 'Unexpected end of file')
    else:
        return res


def parse_array(lines, expectEnd):
    res = []
    size = []
    nowEnd=False
    nowData=False
    nowHead=True
    n_elements=1

    while len(lines) > 0:
        l = lines.pop(0)
        l = skip_comments(l)
        field = l.split()
        if len(field)<1:
            continue
        
        if nowEnd==True:
            if field[0].lower() != "end":
                raise ParseError, (l, 'No end keyword found for in-line array')
            else:
                return tuple(res)
        

        if nowData==True:
            field = re.split('\s*"(.*?)"\s*|\s*',l)
            for value in field:
                if (value == '' or value == None):
                    continue

                if tipo == "uint" or tipo == "int" or tipo == "byte" or tipo == "long":
                    base = 10
                    if value[0] == 'x' or value[0] == 'X':
                        value = '0'+value
                        base = 16
                    res.append(int(value, base))
                    n_elements -= 1
        
                elif tipo == "ulong" or tipo == "ulong64" or tipo == "long64":
                    base = 10
                    if value[0] == 'x' or value[0]=='X':
                        value = '0'+value
                        base = 16
                    res.append(long(value, base))
                    n_elements -= 1
    
                elif tipo == "float" or tipo == "double":
                    # convert 2.5d3 -> 2.5e3
                    value = replace(value,'d','e')
                    value = replace(value,'D','e')
                    res.append(float(value))
                    n_elements -= 1

                elif tipo == "complex" or tipo == "dcomplex":
                    stringa = 'Unsupported type '+ tipo +' (line: '+l+ ')'
                    raise TypeError, (stringa)

                elif tipo == "string":
                    if value[0] == '"' or value[0] == '\'' :
                        value = value[1:]
                    if value[-1] == '"' or value[-1] == '\'' :
                        value = value[:-1]
                    res.append(value)
                    n_elements -= 1
                    
                else:
                    stringa = 'Unsupported type '+ tipo +' (line: '+l+ ')'
                    raise TypeError, (stringa)
                
            if n_elements==0: # all elements read. 
                if expectEnd == True:
                    nowEnd = True  #check next line contains "end"
                    nowData= False
                    continue
                else:
                    return tuple(res)

        if nowHead==True:
            tipo = field.pop(0).lower()
            dimensions=field
            if len(dimensions)<1:
                raise ParseError, (l, 'At least one dimension size must be specified')
            
            size.append(len(dimensions)) 
            for dim in dimensions:
                size.append(int(dim))
                n_elements *= int(dim)
            size.append(str_to_code(tipo))
            size.append(n_elements)
            res.append(tuple(size))
            nowData=True
            nowHead=False
        
    raise ParseError, (l, 'Unexpected end of file')

def str_to_code(tipo):
    if tipo == "uint" or tipo == "int" or tipo == "byte":
        return 2
    elif tipo == "ulong" or tipo == "long" or \
         tipo == "ulong64" or tipo == "long64":
        return 3
    elif tipo == "float" or tipo == "double":
        return 5
    elif tipo == "string":
        return 7
    else:
        return -1

def skip_comments(l):
    pos = l.find(';')
    if pos >=0:
        l = l[0:pos]
    pos = l.find('#')
    if pos >=0:
        l = l[0:pos]
    return l

def dump(res, dic, basename='', useAOVar = True):
    if type(res) == TupleType:
        size = res[0]
        res = res[1:]
        if useAOVar:
            dic[basename+'.sz'] = Conv2AOVar({'name':basename+'.sz','value':size, 'type':get_rtdb_type(size[0])})
            dic[basename] = Conv2AOVar({'name':basename, 'value':res, 'type':get_rtdb_type(res[0])})
        else:
            dic[basename+'.sz'] = size
            dic[basename] = res
    else:
        if basename != '':
            basename += '.'
        for name , value in res.iteritems():
            if type(value) == DictType or type(value) == TupleType:
                dump(value, dic, basename+name, useAOVar)
            else:
                if useAOVar:
                    dic[basename+name] = Conv2AOVar({'name':basename+name, 'value':value, 'type':get_rtdb_type(value)})
                else:
                    dic[basename+name] = value

def get_rtdb_type(item):
    if type(item) == FloatType:
        return 'REAL_VARIABLE'
    elif type(item) == IntType: 
        return 'INT_VARIABLE'
    elif type(item) == LongType:
        return 'BIT64_VARIABLE'
    elif type(item) == StringType:
        return 'CHAR_VARIABLE'
    elif type(item) == TupleType:
        return get_rtdb_type(item[0]) 
    else:
        return 'unknown'
        
def Conv2AOVar(item):
#if item['type'] == 'long':
#       errstr = "Conv2AOVar: item %s has type long (IDL long64/ulong64) not supported in rtdb" % (item['name'])
#       raise TypeError, (errstr)
    # convert array of string into a string=str(array)
    if item['type'] == 'CHAR_VARIABLE' and type(item['value']) == TupleType:
        item['value'] = str(item['value'])
    # convert (singleton,) into scalar
    if type(item['value']) == TupleType and len(item['value']) == 1:
        item['value'] = item['value'][0]
    return AOVar.AOVar(item['name'], item['type'], item['value'])

def createSide(fullpath):
    '''
    Returns an AOVar with the right mySide parameter for the specified configuration file
    '''

    if fullpath.find('conf/left') >= 0:
         v = Conv2AOVar({'name':'mySide', 'value':'L', 'type':'CHAR_VARIABLE'})
    elif fullpath.find('conf/right') >= 0:
         v = Conv2AOVar({'name':'mySide', 'value':'R', 'type':'CHAR_VARIABLE'})
    else:
         v = Conv2AOVar({'name':'mySide', 'value':'C', 'type':'CHAR_VARIABLE'})

    return v



if __name__ == "__main__":
    import sys

    if len(sys.argv) == 1 or sys.argv[1] == '-h':
        print """
        read_config.py   L.Busoni, Sep 2005

        Usage: read_config.py input_files
        input_files are conforming to the rules described in
        read_ascii_array/structure.pro in the idl tree of CVS.
        """
        sys.exit(-1)

    for filename in sys.argv[1:]:

        try:
            dic = read_config(filename)
        except IOError, (errno, strerror):
            print "I/O error(%s): %s - %s" % (errno, strerror, filename)
        except ParseError, e:
            print "Parsing error: %s (line: %s)" % (e.message, e.expression)
        except TypeError, e:
            print "Type error: %s" % e
        except:
            print "Unexpected error:", sys.exc_info()[0]
            raise

        for k in dic.keys():
            print str(dic[k][0]).ljust(35), str(dic[k][1]).ljust(25), str(dic[k][2])


