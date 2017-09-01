#!/usr/bin/env python
 
import socket


host = ''
port = 1024
reply = "!0100"

print "adamDummy"
print "Just listen to port ", port, " and replies " + reply
print "NOTE:"
print "localhost: " + socket.gethostbyname(socket.gethostname())
print "adamdx:    " + socket.gethostbyname('adamdx')
print "adam2dx:   " + socket.gethostbyname('adam2dx')

 
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
s.bind((host, port))

while 1:
    message, address = s.recvfrom(8192)
    print "Got data from ", address
    # s.sendto(message, address)
    s.sendto(reply, address)
s.close()
