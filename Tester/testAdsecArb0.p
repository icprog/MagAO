#
# Test macro to be used to test the AdSecArbitrator
#
# Usage: 
#
#    thrdtest -e testAdsecArb0.p
#
# The procedure writes values to proper variables to check
# the safety algorithms in the Adsec Arbitrator
#

#  Variable prefix
reg 1 R    

echo Testing side: \1
echo
echo -n Setting variables to 0 at \t

write \1.AOS.AMB.WINDSPEED r 0
write \1.AOS.SWA.DEPLOYED i 1
write C.AOS.TEL.EL r 324000
sleep 2
echo -n .

write \1.AOS.AMB.WINDSPEED r 0
write \1.AOS.SWA.DEPLOYED i 1
write C.AOS.TEL.EL r 324000
sleep 2
echo 

echo -n Testing windspeed variations at \t

write \1.AOS.AMB.WINDSPEED r 5
write \1.AOS.SWA.DEPLOYED i 1
write C.AOS.TEL.EL r 324000
sleep 2
echo -n .

write \1.AOS.AMB.WINDSPEED r 5
write \1.AOS.SWA.DEPLOYED i 1
write C.AOS.TEL.EL r 324000
sleep 2
echo -n .

write \1.AOS.AMB.WINDSPEED r 10
write \1.AOS.SWA.DEPLOYED i 1
write C.AOS.TEL.EL r 324000
sleep 2
echo -n .

write \1.AOS.AMB.WINDSPEED r 15
write \1.AOS.SWA.DEPLOYED i 1
write C.AOS.TEL.EL r 324000
sleep 2
echo -n .

write \1.AOS.AMB.WINDSPEED r 10
write \1.AOS.SWA.DEPLOYED i 1
write C.AOS.TEL.EL r 324000
sleep 2
echo -n .

write \1.AOS.AMB.WINDSPEED r 6
write \1.AOS.SWA.DEPLOYED i 1
write C.AOS.TEL.EL r 324000
sleep 2
echo 

echo -n Testing elevation variations at \t

write \1.AOS.AMB.WINDSPEED r 0
write \1.AOS.SWA.DEPLOYED i 1
write C.AOS.TEL.EL r 270000
sleep 2
echo -n .

write \1.AOS.AMB.WINDSPEED r 0
write \1.AOS.SWA.DEPLOYED i 1
write C.AOS.TEL.EL r 216000
sleep 2
echo -n .

write \1.AOS.AMB.WINDSPEED r 0
write \1.AOS.SWA.DEPLOYED i 1
write C.AOS.TEL.EL r 162000
sleep 2
echo -n .

write \1.AOS.AMB.WINDSPEED r 0
write \1.AOS.SWA.DEPLOYED i 1
write C.AOS.TEL.EL r 108000
sleep 2
echo -n .

write \1.AOS.AMB.WINDSPEED r 0
write \1.AOS.SWA.DEPLOYED i 1
write C.AOS.TEL.EL r 36000
sleep 2
echo -n .

write \1.AOS.AMB.WINDSPEED r 0
write \1.AOS.SWA.DEPLOYED i 1
write C.AOS.TEL.EL r 162000
sleep 2
echo 

echo -n Testing expiration of elevation at \t

write \1.AOS.AMB.WINDSPEED r 0
write \1.AOS.SWA.DEPLOYED i 1
sleep 2
echo -n .
write \1.AOS.AMB.WINDSPEED r 0
write \1.AOS.SWA.DEPLOYED i 1
sleep 2
echo -n .
write \1.AOS.AMB.WINDSPEED r 0
write \1.AOS.SWA.DEPLOYED i 1
sleep 2
echo -n .
write \1.AOS.AMB.WINDSPEED r 0
write \1.AOS.SWA.DEPLOYED i 1
sleep 2
echo -n .
write \1.AOS.AMB.WINDSPEED r 0
write \1.AOS.SWA.DEPLOYED i 1
sleep 2
echo -n .
write \1.AOS.AMB.WINDSPEED r 0
write \1.AOS.SWA.DEPLOYED i 1
sleep 2
echo 

echo -n Restoring elevation at \t

write \1.AOS.AMB.WINDSPEED r 0
write \1.AOS.SWA.DEPLOYED i 1
write C.AOS.TEL.EL r 162000
sleep 2
echo -n .
write \1.AOS.AMB.WINDSPEED r 0
write \1.AOS.SWA.DEPLOYED i 1
write C.AOS.TEL.EL r 162000
sleep 2
echo 

echo -n Checking swing arm removing at \t

write \1.AOS.AMB.WINDSPEED r 0
write \1.AOS.SWA.DEPLOYED i 0
write C.AOS.TEL.EL r 162000
sleep 2
echo -n .
write \1.AOS.AMB.WINDSPEED r 0
write \1.AOS.SWA.DEPLOYED i 0
write C.AOS.TEL.EL r 162000
sleep 2
echo 

echo Checking swing arm deploying at \t

write \1.AOS.AMB.WINDSPEED r 0
write \1.AOS.SWA.DEPLOYED i 1
write C.AOS.TEL.EL r 162000

echo
echo No more commands: variables should expire in 10 secs
