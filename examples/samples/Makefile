#!smake
#
# Copyright (c) 1993-1997, Silicon Graphics, Inc.
# ALL RIGHTS RESERVED
# Permission to use, copy, modify, and distribute this software for
# any purpose and without fee is hereby granted, provided that the above
# copyright notice appear in all copies and that both the copyright notice
# and this permission notice appear in supporting documentation, and that
# the name of Silicon Graphics, Inc. not be used in advertising
# or publicity pertaining to distribution of the software without specific,
# written prior permission.
#
# THE MATERIAL EMBODIED ON THIS SOFTWARE IS PROVIDED TO YOU "AS-IS"
# AND WITHOUT WARRANTY OF ANY KIND, EXPRESS, IMPLIED OR OTHERWISE,
# INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY OR
# FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL SILICON
# GRAPHICS, INC.  BE LIABLE TO YOU OR ANYONE ELSE FOR ANY DIRECT,
# SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY
# KIND, OR ANY DAMAGES WHATSOEVER, INCLUDING WITHOUT LIMITATION,
# LOSS OF PROFIT, LOSS OF USE, SAVINGS OR REVENUE, OR THE CLAIMS OF
# THIRD PARTIES, WHETHER OR NOT SILICON GRAPHICS, INC.  HAS BEEN
# ADVISED OF THE POSSIBILITY OF SUCH LOSS, HOWEVER CAUSED AND ON
# ANY THEORY OF LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE
# POSSESSION, USE OR PERFORMANCE OF THIS SOFTWARE.
#
# US Government Users Restricted Rights
# Use, duplication, or disclosure by the Government is subject to
# restrictions set forth in FAR 52.227.19(c)(2) or subparagraph
# (c)(1)(ii) of the Rights in Technical Data and Computer Software
# clause at DFARS 252.227-7013 and/or in similar or successor
# clauses in the FAR or the DOD or NASA FAR Supplement.
# Unpublished-- rights reserved under the copyright laws of the
# United States.  Contractor/manufacturer is Silicon Graphics,
# Inc., 2011 N.  Shoreline Blvd., Mountain View, CA 94039-7311.
#
# OpenGL(R) is a registered trademark of Silicon Graphics, Inc.
#



# The subdirs here aren't used because they are identical to 
# the ones in more_samples.  We make them here, but we don't
# install them.

SUBDIRS = meshview chess shadow

TARGETS = csg fog tri abgr copy eval sphere line logo nurb point prim \
	quad star texproxy wave copytex accum varray packedpix stretch \
	shell depth bitmap2 bitmap1 teapot select blendeq stencil texobj  \
	texsubimage surfgrid texture sgiflag signal olympic rainbow  \
	twotextures blendxor \

TOP = ../../toolkits/glut-3.5

GLUT = $(TOP)/lib/glut/libglut.a
LLDLIBS = $(GLUT) -lGLU -lGL -lXmu -lXext -lX11 -lm

LDIRT = $(TARGETS) *.o

include $(ROOT)/usr/include/make/commondefs

default: all

COMMONPREF=xxx
include $(COMMONRULES)

$(COMMONTARGS): $(COMMONPREF)$$@
	$(SUBDIRS_MAKERULE)

all: $(TARGETS) $(SUBDIRS)

.c.o:
	cc -c -I$(TOP)/include $<

$(TARGETS): $$@.o rgb.o trackball.o font.o
	cc $@.o rgb.o trackball.o font.o $(LLDLIBS) -o $@

$(SUBDIRS): $(_FORCE)
	cd $@; $(MAKE)

$(_FORCE):

INSTALL_DIR = /usr/demos/SIGGRAPH97/examples/samples

DATAFILES = 1.rgb 2.rgb 3.rgb
SHELLFILES = copy.sh

install: $(TARGETS) $(SUBDIRS)
	if test ! -d $(INSTALL_DIR); then \
	    mkdir -p $(INSTALL_DIR); \
	fi; \
	for file in $(TARGETS) $(DATAFILES) $(SHELLFILES); do \
	    rm -f $(INSTALL_DIR)/$$file; \
	    curdir=`pwd`; \
	    ln -s $$curdir/$$file $(INSTALL_DIR)/$$file; \
	done

