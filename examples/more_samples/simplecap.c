/*
** Simple demonstration of using OpenGL's stencil test to cap clipped objects.
**
** cc -o simplecap simplecap.c -lGL -lXext -lX11 -lm
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#if defined(WIN32)
#include <windows.h>
#include <GL/gl.h>
#else
#include <GL/gl.h>
#include <GL/glx.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#endif

static char *winName = "Simple Capping";
static int winX = 0, winY = 0;
static int winWidth = 256, winHeight = 256;

void
drawCylinder(void)
{
    const int numMajor = 8;
    const int numMinor = 32;
    const float height = 1.2F;
    const float radius = 0.6F;
    float majorStep = height / numMajor;
    float minorStep = 2.0F*3.14159F / numMinor;
    int i, j;

    for (i=0; i<numMajor; ++i) {
	GLfloat z0 = 0.5F*height - i * majorStep;
	GLfloat z1 = z0 - majorStep;

	glBegin(GL_TRIANGLE_STRIP);
	for (j=0; j<=numMinor; ++j) {
	    double a = j * minorStep;
	    GLfloat nx = (GLfloat) cos(a);
	    GLfloat ny = (GLfloat) sin(a);
	    GLfloat x = radius * nx;
	    GLfloat y = radius * ny;

	    glNormal3f(nx, ny, 0.0F);
	    glVertex3f(x, y, z0);

	    glNormal3f(nx, ny, 0.0F);
	    glVertex3f(x, y, z1);
	}
	glEnd();
    }

    glNormal3f(0.0F, 0.0F, 1.0F);
    glBegin(GL_TRIANGLE_FAN);
    for (i=0; i<numMinor; ++i) {
	float a = i * minorStep;
	GLfloat x = radius * (GLfloat) cos(a);
	GLfloat y = radius * (GLfloat) sin(a);

	glVertex3f(x, y, 0.5F*height);
    }
    glEnd();

    glNormal3f(0.0F, 0.0F, -1.0F);
    glBegin(GL_TRIANGLE_FAN);
    for (i=0; i<numMinor; ++i) {
	float a = i * minorStep;
	GLfloat x = radius * (GLfloat) cos(a);
	GLfloat y = radius * (GLfloat) sin(a);

	glVertex3f(-x, y, -0.5F*height);
    }
    glEnd();
}

void
setMaterial(void)
{
    GLfloat ambient[4] = { 0.05F, 0.05F, 0.05F, 1.0F };
    GLfloat diffuse[4] = { 0.6F, 0.0F, 0.2F, 1.0F };
    GLfloat specular[4] = { 1.0F, 1.0F, 1.0F, 1.0F };

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, diffuse);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 20.0F);
}

void
initialize(void)
{
    /* set the projection transformation */
    glMatrixMode(GL_PROJECTION);
    glFrustum(-0.5F, 0.5F, -0.5F, 0.5F, 1.0F, 3.0F);

    /* set the viewing transformation */
    glMatrixMode(GL_MODELVIEW);
    glTranslatef(0.0F, 0.0F, -2.0F);

    glClear(GL_STENCIL_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    setMaterial();
}

void
resize(void)
{
    /* set the viewport to cover the window */
    glViewport(0, 0, winWidth, winHeight);

    glClear(GL_STENCIL_BUFFER_BIT);
}

GLboolean moveObject;
GLfloat obX = 45.0F, obY = 45.0F, obZ;

GLboolean moveClipPlane;
GLfloat cpX = -45.0F, cpY = -45.0F, cpZ;

GLdouble clipPlane[4] = { 0.0, 0.0, -1.0, 0.5 };

GLfloat clipPlaneNormal[3] = {
    0.0F, 0.0F, 1.0F,
};
GLfloat clipPlaneVerts[4][3] = {
    { 10.0F, 10.0F, 0.5F },
    {-10.0F, 10.0F, 0.5F },
    {-10.0F,-10.0F, 0.5F },
    { 10.0F,-10.0F, 0.5F },
};

void
setClipPlane(void)
{
    glPushMatrix();
    glRotatef(cpX, 1.0F, 0.0F, 0.0F);
    glRotatef(cpY, 0.0F, 1.0F, 0.0F);
    glRotatef(cpZ, 0.0F, 0.0F, 1.0F);

    glClipPlane(GL_CLIP_PLANE0, clipPlane);

    glPopMatrix();
}

void
drawClipPlane(void)
{
    glPushMatrix();
    glRotatef(cpX, 1.0F, 0.0F, 0.0F);
    glRotatef(cpY, 0.0F, 1.0F, 0.0F);
    glRotatef(cpZ, 0.0F, 0.0F, 1.0F);

    glNormal3fv(clipPlaneNormal);
    glBegin(GL_QUADS);
    glVertex3fv(clipPlaneVerts[0]);
    glVertex3fv(clipPlaneVerts[1]);
    glVertex3fv(clipPlaneVerts[2]);
    glVertex3fv(clipPlaneVerts[3]);
    glEnd();

    glPopMatrix();
}

void
redraw(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /* object modeling transformations */
    glPushMatrix();
    glRotatef(obX, 1.0F, 0.0F, 0.0F);
    glRotatef(obY, 0.0F, 1.0F, 0.0F);
    glRotatef(obZ, 0.0F, 0.0F, 1.0F);

    /* position clip plane relative to object */
    setClipPlane();

    glEnable(GL_STENCIL_TEST);

    /*
    ** Use a stencil func/op which leaves a bit set in the stencil buffer
    ** wherever the inside of the object is exposed by the clipplane.
    */
    glStencilFunc(GL_ALWAYS, 1, 1);
    glStencilOp(GL_KEEP, GL_INVERT, GL_INVERT);

    glEnable(GL_CLIP_PLANE0);

    drawCylinder();

    glDisable(GL_CLIP_PLANE0);

    /*
    ** Use a stencil func/op which passes only where there is a bit set
    ** in the stencil buffer.  We also zero the bits as we go so that we
    ** don't have to explicitly clear the stencil buffer each frame.
    */
    glStencilFunc(GL_EQUAL, 1, 1);
    glStencilOp(GL_KEEP, GL_ZERO, GL_ZERO);

    drawClipPlane();

    glDisable(GL_STENCIL_TEST);
    glPopMatrix();
}

/************************************************************/

#if defined(WIN32)
/*
** Win32 Window System Specific Code
*/

static char *className = "OpenGL";
static HDC hDC;
static HGLRC hGLRC;
static HPALETTE hPalette;
static BOOL needsRedisplay = TRUE;

void
doRedisplay(void)
{
    if (needsRedisplay) {
	redraw();
	SwapBuffers(hDC);
	needsRedisplay = FALSE;
    }
}

void
redisplay(void)
{
    needsRedisplay = TRUE;
}

void
setupPixelFormat(HDC hDC)
{
    PIXELFORMATDESCRIPTOR pfd = {
	sizeof(PIXELFORMATDESCRIPTOR),	/* size */
	1,				/* version */
	PFD_SUPPORT_OPENGL |
	PFD_DRAW_TO_WINDOW |
	PFD_DOUBLEBUFFER,		/* support double-buffering */
	PFD_TYPE_RGBA,			/* color type */
	16,				/* prefered color depth */
	0, 0, 0, 0, 0, 0,		/* color bits (ignored) */
	0,				/* no alpha buffer */
	0,				/* alpha bits (ignored) */
	0,				/* no accumulation buffer */
	0, 0, 0, 0,			/* accum bits (ignored) */
	16,				/* depth buffer */
	1,				/* no stencil buffer */
	0,				/* no auxiliary buffers */
	PFD_MAIN_PLANE,			/* main layer */
	0,				/* reserved */
	0, 0, 0,			/* no layer, visible, damage masks */
    };
    int pixelFormat;

    pixelFormat = ChoosePixelFormat(hDC, &pfd);
    if (pixelFormat == 0) {
	MessageBox(WindowFromDC(hDC), "ChoosePixelFormat failed.", "Error",
		MB_ICONERROR | MB_OK);
	exit(1);
    }

    if (SetPixelFormat(hDC, pixelFormat, &pfd) != TRUE) {
	MessageBox(WindowFromDC(hDC), "SetPixelFormat failed.", "Error",
		MB_ICONERROR | MB_OK);
	exit(1);
    }
}

void
setupPalette(HDC hDC)
{
    int pixelFormat = GetPixelFormat(hDC);
    PIXELFORMATDESCRIPTOR pfd;
    LOGPALETTE* pPal;
    int paletteSize;

    DescribePixelFormat(hDC, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

    if (pfd.dwFlags & PFD_NEED_PALETTE) {
	paletteSize = 1 << pfd.cColorBits;
    } else {
	return;
    }

    pPal = (LOGPALETTE*)
	malloc(sizeof(LOGPALETTE) + paletteSize * sizeof(PALETTEENTRY));
    pPal->palVersion = 0x300;
    pPal->palNumEntries = paletteSize;

    /* build a simple RGB color palette */
    {
	int redMask = (1 << pfd.cRedBits) - 1;
	int greenMask = (1 << pfd.cGreenBits) - 1;
	int blueMask = (1 << pfd.cBlueBits) - 1;
	int i;

	for (i=0; i<paletteSize; ++i) {
	    pPal->palPalEntry[i].peRed =
		    (((i >> pfd.cRedShift) & redMask) * 255) / redMask;
	    pPal->palPalEntry[i].peGreen =
		    (((i >> pfd.cGreenShift) & greenMask) * 255) / greenMask;
	    pPal->palPalEntry[i].peBlue =
		    (((i >> pfd.cBlueShift) & blueMask) * 255) / blueMask;
	    pPal->palPalEntry[i].peFlags = 0;
	}
    }

    hPalette = CreatePalette(pPal);
    free(pPal);

    if (hPalette) {
	SelectPalette(hDC, hPalette, FALSE);
	RealizePalette(hDC);
    }
}

LRESULT APIENTRY
WndProc(
    HWND hWnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    static GLint x0, x1, y0, y1;

    switch (message) {
    case WM_CREATE:
	return 0;
    case WM_DESTROY:
	PostQuitMessage(0);
	return 0;
    case WM_SIZE:
	/* track window size changes */
	if (hGLRC) {
	    winWidth = (int) LOWORD(lParam);
	    winHeight = (int) HIWORD(lParam);
	    resize();
	    return 0;
	}
    case WM_PALETTECHANGED:
	/* realize palette if this is *not* the current window */
	if (hGLRC && hPalette && (HWND) wParam != hWnd) {
	    UnrealizeObject(hPalette);
	    SelectPalette(hDC, hPalette, FALSE);
	    RealizePalette(hDC);
	    redisplay();
	    break;
	}
	break;
    case WM_QUERYNEWPALETTE:
	/* realize palette if this is the current window */
	if (hGLRC && hPalette) {
	    UnrealizeObject(hPalette);
	    SelectPalette(hDC, hPalette, FALSE);
	    RealizePalette(hDC);
	    redisplay();
	    return TRUE;
	}
	break;
    case WM_PAINT:
	{
	    PAINTSTRUCT ps;
	    BeginPaint(hWnd, &ps);
	    if (hGLRC) {
		redisplay();
	    }
	    EndPaint(hWnd, &ps);
	    return 0;
	}
	break;
    case WM_CHAR:
	/* handle keyboard input */
	switch ((int)wParam) {
	case VK_ESCAPE:
	    DestroyWindow(hWnd);
	    return 0;
	case ' ':
	    redisplay();
	    break;
	default:
	    break;
	}
	break;
    case WM_LBUTTONDOWN:
	if (!moveObject && !moveClipPlane) {
	    SetCapture(hWnd);
	    moveObject = GL_TRUE;
	    x0 = ((int) LOWORD(lParam) << 16) >> 16;
	    y0 = ((int) HIWORD(lParam) << 16) >> 16;
	    redisplay();
	}
	break;
    case WM_RBUTTONDOWN:
	if (!moveObject && !moveClipPlane) {
	    SetCapture(hWnd);
	    moveClipPlane = GL_TRUE;
	    x0 = ((int) LOWORD(lParam) << 16) >> 16;
	    y0 = ((int) HIWORD(lParam) << 16) >> 16;
	    redisplay();
	}
	break;
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
	if (moveObject || moveClipPlane) {
	    ReleaseCapture();
	    moveObject = moveClipPlane = FALSE;
	}
	break;
    case WM_MOUSEMOVE:
	x1 = ((int) LOWORD(lParam) << 16) >> 16;
	y1 = ((int) HIWORD(lParam) << 16) >> 16;
	if (moveObject) {
	    obX += 360.0F * (y1 - y0) / winHeight;
	    obY += 360.0F * (x1 - x0) / winWidth;
	} else if (moveClipPlane) {
	    cpX += 360.0F * (y1 - y0) / winHeight;
	    cpY += 360.0F * (x1 - x0) / winWidth;
	}
	x0 = x1;
	y0 = y1;
	redisplay();
	break;
    default:
	break;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

int APIENTRY
WinMain(
    HINSTANCE hCurrentInst,
    HINSTANCE hPreviousInst,
    LPSTR lpszCmdLine,
    int nCmdShow)
{
    WNDCLASS wndClass;
    HWND hWnd;
    MSG msg;

    /* register window class */
    wndClass.style = CS_HREDRAW | CS_VREDRAW;
    wndClass.lpfnWndProc = WndProc;
    wndClass.cbClsExtra = 0;
    wndClass.cbWndExtra = 0;
    wndClass.hInstance = hCurrentInst;
    wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndClass.hbrBackground = NULL;
    wndClass.lpszMenuName = NULL;
    wndClass.lpszClassName = className;
    RegisterClass(&wndClass);

    /* create window */
    hWnd = CreateWindow(
	className, winName,
	WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
	winX, winY, winWidth, winHeight,
	NULL, NULL, hCurrentInst, NULL);

    /* display window */
    ShowWindow(hWnd, SW_SHOWNORMAL);
    UpdateWindow(hWnd);

    /* initialize OpenGL rendering */
    hDC = GetDC(hWnd);
    setupPixelFormat(hDC);
    setupPalette(hDC);
    hGLRC = wglCreateContext(hDC);
    wglMakeCurrent(hDC, hGLRC);
    initialize();

    /* process messages */
    while (1) {
	if (needsRedisplay &&
	    PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE) == FALSE)
	{
	    doRedisplay();
	}
	if (GetMessage(&msg, NULL, 0, 0) != TRUE) {
	    break;
	}
	TranslateMessage(&msg);
	DispatchMessage(&msg);
	doRedisplay();
    }

    /* finish OpenGL rendering */
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hGLRC);
    if (hPalette) {
	DeleteObject(hPalette);
    }
    ReleaseDC(hWnd, hDC);

    return msg.wParam;
}

#else
/*
** X11 Window System Specific Code
*/

static Display *dpy;
static Window win;
static GLXContext ctx;
static Bool needsRedisplay = True;

void
doRedisplay(void)
{
    if (needsRedisplay) {
	redraw();
	glFlush();
	glXSwapBuffers(dpy, win);
	needsRedisplay = False;
    }
}

void
redisplay(void)
{
    needsRedisplay = True;
}

Bool
waitForMapNotify(Display *dpy, XEvent *ev, XPointer arg)
{
    return (ev->type == MapNotify && ev->xmap.window == *((Window *) arg));
}

void
createWindow(int argc, char **argv)
{
    int visualAttr[] = {
	GLX_RGBA, GLX_RED_SIZE, 1,
	GLX_DEPTH_SIZE, 1, GLX_STENCIL_SIZE, 1, GLX_DOUBLEBUFFER, None
    };
    int scrn;
    Window root;
    XVisualInfo *vis;
    Colormap cmap;
    XSetWindowAttributes swa;
    XEvent ev;

    if ((dpy = XOpenDisplay(NULL)) == NULL) {
	fprintf(stderr, "can't open display\n");
	exit(EXIT_FAILURE);
    }
    scrn = DefaultScreen(dpy);
    root = RootWindow(dpy, scrn);

    if ((vis = glXChooseVisual(dpy, scrn, visualAttr)) == NULL) {
	fprintf(stderr, "can't find visual\n");
	exit(EXIT_FAILURE);
    }

    if ((cmap = XCreateColormap(dpy, root, vis->visual, AllocNone)) == None) {
	fprintf(stderr, "can't create colormap\n");
	exit(EXIT_FAILURE);
    }

    swa.background_pixel = None;
    swa.border_pixel = None;
    swa.colormap = cmap;
    swa.event_mask = StructureNotifyMask | ExposureMask | KeyPressMask |
			ButtonPressMask | ButtonReleaseMask | ButtonMotionMask;
    win = XCreateWindow(dpy, root, winX, winY, winWidth, winHeight, 0,
			vis->depth, InputOutput, vis->visual,
			CWBackPixel | CWBorderPixel | CWColormap | CWEventMask,
			&swa);
    
    XmbSetWMProperties(dpy, win, winName, winName, argv, argc, NULL, NULL, NULL);
    
    XMapWindow(dpy, win);
    XIfEvent(dpy, &ev, waitForMapNotify, (XPointer) &win);

    if ((ctx = glXCreateContext(dpy, vis, NULL, True)) == NULL) {
	fprintf(stderr, "can't create context\n");
	exit(EXIT_FAILURE);
    }

    if (!glXMakeCurrent(dpy, win, ctx)) {
	fprintf(stderr, "make current failed\n");
	exit(EXIT_FAILURE);
    }
}

void
processEvents(void)
{
    static GLint x0, x1, y0, y1;
    XEvent ev;

    while (1) {
	do {
	    XNextEvent(dpy, &ev);
	    switch (ev.type) {
	    case KeyPress:
		{
		    KeySym ks;

		    XLookupString(&ev.xkey, NULL, 0, &ks, NULL);
		    switch (ks) {
		    case XK_Escape:
			exit(EXIT_SUCCESS);
			break;
		    case XK_space:
			redisplay();
			break;
		    default:
			break;
		    }
		}
		break;
	    case ConfigureNotify:
		winWidth = ev.xconfigure.width;
		winHeight = ev.xconfigure.height;
		resize();
		redisplay();
		break;
	    case Expose:
		redisplay();
		break;
	    case ButtonPress:
		if (!moveObject && !moveClipPlane) {
		    if (ev.xbutton.button == Button1) {
			moveObject = GL_TRUE;
		    } else if (ev.xbutton.button == Button2) {
			moveClipPlane = GL_TRUE;
		    }
		    x0 = ev.xbutton.x;
		    y0 = ev.xbutton.y;
		    redisplay();
		}
		break;
	    case ButtonRelease:
		moveObject = moveClipPlane = False;
		break;
	    case MotionNotify:
		x1 = ev.xmotion.x;
		y1 = ev.xmotion.y;
		if (moveObject) {
		    obX += 360.0F * (y1 - y0) / winHeight;
		    obY += 360.0F * (x1 - x0) / winWidth;
		} else if (moveClipPlane) {
		    cpX += 360.0F * (y1 - y0) / winHeight;
		    cpY += 360.0F * (x1 - x0) / winWidth;
		}
		x0 = x1;
		y0 = y1;
		redisplay();
		break;
	    default:
		break;
	    }
	} while (XPending(dpy));
	doRedisplay();
    }
}

int
main(int argc, char **argv)
{
    createWindow(argc, argv);
    initialize();
    processEvents();
}
#endif
