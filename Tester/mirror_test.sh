#
# Procedure to launch the variable mirroring tetsts
#
#  ../MsgD-RTDB/mirror_test.sh
#  ../Tester/mirror_test.sh
#
# Inspect log files from Msgd peers (MSGD_UNO.log, MSGD_DUE.log, MSGD_TRE.log)
#
./thrdtest -m :9751 -e mirror_test_uno 1 &
./thrdtest -m :9752 -e mirror_test_due 2 &
./thrdtest -m :9753 -e mirror_test_tre 3 &
