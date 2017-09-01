#!/usr/bin/perl
# Config.pl
#
# Script to read an adopt configuration file and produce a header file.
#
# Reads from standard input and writes to standard output.
#
# A. Puglisi, Apr 2005

while (<>)
	{
	$comments = "";
	if ( s/\#(.*)$//)		# save comments for later
		{
		$comments = $1;
		}
	# skip empty lines
	if ( /^\s*$/)
		{
		print STDOUT "// $comments" if $comments;	
		print "\n";
		next;
		}

	if (/^(\S+)\s+(\S+)\s+(.*)$/)
		{
		$name = uc $1;
		$name .= " " x (30-length($name));
		$type = $2;
		$value = $3;

        $value =~ s/^x/0x/;

		$comments = "// ".$comments if $comments;
		print STDOUT "\#define $name ($value) $comments\n";
		}
	}
	
	
	
