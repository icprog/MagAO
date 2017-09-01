# The following message daemons are started on the same machine
# (different listening ports) as intrercommunicating peers
#
# Their names will be, respectively, MSGD_UNO, MSGD_DUE and MSGD_TRE
#
# Identity for each MSGD is specified by argument -i
#
# Peer list here is specified by the argument -c
#
# Another way to specify peer list is by the config argument: peers
#
# All the daemons will look for the file:
#
# $ADOPT_ROOT/conf/msgdrtdb.mvar
#
# For a list of mirrored variables
#

./msgdrtdb -i UNO -p 9751 -c UNO:9751,DUE:9752,TRE:9753 &
./msgdrtdb -i DUE -p 9752 -c UNO:9751,DUE:9752,TRE:9753 &
./msgdrtdb -i TRE -p 9753 -c UNO:9751,DUE:9752,TRE:9753 &
