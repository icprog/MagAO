#!/usr/bin/env python
#
#+File: util_processes.py
#
# AO utilities process control. Starts, stops and restarts AO-related processes.
#

import sys


### DEPRECATED ###
print 'ATTENTION: util_processes.py use is deprecated! Use startAO.py instead'
sys.exit()


### OLD CODE ###
import os, sys
from qt import *

from AOprocess import *

base_dir = os.environ['ADOPT_ROOT']+"/"


# Start application

app = QApplication( sys.argv)

main = AOprocess( multi=1, rowSize=20, caption = "AO software")

main.addTitle("<b>PROCEDURES</b>")
main.addSeparator()

for k in cfg.procedures:
    main.addProcess( k)

main.addSeparator()
main.addTitle("<b>INTERFACES</b>")
main.addSeparator()

for k in cfg.interfaces:
    main.addProcess( k)

app.setMainWidget(main)
main.show()

app.exec_loop()
