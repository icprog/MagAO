#!/usr/bin/env python
 
import socket


host = ''
port = 502
reply = "!0100"

print "adamDummyTcp (call will sudo so it serves ports below 1024)"
print "Just listen to port ", port, " and replies " + reply
print "NOTE:"
print "localhost: " + socket.gethostbyname(socket.gethostname())
print "adamdx:    " + socket.gethostbyname('adamdx')
print "adam2dx:   " + socket.gethostbyname('adam2dx')

 
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind((host, port))

while 1:
    s.listen(1)
    conn,addr = s.accept()

    print 'Connected to ', addr

    while 1:
        message = conn.recv(4096)
        print "Got data: ", message

        if not message:
            break
        conn.send(reply)
    conn.close()
    print 'Disconnected'

