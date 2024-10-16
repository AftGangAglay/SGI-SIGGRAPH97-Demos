#!smake
#
# Copyright (c) Mark J. Kilgard, 1996, 1997.
#

TOP = ../..
include $(TOP)/glutdefs
include $(ROOT)/usr/include/make/commondefs

LN = ln -s
MV = mv
RM = -rm -rf

TARGETS = hiddenline haloed silhouette motionblur softshadow accumaa \
  field shadowvol shadowmap projshadow convolve tess textile comp csg \
  dissolve envmap envphong decal textext genmipmap imgproc mipmap_lines \
  projtex textrim tvertex vox warp zcomposite videoresize occlude \
  af_teapots af_depthcue multilight boundary shadowfun hello2rts \
  rasonly

LLDLIBS = $(GLUT) -lGLU -lGL -lXmu -lXi -lXext -lX11 -lm

SRCS =	hiddenline.c haloed.c silhouette.c motionblur.c softshadow.c \
  accumaa.c field.c shadowvol.c shadowmap.c projshadow.c convolve.c \
  tess.c sphere.c texture.c textile.c comp.c csg.c dissolve.c envmap.c \
  envphong.c decal.c textext.c textmap.c genmipmap.c imgproc.c izoom.c \
  mipmap_lines.c projtex.c textrim.c tvertex.c vox.c warp.c zcomposite.c \
  videoresize.c occlude.c addfog.c af_depthcue.c af_teapots.c multilight.c \
  boundary.c shadowfun.c hello2rts.c rts.c rasonly.c
OBJS =	$(SRCS:.c=.o)

DATA_LINKS = 00.rgb 02.rgb 04.rgb a.rgb mandrill.rgb 01.rgb 03.rgb 05.rgb b.rgb tree.rgb vox.bin.gz

LCOPTS = -I$(TOP)/include -fullwarn
LWOFF = ,813,852,827,826,819
LDIRT = *~ *.bak *.pure $(DATA_LINKS) *.pix32 *.Counts *.pixie

default : $(TARGETS)

tess: tess.o sphere.o
	$(CC) -o $@ tess.o sphere.o $(LDFLAGS)

videoresize: videoresize.o sphere.o
	$(CC) -o $@ videoresize.o sphere.o $(LDFLAGS)
	
textile: textile.o texture.o
	$(CC) -o $@ textile.o texture.o $(LDFLAGS)

comp: comp.o texture.o
	$(CC) -o $@ comp.o texture.o $(LDFLAGS)

envmap: envmap.o texture.o
	$(CC) -o $@ envmap.o texture.o $(LDFLAGS)

textext: textext.o textmap.o texture.o
	$(CC) -o $@ textext.o textmap.o texture.o $(LDFLAGS)

genmipmap: genmipmap.o texture.o
	$(CC) -o $@ genmipmap.o texture.o $(LDFLAGS)

imgproc: imgproc.o texture.o
	$(CC) -o $@ imgproc.o texture.o $(LDFLAGS)

projtex: projtex.o texture.o
	$(CC) -o $@ projtex.o texture.o $(LDFLAGS)

textrim: textrim.o texture.o
	$(CC) -o $@ textrim.o texture.o $(LDFLAGS)

warp: warp.o texture.o
	$(CC) -o $@ warp.o texture.o $(LDFLAGS)

af_depthcue: af_depthcue.o addfog.o
	$(CC) -o $@ af_depthcue.o addfog.o $(LDFLAGS)

af_teapots: af_teapots.o addfog.o
	$(CC) -o $@ af_teapots.o addfog.o $(LDFLAGS)

mipmap_lines: mipmap_lines.o izoom.o texture.o
	$(CC) -o $@ mipmap_lines.o izoom.o texture.o $(LDFLAGS)

hello2rts: hello2rts.o rts.o
	$(CC) -g -o $@ hello2rts.o rts.o $(LDFLAGS)

links:
	for i in $(DATA_LINKS); do \
	  /bin/rm -rf $$i ; \
	  ln -s ../../data/$$i . ; \
	done

include $(COMMONRULES)

INSTALL_DIR = /usr/demos/SIGGRAPH97/toolkits/glut-3.5/progs/advanced
DATAFILES = $(DATALINKS)

install:: $(TARGETS)
	if test ! -d $(INSTALL_DIR); then \
	    mkdir -p $(INSTALL_DIR); \
	fi; \
	for file in $(TARGETS) $(DATAFILES); do \
	    rm -f $(INSTALL_DIR)/$$file; \
	    curdir=`pwd`; \
	    ln -s $$curdir/$$file $(INSTALL_DIR)/$$file; \
	done
