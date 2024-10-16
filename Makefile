#!smake
#
# This is the top-level Makefile that sets up the web-based demos.
# Run "make install" as root.
#
# To run the web pages, the scripts in cgi-bin need to be installed
# in the local server's cgi-bin.
#

SUBDIRS = \
	courses \
	examples/redbook \
	examples/samples \
	examples/more_samples \
	toolkits/glut-3.5/progs/advanced \
	toolkits/glut-3.5/progs/contrib \
	toolkits/glut-3.5/progs/demos \
	toolkits/glut-3.5/progs/examples \
	toolkits/glut-3.5/progs/texfont \


include $(ROOT)/usr/include/make/commondefs

$(COMMONTARGS) default install: $(_FORCE)
	$(SUBDIRS_MAKERULE)

$(_FORCE):

$(SUBDIRS): $(_FORCE)
	cd $@; $(MAKE)


