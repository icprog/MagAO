#!/usr/bin/env python


from simplemotor_gui import *

class adclow1( simplemotor_gui):

    def __init__(self, AOwindow):
        self.prefix = "adc1"
        self.name = "Adc wheel 1"
	self.unit = "degrees"
        simplemotor_gui.__init__(self, AOwindow)
                                             
