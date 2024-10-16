#!smake
##
##  Makefile for SGI
##


GLUTHOME = ../../lib/glut


# defines
CFLAGS	= -g -I$(GLUTHOME)
LIBS	= -L$(GLUTHOME) -lglut -lGLU -lGL -lXext -lX11 -lXmu -lm
SRCS	= shadow.c
EXES	= $(SRCS:.c=)


# default rule
default	: $(EXES)


# cleanup rules
clean	:
	rm -f *.o *~ #* .~*

clobber	: 
	rm -f $(EXES)


# inference rules
$(EXES)	: $(SRCS:.c=.o)
	$(CC) $(CFLAGS) -o $@ $? $(LIBS)
.c.o	: 
	$(CC) $(CFLAGS) -c $<


# dependencies (must come AFTER inference rules)
$(EXES)		: trackball.o glm.o gltx.o
trackball.o	: trackball.h
glm.o		: glm.h
gltx.o		: gltx.h

TARGETS = $(EXES)
INSTALL_DIR = /usr/demos/SIGGRAPH97/examples/more_samples
DATADIR = data
DATAFILES = data/*

install:: $(TARGETS)
	if test ! -d $(INSTALL_DIR)/$(DATADIR); then \
	    mkdir -p $(INSTALL_DIR)/$(DATADIR); \
	fi; \
	for file in $(DATAFILES); do \
	    file=`basename $$file`; \
	    rm -f $(INSTALL_DIR)/$(DATADIR)/$$file; \
	    curdir=`pwd`; \
	    ln -s $$curdir/$(DATADIR)/$$file $(INSTALL_DIR)/$(DATADIR)/$$file; \
	done; \
	for file in $(EXES); do \
	    rm -f $(INSTALL_DIR)/$$file; \
	    curdir=`pwd`; \
	    ln -s $$curdir/$$file $(INSTALL_DIR)/$$file; \
	done

