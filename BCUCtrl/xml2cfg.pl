#!/usr/bin/env perl
# map.pl
#
# Script to read the slope computer memory xml map and produce
# a standard configuration file
#
# Reads from standard input and writes to standard output.
#
# A. Puglisi, Aug 2005

$match1 = "<SYMBOL name='";
$match2 = "' address='";
$match3 = "'";
$type = "ulong";

$column_align = 30;
$first_line ="global_counter";
$first_line_found =0;

while (<>)
	{
    $first_line_found =1 if (/$first_line/);
    next if !$first_line_found;

	if (/$match1(.*?)$match2(.*?)$match3/)
		{
		$name = $1;
		$name =~ s/\./_/g;		# avoid special characters
		$name .= " " x ($column_align-length($name));

		$addr = $2;
        $addr =~ s/^0//;

		print STDOUT "$name long $addr\n";

		}
	}
	
	
	
