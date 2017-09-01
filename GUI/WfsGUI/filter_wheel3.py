#!/usr/bin/env python


from simplemotor_gui import *

class filter_wheel3( simplemotor_gui):

    def __init__(self, AOwindow):
        self.prefix = "filterwheel3"
        self.name = "Filter wheel 3"
        self.unit = "filters"
        simplemotor_gui.__init__(self, AOwindow)
                                             
