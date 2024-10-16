/*  
 *  Simple program to visualize a mesh with a color ramp scale.
 *  Press 'h' for a help menu.  Specify a data file as a command 
 *  line argument.
 *
 *  author: Nate Robins
 *  email: ndr@pobox.com
 *  www: http://www.pobox.com/~ndr
 */



#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include <GL/glut.h>
#include "trackball.h"


/* globals */
GLuint     mesh_list = 0;		/* display list for mesh */
GLfloat*** mesh_vertices;		/* mesh vertices */
GLfloat*** mesh_normals;		/* mesh normals */
GLuint     mesh_columns;		/* number of columns in mesh data */
GLuint     mesh_rows;			/* number of rows in mesh data */
GLfloat    mesh_maximum;		/* maximum height value in mesh */
GLfloat    mesh_minimum;		/* minimum height value in mesh */
GLboolean  mesh_fill;			/* filled or wireframe mesh? */
GLuint     mesh_columnstep = 1;		/* column step factor 1 = all points */
GLuint     mesh_rowstep = 1;		/* row step factor 1 = all points */
GLuint     normals_list;		/* display list for normals */
GLuint     legend_list;			/* display list for the legend */
GLboolean  draw_normals = GL_FALSE;	/* draw the normals? */
GLuint     window_width;		/* current width of window */
GLuint     window_height;		/* current height of window */

#define MESH_COLORS 9
GLfloat mesh_colors[MESH_COLORS][3] = { 
  0.5, 0.0, 1.0,			/* violet */
  0.0, 0.0, 1.0,			/* blue */
  0.0, 1.0, 1.0,			/* cyan */
  0.0, 1.0, 0.0,			/* green */
  1.0, 1.0, 0.0,			/* yellow */
  1.0, 0.5, 0.0,			/* orange */
  1.0, 0.0, 0.0,			/* red */
  1.0, 1.0, 1.0,			/* white */
  0.0, 0.0, 0.0,			/* black */
};

/* trackball globals */
GLuint    trackball_lasttime;
GLfloat   trackball_lastposition[3];
GLfloat   trackball_angle = 0;
GLfloat   trackball_axis[3];
GLboolean trackball_animate = GL_FALSE;
GLfloat   trackball_transform[4][4];

void
normalizedcross(GLfloat* u, GLfloat* v, GLfloat* n)
{
  GLfloat l;

  /* compute the cross product (u x v for right-handed [ccw]) */
  n[0] = u[1] * v[2] - u[2] * v[1];
  n[1] = u[2] * v[0] - u[0] * v[2];
  n[2] = u[0] * v[1] - u[1] * v[0];

  /* normalize */
  l = (GLfloat)sqrt(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
  n[0] /= l;
  n[1] /= l;
  n[2] /= l;
}

/* text: general purpose text routine.  draws a string according to
   format in a stroke font at x, y after scaling it by scale in all
   three dimensions. */
void text(float x, float y, float scale, char *format, ...)
{
  va_list args;
  char buffer[200], *p;

  va_start(args, format);
  vsprintf(buffer, format, args);
  va_end(args);

  glPushMatrix();
    glPushAttrib(GL_ENABLE_BIT);

      glDisable(GL_LIGHTING);
      glDisable(GL_TEXTURE_2D);
      glDisable(GL_DEPTH_TEST);
      glTranslatef(x, y, 0.0);
      glScalef(scale, scale, scale);
      for(p = buffer; *p; p++)
	glutStrokeCharacter(GLUT_STROKE_ROMAN, *p);

    glPopAttrib();
  glPopMatrix();
}

void
meshread(char* filename)
{
  unsigned int i, j;
  FILE* file;
  GLfloat**  mesh_data;
  GLfloat*** mesh_facetnormals;
  GLfloat u[3], v[3], n[3], l;

  /* open the file */
  file = fopen(filename, "r");
  if (!file) {
    fprintf(stderr, "meshview: cannot open data file \"%s\"\n", filename);
    exit(1);
  }

  /* get the extents of the mesh */
  fscanf(file, "%d %d", &mesh_columns, &mesh_rows);

  /* allocate some space for the mesh */
  mesh_data         = (GLfloat**)malloc(sizeof(GLfloat)*mesh_columns);
  mesh_vertices     = (GLfloat***)malloc(sizeof(GLfloat)*mesh_columns);
  mesh_normals      = (GLfloat***)malloc(sizeof(GLfloat)*mesh_columns);
  mesh_facetnormals = (GLfloat***)malloc(sizeof(GLfloat)*mesh_columns);
  for (i = 0; i < mesh_columns; i++) {
    mesh_data[i]         = (GLfloat*)malloc(sizeof(GLfloat)*mesh_rows);
    mesh_vertices[i]     = (GLfloat**)malloc(sizeof(GLfloat)*mesh_rows);
    mesh_normals[i]      = (GLfloat**)malloc(sizeof(GLfloat)*mesh_rows);
    mesh_facetnormals[i] = (GLfloat**)malloc(sizeof(GLfloat)*mesh_rows);
    for (j = 0; j < mesh_rows; j++) {
      mesh_vertices[i][j]     = (GLfloat*)malloc(sizeof(GLfloat)*3);
      mesh_normals[i][j]      = (GLfloat*)malloc(sizeof(GLfloat)*3);
      mesh_facetnormals[i][j] = (GLfloat*)malloc(sizeof(GLfloat)*3);
    }
  }

  /* get the data */
  mesh_maximum = mesh_minimum = 0;
  for (j = 0; j < mesh_rows; j++) {
    for (i = 0; i < mesh_columns; i++) {
      if (fscanf(file, "%f", &mesh_data[i][j]) != 1) {
	fprintf(stderr, "meshview: error in data file \"%s\"\n", filename);
	exit(1);
      }

      /* find the maximum data value */
      if (mesh_maximum < mesh_data[i][j]) 
	mesh_maximum = mesh_data[i][j];

      /* find the minimum data value */
      if (mesh_minimum > mesh_data[i][j]) 
	mesh_minimum = mesh_data[i][j];
    }
  }

  /* set mesh_maximum to reflect the maximum extent of the height of
     the mesh. */
  mesh_maximum -= mesh_minimum;

  /* bump the mesh_data value up to the zero point by adding the
     minimum to it. (normalize to + side of numberline). */
  for (i = 0; i < mesh_columns; i++) {
    for (j = 0; j < mesh_rows; j++) {
      mesh_data[i][j] += -mesh_minimum;
    }
  }

  /* fill out the vertex array and calculate facet normals for the
     mesh. */
  for (i = 0; i < mesh_columns - 1; i++) {
    for (j = 0; j < mesh_rows - 1; j++) {
      /* assign the data to vertices.  some of the vertices will be
         overwritten in subsequent iterations of the loop, but this is
         okay, since they will be identical. */
      mesh_vertices[i][j][0] = ((GLfloat)i / (GLfloat)mesh_columns) - 0.5; 
      mesh_vertices[i][j][1] = (mesh_data[i][j] / mesh_maximum) - 0.5;
      mesh_vertices[i][j][2] = ((GLfloat)j / (GLfloat)mesh_rows) - 0.5;
      
      mesh_vertices[i][j+1][0] = mesh_vertices[i][j][0];
      mesh_vertices[i][j+1][1] = (mesh_data[i][j+1] / mesh_maximum) - 0.5;
      mesh_vertices[i][j+1][2] = ((GLfloat)(j+1) / (GLfloat)mesh_rows) - 0.5;
      
      mesh_vertices[i+1][j][0] = ((GLfloat)(i+1) / (GLfloat)mesh_columns)- 0.5;
      mesh_vertices[i+1][j][1] = (mesh_data[i+1][j] / mesh_maximum) - 0.5;
      mesh_vertices[i+1][j][2] = mesh_vertices[i][j][2];
       
      /* get two vectors to cross */
      u[0] = mesh_vertices[i][j+1][0] - mesh_vertices[i][j][0];
      u[1] = mesh_vertices[i][j+1][1] - mesh_vertices[i][j][1];
      u[2] = mesh_vertices[i][j+1][2] - mesh_vertices[i][j][2];

      v[0] = mesh_vertices[i+1][j][0] - mesh_vertices[i][j][0];
      v[1] = mesh_vertices[i+1][j][1] - mesh_vertices[i][j][1];
      v[2] = mesh_vertices[i+1][j][2] - mesh_vertices[i][j][2];

      /* get the normalized cross product */
      normalizedcross(u, v, n);
      
      /* put the facet normal in the i, j position for later averaging
         with other normals. */
      mesh_facetnormals[i][j][0] = n[0];
      mesh_facetnormals[i][j][1] = n[1];
      mesh_facetnormals[i][j][2] = n[2];
    }
  }

  /* fill in the last vertex & it's facet normal */
  mesh_vertices[i][j][0] = ((GLfloat)i / (GLfloat)mesh_columns) - 0.5; 
  mesh_vertices[i][j][1] = (mesh_data[i][j] / mesh_maximum) - 0.5;
  mesh_vertices[i][j][2] = ((GLfloat)j / (GLfloat)mesh_rows) - 0.5;

  mesh_facetnormals[i][j][0] = n[0];
  mesh_facetnormals[i][j][1] = n[1];
  mesh_facetnormals[i][j][2] = n[2];

  /* calculate normals for the mesh */
  for (i = 1; i < mesh_columns - 1; i++) {
    for (j = 1; j < mesh_rows - 1; j++) {
      /* average all the neighboring normals. */
      n[0] = mesh_facetnormals[i-1][j-1][0];
      n[1] = mesh_facetnormals[i-1][j-1][1];
      n[2] = mesh_facetnormals[i-1][j-1][2];

      n[0] += mesh_facetnormals[i][j-1][0];
      n[1] += mesh_facetnormals[i][j-1][1];
      n[2] += mesh_facetnormals[i][j-1][2];

      n[0] += mesh_facetnormals[i-1][j][0];
      n[1] += mesh_facetnormals[i-1][j][1];
      n[2] += mesh_facetnormals[i-1][j][2];

      n[0] += mesh_facetnormals[i][j][0];
      n[1] += mesh_facetnormals[i][j][1];
      n[2] += mesh_facetnormals[i][j][2];
      
      l = (GLfloat)sqrt(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
      mesh_normals[i][j][0] = n[0] /= l;
      mesh_normals[i][j][1] = n[1] /= l;
      mesh_normals[i][j][2] = n[2] /= l;
    }
  }

  /* fill in the normals on the top/bottom edge of the mesh (simply
     copy the one below/above it). */
  for (i = 0; i < mesh_columns; i++) {
    mesh_normals[i][0][0] = mesh_normals[i][1][0];
    mesh_normals[i][0][1] = mesh_normals[i][1][1];
    mesh_normals[i][0][2] = mesh_normals[i][1][2];

    mesh_normals[i][mesh_rows-1][0] = mesh_normals[i][mesh_rows-2][0];
    mesh_normals[i][mesh_rows-1][1] = mesh_normals[i][mesh_rows-2][1];
    mesh_normals[i][mesh_rows-1][2] = mesh_normals[i][mesh_rows-2][2];
  }

  /* fill in the normals on the left/right edge of the mesh (simply
     copy the one right/left of it). */
  for (j = 0; j < mesh_rows; j++) {
    mesh_normals[0][j][0] = mesh_normals[1][j][0];
    mesh_normals[0][j][1] = mesh_normals[1][j][1];
    mesh_normals[0][j][2] = mesh_normals[1][j][2];

    mesh_normals[mesh_columns-1][j][0] = mesh_normals[mesh_columns-2][j][0];
    mesh_normals[mesh_columns-1][j][1] = mesh_normals[mesh_columns-2][j][1];
    mesh_normals[mesh_columns-1][j][2] = mesh_normals[mesh_columns-2][j][2];
  }

  printf("%d x %d x %.0f mesh\n", mesh_columns, mesh_rows, mesh_maximum);

  /* free the mesh data & facet normals */
  for (i = 0; i < mesh_columns; i++) {
    for (j = 0; j < mesh_rows; j++)
      free(mesh_facetnormals[i][j]);
    free(mesh_facetnormals[i]);
    free(mesh_data[i]);
  }
  free(mesh_data);
  free(mesh_facetnormals);

  /* close the file */
  fclose(file);
}

void
meshlist(void)
{
  GLuint i, j;
  char buf[16];

  /* destroy any previous lists */
  if (mesh_list)
    glDeleteLists(mesh_list, 1);
  if (normals_list)
    glDeleteLists(normals_list, 1);
  if (legend_list)
    glDeleteLists(legend_list, 1);

  /* generate a display list for the mesh major = x axis, minor = z axis*/
  mesh_list = glGenLists(1);
  glNewList(mesh_list, GL_COMPILE);

  for (i = 0; i < mesh_columns - mesh_columnstep; i += mesh_columnstep) {

    glPolygonMode(GL_BACK, GL_LINE);
    glBegin(GL_TRIANGLE_STRIP);

    for (j = 0; j < mesh_rows; j += mesh_rowstep) {
      glColor3fv(mesh_colors[(int)((MESH_COLORS-2) * 
				   (mesh_vertices[i+mesh_columnstep][j][1] 
				    + 0.5))]);
      glNormal3fv(mesh_normals[i+mesh_columnstep][j]);
      glVertex3fv(mesh_vertices[i+mesh_columnstep][j]);
      glColor3fv(mesh_colors[(int)((MESH_COLORS-2) * 
				   (mesh_vertices[i][j][1] + 0.5))]);
      glNormal3fv(mesh_normals[i][j]);
      glVertex3fv(mesh_vertices[i][j]);
    }

    glEnd();
  }
  glEndList();

  normals_list = glGenLists(1);
  glNewList(normals_list, GL_COMPILE);
  /* draw normals */
  for (i = 0; i < mesh_columns; i += mesh_columnstep) {
    for (j = 0; j < mesh_rows; j += mesh_rowstep) {
      glPushMatrix();
      glTranslatef(mesh_vertices[i][j][0], 
		   mesh_vertices[i][j][1], 
		   mesh_vertices[i][j][2]);
      glScalef(0.1, 0.1, 0.1);
      glBegin(GL_LINES);
      glColor3f(1.0, 0.0, 0.0);
      glVertex3f(0.0, 0.0, 0.0);
      glColor3f(0.0, 1.0, 0.0);
      glVertex3fv(mesh_normals[i][j]);
      glEnd();
      glScalef(1.0, 1.0, 1.0);
      glPopMatrix();
    }
  }
  glEndList();

  legend_list = glGenLists(1);
  glNewList(legend_list, GL_COMPILE);
  glPushMatrix();
  glLoadIdentity();
  glTranslatef(0.0, 0.0, -2.0);
  for (i = MESH_COLORS; i >= 1; i--) {
    glColor3fv(mesh_colors[i-1]);
    glPolygonMode(GL_BACK, GL_FILL);
    glNormal3f(0.577, 0.577, 0.577);
    glRectf(-0.98, (GLfloat)(i+2) / (GLfloat)(MESH_COLORS) - 0.98,
	    -0.78, (GLfloat)(i+1) / (GLfloat)(MESH_COLORS) - 0.98);
  }
  glColor3f(1.0, 1.0, 1.0);
  sprintf(buf, "%.1f", mesh_maximum + mesh_minimum);
  text(-0.98, (GLfloat)(MESH_COLORS) / (GLfloat)(MESH_COLORS-1) - 0.95, 0.0005,
       buf);
  sprintf(buf, "%.1f", mesh_minimum);
  text(-0.98, -0.95, 0.0005, buf);
  glPopMatrix();
  glEndList();
}

void
init(void)
{
  /* create a new list */
  meshlist();

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_COLOR_MATERIAL);

  glEnable(GL_DEPTH_TEST);

  /* put the identity in the trackball transform. */
  glPushMatrix();
  glLoadIdentity();
  glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat *)trackball_transform);
  glPopMatrix();

  tbInit(GLUT_MIDDLE_BUTTON);
}

void
reshape(int width, int height)
{
  GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0 }; 

  tbReshape(width, height);

  window_width = width;
  window_height = height;

  glViewport(0, 0, width, height);
  
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glFrustum(-0.5, 0.5, -0.5, 0.5, 1.0, 8.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslatef(0.0, 0.0, -2.0);
  glLightfv(GL_LIGHT0, GL_POSITION, light_position);
}

void
display(void)
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glPushMatrix();

  tbMatrix();

  glColor3f(1.0, 1.0, 1.0);
  glCallList(mesh_list);

  if (draw_normals)
    glCallList(normals_list);

  glCallList(legend_list);

  glPopMatrix();

  glutSwapBuffers();
}

void
special(int key, int x, int y)
{
  switch (key) { 
    case GLUT_KEY_UP:
      mesh_rowstep += 1;
      if (mesh_rowstep >= mesh_rows)
	mesh_rowstep = mesh_rows - 1;
      meshlist();
      printf("mesh_rowstep = %d\n", mesh_rowstep);
      break;

    case GLUT_KEY_DOWN:
      mesh_rowstep -= 1;
      if (mesh_rowstep <= 0)
	mesh_rowstep = 1;
      meshlist();
      printf("mesh_rowstep = %d\n", mesh_rowstep);
      break;

    case GLUT_KEY_RIGHT:
      mesh_columnstep += 1;
      if (mesh_columnstep >= mesh_columns)
	mesh_columnstep = mesh_columns - 1;
      meshlist();
      printf("mesh_columnstep = %d\n", mesh_columnstep);
      break;

    case GLUT_KEY_LEFT:
      mesh_columnstep -= 1;
      if (mesh_columnstep <= 0)
	mesh_columnstep = 1;
      meshlist();
      printf("mesh_columnstep = %d\n", mesh_columnstep);
      break;
 }

  glutPostRedisplay();
}

void
keyboard(char key, int x, int y)
{
  switch (key) {
  case 'h':
    printf("meshview help\n\n");
    printf("f            -  Filled mesh\n");
    printf("w            -  Wireframe mesh\n");
    printf("l            -  Toggle lighting\n");
    printf("n            -  Toggle normals\n");
    printf("c            -  Toggle colormap\n");
    printf("left arrow   -  Decrease column step\n");
    printf("right arrow  -  Increase column step\n");
    printf("down arrow   -  Decrease row step\n");
    printf("up arrow     -  Increase row step\n");
    printf("escape or q  -  Quit\n\n");
    break;

  case 'f':
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    break;

  case 'w':
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    break;

  case 'l':
    if (glIsEnabled(GL_LIGHTING))
      glDisable(GL_LIGHTING);
    else
      glEnable(GL_LIGHTING);
    break;

  case 'n':
    draw_normals = !draw_normals;
    break;

  case 'c':
    if (glIsEnabled(GL_COLOR_MATERIAL))
      glDisable(GL_COLOR_MATERIAL);
    else
      glEnable(GL_COLOR_MATERIAL);
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

  glutPostRedisplay();
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
  if (argc < 2) {
    meshread("sine.dat");
  } else {
    meshread(argv[1]);
  }

  glutInit(&argc, argv);

  glutInitWindowSize(256, 256);
  glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
  glutCreateWindow("Meshview");
  
  glutReshapeFunc(reshape);
  glutDisplayFunc(display);
  glutKeyboardFunc(keyboard);
  glutSpecialFunc(special);
  glutMouseFunc(mouse);
  glutMotionFunc(motion);

  init();
  
  glutMainLoop();
  return 0;
}
