#!/bin/tcsh

/bin/rm /tmp/ao_restart.stdout
tail -s 0.1 -F /tmp/ao_restart.stdout | $ADOPT_ROOT/bin/msg.py &
set MSGBOX = $!

$ADOPT_ROOT/bin/ao_restart_script.sh > /tmp/ao_restart.stdout
kill -9 $MSGBOX

$ADOPT_ROOT/bin/startAO.py  &

 

