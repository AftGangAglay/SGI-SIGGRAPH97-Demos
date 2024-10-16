#!smake
#
# Copyright (c) Mark J. Kilgard, 1996, 1997.
#

TOP = ../../..
include $(TOP)/glutdefs
include $(ROOT)/usr/include/make/commondefs

LN = ln -s
MV = mv
RM = -rm -rf

DATA_FILES = chess.inp

TARGETS = chess

LLDLIBS = $(GLUT) -lGLU -lGL -lXmu -lXext -lX11 -lm

SRCS = chess.c main.c pathplan.c animate.c texture.c
OBJS =  $(SRCS:.c=.o)

LCOPTS = -I$(TOP)/include -fullwarn
LWOFF = ,813,852,827,826
LDIRT = *~ *.bak *.pure $(TARGETS)

default : $(TARGETS)

chess : $(OBJS) $(GLUT)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

include $(COMMONRULES)

INSTALL_DIR = /usr/demos/SIGGRAPH97/toolkits/glut-3.5/progs/demos

install:: $(TARGETS)
	if test ! -d $(INSTALL_DIR); then \
	    mkdir -p $(INSTALL_DIR); \
	fi; \
	for file in $(TARGETS) $(DATA_FILES); do \
	    rm -f $(INSTALL_DIR)/$$file; \
	    curdir=`pwd`; \
	    ln -s $$curdir/$$file $(INSTALL_DIR)/$$file; \
	done

