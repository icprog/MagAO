#!/usr/bin/env python


from simplemotor_gui import *

class filter_wheel1( simplemotor_gui):

    def __init__(self, AOwindow):
        self.prefix = "filterwheel1"
        self.name = "Filter wheel 1"
        self.unit = "filters"
        simplemotor_gui.__init__(self, AOwindow)
                                             
