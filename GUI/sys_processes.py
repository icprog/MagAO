#!/usr/bin/env python
#
#@File: sys_processes.py
#
# AO system process control. Starts, stops and restarts AO-related processes.
#@


import sys
from qt import *
from AdOpt import cfg, AOprocess

# Start application

app = QApplication( sys.argv)

font = QFont("System", 10)
app.setFont(font)

main = AOprocess.AOprocess( app, multi=True, rowSize=20, caption = "System processes", showStatus=True)

main.addTitle("<b>System</b>")
main.addSeparator()

for k in cfg.sys_processes.split():
    main.addProcess( k)
main.addSeparator()

if cfg.subsystem == 'ADSEC':
    main.addTitle("<b>Adsec</b>")
    main.addSeparator()

    for k in cfg.adsec_processes.split():
        main.addProcess( k)

if cfg.subsystem == 'WFS':
    main.addTitle("<b>WFS</b>")
    main.addSeparator()

    for k in cfg.wfs_processes.split():
        main.addProcess( k)

app.setMainWidget(main)
main.show()

app.exec_loop()
