#!/usr/bin/env python


from VisAOsimplemotor_gui import *

class VisAOfilter_wheel2( VisAOsimplemotor_gui):

    def __init__(self, AOwindow):
        self.prefix = "filterwheel2"
        self.name = "Filter wheel 2"
        self.unit = "filters"
        VisAOsimplemotor_gui.__init__(self, AOwindow)
                                             
