#!/usr/bin/env perl

while (<>)
    {
    chomp;

    next if /^;/;
    next if /^\s*$/;

    /(\d+)\s+([\-\d]+)\s+(.+)/;

    print "P$1\t0\t$2\t$3\n";
    }
