/*
 * eye.c - Simple image-processing demo program.  Inspired by the book
 *         "Photoshop IQ", by Froebisch, Lindner, Steffen and Wondrack.
 *
 * Phil Lacroute
 * Silicon Graphics Computer Systems
 * July 1997
 *
 * To compile:
 *    cc -o eye eye.c -lglut -limage -lGLU -lGL -lXmu -lXext -lX11 -lm
 *
 * To run:
 *    eye eye512.rgb
 */

#include <stdio.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <gl/image.h>
#include <math.h>

/* configuration constants */
#define BLACK_LEVEL		2
#define WHITE_LEVEL		212
#define CONTRAST_GAMMA		0.7
#define LAPLACIAN_BLUR_PASSES	10
#define PICTOGRAM_BLUR_PASSES	18
#define PICTOGRAM_THRESH	28
#define THRESH_LUT_SIZE		256
#define CONTRAST_LUT_SIZE	256
#define ZERO_LUT_SIZE		16
#define POSTERIZE_LUT_SIZE	4
#define LAPLACIAN_FILTER_SIZE	LAPLACIAN_7x7_FILTER_SIZE
#define LAPLACIAN_FILTER	laplacian_7x7_filter
#define BRUSH_RADIUS		3

/* menu modes */
#define ORIGINAL_MODE		0
#define PICTOGRAM_MODE		1
#define POSTERIZE_MODE		2
#define OIL_PAINT_MODE		3
#define SHARPEN_MODE		4
#define HOR_EDGE_MODE		5
#define VERT_EDGE_MODE		6
#define DIAG_EDGE_MODE		7
#define LAPLACIAN_MODE		8

/* filter definitions */
#define MAX_BORDER		3
#define IMAGE_X			MAX_BORDER
#define IMAGE_Y			MAX_BORDER

#define SQUARE(x)	((x)*(x))
#define FILTER_BORDER(filter_size)	(((filter_size)-1)/2)

#define H_EDGES			0
#define V_EDGES			1
#define D_EDGES			2
#define H_EDGE_FILTER_SIZE	3
float h_edge_filter[SQUARE(H_EDGE_FILTER_SIZE)] = {
    -1, -2, -1,
     0,  0,  0,
     1,  2,  1
};

#define V_EDGE_FILTER_SIZE	3
float v_edge_filter[SQUARE(V_EDGE_FILTER_SIZE)] = {
    -1,  0,  1,
    -2,  0,  2,
    -1,  0,  1
};

#define D_EDGE_FILTER_SIZE	3
float d_edge_filter[SQUARE(D_EDGE_FILTER_SIZE)] = {
    -1, -2, 0,
    -2,  0, 2,
     0,  2, 1
};

#define GAUSSIAN_FILTER_SIZE	7
float gaussian_filter[SQUARE(GAUSSIAN_FILTER_SIZE)] = {
    .000, .000, .000, .004, .000, .000, .000,
    .000, .004, .016, .027, .016, .004, .000,
    .000, .016, .055, .086, .055, .016, .000,
    .004, .027, .086, .137, .086, .027, .004,
    .000, .016, .055, .086, .055, .016, .000,
    .000, .004, .016, .027, .016, .004, .000,
    .000, .000, .000, .004, .000, .000, .000
};

#define LAPLACIAN_3x3_FILTER_SIZE	3
float laplacian_3x3_filter[SQUARE(LAPLACIAN_3x3_FILTER_SIZE)] = {
     0.0,  -0.25,  0.0,
    -0.25,  1.0,  -0.25,
     0.0,  -0.25,  0.0
};

#define LAPLACIAN_5x5_FILTER_SIZE	5
float laplacian_5x5_filter[SQUARE(LAPLACIAN_5x5_FILTER_SIZE)] = {
    /* sigma = 2.0 */
    -0.00126, -0.05767, -0.10150, -0.05767, -0.00126,
    -0.05767,  0.00000,  0.16547,  0.00000, -0.05767,
    -0.10150,  0.16547,  0.25000,  0.16547, -0.10150,
    -0.05767,  0.00000,  0.16547,  0.00000, -0.05767,
    -0.00126, -0.05767, -0.10150, -0.05767, -0.00126
};

#define LAPLACIAN_7x7_FILTER_SIZE	7
float laplacian_7x7_filter[SQUARE(LAPLACIAN_7x7_FILTER_SIZE)] = {
    /* sigma = 4.0 */
    -0.00005, -0.00304, -0.01442, -0.02020, -0.01442, -0.00304, -0.00005,
    -0.00304, -0.02538, -0.01610,  0.00000, -0.01610, -0.02538, -0.00304,
    -0.01442, -0.01610,  0.04137,  0.05679,  0.04137, -0.01610, -0.01442,
    -0.02020,  0.00000,  0.05679,  0.06250,  0.05679,  0.00000, -0.02020,
    -0.01442, -0.01610,  0.04137,  0.05679,  0.04137, -0.01610, -0.01442,
    -0.00304, -0.02538, -0.01610,  0.00000, -0.01610, -0.02538, -0.00304,
    -0.00005, -0.00304, -0.01442, -0.02020, -0.01442, -0.00304, -0.00005
};

#define ZERO_FILTER_SIZE	3
float zero_filter[SQUARE(ZERO_FILTER_SIZE)] = {
    0.0, 0.2, 0.0,
    0.2, 0.2, 0.2,
    0.0, 0.2, 0.0
};

#define SHARPEN_FILTER_SIZE	3
float sharpen_filter[SQUARE(SHARPEN_FILTER_SIZE)] = {
    -1, -2, -1,
    -2, 13, -2,
    -1, -2, -1
};

/* global variables */
char *program;
unsigned char *image;
int image_w, image_h;
int mode = ORIGINAL_MODE;

/* load an image from a file */
void
loadimage(char *filename)
{
    IMAGE *im;
    short *row, *row_ptr;
    int x, y;
    unsigned char *image_ptr;
    int c;
    unsigned image_size;

    if ((im = iopen(filename, "r")) == NULL) {
	fprintf(stderr, "%s: could not open image file %s\n",
		program, filename);
	exit(1);
    }
    if (im->zsize != 1) {
	fprintf(stderr, "%s: cannot handle color images\n", program);
	exit(1);
    }
    image_w = im->xsize;
    image_h = im->ysize;

    image = (unsigned char *)malloc(image_w * image_h);
    row = (short *)malloc(image_w * sizeof(short));

    image_ptr = image;
    for (y = 0; y < image_h; y++) {
	getrow(im, row, y, 0);
	row_ptr = row;
	for (x = 0; x < image_w; x++)
	    *image_ptr++ = *row_ptr++;
    }
    free(row);
    iclose(im);
}

/* copy edges of image into the border around the image */
void
expand_image(int border)
{
    /* fill the left border */
    glRasterPos2i(IMAGE_X, IMAGE_Y);
    glPixelZoom(-border, 1);
    glCopyPixels(IMAGE_X, IMAGE_Y, 1, image_h, GL_COLOR);

    /* fill the right border */
    glRasterPos2i(IMAGE_X + image_w, IMAGE_Y);
    glPixelZoom(border, 1);
    glCopyPixels(IMAGE_X + image_w-1, IMAGE_Y, 1, image_h, GL_COLOR);

    /* fill the bottom border */
    glRasterPos2i(IMAGE_X - border, IMAGE_Y);
    glPixelZoom(1, -border);
    glCopyPixels(IMAGE_X - border, IMAGE_Y, image_w + 2*border, 1, GL_COLOR);

    /* fill the top border */
    glRasterPos2i(IMAGE_X - border, IMAGE_Y + image_h);
    glPixelZoom(1, border);
    glCopyPixels(IMAGE_X - border, IMAGE_Y + image_h-1,
		 image_w + 2*border, 1, GL_COLOR);

    glPixelZoom(1, 1);
    glRasterPos2f(IMAGE_X, IMAGE_Y);
}

/* draw a black border around the image (to cover up junk pixels in
   the border after a convolution) */
void
draw_border(void)
{
    glColor3f(0, 0, 0);
    glBegin(GL_QUADS);
        glVertex2f(0, 0);
	glVertex2f(image_w + 2*MAX_BORDER, 0);
	glVertex2f(image_w + 2*MAX_BORDER, MAX_BORDER);
	glVertex2f(0, MAX_BORDER);

	glVertex2f(image_w + MAX_BORDER, MAX_BORDER);
	glVertex2f(image_w + 2*MAX_BORDER, MAX_BORDER);
	glVertex2f(image_w + 2*MAX_BORDER, image_h + MAX_BORDER);
	glVertex2f(image_w + MAX_BORDER, image_h + MAX_BORDER);

	glVertex2f(0, image_h + MAX_BORDER);
	glVertex2f(image_w + 2*MAX_BORDER, image_h + MAX_BORDER);
	glVertex2f(image_w + 2*MAX_BORDER, image_h + 2*MAX_BORDER);
	glVertex2f(0, image_h + 2*MAX_BORDER);

	glVertex2f(0, MAX_BORDER);
	glVertex2f(MAX_BORDER, MAX_BORDER);
	glVertex2f(MAX_BORDER, image_h + MAX_BORDER);
	glVertex2f(0, image_h + MAX_BORDER);
    glEnd();
}

/* set the post-convolution bias */
void
convolution_bias(float bias)
{
    glPixelTransferf(GL_POST_CONVOLUTION_RED_BIAS_EXT, bias);
    glPixelTransferf(GL_POST_CONVOLUTION_GREEN_BIAS_EXT, bias);
    glPixelTransferf(GL_POST_CONVOLUTION_BLUE_BIAS_EXT, bias);
    glPixelTransferf(GL_POST_CONVOLUTION_ALPHA_BIAS_EXT, bias);
}

/* compute a pictogram */
void
pictogram(void)
{
    int border, i;
    float thresh_lut[THRESH_LUT_SIZE];

    border = FILTER_BORDER(GAUSSIAN_FILTER_SIZE);
    glConvolutionFilter2DEXT(GL_CONVOLUTION_2D_EXT, GL_LUMINANCE,
			     GAUSSIAN_FILTER_SIZE, GAUSSIAN_FILTER_SIZE,
			     GL_LUMINANCE, GL_FLOAT, gaussian_filter);
    for (i = 0; i < THRESH_LUT_SIZE; i++) {
	if (i < PICTOGRAM_THRESH) {
	    thresh_lut[i] = 0;
	} else {
	    thresh_lut[i] = 1;
	}
    }
    glColorTableSGI(GL_POST_CONVOLUTION_COLOR_TABLE_SGI, GL_LUMINANCE,
		    THRESH_LUT_SIZE, GL_LUMINANCE, GL_FLOAT, thresh_lut);
    for (i = 0; i < PICTOGRAM_BLUR_PASSES; i++) {
	expand_image(border);
	glEnable(GL_CONVOLUTION_2D_EXT);
	if (i == PICTOGRAM_BLUR_PASSES-1) {
	    glEnable(GL_POST_CONVOLUTION_COLOR_TABLE_SGI);
	}
	glCopyPixels(IMAGE_X-border, IMAGE_Y-border,
		     image_w + GAUSSIAN_FILTER_SIZE-1,
		     image_h + GAUSSIAN_FILTER_SIZE-1, GL_COLOR);
	glDisable(GL_CONVOLUTION_2D_EXT);
    }
    glDisable(GL_POST_CONVOLUTION_COLOR_TABLE_SGI);
}

/* posterize the image (reduce the number of gray levels) */
void
posterize(void)
{
    float lut[POSTERIZE_LUT_SIZE];
    int i;

    for (i = 0; i < POSTERIZE_LUT_SIZE; i++) {
	lut[i] = (double)i / (double)POSTERIZE_LUT_SIZE;
    }
    glColorTableSGI(GL_COLOR_TABLE_SGI, GL_LUMINANCE,
		    POSTERIZE_LUT_SIZE, GL_LUMINANCE, GL_FLOAT, lut);
    glEnable(GL_COLOR_TABLE_SGI);
    glCopyPixels(IMAGE_X, IMAGE_Y, image_w, image_h, GL_COLOR);
    glDisable(GL_COLOR_TABLE_SGI);
}

/* copy the image from texture memory to the framebuffer */
void
drawtexture(int delta_x, int delta_y)
{
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0);
        glVertex2f(IMAGE_X + delta_x, IMAGE_Y + delta_y);
        glTexCoord2f(1, 0);
	glVertex2f(IMAGE_X + delta_x + image_w, IMAGE_Y + delta_y);
        glTexCoord2f(1, 1);
	glVertex2f(IMAGE_X + delta_x + image_w, IMAGE_Y + delta_y + image_h);
        glTexCoord2f(0, 1);
	glVertex2f(IMAGE_X + delta_x, IMAGE_Y + delta_y + image_h);
    glEnd();
}

/* simulate an oil painting */
void
oil_painting(void)
{
    int x, y;

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, 1, image_w, image_h, 0,
		 GL_LUMINANCE, GL_UNSIGNED_BYTE, image);
    glEnable(GL_TEXTURE_2D);
    glColor4f(1, 1, 1, 1);
    drawtexture(0, 0);
    glBlendEquationEXT(GL_MAX_EXT);
    glEnable(GL_BLEND);
    for (y = -BRUSH_RADIUS; y <= BRUSH_RADIUS; y++) {
	for (x = -BRUSH_RADIUS; x <= BRUSH_RADIUS; x++) {
	    if (sqrt(x*x + y*y) <= BRUSH_RADIUS) {
		drawtexture(x, y);
	    }
	}
    }
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    draw_border();
}

/* sharpen the image */
void
sharpen(void)
{
    int border;

    border = FILTER_BORDER(SHARPEN_FILTER_SIZE);
    expand_image(border);
    glConvolutionFilter2DEXT(GL_CONVOLUTION_2D_EXT, GL_LUMINANCE,
			     SHARPEN_FILTER_SIZE, SHARPEN_FILTER_SIZE,
			     GL_LUMINANCE, GL_FLOAT, sharpen_filter);
    glEnable(GL_CONVOLUTION_2D_EXT);
    glCopyPixels(IMAGE_X-border, IMAGE_Y-border,
		 image_w + SHARPEN_FILTER_SIZE-1,
		 image_h + SHARPEN_FILTER_SIZE-1, GL_COLOR);
    glDisable(GL_CONVOLUTION_2D_EXT);
    draw_border();
}

/* detect edges using a gradient filter (1st spatial derivative) */
void
gradient_edge_detect(int direction)
{
    int filter_size, border;
    float *filter;

    if (direction == H_EDGES) {
	filter_size = H_EDGE_FILTER_SIZE;
	filter = h_edge_filter;
    } else if (direction == V_EDGES) {
	filter_size = V_EDGE_FILTER_SIZE;
	filter = v_edge_filter;
    } else {
	filter_size = D_EDGE_FILTER_SIZE;
	filter = d_edge_filter;
    }
    border = FILTER_BORDER(filter_size);
    expand_image(border);
    glConvolutionFilter2DEXT(GL_CONVOLUTION_2D_EXT, GL_LUMINANCE,
			     filter_size, filter_size,
			     GL_LUMINANCE, GL_FLOAT, filter);
    glEnable(GL_CONVOLUTION_2D_EXT);
    convolution_bias(0.5);
    glCopyPixels(IMAGE_X-border, IMAGE_Y-border,
		 image_w + filter_size-1,
		 image_h + filter_size-1, GL_COLOR);
    convolution_bias(0);
    glDisable(GL_CONVOLUTION_2D_EXT);
    draw_border();
}

/* detect edges using a laplacian filter (2nd spatial derivative) */
void
laplacian_edge_detect(void)
{
    int border, i;
    float binary_lut[2], zero_lut[ZERO_LUT_SIZE];

    /* blur */
    border = FILTER_BORDER(GAUSSIAN_FILTER_SIZE);
    glConvolutionFilter2DEXT(GL_CONVOLUTION_2D_EXT, GL_LUMINANCE,
			     GAUSSIAN_FILTER_SIZE, GAUSSIAN_FILTER_SIZE,
			     GL_LUMINANCE, GL_FLOAT, gaussian_filter);
    for (i = 0; i < LAPLACIAN_BLUR_PASSES; i++) {
	expand_image(border);
	glEnable(GL_CONVOLUTION_2D_EXT);
	glCopyPixels(IMAGE_X-border, IMAGE_Y-border,
		     image_w + GAUSSIAN_FILTER_SIZE-1,
		     image_h + GAUSSIAN_FILTER_SIZE-1, GL_COLOR);
	glDisable(GL_CONVOLUTION_2D_EXT);
    }

    /* laplacian with threshold */
    border = FILTER_BORDER(LAPLACIAN_FILTER_SIZE);
    expand_image(border);
    glConvolutionFilter2DEXT(GL_CONVOLUTION_2D_EXT, GL_LUMINANCE,
			     LAPLACIAN_FILTER_SIZE, LAPLACIAN_FILTER_SIZE,
			     GL_LUMINANCE, GL_FLOAT, LAPLACIAN_FILTER);
    glEnable(GL_CONVOLUTION_2D_EXT);
    convolution_bias(0.5);
    binary_lut[0] = 0;
    binary_lut[1] = 1;
    glColorTableSGI(GL_POST_CONVOLUTION_COLOR_TABLE_SGI,
		    GL_LUMINANCE, 2, GL_LUMINANCE, GL_FLOAT, binary_lut);
    glEnable(GL_POST_CONVOLUTION_COLOR_TABLE_SGI);
    glCopyPixels(IMAGE_X-border, IMAGE_Y-border,
		 image_w + LAPLACIAN_FILTER_SIZE-1,
		 image_h + LAPLACIAN_FILTER_SIZE-1, GL_COLOR);
    glDisable(GL_POST_CONVOLUTION_COLOR_TABLE_SGI);
    convolution_bias(0);
    glDisable(GL_CONVOLUTION_2D_EXT);

    /* zero crossing detector */
    border = FILTER_BORDER(ZERO_FILTER_SIZE);
    expand_image(border);
    glConvolutionFilter2DEXT(GL_CONVOLUTION_2D_EXT, GL_LUMINANCE,
			     ZERO_FILTER_SIZE, ZERO_FILTER_SIZE,
			     GL_LUMINANCE, GL_FLOAT, zero_filter);
    glEnable(GL_CONVOLUTION_2D_EXT);
    zero_lut[0] = 0;
    for (i = 1; i < ZERO_LUT_SIZE; i++) {
	zero_lut[i] = 1;
    }
    zero_lut[ZERO_LUT_SIZE-1] = 0;
    glColorTableSGI(GL_POST_CONVOLUTION_COLOR_TABLE_SGI, GL_LUMINANCE,
		    ZERO_LUT_SIZE, GL_LUMINANCE, GL_FLOAT, zero_lut);
    glEnable(GL_POST_CONVOLUTION_COLOR_TABLE_SGI);
    glCopyPixels(IMAGE_X-border, IMAGE_Y-border,
		 image_w + ZERO_FILTER_SIZE-1,
		 image_h + ZERO_FILTER_SIZE-1, GL_COLOR);
    glDisable(GL_POST_CONVOLUTION_COLOR_TABLE_SGI);
    glDisable(GL_CONVOLUTION_2D_EXT);

    draw_border();
}

/* display the image */
void
display(void)
{
    int win_width, win_height, i, error;
    float contrast_lut[CONTRAST_LUT_SIZE], scaled_pixel;

    win_width = glutGet(GLUT_WINDOW_WIDTH);
    win_height = glutGet(GLUT_WINDOW_HEIGHT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, win_width, 0, win_height, -1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    /* draw original image (with some contrast enhancement) */
    for (i = 0; i < CONTRAST_LUT_SIZE; i++) {
	if (i <= BLACK_LEVEL) {
	    contrast_lut[i] = 0;
	} else if (i >= WHITE_LEVEL) {
	    contrast_lut[i] = 1;
	} else {
	    scaled_pixel = (double)(i - BLACK_LEVEL) /
		           (double)(WHITE_LEVEL - BLACK_LEVEL);
	    contrast_lut[i] = pow(scaled_pixel, CONTRAST_GAMMA);
	}
    }
    glColorTableSGI(GL_COLOR_TABLE_SGI, GL_LUMINANCE, CONTRAST_LUT_SIZE,
		    GL_LUMINANCE, GL_FLOAT, contrast_lut);
    glEnable(GL_COLOR_TABLE_SGI);
    glRasterPos2f(IMAGE_X, IMAGE_Y);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glDrawPixels(image_w, image_h, GL_LUMINANCE, GL_UNSIGNED_BYTE, image);
    glDisable(GL_COLOR_TABLE_SGI);

    /* modify the image according to the current mode */
    switch (mode) {
    case ORIGINAL_MODE:
	break;
    case PICTOGRAM_MODE:
	pictogram();
	break;
    case POSTERIZE_MODE:
	posterize();
	break;
    case OIL_PAINT_MODE:
	oil_painting();
	break;
    case SHARPEN_MODE:
	sharpen();
	break;
    case HOR_EDGE_MODE:
	gradient_edge_detect(H_EDGES);
	break;
    case VERT_EDGE_MODE:
	gradient_edge_detect(V_EDGES);
	break;
    case DIAG_EDGE_MODE:
	gradient_edge_detect(D_EDGES);
	break;
    case LAPLACIAN_MODE:
	laplacian_edge_detect();
	break;
    }

    glutSwapBuffers();

    while ((error = glGetError()) != GL_NO_ERROR) {
	fprintf(stderr, "%s: GL error: %s\n", program, gluErrorString(error));
    }
}

/* respond to keyboard events */
void
keypress(unsigned char key, int x, int y)
{
    switch (key) {
    case '\033':
	exit(0);
    }
}

/* respond to a new menu mode */
void
choose_mode(int new_mode)
{
    mode = new_mode;
    glutPostRedisplay();
}

/* create the menu of options */
void
initmenu(void)
{
    glutCreateMenu(choose_mode);
    glutAddMenuEntry("Original Image", ORIGINAL_MODE);
    glutAddMenuEntry("Posterization", POSTERIZE_MODE);
    glutAddMenuEntry("Oil Painting", OIL_PAINT_MODE);
    glutAddMenuEntry("Sharpened Image", SHARPEN_MODE);
    glutAddMenuEntry("Pictogram", PICTOGRAM_MODE);
    glutAddMenuEntry("Horizontal Edges", HOR_EDGE_MODE);
    glutAddMenuEntry("Vertical Edges", VERT_EDGE_MODE);
    glutAddMenuEntry("Diagonal Edges", DIAG_EDGE_MODE);
    if (strcmp(glGetString(GL_RENDERER), "CRIME")) {
	/* don't try this on an O2! */
	glutAddMenuEntry("All Edges", LAPLACIAN_MODE);
    }
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void
main(int argc, char **argv)
{
    program = argv[0];
    glutInit(&argc, argv);
    if (argc > 2) {
	fprintf(stderr, "Usage: %s imagefile\n", program);
	exit(1);
    }
    if (argc == 2) {
	loadimage(argv[1]);
    } else {
	loadimage("eye512.rgb");
    }
    glutInitWindowSize(image_w + 2*MAX_BORDER, image_h + 2*MAX_BORDER);
    glutInitDisplayMode(GLUT_RGB);
    glutCreateWindow(program);
    glutDisplayFunc(display);
    glutKeyboardFunc(keypress);
    initmenu();

    if (!glutExtensionSupported("GL_EXT_convolution")) {
	fprintf(stderr, "%s: Warning: need the convolution OpenGL extension\n",
	       program);
    }
    if (!glutExtensionSupported("GL_SGI_color_table")) {
	fprintf(stderr, "%s: Warning: need the color_table OpenGL extension\n",
	       program);
    }
    if (!glutExtensionSupported("GL_EXT_blend_minmax")) {
	fprintf(stderr, "%s: Warning: need the color_table OpenGL extension\n",
	       program);
    }

    glutMainLoop();
    exit(0);
}
