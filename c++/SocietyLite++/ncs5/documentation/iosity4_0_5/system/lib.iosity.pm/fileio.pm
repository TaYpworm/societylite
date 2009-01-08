################################################################################
#   fileio.pm - File I/O Manager in IOsity v4.0
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

#Fails if file does not exist
sub _lib_iosity_FileExists ()
{
	my $filename = $_[0];
	(-e $filename) ? return 1 : return 0;
}

#Checks security and location file direction information
sub _lib_iosity_SetFilename ()
{
	my $location = $_[0];
	my $type     = $_[1];

	my $filename;
	#Directing for root files
	if (substr ($location, 0, 1) eq "/")
	{
		$filename = $CONFIG {"IOSITY_SYSTEM_PATH"} . "data" . $location;
	}
	#Directing for normal files
	elsif ($VARIABLES {"DisableSecurity"} ne "true")
	{
			$filename = "../data/" . $location;
	}
	#Directing for normal files in disabled security mode
	else
	{
		$filename = $VARIABLES {"ReferenceDirectory"} . $location;
	}
	return $filename . "." . $type;
}

#Returns data from a file, checks for file existence
sub _lib_iosity_OpenFile ()
{
	my $filename = $_[0];
	my $type     = $_[1];

	my $filename = &_lib_iosity_SetFilename ($filename, $type);
	open (fHANDLE, $filename);
	$data = join ("", (<fHANDLE>));
	close (fHANDLE);
	
	return $data;
}
