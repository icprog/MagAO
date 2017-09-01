#!/usr/bin/env python
#
# Proxy for a rerotator controller from IDL-Windows
#
# Basically a little network server

import socket, time, sys

from AdOpt import cfg, thAOApp, AOConfig
from AdOpt.hwctrl import simplemotor


while len(sys.argv)>0:
    opt = sys.argv.pop(0)
    if opt == '-h':
        print """
rerot_proxy.py   Jun 2007 A.Puglisi

Proxy server to control the pupil rerotator from IDL

Usage: rerot_proxy.py -f <config_file> 
        """
        sys.exit(-1)
    elif opt == '-f':
        filename = sys.argv.pop(0)
        mycfg = AOConfig.read_config( filename, add_side = True)

myName        = mycfg['MyName'].Value()
mySide        = mycfg['mySide'].Value()


app = thAOApp(myName)

task = mycfg['ControlledTask'].Value()

varPosReq = app.ReadVar(cfg.varname(task, 'POS.REQ'))
varPosCur = app.ReadVar(cfg.varname(task, 'POS.CUR'))

motorcfg = cfg.cfg[mycfg['ControlledMotor'].Value()]

homingPos = motorcfg['HomingPosition'].Value()
abortPos = motorcfg['AbortPosition'].Value()
posWindow = motorcfg['GoodWindow'].Value()

timeout = 60.0

rerot = simplemotor.SimpleMotor(mycfg['ControlledMotor'].Value(), app)

PORT = mycfg['ListenPort'].Value()


s = socket.socket( socket.AF_INET, socket.SOCK_STREAM)
s.bind(('',PORT))
s.listen(1)

while 1:
  try:
	conn, addr = s.accept()
	print 'Connected by', addr

#	conn.settimeout(5)

	while 1:
		data = conn.recv(128)
		if not data:
		    print 'Connection closed, exiting loop'	
		    break	

		print 'Received command', data

		if data[0:4] == 'home':
			app.SetVar( varPosReq, homingPos)
			conn.send('OK')

		elif data[0:5] == 'abort':
			app.SetVar( varPosReq, abortPos)
			conn.send('OK')

		elif data[0:4] == 'exit':
		    print 'Exiting'
		    conn.close()
		    s.close()
		    sys.exit(0)

		elif data[0:4] == 'move':
			targetPos = float(data[5:])
			app.SetVar( varPosReq, targetPos)

			start = time.time()
			while 1:
				varPosCur = app.ReadVar( cfg.varname(task,'POS.CUR'))
				pos = varPosCur.Value()
				if abs(targetPos - pos) < posWindow:
					print 'Position OK'
					conn.send('OK')
					break
				time.sleep(0.5)
				now = time.time()
				print 'Cur pos: %f - Req pos: %f - time left: %f' % (pos, targetPos, timeout - (now-start))
				if now-start > timeout:
					print 'Timeout moving rerotator'
					break

#        conn.close()
#        break

  except socket.error:
    pass

s.close()
