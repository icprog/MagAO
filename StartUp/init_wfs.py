#!/usr/bin/env python

from AdOpt import processControl, cfg

def init_wfs(app):
    '''
    Starts the WFS arbitrator and its GUI. Shuts down any previous WFS process.
    '''

    print "Shutting down previous AGW system (if any)"

    # Stop all running processes and wait for proper completion
    for p in ['wfsarb'] + cfg.wfs_processes:
        processControl.stopProcessByName( p, kill=True)

    # Start a brand new arbitrator and its GUI

    processControl.startAndWait( app, 'wfsarb')
    processControl.startProcessByName( 'wfsarbgui', multi=True)

    print "All done"


######################




if __name__ == "__main__":

    from AdOpt import *
    from qt import *
    import sys

    main = thAOApp("STARTUP")

    qApp = QApplication(sys.argv)
    answer = QMessageBox.question(None, 'Startup', 'Restart system? It will shutdown any previous W software running', QMessageBox.Yes, QMessageBox.No)

    if answer == QMessageBox.Yes:
        init_wfs(main)
