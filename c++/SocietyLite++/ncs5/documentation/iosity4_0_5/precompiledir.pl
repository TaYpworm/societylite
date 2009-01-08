#!/usr/bin/perl

################################################################################
#   precompiledir.pl - Directory Pre-compiler in IOsity v4.0
#   Copyright (C) 2001, 2002  Brian Westphal
#   
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#   
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#   
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
################################################################################

my $webpagefile;
my $filename;

#Checking number of parameters
if (@ARGV != 4)
{
	print "\nUsage: precompiledir -i inputdir -o outputdir\n\n";
	exit (1);
}
else
{
	if (($ARGV[0] eq "-i" && $ARGV[2] eq "-o") || ($ARGV[0] eq "-o" && $ARGV[2] eq "-i"))
	{
		if ($ARGV[0] eq "-i")
		{
			$webpagefile = $ARGV[1];
			$filename    = $ARGV[3];
		}
		else
		{
			$webpagefile = $ARGV[3];
			$filename    = $ARGV[1];
		}
	}
}

opendir (dHANDLE, $webpagefile);
	my @files = readdir (dHANDLE);
closedir (dHANDLE);

foreach my $file (@files)
{
	if (length ($file) >= 8 && substr ($file, length ($file) - 8) eq ".webpage")
	{
		my $executecommand = "./precompile.pl -i \"" . $webpagefile . substr ($file, 0, length ($file) - 8) . "\" -o \"" . $filename . substr ($file, 0, length ($file) - 8) . ".html\"";
		qx /$executecommand/;
	}
}
