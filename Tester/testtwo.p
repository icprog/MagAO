#
# Test macro to be used together with testmany.sh
#
# This procedure is launched by each client of group 2 at start

log 6 I'm starting. Writing some variables

notify TEST1*

write TEST2.0.\$ i \? \? \? \? \?
write TEST2.1.\$ r \? \? \? \? \?
write TEST2.2.\$ i \? \? \? \? \?
write TEST2.3.\$ i \? \? \? \? \?
write TEST2.4.\$ r \? \? \? \? \?
write TEST2.5.\$ i \? \? \? \? \?
write TEST2.6.\$ i \? \? \? \? \?
write TEST2.7.\$ r \? \? \? \? \?
write TEST2.8.\$ i \? \? \? \? \?
write TEST2.9.\$ r \? \? \? \? \?

sleep 10

log 6 waiting for TESTMANY00
wait TESTMANY00 
nowait

log 6 Ready received from TESTMANY00. Rewriting my variables

set TEST2.0.\$ r \? \? \? \? \?
sleep 0.\?
set TEST2.1.\$ i \? \? \? \? \?
sleep 0.\?
set TEST2.2.\$ i \? \? \? \? \?
sleep 0.\?
set TEST2.3.\$ r \? \? \? \? \?
sleep 0.\?
set TEST2.4.\$ i \? \? \? \? \?
sleep 0.\?
set TEST2.5.\$ r \? \? \? \? \?
sleep 0.\?
set TEST2.6.\$ r \? \? \? \? \?
sleep 0.\?
set TEST2.7.\$ i \? \? \? \? \?
sleep 0.\?
set TEST2.8.\$ r \? \? \? \? \?
sleep 0.\?
set TEST2.9.\$ i \? \? \? \? \?

log 6 Now sleeping for 300 sec
sleep 300
log 6 Quitting
quit
