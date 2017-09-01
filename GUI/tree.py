#import collections
from __future__ import generators #for python 2.2 
from qt import *

# A Tree class.
class Tree:

    def __init__(self, parent=None):
        self.parent = parent
        self.childs    = []

        if parent:
            self.parent.childs.append(self)

    def __repr__(self, level=0, indent="-----"):
        s = level*indent + `self.label`
        if self.childs:
            for son in self.childs:
                s = s + "\n" + son.__repr__(level+1, indent)
        return s

    def __iter__(self):
        if self:
            yield self
            if self.childs:
                for x in self.childs:
                    for elem in x:
                        yield elem
    
#def __getitem__(self,i):
#        if self:
#            ii = 0
#            while ii < i:
#                item = next(self)
#                ii += i
#        yield item

#    def level(self,ll=0): 
#if self.parent:
#           self.parent.level(ll+1)
#        return ll



## Print the nodes of the tree in in-order.
#t=Board('t')
#at=Board('at', t)
#bt=Board('bt', t)
#cbt=Board('cbt', bt)
#dat=Board('dat', at)
#eat=Board('eat', at)
#feat=Board('feat', eat)

##print at.parent, at.childs
#print repr(t)

#for x in t:
#    print x.parent

