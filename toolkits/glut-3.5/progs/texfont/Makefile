#!smake
#
# Copyright (c) Mark J. Kilgard, 1995, 1997.
#

TOP = ../..

LN = ln -s
MV = mv
RM = -rm -rf

TARGETS = gentexfont showtxf simpletxf txfdemo

LLDLIBS = $(GLUT) -lGLU -lGL -lXmu -lXext -lX11 -lm

SRCS = gentexfont.c showtxf.c simpletxf.c texfont.c txfdemo.c
OBJS = $(SRCS:.c=.o)

LCOPTS = -I$(TOP)/include -fullwarn
LWOFF = ,813,852,827,826
LDIRT = *~ *.bak *.pure $(TARGETS)

FONTS = curlfont.txf sorority.txf haeberli.txf djb.txf rockfont.txf default.txf

include $(TOP)/glutdefs
include $(ROOT)/usr/include/make/commondefs

default : $(TARGETS)

gentexfont: gentexfont.o
	$(CC) -o $@ $(CFLAGS) gentexfont.o -lX11

showtxf: showtxf.o texfont.o
	$(CC) -o $@ $(CFLAGS) showtxf.o texfont.o $(LDLIBS)

simpletxf: simpletxf.o texfont.o
	$(CC) -o $@ $(CFLAGS) simpletxf.o texfont.o $(LDLIBS)

txfdemo: txfdemo.o texfont.o
	$(CC) -o $@ $(CFLAGS) txfdemo.o texfont.o $(LDLIBS)

test: all $(_FORCE)
	./gentexfont
	./showtxf
	./showtxf rockfont.txf
	./simpletxf
	./txfdemo

fonts: $(FONTS)

default.txf: gentexfont
	./gentexfont

rockfont.txf: gentexfont Makefile
	./gentexfont \
	  -fn '-sgi-rock-medium-r-normal--100-*-*-*-p-*-iso8859-1' \
	  -file $@ \
	  -glist 'OpenGL Makes Text 3D' \
	  -bitmap \
	  -w 256 \
	  -h 256 \
	  -gap 3 \
	  $(NULL)

curlfont.txf: gentexfont Makefile
	./gentexfont \
	  -fn '-sgi-curl-medium-r-normal--48-*-*-*-p-*--ascii' \
	  -file $@ \
	  -glist 'ABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890 \!@#$$%^&*()-=+/,.<>;:~{}[]' \
	  -bitmap \
	  -w 256 \
	  -h 256 \
	  -gap 3 \
	  $(NULL)

sorority.txf: gentexfont Makefile
	./gentexfont \
	  -fn '-sgi-sorority-medium-r-normal--40-*-*-*-p-*--ascii' \
	  -file $@ \
	  -glist '`"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890 \!@#$$%^&*()-=+/,.<>;:~{}[]' \
	  -bitmap \
	  -w 256 \
	  -h 256 \
	  -gap 3 \
	  $(NULL)

haeberli.txf: gentexfont Makefile
	./gentexfont \
	  -fn '-sgi-haeberli-medium-r-normal--48-*-*-*-p-*-iso8859-1' \
	  -file $@ \
	  -glist '3ABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890 \!@#$$%^&*()-=+/,.<>;:~{}[]' \
	  -bitmap \
	  -w 256 \
	  -h 256 \
	  -gap 3 \
	  $(NULL)

djb.txf: gentexfont Makefile
	./gentexfont \
	  -fn '-sgi-djb-medium-r-normal--35-*-*-*-p-*--ascii' \
	  -file $@ \
	  -glist 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890 \!@#$$%^&*()-=+/,.<>;:~{}[]' \
	  -bitmap \
	  -w 256 \
	  -h 256 \
	  -gap 3 \
	  $(NULL)

include $(COMMONRULES)

INSTALL_DIR = /usr/demos/SIGGRAPH97/toolkits/glut-3.5/progs/texfont
DATAFILES = *.txf

install:: $(TARGETS)
	if test ! -d $(INSTALL_DIR); then \
	    mkdir -p $(INSTALL_DIR); \
	fi; \
	for file in $(TARGETS) $(DATAFILES); do \
	    rm -f $(INSTALL_DIR)/$$file; \
	    curdir=`pwd`; \
	    ln -s $$curdir/$$file $(INSTALL_DIR)/$$file; \
	done

