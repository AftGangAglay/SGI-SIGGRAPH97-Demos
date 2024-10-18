/*    
 * Wavefront .obj file format reader.
 *
 * author: Nate Robins
 * email: ndr@pobox.com
 * www: http://www.pobox.com/~ndr
 */

/*
 * TODO: Move out `gl.h' alongside `std.h' etc. to separate base library.
 */
#define AGA_WANT_MATH
#include "../../../../../include/aga/std.h"

/* defines */
typedef enum _GLMflags {
	/* render with only vertices */
	GLM_NONE = 0,
	/* render with facet normals */
	GLM_FLAT = 1 << 0,
	/* render with vertex normals */
	GLM_SMOOTH = 1 << 1,
	/* render with texture coords */
	GLM_TEXTURE = 1 << 2,
	/* render with colors */
	GLM_COLOR = 1 << 3,
	/* render with materials */
	GLM_MATERIAL = 1 << 4
} GLMflags;

/* structs */

/* TODO: Remove once we have baselib. */
typedef enum _GLMboolean {
	GLM_FALSE = 0,
	GLM_TRUE = 1
} GLMboolean;

/* GLMmaterial: Structure that defines a material in a model. */
typedef struct _GLMmaterial {
	char* name; /* name of material */
	float diffuse[4]; /* diffuse component */
	float ambient[4]; /* ambient component */
	float specular[4]; /* specular component */
	/* TODO: This is never referenced? */
	float emmissive[4]; /* emmissive component */
	float shininess; /* specular exponent */
} GLMmaterial;

/* GLMtriangle: Structure that defines a triangle in a model. */
typedef struct _GLMtriangle {
	unsigned v_inds[3]; /* array of triangle vertex indices */
	unsigned n_inds[3]; /* array of triangle normal indices */
	unsigned t_inds[3]; /* array of triangle texcoord indices*/
	unsigned findex; /* index of triangle facet normal */
} GLMtriangle;

/* GLMgroup: Structure that defines a group in a model. */
typedef struct _GLMgroup {
	char* name; /* name of this group */
	unsigned ntris; /* number of triangles in this group */
	unsigned* triangles; /* array of triangle indices */
	unsigned material; /* index to material for group */
	struct _GLMgroup* next; /* pointer to next group in model */
} GLMgroup;

/* GLMmodel: Structure that defines a model. */
typedef struct _GLMmodel {
	char* pathname; /* path to this model */
	char* mtllibname; /* name of the material library */

	unsigned nverts; /* number of vertices in model */
	float* verts; /* array of vertices  */

	unsigned nnorms; /* number of normals in model */
	float* norms; /* array of normals */

	unsigned nuvs; /* number of texcoords in model */
	float* uvs; /* array of texture coordinates */

	unsigned nfnorms; /* number of facetnorms in model */
	float* fnorms; /* array of facetnorms */

	unsigned ntris; /* number of triangles in model */
	GLMtriangle* triangles; /* array of triangles */

	unsigned nummaterials; /* number of materials in model */
	GLMmaterial* materials; /* array of materials */

	unsigned numgroups; /* number of groups in model */
	GLMgroup* groups; /* linked list of groups */

	float position[3]; /* position of the model */
} GLMmodel;


/* public functions */

/*
 * glmUnitize: "unitize" a model by translating it to the origin and
 * scaling it to fit in a unit cube around the origin.  Returns the
 * scalefactor used.
 *
 * model - properly initialized GLMmodel structure 
 */
float glmUnitize(GLMmodel* model);

/*
 * glmDimensions: Calculates the dimensions (width, height, depth) of
 * a model.
 *
 * model - initialized GLMmodel structure
 * dimensions - array of 3 floats (float dimensions[3])
 */
void glmDimensions(GLMmodel* model, float* dimensions);

/* glmScale: Scales a model by a given amount.
 * 
 * model - properly initialized GLMmodel structure
 * scale - scalefactor (0.5 = half as large, 2.0 = twice as large)
 */
void glmScale(GLMmodel* model, float scale);

/*
 * glmReverseWinding: Reverse the polygon winding for all polygons in
 * this model. Default winding is counter-clockwise.  Also changes
 * the direction of the normals.
 * 
 * model - properly initialized GLMmodel structure 
 */
void glmReverseWinding(GLMmodel* model);

/*
 * glmFacetNormals: Generates facet normals for a model (by taking the
 * cross product of the two vectors derived from the sides of each
 * triangle). Assumes a counter-clockwise winding.
 *
 * model - initialized GLMmodel structure
 */
void glmFacetNormals(GLMmodel* model);

/*
 * glmVertexNormals: Generates smooth vertex normals for a model.
 * First builds a list of all the triangles each vertex is in.  Then
 * loops through each vertex in the list averaging all the facet
 * normals of the triangles each vertex is in.  Finally, sets the
 * normal index in the triangle for the vertex to the generated smooth
 * normal.  If the dot product of a facet normal and the facet normal
 * associated with the first triangle in the list of triangles the
 * current vertex is in is greater than the cosine of the angle
 * parameter to the function, that facet normal is not added into the
 * average normal calculation and the corresponding vertex is given
 * the facet normal.  This tends to preserve hard edges.  The angle to
 * use depends on the model, but 90 degrees is usually a good start.
 *
 * model - initialized GLMmodel structure
 * angle - maximum angle (in degrees) to smooth across
 */
void glmVertexNormals(GLMmodel* model, float angle);

/*
 * glmLinearTexture: Generates texture coordinates according to a
 * linear projection of the texture map.  It generates these by
 * linearly mapping the vertices onto a square.
 *
 * model - pointer to initialized GLMmodel structure
 */
void glmLinearTexture(GLMmodel* model);

/*
 * glmSpheremapTexture: Generates texture coordinates according to a
 * spherical projection of the texture map.  Sometimes referred to as
 * spheremap, or reflection map texture coordinates.  It generates
 * these by using the normal to calculate where that vertex would map
 * onto a sphere.  Since it is impossible to map something flat
 * perfectly onto something spherical, there is distortion at the
 * poles.  This particular implementation causes the poles along the X
 * axis to be distorted.
 *
 * model - pointer to initialized GLMmodel structure
 */
void glmSpheremapTexture(GLMmodel* model);

/*
 * glmDelete: Deletes a GLMmodel structure.
 *
 * model - initialized GLMmodel structure
 */
void glmDelete(GLMmodel* model);

/*
 * glmReadOBJ: Reads a model description from a Wavefront .OBJ file.
 * Returns a pointer to the created object which should be free'd with
 * glmDelete().
 *
 * filename - name of the file containing the Wavefront .OBJ format data.  
 */
GLMmodel* glmReadOBJ(const char* filename);

/*
 * glmWriteOBJ: Writes a model description in Wavefront .OBJ format to
 * a file.
 *
 * model - initialized GLMmodel structure
 * filename - name of the file to write the Wavefront .OBJ format data to
 * mode - a bitwise or of values describing what is written to the file
 *            GLM_NONE - write only vertices
 *            GLM_FLAT - write facet normals
 *            GLM_SMOOTH - write vertex normals
 *            GLM_TEXTURE - write texture coords
 *            GLM_FLAT and GLM_SMOOTH should not both be specified.
 */
void glmWriteOBJ(const GLMmodel* model, const char* filename, GLMflags mode);

/*
 * glmWeld: eliminate (weld) vectors that are within an epsilon of
 * each other.
 *
 * model - initialized GLMmodel structure
 * epsilon - maximum difference between vertices
 *              ( 0.00001 is a good start for a unitized model)
 */
void glmWeld(GLMmodel* model, float epsilon);
