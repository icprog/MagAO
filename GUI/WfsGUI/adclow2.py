#!/usr/bin/env python


from simplemotor_gui import *

class adclow2( simplemotor_gui):

    def __init__(self, AOwindow):
        self.prefix = "adc2"
        self.name = "Adc wheel 2"
	self.unit = "degrees"
        simplemotor_gui.__init__(self, AOwindow)
                                             
