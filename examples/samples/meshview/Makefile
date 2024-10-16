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
