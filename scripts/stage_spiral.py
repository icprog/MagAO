#!/usr/bin/env python


num_frames = 20


######################
# Spiral definition

def move_right(x,y):
    return x+1, y

def move_down(x,y):
    return x,y-1

def move_left(x,y):
    return x-1,y

def move_up(x,y):
    return x,y+1

moves = [move_right, move_down, move_left, move_up]

def spiral_points(scale=1.0, end=None):
    if end==None:
       end = 1000
    from itertools import cycle
    _moves = cycle(moves)
    n = 1
    pos = 0.,0.
    times_to_move = 1
    yield pos
    while True:
        for _ in range(2):
            move = _moves.next()
            for _ in range(times_to_move):
                if n >= end:
                    return
                pos = move(*pos)
                n+=1
                yield (pos[0]*scale, pos[1]*scale)
        times_to_move+=1


from AdOpt import thAOApp, frames_lib, cfg
from AdOpt.hwctrl import ccd, bayside_stage
from AdOpt.wrappers import diagbuf
import sys, time

try:
  radius = float(sys.argv[1])
  step   = float(sys.argv[2])
except:
  print
  print 'Usage: spiral.py <radius in arcsec> <step size in arcsec>'
  print 
  print 'Example: spiral.py 3.0 1.5'
  print ' will execute a spiral movement with a radius of 3.0 arcsecs'
  print ' from the current point (a 6x6 arcsec range in total) with a'
  print ' step size of 1.5 arcsec'
  print
  sys.exit(-1)

self = thAOApp(myname='spiral')

ccd39 = ccd.ccd('ccd39', self)
diagInterface = diagbuf.diagbuf( self, ccd39.ccdnum)
stagex = bayside_stage.bayside_stage("baysidex", self)
stagey = bayside_stage.bayside_stage("baysidey", self)
arcsec2mm = 0.500


def measure(n):
   frame = frames_lib.averageFrames( ccd39.get_frames( n, 'pixels', diagInterface))
   return frame.sum()

def offset(deltax, deltay):
#   print 'Moving by %f, %f' % (deltax, deltay)

   posx = stagex.getPos()
   posy = stagey.getPos()

   posx += deltax
   posy += deltay

   stagex.moveTo( posx, waitTimeout=20)
   stagey.moveTo( posy, waitTimeout=20)


gen = spiral_points(scale=step)

meas= {}
pos = {}
stages = {}
p1 = gen.next()
count=0
dummy = measure(num_frames) # Test ccd39 and print output before our listing

print
print '%10s %10s %10s %10s %10s' % ('Pos X', 'Pos Y', 'Flux', 'Stage X', 'Stage Y')
print

while True:
   meas[count] = 0# measure(num_frames)
   pos[count] = p1
   stages[count]  = {'x': stagex.getPos(), 'y': stagey.getPos()}

   print  '%10.1f %10.1f %10d %10.3f %10.3f' % (pos[count][0], pos[count][1], meas[count], stages[count]['x'], stages[count]['y'])

   p2 = gen.next()
   if (abs(p2[0]) > radius) or (abs(p2[1]) > radius):
      break

   deltax = p2[0]-p1[0]
   deltay = p2[1]-p1[1]

   offset(deltax*arcsec2mm, deltay*arcsec2mm)

   p1 = p2
   count +=1



