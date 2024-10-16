/*  
 *  Demonstration of palette animation in color index mode.
 *  Press 'h' for a help menu, middle button spins.
 *
 *  author: Nate Robins
 *  email: ndr@pobox.com
 *  www: http://www.pobox.com/~ndr
 */


/* includes */
#include <math.h>
#include <stdio.h>
#include <GL/glut.h>
#include "trackball.h"


/* globals */
GLuint cube_list = 0;			/* display list for cube */

GLboolean auto_rotate = GL_TRUE;	/* automatically rotate */

#define NUMCOLORS 12
GLfloat colors[3 * NUMCOLORS * 2] = { 
  1.0, 0.0, 0.0,
  1.0, 0.5, 0.0,
  1.0, 1.0, 0.0,
  0.5, 1.0, 0.0,
  0.0, 1.0, 0.0,
  0.0, 1.0, 0.5,
  0.0, 1.0, 1.0,
  0.0, 0.5, 1.0,
  0.0, 0.0, 1.0,
  0.5, 0.0, 1.0,
  1.0, 0.0, 1.0,
  1.0, 0.0, 0.5,
  1.0, 0.0, 0.0,			/* duplicate the list */
  1.0, 0.5, 0.0,
  1.0, 1.0, 0.0,
  0.5, 1.0, 0.0,
  0.0, 1.0, 0.0,
  0.0, 1.0, 0.5,
  0.0, 1.0, 1.0,
  0.0, 0.5, 1.0,
  0.0, 0.0, 1.0,
  0.5, 0.0, 1.0,
  1.0, 0.0, 1.0,
  1.0, 0.0, 0.5,
};

/* functions */

void
lists()
{
  /* cube */
  if (cube_list)
    glDeleteLists(cube_list, 1);

  /* generate a display list */
  cube_list = glGenLists(1);
  glNewList(cube_list, GL_COMPILE);
  glBegin(GL_QUADS);

  /* front */
  glIndexi(0);
  glVertex3i(-3,  3,  3);
  glVertex3i(-3, -3,  3);
  glVertex3i(-1, -3,  3);
  glVertex3i(-1,  3,  3);

  glIndexi(1);
  glVertex3i(-1,  3,  3);
  glVertex3i(-1, -3,  3);
  glVertex3i( 1, -3,  3);
  glVertex3i( 1,  3,  3);

  glIndexi(2);
  glVertex3i( 1,  3,  3);
  glVertex3i( 1, -3,  3);
  glVertex3i( 3, -3,  3);
  glVertex3i( 3,  3,  3);

  /* back */
  glIndexi(9);
  glVertex3i(-1,  3, -3);
  glVertex3i(-1, -3, -3);
  glVertex3i(-3, -3, -3);
  glVertex3i(-3,  3, -3);

  glIndexi(10);
  glVertex3i( 1,  3, -3);
  glVertex3i( 1, -3, -3);
  glVertex3i(-1, -3, -3);
  glVertex3i(-1,  3, -3);

  glIndexi(11);
  glVertex3i( 3,  3, -3);
  glVertex3i( 3, -3, -3);
  glVertex3i( 1, -3, -3);
  glVertex3i( 1,  3, -3);

  /* right */
  glIndexi(2);
  glVertex3i( 3,  3,  3);
  glVertex3i( 3, -3,  3);
  glVertex3i( 3, -3,  1);
  glVertex3i( 3,  3,  1);

  glIndexi(1);
  glVertex3i( 3,  3,  1);
  glVertex3i( 3, -3,  1);
  glVertex3i( 3, -3, -1);
  glVertex3i( 3,  3, -1);

  glIndexi(0);
  glVertex3i( 3,  3, -1);
  glVertex3i( 3, -3, -1);
  glVertex3i( 3, -3, -3);
  glVertex3i( 3,  3, -3);

  /* left */
  glIndexi(11);
  glVertex3i(-3,  3,  1);
  glVertex3i(-3, -3,  1);
  glVertex3i(-3, -3,  3);
  glVertex3i(-3,  3,  3);

  glIndexi(10);
  glVertex3i(-3,  3, -1);
  glVertex3i(-3, -3, -1);
  glVertex3i(-3, -3,  1);
  glVertex3i(-3,  3,  1);

  glIndexi(9);
  glVertex3i(-3,  3, -3);
  glVertex3i(-3, -3, -3);
  glVertex3i(-3, -3, -1);
  glVertex3i(-3,  3, -1);

  /* top */
  glIndexi(2);
  glVertex3i( 1,  3,  3);
  glVertex3i( 3,  3,  3);
  glVertex3i( 3,  3,  1);
  glVertex3i( 1,  3,  3);		/* degenerate */

  glIndexi(1);
  glVertex3i(-1,  3,  3);
  glVertex3i( 1,  3,  3);
  glVertex3i( 3,  3,  1);
  glVertex3i( 3,  3, -1);

  glIndexi(0);
  glVertex3i(-3,  3,  3);
  glVertex3i(-1,  3,  3);
  glVertex3i( 3,  3, -1);
  glVertex3i( 3,  3, -3);

  glIndexi(11);
  glVertex3i(-3,  3,  1);
  glVertex3i(-3,  3,  3);
  glVertex3i( 3,  3, -3);
  glVertex3i( 1,  3, -3);

  glIndexi(10);
  glVertex3i(-3,  3, -1);
  glVertex3i(-3,  3,  1);
  glVertex3i( 1,  3, -3);
  glVertex3i(-1,  3, -3);

  glIndexi(9);
  glVertex3i(-3,  3, -3);
  glVertex3i(-3,  3, -1);
  glVertex3i(-1,  3, -3);
  glVertex3i(-3,  3, -3);		/* degenerate */

  /* bottom */
  glIndexi(2);
  glVertex3i( 1, -3,  3);		/* degenerate */
  glVertex3i( 3, -3,  1);
  glVertex3i( 3, -3,  3);
  glVertex3i( 1, -3,  3);

  glIndexi(1);
  glVertex3i( 3, -3, -1);
  glVertex3i( 3, -3,  1);
  glVertex3i( 1, -3,  3);
  glVertex3i(-1, -3,  3);

  glIndexi(0);
  glVertex3i( 3, -3, -3);
  glVertex3i( 3, -3, -1);
  glVertex3i(-1, -3,  3);
  glVertex3i(-3, -3,  3);

  glIndexi(11);
  glVertex3i( 1, -3, -3);
  glVertex3i( 3, -3, -3);
  glVertex3i(-3, -3,  3);
  glVertex3i(-3, -3,  1);

  glIndexi(10);
  glVertex3i(-1, -3, -3);
  glVertex3i( 1, -3, -3);
  glVertex3i(-3, -3,  1);
  glVertex3i(-3, -3, -1);

  glIndexi(9);
  glVertex3i(-3, -3, -3);		/* degenerate */
  glVertex3i(-1, -3, -3);
  glVertex3i(-3, -3, -1);
  glVertex3i(-3, -3, -3);

  glEnd();
  glEndList();
}

void
init(void)
{
  int i;

  tbInit(GLUT_MIDDLE_BUTTON);
  tbAnimate(GL_FALSE);

  /* create initial display lists */
  lists();

  /* lighting */
  glDisable(GL_LIGHTING);

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);

  glEnable(GL_CULL_FACE);

  glutSetColor(NUMCOLORS+1, 0.0, 0.0, 0.0);
  glClearIndex(NUMCOLORS+1);

  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);

}

void
reshape(int width, int height)
{
  GLfloat light_position[4] = { 1.0, 1.0, 1.0, 0.0 };

  tbReshape(width, height);

  glViewport(0, 0, width, height);
  
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(60.0, (GLfloat)height / (GLfloat)width, 1.0, 128.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glLightfv(GL_LIGHT0, GL_POSITION, light_position);

  glTranslatef(0.0, 0.0, -12.0);
  glRotatef(15.0, 0.0, 1.0, 0.0);
}

void
display(void)
{
  static int rotate = 0;
  static int i, spin;

  /* setup the colors */
  for (i = rotate; i < NUMCOLORS + rotate; i++) {
      glutSetColor(i-rotate,
		   colors[3 * i + 0], colors[3 * i + 1], colors[3 * i + 2]);
  }

  rotate++;
  rotate %= NUMCOLORS;

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glPushMatrix();

  if (auto_rotate) {
    glRotatef(spin++, 1.0, 1.0, 1.0);
    spin %= 360;
  } else
    tbMatrix();

  glCallList(cube_list);

  glPopMatrix();

  glutSwapBuffers();
}

void
keyboard(char key, int x, int y)
{
  switch (key) {
  case 'h':
    printf("help\n\n");
    printf("f            -  Filled\n");
    printf("w            -  Wireframe\n");
    printf("a            -  Toggle auto-rotation\n");
    printf("backspace    -  Reset\n");
    printf("escape or q  -  Quit\n\n");
    break;

  case 'a':
    auto_rotate = !auto_rotate;
    break;

  case 'f':
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    break;

  case 'w':
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    break;

  case '\b':
    init();
    break;

  case 'q':
  case 27:
    exit(0);
    break;
  }

  glutPostRedisplay();
}

void
mouse(int button, int state, int x, int y)
{
  tbMouse(button, state, x, y);
}

void
motion(int x, int y)
{
  tbMotion(x, y);
  glutPostRedisplay();
}

int
main(int argc, char** argv)
{
  glutInit(&argc, argv);

  glutInitWindowSize(512, 512);
  glutInitDisplayMode(GLUT_INDEX | GLUT_DEPTH | GLUT_DOUBLE);
  glutCreateWindow("Rainbow");
  
  glutReshapeFunc(reshape);
  glutDisplayFunc(display);
  glutIdleFunc(display);
  glutKeyboardFunc(keyboard);
  glutMouseFunc(mouse);
  glutMotionFunc(motion);
  
  init();
  
  glutMainLoop();
  return 0;
}
