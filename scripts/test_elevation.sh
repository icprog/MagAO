#!/bin/bash
echo -e "write AOS.L.SWA.DEPLOYED i 1\nwrite AOS.AMB.WINDSPEED r 3\nwrite AOS.TEL.EL r 90\nquit"  > /tmp/elevtest
thrdtest < /tmp/elevtest
sleep 1


#/bin/bash
echo -e "write L.LAB.MODE i 0\nquit" > /tmp/labmode
thrdtest < /tmp/labmode

for i in `seq 1 20`; do
    W=3
    TMP=$[90/20-15/20]
    EL=$[90*3600-$i*TMP*3600]
    S=1
    echo "SwingArm: $S, Wind: $W. Elevation: $EL"
    echo -e "write AOS.L.SWA.DEPLOYED i $S\nwrite AOS.AMB.WINDSPEED r $W\nwrite AOS.TEL.EL r $EL\nquit"  > /tmp/elevtest
    thrdtest < /tmp/elevtest
    sleep 1
done


#/bin/bash
echo -e "write L.LAB.MODE i 1\nquit" > /tmp/labmode
thrdtest < /tmp/labmode
