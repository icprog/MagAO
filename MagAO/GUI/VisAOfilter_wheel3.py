#!/usr/bin/env python


from VisAOsimplemotor_gui import *

class VisAOfilter_wheel3( VisAOsimplemotor_gui):

    def __init__(self, AOwindow):
        self.prefix = "filterwheel3"
        self.name = "Filter wheel 3"
        self.unit = "filters"
        VisAOsimplemotor_gui.__init__(self, AOwindow)
                                             
