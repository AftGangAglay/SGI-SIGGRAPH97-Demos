/*
 * Copyright (c) 1993-1997, Silicon Graphics, Inc.
 * ALL RIGHTS RESERVED 
 * Permission to use, copy, modify, and distribute this software for 
 * any purpose and without fee is hereby granted, provided that the above
 * copyright notice appear in all copies and that both the copyright notice
 * and this permission notice appear in supporting documentation, and that 
 * the name of Silicon Graphics, Inc. not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission. 
 *
 * THE MATERIAL EMBODIED ON THIS SOFTWARE IS PROVIDED TO YOU "AS-IS"
 * AND WITHOUT WARRANTY OF ANY KIND, EXPRESS, IMPLIED OR OTHERWISE,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY OR
 * FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL SILICON
 * GRAPHICS, INC.  BE LIABLE TO YOU OR ANYONE ELSE FOR ANY DIRECT,
 * SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY
 * KIND, OR ANY DAMAGES WHATSOEVER, INCLUDING WITHOUT LIMITATION,
 * LOSS OF PROFIT, LOSS OF USE, SAVINGS OR REVENUE, OR THE CLAIMS OF
 * THIRD PARTIES, WHETHER OR NOT SILICON GRAPHICS, INC.  HAS BEEN
 * ADVISED OF THE POSSIBILITY OF SUCH LOSS, HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE
 * POSSESSION, USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 * US Government Users Restricted Rights 
 * Use, duplication, or disclosure by the Government is subject to
 * restrictions set forth in FAR 52.227.19(c)(2) or subparagraph
 * (c)(1)(ii) of the Rights in Technical Data and Computer Software
 * clause at DFARS 252.227-7013 and/or in similar or successor
 * clauses in the FAR or the DOD or NASA FAR Supplement.
 * Unpublished-- rights reserved under the copyright laws of the
 * United States.  Contractor/manufacturer is Silicon Graphics,
 * Inc., 2011 N.  Shoreline Blvd., Mountain View, CA 94039-7311.
 *
 * OpenGL(R) is a registered trademark of Silicon Graphics, Inc.
 */

#include <GL/glut.h>
#include <mui/gizmo.h>
#include <stdio.h>

int mui_xorg, mui_yorg, mui_xsize, mui_ysize;
static GLint viewport[4];
int mui_singlebuffered = 0;
int menuinuse = 0;

static int mbleft = 0;

void setmousebuttons(int b, int s)
{
    switch (b) {
	case GLUT_LEFT_BUTTON:
	    mbleft = (s == GLUT_DOWN);
	    break;
    }
}

void mui_drawgeom(void)
{
    glViewport(0, 0, mui_xsize, mui_ysize);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, mui_xsize, 0, mui_ysize);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    muiDrawUIList(1);
    if (mui_singlebuffered == 0)
	glutSwapBuffers();
}

void mui_keyboard(unsigned char c, int x, int y)
{
    muiHandleEvent(MUI_KEYSTROKE, c, x, mui_ysize-y);
    glutPostRedisplay();
}

void mui_mouse(int b, int s, int x, int y)
{
    setmousebuttons(b, s);
    if (b == GLUT_MIDDLE_BUTTON && s == GLUT_DOWN) {
	muiHandleEvent(MUI_DEVICE_DOUBLE_CLICK, 0, x, mui_ysize-y);
	glutPostRedisplay();
    }
    if (b != GLUT_LEFT_BUTTON) { return; }
    muiHandleEvent((s==GLUT_DOWN)?MUI_DEVICE_PRESS:MUI_DEVICE_RELEASE, 0, x, mui_ysize-y);
    glutPostRedisplay();
}

static void mui_Reshape(int width, int height)
{
    mui_xorg = glutGet(GLUT_WINDOW_X);
    mui_yorg = glutGet(GLUT_WINDOW_Y);
    mui_xsize = width;
    mui_ysize = height;
    glViewport(0, 0, mui_xsize, mui_ysize);
    glGetIntegerv(GL_VIEWPORT, viewport);
    glutPostRedisplay();
}

void mui_glutmotion(int x, int y)
{
    if (mbleft == 0) return;
    muiHandleEvent(MUI_DEVICE_DOWN, 0, x, mui_ysize-y);
    glutPostRedisplay();
}

void mui_glutpassivemotion(int x, int y)
{
    muiHandleEvent(MUI_DEVICE_UP, 0, x, mui_ysize-y);
    glutPostRedisplay();
}

void mui_menufunc(int state)
{
    menuinuse = state;
}

void muiInit(void)
{
    glutKeyboardFunc(mui_keyboard);
    glutMouseFunc(mui_mouse);
    glutReshapeFunc(mui_Reshape);
    glutMotionFunc(mui_glutmotion);
    glutPassiveMotionFunc(mui_glutpassivemotion);
    glutDisplayFunc(mui_drawgeom);
    glutMenuStateFunc(mui_menufunc);
}
