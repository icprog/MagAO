#!/usr/bin/env perl
# map.pl
#
# Script to read the slope computer memory xml map and produce
# a header file
#
# Reads from standard input and writes to standard output.
#
# A. Puglisi, Aug 2005

$match1 = "<SYMBOL name='";
$match2 = "' address='";
$match3 = "'";
$type = "ulong";

while (<>)
	{
	if (/$match1(.*?)$match2(.*?)$match3/)
		{
		$name = uc $1;
		$name =~ s/\./_/g;		# avoid special characters
		$name .= " " x (40-length($name));

		$addr = $2;

		print STDOUT "\#define SLPCOMP_$name ($addr)\n";

		}
	}
	
	
	
