#!/usr/bin/perl

################################################################################
#   precompile.pl - File Pre-compiler in IOsity v4.0
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
	print "\nUsage: precompile -i inputfile -o outputfile\n\n";
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

#Setting alarm (protects agains infinite recursive loops)
alarm (10);

#Disabling security
$VARIABLES {"DisableSecurity"} = "true";
$VARIABLES {"ReferenceDirectory"} = "";

$CONFIG {"REQUEST_TIME"} = time ();

$CONFIG {"IOSITY_SYSTEM_PATH"} = "system/";

do $CONFIG {"IOSITY_SYSTEM_PATH"} . "iosity.pm";

#If file is not in current directory, changing reference information
if (rindex ($webpagefile, "/") >= 0)
{
	$VARIABLES {"ReferenceDirectory"} = substr ($webpagefile, 0, rindex ($webpagefile, "/") + 1);
	$webpagefile = substr ($webpagefile, rindex ($webpagefile, "/") + 1);
}

#Setting standard location value
$VARIABLES {"#PARAM Location"} = $webpagefile;

#Reopening stdout for the output filename
open (STDOUT, ">" . $filename);
$VARIABLES {"Mode"} = "OutputToHTMLFile";
&_lib_iosity_DrawLocation ();
close (STDOUT);

if ($VARIABLES {"Mode"} eq "OutputToWebpageFile")
{
	my $webpageoutputfilename = substr ($filename, 0, index ($filename, ".htm")) . ".webpage";
	print stderr "Warning: File " . $filename . " was written to " . $webpageoutputfilename . "!\n";
	print stderr "    ->   " . $VARIABLES {"Dynamic Object Files"} . "\n";
	system ("mv \"" . $filename . "\" \"" . $webpageoutputfilename . "\"");
}
