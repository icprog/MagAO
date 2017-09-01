# Pyro I4D Server startup script
# Put this script - and ONLY this - in the 4D/scripts directory

import sys
import time

# Add the I4D Pyro Server package location to path
sys.path.append('C:\Obelix_CVS\Supervisor\PyModules')


from I4D import Server
    
def start():
    print 'Starting I4D Pyro Server Daemon...'
    I4DServer = Server.Server()
    I4DServer.start()
    print 'I4D Pyro Server Daemon started !'


# Start the main method
start()
