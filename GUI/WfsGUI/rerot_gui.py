#!/usr/bin/env python


from simplemotor_gui import *

class rerot_gui( simplemotor_gui):

    def __init__(self, AOwindow):
        self.prefix = "rerotator"
        self.name = "Pupil rerotator"
        self.unit = "degrees"
        simplemotor_gui.__init__(self, AOwindow)
                                             
