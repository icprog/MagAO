#
# Test macro file for (thrdtest)
#
# This macro exercises many commands and features of msgdrtdb
#
write abcd i 1 2 3
write aefg r 1 2 3
write ahij i 1 2 3
write almn r 1 2 3
write aopq i 1 2 3
write arst r 1 2 3
write auvz s Sopra la panca la capra campa

write bbcd r 1 2 3 6 5 4
write befg r 1 2 3
write bhij r 1 2 3
write blmn r 1 2 3
write bopq r 1 2 3
write brst i 1 2 3 6 5 4
write buvz i 1 2 3
dinfo
ready

vlist *

write cbcd i 1 2 3 6 5 4
write cefg i 1 2 3
write chij i 1 2 3
write clmn i 1 2 3 6 5 4
write copq i 1 2 3
write crst i 1 2 3
write cuvz s Trentatre' trentini enmtrarono in Trento

notify chij 
notify blmn
notify aefg 

write aefg r 3 2 1 9 8 7 6 5 

del aefg

trace ahij
write ahij i 5 4 3 2 1

vlist b*

set chij r 3 2 1 9 8 7 6 5 

ndump
bdump
cdump

vdump *

give blmn

ready

set ahij r 3 4 5

lock a*
set aopq i 1 2 3 4 5 6 7 
unlock a*

