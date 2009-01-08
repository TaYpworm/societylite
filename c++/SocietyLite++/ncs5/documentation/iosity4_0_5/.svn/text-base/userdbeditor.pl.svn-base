#!/usr/bin/perl

################################################################################
#   userdbeditor.pl - User Database Editor in IOsity v4.0
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

if (@ARGV != 1)
{
	print "You must specify a filename for the user database!\n";
	print "Usage: userdbeditor.pl filename\n";
	exit (1);
}

my $filename = $ARGV[0];

open (fHANDLE, "+<" . $filename);
flock (fHANDLE, 2);

my @records = split ("<;>\n", join ("", (<fHANDLE>)));

my $mainmenuchoice;
do
{
	print "IOsity User Database Editor\n";
	print "----------------------------------------\n";
	print "1. Add User\n";
	print "2. Remove User\n";
	print "3. Edit User\n";
	print "4. List by First/Last/User Name\n";
	print "\n";
	print "Q. Quit\n";
	print "----------------------------------------\n";
	print "> ";
	$mainmenuchoice = (<stdin>); chop ($mainmenuchoice);
	
	# 0            1           2           3          4             5           6        7
	#ID<,>First Name<,>Last Name<,>User Name<,>Password<,>EncPassword<,>Last Used<,>Groups<;>

	#Main Menu: Add User
	if ($mainmenuchoice eq "1")
	{
		print "Adding User\n";
		print "----------------------------------------\n";
	
		my $idisunique = 0;
		my $id;
		do
		{
			print "Unique ID#: ";
			$id = (<stdin>); chop ($id);

			$idisunique = 1;
			foreach my $record (@records)
			{
				my @fields = split ("<,>", $record);
				
				if ($fields[0] eq $id)
				{
					$idisunique = 0;
					print "Please enter a unique ID#\n";
				}
			}
		} while (!$idisunique);
		
		print "First Name: ";
		my $firstname = (<stdin>); chop ($firstname);
		
		print "Last Name: ";
		my $lastname = (<stdin>); chop ($lastname);
		
		my $usernamenumber = "";
		my $usernameisunique = 0;
		my $username;
		do
		{
			my $defaultusername = lc (substr ($firstname, 0, 1)) . lc ($lastname) . $usernamenumber;
			print "User Name (Enter = " . $defaultusername . "): ";
			$username = (<stdin>); chop ($username);
			if ($username eq "")
			{
				$username = $defaultusername;
			}
			
			$usernameisunique = 1;
			foreach my $record (@records)
			{
				my @fields = split ("<,>", $record);
				
				if ($fields[3] eq $username)
				{
					print "Username \'" . $username . "\' is not unique, continue? (y/n): ";
					my $continueconfirmchoice = (<stdin>); chop ($continueconfirmchoice);
					
					if (uc ($continueconfirmchoice) eq "N")
					{
						$usernameisunique = 0;
						$usernamenumber++;
					}
				}
			}
		} while (!$usernameisunique);
		
		print "Password (Enter = random): ";
		my $password = (<stdin>); chop ($password);
		if ($password eq "")
		{
			$password = chr (rand () * 26 + 65) . chr (rand () * 26 + 65) . chr (rand () * 26 + 65) . chr (rand () * 26 + 65) . chr (rand () * 26 + 65) . chr (rand () * 26 + 65);
			print "Password is " . $password . "\n";
		}
		
		print "Groups (Enter = user): ";
		my $groups = (<stdin>); chop ($groups);
		if ($groups eq "")
		{
			$groups = "user";
		}

		open  (fHANDLE, ">>" . $filename);
		
		my $output = $id . "<,>" . $firstname . "<,>" . $lastname . "<,>" . $username . "<,>" . crypt ($password, uc (substr ($password, 0, 2))) . "<,><,><,>" . $groups;
		$records[@records] = $output;
		
		print fHANDLE $output . "<;>\n";
		
		close (fHANDLE);
	}
	#Main Menu: Remove User
	elsif ($mainmenuchoice eq "2")
	{
		my $removeusermenuchoice;
		do
		{
			print "Removing User\n";
			print "----------------------------------------\n";
			print "1. Select by ID#\n";
			print "2. List by First/Last/User Name\n";
			print "\n";
			print "C. Cancel/Done\n";
			print "----------------------------------------\n";
			print "> ";
			$removeusermenuchoice = (<stdin>); chop ($removeusermenuchoice);

			#Remove User Menu: Select by ID#
			if ($removeusermenuchoice eq "1")
			{
				print "ID#: ";
				my $searchid = (<stdin>); chop ($searchid);

				my $output = "";
				foreach my $record (@records)
				{
					my @fields = split ("<,>", $record);
					if ($fields[0] ne $searchid)
					{
						$output .= $record . "<;>\n";
					}
					else
					{
						print "Really delete user record? (y/n): ";
						my $deleteconfirmchoice = (<stdin>); chop ($deleteconfirmchoice);
						if (uc ($deleteconfirmchoice) eq "N")
						{
							$output .= $record . "<;>\n";
						}
					}
				}
				
				@records = split ("<;>\n", $output);
				
				seek (fHANDLE, 0, SEEK_SET);
				print fHANDLE $output;
				truncate (fHANDLE, length ($output));
			}
			#Remove User Menu: List by First/Last/User Name
			elsif ($removeusermenuchoice eq "2")
			{
				&ListByName ();
			}
		} while (uc ($removeusermenuchoice) ne "C");
	}
	#Main Menu: Edit User
	elsif ($mainmenuchoice eq "3")
	{
		my $editusermenuchoice;
		do
		{
			print "Editing User\n";
			print "----------------------------------------\n";
			print "1. Select by ID#\n";
			print "2. List by First/Last/User Name\n";
			print "\n";
			print "C. Cancel/Done\n";
			print "----------------------------------------\n";
			print "> ";
			$editusermenuchoice = (<stdin>); chop ($editusermenuchoice);

			#Edit User Menu: Select by ID#
			if ($editusermenuchoice eq "1")
			{
				print "ID#: ";
				my $searchid = (<stdin>); chop ($searchid);
				
				my $output = "";
				foreach my $record (@records)
				{
					my @fields = split ("<,>", $record);
					if ($fields[0] ne $searchid)
					{
						$output .= $record . "<;>\n";
					}
					else
					{
						print "Unique ID# (Enter = \'" . $fields[0] . "\'): ";
						my $id = (<stdin>); chop ($id);
						if ($id ne "")
						{
							$fields[0] = $id;
						}

						print "First Name (Enter = \'" . $fields[1] . "\'): ";
						my $firstname = (<stdin>); chop ($firstname);
						if ($firstname ne "")
						{
							$fields[1] = $firstname;
						}
						
						print "Last Name (Enter = \'" . $fields[2] . "\'): ";
						my $lastname = (<stdin>); chop ($lastname);
						if ($lastname ne "")
						{
							$fields[2] = $lastname;
						}

						print "User Name (Enter = \'" . $fields[3] . "\'): ";
						my $username = (<stdin>); chop ($username);
						if ($username ne "")
						{
							$fields[3] = $username;
						}

						print "Password (Enter = \'********\'): ";
						my $password = (<stdin>); chop ($password);
						if ($password ne "")
						{
							$fields[4] = crypt ($password, uc (substr ($password, 0, 2)));
						}

						print "Groups (Enter = \'" . $fields[7] . "\'): ";
						my $groups = (<stdin>); chop ($groups);
						if ($groups ne "")
						{
							$fields[7] = $groups;
						}
						
						print "Really edit user record? (y/n): ";
						my $editconfirmchoice = (<stdin>); chop ($editconfirmchoice);
						if (uc ($editconfirmchoice) eq "Y")
						{
							$output .= join ("<,>", @fields) . "<;>\n";
						}
						else
						{
							$output .= $record . "<;>\n";
						}
					}
				}
				
				@records = split ("<;>\n", $output);
				
				seek (fHANDLE, 0, SEEK_SET);
				print fHANDLE $output;
				truncate (fHANDLE, length ($output));
			}
			#Edit User Menu: List by First/Last/User Name
			elsif ($editusermenuchoice eq "2")
			{
				&ListByName ();
			}
		} while (uc ($editusermenuchoice) ne "C");
	}
	elsif ($mainmenuchoice eq "4")
	{
		&ListByName ();
	}
} while (uc ($mainmenuchoice) ne "Q");

flock (fHANDLE, 8);
close (fHANDLE);

sub ListByName ()
{
	print "Name: ";
	my $searchname = (<stdin>); chop ($searchname);
	
	print "           /--------------------------------------------------------\\\n";
	print "           |ID#      First Name      Last Name       User Name      |\n";
	print "           |========================================================|\n";
	foreach my $record (sort @records)
	{
		my @fields = split ("<,>", $record);
		if ($fields[1] =~ /$searchname/i || $fields[2] =~ /$searchname/i || $fields[3] =~ /$searchname/i)
		{
			printf ("           |%-8s %-15s %-15s %-15s|\n", $fields[0], $fields[1], $fields[2], $fields[3]);
		}
	}
	print "           |                                                        |\n";
	print "           |                     End of Records                     |\n";
	print "           \\--------------------------------------------------------/\n";
}
