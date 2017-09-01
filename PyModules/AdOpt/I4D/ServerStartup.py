# Pyro I4D Server startup script
# Put this script - and ONLY this - in the interferometer PC, C:/4D/scripts directory

import sys

# Add the I4D Pyro Server package location to path
# Note that the folder "I4D" is used as package
sys.path.append('C:\Obelix_CVS\Supervisor\PyModules')

# Create athe Server
from I4D import Server
I4DServer = Server.Server()

# Start the Server
print 'Starting I4D Pyro Server Daemon...'
I4DServer.start()
print 'I4D Pyro Server Daemon started !'


# Test the Controller
from I4D import Controller
print 'Testing I4D Controller...'
controller = Controller.Controller()
controller.testAcquire()
print 'I4D Controller tested!'