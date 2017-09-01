#!/usr/bin/env python

import AOConstants

#################################
# Exceptions
################################


class AdOptError(Exception):
    """Base class for exception in an AdOpt application
    Attributes:
        errstr:   (string)  explanation of the error 
        code:     (int)     AO error code
    """
    def __init__(self, code=AOConstants.AOErrCode['GENERIC_SYS_ERROR'],errstr=None):
        self.errstr=errstr
        self.code=code

    def __str__(self):
        if self.errstr == None:
            self.errstr = ''
        stringa = "%s - code: %d %s"%(self.errstr, self.code, AOConstants.AOErrType[- (-self.code & 0x0FFFF)])
        return stringa

class AOTypeError(AdOptError):
    def __init__(self):
        self.message="Illegal data type"

    def __str__(self):
        return self.message


####################
# Var Errors
#
# The main class should extend AdOptError !!!
####################
class AOVarError(AdOptError):
    """Exception raised for error on create/set/get rtdb variables
    Attributes:
        errstr:   (string)  explanation of the error 
        message:  (dict)    message got from MsgD
        var:      (AOVar)   variable on which failed the operation
    """

    def __init__(self, errstr=None, message=None, var=None):
        self.errstr=errstr
        self.message=message
        self.var=var

    def __str__(self):
        stringa = "%s\nmessage: %s\nvar: %s"%(self.errstr, self.message, self.var)
        return stringa

class AOVarNotInRTDB(AOVarError):
    """Exception raised when a variable is not found in the RTDB"""
    def __init__(self, errstr=None, message=None, var=None):
        AOVarError.__init__(self,errstr,message,var)
    def __str__(self):
        return AOVarError.__str__(self)

class AODuplicatedVarError(AOVarError):
    """Exception raised when a variable already present in the RTDB is created"""
    def __init__(self, errstr=None, message=None, var=None):
        AOVarError.__init__(self,errstr,message,var)
    def __str__(self):
        return AOVarError.__str__(self)

#@Class: AORetryException
#
# Exception which signals a temporary error.
#
# Exception for Arbitrator function that are unable to complete
# because of external reasons (e.g. star not found during acquireRef),
# which need a wfs\_RETRY return value.
#@

class AORetryException(AdOptError):

    def __init__(self, code=None, errstr=None):
        self.code = code
        self.errstr=errstr


######################
# Miscell errors
####################

class AOConnectionError(AdOptError):
    """Exception raised for error on connection/communication with MsgD
    Attributes:
        errstr:   (string)  explanation of the error 
        message:  (dict)    message got from MsgD
    """
    def __init__(self, errstr=None, message=None ):
        self.errstr=errstr
        self.message=message

    def __str__(self):
        return repr(self.errstr)

class AOIDLError(AdOptError):
    """Exception raised when an IDL command returns an error
    Attributes:
        errstr:   (string)  explanation of the error 
        idlans:  (dict)    Answer got from IDL
    """
    def __init__(self, errstr=None, idlans=None):
        self.errstr=errstr
        self.idlans=idlans

    def __str__(self):
        return repr(self.errstr)


