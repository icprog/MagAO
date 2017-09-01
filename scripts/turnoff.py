
ccd39.stop()
time.sleep(1)

print 'Setting zero modulation amplitude...'
tt.set( amp = 0)
time.sleep(10)

print 'Resting TT mirror...'
tt.mirrorSet(0)
time.sleep(1)

print 'Turning off power...'
power.turnoff('ccd39')
time.sleep(1)
power.turnoff('bcu')
time.sleep(1)
