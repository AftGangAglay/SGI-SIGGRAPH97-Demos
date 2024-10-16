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

# DEMO_execute.cgi
#
# Execute a Demo
#
# David J. Anderson (sprout@lick.esd.sgi.com) 
# http://reality.sgi.com/sprout/


# Get the required scripts
require "DEMO_base.cgi";

# Read info from viewer
&ReadParse;

# Print header
print "Content-type: text/html\n\n";

$debug = 0;
$basedir = "/usr/demos/SIGGRAPH97";

if (%in) {
        # Set the DISPLAY environment variable
	printf ("setting DISPLAY to localhost:0.0<BR>\n") if ($debug);
        $ENV{'DISPLAY'} = "localhost:0.0";

	$demodir = $in{'demodir'};
	printf ("demodir = $demodir<BR>\n") if ($debug);
        # Run demo from the correct directory
	printf ("chdir (${basedir}/${demodir}<BR>\n") if ($debug);
	chdir("${basedir}/${demodir}") || do {
		printf ("<P>chdir failed: $!</P>\n");
		exit(0);
	};
	$demo = $in{'demo'};
	printf ("running $demo<BR>\n");

	if (open(PIPE, "./${demo} |")) {
	    printf ("<pre>\n");
	    while(<PIPE>) {
		printf("%s", $_);
	    }
	    close(PIPE);
	    printf ("</PRE>\n");
	} else {
	    printf ("<P> Could not execute '${demo}': $!</P>\n");
	}
#	`rm -f /usr/tmp/demo.out`;
#	`./${demo} 1>/usr/tmp/demo.out 2>&1`;
#	print  `cat /usr/tmp/demo.out`;
	print "<br></html>\n";

#	print `./${demo} &` "</html>\n";
#	`./${demo} & > /dev/null`;
	
}
