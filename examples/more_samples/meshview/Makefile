#!smake
##
##  Makefile for SGI
##


GLUTHOME = ../../lib/glut


# defines
CFLAGS	= -g -I$(GLUTHOME)
LIBS	= -L$(GLUTHOME) -lglut -lGLU -lGL -lXext -lX11 -lXmu -lm
SRCS	= meshview.c sine.c sinc.c
EXES	= $(SRCS:.c=)


# default rule
default	: $(EXES)


# cleanup rules
clean	:
	rm -f *.o *~ #* .~*

clobber	: 
	rm -f $(EXES)


# inference rules
meshview: meshview.o
	$(CC) $(CFLAGS) -o $@ $@.o trackball.o $(LIBS)
sine	: sine.o
	$(CC) $(CFLAGS) -o $@ $@.o $(LIBS)
sinc	: sinc.o
	$(CC) $(CFLAGS) -o $@ $@.o $(LIBS)
.c.o	: 
	$(CC) $(CFLAGS) -c $<


# dependencies (must come AFTER inference rules)
$(EXES)		: trackball.o
trackball.o	: trackball.h

TARGETS = $(EXES)
INSTALL_DIR = /usr/demos/SIGGRAPH97/examples/more_samples
DATAFILES = *.dat

install:: $(TARGETS)
	if test ! -d $(INSTALL_DIR); then \
	    mkdir -p $(INSTALL_DIR); \
	fi; \
	for file in $(EXES) $(DATAFILES); do \
	    rm -f $(INSTALL_DIR)/$$file; \
	    curdir=`pwd`; \
	    ln -s $$curdir/$$file $(INSTALL_DIR)/$$file; \
	done



