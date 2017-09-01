#
# Test macro file for exec command  (thrdtest)
vlist *
write prova i 1 2 3
ready
vlist *

# The following command sequence exercises the 
# message expiration mechanism
# NOTE: you must start the ECHOER program for test to work

debug 1
ttl 10   # set default time to live
info *   # show thrdlib status
msg 3000 0 n ECHOER     # This message should be removed from message queue
                        # After about 10 seconds
sleep 1
msg 3000 0 n ECHOER     # This message also should be removed from message queue
                        # After about 10 seconds
sleep 2
info *
msg 3000 0 ECHOER       # This message should show up in the generic handler
sleep 8
info *
