#
# Test macro to be used together with duplicate.sh
#
# This procedure is launched by each client of group 1 at start

log 6 I'm starting. Writing some variables

notify TEST2*

write TEST1.0.\$ i \? \? \? \? \?
write TEST1.1.\$ r \? \? \? \? \?
write TEST1.2.\$ i \? \? \? \? \?
write TEST1.3.\$ i \? \? \? \? \?
write TEST1.4.\$ r \? \? \? \? \?
write TEST1.5.\$ i \? \? \? \? \?
write TEST1.6.\$ i \? \? \? \? \?
write TEST1.7.\$ r \? \? \? \? \?
write TEST1.8.\$ i \? \? \? \? \?
write TEST1.9.\$ r \? \? \? \? \?

sleep 10

log 6 waiting for TESTMANY00
wait TESTMANY00 
nowait

log 6 Ready received from TESTMANY00 Rewriting my variables

set TEST1.0.\$ r \? \? \? \? \?
sleep 0.\?
set TEST1.1.\$ i \? \? \? \? \?
sleep 0.\?
set TEST1.2.\$ i \? \? \? \? \?
sleep 0.\?
set TEST1.3.\$ r \? \? \? \? \?
sleep 0.\?
set TEST1.4.\$ i \? \? \? \? \?
sleep 0.\?
set TEST1.5.\$ r \? \? \? \? \?
sleep 0.\?
set TEST1.6.\$ r \? \? \? \? \?
sleep 0.\?
set TEST1.7.\$ i \? \? \? \? \?
sleep 0.\?
set TEST1.8.\$ r \? \? \? \? \?
sleep 0.\?
set TEST1.9.\$ i \? \? \? \? \?

log 6 Now sleeping for 300 sec
sleep 300
log 6 Quitting
quit
