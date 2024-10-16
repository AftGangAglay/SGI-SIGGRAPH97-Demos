#!smake
#
# Copyright (c) Mark J. Kilgard, 1995.
#

IDB_TAG = GLUT_DEV_MAN
IDB_PATH = /usr/share/catman/p_man/cat3/GLUT

MANPAGES = glutAddMenuEntry.man glutAddSubMenu.man glutAttachMenu.man \
  glutBitmapCharacter.man glutBitmapWidth.man glutButtonBoxFunc.man \
  glutChangeToMenuEntry.man glutChangeToSubMenu.man glutCopyColormap.man \
  glutCreateMenu.man glutCreateSubWindow.man glutCreateWindow.man \
  glutDestroyMenu.man glutDestroyWindow.man glutDeviceGet.man \
  glutDialsFunc.man glutDisplayFunc.man glutEntryFunc.man \
  glutEstablishOverlay.man glutExtensionSupported.man glutFullScreen.man \
  glutGet.man glutGetColor.man glutGetModifiers.man glutIdleFunc.man \
  glutInit.man glutInitDisplayMode.man glutInitWindowPosition.man \
  glutKeyboardFunc.man glutLayerGet.man glutMainLoop.man \
  glutMenuStatusFunc.man glutMotionFunc.man glutMouseFunc.man \
  glutOverlayDisplayFunc.man glutPopWindow.man glutPositionWindow.man \
  glutPostOverlayRedisplay.man glutPostRedisplay.man \
  glutRemoveMenuItem.man glutRemoveOverlay.man glutReshapeFunc.man \
  glutReshapeWindow.man glutSetColor.man glutSetCursor.man \
  glutSetMenu.man glutSetWindow.man glutSetWindowTitle.man \
  glutShowOverlay.man glutShowWindow.man glutSolidCone.man \
  glutSolidCube.man glutSolidDodecahedron.man glutSolidIcosahedron.man \
  glutSolidOctahedron.man glutSolidSphere.man glutSolidTeapot.man \
  glutSolidTetrahedron.man glutSolidTorus.man \
  glutSpaceballButtonFunc.man glutSpaceballMotionFunc.man \
  glutSpaceballRotateFunc.man glutSpecialFunc.man \
  glutStrokeCharacter.man glutStrokeWidth.man glutSwapBuffers.man \
  glutTabletButtonFunc.man glutTabletMotionFunc.man glutTimerFunc.man \
  glutUseLayer.man glutVisibilityFunc.man glut.man \
  glutInitDisplayString.man glutReportErrors.man glutWarpPointer.man

FORMATTED = $(MANPAGES:.man=.z)

POSTSCRIPT = $(MANPAGES:.man=.ps)

.SUFFIXES: .man .z .ps

default all: $(FORMATTED)

postscript: $(POSTSCRIPT)

install: $(FORMATTED)
	$(INSTALL) -idb "$(IDB_TAG)" -m 444 -F $(IDB_PATH) *.z
	$(INSTALL) -idb "$(IDB_TAG)" -lns glutSolidCone.z -F $(IDB_PATH) glutWireCone.z
	$(INSTALL) -idb "$(IDB_TAG)" -lns glutSolidCube.z -F $(IDB_PATH) glutWireCube.z
	$(INSTALL) -idb "$(IDB_TAG)" -lns glutSolidDodecahedron.z -F $(IDB_PATH) glutWireDodecahedron.z
	$(INSTALL) -idb "$(IDB_TAG)" -lns glutSolidIcosahedron.z -F $(IDB_PATH) glutWireIcosahedron.z
	$(INSTALL) -idb "$(IDB_TAG)" -lns glutSolidOctahedron.z -F $(IDB_PATH) glutWireOctahedron.z
	$(INSTALL) -idb "$(IDB_TAG)" -lns glutSolidSphere.z -F $(IDB_PATH) glutWireSphere.z
	$(INSTALL) -idb "$(IDB_TAG)" -lns glutSolidTeapot.z -F $(IDB_PATH) glutWireTeapot.z
	$(INSTALL) -idb "$(IDB_TAG)" -lns glutSolidTetrahedron.z -F $(IDB_PATH) glutWireTetrahedron.z
	$(INSTALL) -idb "$(IDB_TAG)" -lns glutSolidTorus.z -F $(IDB_PATH) glutWireTorus.z
	$(INSTALL) -idb "$(IDB_TAG)" -lns glutSetWindow.z -F $(IDB_PATH) glutGetWindow.z
	$(INSTALL) -idb "$(IDB_TAG)" -lns glutPopWindow.z -F $(IDB_PATH) glutPushWindow.z
	$(INSTALL) -idb "$(IDB_TAG)" -lns glutShowWindow.z -F $(IDB_PATH) glutHideWindow.z glutIconifyWindow.z
	$(INSTALL) -idb "$(IDB_TAG)" -lns glutSetWindowTitle.z -F $(IDB_PATH) glutSetIconTitle.z
	$(INSTALL) -idb "$(IDB_TAG)" -lns glutShowOverlay.z -F $(IDB_PATH) glutHideOverlay.z
	$(INSTALL) -idb "$(IDB_TAG)" -lns glutSetMenu.z -F $(IDB_PATH) glutGetMenu.z
	$(INSTALL) -idb "$(IDB_TAG)" -lns glutAttachMenu.z -F $(IDB_PATH) glutDetachMenu.z
	$(INSTALL) -idb "$(IDB_TAG)" -lns glutMotionFunc.z -F $(IDB_PATH) glutPassiveMotionFunc.z
	$(INSTALL) -idb "$(IDB_TAG)" -lns glut.z -F $(IDB_PATH) intro.z
	$(INSTALL) -idb "$(IDB_TAG)" -lns glutBitmapWidth.z -F $(IDB_PATH) glutBitmapLength.z
	$(INSTALL) -idb "$(IDB_TAG)" -lns glutStrokeWidth.z -F $(IDB_PATH) glutStrokeLength.z
	
.man.z:
	nroff -man < $< > $*.nr
	pack $*.nr
	mv $*.nr.z $@

.man.ps:
	psroff -t -man < $< > $*.ps

clobber clean:
	-rm -rf *.z *.nr *.ps *.o *~
