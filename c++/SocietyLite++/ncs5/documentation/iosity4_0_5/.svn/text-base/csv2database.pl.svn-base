#!/usr/bin/perl

################################################################################
#   csv2database.pl - Database File Converter in IOsity v4.0
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

sub PrintCenter ()
{
	my $text = $_[0];
	my $spaces = (80 - length ($text)) / 2;
	my $offset;
	for (my $index = 0; $index < $spaces; $index++)
	{
		$offset .= " ";
	}
	print $offset . $text . "\n";
}

sub charat ()
{
	my $string = $_[0];
	my $index  = $_[1];
	return substr ($string, $index, 1);
}

sub GetNextElementLong ()
{
	my $row   = $_[0];
	my $index = $_[1];
	my $element;

	my $endindex = $index;
	my $success = 0;
	do
	{
		$endindex = index ($row, "\",", $endindex);
		if ($endindex < 0)
		{
			$endindex = length ($row) + 1;
		}
		else
		{
			$endindex++;
		}

		$element = substr ($row, $index + 1, $endindex - $index - 2);

		$element =~ s/\"\"//g;
		
		print $element . "\n";
		
		if (index ($element, "\"") < 0)
		{
			$element = substr ($row, $index + 1, $endindex - $index - 2);
			$element =~ s/\"\"/\"/g;
			$success = 1;
		}
	} while (!$success);

	return ($element, $endindex);	
}

sub GetNextElementShort ()
{
	my $row   = $_[0];
	my $index = $_[1];

	my $endindex = index ($row, ",", $index);
	if ($endindex < 0)
	{
		$endindex = length ($row) + 1;
	}

	return (substr ($row, $index, $endindex - $index), $endindex);
}

sub SeperateElements ()
{
	my $row = $_[0];
	my @elements;
	my $index = -1;
	
	while ($index < length ($row))
	{
		if (&charat ($row, $index + 1) eq "\"")
		{
			($elements[@elements], $index) = &GetNextElementLong ($row, $index + 1);
		}
		else
		{
			($elements[@elements], $index) = &GetNextElementShort ($row, $index + 1);
		}
	}
	
	return @elements;
}

sub ConvertFile_csv2database ()
{
	my $filename = $_[0];
	if (!(-e $filename))
	{
		print "\n\nUnable to read file " . $filename . "\n\n";
		return 0;
	}

	open (fHANDLE, $filename);
	my $data = join ("", (<fHANDLE>));
	close (fHANDLE);

	my @rows = split ("\r", $data);
	
	open (fHANDLE, ">" . substr ($filename, 0, length ($filename) - 3) . "database");
	foreach my $row (@rows)
	{
		print fHANDLE join ("<,>", &SeperateElements ($row)) . "<;>\n";
	}
	close (fHANDLE);
}

sub main ()
{
	system ("clear");

	&PrintCenter ("IOsity v4.0 csv2database Conversion Software");
	&PrintCenter ("Created and maintained by: fokno.org");
	&PrintCenter ("©2001, 2002 fokno.org, all rights reserved");
	print "--------------------------------------------------------------------------------\n\n";
	print "^c to exit\n";

	while (1)
	{
		print "Enter csv filename: ";
		my $filename = (<STDIN>); chop ($filename);
		&ConvertFile_csv2database ($filename);
	}
}

&main ();
