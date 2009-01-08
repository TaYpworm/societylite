################################################################################
#   parameters.pm - Parameter Parser in IOsity v4.0
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

#Stores parameter values into global %VARIABLES associative array
#For standard parameter values the variable name will be preceeded by "#PARAM "
#For file upload values the variable name for data will be preceeded by "#PARAM "
#For file upload values the variable name for filename data will be preceeded by "#FILENAME "
sub _lib_iosity_LoadParameters ()
{
	#Loading parameters for standard get/post methods
	my $ENCTYPE;
	($ENCTYPE) = split (";", $ENV {"CONTENT_TYPE"});
	if ($ENCTYPE ne "multipart/form-data")
	{
		#Parameter format: variablename=value&variablename=value&...
		if ($ENV {"REQUEST_METHOD"} eq "POST") { read (STDIN, $variables, $ENV {"CONTENT_LENGTH"}); }
		else { $variables = $ENV {"QUERY_STRING"}; }
	
		my @param = split ("&", $variables);
		foreach my $param (@param)
		{
			my @parse = split ("=", $param);
			
			$parse[0] =~ s/\+/ /g;
			$parse[0] =~ s/%(..)/pack("c", hex($1))/ge;
			$parse[1] =~ s/\+/ /g;
			$parse[1] =~ s/%(..)/pack("c", hex($1))/ge;
			$VARIABLES {"#PARAM " . $parse[0]} = $parse[1];
		}
	}
	#Loading parameters for multipart/form-data post method
	else
	{
		my $seperator = (<STDIN>);
		my @data      = (<STDIN>);
	
		#Forces compatibility with Windows/Mac OS/UNIX
		my $linebreak;
		if    (substr ($seperator, length ($seperator) - 2, 2) eq "\r\n") { $linebreak = "\r\n"; $seperator = "\r\n" . $seperator; $data[@data - 2] = substr ($data[@data - 2], 0, length ($data[@data - 2]) - 2); }
		elsif (substr ($seperator, length ($seperator) - 1, 1) eq "\n")   { $linebreak = "\n";   $seperator = "\n"   . $seperator; $data[@data - 2] = substr ($data[@data - 2], 0, length ($data[@data - 2]) - 1); }
		elsif (substr ($seperator, length ($seperator) - 1, 1) eq "\r")   { $linebreak = "\r";   $seperator = "\r"   . $seperator; $data[@data - 2] = substr ($data[@data - 2], 0, length ($data[@data - 2]) - 1); }
	
		$data[@data - 1] = "";
	
		my $data = join ("", @data);
		@data = split (/$seperator/, $data);
		
		foreach my $data (@data)
		{
			my ($param) = split ($linebreak, $data);
			my @parse = split (/\;\s*/, $param);
	
			my $current;
			($trash, $current) = split ("=", $parse[1]);
			$current = substr ($current, 1);
			$current = substr ($current, 0, length ($current) - 1);
			
			my $store;
	
			if ($parse[2] eq "")
			{
				($trash1, $trash2) = split ($linebreak, $data);
				$store = substr ($data, length ($trash1) + length ($linebreak) + length ($trash2) + length ($linebreak));

				$current =~ s/\+/ /g;
				$current =~ s/%(..)/pack("c", hex($1))/ge;
				$store   =~ s/\+/ /g;
				$store   =~ s/%(..)/pack("c", hex($1))/ge;
			}
			else
			{
				my $filename = "";
				if (index ($parse[2], "\\") > -1)
				{
					$filename = substr ($parse[2], rindex ($parse[2], "\\") + 1);
					$filename = substr ($filename, 0, length ($filename) - 1);
				}
				else
				{
					$filename = substr ($parse[2], index ($parse[2], "\"") + 1);
					$filename = substr ($filename, 0, length ($filename) - 1);
				}

				$filename =~ s/\+/ /g;
				$filename =~ s/%(..)/pack("c", hex($1))/ge;
	
				$VARIABLES {"#FILENAME " . $current} = $filename;
				
				($trash1, $trash2, $trash3) = split ($linebreak, $data);
				if ($trash2 eq "")
				{
					$store = substr ($data, length ($trash1) + length ($linebreak) + length ($trash2) + length ($linebreak));
				}
				else
				{
					$store = substr ($data, length ($trash1) + length ($linebreak) + length ($trash2) + length ($linebreak) + length ($trash3) + length ($linebreak));
				}
			}
	
			$VARIABLES {"#PARAM " . $current} = $store;
		}
	}
}
