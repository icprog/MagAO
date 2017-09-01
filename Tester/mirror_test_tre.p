#
# Test procedure 3. (To be launched by mirror_test.sh)
#
write T.A.tre i 1 2 3 4 5
write BA.tre r 3 4 5 6
write M-ONLY s Tre

sleep 3
dinfo
cdump
vdump *
get T.A.due
get BA.due

set T.A.due i 5 4 3 2 1
set BA.uno r 5 6 7
set M-ONLY s Tre

get T.A.uno
get BA.uno
set M-ONLY s Tre

set T.A.uno i 10 9 8
set BA.due r .1 .2 .3

get T.A.due
get BA.uno

set T.A.uno i 5 4 3 2 1
set BA.uno r 5 6 7

get T.A.due
get BA.due
set M-ONLY s Tre

set T.A.due i 10 9 8
set BA.due r .1 .2 .3

get T.A.uno
get BA.due

set T.A.uno i 5 4 3 2 1
set BA.due r 5 6 7
set M-ONLY s Tre

get T.A.uno
get BA.uno

set T.A.uno i 10 9 8
set BA.uno r .1 .2 .3
set M-ONLY s Tre

get T.A.due
get BA.due

set T.A.uno i 5 4 3 2 1
set BA.due r 5 6 7

get T.A.due
get BA.uno

set T.A.uno i 10 9 8
set BA.due r .1 .2 .3
set M-ONLY s Tre

get T.A.uno
get BA.uno

set T.A.uno i 5 4 3 2 1
set BA.due r 5 6 7

get T.A.uno
get BA.due
set M-ONLY s Tre

set T.A.due i 10 9 8
set BA.uno r .1 .2 .3

get T.A.uno
get BA.uno

set T.A.due i 5 4 3 2 1
set BA.uno r 5 6 7
set M-ONLY s Tre

get T.A.uno
get BA.uno

set T.A.uno i 10 9 8
set BA.uno r .1 .2 .3

get T.A.due
get BA.uno
set M-ONLY s Tre

set T.A.uno i 5 4 3 2 1
set BA.uno r 5 6 7

get T.A.uno
get BA.due
set M-ONLY s Tre

set T.A.uno i 10 9 8
set BA.due r .1 .2 .3

get T.A.uno
get BA.uno

set T.A.due i 5 4 3 2 1
set BA.uno r 5 6 7

get T.A.uno
get BA.uno
set M-ONLY s Tre

set T.A.due i 10 9 8
set BA.uno r .1 .2 .3

dinfo
vdump *
term
exit
