#
# Test procedure 1. (To be launched by mirror_test.sh)
#
write T.A.uno i 1 2 3 4 5
write BA.uno r 3 4 5 6
write M-ONLY s Uno

sleep 3
dinfo
cdump
vdump *
get T.A.due
get BA.due

set T.A.due i 5 4 3 2 1
set BA.tre r 5 6 7
set M-ONLY s Uno

get T.A.tre
get BA.tre
set M-ONLY s Uno

set T.A.tre i 10 9 8
set BA.due r .1 .2 .3

get T.A.due
get BA.tre

set T.A.tre i 5 4 3 2 1
set BA.tre r 5 6 7

get T.A.due
get BA.due
set M-ONLY s Uno

set T.A.due i 10 9 8
set BA.due r .1 .2 .3

get T.A.tre
get BA.due

set T.A.tre i 5 4 3 2 1
set BA.due r 5 6 7
set M-ONLY s Uno

get T.A.tre
get BA.tre

set T.A.tre i 10 9 8
set BA.tre r .1 .2 .3
set M-ONLY s Uno

get T.A.due
get BA.due

set T.A.tre i 5 4 3 2 1
set BA.due r 5 6 7

get T.A.due
get BA.tre

set T.A.tre i 10 9 8
set BA.due r .1 .2 .3
set M-ONLY s Uno

get T.A.tre
get BA.tre

set T.A.tre i 5 4 3 2 1
set BA.due r 5 6 7

get T.A.tre
get BA.due
set M-ONLY s Uno

set T.A.due i 10 9 8
set BA.tre r .1 .2 .3

get T.A.tre
get BA.tre

set T.A.due i 5 4 3 2 1
set BA.tre r 5 6 7
set M-ONLY s Uno

get T.A.tre
get BA.tre

set T.A.tre i 10 9 8
set BA.tre r .1 .2 .3

get T.A.due
get BA.tre
set M-ONLY s Uno

set T.A.tre i 5 4 3 2 1
set BA.tre r 5 6 7

get T.A.tre
get BA.due
set M-ONLY s Uno

set T.A.tre i 10 9 8
set BA.due r .1 .2 .3

get T.A.tre
get BA.tre

set T.A.due i 5 4 3 2 1
set BA.tre r 5 6 7

get T.A.tre
get BA.tre
set M-ONLY s Uno

set T.A.due i 10 9 8
set BA.tre r .1 .2 .3

dinfo
vdump *
term
exit
