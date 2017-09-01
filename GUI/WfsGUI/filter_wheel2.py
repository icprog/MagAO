#!/usr/bin/env python


from simplemotor_gui import *

class filter_wheel2( simplemotor_gui):

    def __init__(self, AOwindow):
        self.prefix = "filterwheel2"
        self.name = "Filter wheel 2"
        self.unit = "filters"
        simplemotor_gui.__init__(self, AOwindow)
                                             
