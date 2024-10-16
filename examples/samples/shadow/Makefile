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
