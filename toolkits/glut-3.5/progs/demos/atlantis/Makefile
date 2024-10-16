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

TARGETS = atlantis

LLDLIBS = $(GLUT) -lGLU -lGL -lXmu -lXi -lXext -lX11 -lm

SRCS =	swim.c atlantis.c whale.c dolphin.c shark.c
HDRS =	atlantis.h
OBJS =	$(SRCS:.c=.o)

LCOPTS = -I$(TOP)/include -fullwarn
LWOFF = ,813,852,827,826
LDIRT = *~ mjkimage.c *.bak *.pure $(TARGETS)

default : $(TARGETS)

atlantis : atlantis.o whale.o dolphin.o shark.o swim.o
	$(RM) $@
	$(CC) -o $@ atlantis.o whale.o dolphin.o shark.o swim.o $(LDFLAGS)

./atlantis.h : atlantis.h

include $(COMMONRULES)

INSTALL_DIR = /usr/demos/SIGGRAPH97/toolkits/glut-3.5/progs/demos

install:: $(TARGETS)
	if test ! -d $(INSTALL_DIR); then \
	    mkdir -p $(INSTALL_DIR); \
	fi; \
	for file in $(TARGETS); do \
	    rm -f $(INSTALL_DIR)/$$file; \
	    curdir=`pwd`; \
	    ln -s $$curdir/$$file $(INSTALL_DIR)/$$file; \
	done
