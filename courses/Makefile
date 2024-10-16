#!smake

TARGETS = cube eye

default: $(TARGETS)

# CFLAGS = -g

# LDFLAGS = -ignore_unresolved -lGL -lGLU -lX11 -lXext
LDFLAGS = -lglut -lGLU -lGL -lXmu -lXi -lXext -lX11 -lm -limage

INSTALL_DIR = /usr/demos/SIGGRAPH97/courses
DATAFILES = *.rgb

clean::
	rm -f $(TARGETS) *.o

install:: $(TARGETS)
	if test ! -d $(INSTALL_DIR); then \
	    mkdir -p $(INSTALL_DIR); \
	fi; \
	for file in $(TARGETS) $(DATAFILES); do \
	    rm -f $(INSTALL_DIR)/$$file; \
	    curdir=`pwd`; \
	    ln -s $$curdir/$$file $(INSTALL_DIR)/$$file; \
	done

