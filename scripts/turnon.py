
from AdOpt import WfsArbScripts

# 1. Turn on BCU

print 'Turning BCU power ON'
power.turnon('bcu')

# 2. Wait until bcu is up

print 'Waiting until BCU answers on the Ethernet...'
setupDevices.waitIPhost( app, 'bcu39', waitTimeout=60)

# 3. Force TT/CL rest (in case things were already on)

print 'Ensuring that tip/tilt mirror is RESTed...'
tt.mirrorSet(0)

# 4. Turn on CCD

print 'Turning ON ccd39'
power.turnon('ccd39')

# 5. Wait until CCD is online and stop it

print 'Waiting until ccd39 answers on the serial line'
ccd39.waitActive( timeout = 30)
ccd39.stop()

# 7. Set TT

print 'Setting TT/CL...'
tt.mirrorSet(1)

# 8. Configure CCD

print 'Configuring CCD with bin1x1, 200 Hz...'
WfsArbScripts.setHObinning( app, 1)
ccd39.set_framerate(200)






