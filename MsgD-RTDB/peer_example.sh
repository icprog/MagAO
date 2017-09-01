# The following message daemons are started on the same machine
# (different listening ports) as intercommunicating peers
#
# Their names will be, respectively, M_ADSEC, M_LBTIWFS and M_FLAOWFS
#
# Identity for each MSGD is specified by argument -i
#
# Peer list here is specified by the argument -c
#
# Another way to specify peer list is by the config argument: peers
#
echo
echo "Note: log output is sent to files, respectively M_ADSEC.log, "
echo "      M_FLAOWFS.log, M_LBTIWFS.log"
echo

./msgdrtdb -n -i FLAOWFS -p 9751 -c FLAOWFS:9751,ADSEC:9752,LBTIWFS:9753 > M_FLAOWFS.log &
./msgdrtdb -n -i ADSEC   -p 9752 -c FLAOWFS:9751,ADSEC:9752,LBTIWFS:9753 > M_ADSEC.log   &
./msgdrtdb -n -i LBTIWFS -p 9753 -c FLAOWFS:9751,ADSEC:9752,LBTIWFS:9753 > M_LBTIWFS.log &

