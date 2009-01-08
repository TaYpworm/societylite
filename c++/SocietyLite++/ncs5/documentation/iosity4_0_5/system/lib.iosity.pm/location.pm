################################################################################
#   location.pm - HTML Builder in IOsity v4.0
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

sub _lib_iosity_DrawLocation ()
{
	#Fails if location contains "../"
	sub VerifyLocationSecurity ()
	{
		if ($VARIABLES {"DisableSecurity"} eq "true") { return 1; }
	
		if (length ($VARIABLES {"#PARAM Location"}) == 0) {return 0; }
		elsif (index ($VARIABLES {"#PARAM Location"}, "../") != -1) { return 0; }
		elsif (substr ($VARIABLES {"#PARAM Location"}, length ($VARIABLES {"#PARAM Location"}) - 1) eq "/") { return 0; }
		return 1;
	}

	#Returns components of .webpage file, checks for file existence
	#Format:
	#   1: semicolon or template filename
	#   2: semicolon or variable filename
	#   3: semicolon or list of object files seperated by commas
	#   4...: webpage file data
	#         if the webpage file specifies a template, the webpage file is a variable file
	#         if the webpage file does not specify a template, the webpage file is a template file
	sub OpenWebpageFile ()
	{
		my $filename = $_[0];
		
		my $template, $variables, $objects, $data;
		
		#Location file exists
		if (!&_lib_iosity_FileExists ($filename))
		{
			$VARIABLES {"#PARAM Location"} = "/BrokenLink";
			&_lib_iosity_DrawLocation ();
			exit (0);
		}

		open (fHANDLE, $filename);
		$template  = (<fHANDLE>); chop ($template);
		$variables = (<fHANDLE>); chop ($variables);
		$objects   = (<fHANDLE>); chop ($objects);
		$data = join ("", (<fHANDLE>));
		close (fHANDLE);
		
		return ($template, $variables, $objects, $data);
	}

	#Returns data from a file, checks for file existence
	sub TestOpenFile ()
	{
		my $filename = $_[0];
		my $type     = $_[1];
	
		my $filename = &_lib_iosity_SetFilename ($filename, $type);
		if (&_lib_iosity_FileExists ($filename))
		{
			open (fHANDLE, $filename);
			$data = join ("", (<fHANDLE>));
			close (fHANDLE);
		}
		else
		{
			$VARIABLES {"#PARAM Location"} = "/ProcessingError";
			&_lib_iosity_DrawLocation ();
			exit (0);
		}
		
		return $data;
	}

	#Adds comment with various page/connection information
	sub PrintConnectionInfo ()
	{
		my $filesize = sprintf ("%.2f", (length ($_[0]) / 1024.0));
		my $processingtime = time () - $CONFIG {"REQUEST_TIME"};

		print "\n\n<!--\n";
		print "IOsity Page/Connection Information\n";
		print "----------------------------------\n";
		print "Location:        " . $VARIABLES {"#PARAM Location"} . "\n";
		print "HTML Filesize:   " . $filesize . "KB\n";
		print "Processing Time: " . $processingtime . "s\n";
		print "-->\n";
	}

	#Places data from variables file into global %VARIABLES associative array 
	#Format: variablename<=>value<;>variablename<=>value<;>...
	sub ParseVariables ()
	{
		my $variables = $_[0];

		my @combos = split (/<;>\s*/, $variables);
		foreach my $combo (@combos)
		{
			my $index = index ($combo, "<=>");
			if ($index >= 0)
			{
				$VARIABLES {substr ($combo, 0, $index)} = substr ($combo, $index + 3);
			}
		}
	}

	#Replaces IOsity tags (i.e. "<#PARAM Location>") with their values, iterative up to 10 levels
	sub ReplaceValues ()
	{
		my $template = $_[0];
		my $changed;
		my $level = 0;
		my $found;
		do
		{
			$found = 0;
			foreach my $variable (keys %VARIABLES)
			{
				my $value = $VARIABLES {$variable};
				if (($template =~ s/<$variable>/$value/g) > 0) { $found = 1; }
			}
			$level++;

			($template, $changed) = &ProcessObjects ($template);
		} while (($found || $changed) && $level < 10);
		return $template;
	}

	#Loads IOsity objects into memory
	sub LoadObjects ()
	{
		foreach my $object (@OBJECTS)
		{
			my $filename = &_lib_iosity_SetFilename ($object, "object");
			if (&_lib_iosity_FileExists ($filename))
			{
				do $filename;
			}
			else
			{
				$VARIABLES {"#PARAM Location"} = "/ProcessingError";
				&_lib_iosity_DrawLocation ();
				exit (0);
			}
		}
	}

	#Returns the name (first) parameter
	#Format parameter,parameter...
	sub GetName ()
	{
		my $parameters = $_[0];
		if ($parameters ne "")
		{
			my @parameters = split (/,\s*/, $parameters);
			return substr ($parameters[0], 1, length ($parameters[0]) - 2);
		}
		return "";
	}

	#Calls object functions with corresponding parameters and replaces the tag with the result
	sub ProcessObjects ()
	{
		my $template = $_[0];
		my $changed = 0;
	
		foreach my $object (@OBJECTS)
		{
			my $objectname = substr ($object, rindex ($object, "/") + 1);
			my $index;
			do
			{
				if ($objectname ne "ObjectBasecode")
				{
					$index = index ($template, "<#" . $objectname . " ");
				}
				else
				{
					$index = index ($template, "<#" . $objectname);
				}
				my $endindex  = index ($template, ">", $index);
				
				my $parameters;				
				if ($endindex > $index + length ($objectname) + 2) { $parameters = substr ($template, $index + length ($objectname) + 3, $endindex - ($index + length ($objectname) + 3)); }

				if ($VARIABLES {"#PARAM SendEvent_On"} eq "true" && &GetName ($parameters) eq $VARIABLES {"#PARAM SendEvent_Object"})
				{
					$parameters = "\"" . $VARIABLES {"#PARAM SendEvent_Event"} . "\", \"" . $VARIABLES {"#PARAM SendEvent_Value"} . "\", " . $parameters;
					$parameters = "&object_" . $objectname . "_handleEvent (" . $parameters . ")";
				}

				if ($index >= 0)
				{
					$changed = 1;
					my $functioncall = "&object_" . $objectname . " (" . $parameters . ");";
					my $functionoutput = eval ($functioncall);
					if ($functionoutput eq "Switch Modes")
					{
						$template = substr ($template, 0, $index) . "<[#" . substr ($template, $index + 2, $endindex - ($index + 2)) . "]>" . substr ($template, $endindex + 1);
						if ($VARIABLES {"Dynamic Object Files"} ne "")
						{
							$VARIABLES {"Dynamic Object Files"} .= ",";
						}
						$VARIABLES {"Dynamic Object Files"} .= $object;
					}
					else
					{
						$template = substr ($template, 0, $index) . $functionoutput . substr ($template, $endindex + 1);
					}
				}
			} while ($index != -1);
		}
		
		return  ($template, $changed);
	}

#===============================================================================
	if (!&VerifyLocationSecurity ())
	{
		$VARIABLES {"#PARAM Location"} = "/SecurityBreach";
		&_lib_iosity_DrawLocation ();
		exit (0);
	}

	my $filename = &_lib_iosity_SetFilename ($VARIABLES {"#PARAM Location"}, "webpage");

	my ($template, $variables, $objects, $data) = &OpenWebpageFile ($filename);

#===============================================================================
	my $webpagefiletype = "variable";
	#If no template is specified, the webpage file is a template file
	if ($template eq ";")
	{
		$webpagefiletype = "template";
		$template = $data;
	}
	#If a template is specified, the webpage file is a variable file
	else
	{
		$template = &TestOpenFile ($template, "template");
	}

#===============================================================================
	if ($variables eq ";")
	{
		$variables = "";
	}
	else
	{
		$variables = &TestOpenFile ($variables, "variables");
	}

	if ($webpagefiletype eq "variable")
	{
		$variables .= $data;
	}

	if ($objects ne ";")
	{
		my @objects = split (",", $objects);
		foreach my $object (@objects)
		{
			$OBJECTS[@OBJECTS] = $object;
		}
	}

	&LoadObjects ();

	&ParseVariables ($variables);

	$template = &ReplaceValues ($template);

#===============================================================================

	if ($VARIABLES {"Mode"} eq "")
	{
		print "Content-type: text/html\n\n";
		print $template;
		if ($DEBUG ne "") { print "<BR><BR><HR><BR>" . $DEBUG; }

		&PrintConnectionInfo ($template);
	}
	elsif ($VARIABLES {"Mode"} eq "OutputToHTMLFile")
	{
		print $template;
		if ($DEBUG ne "") { print "<BR><BR><HR><BR>" . $DEBUG; }
	}
	elsif ($VARIABLES {"Mode"} eq "OutputToWebpageFile")
	{
		my $index;
		do
		{
			$index        = index ($template, "<[#");
			my $endindex  = index ($template, "]>", $index);

			$template = substr ($template, 0, $index) . "<#" . substr ($template, $index + 3, $endindex - ($index + 3)) . ">" . substr ($template, $endindex + 2);
		} while ($index != -1);

		print ";\n;\n" . $VARIABLES {"Dynamic Object Files"} . "\n" . $template;
		if ($DEBUG ne "") { print "<BR><BR><HR><BR>" . $DEBUG; }
	}
}
