#!/usr/bin/env python


from simplemotor_gui import *

class mercury( simplemotor_gui):

    def __init__(self, AOwindow):
        self.prefix = "mercury"
        self.name = "Cube rotator"
	self.unit = "degrees"
        simplemotor_gui.__init__(self, AOwindow)
                                             
