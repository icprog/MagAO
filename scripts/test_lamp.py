
# test lamp luminosity

num_frames = 10
outfile = 'test.txt'
steps = 256

#Prepare things
lamp.setIntensity(0)
ccd39.reprogram(speed=2500,xbin=1,ybin=1)
ccd39.set_rep(1050)
ccd39.equalize_quadrants( target = 1000, delay=0.1)

import numpy, time

t=range(steps)

for i in range(steps):
	lamp.setIntensity(i, waitTimeout=10)
	time.sleep(0.1)	

	t[i] = frames_lib.averageFrames( ccd39.get_frames(num_frames)).sum()


print 'setting     intensity (total ccd counts @ 1Khz)'

outf = file(outfile,'w')
for i in range(steps):
	print '%d     %f' % (i, t[i]-t[0])
	outf.write('%d     %f\n' % (i, t[i]-t[0]))

outf.close()





