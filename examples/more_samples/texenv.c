/*
** Demonstrates texture environment modes and internal image formats.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>

#if !defined(max)
#define max(a,b)	((a) >= (b) ? (a) : (b))
#endif
#if !defined(min)
#define min(a,b)	((a) <= (b) ? (a) : (b))
#endif

char *className = "OpenGL";
char *windowName = "Texture Env Test";
int winX = 0, winY = 0;
int winWidth = 580, winHeight = 720;

HDC hDC;
HGLRC hGLRC;
HPALETTE hPalette;
HFONT hFont;

GLfloat lightCheck[4] = { 0.7F, 0.7F, 0.7F, 1.0F };
GLfloat darkCheck[4] = { 0.3F, 0.3F, 0.3F, 1.0F };

GLfloat labelColor0[4] = { 1.0F, 1.0F, 1.0F, 1.0F };
GLfloat labelColor1[4] = { 1.0F, 1.0F, 0.4F, 1.0F };
GLfloat *labelInfoColor = labelColor0;
GLfloat labelLevelColor0[4] = { 0.8F, 0.8F, 0.1F, 1.0F };
GLfloat labelLevelColor1[4] = { 0.0F, 0.0F, 0.0F, 1.0F };

GLboolean highlightFormat = GL_FALSE;
GLboolean colorIndexMode = GL_FALSE;
GLboolean doubleBuffered = GL_FALSE;
GLboolean drawBackground = GL_FALSE;
GLboolean drawBlended = GL_TRUE;
GLboolean drawDithered = GL_TRUE;
GLboolean drawSmooth = GL_FALSE;
GLboolean drawTextured = GL_TRUE;
GLboolean displayLevelInfo = GL_FALSE;
GLboolean filterNearest = GL_TRUE;
GLboolean wrapRepeat = GL_TRUE;
GLboolean perspectiveFastest = GL_TRUE;

int textureWidth = 64;
int textureHeight = 64;

struct formatInfo {
    GLenum baseFormat;
    GLenum internalFormat;
    char *name;
};

#define NUM_LUMINANCE_FORMATS \
	    (sizeof(luminanceFormats) / sizeof(luminanceFormats[0]))
struct formatInfo luminanceFormats[] = {
    { GL_LUMINANCE, 1, "LUMINANCE" },
    { GL_LUMINANCE, GL_LUMINANCE4, "LUMINANCE4" },
    { GL_LUMINANCE, GL_LUMINANCE8, "LUMINANCE8" },
    { GL_LUMINANCE, GL_LUMINANCE12, "LUMINANCE12" },
    { GL_LUMINANCE, GL_LUMINANCE16, "LUMINANCE16" },
};

#define NUM_ALPHA_FORMATS \
	    (sizeof(alphaFormats) / sizeof(alphaFormats[0]))
struct formatInfo alphaFormats[] = {
    { GL_ALPHA, GL_ALPHA, "ALPHA" },
    { GL_ALPHA, GL_ALPHA4, "ALPHA4" },
    { GL_ALPHA, GL_ALPHA8, "ALPHA8" },
    { GL_ALPHA, GL_ALPHA12, "ALPHA12" },
    { GL_ALPHA, GL_ALPHA16, "ALPHA16" },
};

#define NUM_INTENSITY_FORMATS \
	    (sizeof(intensityFormats) / sizeof(intensityFormats[0]))
struct formatInfo intensityFormats[] = {
    { GL_INTENSITY, GL_INTENSITY, "INTENSITY" },
    { GL_INTENSITY, GL_INTENSITY4, "INTENSITY4" },
    { GL_INTENSITY, GL_INTENSITY8, "INTENSITY8" },
    { GL_INTENSITY, GL_INTENSITY12, "INTENSITY12" },
    { GL_INTENSITY, GL_INTENSITY16, "INTENSITY16" },
};

#define NUM_LUMINANCE_ALPHA_FORMATS \
	    (sizeof(luminanceAlphaFormats) / sizeof(luminanceAlphaFormats[0]))
struct formatInfo luminanceAlphaFormats[] = {
    { GL_LUMINANCE_ALPHA, 2, "LUMINANCE_ALPHA" },
    { GL_LUMINANCE_ALPHA, GL_LUMINANCE4_ALPHA4, "LUMINANCE4_ALPHA4" },
    { GL_LUMINANCE_ALPHA, GL_LUMINANCE6_ALPHA2, "LUMINANCE6_ALPHA2" },
    { GL_LUMINANCE_ALPHA, GL_LUMINANCE8_ALPHA8, "LUMINANCE8_ALPHA8" },
    { GL_LUMINANCE_ALPHA, GL_LUMINANCE12_ALPHA4, "LUMINANCE12_ALPHA4" },
    { GL_LUMINANCE_ALPHA, GL_LUMINANCE12_ALPHA12, "LUMINANCE12_ALPHA12" },
    { GL_LUMINANCE_ALPHA, GL_LUMINANCE16_ALPHA16, "LUMINANCE16_ALPHA16" },
};

#define NUM_RGB_FORMATS \
	    (sizeof(rgbFormats) / sizeof(rgbFormats[0]))
struct formatInfo rgbFormats[] = {
    { GL_RGB, 3, "RGB" },
    { GL_RGB, GL_R3_G3_B2, "R3_G3_B2" },
    { GL_RGB, GL_RGB4, "RGB4" },
    { GL_RGB, GL_RGB5, "RGB5" },
    { GL_RGB, GL_RGB8, "RGB8" },
    { GL_RGB, GL_RGB10, "RGB10" },
    { GL_RGB, GL_RGB12, "RGB12" },
    { GL_RGB, GL_RGB16, "RGB16" },
};

#define NUM_RGBA_FORMATS \
	    (sizeof(rgbaFormats) / sizeof(rgbaFormats[0]))
struct formatInfo rgbaFormats[] = {
    { GL_RGBA, 4, "RGBA" },
    { GL_RGBA, GL_RGBA2, "RGBA2" },
    { GL_RGBA, GL_RGBA4, "RGBA4" },
    { GL_RGBA, GL_RGBA8, "RGBA8" },
    { GL_RGBA, GL_RGBA12, "RGBA12" },
    { GL_RGBA, GL_RGBA16, "RGBA16" },
    { GL_RGBA, GL_RGB5_A1, "RGB5_A1" },
    { GL_RGBA, GL_RGB10_A2, "RGB10_A2" },
};

struct baseFormatInfo {
    struct formatInfo *format;
    int current, number;
};

#define NUM_BASE_FORMATS \
	    (sizeof(baseFormats) / sizeof(baseFormats[0]))
int baseFormat;
struct baseFormatInfo baseFormats[] = {
    { luminanceFormats, 0, NUM_LUMINANCE_FORMATS },
    { alphaFormats, 0, NUM_ALPHA_FORMATS },
    { intensityFormats, 0, NUM_INTENSITY_FORMATS },
    { luminanceAlphaFormats, 0, NUM_LUMINANCE_ALPHA_FORMATS },
    { rgbFormats, 0, NUM_RGB_FORMATS },
    { rgbaFormats, 0, NUM_RGBA_FORMATS },
};

#define NUM_ENV_COLORS \
	    (sizeof(envColors) / sizeof(envColors[0]))
int envColor;
GLfloat envColors[][4] = {
    { 0.0F, 0.0F, 0.0F, 1.0F },
    { 1.0F, 0.0F, 0.0F, 1.0F },
    { 0.0F, 1.0F, 0.0F, 1.0F },
    { 0.0F, 0.0F, 1.0F, 1.0F },
    { 1.0F, 1.0F, 1.0F, 1.0F },
};

struct envModeInfo {
    GLenum mode;
    char *name;
};

#define NUM_ENV_MODES \
	    (sizeof(envModes) / sizeof(envModes[0]))
struct envModeInfo envModes[] = {
    { GL_REPLACE, "REPLACE" },
    { GL_MODULATE, "MODULATE" },
    { GL_BLEND, "BLEND" },
    { GL_DECAL, "DECAL" },
};

/* struct used to manage color ramps */
struct colorIndexState {
    GLfloat amb[3];	/* ambient color / bottom of ramp */
    GLfloat diff[3];	/* diffuse color / middle of ramp */
    GLfloat spec[3];	/* specular color / top of ramp */
    GLfloat ratio;	/* ratio of diffuse to specular in ramp */
    GLint indexes[3];	/* where ramp was placed in palette */
};

#define NUM_COLORS (sizeof(colors) / sizeof(colors[0]))
struct colorIndexState colors[] = {
    {
        { 0.0F, 0.0F, 0.0F },
        { 0.2F, 0.6F, 0.4F },
        { 1.0F, 1.0F, 1.0F },
        0.75F, { 0, 0, 0 },
    },
    {
        { 0.0F, 0.05F, 0.05F },
        { 0.9F, 0.0F, 1.0F },
        { 1.0F, 1.0F, 1.0F },
        0.75F, { 0, 0, 0 },
    },
    /*
    {
        { 0.0F, 0.0F, 0.0F },
        { 1.0F, 0.9F, 0.1F },
        { 1.0F, 1.0F, 1.0F },
        0.75F, { 0, 0, 0 },
    },
    {
        { 0.0F, 0.0F, 0.0F },
        { 0.1F, 1.0F, 0.9F },
        { 1.0F, 1.0F, 1.0F },
        0.75F, { 0, 0, 0 },
    },
    */
};

void
checkErrors(void)
{
    GLenum error;

    while ((error = glGetError()) != GL_NO_ERROR) {
	fprintf(stderr, "Error: %s\n", (char *) gluErrorString(error));
    }
}

static void
initializeFont(HDC hDC)
{
#if 0
#define FONTNAME "-*-fixed-medium-r-normal--10-*-*-*-c-60-iso8859-1"
#define FONTBASE 0x1000

    XFontStruct *fontStruct;
    int firstRow, lastRow, rows;

    if ((fontStruct = XLoadQueryFont(dpy, FONTNAME)) == NULL) {
	fprintf(stderr, "font %s not found\n", FONTNAME);
	exit(EXIT_FAILURE);
    }
    firstRow = fontStruct->min_byte1;
    lastRow = fontStruct->max_byte1;
    rows = lastRow - firstRow + 1;
    
    glXUseXFont(fontStruct->fid, firstRow<<8, rows*256, FONTBASE);
    glListBase(FONTBASE);
#else
#define FONTBASE 0x1000
    /*
    hFont = GetStockObject(SYSTEM_FONT);
    hFont = CreateFont(h, w, esc, orient, weight,
		ital, under, strike, set, out, clip, qual, pitch/fam, face);
    */
    hFont = CreateFont(12, 0, 0, 0, FW_NORMAL,
		FALSE, FALSE, FALSE, ANSI_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DRAFT_QUALITY,
		FIXED_PITCH | FF_MODERN, "Arial");
    if (!hFont) {
	MessageBox(WindowFromDC(hDC),
		"Failed to find acceptable bitmap font.",
		"OpenGL Application Error",
		MB_ICONERROR | MB_OK);
	exit(1);
    }
    (void) SelectObject(hDC, hFont);
    wglUseFontBitmaps(hDC, 0, 255, FONTBASE);
    glListBase(FONTBASE);
#endif
}

static void
drawString(char *string, GLfloat x, GLfloat y, GLfloat color[4])
{
    glColor4fv(color);
    glRasterPos2f(x, y);
    glCallLists(strlen(string), GL_BYTE, (GLbyte *) string);
}

static void
drawStringOutline(char *string, GLfloat x, GLfloat y,
		  GLfloat color[4], GLfloat outline[4])
{
    drawString(string, x-1.0F, y, outline);
    drawString(string, x+1.0F, y, outline);
    drawString(string, x, y-1.0F, outline);
    drawString(string, x, y+1.0F, outline);
    drawString(string, x, y, color);
}

static void
begin2D(int width, int height)
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0F, (GLfloat) width, 0.0F, (GLfloat) height, -1.0F, 1.0F);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
}

static void
end2D(void)
{
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

static void
initialize(HDC hDC)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.5F, 1.5F, -1.5F, 1.5F, -1.5F, 1.5F);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glShadeModel(GL_FLAT);
}

static void
loadTexture(int width, int height, struct formatInfo *format)
{
    int luminanceSize = 0;
    int alphaSize = 0;
    int rgbSize = 0;
    GLenum textureFormat;
    GLubyte *texImage, *p;
    int elementsPerGroup, elementSize, groupSize, rowSize;
    int i, j;

    switch (format->baseFormat) {
    case GL_LUMINANCE:
    case GL_INTENSITY:
	luminanceSize = 1;
	textureFormat = GL_LUMINANCE;
	break;
    case GL_ALPHA:
	alphaSize = 1;
	textureFormat = GL_ALPHA;
	break;
    case GL_LUMINANCE_ALPHA:
	luminanceSize = 1;
	alphaSize = 1;
	textureFormat = GL_LUMINANCE_ALPHA;
	break;
    case GL_RGB:
	rgbSize = 3;
	textureFormat = GL_RGB;
	break;
    case GL_RGBA:
	rgbSize = 3;
	alphaSize = 1;
	textureFormat = GL_RGBA;
	break;
    default:
	fprintf(stderr, "bad internal format info\n");
	return;
    }

    elementsPerGroup = luminanceSize + alphaSize + rgbSize;
    elementSize = sizeof(GLubyte);
    groupSize = elementsPerGroup * elementSize;
    rowSize = width * groupSize;

    if ((texImage = (GLubyte *) malloc(height * rowSize)) == NULL) {
	fprintf(stderr, "texture malloc failed\n");
	return;
    }

    for (i=0; i<height; ++i) {
	p = texImage + i*rowSize;
	for (j=0; j<width; ++j) {
	    if (luminanceSize > 0) {
		/*
		** +-----+-----+
		** |     |     |
		** |  W  | LG  |
		** |     |     |
		** +-----+-----+
		** |     |     |
		** | DG  |  B  |
		** |     |     |
		** +-----+-----+
		*/
		if (i > height/2) {
		   if (j < width/2) {
		       p[0] = 0xff;
		   } else {
		       p[0] = 0xaa;
		   }
		} else {
		   if (j < width/2) {
		       p[0] = 0x55;
		   } else {
		       p[0] = 0x00;
		   }
		}
		p += elementSize;
	    }
	    if (rgbSize > 0) {
		/*
		** +-----+-----+
		** |     |     |
		** |  R  |  G  |
		** |     |     |
		** +-----+-----+
		** |     |     |
		** |  Y  |  B  |
		** |     |     |
		** +-----+-----+
		*/
		if (i > height/2) {
		   if (j < width/2) {
		       p[0] = 0xff;
		       p[1] = 0x00;
		       p[2] = 0x00;
		   } else {
		       p[0] = 0x00;
		       p[1] = 0xff;
		       p[2] = 0x00;
		   }
		} else {
		   if (j < width/2) {
		       p[0] = 0xff;
		       p[1] = 0xff;
		       p[2] = 0x00;
		   } else {
		       p[0] = 0x00;
		       p[1] = 0x00;
		       p[2] = 0xff;
		   }
		}
		p += 3*elementSize;
	    }
	    if (alphaSize > 0) {
		/*
		** +-----------+
		** |     W     |
		** |  +-----+  |
		** |  |     |  |
		** |  |  B  |  |
		** |  |     |  |
		** |  +-----+  |
		** |           |
		** +-----------+
		*/
		int i2 = i - height/2;
		int j2 = j - width/2;
		int h8 = height/8;
		int w8 = width/8;
		if (-h8<=i2 && i2<=h8 && -w8<=j2 && j2<=w8) {
		    p[0] = 0x00;
		} else if (-2*h8<=i2 && i2<=2*h8 && -2*w8<=j2 && j2<=2*w8) {
		    p[0] = 0x55;
		} else if (-3*h8<=i2 && i2<=3*h8 && -3*w8<=j2 && j2<=3*w8) {
		    p[0] = 0xaa;
		} else {
		    p[0] = 0xff;
		}
		p += elementSize;
	    }
	}
    }

    glTexImage2D(GL_TEXTURE_2D, 0,
		 format->internalFormat, width, height, 0,
		 textureFormat, GL_UNSIGNED_BYTE, texImage);
    
    free(texImage);
}

static void
drawCheck(int w, int h, GLfloat lightCheck[4], GLfloat darkCheck[4])
{
    float dw = 2.0F / w;
    float dh = 2.0F / h;
    int i, j;

    for (i=0; i<w; ++i) {
	GLfloat x0 = -1.0F + i*dw;
	GLfloat x1 = x0 + dw;

	glBegin(GL_QUAD_STRIP);
	for (j=0; j<=h; ++j) {
	    GLfloat y = -1.0F + j*dh;

	    if ((i ^ j) & 1) {
		glColor4fv(lightCheck);
	    } else {
		glColor4fv(darkCheck);
	    }

	    glVertex2f(x0, y);
	    glVertex2f(x1, y);
	}
	glEnd();
    }
}

static void
drawSample(int x, int y, int w, int h,
	   struct formatInfo *format, struct envModeInfo *envMode)
{
    glViewport(x, y, w, h);
    glScissor(x, y, w, h);

    glClearColor(0.1F, 0.1F, 0.1F, 1.0F);
//    glClear(GL_COLOR_BUFFER_BIT);

    begin2D(w, h);
    drawString(format->name, 10.0F, h-15.0F, labelInfoColor);
    drawString(envMode->name, 10.0F, 5.0F, labelInfoColor);
    end2D();

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, envMode->mode);
    glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, envColors[envColor]);

    if (perspectiveFastest) {
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
    } else {
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    }

    if (filterNearest) {
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    } else {
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    if (wrapRepeat) {
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    } else {
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    }

    loadTexture(textureWidth, textureHeight, format);

    if (drawBackground) {
	drawCheck(7, 7, lightCheck, darkCheck);
    }
    if (drawBlended) {
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
    }
    if (drawSmooth) {
	glShadeModel(GL_SMOOTH);
    }
    if (drawTextured) {
	glEnable(GL_TEXTURE_2D);
    }

    glBegin(GL_QUADS);
    glColor4f(1.0F, 0.0F, 0.0F, 1.0F); glTexCoord2f(0.0F, 0.0F);
    glVertex2f(-0.8F, -0.8F);
    glColor4f(0.0F, 1.0F, 0.0F, 1.0F); glTexCoord2f(1.0F, 0.0F);
    glVertex2f( 0.8F, -0.8F);
    glColor4f(0.0F, 0.0F, 1.0F, 1.0F); glTexCoord2f(1.0F, 1.0F);
    glVertex2f( 0.8F,  0.8F);
    glColor4f(1.0F, 1.0F, 1.0F, 1.0F); glTexCoord2f(0.0F, 1.0F);
    glVertex2f(-0.8F,  0.8F);
    glEnd();

    glDisable(GL_BLEND);
    glShadeModel(GL_FLAT);
    glDisable(GL_TEXTURE_2D);

    if (displayLevelInfo) {
	GLint width, height, border, components;
	GLint redSize, greenSize, blueSize, alphaSize;
	GLint luminanceSize, intensitySize;
	char buf[255];

	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0,
				 GL_TEXTURE_WIDTH, &width);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0,
				 GL_TEXTURE_HEIGHT, &height);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0,
				 GL_TEXTURE_BORDER, &border);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0,
				 GL_TEXTURE_COMPONENTS, &components);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0,
				 GL_TEXTURE_RED_SIZE, &redSize);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0,
				 GL_TEXTURE_GREEN_SIZE, &greenSize);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0,
				 GL_TEXTURE_BLUE_SIZE, &blueSize);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0,
				 GL_TEXTURE_ALPHA_SIZE, &alphaSize);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0,
				 GL_TEXTURE_LUMINANCE_SIZE, &luminanceSize);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0,
				 GL_TEXTURE_INTENSITY_SIZE, &intensitySize);

	begin2D(w, h);
	sprintf(buf, "dimensions: %d x %d", width, height);
	drawStringOutline(buf, 15.0F, h/2.0F+20.0F, labelLevelColor0, labelLevelColor1);

	sprintf(buf, "border: %d", border);
	drawStringOutline(buf, 15.0F, h/2.0F+10.0F, labelLevelColor0, labelLevelColor1);

	sprintf(buf, "components: 0x%04X", components);
	drawStringOutline(buf, 15.0F, h/2.0F, labelLevelColor0, labelLevelColor1);

	sprintf(buf, "sizes:");
	drawStringOutline(buf, 15.0F, h/2.0F-10.0F, labelLevelColor0, labelLevelColor1);

	sprintf(buf, "  %d/%d/%d/%d/%d/%d",
		redSize, greenSize, blueSize, alphaSize,
		luminanceSize, intensitySize);
	drawStringOutline(buf, 15.0F, h/2.0F-20.0F, labelLevelColor0, labelLevelColor1);
	end2D();
    }
}

static void
redraw(HDC hDC)
{
    int numX = NUM_ENV_MODES, numY = NUM_BASE_FORMATS;
    float xBase = (float) winWidth * 0.01F;
    float xOffset = (winWidth - xBase) / numX;
    float xSize = max(xOffset - xBase, 1);
    float yBase = (float) winHeight * 0.01F;
    float yOffset = (winHeight - yBase) / numY;
    float ySize = max(yOffset - yBase, 1);
    float x, y;
    int i, j;

    if (drawDithered) {
	glEnable(GL_DITHER);
    } else {
	glDisable(GL_DITHER);
    }

    glDisable(GL_SCISSOR_TEST);
    glClearColor(0.0F, 0.0F, 0.0F, 0.0F);
    glClear(GL_COLOR_BUFFER_BIT);
    //glEnable(GL_SCISSOR_TEST);

    x = xBase; y = (winHeight - 1) - yOffset;
    for (i=0; i<NUM_BASE_FORMATS; ++i) {
	struct formatInfo *format;

	if (highlightFormat && (i == baseFormat)) {
	    labelInfoColor = labelColor1;
	} else {
	    labelInfoColor = labelColor0;
	}

	format = &baseFormats[i].format[baseFormats[i].current];
	for (j=0; j<NUM_ENV_MODES; ++j) {
	    struct envModeInfo *envMode;

	    envMode = &envModes[j];
	    drawSample((GLint) x, (GLint) y,
		       (GLint) xSize, (GLint) ySize, format, envMode);
	    x += xOffset;
	}
	x = xBase; y -= yOffset;
    }

    glFlush();
    SwapBuffers(hDC);
}

void
resize(HDC hDC)
{
    glViewport(0, 0, winWidth, winHeight);
}

/*****************************************************************/

/* Main Menu Items */
#define IDM_APPLICATION_EXIT	100
#define IDM_BLEND		101
#define IDM_DITHER		102
#define IDM_BACKGROUND		104
#define IDM_SMOOTH		105
#define IDM_TEXTURE		106
#define IDM_LEVEL_INFO		107

/* Perspective Correction Menu Items */
#define IDM_PERSPECTIVE_FASTEST	200
#define IDM_PERSPECTIVE_NICEST	201

/* Filter Menu Items */
#define IDM_FILTER_NEAREST	300
#define IDM_FILTER_LINEAR	301

/* Wrap Menu Items */
#define IDM_WRAP_REPEAT		400
#define IDM_WRAP_CLAMP		401

HMENU hMainPup, hPerspPup, hFilterPup, hWrapPup;

void
createPopup(void)
{
    hPerspPup = CreatePopupMenu();
    AppendMenu(hPerspPup, MF_STRING, IDM_PERSPECTIVE_FASTEST, "Fastest");
    AppendMenu(hPerspPup, MF_STRING, IDM_PERSPECTIVE_NICEST, "Nicest");

    hFilterPup = CreatePopupMenu();
    AppendMenu(hFilterPup, MF_STRING, IDM_FILTER_NEAREST, "Nearest");
    AppendMenu(hFilterPup, MF_STRING, IDM_FILTER_LINEAR, "Linear");

    hWrapPup = CreatePopupMenu();
    AppendMenu(hWrapPup, MF_STRING, IDM_WRAP_REPEAT, "Repeat");
    AppendMenu(hWrapPup, MF_STRING, IDM_WRAP_CLAMP, "Clamp");

    hMainPup = CreatePopupMenu();
    AppendMenu(hMainPup, MF_STRING, IDM_BLEND, "Blend");
    AppendMenu(hMainPup, MF_STRING, IDM_DITHER, "Dither");
    AppendMenu(hMainPup, MF_STRING, IDM_TEXTURE, "Texture");
    AppendMenu(hMainPup, MF_STRING, IDM_BACKGROUND, "Background");
    AppendMenu(hMainPup, MF_STRING, IDM_SMOOTH, "Smooth Color");
    AppendMenu(hMainPup, MF_STRING, IDM_LEVEL_INFO, "Level Info");
    AppendMenu(hMainPup, MF_SEPARATOR, 0, NULL);
    AppendMenu(hMainPup, MF_POPUP|MF_STRING, (UINT) hPerspPup, "Perspective");
    AppendMenu(hMainPup, MF_POPUP|MF_STRING, (UINT) hFilterPup, "Filter");
    AppendMenu(hMainPup, MF_POPUP|MF_STRING, (UINT) hWrapPup, "Wrap");
    AppendMenu(hMainPup, MF_SEPARATOR, 0, NULL);
    AppendMenu(hMainPup, MF_STRING, IDM_APPLICATION_EXIT, "Exit");
}

void
updatePopup(void)
{
    MENUITEMINFO miinfo;

    miinfo.cbSize = sizeof(miinfo);
    miinfo.fMask = MIIM_STATE;

    miinfo.fState = perspectiveFastest ? MFS_CHECKED : MFS_UNCHECKED;
    SetMenuItemInfo(hPerspPup, IDM_PERSPECTIVE_FASTEST, FALSE, &miinfo);
    miinfo.fState = !perspectiveFastest ? MFS_CHECKED : MFS_UNCHECKED;
    SetMenuItemInfo(hPerspPup, IDM_PERSPECTIVE_NICEST, FALSE, &miinfo);

    miinfo.fState = filterNearest ? MFS_CHECKED : MFS_UNCHECKED;
    SetMenuItemInfo(hFilterPup, IDM_FILTER_NEAREST, FALSE, &miinfo);
    miinfo.fState = !filterNearest ? MFS_CHECKED : MFS_UNCHECKED;
    SetMenuItemInfo(hFilterPup, IDM_FILTER_LINEAR, FALSE, &miinfo);

    miinfo.fState = wrapRepeat ? MFS_CHECKED : MFS_UNCHECKED;
    SetMenuItemInfo(hWrapPup, IDM_WRAP_REPEAT, FALSE, &miinfo);
    miinfo.fState = !wrapRepeat ? MFS_CHECKED : MFS_UNCHECKED;
    SetMenuItemInfo(hWrapPup, IDM_WRAP_CLAMP, FALSE, &miinfo);

    miinfo.fState = drawBlended ? MFS_CHECKED : MFS_UNCHECKED;
    SetMenuItemInfo(hMainPup, IDM_BLEND, FALSE, &miinfo);
    miinfo.fState = drawDithered ? MFS_CHECKED : MFS_UNCHECKED;
    SetMenuItemInfo(hMainPup, IDM_DITHER, FALSE, &miinfo);
    miinfo.fState = drawBackground ? MFS_CHECKED : MFS_UNCHECKED;
    SetMenuItemInfo(hMainPup, IDM_BACKGROUND, FALSE, &miinfo);
    miinfo.fState = drawSmooth ? MFS_CHECKED : MFS_UNCHECKED;
    SetMenuItemInfo(hMainPup, IDM_SMOOTH, FALSE, &miinfo);
    miinfo.fState = drawTextured ? MFS_CHECKED : MFS_UNCHECKED;
    SetMenuItemInfo(hMainPup, IDM_TEXTURE, FALSE, &miinfo);
    miinfo.fState = displayLevelInfo ? MFS_CHECKED : MFS_UNCHECKED;
    SetMenuItemInfo(hMainPup, IDM_LEVEL_INFO, FALSE, &miinfo);
}

void
setupPalette(HDC hDC)
{
    PIXELFORMATDESCRIPTOR pfd;
    LOGPALETTE* pPal;
    int pixelFormat = GetPixelFormat(hDC);
    int paletteSize;

    (void) DescribePixelFormat(hDC, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
    if (!(pfd.dwFlags & PFD_NEED_PALETTE)) {
	return;
    }

    paletteSize = 1 << pfd.cColorBits;
    pPal = (LOGPALETTE*)
	malloc(sizeof(LOGPALETTE) + paletteSize * sizeof(PALETTEENTRY));
    memset(pPal, 0, sizeof(LOGPALETTE) + paletteSize * sizeof(PALETTEENTRY));
    pPal->palVersion = 0x300;
    pPal->palNumEntries = paletteSize;

    /* start with a copy of the current system palette */
    (void) GetSystemPaletteEntries(hDC, 0, paletteSize, &pPal->palPalEntry[0]);

    if (pfd.iPixelType == PFD_TYPE_RGBA) {
	/* fill in an RGBA color palette */
	int redMask = (1 << pfd.cRedBits) - 1;
	int greenMask = (1 << pfd.cGreenBits) - 1;
	int blueMask = (1 << pfd.cBlueBits) - 1;
	int i;

	for (i=10; i<paletteSize-10; ++i) {
	    pPal->palPalEntry[i].peRed =
		    (((i >> pfd.cRedShift) & redMask) * 255) / redMask;
	    pPal->palPalEntry[i].peGreen =
		    (((i >> pfd.cGreenShift) & greenMask) * 255) / greenMask;
	    pPal->palPalEntry[i].peBlue =
		    (((i >> pfd.cBlueShift) & blueMask) * 255) / blueMask;
	    pPal->palPalEntry[i].peFlags = PC_NOCOLLAPSE;
	}
    } else {
	/* fill in a Color Index ramp color palette */
	int numRamps = NUM_COLORS;
	int rampSize = (paletteSize - 10) / numRamps;
	int i, r;

	for (r=0; r<numRamps; ++r) {
	    int rampBase = r * rampSize + 10;
	    PALETTEENTRY *pe = &pPal->palPalEntry[rampBase];
	    int diffSize = (int) (rampSize * colors[r].ratio);
	    int specSize = rampSize - diffSize;

	    for (i=0; i<rampSize; ++i) {
		GLfloat *c0, *c1;
		GLint a;

		if (i < diffSize) {
		    c0 = colors[r].amb;
		    c1 = colors[r].diff;
		    a = (i * 255) / (diffSize - 1);
		} else {
		    c0 = colors[r].diff;
		    c1 = colors[r].spec;
		    a = ((i - diffSize) * 255) / (specSize - 1);
		}

		pe[i].peRed = (BYTE) (a * (c1[0] - c0[0]) + 255 * c0[0]);
		pe[i].peGreen = (BYTE) (a * (c1[1] - c0[1]) + 255 * c0[1]);
		pe[i].peBlue = (BYTE) (a * (c1[2] - c0[2]) + 255 * c0[2]);
		pe[i].peFlags = PC_NOCOLLAPSE;
	    }

	    colors[r].indexes[0] = rampBase;
	    colors[r].indexes[1] = rampBase + (diffSize-1);
	    colors[r].indexes[2] = rampBase + (rampSize-1);
	}
    }

    hPalette = CreatePalette(pPal);
    free(pPal);

    if (hPalette) {
	SelectPalette(hDC, hPalette, FALSE);
	RealizePalette(hDC);
    }
}

void
setupPixelFormat(HDC hDC)
{
    PIXELFORMATDESCRIPTOR pfd = {
	sizeof(PIXELFORMATDESCRIPTOR),	/* size of this pfd */
	1,				/* version num */
	PFD_DRAW_TO_WINDOW |		/* support window */
	PFD_SUPPORT_OPENGL |		/* support OpenGL */
	PFD_DOUBLEBUFFER,		/* support double buffering */
	0,				/* color type (filled below) */
	8,				/* 8-bit color depth */
	0, 0, 0, 0, 0, 0,		/* color bits (ignored) */
	0,				/* no alpha buffer */
	0,				/* alpha bits (ignored) */
	0,				/* no accumulation buffer */
	0, 0, 0, 0,			/* accum bits (ignored) */
	16,				/* 16-bit depth buffer */
	0,				/* no stencil buffer */
	0,				/* no auxiliary buffers */
	PFD_MAIN_PLANE,			/* main layer */
	0,				/* reserved */
	0, 0, 0,			/* no layer, visible, damage masks */
    };
    int SelectedPixelFormat;
    BOOL retVal;

    if (0 && GetDeviceCaps(hDC, BITSPIXEL) == 8) {
	colorIndexMode = TRUE;
	pfd.iPixelType = PFD_TYPE_COLORINDEX;
    } else {
	colorIndexMode = FALSE;
	pfd.iPixelType = PFD_TYPE_RGBA;
    }

    SelectedPixelFormat = ChoosePixelFormat(hDC, &pfd);
    if (SelectedPixelFormat == 0) {
	MessageBox(WindowFromDC(hDC),
		"Failed to find acceptable pixel format.",
		"OpenGL Application Error",
		MB_ICONERROR | MB_OK);
	exit(1);
    }

    retVal = SetPixelFormat(hDC, SelectedPixelFormat, &pfd);
    if (retVal != TRUE) {
	MessageBox(WindowFromDC(hDC),
		"Failed to set pixel format.",
		"OpenGL Application Error",
		MB_ICONERROR | MB_OK);
	exit(1);
    }
}

LRESULT APIENTRY
WndProc(
    HWND hWnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (message) {
    case WM_CREATE:
	hDC = GetDC(hWnd);
	setupPixelFormat(hDC);
	setupPalette(hDC);
	hGLRC = wglCreateContext(hDC);
	if (hGLRC) {
	    wglMakeCurrent(hDC, hGLRC);
	    initializeFont(hDC);
	    initialize(hDC);
	}
	return 0;
    case WM_DESTROY:
	if (hGLRC) {
	    wglMakeCurrent(NULL, NULL);
	    wglDeleteContext(hGLRC);
	}
	ReleaseDC(hWnd, hDC);
	PostQuitMessage(0);
	return 0;
    case WM_SIZE:
	if (hGLRC) {
	    winWidth = (int) LOWORD(lParam);
	    winHeight = (int) HIWORD(lParam);
	    resize(hDC);
	    return 0;
	}
	break;
    case WM_PALETTECHANGED:
	if (hPalette != NULL && (HWND) wParam != hWnd) {
	    UnrealizeObject(hPalette);
	    SelectPalette(hDC, hPalette, FALSE);
	    RealizePalette(hDC);
	    if (hGLRC) redraw(hDC);
	    return 0;
	}
	break;
    case WM_QUERYNEWPALETTE:
	if (hPalette != NULL) {
	    UnrealizeObject(hPalette);
	    SelectPalette(hDC, hPalette, FALSE);
	    RealizePalette(hDC);
	    if (hGLRC) redraw(hDC);
	    return TRUE;
	}
	break;
    case WM_PAINT:
	if (hGLRC) {
	    PAINTSTRUCT ps;
	    HDC hPaintDC = BeginPaint(hWnd, &ps);
	    redraw(hDC);
	    EndPaint(hWnd, &ps);
	    return 0;
	}
    case WM_CHAR:
	switch ((int)wParam) {
	case 'b':
	    SendMessage(hWnd, WM_COMMAND, IDM_BLEND, 0);
	    break;
	case 'd':
	    SendMessage(hWnd, WM_COMMAND, IDM_DITHER, 0);
	    break;
	case 'g':
	    SendMessage(hWnd, WM_COMMAND, IDM_BACKGROUND, 0);
	    break;
	case 's':
	    SendMessage(hWnd, WM_COMMAND, IDM_SMOOTH, 0);
	    break;
	case 't':
	    SendMessage(hWnd, WM_COMMAND, IDM_TEXTURE, 0);
	    break;
	case 'i':
	    SendMessage(hWnd, WM_COMMAND, IDM_LEVEL_INFO, 0);
	    break;
	case 'p':
	    if (perspectiveFastest) {
		SendMessage(hWnd, WM_COMMAND, IDM_PERSPECTIVE_NICEST, 0);
	    } else {
		SendMessage(hWnd, WM_COMMAND, IDM_PERSPECTIVE_FASTEST, 0);
	    }
	    break;
	case 'f':
	    if (filterNearest) {
		SendMessage(hWnd, WM_COMMAND, IDM_FILTER_LINEAR, 0);
	    } else {
		SendMessage(hWnd, WM_COMMAND, IDM_FILTER_NEAREST, 0);
	    }
	    break;
	case 'w':
	    if (wrapRepeat) {
		SendMessage(hWnd, WM_COMMAND, IDM_WRAP_CLAMP, 0);
	    } else {
		SendMessage(hWnd, WM_COMMAND, IDM_WRAP_REPEAT, 0);
	    }
	    break;
	case 'c':
	    envColor = ++envColor % NUM_ENV_COLORS;
	    break;
	default:
	    break;
	}
	if (hGLRC) redraw(hDC);
	return 0;
    case WM_KEYDOWN:
	switch ((int)wParam) {
	case VK_ESCAPE:
	    DestroyWindow(hWnd);
	    break;
	case VK_SPACE:
	    highlightFormat = !highlightFormat;
	    break;
	case VK_DOWN:
	    if (highlightFormat) {
		if (++baseFormat > NUM_BASE_FORMATS-1) baseFormat = 0;
	    }
	    break;
	case VK_UP:
	    if (highlightFormat) {
		if (--baseFormat < 0) baseFormat = NUM_BASE_FORMATS-1;
	    }
	    break;
	case VK_LEFT:
	    if (highlightFormat) {
		--baseFormats[baseFormat].current;
		if (baseFormats[baseFormat].current < 0)
		    baseFormats[baseFormat].current =
			baseFormats[baseFormat].number-1;
	    }
	    break;
	case VK_RIGHT:
	    if (highlightFormat) {
		++baseFormats[baseFormat].current;
		if (baseFormats[baseFormat].current > baseFormats[baseFormat].number-1)
		    baseFormats[baseFormat].current = 0;
	    }
	    break;
	default:
	    break;
	}
	if (hGLRC) redraw(hDC);
	return 0;
    case WM_COMMAND:
	switch (LOWORD(wParam)) {
	case IDM_APPLICATION_EXIT:
	    DestroyWindow(hWnd);
	    break;
	case IDM_BLEND:
	    drawBlended = !drawBlended;
	    break;
	case IDM_DITHER:
	    drawDithered = !drawDithered;
	    break;
	case IDM_BACKGROUND:
	    drawBackground = !drawBackground;
	    break;
	case IDM_SMOOTH:
	    drawSmooth = !drawSmooth;
	    break;
	case IDM_TEXTURE:
	    drawTextured = !drawTextured;
	    break;
	case IDM_LEVEL_INFO:
	    displayLevelInfo = !displayLevelInfo;
	    break;
	case IDM_PERSPECTIVE_FASTEST:
	    perspectiveFastest = GL_TRUE;
	    break;
	case IDM_PERSPECTIVE_NICEST:
	    perspectiveFastest = GL_FALSE;
	    break;
	case IDM_FILTER_NEAREST:
	    filterNearest = GL_TRUE;
	    break;
	case IDM_FILTER_LINEAR:
	    filterNearest = GL_FALSE;
	    break;
	case IDM_WRAP_REPEAT:
	    wrapRepeat = GL_TRUE;
	    break;
	case IDM_WRAP_CLAMP:
	    wrapRepeat = GL_FALSE;
	    break;
	default:
	    /* unknown command -- do nothing */
	    break;
	}
	updatePopup();
	if (hGLRC) redraw(hDC);
	break;
    case WM_CONTEXTMENU:
	TrackPopupMenu(hMainPup, TPM_LEFTALIGN | TPM_RIGHTBUTTON,
		    LOWORD(lParam), HIWORD(lParam), 0, hWnd, NULL);
	break;
    default:
	break;
    }

    /* Deal with any unprocessed messages */
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

    /* Define and register the window class */
    wndClass.style = CS_HREDRAW | CS_VREDRAW;
    wndClass.lpfnWndProc = WndProc;
    wndClass.cbClsExtra = 0;
    wndClass.cbWndExtra = 0;
    wndClass.hInstance = hCurrentInst;
    wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndClass.hbrBackground = GetStockObject(WHITE_BRUSH);
    wndClass.lpszMenuName = NULL;
    wndClass.lpszClassName = className;
    RegisterClass(&wndClass);

    /* Create a window of the previously defined class */
    hWnd = CreateWindow(
	className,		/* Window class's name */
	windowName,		/* Title bar text */
	WS_OVERLAPPEDWINDOW |	/* The window's style */
	WS_CLIPCHILDREN |
	WS_CLIPSIBLINGS,
	winX, winY,		/* Position */
	winWidth, winHeight,	/* Size */
	NULL,			/* Parent window's handle */
	NULL,			/* Menu handle */
	hCurrentInst,		/* Instance handle */
	NULL);			/* No additional data */

    /* Map the window to the screen */
    ShowWindow(hWnd, nCmdShow);

    /* Force the window to repaint itself */
    UpdateWindow(hWnd);

    createPopup();
    updatePopup();

    /* Message loop */
    while (GetMessage(&msg, NULL, 0, 0) == TRUE) {
	TranslateMessage(&msg);
	DispatchMessage(&msg);
    }

    return msg.wParam;
}
