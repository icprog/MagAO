#!/usr/bin/env python
#
# Board setup save/load procedures

from AdOpt import setupDevices

def save( app, filename):
    '''
    Saves a complete board setup into the specified filename.
    The board setup is a series of commands that can be interpreted by a python aoshell
    and can be manually modified, if necessary.
    '''

    app.mute()
    devices = setupDevices.loadDevices(app)
    app.unmute()

    cmdfile = ''

    movements = ['fw1', 'rerot', 'cuberot', 'cubestage', 'stagex', 'stagey', 'stagez']
    devices = ['lamp', 'cameralens', 'adc']#,'tt', 'flowerpot']
    to_wait = []
    saved=[]

    # Filter the list with the devices list - only the valid ones will survive
    #movements = [dev if dev in devices for dev in tosave]

    for device in movements:

        try:
            cmdfile += 'self.%s.moveTo(%f)\n' % (device, app.__dict__[device].getPos())
            to_wait.append(device)
            saved.append(device)
        except Exception, e:
            app.log('Not saving %s status: %s' % (device, str(e)))
            pass

    if 'lamp' in devices:
        try:
            if lamp.isOnline():
            	cmdfile += 'self.lamp.setIntensity(%d)\n' % app.lamp.getIntensity()
            	saved.append('lamp')
            else:
                app.log('Not saving lamp status: flowerpot offline')
        except Exception, e:
            app.log('Not saving lamp status: %s' % str(e))
            pass

    if 'cameralens' in devices:
        try:
            cmdfile += 'self.lens.moveTo(%f,%f)\n' % (app.lens.posx(), app.lens.posy())
            saved.append('cameralens')
        except Exception, e:
            app.log('Not saving cameralens status: %s' % str(e))
            pass

    if 'tt' in devices:
        try:
            cmdfile += 'self.tt.set(amp=%f,freq=%f,offx=%f, offy=%f)\n' % (app.tt.amp(), app.tt.freq(), app.tt.offx(), app.tt.offy())
            saved.append('tt')
        except Exception, e:
            app.log('Not saving tt status: %s' % str(e))
            pass

    if 'adc' in devices:
	try:
	    cmdfile +='self.adc._motor1.moveTo(%f)\n' % app.adc._motor1.getPos()
	    cmdfile +='self.adc._motor2.moveTo(%f)\n' % app.adc._motor2.getPos()
	    to_wait.append('adc._motor1')
	    to_wait.append('adc._motor2')
            saved.append('adc')
        except:
            app.log('Not saving adc status: %s' % str(e))
            pass

    # See if the anything flowerpot was included in the device list
    if 'flowerpot' in devices:
        flowerpotfile = 'self.flowerpot.setOperatingState(%d)\n' % app.flowerpot.getOperatingState()
        cmdfile = flowerpotfile + cmdfile

    for device in to_wait:
        cmdfile += 'self.%s.waitTargetReached(checkStatus= True)\n' % device

    f = file(filename, 'w')
    f.write(cmdfile)
    f.close()

def load(app, filename):
    '''
    Loads a board setup.
    '''

    app.mute()
    devices = setupDevices.loadDevices(app)
    app.unmute()

    app.log('Applying setup, please wait...')

    # Transfer devices in the main namespace

    for device in devices:
        cmd = '%s = app.%s' % (device, device)
        bytecode = compile(cmd, '<string>', 'single')
        eval(bytecode)

    # Read commands from setup file
    app.execfile(filename)


def usage():
    print 'Usage: boardSetup.py [-load|-save] filename'


def run():

    import sys
    from AdOpt import thAOApp

    if len(sys.argv) != 3:
        usage()
        sys.exit(0)

    task = sys.argv[1]
    filename = sys.argv[2]

    app = thAOApp('boardsetup', argv = sys.argv)

    if task == '-load':
        load( app, filename)
    elif task == '-save':
        save( app, filename)
    else:
        usage()

if __name__ == '__main__':

    try:
        run()
    except Exception, e:
    
        errstr = "Error: "+str(e)
        app.log(errstr)
        print errstr
        import time
        time.sleep(200)



