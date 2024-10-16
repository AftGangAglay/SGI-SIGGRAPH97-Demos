#!/usr/sbin/perl

#**************************************************************************
#*                                                                        *
#*            Copyright (c) 1996 Silicon Graphics, Inc.                   *
#*                      All Rights Reserved                               *
#*                                                                        *
#*         THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF SGI             *
#*                                                                        *
#* The copyright notice above does not evidence any actual of intended    *
#* publication of such source code, and is an unpublished work by Silicon *
#* Graphics, Inc. This material contains CONFIDENTIAL INFORMATION that is *
#* the property of Silicon Graphics, Inc. Any use, duplication or         *
#* disclosure not specifically authorized by Silicon Graphics is strictly *
#* prohibited.                                                            *
#*                                                                        *
#* RESTRICTED RIGHTS LEGEND:                                              *
#*                                                                        *
#* Use, duplication or disclosure by the Government is subject to         *
#* restrictions as set forth in subdivision (c)(1)(ii) of the Rights in   *
#* Technical Data and Computer Software clause at DFARS 52.227-7013,      *
#* and/or in similar or successor clauses in the FAR, DOD or NASA FAR     *
#* Supplement. Unpublished - rights reserved under the Copyright Laws of  *
#* the United States. Contractor is SILICON GRAPHICS, INC., 2011 N.       *
#* Shoreline Blvd., Mountain View, CA 94039-7311                          *
#**************************************************************************

# DEMO_base.cgi
#
# Basic information for the Web and Buttonfly interfaces  
# Contains the categories for organization and the ReadParse 
# subroutine for interpreting the Web Query
#
# David J. Anderson (sprout@lick.esd.sgi.com) 
# http://reality.sgi.com/sprout/


# Set the base directory that the demos source tree is installed under
# On most systems, this is /usr/demos

$basedir = "/usr/demos";

# These are the lists of elements from which the interface makes its Industries
# Features and Hardware tables.

# HARDWARE

@hardwarereq = ('Audio','Video','Video Compression','Hardware Texturing',
	'Dual Head','Three Pipe','High End Graphics','Stereo Graphics');

# FEATURES

@featurereq = ('Audio','Collaboration','Compression','GL','Open GL','Image Processing',
	'Inventor','Java','Multi-Processing','Networking','Performer',
	'Reflections','Rendering','Software Developement','Super-Computing',
	'Texture Mapping','Texture Paging','Video','Volume Rendering','VRML');

# MARKETS

@marketsreq = ('Animation','Architecture','Broadcast/ Film/ Video','CAD/ MCAD/ CFD/ MCAM',
	'Chemistry','Education','Financial/ Economic','Games','GIS',
	'Manufacturing Industries','Medical','Publishing','Spatialized Audio',
	'Scientific Visualization','Visual Simulation','World Wide Web');



# These are all the diferent tags avaliable in the .index files

@elements = ('TITLE','AUTHOR','DESCRIPTION','BRIEF_INSTRUCTIONS',
	'DETAILED_INSTRUCTIONS','OTHER_INFORMATION','SYSTEM_REQS','FEEDBACK',
	'MARKETS','FEATURES','HARDWARE');


# Redirect error messages to standard out

open(STDERR, '>&STOUT');
$| = 1;

# Create a list of all demos loaded into the UI.  
# It does this by going through through the General_Demos
# directory and finding all demo dirs that have all the required elements
# This routine DOES CHECK FOR WRITE PERMISSIONS unless you ask it by setting 
# the $_[0] = "CheckWrite".  It outputs the list into an array named @demos
# Note: any changes to this should probably also be changed 
# in DEMO_add.cgi and DEMO_add.tree.cgi

sub ListAllDemos {
	# Open up demos directory and grab all directories
	chdir("$basedir/General_Demos");
	opendir(DIR, ".");
	@alldirs = grep(-d, readdir(DIR));
	@alldirs = grep(!/^\.\.?$/, @alldirs);
	closedir(DIR);

	# For each dir, check for .index, thumbnail.jpg, header.gif, executable
	for (@alldirs) {
		$indexstate = "";
		$headerstate = "";
		$thumbstate = "";
		$demostate = "";
	
		opendir(DIR, "$_");
		@allfiles = readdir(DIR);
		closedir(DIR);

		# Check for .index
		if (grep(/^\.index$/, @allfiles)) {
			stat("$basedir/General_Demos/$_/.index");
			$indexstate = 1 if -w _;
		}
		# Check for header.jpg
		if (grep(/^header.jpg$/, @allfiles)) {
			stat("$basedir/General_Demos/$_/header.jpg");
  			$headerstate = 1 if -w _;
		}

		# Check for thumbnail.gif
		if (grep(/^thumbnail.gif$/, @allfiles)) {
			stat("$basedir/General_Demos/$_/thumbnail.gif");
			$thumbstate = 1 ;
		}
		# Check for RUN script
		if (grep(/^RUN$/, @allfiles)) {
			stat("$basedir/General_Demos/$_/RUN");
			$demostate = 1 if -x _;
		}
		# Check for demo.jpg
		if (grep(/^demo.jpg$/, @allfiles)) {
			stat("$basedir/General_Demos/$_/demo.jpg");
			$picstate = 1;
		}
		# If all elememts are there, add to an array of demos and print
		# results if so desired
		$dir = $_;
		if ( $demostate && $thumbstate && $headerstate && $indexstate && picstate) {
			push(@demos,$_);
		} else {
			if (!$thumbstate) {
				&PrintErrorLog("The file $basedir/General_Demos/$dir/thumbnail.gif is missing.");
			}
			if (!$demostate) {
				&PrintErrorLog("The file $basedir/General_Demos/$dir/RUN is missing or not in executable state.");
			}
			if (!$picstate) {
				&PrintErrorLog("The file $basedir/General_Demos/$dir/demo.jpg is missing.");
			}
			if (!$headerstate) {
				if (!&CheckCDLink("$dir")) {
					&PrintErrorLog("The file $basedir/General_Demos/$dir/header.jpg is missing or not in writable state.");
				} else {
					$headerstate = 1;
					&PrintErrorLog("The demo $dir is linked from the CD.");
				}
			}
			if (!$indexstate) {
				if (!&CheckCDLink("$dir")) {
					&PrintErrorLog("The file $basedir/General_Demos/$dir/.index is missing or not in writable state.");
				} else {
					$indexstate = 1;
					&PrintErrorLog("The demo $dir is linked from the CD.");
				}
			
			}
			if ( $demostate && $thumbstate && $headerstate && $indexstate && picstate) {
				push(@demos,$dir);
			}
		}
	}
	@demos = sort @demos;
	return(@demos);
}



# Keep the tabs on, and print to the error log
sub PrintErrorLog {
	$length = 500;
	open(LOG, "$basedir/Demo_Interfaces/Web/tables/demo_error.log");
	while(<LOG>) {
		$lines = $.;
	}
	close(LOG);
	($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime(time);
	@weekday = (Sun,Mon,Tues,Web,Thurs,Fri,Sat);
	$wday = @weekday[$wday];
	@month = (Jan,Feb,Mar,Apr,May,Jun,Jul,Aug,Sep,Oct,Nov,Dec);
	$mon = $month[$mon];
	$timestamp = "$wday, $mon $mday, $year @ $hour:$min:$sec";
	if ($lines > $length) {
		open(LOG, ">$basedir/Demo_Interfaces/Web/tables/demo_error.log");
	} else {
		open(LOG, ">>$basedir/Demo_Interfaces/Web/tables/demo_error.log");
	}
	print LOG "$timestamp $_[0]\n";
	close(LOG);
}


sub CheckCDLink {
	local($arg) = $_[0];
	$link = readlink("$basedir/General_Demos/$arg");
	if (defined $link) {
		if (grep(/\/CDROM/, $link)) {
			return 1;
		} else {
			return 0;
		}
	} else {
		return 0;
	}
}


# Read data in from the user with either GET or POST.  If they are using multiple
# selections, add these elements into one variable split by newline.  Output an
# associative array called %tags. Call subroutine ReadParse

sub ReadParse {
	local (*in) = @_ if @_;
	local ($i, $key, $val);

	# Read in text
	if (&MethGet) {
		$in = $ENV{'QUERY_STRING'};
	} elsif (&MethPost) {
		read(STDIN,$in,$ENV{'CONTENT_LENGTH'});
	}

	# Split text on the "&"
	@in = split(/[&;]/,$in);

	foreach $i (0 .. $#in) {
		# Convert plus's to spaces
		$in[$i] =~ s/\+/ /g;

		# Convert hexadecimal numbers
		$in[$i] =~ s/%(..)/pack("c",hex($1))/ge;

		# Split into key and value.
		($key, $val) = split(/=/,$in[$i],2); # splits on the first =.

		# Make associative array, with check for multiple selections
		if($in{$key}) { $in{$key} .= "\n$val" }
		else { $in{$key} = $val; }
    
	}
return scalar(@in);
}


# MethGet
# Return true if this cgi call was using the GET request, false otherwise

sub MethGet {
  return ($ENV{'REQUEST_METHOD'} eq "GET");
}


# MethPost
# Return true if this cgi call was using the POST request, false otherwise

sub MethPost {
  return ($ENV{'REQUEST_METHOD'} eq "POST");
}

sub Alert {
	$ENV{'DISPLAY'} = "localhost:0.0";
	`/usr/bin/X11/xconfirm -t "$_[0]">/dev/null&`;
}

print "";
