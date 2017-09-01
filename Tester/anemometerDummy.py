#!/usr/bin/env python
 
import socket
import time
import random


host = ''
port = 4001
reply = '\002Q,+0.01,+0.02,-0.03,M,00,\0032F\n'

print "anemometerDummy"
print "Just listen to port ", port, " and replies " + reply
 
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind((host, port))

while 1:
    s.listen(1)
    conn,addr = s.accept()
    print 'Connected to ', addr
    conn.send(reply)
    while 1:
        data = "+%.2f,+%.2f,+%.2f" % (random.random()/10, random.random()/10+0.5, random.random()/10+0.8)
        #reply = '\002Q,'+data+',M,00,\0032F\n'
        print 'sendind data:' + reply
        try:
            conn.send(reply)
            time.sleep(0.1)
        except:
            break
        #        message = conn.recv(1)
        #        print "Got data: ", message
    conn.close()
    print 'Disconnected'

