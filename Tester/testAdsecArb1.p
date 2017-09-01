#
# Test macro to be used to test the AdSecArbitrator
#
# Usage: 
#
#    thrdtest -e testAdsecArb0.p
#
# The procedure simulates the AOS up and running and updating
# the variables: x.AOS.AMB.WINDSPEED x.AOS.SWA.DEPLOYED x.AOS.TEL.EL 
#
# the procedure is an infinite loop which must be stopped
# sennding a TERMINATE message to it
#
# while running, the variable values can be modified by another
# running thrdtest
#

reg 1 R   # store side prefix into register 1 

echo Testing side: \1
echo
echo setting variables to initial values
write \1.AOS.AMB.WINDSPEED r 0
write \1.AOS.SWA.DEPLOYED i 1
write C.AOS.TEL.EL r 324000
echo Starting loop. Terminate it by sending: term \?

loop 0                        # Loop forever
get \1.AOS.AMB.WINDSPEED  0       # Get current value of var
write \1.AOS.AMB.WINDSPEED r \0   # Write it back
get \1.AOS.SWA.DEPLOYED 0
write \1.AOS.SWA.DEPLOYED i \0
get C.AOS.TEL.EL 0
write C.AOS.TEL.EL r \0
sleep 2
endloop

