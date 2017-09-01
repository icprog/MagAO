#
# Test procedure 2. (To be launched by mirror_test.sh)
#
write T.A.due i 1 2 3 4 5
write BA.due r 3 4 5 6
write M-ONLY s Due

sleep 3
dinfo
cdump
vdump *
get T.A.uno
get BA.uno

set T.A.uno i 5 4 3 2 1
set BA.tre r 5 6 7

get T.A.tre
get BA.tre
set M-ONLY s Due

set T.A.tre i 10 9 8
set BA.uno r .1 .2 .3

get T.A.uno
get BA.tre

set T.A.tre i 5 4 3 2 1
set BA.tre r 5 6 7

get T.A.uno
get BA.uno

set T.A.uno i 10 9 8
set BA.uno r .1 .2 .3

get T.A.tre
get BA.uno

set T.A.tre i 5 4 3 2 1
set BA.uno r 5 6 7
set M-ONLY s Due

get T.A.tre
get BA.tre

set T.A.tre i 10 9 8
set BA.tre r .1 .2 .3

get T.A.uno
get BA.uno

set T.A.tre i 5 4 3 2 1
set BA.uno r 5 6 7

get T.A.uno
get BA.tre

set T.A.tre i 10 9 8
set BA.uno r .1 .2 .3
set M-ONLY s Due

get T.A.tre
get BA.tre

set T.A.tre i 5 4 3 2 1
set BA.uno r 5 6 7

get T.A.tre
get BA.uno

set T.A.uno i 10 9 8
set BA.tre r .1 .2 .3

get T.A.tre
get BA.tre

set T.A.uno i 5 4 3 2 1
set BA.tre r 5 6 7

get T.A.tre
get BA.tre
set M-ONLY s Due

set T.A.tre i 10 9 8
set BA.tre r .1 .2 .3

get T.A.uno
get BA.tre

set T.A.tre i 5 4 3 2 1
set BA.tre r 5 6 7

get T.A.tre
get BA.uno
set M-ONLY s Due

set T.A.tre i 10 9 8
set BA.uno r .1 .2 .3

get T.A.tre
get BA.tre
set M-ONLY s Due

set T.A.uno i 5 4 3 2 1
set BA.tre r 5 6 7

get T.A.tre
get BA.tre
set M-ONLY s Due

set T.A.uno i 10 9 8
set BA.tre r .1 .2 .3

dinfo
vdump *
term
exit
