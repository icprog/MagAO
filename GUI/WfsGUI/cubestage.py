#!/usr/bin/env python


from simplemotor_gui import *

class cubestage( simplemotor_gui):

    def __init__(self, AOwindow):
        self.prefix = "cubestage"
        self.name = "Cube stage"
        self.unit = "mm"
        simplemotor_gui.__init__(self, AOwindow)
                                             
