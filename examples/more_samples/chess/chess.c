#define PROGRAM "chess"
#define VERSION "1.00"
/*  
 *  author: Nate Robins
 *  email: ndr@pobox.com
 *  www: http://www.pobox.com/~ndr
 */


/* #includes */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#include <GL/glut.h>

#include "glm.h"
#include "gltx.h"
#include "trackball.h"


/* macros */
#define rnd ((float)rand() / ((float)RAND_MAX * 1.7))

/* defines */
#define DFLT_WIN_WIDTH      500
#define DFLT_WIN_HEIGHT     500

/* defines for bounce */

#define MAX_STEPS 32
#define ACCELERATION 0.1
#define FRICTION 0.1
#define ABSORB 0.6
#define PI 3.1415927

#define SELECT_BUFSIZE 128


/* material structure */
typedef struct _Material
{
  GLfloat ambient[4];
  GLfloat diffuse[4];
  GLfloat specular[4];
  GLfloat shininess;
} Material;

typedef enum 
{
  NONE = 0, 
  KING,
  QUEEN,
  BISHOP,
  KNIGHT,
  ROOK,
  PAWN
} Piece;

/* structures for chess pieces */
typedef struct _OGLpiece
{
  float height;
  int dlist;
} OGLpiece;

/* globals */
OGLpiece Pieces[7];  /* array for each type of piece - starts at 1
		      * (king, queen, bishop, knight, rook, pawn)
		      */

Piece board[8][8] =
{
  ROOK, KNIGHT, BISHOP, QUEEN, KING, BISHOP, KNIGHT, ROOK,
  PAWN, PAWN, PAWN, PAWN, PAWN, PAWN, PAWN, PAWN,
  NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
  NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
  NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
  NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
  -PAWN, -PAWN, -PAWN, -PAWN, -PAWN, -PAWN, -PAWN, -PAWN,
  -ROOK, -KNIGHT, -BISHOP, -QUEEN, -KING, -BISHOP, -KNIGHT, -ROOK
};


/* materials for white/black each side, and
 * materials for highlighting
 */
Material White =
{ 
  { 0.5, 0.5, 0.5, 1.0 },
  { 0.8, 0.8, 0.8, 1.0 },
  { 1.0, 1.0, 1.0, 1.0 },
  100.0
};

Material Black = 
{ 
  { 0.1, 0.1, 0.8, 1.0 },
  { 0.1, 0.1, 0.8, 1.0 },
  { 1.0, 1.0, 1.0, 1.0 },
  100.0
};

Material Red =
{
  { 1.0, 0.0, 0.0, 1.0 },
  { 1.0, 0.0, 0.0, 1.0 },
  { 1.0, 1.0, 1.0, 1.0 },
  100.0
};

Material Yellow =
{
  { 1.0, 1.0, 0.0, 1.0 },
  { 1.0, 1.0, 0.0, 1.0 },
  { 1.0, 1.0, 1.0, 1.0 },
  100.0
};

GLuint Select_Buffer[SELECT_BUFSIZE];  /* select buffer */

GLint
Win_Width       = DFLT_WIN_WIDTH,  /* current width of window */
  Win_Height      = DFLT_WIN_HEIGHT, /* current height of window */
  Buffer          = GLUT_DOUBLE,     /* single/double buffered */ 
  Mouse_X         = -1,              /* current X of mouse cursor (on board) */
  Mouse_Y         = -1,              /* current Y of mouse cursor (on board) */
  Old_Mouse_X     = -1,              /* old X of mouse cursor (on board) */
  Old_Mouse_Y     = -1,              /* old Y of mouse cursor (on board) */
  Move_Mouse_X    = -1,              /* move to X of mouse cursor (on board) */
  Move_Mouse_Y    = -1,              /* move to Y of mouse cursor (on board) */
  Chessboard      = 0,               /* display list for chessboard */
  Chessborder     = 0,               /* display list for chessboard border */
  Marble          = 0,               /* display list for marble texture */
  Wood            = 0;               /* display list for wood texture */

GLfloat
X_Rot   = 40.0,    /* rotation about x axis */
  Spin    = 20.0,     /* rotation about y axis */
  Z_Trans = -20.0;   /* z translation of whole scene */

GLboolean
Texturing = GL_TRUE,      /* texturing enabled? */
  Start_Moving = GL_FALSE,   /* start a moving animation? */
  Copyright  = GL_FALSE,    /* display the copyright text? */
  Grabbing  = GL_FALSE,     /* grabbing a piece? */
  Line_Mode = GL_FALSE,     /* polygon fill mode is lines? */
  Step      = GL_FALSE,     /* step once in the rotation? */
  Frozen    = GL_TRUE;      /* currently frozen (not spinning)? */

/* default filenames */
char *Marble_File   = "data/marble.rgb";
char *Wood_File     = "data/woodgrain.rgb";
char *Piece_Files[] = 
{
  "data/king.obj",    /* these MUST be in this order, and the king */
  "data/queen.obj",   /* MUST be the tallest piece                 */
  "data/bishop.obj",  
  "data/knight.obj",
  "data/rook.obj",
  "data/pawn.obj",
};

GLfloat fallht[MAX_STEPS];  /* fall/bounce function */
int Fall_Steps;


/*
 *  usage()
 *
 *    Shows a message on what command line
 *    options are available.  Also prints
 *    version number.
 */
void usage(
	   char *format, ...   /* format e.g., "%s %s\n", "hello", "world" */
	   )
{
  va_list args;
  char buffer[256];
    
  va_start(args, format);
  vsprintf(buffer, format, args);
  va_end(args);
    
  fprintf(stderr, "\n%s version %s\n\n", PROGRAM, VERSION);
  if(format)
    fprintf(stderr, "ERROR - %s\n\n", buffer);
  fprintf(stderr, "usage: %s [options ...]\n", PROGRAM);
  fprintf(stderr, "  where options include:\n");
  fprintf(stderr, "\t-display DISPLAY   " 
	  "server to connect to               [%s]\n", ":0");
  fprintf(stderr, "\t-geometry WxH+X+Y  " 
	  "window geometry specification      [%dx%d+0+0]\n",
	  (int)Win_Width, (int)Win_Height);
  fprintf(stderr, "\t-direct            " 
	  "use a direct rendering context     [default]\n");
  fprintf(stderr, "\t-indirect          " 
	  "use an indirect rendering context\n");
  fprintf(stderr, "\t-double  (-db)     " 
	  "use a double buffered context      %s\n",
	  Buffer == GLUT_DOUBLE ? "[default]" : "");
  fprintf(stderr, "\t-single  (-sb)     " 
	  "use a single buffered context      %s\n",
	  Buffer == GLUT_SINGLE ? "[default]" : "");
  fprintf(stderr, "\t-host HOSTNAME     "
	  "chess server to connect to         [chess.onenet.net]\n");
  fprintf(stderr, "\n");
  exit(1);

}
    
/*
 *  parse()
 *
 *    Parses the command line arguments and sets
 *    appropriate conditions for startup.
 */
void parse(
	   int argc,    /* argc passed into main() */
	   char **argv  /* argv passed into main() */
	   )
{
  while(--argc)
    {
      ++argv;
	
      if(!strcmp("-help", argv[0]) || !strcmp("-h", argv[0]))
        {
	  usage(NULL);
        }
      else if(!strcmp("-display", argv[0]))
        {
	  if(argc == 1)
	    usage("Option `%s' requires an argument!", argv[0]);
	  --argc; ++argv;
	  /* GLUT takes care of this argument */
        }
      else if(!strcmp("-geometry", argv[0]))
        {
	  if(argc == 1)
	    usage("Option `%s' requires an argument!", argv[0]);
	  --argc; ++argv;
	  /* GLUT takes care of this argument */
        }
      else if(!strcmp("-direct", argv[0]))
        {
	  /* GLUT takes care of this argument */
        }
      else if(!strcmp("-indirect", argv[0]))
        {
	  /* GLUT takes care of this argument */
        }
      else if(!strcmp("-double", argv[0]) || !strcmp("-db", argv[0]))
	{
	  Buffer = GLUT_DOUBLE;
	}
      else if(!strcmp("-single", argv[0]) || !strcmp("-sb", argv[0]))
        {
	  Buffer = GLUT_SINGLE;
        }
      else
	usage("Invalid option `%s'!", argv[0]);
    }
}

/*
 *  lighting()
 *
 *    Initialize lighting for the scene.
 */
void lighting()
{
  GLfloat ambient[]  = { 0.0, 0.0, 0.0, 1.0 };  /* get ambient from global */
  GLfloat diffuse[]  = { 1.0, 1.0, 1.0, 1.0 };  /* diffuse and specular */
  GLfloat specular[] = { 1.0, 1.0, 1.0, 1.0 };  /* should be the same   */
  GLfloat global_ambient[] = { 0.5, 0.5, 0.5, 1.0 };

  /* light model */
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);

  /* light 0 */
  glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
  glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
  glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
}

/*
 *  texture()
 *
 *    Initialize all texture images needed.
 *    Creates appropriate display lists.
 */
void texture()
{
  GLTXimage *image;

  image = gltxReadRGB(Marble_File);
  Marble = glGenLists(1);
  glNewList(Marble, GL_COMPILE_AND_EXECUTE);
  {
    /* modulate the texture for highlights */
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    /* bi-linear mipmapping */
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    /* build the mipmaps */
    gluBuild2DMipmaps(GL_TEXTURE_2D, 3, image->width, image->height,
		      GL_RGB, GL_UNSIGNED_BYTE, image->data);

  } glEndList();
  gltxDelete(image);

  image = gltxReadRGB(Wood_File);
  Wood = glGenLists(1);
  glNewList(Wood, GL_COMPILE_AND_EXECUTE);
  {
    /* modulate the texture for highlights */
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    /* bi-linear mipmapping */
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    /* build the mipmaps */
    gluBuild2DMipmaps(GL_TEXTURE_2D, 3, image->width, image->height,
		      GL_RGB, GL_UNSIGNED_BYTE, image->data);

  } glEndList();
  gltxDelete(image);
}

/*
 *  object()
 *
 *    Initialize all the object to be used and
 *    make appropriate display lists.
 */
void object()
{
  GLMmodel *object;
  GLfloat x, z, scalefactor = 0.0;
  GLfloat brown[]    = { 0.857, 0.147, 0.000, 1.0 };
  GLfloat green[]    = { 0.243, 0.874, 0.143, 1.0 };
  GLfloat blue[]     = { 0.123, 0.32, 0.798, 1.0 };
  GLfloat specular[] = { 0.1, 0.1, 0.1, 1.0 };
  GLfloat dimensions[3];
  int i, name;

#define YT -1.0          /* y translation */
#define OB  5.0          /* outside border */
#define IB  4.0          /* inside border */
#define OBB (OB + 0.25)  /* outside border of bevel */
#define BV (YT - 0.25)    /* bevel drop */
#define T  (BV - 0.75)   /* thickness of board */

  Chessborder = glGenLists(1);
  glNewList(Chessborder, GL_COMPILE);
  {
    /* set the color */
    glMaterialfv(GL_FRONT, GL_DIFFUSE, brown);
    glMaterialfv(GL_FRONT, GL_AMBIENT, brown);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT, GL_SHININESS, 10.0);

    /* chess board border */
    glBegin(GL_QUADS);
    {
      /* horizontal part */
      glTexCoord2f(0.0, 0.0);      /* back */
      glNormal3f(0.0, 1.0, 0.0);
      glVertex3f(-OB, YT, -OB);
      glTexCoord2f(0.0, 1.0);
      glNormal3f(0.0, 1.0, 0.0);
      glVertex3f(-IB, YT, -IB);
      glTexCoord2f(1.0, 1.0);
      glNormal3f(0.0, 1.0, 0.0);
      glVertex3f( IB, YT, -IB);
      glTexCoord2f(1.0, 0.0);
      glNormal3f(0.0, 1.0, 0.0);
      glVertex3f( OB, YT, -OB);

      glTexCoord2f(0.0, 0.0);      /* right */
      glNormal3f(0.0, 1.0, 0.0);
      glVertex3f( OB, YT, -OB);
      glTexCoord2f(0.0, 1.0);
      glNormal3f(0.0, 1.0, 0.0);
      glVertex3f( IB, YT, -IB);
      glTexCoord2f(1.0, 1.0);
      glNormal3f(0.0, 1.0, 0.0);
      glVertex3f( IB, YT,  IB);
      glTexCoord2f(1.0, 0.0);    
      glNormal3f(0.0, 1.0, 0.0);
      glVertex3f( OB, YT,  OB);

      glTexCoord2f(0.0, 0.0);      /* front */
      glNormal3f(0.0, 1.0, 0.0);
      glVertex3f( OB, YT,  OB);
      glTexCoord2f(0.0, 1.0);
      glNormal3f(0.0, 1.0, 0.0);
      glVertex3f( IB, YT,  IB);
      glTexCoord2f(1.0, 1.0);
      glNormal3f(0.0, 1.0, 0.0);
      glVertex3f(-IB, YT,  IB); 
      glTexCoord2f(1.0, 0.0);
      glNormal3f(0.0, 1.0, 0.0);
      glVertex3f(-OB, YT,  OB);  

      glTexCoord2f(0.0, 0.0);      /* left */
      glNormal3f(0.0, 1.0, 0.0);
      glVertex3f(-OB, YT,  OB);  
      glTexCoord2f(0.0, 1.0);
      glNormal3f(0.0, 1.0, 0.0);
      glVertex3f(-IB, YT,  IB); 
      glTexCoord2f(1.0, 1.0);
      glNormal3f(0.0, 1.0, 0.0);
      glVertex3f(-IB, YT, -IB);
      glTexCoord2f(1.0, 0.0);
      glNormal3f(0.0, 1.0, 0.0);
      glVertex3f(-OB, YT, -OB);

      /* bevel */
      glTexCoord2f(1.0, 1.0);       /* back */
      glNormal3f(-1.0, 1.0, -1.0);  
      glVertex3f(-OBB, BV, -OBB);
      glTexCoord2f(1.0, 0.0);
      glNormal3f(-1.0, 1.0, -1.0);
      glVertex3f(-OB, YT, -OB);
      glTexCoord2f(0.0, 1.0);
      glNormal3f(1.0, 1.0, -1.0);
      glVertex3f( OB, YT, -OB);
      glTexCoord2f(0.0, 0.0);
      glNormal3f(1.0, 1.0, -1.0);
      glVertex3f( OBB, BV, -OBB); 

      glTexCoord2f(1.0, 1.0);       /* right */
      glNormal3f(1.0, 1.0, -1.0);  
      glVertex3f( OBB, BV, -OBB); 
      glTexCoord2f(1.0, 0.0);
      glNormal3f(1.0, 1.0, -1.0);
      glVertex3f( OB, YT, -OB);
      glTexCoord2f(0.0, 1.0);
      glNormal3f(1.0, 1.0, 1.0);
      glVertex3f( OB, YT,  OB);
      glTexCoord2f(0.0, 0.0);
      glNormal3f(1.0, 1.0, 1.0);   
      glVertex3f( OBB, BV,  OBB);  

      glTexCoord2f(1.0, 1.0);      /* front */
      glNormal3f(1.0, 1.0, 1.0);   
      glVertex3f( OBB, BV,  OBB);  
      glTexCoord2f(1.0, 0.0);
      glNormal3f(1.0, 1.0, 1.0);
      glVertex3f( OB, YT,  OB);
      glTexCoord2f(0.0, 1.0);
      glNormal3f(-1.0, 1.0, 1.0);
      glVertex3f(-OB, YT,  OB); 
      glTexCoord2f(0.0, 0.0);
      glNormal3f(-1.0, 1.0, 1.0);
      glVertex3f(-OBB, BV,  OBB);  

      glTexCoord2f(1.0, 1.0);      /* left */
      glNormal3f(-1.0, 1.0, 1.0);  
      glVertex3f(-OBB, BV,  OBB);  
      glTexCoord2f(1.0, 0.0);
      glNormal3f(-1.0, 1.0, 1.0);
      glVertex3f(-OB, YT,  OB); 
      glTexCoord2f(0.0, 1.0);
      glNormal3f(-1.0, 1.0, -1.0);
      glVertex3f(-OB, YT, -OB);
      glTexCoord2f(0.0, 0.0);
      glNormal3f(-1.0, 1.0, -1.0);
      glVertex3f(-OBB, BV, -OBB);
    } glEnd();

    glBegin(GL_QUADS);
    {
      /* sides - though similar to the horizontal pieces,
       * these have to be done in the opposite order
       * that the horizontal parts were done to
       * ensure CCW ordering of polygons.
       */
      glTexCoord2f(0.0, 0.0);      /* back */
      glNormal3f(0.0, 0.0, -1.0);  
      glVertex3f(-OBB, T, -OBB); 
      glTexCoord2f(0.0, 1.0);
      glNormal3f(0.0, 0.0, -1.0);
      glVertex3f(-OBB, BV, -OBB);
      glTexCoord2f(1.0, 0.0);
      glNormal3f(0.0, 0.0, -1.0);
      glVertex3f( OBB, BV, -OBB);
      glTexCoord2f(1.0, 1.0);
      glNormal3f(0.0, 0.0, -1.0);
      glVertex3f( OBB, T, -OBB);

      glTexCoord2f(0.0, 0.0);      /* right */
      glNormal3f(1.0, 0.0, 0.0);  
      glVertex3f( OBB, T, -OBB);
      glTexCoord2f(0.0, 1.0);
      glNormal3f(1.0, 0.0, 0.0);
      glVertex3f( OBB, BV, -OBB);
      glTexCoord2f(1.0, 0.0);
      glNormal3f(1.0, 0.0, 0.0);
      glVertex3f( OBB, BV,  OBB);
      glTexCoord2f(1.0, 1.0);
      glNormal3f(1.0, 0.0, 0.0);
      glVertex3f( OBB, T,  OBB); 

      glTexCoord2f(0.0, 0.0);      /* front */
      glNormal3f(0.0, 0.0, 1.0);  
      glVertex3f( OBB, T,  OBB); 
      glTexCoord2f(0.0, 1.0);
      glNormal3f(0.0, 0.0, 1.0);
      glVertex3f( OBB, BV,  OBB);
      glTexCoord2f(1.0, 0.0);
      glNormal3f(0.0, 0.0, 1.0);
      glVertex3f(-OBB, BV,  OBB);
      glTexCoord2f(1.0, 1.0);
      glNormal3f(0.0, 0.0, 1.0);
      glVertex3f(-OBB, T,  OBB); 

      glTexCoord2f(0.0, 0.0);      /* left */
      glNormal3f(-1.0, 0.0, 0.0);  
      glVertex3f(-OBB, T,  OBB); 
      glTexCoord2f(0.0, 1.0);
      glNormal3f(-1.0, 0.0, 0.0);
      glVertex3f(-OBB, BV,  OBB);
      glTexCoord2f(1.0, 0.0);
      glNormal3f(-1.0, 0.0, 0.0);
      glVertex3f(-OBB, BV, -OBB);
      glTexCoord2f(1.0, 1.0);
      glNormal3f(-1.0, 0.0, 0.0);
      glVertex3f(-OBB, T, -OBB);
    } glEnd();

  } glEndList();

  Chessboard = glGenLists(1);
  glNewList(Chessboard, GL_COMPILE);
  {
    /* set the material */
    glMaterialfv(GL_FRONT, GL_DIFFUSE, green);
    glMaterialfv(GL_FRONT, GL_AMBIENT, green);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT, GL_SHININESS, 10.0);

    /* use i to count the tiles */
    i = name = 0;
    for(x = -4.0; x < 4.0; x += 1.0)
      {
	for(z = -4.0; z < 4.0; z += 1.0)
	  {
	    i++;
	    /* draw only every other tile -
	     * use i to determine if we need
	     * to draw this tile or not
	     */
	    if(i % 2)
	      {
		glLoadName(name++);
		glBegin(GL_QUADS);
		{
		  glTexCoord2f(0.0 + rnd, 1.0 + rnd);
		  glNormal3f(0.0, 1.0, 0.0);
		  glVertex3f(x, YT, z);
		  glTexCoord2f(0.0 + rnd, 0.0 + rnd);
		  glNormal3f(0.0, 1.0, 0.0);
		  glVertex3f(x, YT, z + 1.0);
		  glTexCoord2f(1.0 - rnd, 0.0 + rnd);
		  glNormal3f(0.0, 1.0, 0.0);
		  glVertex3f(x + 1.0, YT, z + 1.0);
		  glTexCoord2f(1.0 - rnd, 1.0 - rnd);
		  glNormal3f(0.0, 1.0, 0.0);
		  glVertex3f(x + 1.0, YT, z);
		} glEnd();
	      }
	  }
	    
	/* add 1 to i, so we start on the
	 * opposite type of number (odd/even)
	 * each row
	 */
	i++;
      }

    /* do it again so we get the
     * other tiles, but this time in
     * a different material - set i
     * to 1, so that we start on a
     * different tile than before
     */
    i = 1;
	
    /* set the material */
    glMaterialfv(GL_FRONT, GL_DIFFUSE, blue);
    glMaterialfv(GL_FRONT, GL_AMBIENT, blue);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT, GL_SHININESS, 10.0);
	
    for(x = -4.0; x < 4.0; x += 1.0)
      {
	for(z = -4.0; z < 4.0; z += 1.0)
	  {
	    i++;
	    /* draw only every other tile -
	     * use i to determine if we need
	     * to draw this tile or not
	     */
	    if(i % 2)
	      {
		glLoadName(name++);
		glBegin(GL_QUADS);
		{
		  glTexCoord2f(0.0 + rnd, 1.0 + rnd);
		  glNormal3f(0.0, 1.0, 0.0);
		  glVertex3f(x, YT, z);
		  glTexCoord2f(0.0 + rnd, 0.0 + rnd);
		  glNormal3f(0.0, 1.0, 0.0);
		  glVertex3f(x, YT, z + 1.0);
		  glTexCoord2f(1.0 - rnd, 0.0 + rnd);
		  glNormal3f(0.0, 1.0, 0.0);
		  glVertex3f(x + 1.0, YT, z + 1.0);
		  glTexCoord2f(1.0 - rnd, 1.0 - rnd);
		  glNormal3f(0.0, 1.0, 0.0);
		  glVertex3f(x + 1.0, YT, z);
		} glEnd();
	      }
	  }
	    
	/* add 1 to i, so we start on the
	 * opposite type of number (odd/even)
	 * each row
	 */
	i++;
      }

  } glEndList();

  /* get all the chess pieces */
  for(i = 1; i <= 6; i++)
    {
      /* read the object in */
      object = glmReadOBJ(Piece_Files[i - 1]);
      if(!object) /* bail if no object - error message already sent */
	exit(1);
	
      /* scale the first object (the king and
       * also the biggest) to between -1.0 and 1.0
       * and move it to (0, 0, 0), and keep the
       * scalefactor so that we can scale all the
       * other peices equally.
       */
      if(!scalefactor) {
	scalefactor = glmUnitize(object);
      } else {
	glmScale(object, scalefactor);
      }

      glmScale(object, 1.2);
	
#if 0
      /* generate normals if this
       * object doesn't have them.
       */
      if(object->num_normals == 0)
	tdGenSmoothNormals(object);
	
      /* generate texture vertices if
       * this object doesn't have them
       */
      if(object->num_texvertices == 0)
	tdGenSpheremapTexvertices(object);
#endif

      /* build a display list */
      Pieces[i].dlist = glmList(object, GLM_SMOOTH);
	
      glmDimensions(object, dimensions);
      /* grab the height for translating */
      Pieces[i].height = dimensions[1] / 2.0;
	
      /* nuke it, we don't need it anymore */
      glmDelete(object);
    }
}

/*
 *  init()
 *
 *    Initializes state and display lists, etc.
 */
void init()
{
  int cnt;
  GLfloat ht, vc;

  tbInit(GLUT_MIDDLE_BUTTON);

  /* depth */
  glDepthFunc(GL_LEQUAL);
  glEnable(GL_DEPTH_TEST);

  /* lighting */
  lighting();
  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHTING);

  /* blending */
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  /* culling */
  glCullFace(GL_BACK);
  glEnable(GL_CULL_FACE);

  /* polygon mode */
  Line_Mode ?	glPolygonMode(GL_FRONT, GL_LINE) 
    :	glPolygonMode(GL_FRONT, GL_FILL);

  /* select buffer */
  glSelectBuffer(SELECT_BUFSIZE, Select_Buffer);

  /* shading mode */
  glShadeModel(GL_SMOOTH);

  /* clearcolor */
  glClearColor(0.0, 0.0, 0.0, 0.0);

  /* texture */
  texture();
  Texturing ? glEnable(GL_TEXTURE_2D) : glDisable(GL_TEXTURE_2D);

  /* object */
  object();

  /* init fallht array */
  
  ht = 1.0;  /* initial height (from movement) */
  vc = 0.0;    /* initial velocity */
    
  for (cnt = 0; cnt < MAX_STEPS; cnt++)
    {
      ht -= vc;
      vc += ACCELERATION;
      if (ht < 0) 
	{
	  ht = 0;
	  vc = (-vc * (1 - ABSORB)) + FRICTION;
	  if (vc > 0) break;
	}
      fallht[cnt] = ht;
    }
  fallht[cnt] = 0;
  Fall_Steps = cnt + 1;
}

/*
 *  reshape()
 *
 *    Reshape callback routine.  Resets the windows
 *    viewport and matrices.  Also sets the light 
 *    positions.
 */
void reshape(int width, int height)
{
  Win_Width = width;
  Win_Height = height;

  tbReshape(width, height);

  /* reset the viewport */
  glViewport(0, 0, width, height);

  /* reset projection */
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  /* Putting in 1.0 for the second argument to 
   * gluPerspective() causes the aspect ratio to be
   * calculated according to the actual size of the
   * window.  If it is long and skinny, the picture
   * drawn will be long and skinny (distorted).
   * It can be made to always respect an aspect ratio
   * by putting (GLfloat)width / (GLfloat)height
   * in as the second argument to gluPerspective().
   */
  gluPerspective(30.0, 1.0, 1.0, 64.0);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

}

/* 
 *  text()
 *
 *    General purpose text routine.  Draws a string
 *    according to format in a stroke font at x, y
 *    after scaling it by scale in all three dimensions.
 */
void text(float x, float y, float scale, char *format, ...)
{
  va_list args;
  char buffer[200], *p;

  va_start(args, format);
  vsprintf(buffer, format, args);
  va_end(args);

  glPushMatrix();
  glTranslatef(x, y, 0.0);
  glScalef(scale, scale, scale);
  for(p = buffer; *p; p++)
    glutStrokeCharacter(GLUT_STROKE_ROMAN, *p);
  glPopMatrix();
}

/*
 *
 *
 *
 */
void selection(int x, int y)
{
  static GLint vp[4];
  int hits;

  hits = glRenderMode(GL_SELECT);
  glInitNames();
  glPushName((unsigned)-1);

  glPushMatrix();
  {
    /* reset the viewport */
    glViewport(0, 0, Win_Width, Win_Height);
	
    /* reset the projection */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    /* get the current viewport */
    glGetIntegerv(GL_VIEWPORT, vp);

    /* set up a picking matrix */
    gluPickMatrix(x, Win_Height-y, 5.0, 5.0, vp);

    /* set the viewing frustum */
    gluPerspective(30.0, 1.0, 1.0, 64.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    /* default translation - translate in Z because
     * most everything is drawn at or around (0, 0, 0)
     */
    glTranslatef(0.0, 0.0, Z_Trans);
	
    /* rotation around the X axis (tilt toward plane of screen) */
    glRotatef(X_Rot, 1.0, 0.0, 0.0);	
    glRotatef(Spin, 0.0, 1.0, 0.0);
	
    glCallList(Chessboard);

  } glPopMatrix();

  /* see how many hits we got */
  hits = glRenderMode(GL_RENDER); 
	
  /* reset to the correct matrices */
  reshape(Win_Width, Win_Height);

  /* change the selection buffer name into an x, y pair */
  if(hits)
    {
      Mouse_X = (Select_Buffer[3] % 32) / 4;
      Mouse_Y = 7 - ((Select_Buffer[3] % 4) * 2 + 
		     (((Select_Buffer[3] % 8) / 4) ^ 
		      (Select_Buffer[3] / 32)));
    }
  else
    {
      Mouse_X = -1;
      Mouse_Y = -1;
    }
}

/*
 *  idle()
 *
 *    Idle function.  Does all the rendering.
 */
void idle(void)
{
#define STEP 20.0

  static Piece piece = 0, signed_piece, moving_piece = 0;
  static int i, j, steps, fallstep;
  static GLboolean rising, falling, moving = GL_FALSE;
  static float dx, dy, dh, start_x, start_y, end_x, end_y;
  static GLfloat light_position0[] = {  1.0, 1.0, 1.0, 0.0 };
 
#if 0
  if(!Frozen || Step)
    {
      Spin += 1.0;
      if(Spin > 360.0)    /* reset after full rotation - avoid an overflow */
	Spin -= 360.0;

      Step = GL_FALSE;
    }
#endif

  if(Start_Moving && Old_Mouse_Y != -1 && Old_Mouse_X != -1)
    {
      moving_piece = board[Old_Mouse_Y][Old_Mouse_X];
      board[Old_Mouse_Y][Old_Mouse_X] = NONE;

      start_x = Old_Mouse_X;
      start_y = Old_Mouse_Y;
      end_x = Move_Mouse_X;
      end_y = Move_Mouse_Y;

      dx = dy = dh = 0.0;  /* reset the movement deltas */
      steps = 1;           /* reset the number of steps */

      rising = GL_TRUE;    /* start off by raising the piece */
      moving = GL_TRUE;    /* we are moving! */
      Start_Moving = GL_FALSE;
    }

  /* clear the buffer */
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  /* show stats */
  if(Copyright)
    {
      glPushMatrix();
      {
	glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT);
	glDisable(GL_LIGHTING);
	glEnable(GL_LINE_SMOOTH);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glTranslatef(0.0, 0.0, -1.0);
	glColor3f(1.0, 1.0, 1.0);
	text(-0.25, -0.25, 0.000155,
	     "Chess  Copyright (C) 1995 Evans & Sutherland");
	glPopAttrib();
      } glPopMatrix();
    }

  glPushMatrix();
  {
    /* default translation - translate in Z because
     * most everything is drawn at or around (0, 0, 0)
     */
    glTranslatef(0.0, 0.0, Z_Trans);
	
    /* position the lights */
    glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
	
    tbMatrix();

#if 0
    /* rotation around the X axis (tilt toward plane of screen) */
    glRotatef(X_Rot, 1.0, 0.0, 0.0);	
    glRotatef(Spin, 0.0, 1.0, 0.0);
#endif

    /* draw the actual chessboard and its
     * accompanying border
     */
    glPushMatrix();
    {
      if(Texturing)
	{
	  glEnable(GL_TEXTURE_2D);
	  glCallList(Wood);
	  glCallList(Chessborder);
	  glCallList(Marble);
	  glCallList(Chessboard);
	  glDisable(GL_TEXTURE_2D); 
	}
      else
	{
	  glCallList(Chessborder);
	  glCallList(Chessboard);
	}
    } glPopMatrix();

    /* translate (everything that is to follow)
     * to the center of the near left corner tile
     * this makes the translations for each piece
     * easy because we get them as x,y pairs 
     * according to the following chart
     *                 (black side) 
     *       +---+---+---+---+---+---+---+---+
     *     7 |0,7| . | . | . |4,7|5,7|6,7|7,7|
     *       +---+---+---+---+---+---+---+---+
     *     6 |0,6| . | . | . | . |5,6|6,6|7,6|
     *       +---+---+---+---+---+---+---+---+
     *     5 |0,5| . | . | . | . | . |6,5|7,5|
     *       +---+---+---+---+---+---+---+---+
     *     4 |0,4| . | . |   | . | . |6,4|7,4|
     *  y    +---+---+---+---+---+---+---+---+
     *     3 |0,3|1,3| . | . | . | . | . |7,3|
     *       +---+---+---+---+---+---+---+---+
     *     2 |0,2|1,2| . | . | . | . | . |7,2|
     *       +---+---+---+---+---+---+---+---+
     *     1 |0,1|1,1|2,1| . | . | . | . |7,1|
     *       +---+---+---+---+---+---+---+---+
     *     0 |0,0|1,0|2,0|3,0| . | . | . |7,0|
     *       +---+---+---+---+---+---+---+---+
     *         0   1   2   3   4   5   6   7
     *                x (white side)
     * so all that needs to be done is a translate
     * in +X (according to x), and a translate in
     * -Z (according to y) (NOTE the negative Z!).
     */
    glTranslatef(-IB + 0.5, YT, IB - 0.5);

    /*  draw the highlighting square to indicate
     *  which tile the cursor is currently on
     */
    glPushMatrix();
    {
      /* if we aren't grabbing, don't draw any of the
       * position indicator squares
       */
      if(Grabbing)
	{
	  /* make it a thick square */
	  glLineWidth(4.0);

	  if(Grabbing && Old_Mouse_X != -1)
	    {
	      /* set the square color */
	      glMaterialfv(GL_FRONT, GL_DIFFUSE,
			   Yellow.diffuse);
	      glMaterialfv(GL_FRONT, GL_AMBIENT,
			   Yellow.ambient);
	      glMaterialfv(GL_FRONT, GL_SPECULAR,
			   Yellow.specular);
	      glMaterialf(GL_FRONT, GL_SHININESS,
			  Yellow.shininess);

	      glPushMatrix();
	      {
		/* translate to the correct tile according
		 * to the Mouse_X, and Mouse_Y (really Z, tho)
		 * variables that are set in the selection()
		 * routine.  translate up a bit too, so that
		 * we don't clip with the board.
		 */
		glTranslatef((float)Old_Mouse_X,
			     0.01, (float)-Old_Mouse_Y);
			
		/* draw the square */
		glBegin(GL_LINE_LOOP);
		{
		  glVertex3f(-0.5, 0.0, -0.5);
		  glVertex3f(-0.5, 0.0,  0.5);
		  glVertex3f( 0.5, 0.0,  0.5);
		  glVertex3f( 0.5, 0.0, -0.5);
		} glEnd();

	      } glPopMatrix();
	    }

	  /* set the square color */
	  glMaterialfv(GL_FRONT, GL_DIFFUSE,
		       Red.diffuse);
	  glMaterialfv(GL_FRONT, GL_AMBIENT,
		       Red.ambient);
	  glMaterialfv(GL_FRONT, GL_SPECULAR,
		       Red.specular);
	  glMaterialf(GL_FRONT, GL_SHININESS,
		      Red.shininess);

	  /* translate to the correct tile according
	   * to the Mouse_X, and Mouse_Y (really Z, tho)
	   * variables that are set in the selection()
	   * routine.  translate up a bit too, so that
	   * we don't clip with the board.
	   */
	  glTranslatef((float)Mouse_X, 0.01, (float)-Mouse_Y);

	  /* draw the square */
	  glBegin(GL_LINE_LOOP);
	  {
	    glVertex3f(-0.5, 0.0, -0.5);
	    glVertex3f(-0.5, 0.0,  0.5);
	    glVertex3f( 0.5, 0.0,  0.5);
	    glVertex3f( 0.5, 0.0, -0.5);
	  } glEnd();

	  glLineWidth(1.0);
	}
    } glPopMatrix();

    /* KLUDGE - change the face ordering for
     * the chess pieces (they are all CW), but
     * eventually, need to change the chess
     * pieces to be CCW.
     */
    /*	glFrontFace(GL_CW); */

    /* draw the next frame in the animated moving
     * sequence if we are currently in one.
     */
    glPushMatrix();
    {
      if(moving)
	{
	  /* calculate rising delta, if we are rising */
	  if(rising)
	    {
	      dh += 0.1;
	      if(dh > 1.0)
		rising = GL_FALSE;  /* done rising */
	    }
	  else if(falling)  /* calculate falling delta, if falling */
	    {
	      dh = fallht[fallstep];
	      fallstep++;
	      if(fallstep == Fall_Steps)
		{
		  falling = GL_FALSE;  /* done falling */
		  moving = GL_FALSE;   /* we are done moving as well */
		  board[Move_Mouse_Y][Move_Mouse_X] = moving_piece;
		}
	    } 
	  else  /* must be moving over to the new tile */
	    {
	      dx += (end_x - start_x) / STEP;
	      dy += (end_y - start_y) / STEP;
	      steps++;
	      if(steps > STEP)
		{
		  falling = GL_TRUE;  /* done moving over, so fall now */
		  fallstep = 0;
		}
	    }

	  if(moving_piece < 0)  /* -piece = piece is a black one */
	    {
	      /* send a color for the piece */
	      glMaterialfv(GL_FRONT, GL_DIFFUSE,
			   Black.diffuse);
	      glMaterialfv(GL_FRONT, GL_AMBIENT,
			   Black.ambient);
	      glMaterialfv(GL_FRONT, GL_SPECULAR,
			   Black.specular);
	      glMaterialf(GL_FRONT, GL_SHININESS,
			  Black.shininess);
	    }
	  else  /* +piece = piece is a white one */
	    {
	      /* send a color for the piece */
	      glMaterialfv(GL_FRONT, GL_DIFFUSE,
			   White.diffuse);
	      glMaterialfv(GL_FRONT, GL_AMBIENT,
			   White.ambient);
	      glMaterialfv(GL_FRONT, GL_SPECULAR,
			   White.specular);
	      glMaterialf(GL_FRONT, GL_SHININESS,
			  White.shininess);
	    }
		
	  /* set the actual piece (not negative) to 
	   * index the Pieces array
	   */
	  piece = moving_piece < 0 ? -moving_piece : moving_piece;

	  /* translate to the start tile & height */
	  glTranslatef(start_x + dx,
		       Pieces[piece].height + dh,
		       -start_y - dy);

	  /* rotate the knight if it is on the 
	   * white side -- it only matches if
	   * the piece is a white KNIGHT (+KNIGHT)
	   */
	  if(moving_piece == KNIGHT)
	    glRotatef(180.0, 0.0, 1.0, 0.0);
		
	  /* draw the piece */
	  glCallList(Pieces[piece].dlist);
	}

    } glPopMatrix();
	
    /* go through the entire board and draw a piece if
     * there is one on the current tile.  draw front
     * to back (closest row first, furthest row last)
     */
    glPushMatrix();
    {
      /* draw each piece on the board */
	    
      for(i = 0; i < 8; i++)
	{
	  for(j = 0; j < 8; j++)
	    {
	      glPushMatrix();
	      {
		if (board[i][j] != 0) 
		  /* there is a piece here */
		  {
		    /* get the piece with its sign */
		    /* if an attacked sq, get that piece instead */
			    
		    signed_piece = board[i][j];

		    /* set the actual piece (not negative) to 
		     * index the Pieces array
		     */
		    piece = signed_piece < 0 ? -signed_piece
		      : signed_piece;
			    
		    if(signed_piece < 0)  /* -piece = piece is black */
		      {
			/* send a color for the piece */
			glMaterialfv(GL_FRONT, GL_DIFFUSE,
				     Black.diffuse);
			glMaterialfv(GL_FRONT, GL_AMBIENT,
				     Black.ambient);
			glMaterialfv(GL_FRONT, GL_SPECULAR,
				     Black.specular);
			glMaterialf(GL_FRONT, GL_SHININESS,
				    Black.shininess);
		      }
		    else  /* +piece = piece is a white one */
		      {
			/* send a color for the piece */
			glMaterialfv(GL_FRONT, GL_DIFFUSE,
				     White.diffuse);
			glMaterialfv(GL_FRONT, GL_AMBIENT,
				     White.ambient);
			glMaterialfv(GL_FRONT, GL_SPECULAR,
				     White.specular);
			glMaterialf(GL_FRONT, GL_SHININESS,
				    White.shininess);
		      }
			    
		    /* translate to the correct tile & height */
		    glTranslatef(j, Pieces[piece].height, -i);
			    
		    /* rotate the knight if it is on the 
		     * white side -- it only matches if
		     * the piece is a white KNIGHT (+KNIGHT)
		     */
		    if(signed_piece == KNIGHT)
		      glRotatef(180.0, 0.0, 1.0, 0.0);

		    /* draw the piece */
		    glCallList(Pieces[piece].dlist);
		  }
	      } glPopMatrix();
	    }
	}
	    
    } glPopMatrix();
	
    /* KLUDGE - reset the front faces to the
     * "proper" direction
     */
    glFrontFace(GL_CCW);
	    
  } glPopMatrix();

  /* swap the buffers and flush */
  glutSwapBuffers();
}

/*
 *  display()
 *
 *    Display callback routine.  Simply call the 
 *    idle function, because it does all the rendering.
 */
void display(void)
{
  idle();
}

/*
 *  keyboard()
 *
 *   Normal key press callback routine.
 */
void keyboard(unsigned char key, int x, int y)
{
  switch(key)
    {
    case 27:                 /* Esc quits */
      exit(1);
      break;

    case 'Z':                /* zoom in */
      Z_Trans += 0.25;
      if(Z_Trans > -1.75)  /* check for clipping */
	Z_Trans = -1.75;
      break;

    case 'z':                /* zoom out */
      Z_Trans -= 0.25;
      break;
	
    case 'X':                /* tilt up */
      X_Rot -= 2.0;
      break;
	
    case 'x':                /* tilt down */
      X_Rot += 2.0;
      break;

    case '\r':
      Step = GL_TRUE;
      break;

    case 't':
      Texturing = !Texturing;
      Texturing ? glEnable(GL_TEXTURE_2D) : glDisable(GL_TEXTURE_2D);
      break;

    case 'p':
      Line_Mode = !Line_Mode;
      Line_Mode ? glPolygonMode(GL_FRONT, GL_LINE) 
	: glPolygonMode(GL_FRONT, GL_FILL);
      break;

    default:
      break;
    }

  glutPostRedisplay();

}

/*
 *  special()
 *
 *    Special keys (non-ascii/non-printable) callback routine.
 */
void special(int key, int x, int y)
{
  switch(key)
    {
    case GLUT_KEY_F12:
      Frozen = !Frozen;
      break;

    case GLUT_KEY_HOME:
      Copyright = !Copyright;
      break;

    default:
      break;

    }

  glutPostRedisplay();
}

/*
 *  menustate()
 *
 *    Turn off rendering when the menu is visible.  This
 *    eases the use of the menus if something complicated
 *    is being rendered.
 */
void menustate(int state)
{
  if(!Frozen)
    state == GLUT_MENU_IN_USE ? glutIdleFunc(NULL) : glutIdleFunc(idle);
}

/*
 *  visibility()
 *
 *    When the window isn't visible, turn off rendering
 */
void visiblity(int state)
{
  if(!Frozen)
    state == GLUT_VISIBLE ? glutIdleFunc(idle) : glutIdleFunc(NULL);
}

/*
 *  menu()
 *
 *    Send all menu requests to the keyboard() or
 *    special() key press callbacks.  If the value
 *    is negative, it is a special key, if positive
 *    a normal ascii key.
 */
void menu(int value)
{
  if(value < 0)
    special(-value, 0, 0);
  else
    keyboard((char)value, 0, 0);
}

/*
 *  mouse()
 *
 *
 */
void mouse(int button, int state, int x, int y)
{
  int old_button = 0;

  tbMouse(button, state, x, y);

  if(state == GLUT_DOWN)
    {
      if(button == GLUT_LEFT)
	{
	  Grabbing = GL_TRUE;
	  selection(x, y);
	  Old_Mouse_X = Mouse_X;
	  Old_Mouse_Y = Mouse_Y;
	}
      else
	Grabbing = GL_FALSE;
    }
  else if(state == GLUT_UP)
    {
      if(button == GLUT_LEFT)
	{
	  /* check to be sure that they didn't let go on the same
	   * tile
	   */
	  Move_Mouse_X = Mouse_X;
	  Move_Mouse_Y = Mouse_Y;
	}
      Grabbing = GL_FALSE;
      Start_Moving = GL_TRUE;
    }
}

/*
 *  motion()
 *
 *
 */
void motion(int x, int y)
{
  tbMotion(x, y);

  if(Grabbing)
    {
      selection(x, y);
      glutPostRedisplay();
    }
}

/*
 *  main()
 */
void main(int argc, char **argv)
{
  glutInitWindowSize(Win_Width, Win_Height);
  glutInit(&argc, argv);

  parse(argc, argv);
    
  glutInitDisplayMode(GLUT_RGB | Buffer | GLUT_DEPTH);
  glutCreateWindow("Chess");

  init();

  glutReshapeFunc(reshape);
  glutDisplayFunc(display);
  glutIdleFunc(idle);
  glutKeyboardFunc(keyboard);
  glutSpecialFunc(special);
  glutVisibilityFunc(visiblity);
  glutMenuStateFunc(menustate);
  glutMotionFunc(motion);
  glutMouseFunc(mouse);
  glutCreateMenu(menu);
  glutAddMenuEntry("             Chess              ", '\0');
  glutAddMenuEntry("                                 ", '\0');
  glutAddMenuEntry("[X]   Tilt up                    ", 'X');
  glutAddMenuEntry("[x]   Tilt down                  ", 'x');
  glutAddMenuEntry("[Z]   Zoom in                    ", 'Z');
  glutAddMenuEntry("[z]   Zoom out                   ", 'z');
  glutAddMenuEntry("[F12] Toggle freeze-frame        ", -GLUT_KEY_F12);
  glutAddMenuEntry("                                 ", '\0');
  glutAddMenuEntry("[Esc] Quit                       ", 27);
  glutAttachMenu(GLUT_RIGHT_BUTTON);

  glutMainLoop();
}
