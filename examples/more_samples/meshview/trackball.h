/*
 *  Simple trackball-like motion ripped off from projtex.c (written by
 *  David Yu and David Blythe).  See the GLUT sample programs (advanced).
 *
 *  Copyright (C) 1997 Nate Robins (ndr@pobox.com)
 *
 *  email: ndr@pobox.com
 *  www: http://www.pobox.com/~ndr
 *
 *  This program is freely distributable without licensing fees and is
 *  provided without guarantee or warrantee expressed or implied. This
 *  program is not in the public domain.
 *
 *
 *  Usage:
 *  
 *  o  call tbInit() in before any other tb call
 *  o  call tbReshape() from the reshape callback
 *  o  call tbMatrix() to get the trackball matrix rotation
 *  o  call tbStartMotion() to begin trackball movememt
 *  o  call tbStopMotion() to stop trackball movememt
 *  o  call tbMotion() from the motion callback
 *
 *  Typical setup:
 *
 *
    void
    init(void)
    {
      tbInit(GLUT_MIDDLE_BUTTON);
      . . .
    }

    void
    reshape(int width, int height)
    {
      tbReshape(width, height);
      . . .
    }

    void
    display(void)
    {
      glPushMatrix();

      tbMatrix();
      . . . draw the scene . . .

      glPopMatrix();
    }

    void
    mouse(int button, int state, int x, int y)
    {
      tbMouse(button, state, x, y);
      . . .
    }

    void
    motion(int x, int y)
    {
      tbMotion(x, y);
      . . .
    }

    int
    main(int argc, char** argv)
    {
      . . .
      init();
      glutReshapeFunc(reshape);
      glutDisplayFunc(display);
      glutMouseFunc(mouse);
      glutMotionFunc(motion);
      . . .
    }
 *
 *
 */


/* functions */
void
tbInit(GLuint button);

void
tbMatrix();

void
tbReshape(int width, int height);

void
tbMouse(int button, int state, int x, int y);

void
tbMotion(int x, int y);
