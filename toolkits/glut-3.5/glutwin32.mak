
# Be sure to modify the definitions in this file to agree with your
# systems installation.
#  NOTE: be sure that the install directories use '\' not '/' for paths.


# MSVC install directories
LIBINSTALL     = \msdev\lib
INCLUDEINSTALL = \msdev\include\GL

# Win95 dll directory
DLLINSTALL     = \windows\system

# WinNT dll directory
#DLLINSTALL     = \winnt\system32

# SGI OpenGL libraries (formerly Cosmo)
GLU     = glu.lib
OPENGL  = opengl.lib
GLUT    = $(TOP)/lib/glut/glut.lib
GLUTLIB = glut.lib
GLUTDLL = glut.dll

# Microsoft OpenGL libraries
#GLU    = glu32.lib
#OPENGL = opengl32.lib
#GLUT   = $(TOP)/lib/glut/glut32.lib
#GLUTLIB = glut32.lib
#GLUTDLL = glut32.dll

# The Micro UI lib
MUI     = $(TOP)/lib/mui/mui.lib

# common definitions used by all makefiles
CFLAGS	= $(cflags) $(cdebug) -DWIN32 -I$(TOP)/include
LIBS	= $(lflags) $(ldebug) $(EXTRALIBS) $(GLUT) $(GLU) $(OPENGL) $(guilibs)
EXES	= $(SRCS:.c=.exe)

# default rule
default	: $(EXES)


# cleanup rules
clean	:
	@del *.obj
	@del *.pdb
	@del *.ilk
	@del *.ncb
	@del *~
	@del *.exp

clobber	: 
	@del *.exe
	@del *.dll
	@del *.lib

# inference rules
$(EXES)	: $*.obj
        $(link) -out:$@ $** $(LIBS)
.c.obj	: 
	$(CC) $(CFLAGS) $<
