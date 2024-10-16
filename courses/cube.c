
/* Rotating cube demo*/

/* Prints out instructions */

/*Both normals and colors are assigned to the vertices */
/*Cube is centered at origin so (unnormalized) normals
are the same as the vertex values */

#include <stdlib.h>
#include <math.h>
#include <GL/glut.h>
#include <GL/glu.h>

#define bool int
#define false 0
#define true 1

/*
** Global settings controlled by keyboard input.
*/
bool texEnabled 		= false;
bool mipmapEnabled 		= false;
bool fastTexture 		= true;
bool fogEnabled 		= false;
bool depthEnabled 		= true;
bool lineAAEnabled 		= false;
bool lightingEnabled 		= false;
bool smoothEnabled 		= false;
bool drawLines 			= true;
bool idleSpin 			= true;
bool perspectiveXform 		= false;

/*
** Global settings.
*/

float 	near = 3.0;    	/* near clipping plane in eye coords */
float 	far = 7.0;    	/* far clipping plane in eye coords */
float 	viewxform_z = -5.0;

int 	winWidth, winHeight;

float 	angle = 0.0, axis[3], trans[3];
bool 	trackballEnabled = true;
bool 	trackballMove = false;
bool 	trackingMouse = false;
bool 	redrawContinue = false;

GLfloat lightXform[4][4] = {
    {1.0, 0.0, 0.0, 0.0},
    {0.0, 1.0, 0.0, 0.0},
    {0.0, 0.0, 1.0, 0.0},
    {0.0, 0.0, 0.0, 1.0}
};
GLfloat objectXform[4][4] = {
    {1.0, 0.0, 0.0, 0.0},
    {0.0, 1.0, 0.0, 0.0},
    {0.0, 0.0, 1.0, 0.0},
    {0.0, 0.0, 0.0, 1.0}
};
GLfloat *trackballXform = (GLfloat*)objectXform;

void display(void);
void spinCube(void);
void setMenuEntries(bool);

/*----------------------------------------------------------------------*/
/* 
** Materials setup.
*/
typedef struct materialStruct {
   GLfloat ambient[4];
   GLfloat diffuse[4];
   GLfloat specular[4];
   GLfloat shininess;
} materialStruct;

materialStruct brassMaterials = {
    {0.33F, 0.22F, 0.03F, 1.0F},
    {0.78F, 0.57F, 0.11F, 1.0F},
    {0.99F, 0.91F, 0.81F, 1.0F},
    27.8F
};
materialStruct redPlasticMaterials = {
    {0.3F, 0.0F, 0.0F, 1.0F},
    {0.6F, 0.0F, 0.0F, 1.0F},
    {0.8F, 0.6F, 0.6F, 1.0F},
    32.0F
};
materialStruct colorCubeMaterials = {
    { 1.0F, 1.0F, 1.0F, 1.0F },
    { 1.0F, 1.0F, 1.0F, 1.0F },
    { 1.0F, 1.0F, 1.0F, 1.0F },
    {100.0F}
};

materialStruct *currentMaterials = &redPlasticMaterials;

void materials( materialStruct *materials)
{
    /* define material proerties for front face of all polygons */
    glMaterialfv(GL_FRONT, GL_AMBIENT, materials->ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materials->diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, materials->specular);
    glMaterialf(GL_FRONT, GL_SHININESS, materials->shininess);
}

/*----------------------------------------------------------------------*/
/*
** Lighting setup.
*/
typedef struct lightingStruct {
    GLfloat ambient[4];
    GLfloat diffuse[4];
    GLfloat specular[4];
} lightingStruct;

lightingStruct whiteLighting = {
    {0.0F, 0.0F, 0.0F, 1.0F},
    {1.0F, 1.0F, 1.0F, 1.0F},
    {1.0F, 1.0F, 1.0F, 1.0F}
};
lightingStruct colorCubeLighting = {
    {0.2F, 0.0F, 0.0F, 1.0F},
    {0.0F, 1.0F, 0.0F, 1.0F},
    {0.0F, 0.0F, 1.0F, 1.0F}
};

lightingStruct *currentLighting = &whiteLighting;

void lighting(lightingStruct *lightSettings)
{
    /* set up light 0 ambient, diffuse, specular, and spotlight */

    glLightfv(GL_LIGHT0, GL_AMBIENT, lightSettings->ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightSettings->diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSettings->specular);

    glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 0.0F);
    glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 180.0F);
    glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 1.0F);
    glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.0F);
    glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.0F);

    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
}

/*----------------------------------------------------------------------*/
/*
** Light position setup.
*/
void setLightPos()
{
    GLfloat light0_pos[4] = { 0.90F, 0.90F, 2.25F, 0.00F };
    GLfloat light0_spotDir[3] = {0.0F, 0.0F, -1.0F};

    glLightfv(GL_LIGHT0, GL_POSITION, light0_pos);
    glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, light0_spotDir);
}

/*----------------------------------------------------------------------*/
/*
** Initial texture settings.
*/
void texture()
{
   GLubyte image[64][64][3];
   int i, j, r, c;
   for(i=0;i<64;i++)
   {
      for(j=0;j<64;j++)
      {
	 c = ((((i&0x8)==0)^((j&0x8))==0))*255;
	 image[i][j][0]= (GLubyte) c;
	 image[i][j][1]= (GLubyte) c;
	 image[i][j][2]= (GLubyte) c;
      }
   }
   glPixelStorei(GL_UNPACK_ALIGNMENT,1);
   glTexImage2D(GL_TEXTURE_2D,0,3,64,64,0,GL_RGB,GL_UNSIGNED_BYTE, image);
   glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
   glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
   glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
   glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
   gluBuild2DMipmaps(GL_TEXTURE_2D,3,64,64,GL_RGB,GL_UNSIGNED_BYTE, image);
}

/*
** Set initial state.
*/
void initSettings(void)
{
    texture();
    glLineWidth(3.0);
    setMenuEntries(true);
}

/*----------------------------------------------------------------------*/
/*
** Set state according to user interaction.
*/
void userSettings(void) 
{
    lighting(currentLighting);
    materials(currentMaterials);

    if (lightingEnabled) {
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
    } else {
	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);
    }

    if (smoothEnabled) {
	glShadeModel(GL_SMOOTH);
    } else {
	glShadeModel(GL_FLAT);
    }

    if(idleSpin) {
        glutIdleFunc(spinCube);
    } else {
	glutIdleFunc(NULL);
    }
    if (texEnabled) {
	glEnable(GL_TEXTURE_2D);
    } else {
	glDisable(GL_TEXTURE_2D);
    }
    if (fastTexture) {
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
    } else {
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    }
    if (mipmapEnabled) {
	glTexParameterf(GL_TEXTURE_2D,
	    GL_TEXTURE_MIN_FILTER,GL_NEAREST_MIPMAP_NEAREST);
    } else {
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    }

    if (fogEnabled) {

	float fogColor[] = {0.7, 0.6, 0.6, 1.0};

	glClearColor(fogColor[0], fogColor[1], fogColor[2], fogColor[3]);
	glEnable(GL_FOG);
	glFogi(GL_FOG_MODE, GL_LINEAR);
	glFogf(GL_FOG_DENSITY, 1.0);
	glFogf(GL_FOG_START, near);
	glFogf(GL_FOG_END, far);
	glFogfv(GL_FOG_COLOR, fogColor);

    } else {
	glDisable(GL_FOG);
	glClearColor(0.0,0.0,0.0,1.0);
    }

    if (lineAAEnabled) {
	glEnable(GL_BLEND);
    } else {
	glDisable(GL_BLEND);
    }

    if (lineAAEnabled) {
	glEnable(GL_LINE_SMOOTH);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    } else {
	glDisable(GL_LINE_SMOOTH);
    }

    if (depthEnabled) {
	glEnable(GL_DEPTH_TEST); /* Enable hidden--surface--removal */
    } else {
	glDisable(GL_DEPTH_TEST);
    }

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    if(perspectiveXform) {
	glFrustum(-1.25, 1.25, -1.25, 1.25, near, far);
	viewxform_z = -5.0;
    } else {
	glOrtho(-2.0, 2.0, -2.0, 2.0, near, far);
	viewxform_z = -5.0;
    }
    glMatrixMode(GL_MODELVIEW);
}

/*----------------------------------------------------------------------*/
/*
** Draw the cube.
*/
GLfloat vertices[][3] = {
    {-1.0,-1.0,-1.0},{1.0,-1.0,-1.0}, {1.0,1.0,-1.0}, {-1.0,1.0,-1.0}, 
    {-1.0,-1.0,1.0}, {1.0,-1.0,1.0}, {1.0,1.0,1.0}, {-1.0,1.0,1.0}
};

GLfloat normals[][3] = {
    {-1.0,-1.0,-1.0},{1.0,-1.0,-1.0}, {1.0,1.0,-1.0}, {-1.0,1.0,-1.0}, 
    {-1.0,-1.0,1.0}, {1.0,-1.0,1.0}, {1.0,1.0,1.0}, {-1.0,1.0,1.0}
};

GLfloat fnormals[][3] = {
    { 0.0, 0.0,-1.0}, { 0.0, 1.0, 0.0}, {-1.0, 0.0, 0.0},
    { 1.0, 0.0, 0.0}, { 0.0, 0.0, 1.0}, { 0.0,-1.0, 0.0}
};

GLfloat colors[][3] = {
    {0.0,0.0,0.0},{1.0,0.0,0.0}, {1.0,1.0,0.0}, {0.0,1.0,0.0}, 
    {0.0,0.0,1.0}, {1.0,0.0,1.0}, {1.0,1.0,1.0}, {0.0,1.0,1.0}
};


void polygon(int a, int b, int c , int d, int face)
{

    /* draw a polygon via list of vertices */

    if(drawLines) {
        glColor3f(1.0,1.0,1.0);
        glBegin(GL_LINE_LOOP);
  	glVertex3fv(vertices[a]);
  	glVertex3fv(vertices[b]);
  	glVertex3fv(vertices[c]);
  	glVertex3fv(vertices[d]);
        glEnd();
    }
    else {
	glNormal3fv(fnormals[face]);
        glBegin(GL_POLYGON);
  	glColor3fv(colors[a]);
  	/* glNormal3fv(normals[a]); */
  	glTexCoord2f(0.0,0.0);
  	glVertex3fv(vertices[a]);
  	glColor3fv(colors[b]);
  	/* glNormal3fv(normals[b]); */
  	glTexCoord2f(0.0,1.0);
  	glVertex3fv(vertices[b]);
  	glColor3fv(colors[c]);
  	/* glNormal3fv(normals[c]); */
  	glTexCoord2f(1.0,1.0);
  	glVertex3fv(vertices[c]);
  	glColor3fv(colors[d]);
  	/* glNormal3fv(normals[d]); */
  	glTexCoord2f(1.0,0.0);
  	glVertex3fv(vertices[d]);
        glEnd();
    }
}

void colorcube(void)
{

    /* map vertices to faces */

    polygon(1,0,3,2,0);
    polygon(3,7,6,2,1);
    polygon(7,3,0,4,2);
    polygon(2,6,5,1,3);
    polygon(4,5,6,7,4);
    polygon(5,4,0,1,5);
}

/*----------------------------------------------------------------------*/
/* 
** These functions implement a simple trackball-like motion control.
*/

float lastPos[3] = {0.0F, 0.0F, 0.0F};
int curx, cury;
int startX, startY;

void
trackball_ptov(int x, int y, int width, int height, float v[3])
{
    float d, a;

    /* project x,y onto a hemi-sphere centered within width, height */
    v[0] = (2.0F*x - width) / width;
    v[1] = (height - 2.0F*y) / height;
    d = (float) sqrt(v[0]*v[0] + v[1]*v[1]);
    v[2] = (float) cos((M_PI/2.0F) * ((d < 1.0F) ? d : 1.0F));
    a = 1.0F / (float) sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
    v[0] *= a;
    v[1] *= a;
    v[2] *= a;
}


void
mouseMotion(int x, int y)
{
    float curPos[3], dx, dy, dz;

    trackball_ptov(x, y, winWidth, winHeight, curPos);

    dx = curPos[0] - lastPos[0];
    dy = curPos[1] - lastPos[1];
    dz = curPos[2] - lastPos[2];

    if (dx || dy || dz) {
	angle = 90.0F * sqrt(dx*dx + dy*dy + dz*dz);

	axis[0] = lastPos[1]*curPos[2] - lastPos[2]*curPos[1];
	axis[1] = lastPos[2]*curPos[0] - lastPos[0]*curPos[2];
	axis[2] = lastPos[0]*curPos[1] - lastPos[1]*curPos[0];

	lastPos[0] = curPos[0];
	lastPos[1] = curPos[1];
	lastPos[2] = curPos[2];
    }

    glutPostRedisplay();
}

void
startMotion(long time, int button, int x, int y)
{
    if (!trackballEnabled) return;

    trackingMouse = true;
    redrawContinue = false;
    startX = x; startY = y;
    curx = x; cury = y;
    trackball_ptov(x, y, winWidth, winHeight, lastPos);
    trackballMove = true;
}

void
stopMotion(long time, int button, int x, int y)
{
    if (!trackballEnabled) return;

    trackingMouse = false;

    if (startX != x || startY != y) {
	redrawContinue = true;
    } else {
	angle = 0.0F;
	redrawContinue = false;
	trackballMove = false;
    }
}

/*----------------------------------------------------------------------*/

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    /* view transform */
    glLoadIdentity();
    glTranslatef(0.0,0.0,viewxform_z);

    if (trackballMove) {
	glPushMatrix();
	    glLoadIdentity();
	    glRotatef(angle, axis[0], axis[1], axis[2]);
	    glMultMatrixf((GLfloat *) trackballXform);
	    glGetFloatv(GL_MODELVIEW_MATRIX, trackballXform);
	glPopMatrix();

    }
    glPushMatrix();
	glMultMatrixf((GLfloat*)lightXform);
	setLightPos();
    glPopMatrix();
    glPushMatrix();
	glMultMatrixf((GLfloat *) objectXform);
	colorcube();
    glPopMatrix();

    glFlush();
    glutSwapBuffers();
}

/*----------------------------------------------------------------------*/

void mouseButton(int button, int state, int x, int y)
{
    switch (button) {
      case GLUT_LEFT_BUTTON:
	 trackballXform = (GLfloat*)objectXform;
	 break;
      case GLUT_MIDDLE_BUTTON:
	 trackballXform = (GLfloat*)lightXform;
	 break;
    }
    switch(state) {
    case GLUT_DOWN:
	startMotion(0,1, x,y);
	break;
    case GLUT_UP:
	stopMotion(0,1, x,y);
	break;
    } 
}

void myReshape(int w, int h)
{
    glViewport(0, 0, w, h);
    winWidth = w;
    winHeight = h;
}

void spinCube()
{
    if (redrawContinue) glutPostRedisplay();
}


void userEventAction(int key) {
    switch(key) {
    case '0':				/* wire frame/polygon  */
      drawLines = !drawLines;
      break;
    case '1':
      smoothEnabled = !smoothEnabled;
      break;
    case '2':				/* lighting */
      lightingEnabled = !lightingEnabled;
      break;
    case '3':				/* texture */
      texEnabled = !texEnabled;
      break;
    case '4':				/* fog */
      fogEnabled = !fogEnabled;
      break;
    case '5':				/* HSR */
      depthEnabled = !depthEnabled;
      break;
    case '6':				/* line aa */
      lineAAEnabled = !lineAAEnabled;
      break;
    case '7':				/* perpective texture */
      fastTexture = !fastTexture;
      break;
    case 'b':
      currentMaterials = &brassMaterials;
      break;
    case 'c':
      currentMaterials = &colorCubeMaterials;
      break;
    case 'C':
      currentLighting = &colorCubeLighting;
      break;
    case 'i':
      idleSpin = !idleSpin;
      break;
    case 'm':				/* mipmapped texture */
      mipmapEnabled = !mipmapEnabled;
      break;
    case 'p':				/* perspective/ortho */
      perspectiveXform = !perspectiveXform;
      break;
    case 'r':
      currentMaterials = &redPlasticMaterials;
      break;
    case 'w':
      currentLighting = &whiteLighting;
      break;
    case 27:
      exit(0);
    default:
      break;
    }
    userSettings();
    glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y)
{
    userEventAction(key);
}

/*----------------------------------------------------------------------*/

typedef struct menuEntryStruct {
    char *label;
    char key;
} menuEntryStruct;

static menuEntryStruct mainMenu[] = {
    "lines/polygons", 		'0',
    "flat/smooth", 		'1',
    "lighting", 		'2',
    "texture", 			'3',
    "fog", 			'4',
    "HSR", 			'5',
    "line smooth", 		'6',
    "motion", 			'i',
    "ortho/perspective", 	'p',
    "quit", 			27,
};
int mainMenuEntries = sizeof(mainMenu)/sizeof(menuEntryStruct);

void selectMain(int choice)
{
    userEventAction(mainMenu[choice].key);
}

static menuEntryStruct materialsMenu[] = {
    "brass", 		'b',
    "white",		'c',
    "red plastic", 	'r',
};
int materialsMenuEntries = sizeof(materialsMenu)/sizeof(menuEntryStruct);

void selectMaterials(int choice)
{
    userEventAction(materialsMenu[choice].key);
}


static menuEntryStruct lightingMenu[] = {
    "white", 		'w',
    "color",		'C',
};
int lightingMenuEntries = sizeof(lightingMenu)/sizeof(menuEntryStruct);

void selectLighting(int choice)
{
    userEventAction(lightingMenu[choice].key);
}

void setMenuEntries(bool init)
{
    int i, sub1, sub2;

    if (init) {
	sub1 = glutCreateMenu(selectMaterials);
	for (i=0; i < materialsMenuEntries; i++) {
	    glutAddMenuEntry(materialsMenu[i].label, i);
	}
	sub2 = glutCreateMenu(selectLighting);
	for (i=0; i < lightingMenuEntries; i++) {
	    glutAddMenuEntry(lightingMenu[i].label, i);
	}
	glutCreateMenu(selectMain);
	for (i=0; i < mainMenuEntries; i++) {
	    glutAddMenuEntry(mainMenu[i].label, i);
	}
	glutAddSubMenu("materials", sub1);
	glutAddSubMenu("lighting", sub2);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
    } else {
    }
}

/*----------------------------------------------------------------------*/

void
main(int argc, char **argv)
{
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(500, 500);
    glutCreateWindow("colorcube");
    glutReshapeFunc(myReshape);
    glutDisplayFunc(display);
    glutIdleFunc(spinCube);
    glutMouseFunc(mouseButton);
    glutMotionFunc(mouseMotion);
    glutKeyboardFunc(keyboard);
    initSettings();
    userSettings();
    glutMainLoop();
}


