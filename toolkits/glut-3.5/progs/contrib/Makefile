#!smake
#
# Copyright (c) Mark J. Kilgard, 1995, 1997.
#

TOP = ../..
include $(TOP)/glutdefs
include $(ROOT)/usr/include/make/commondefs

LN = ln -s
MV = mv
RM = -rm -rf

DATAFILES =

TARGETS = lineblend worms rings agv_example fractals hanoi hanoi2 \
   gears noof moth text3d steam

LLDLIBS = $(GLUT) -lGLU -lGL -lXmu -lXext -lX11 -lm

SRCS = lineblend.c worms.c rings.c fractals.c fracviewer.c agviewer.c \
       agv_example.c hanoi.c hanoi2.c engine.c moth.c text3d.c steam.c
OBJS = $(SRCS:.c=.o)

LCOPTS = -I$(TOP)/include -fullwarn
LWOFF = ,813,852,827,826,1506
LDIRT = *~ *.bak *.pure

default : $(TARGETS)

agv_example: agv_example.o agviewer.o
	$(CC) -o $@ agv_example.o agviewer.o $(LDFLAGS)

hanoi2: hanoi2.o engine.o
	$(CC) -o $@ hanoi2.o engine.o $(LDFLAGS)

fractals: fractals.o fracviewer.o
	$(CC) -o $@ fractals.o fracviewer.o $(LDFLAGS)

include $(COMMONRULES)

INSTALL_DIR = /usr/demos/SIGGRAPH97/toolkits/glut-3.5/progs/contrib

install:: $(TARGETS)
	if test ! -d $(INSTALL_DIR); then \
	    mkdir -p $(INSTALL_DIR); \
	fi; \
	for file in $(TARGETS) $(DATAFILES); do \
	    rm -f $(INSTALL_DIR)/$$file; \
	    curdir=`pwd`; \
	    ln -s $$curdir/$$file $(INSTALL_DIR)/$$file; \
	done
