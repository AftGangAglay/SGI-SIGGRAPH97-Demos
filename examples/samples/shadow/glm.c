/*
 * Wavefront .obj file format reader.
 *
 * author: Nate Robins
 * email: ndr@pobox.com
 * www: http://www.pobox.com/~ndr
 */

/* includes */
#include "glm.h"
/*
 * TODO: Move out `gl.h' alongside `std.h' etc. to separate base library.
 */
#define AGA_WANT_MATH
#include "../../../../../include/aga/std.h"

/* defines */
#define _GLM_TRI(x) (model->tris[(x)])

/* typedefs */

/* TODO: Review uses. */
typedef char GLMfixedbuf[128];

/* _GLMnode: general purpose node */
typedef struct _GLMnode {
	unsigned index;
	GLMboolean averaged;
	struct _GLMnode* next;
} GLMnode;

static const GLMmaterial _glmDefaultMaterial = {
		0, /* name */
		{ 0.8f, 0.8f, 0.8f, 1.0f }, /* diffuse */
		{ 0.2f, 0.2f, 0.2f, 1.0f }, /* ambient */
		{ 0.0f, 0.0f, 0.0f, 1.0f }, /* emmissive */
		{ 0.0f, 0.0f, 0.0f, 1.0f }, /* specular */
		0 /* shininess */
};

/* private functions */

/* TODO: Don't need this once we have common base lib. */
static char* _glmStrdup(const char* s) {
	char* ret;
	size_t len = strlen(s);

	if(!(ret = malloc(len + 1))) return 0;
	strcpy(ret, s);

	return ret;
}

/* _glmMax: returns the maximum of two floats */
static float _glmMax(float a, float b) {
	if(a > b) return a;

	return b;
}

/* _glmAbs: returns the absolute value of a float */
static float _glmAbs(float f) {
	if(f < 0) return -f;

	return f;
}

/*
 * _glmDot: compute the dot product of two vectors
 *
 * u - array of 3 floats (float u[3])
 * v - array of 3 floats (float v[3])
 */
static float _glmDot(const float* u, const float* v) {
	/* compute the dot product */
	return u[0] * v[0] + u[1] * v[1] + u[2] * v[2];
}

/*
 * _glmCross: compute the cross product of two vectors
 *
 * u - array of 3 floats (float u[3])
 * v - array of 3 floats (float v[3])
 * n - array of 3 floats (float n[3]) to return the cross product in
 */
static void _glmCross(const float* u, const float* v, float* n) {
	/* compute the cross product (u x v for right-handed [ccw]) */
	n[0] = u[1] * v[2] - u[2] * v[1];
	n[1] = u[2] * v[0] - u[0] * v[2];
	n[2] = u[0] * v[1] - u[1] * v[0];
}

/*
 * _glmNormalize: normalize a vector
 *
 * n - array of 3 floats (float n[3]) to be normalized
 */
static void _glmNormalize(float* n) {
	/* normalize */
	float l = (float) sqrt(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);

	n[0] /= l;
	n[1] /= l;
	n[2] /= l;
}

/*
 * _glmEqual: compares two vectors and returns GLM_TRUE if they are
 * equal (within a certain threshold) or GLM_FALSE if not. An epsilon
 * that works fairly well is 0.000001.
 *
 * u - array of 3 floats (float u[3])
 * v - array of 3 floats (float v[3])
 */
static GLMboolean _glmEqual(const float* u, const float* v, float epsilon) {
	return _glmAbs(u[0] - v[0]) < epsilon &&
			_glmAbs(u[1] - v[1]) < epsilon &&
			_glmAbs(u[2] - v[2]) < epsilon;
}

/*
 * _glmWeldVectors: eliminate (weld) vectors that are within an
 * epsilon of each other.
 *
 * vectors - array of float[3]'s to be welded
 * numvectors - number of float[3]'s in vectors
 * epsilon - maximum difference between vectors
 */
static float* _glmWeldVectors(
		float* vectors, unsigned* numvectors, float epsilon) {

	float* copies;
	unsigned copied;
	unsigned i, j;

	copies = malloc(sizeof(float) * 3 * (*numvectors + 1));
	/* TODO: Clearer EH conditions? */
	if(!copies) return 0;

	memcpy(copies, vectors, (sizeof(float) * 3 * (*numvectors + 1)));

	copied = 1;
	for(i = 1; i <= *numvectors; i++) {
		for(j = 1; j <= copied; j++) {
			if(_glmEqual(&vectors[3 * i], &copies[3 * j], epsilon)) {
				goto duplicate;
			}
		}

		/* must not be any duplicates -- add to the copies array */
		copies[3 * copied + 0] = vectors[3 * i + 0];
		copies[3 * copied + 1] = vectors[3 * i + 1];
		copies[3 * copied + 2] = vectors[3 * i + 2];

		j = copied; /* pass this along for below */
		copied++;

		/*
		 * set the first component of this vector to point at the correct
		 * index into the new copies array
		 */
		duplicate: vectors[3 * i + 0] = (float) j;
	}

	*numvectors = copied - 1;

	return copies;
}

/* _glmFindGroup: Find a group in the model */
static GLMgroup* _glmFindGroup(const GLMmodel* model, const char* name) {
	GLMgroup* group;

	group = model->groups;

	/* TODO: Handle fallthrough case. */
	while(group) {
		if(!strcmp(name, group->name)) break;

		group = group->next;
	}

	return group;
}

/* _glmAddGroup: Add a group to the model */
static GLMgroup* _glmAddGroup(GLMmodel* model, char* name) {
	GLMgroup* group;

	group = _glmFindGroup(model, name);

	if(!group) {
		/* TODO: Clearer EH conditions? */
		group = malloc(sizeof(GLMgroup));
		if(!group) return 0;

		group->name = _glmStrdup(name);
		group->material = 0;
		group->ntris = 0;
		group->tris = NULL;
		group->next = model->groups;
		model->groups = group;
		model->ngroups++;
	}

	return group;
}

/* _glmFindGroup: Find a material in the model */
static unsigned _glmFindMaterial(const GLMmodel* model, const char* name) {
	unsigned i;

	for(i = 0; i < model->nmats; i++) {
		if(!strcmp(model->mats[i].name, name)) goto found;
	}

	/* TODO: Better EH. */
	/* didn't find the name, so set it as the default material */
	printf("_glmFindMaterial():  can't find material \"%s\".\n", name);
	i = 0;

	found: return i;
}


/*
 * _glmDirName: return the directory given a path
 *
 * path - filesystem path
 *
 * The return value should be free'd.
 */
static char* _glmDirName(const char* path) {
	char* dir;
	char* s;

	/* TODO: Clearer EH? */
	dir = _glmStrdup(path);
	if(!dir) return 0;

	s = strrchr(dir, '/');

	if(s) s[1] = '\0';
	else dir[0] = '\0';

	return dir;
}


/*
 * _glmReadMTL: read a wavefront material library file
 *
 * model - properly initialized GLMmodel structure
 * name - name of the material library
 */
static void _glmReadMTL(GLMmodel* model, const char* name) {
	FILE* file;
	char* dir;
	char* filename;
	GLMfixedbuf buf;
	unsigned nmats, i;

	/* TODO: Handle OOM. */
	dir = _glmDirName(model->pathname);
	{
		/* TODO: Handle OOM. */
		filename = malloc(strlen(dir) + strlen(name) + 1);

		strcpy(filename, dir);
		strcat(filename, name);
	}
	free(dir);

	/* open the file */
	/* TODO: Better EH. */
	if(!(file = fopen(filename, "r"))) {
		static const char msg[] =
				"_glmReadMTL() failed: can't open material file \"%s\".\n";

		fprintf(stderr, msg, filename);
		exit(1);
	}
	free(filename);

	/* count the number of mats in the file */
	nmats = 1;

	/* TODO: Re-check all stdio EH in here. */
	while(fscanf(file, "%s", buf) != EOF) {
		switch(buf[0]) {
			/* comment */
			case '#': {
				/* eat up rest of line */
				fgets(buf, sizeof(buf), file);
				break;
			}

			/* newmtl */
			case 'n': {
				fgets(buf, sizeof(buf), file);
				nmats++;
				sscanf(buf, "%s %s", buf, buf);
				break;
			}

			default: {
				/* eat up rest of line */
				fgets(buf, sizeof(buf), file);
				break;
			}
		}
	}

	rewind(file);

	/* allocate memory for the mats */
	/* TODO: Handle OOM. */
	model->mats = malloc(sizeof(GLMmaterial) * nmats);
	model->nmats = nmats;

	/* set the default material */
	for(i = 0; i < nmats; i++) {
		model->mats[i] = _glmDefaultMaterial;
	}
	model->mats[0].name = _glmStrdup("default");

	/* now, read in the data */
	nmats = 0;

	while(fscanf(file, "%s", buf) != EOF) {
		switch(buf[0]) {
			/* comment */
			case '#': {
				/* eat up rest of line */
				fgets(buf, sizeof(buf), file);
				break;
			}

			/* newmtl */
			case 'n': {
				fgets(buf, sizeof(buf), file);
				sscanf(buf, "%s %s", buf, buf);
				nmats++;
				model->mats[nmats].name = _glmStrdup(buf);
				break;
			}

			case 'N': {
				fscanf(file, "%f", &model->mats[nmats].shininess);
				/*
				 * wavefront shininess is from [0, 1000],
				 * so scale for OpenGL
				 */
				model->mats[nmats].shininess /= 1000.0;
				model->mats[nmats].shininess *= 128.0;
				break;
			}

			case 'K': {
				switch(buf[1]) {
					/* TODO: Handle emissive here. */
					case 'd': {
						fscanf(file, "%f %f %f",
								&model->mats[nmats].diffuse[0],
								&model->mats[nmats].diffuse[1],
								&model->mats[nmats].diffuse[2]);
						break;
					}

					case 's': {
						fscanf(file, "%f %f %f",
								&model->mats[nmats].specular[0],
								&model->mats[nmats].specular[1],
								&model->mats[nmats].specular[2]);
						break;
					}

					case 'a': {
						fscanf(file, "%f %f %f",
								&model->mats[nmats].ambient[0],
								&model->mats[nmats].ambient[1],
								&model->mats[nmats].ambient[2]);
						break;
					}

					default: {
						/* eat up rest of line */
						fgets(buf, sizeof(buf), file);
						break;
					}
				}

				break;
			}

			default: {
				/* eat up rest of line */
				fgets(buf, sizeof(buf), file);
				break;
			}
		}
	}

	/* TODO: EH. */
	fclose(file);
}

/*
 * _glmWriteMTL: write a wavefront material library file
 *
 * model - properly initialized GLMmodel structure
 * modelpath - pathname of the model being written
 * mtllibname - name of the material library to be written
 */
static void _glmWriteMTL(
		const GLMmodel* model, const char* modelpath, const char* mtllibname) {

	FILE* file;
	char* dir;
	char* filename;
	GLMmaterial* material;
	unsigned i;

	/* TODO: Handle OOM. */
	dir = _glmDirName(modelpath);
	filename = malloc((strlen(dir) + strlen(mtllibname)));
	strcpy(filename, dir);
	strcat(filename, mtllibname);
	free(dir);

	/* open the file */
	/* TODO: Better EH. */
	/* TODO: Should fopen use binary mode on Windows? */
	file = fopen(filename, "w");
	if(!file) {
		static const char msg[] =
				"_glmWriteMTL() failed: can't open file \"%s\".\n";

		fprintf(stderr, msg, filename);
		exit(1);
	}

	free(filename);

	/* spit out a header */
	fprintf(file, "# \n");
	fprintf(file, "# Wavefront MTL generated by GLM library\n");
	fprintf(file, "# \n");
	fprintf(file, "# GLM library copyright (C) 1997 by Nate Robins\n");
	fprintf(file, "# email: ndr@pobox.com\n");
	fprintf(file, "# www: http://www.pobox.com/~ndr\n");
	fprintf(file, "# \n\n");

	for(i = 0; i < model->nmats; i++) {
		material = &model->mats[i];
		fprintf(file, "newmtl %s\n", material->name);
		fprintf(file, "Ka %f %f %f\n",
		material->ambient[0], material->ambient[1], material->ambient[2]);
		fprintf(file, "Kd %f %f %f\n",
		material->diffuse[0], material->diffuse[1], material->diffuse[2]);
		fprintf(file, "Ks %f %f %f\n",
		material->specular[0],material->specular[1],material->specular[2]);
		fprintf(file, "Ns %f\n", material->shininess);
		fprintf(file, "\n");
	}

	/* TODO: EH. */
	fclose(file);
}


/*
 * _glmFirstPass: first pass at a Wavefront OBJ file that gets all the
 * statistics of the model (such as #verts, #norms, etc)
 *
 * model - properly initialized GLMmodel structure
 * file - (fopen'd) file descriptor
 */
static void _glmFirstPass(GLMmodel* model, FILE* file) {
	unsigned nverts; /* number of verts in model */
	unsigned nnorms; /* number of norms in model */
	unsigned nuvs; /* number of uvs in model */
	unsigned ntris; /* number of tris in model */
	GLMgroup* group; /* current group */
	unsigned v, n, t;
	GLMfixedbuf buf;

	/* make a default group */
	group = _glmAddGroup(model, "default");

	nverts = nnorms = nuvs = ntris = 0;

	while(fscanf(file, "%s", buf) != EOF) {
		switch(buf[0]) {
			/* comment */
			case '#': {
				/* eat up rest of line */
				fgets(buf, sizeof(buf), file);
				break;
			}

			/* v, vn, vt */
			case 'v': {
				switch(buf[1]) {
					/* vertex */
					case '\0': {
						/* eat up rest of line */
						fgets(buf, sizeof(buf), file);
						nverts++;
						break;
					}

					/* normal */
					case 'n': {
						/* eat up rest of line */
						fgets(buf, sizeof(buf), file);
						nnorms++;
						break;
					}

					/* texcoord */
					case 't': {
						/* eat up rest of line */
						fgets(buf, sizeof(buf), file);
						nuvs++;
						break;
					}

					default: {
						static const char msg[] =
								"_glmFirstPass(): Unknown token \"%s\".\n";

						/* TODO: EH. */
						printf(msg, buf);
						exit(1);
						break;
					}
				}

				break;
			}

			case 'm': {
				fgets(buf, sizeof(buf), file);
				sscanf(buf, "%s %s", buf, buf);
				model->mtllibname = _glmStrdup(buf);
				_glmReadMTL(model, buf);
				break;
			}

			/* TODO: Is this something being unhandled? */
			case 'u': {
				/* eat up rest of line */
				fgets(buf, sizeof(buf), file);
				break;
			}

			/* group */
			case 'g': {
				/* eat up rest of line */
				fgets(buf, sizeof(buf), file);
				sscanf(buf, "%s", buf);
				group = _glmAddGroup(model, buf);
				break;
			}

			/* face */
			case 'f': {
				v = n = t = 0;

				fscanf(file, "%s", buf);

				/* can be one of %d, %d//%d, %d/%d, %d/%d/%d %d//%d */
				if(strstr(buf, "//")) {
					/* v//n */
					sscanf(buf, "%u//%u", &v, &n);
					fscanf(file, "%u//%u", &v, &n);
					fscanf(file, "%u//%u", &v, &n);

					ntris++;
					group->ntris++;

					while(fscanf(file, "%u//%u", &v, &n) > 0) {
						ntris++;
						group->ntris++;
					}
				}
				else if(sscanf(buf, "%u/%u/%u", &v, &t, &n) == 3) {
					/* v/t/n */
					fscanf(file, "%u/%u/%u", &v, &t, &n);
					fscanf(file, "%u/%u/%u", &v, &t, &n);

					ntris++;
					group->ntris++;

					while(fscanf(file, "%u/%u/%u", &v, &t, &n) > 0) {
						ntris++;
						group->ntris++;
					}
				}
				else if(sscanf(buf, "%u/%u", &v, &t) == 2) {
					/* v/t */
					fscanf(file, "%u/%u", &v, &t);
					fscanf(file, "%u/%u", &v, &t);

					ntris++;
					group->ntris++;

					while(fscanf(file, "%u/%u", &v, &t) > 0) {
						ntris++;
						group->ntris++;
					}
				}
				else {
					/* v */
					/* TODO: What? */
					fscanf(file, "%u", &v);
					fscanf(file, "%u", &v);

					ntris++;
					group->ntris++;

					while(fscanf(file, "%u", &v) > 0) {
						ntris++;
						group->ntris++;
					}
				}

				break;
			}

			default: {
				/* eat up rest of line */
				fgets(buf, sizeof(buf), file);
				break;
			}
		}
	}

	/* TODO: Verbose mode for loads? */
#if 0
	/* announce the model statistics */
	printf(" verts: %d\n", nverts);
	printf(" norms: %d\n", nnorms);
	printf(" uvs: %d\n", nuvs);
	printf(" tris: %d\n", ntris);
	printf(" Groups: %d\n", model->ngroups);
#endif

	/* set the stats in the model structure */
	model->nverts = nverts;
	model->nnorms = nnorms;
	model->nuvs = nuvs;
	model->ntris = ntris;

	/* allocate memory for the tris in each group */
	group = model->groups;

	while(group) {
		group->tris = malloc(sizeof(unsigned) * group->ntris);
		group->ntris = 0;
		group = group->next;
	}
}

/*
 * _glmSecondPass: second pass at a Wavefront OBJ file that gets all
 * the data.
 *
 * model - properly initialized GLMmodel structure
 * file - (fopen'd) file descriptor
 */
static void _glmSecondPass(GLMmodel* model, FILE* file) {
	unsigned nverts; /* number of verts in model */
	unsigned nnorms; /* number of norms in model */
	unsigned nuvs; /* number of uvs in model */
	unsigned ntris; /* number of tris in model */

	float* verts; /* array of verts  */
	float* norms; /* array of norms */
	float* uvs; /* array of texture coordinates */

	GLMgroup* group; /* current group pointer */
	unsigned material; /* current material */

	unsigned v, n, t;
	unsigned ind;

	GLMfixedbuf buf;

	/* set the pointer shortcuts */
	verts = model->verts;
	norms = model->norms;
	uvs = model->uvs;
	group = model->groups;

	/*
	 * on the second pass through the file, read all the data into the
	 * allocated arrays
	 */
	nverts = nnorms = nuvs = 1;
	ntris = 0;
	material = 0;

	while(fscanf(file, "%s", buf) != EOF) {
		switch(buf[0]) {
			/* comment */
			case '#': {
				/* eat up rest of line */
				fgets(buf, sizeof(buf), file);
				break;
			}

			/* v, vn, vt */
			case 'v': {
				switch(buf[1]) {
					/* vertex */
					case '\0': {
						fscanf(file, "%f %f %f",
								&verts[3 * nverts + 0],
								&verts[3 * nverts + 1],
								&verts[3 * nverts + 2]);

						nverts++;
						break;
					}

					/* normal */
					case 'n': {
						fscanf(file, "%f %f %f",
								&norms[3 * nnorms + 0],
								&norms[3 * nnorms + 1],
								&norms[3 * nnorms + 2]);

						nnorms++;
						break;
					}

					/* texcoord */
					case 't': {
						fscanf(file, "%f %f",
								&uvs[2 * nuvs + 0],
								&uvs[2 * nuvs + 1]);

						nuvs++;
						break;
					}
				}

				break;
			}

			case 'u': {
				fgets(buf, sizeof(buf), file);
				sscanf(buf, "%s %s", buf, buf);
				group->material = material = _glmFindMaterial(model, buf);
				break;
			}

			case 'g': {
				/* eat up rest of line */
				fgets(buf, sizeof(buf), file);
				sscanf(buf, "%s", buf);
				group = _glmFindGroup(model, buf);
				group->material = material;
				break;
			}

			/* face */
			case 'f': {
				v = n = t = 0;

				fscanf(file, "%s", buf);

				/* can be one of %d, %d//%d, %d/%d, %d/%d/%d %d//%d */
				if(strstr(buf, "//")) {
					/* v//n */
					sscanf(buf, "%u//%u", &v, &n);
					_GLM_TRI(ntris).v_inds[0] = v;
					_GLM_TRI(ntris).n_inds[0] = n;

					fscanf(file, "%u//%u", &v, &n);
					_GLM_TRI(ntris).v_inds[1] = v;
					_GLM_TRI(ntris).n_inds[1] = n;

					fscanf(file, "%u//%u", &v, &n);
					_GLM_TRI(ntris).v_inds[2] = v;
					_GLM_TRI(ntris).n_inds[2] = n;

					group->tris[group->ntris++] = ntris;
					ntris++;

					while(fscanf(file, "%u//%u", &v, &n) > 0) {
						ind = _GLM_TRI(ntris - 1).v_inds[0];
						_GLM_TRI(ntris).v_inds[0] = ind;
						ind = _GLM_TRI(ntris - 1).n_inds[0];

						_GLM_TRI(ntris).n_inds[0] = ind;
						ind = _GLM_TRI(ntris - 1).v_inds[2];
						_GLM_TRI(ntris).v_inds[1] = ind;
						ind = _GLM_TRI(ntris - 1).n_inds[2];
						_GLM_TRI(ntris).n_inds[1] = ind;

						_GLM_TRI(ntris).v_inds[2] = v;
						_GLM_TRI(ntris).n_inds[2] = n;

						group->tris[group->ntris++] = ntris;
						ntris++;
					}
				}
				else if(sscanf(buf, "%u/%u/%u", &v, &t, &n) == 3) {
					/* v/t/n */
					_GLM_TRI(ntris).v_inds[0] = v;
					_GLM_TRI(ntris).t_inds[0] = t;
					_GLM_TRI(ntris).n_inds[0] = n;

					fscanf(file, "%u/%u/%u", &v, &t, &n);
					_GLM_TRI(ntris).v_inds[1] = v;
					_GLM_TRI(ntris).t_inds[1] = t;
					_GLM_TRI(ntris).n_inds[1] = n;

					fscanf(file, "%u/%u/%u", &v, &t, &n);
					_GLM_TRI(ntris).v_inds[2] = v;
					_GLM_TRI(ntris).t_inds[2] = t;
					_GLM_TRI(ntris).n_inds[2] = n;

					group->tris[group->ntris++] = ntris;
					ntris++;

					while(fscanf(file, "%u/%u/%u", &v, &t, &n) > 0) {
						ind = _GLM_TRI(ntris - 1).v_inds[0];
						_GLM_TRI(ntris).v_inds[0] = ind;
						ind = _GLM_TRI(ntris - 1).t_inds[0];
						_GLM_TRI(ntris).t_inds[0] = ind;
						ind = _GLM_TRI(ntris - 1).n_inds[0];
						_GLM_TRI(ntris).n_inds[0] = ind;

						ind = _GLM_TRI(ntris - 1).v_inds[2];
						_GLM_TRI(ntris).v_inds[1] = ind;
						ind = _GLM_TRI(ntris - 1).t_inds[2];
						_GLM_TRI(ntris).t_inds[1] = ind;
						ind = _GLM_TRI(ntris - 1).n_inds[2];
						_GLM_TRI(ntris).n_inds[1] = ind;

						_GLM_TRI(ntris).v_inds[2] = v;
						_GLM_TRI(ntris).t_inds[2] = t;
						_GLM_TRI(ntris).n_inds[2] = n;

						group->tris[group->ntris++] = ntris;
						ntris++;
					}
				}
				else if(sscanf(buf, "%u/%u", &v, &t) == 2) {
					/* v/t */
					_GLM_TRI(ntris).v_inds[0] = v;
					_GLM_TRI(ntris).t_inds[0] = t;

					fscanf(file, "%u/%u", &v, &t);
					_GLM_TRI(ntris).v_inds[1] = v;
					_GLM_TRI(ntris).t_inds[1] = t;

					fscanf(file, "%u/%u", &v, &t);
					_GLM_TRI(ntris).v_inds[2] = v;
					_GLM_TRI(ntris).t_inds[2] = t;

					group->tris[group->ntris++] = ntris;
					ntris++;

					while(fscanf(file, "%u/%u", &v, &t) > 0) {
						ind = _GLM_TRI(ntris - 1).v_inds[0];
						_GLM_TRI(ntris).v_inds[0] = ind;
						ind = _GLM_TRI(ntris - 1).t_inds[0];
						_GLM_TRI(ntris).t_inds[0] = ind;

						ind = _GLM_TRI(ntris - 1).v_inds[2];
						_GLM_TRI(ntris).v_inds[1] = ind;
						ind = _GLM_TRI(ntris - 1).t_inds[2];
						_GLM_TRI(ntris).t_inds[1] = ind;

						_GLM_TRI(ntris).v_inds[2] = v;
						_GLM_TRI(ntris).t_inds[2] = t;

						group->tris[group->ntris++] = ntris;
						ntris++;
					}
				}
				else {
					/* v */
					sscanf(buf, "%u", &v);
					_GLM_TRI(ntris).v_inds[0] = v;

					fscanf(file, "%u", &v);
					_GLM_TRI(ntris).v_inds[1] = v;

					fscanf(file, "%u", &v);
					_GLM_TRI(ntris).v_inds[2] = v;

					group->tris[group->ntris++] = ntris;
					ntris++;

					while(fscanf(file, "%u", &v) > 0) {
						ind = _GLM_TRI(ntris - 1).v_inds[0];
						_GLM_TRI(ntris).v_inds[0] = ind;

						ind = _GLM_TRI(ntris - 1).v_inds[2];
						_GLM_TRI(ntris).v_inds[1] = ind;

						_GLM_TRI(ntris).v_inds[2] = v;

						group->tris[group->ntris++] = ntris;
						ntris++;
					}
				}

				break;
			}

			default: {
				/* eat up rest of line */
				fgets(buf, sizeof(buf), file);
				break;
			}
		}
	}

	/* TODO: Verbose output. */
#if 0
	/* announce the memory requirements */
	printf(" Memory: %d bytes\n",
	nverts  * 3*sizeof(float) +
	nnorms   * 3*sizeof(float) * (nnorms ? 1 : 0) +
	nuvs * 3*sizeof(float) * (nuvs ? 1 : 0) +
	ntris * sizeof(GLMtriangle));
#endif
}


/* public functions */

float glmUnitize(GLMmodel* model) {
	unsigned i;
	float extent[6];
	float cx, cy, cz, w, h, d;
	float scale;

	/* get the max/mins */
	extent[3] = extent[0] = model->verts[3 + 0];
	extent[4] = extent[1] = model->verts[3 + 1];
	extent[5] = extent[2] = model->verts[3 + 2];

	for(i = 1; i <= model->nverts; i++) {
		unsigned x = 3 * i;

		if(extent[3] < model->verts[x + 0]) extent[3] = model->verts[x + 0];
		if(extent[0] > model->verts[x + 0]) extent[0] = model->verts[x + 0];

		if(extent[4] < model->verts[x + 1]) extent[4] = model->verts[x + 1];
		if(extent[1] > model->verts[x + 1]) extent[1] = model->verts[x + 1];

		if(extent[5] < model->verts[x + 2]) extent[5] = model->verts[x + 2];
		if(extent[2] > model->verts[x + 2]) extent[2] = model->verts[x + 2];
	}

	/* calculate model width, height, and depth */
	w = _glmAbs(extent[3]) + _glmAbs(extent[0]);
	h = _glmAbs(extent[4]) + _glmAbs(extent[1]);
	d = _glmAbs(extent[5]) + _glmAbs(extent[2]);

	/* calculate center of the model */
	cx = (extent[3] + extent[0]) / 2.0f;
	cy = (extent[4] + extent[1]) / 2.0f;
	cz = (extent[5] + extent[2]) / 2.0f;

	/* calculate unitizing scale factor */
	scale = 2.0f / _glmMax(_glmMax(w, h), d);

	/* translate around center then scale */
	for(i = 1; i <= model->nverts; i++) {
		unsigned x = 3 * i;

		model->verts[x + 0] -= cx;
		model->verts[x + 1] -= cy;
		model->verts[x + 2] -= cz;

		model->verts[x + 0] *= scale;
		model->verts[x + 1] *= scale;
		model->verts[x + 2] *= scale;
	}

	return scale;
}

void glmExtent(GLMmodel* model, float* extent) {
	unsigned i;

	/* get the max/mins */
	extent[3] = extent[0] = model->verts[3 + 0];
	extent[4] = extent[1] = model->verts[3 + 1];
	extent[5] = extent[2] = model->verts[3 + 2];

	for(i = 1; i <= model->nverts; i++) {
		unsigned x = 3 * i;

		if(extent[3] < model->verts[x + 0]) extent[3] = model->verts[x + 0];
		if(extent[0] > model->verts[x + 0]) extent[0] = model->verts[x + 0];

		if(extent[4] < model->verts[x + 1]) extent[4] = model->verts[x + 1];
		if(extent[1] > model->verts[x + 1]) extent[1] = model->verts[x + 1];

		if(extent[5] < model->verts[x + 2]) extent[5] = model->verts[x + 2];
		if(extent[2] > model->verts[x + 2]) extent[2] = model->verts[x + 2];
	}
}

void glmDimensions(GLMmodel* model, float* dimensions) {
	float extent[6];

	glmExtent(model, extent);

	/* calculate model width, height, and depth */
	dimensions[0] = _glmAbs(extent[3]) + _glmAbs(extent[0]);
	dimensions[1] = _glmAbs(extent[4]) + _glmAbs(extent[1]);
	dimensions[2] = _glmAbs(extent[5]) + _glmAbs(extent[2]);
}

void glmScale(GLMmodel* model, float scale) {
	unsigned i;

	for(i = 1; i <= model->nverts; i++) {
		model->verts[3 * i + 0] *= scale;
		model->verts[3 * i + 1] *= scale;
		model->verts[3 * i + 2] *= scale;
	}
}

void glmReverseWinding(GLMmodel* model) {
	unsigned i, swap;

	for(i = 0; i < model->ntris; i++) {
		swap = _GLM_TRI(i).v_inds[0];
		_GLM_TRI(i).v_inds[0] = _GLM_TRI(i).v_inds[2];
		_GLM_TRI(i).v_inds[2] = swap;

		if(model->nnorms) {
			swap = _GLM_TRI(i).n_inds[0];
			_GLM_TRI(i).n_inds[0] = _GLM_TRI(i).n_inds[2];
			_GLM_TRI(i).n_inds[2] = swap;
		}

		if(model->nuvs) {
			swap = _GLM_TRI(i).t_inds[0];
			_GLM_TRI(i).t_inds[0] = _GLM_TRI(i).t_inds[2];
			_GLM_TRI(i).t_inds[2] = swap;
		}
	}

	/* reverse facet norms */
	for(i = 1; i <= model->nfnorms; i++) {
		unsigned x = 3 * i;

		model->fnorms[x + 0] = -model->fnorms[x + 0];
		model->fnorms[x + 1] = -model->fnorms[x + 1];
		model->fnorms[x + 2] = -model->fnorms[x + 2];
	}

	/* reverse vertex norms */
	for(i = 1; i <= model->nnorms; i++) {
		unsigned x = 3 * i;

		model->norms[x + 0] = -model->norms[x + 0];
		model->norms[x + 1] = -model->norms[x + 1];
		model->norms[x + 2] = -model->norms[x + 2];
	}
}

void glmFacetNormals(GLMmodel* model) {
	unsigned i;
	float u[3];
	float v[3];

	/* clobber any old facetnormals */
	if(model->fnorms) free(model->fnorms);

	/* allocate memory for the new facet norms */
	model->nfnorms = model->ntris;
	/* TODO: Handle OOM. */
	model->fnorms = malloc(sizeof(float) * 3 * (model->nfnorms + 1));

	for(i = 0; i < model->ntris; i++) {
		unsigned x, y, z;

		model->tris[i].findex = i+1;

		x = _GLM_TRI(i).v_inds[0];
		y = _GLM_TRI(i).v_inds[1];
		z = _GLM_TRI(i).v_inds[2];

		u[0] = model->verts[3 * y + 0] - model->verts[3 * x + 0];
		u[1] = model->verts[3 * y + 1] - model->verts[3 * x + 1];
		u[2] = model->verts[3 * y + 2] - model->verts[3 * x + 2];

		v[0] = model->verts[3 * z + 0] - model->verts[3 * x + 0];
		v[1] = model->verts[3 * z + 1] - model->verts[3 * x + 1];
		v[2] = model->verts[3 * z + 2] - model->verts[3 * x + 2];

		_glmCross(u, v, &model->fnorms[3 * (i + 1)]);
		_glmNormalize(&model->fnorms[3 * (i + 1)]);
	}
}

void glmVertexNormals(GLMmodel* model, float angle) {
	GLMnode* node;
	GLMnode* tail;
	GLMnode** members;
	float* norms;
	unsigned nnorms;
	float average[3];
	float dot, cos_angle;
	unsigned i, avg;

	/* calculate the cosine of the angle (in degrees) */
	cos_angle = cos(angle * M_PI / 180.0);

	/* nuke any previous norms */
	if(model->norms) free(model->norms);

	/* allocate space for new norms */
	model->nnorms = model->ntris * 3; /* 3 norms per triangle */
	/* TODO: Handle OOM. */
	model->norms = malloc(sizeof(float) * 3 * (model->nnorms + 1));

	/*
	 * allocate a structure that will hold a linked list of triangle
	 * indices for each vertex
	 */
	/* TODO: Handle OOM. */
	/* TODO: Why not calloc? */
	members = malloc(sizeof(GLMnode*) * (model->nverts + 1));
	for(i = 1; i <= model->nverts; i++) members[i] = NULL;

	/* for every triangle, create a node for each vertex in it */
	for(i = 0; i < model->ntris; i++) {
		/* TODO: Handle OOM. */
		node = malloc(sizeof(GLMnode));
		node->index = i;
		node->next  = members[_GLM_TRI(i).v_inds[0]];
		members[_GLM_TRI(i).v_inds[0]] = node;

		/* TODO: Handle OOM. */
		node = malloc(sizeof(GLMnode));
		node->index = i;
		node->next  = members[_GLM_TRI(i).v_inds[1]];
		members[_GLM_TRI(i).v_inds[1]] = node;

		/* TODO: Handle OOM. */
		node = malloc(sizeof(GLMnode));
		node->index = i;
		node->next  = members[_GLM_TRI(i).v_inds[2]];
		members[_GLM_TRI(i).v_inds[2]] = node;
	}

	/* calculate the average normal for each vertex */
	nnorms = 1;
	for(i = 1; i <= model->nverts; i++) {
		/*
		 * calculate an average normal for this vertex by averaging the
		 * facet normal of every triangle this vertex is in
		 */
		node = members[i];

		/* TODO: Better EH. */
		if(!node) {
			fprintf(stderr, "glmVertexNormals(): vertex w/o a triangle\n");
		}

		memset(average, 0, sizeof(average));

		avg = 0;
		while(node) {
			unsigned findex = _GLM_TRI(node->index).findex;
			unsigned memb_findex = _GLM_TRI(members[i]->index).findex;

			float* x;
			float* y;

			/*
			 * only average if the dot product of the angle between the two
			 * facet norms is greater than the cosine of the threshold
			 * angle -- or, said another way, the angle between the two
			 * facet norms is less than (or equal to) the threshold angle
			 */
			x = &model->fnorms[3 * findex];
			y = &model->fnorms[3 * memb_findex];
			dot = _glmDot(x, y);

			if(dot > cos_angle) {
				node->averaged = GLM_TRUE;
				average[0] += model->fnorms[3 * findex + 0];
				average[1] += model->fnorms[3 * findex + 1];
				average[2] += model->fnorms[3 * findex + 2];
				avg = 1; /* we averaged at least one normal! */
			}
			else node->averaged = GLM_FALSE;

			node = node->next;
		}

		if(avg) {
			/* normalize the averaged normal */
			_glmNormalize(average);

			/* add the normal to the vertex norms list */
			model->norms[3 * nnorms + 0] = average[0];
			model->norms[3 * nnorms + 1] = average[1];
			model->norms[3 * nnorms + 2] = average[2];

			avg = nnorms;
			nnorms++;
		}

		/* set the normal of this vertex in each triangle it is in */
		node = members[i];

		while(node) {
			unsigned* a = &_GLM_TRI(node->index).v_inds[0];
			unsigned* b = &_GLM_TRI(node->index).v_inds[1];
			unsigned* c = &_GLM_TRI(node->index).v_inds[2];

			if(node->averaged) {
				/* if this node was averaged, use the average normal */
				if(*a == i) *a = avg;
				else if(*b == i) *b = avg;
				else if(*c == i) *c = avg;
			}
			else {
				unsigned findex = _GLM_TRI(node->index).findex;

				/* if this node wasn't averaged, use the facet normal */
				model->norms[3 * nnorms + 0] = model->fnorms[3 * findex + 0];
				model->norms[3 * nnorms + 1] = model->fnorms[3 * findex + 1];
				model->norms[3 * nnorms + 2] = model->fnorms[3 * findex + 2];

				if(*a == i) *a = nnorms;
				else if(*b == i) *b = nnorms;
				else if(*c == i) *c = nnorms;

				nnorms++;
			}

			node = node->next;
		}
	}

	model->nnorms = nnorms - 1;

	/* free the member information */
	for(i = 1; i <= model->nverts; i++) {
		node = members[i];

		while(node) {
			tail = node;
			node = node->next;
			free(tail);
		}
	}
	free(members);

	/*
	 * pack the norms array (we previously allocated the maximum
	 * number of norms that could possibly be created (ntris * 3),
	 * so get rid of some of them (usually alot unless none of the facet
	 * norms were averaged)
	 */
	norms = model->norms;
	/* TODO: Handle OOM. */
	model->norms = malloc(sizeof(float) * 3 * (model->nnorms + 1));

	for(i = 1; i <= model->nnorms; i++) {
		model->norms[3 * i + 0] = norms[3 * i + 0];
		model->norms[3 * i + 1] = norms[3 * i + 1];
		model->norms[3 * i + 2] = norms[3 * i + 2];
	}
	free(norms);

	/* TODO: Verbose output. */
	printf("glmVertexNormals(): %d norms generated\n", model->nnorms);
}

void glmLinearTexture(GLMmodel* model) {
	GLMgroup* group;
	float dimensions[3];
	float x, y, a, scalefactor;
	unsigned i;

	if(model->uvs) free(model->uvs);

	model->nuvs = model->nverts;
	/* TODO: Handle OOM. */
	model->uvs = malloc(sizeof(float) * 2 * (model->nuvs + 1));

	glmDimensions(model, dimensions);
	a = _glmMax(dimensions[0], dimensions[1]);
	a = _glmMax(a, dimensions[2]);
	a = _glmAbs(a);

	scalefactor = 2.0f / a;

	/* do the calculations */
	for(i = 1; i <= model->nverts; i++) {
		x = model->verts[3 * i + 0] * scalefactor;
		y = model->verts[3 * i + 2] * scalefactor;

		model->uvs[2 * i + 0] = (x + 1.0f) / 2.0f;
		model->uvs[2 * i + 1] = (y + 1.0f) / 2.0f;
	}

	/* go through and put texture coordinate indices in all the tris */
	group = model->groups;

	while(group) {
		for(i = 0; i < group->ntris; i++) {
			GLMtriangle* t = &_GLM_TRI(group->tris[i]);

			t->t_inds[0] = t->v_inds[0];
			t->t_inds[1] = t->v_inds[1];
			t->t_inds[2] = t->v_inds[2];
		}

		group = group->next;
	}

#if 0
	printf("glmLinearTexture(): generated %d linear texture coordinates\n",
	model->nuvs);
#endif
}

void glmSpheremapTexture(GLMmodel* model) {
	GLMgroup* group;
	float theta, phi, rho, x, y, z, r;
	unsigned i;

	if(model->uvs) free(model->uvs);

	model->nuvs = model->nnorms;
	/* TODO: Handle OOM. */
	model->uvs = malloc(sizeof(float) * 2 * (model->nuvs + 1));

	/* do the calculations */
	for(i = 1; i <= model->nnorms; i++) {
		z = model->norms[3 * i + 0]; /* re-arrange for pole distortion */
		y = model->norms[3 * i + 1];
		x = model->norms[3 * i + 2];

		r = sqrt((x * x) + (y * y));
		rho = sqrt((r * r) + (z * z));

		if(r == 0.0) {
			theta = 0.0;
			phi = 0.0;
		}
		else {
			if(z == 0.0) phi = M_PI / 2.0;
			else phi = acos(z / rho);

			if(y == 0.0) theta = M_PI / 2.0;	/* acos(x / r); */
			else theta = asin(y / r) + (M_PI / 2.0);
		}

		model->uvs[2 * i + 0] = theta / M_PI;
		model->uvs[2 * i + 1] = phi / M_PI;
	}

	/* go through and put texcoord indices in all the tris */
	group = model->groups;

	while(group) {
		for(i = 0; i < group->ntris; i++) {
			GLMtriangle* t = &_GLM_TRI(group->tris[i]);

			t->t_inds[0] = t->n_inds[0];
			t->t_inds[1] = t->n_inds[1];
			t->t_inds[2] = t->n_inds[2];
		}

		group = group->next;
	}

#if 0
	printf("glmSpheremapTexture(): generated %d spheremap texture coordinates\n",
	model->nuvs);
#endif
}

void glmDelete(GLMmodel* model) {
	GLMgroup* group;
	unsigned i;

	if(model->pathname) free(model->pathname);
	if(model->mtllibname) free(model->mtllibname);
	if(model->verts) free(model->verts);
	if(model->norms) free(model->norms);
	if(model->uvs) free(model->uvs);
	if(model->fnorms) free(model->fnorms);
	if(model->tris) free(model->tris);

	if(model->mats) {
		for(i = 0; i < model->nmats; i++)
		free(model->mats[i].name);
	}
	free(model->mats);

	while(model->groups) {
		group = model->groups;
		model->groups = model->groups->next;

		free(group->name);
		free(group->tris);
		free(group);
	}

	free(model);
}

GLMmodel* glmReadOBJFile(const char* filename, void* file) {
	GLMmodel* model;

#if 0
	/* announce the model name */
	printf("Model: %s\n", filename);
#endif

	/* allocate a new model */
	/* TODO: Handle OOM. */
	model = calloc(1, sizeof(GLMmodel));
	model->pathname = _glmStrdup(filename);

	/*
	 * make a first pass through the file to get a count of the number
	 * of verts, norms, uvs & tris
	 */
	_glmFirstPass(model, file);

	/* allocate memory */
	/* TODO: Handle OOM. */
	model->verts = malloc(sizeof(float) * 3 * (model->nverts + 1));
	model->tris = malloc(sizeof(GLMtriangle) * model->ntris);

	if(model->nnorms) {
		/* TODO: Handle OOM. */
		model->norms = malloc(sizeof(float) * 3 * (model->nnorms + 1));
	}

	/* TODO: Handle OOM. */
	if(model->nuvs) model->uvs = malloc(sizeof(float) * 2 * (model->nuvs + 1));

	/* rewind to beginning of file and read in the data this pass */
	rewind(file);

	_glmSecondPass(model, file);

	return model;
}

GLMmodel* glmReadOBJ(const char* filename) {
	GLMmodel* model;
	FILE* file;

	/* open the file */
	file = fopen(filename, "r");
	if(!file) {
		static const char msg[] =
				"glmReadOBJ() failed: can't open data file \"%s\".\n";

		fprintf(stderr, msg, filename);
		exit(1);
	}

	model = glmReadOBJFile(filename, file);

	/* close the file */
	fclose(file);

	return model;
}

void glmWriteOBJ(const GLMmodel* model, const char* filename, GLMflags mode) {
	unsigned i;
	FILE* file;
	GLMgroup* group;

	/* do a bit of warning */
	/* TODO: Better EH. */
	if(mode & GLM_FLAT && !model->fnorms) {
		printf("glmWriteOBJ() warning: flat normal output requested "
				"with no facet norms defined.\n");

		mode &= ~GLM_FLAT;
	}
	if(mode & GLM_SMOOTH && !model->norms) {
		printf("glmWriteOBJ() warning: smooth normal output requested "
				"with no norms defined.\n");

		mode &= ~GLM_SMOOTH;
	}
	if(mode & GLM_TEXTURE && !model->uvs) {
		printf("glmWriteOBJ() warning: texture coordinate output requested "
				"with no texture coordinates defined.\n");

		mode &= ~GLM_TEXTURE;
	}
	if(mode & GLM_FLAT && mode & GLM_SMOOTH) {
		printf("glmWriteOBJ() warning: flat normal output requested "
				"and smooth normal output requested (using smooth).\n");

		mode &= ~GLM_FLAT;
	}

	/* open the file */
	file = fopen(filename, "w");
	if(!file) {
		static const char msg[] =
				"glmWriteOBJ() failed: can't open file \"%s\" to write.\n";

		/* TODO: Better EH. */
		fprintf(stderr, msg, filename);
		exit(1);
	}

	/* spit out a header */
	/* TODO: Move out header write to common proc. */
	fprintf(file, "# \n");
	fprintf(file, "# Wavefront OBJ generated by GLM library\n");
	fprintf(file, "# \n");
	fprintf(file, "# GLM library copyright (C) 1997 by Nate Robins\n");
	fprintf(file, "# email: ndr@pobox.com\n");
	fprintf(file, "# www:   http://www.pobox.com/~ndr\n");
	fprintf(file, "# \n");

	if(mode & GLM_MATERIAL && model->mtllibname) {
		fprintf(file, "\nmtllib %s\n\n", model->mtllibname);
		_glmWriteMTL(model, filename, model->mtllibname);
	}

	/* spit out the verts */
	fprintf(file, "\n");
	fprintf(file, "# %d verts\n", model->nverts);

	for(i = 1; i <= model->nverts; i++) {
		fprintf(file, "v %f %f %f\n",
				model->verts[3 * i + 0],
				model->verts[3 * i + 1],
				model->verts[3 * i + 2]);
	}

	/* spit out the smooth/flat norms */
	if(mode & GLM_SMOOTH) {
		fprintf(file, "\n");
		fprintf(file, "# %d norms\n", model->nnorms);

		for(i = 1; i <= model->nnorms; i++) {
			fprintf(file, "vn %f %f %f\n",
					model->norms[3 * i + 0],
					model->norms[3 * i + 1],
					model->norms[3 * i + 2]);
		}
	}
	else if(mode & GLM_FLAT) {
		fprintf(file, "\n");
		fprintf(file, "# %d norms\n", model->nfnorms);

		for(i = 1; i <= model->nnorms; i++) {
			fprintf(file, "vn %f %f %f\n",
					model->fnorms[3 * i + 0],
					model->fnorms[3 * i + 1],
					model->fnorms[3 * i + 2]);
		}
	}

	/* spit out the texture coordinates */
	if(mode & GLM_TEXTURE) {
		fprintf(file, "\n");
		fprintf(file, "# %p uvs\n", (void*) model->uvs);

		for(i = 1; i <= model->nuvs; i++) {
			fprintf(file, "vt %f %f\n",
					model->uvs[2 * i + 0],
					model->uvs[2 * i + 1]);
		}
	}

	fprintf(file, "\n");
	fprintf(file, "# %d groups\n", model->ngroups);
	fprintf(file, "# %d faces (tris)\n", model->ntris);
	fprintf(file, "\n");

	group = model->groups;
	while(group) {
		fprintf(file, "g %s\n", group->name);

		if(mode & GLM_MATERIAL) {
			static const char msg[] = "usemtl %s\n";
			fprintf(file, msg, model->mats[group->material].name);
		}

		for(i = 0; i < group->ntris; i++) {
			GLMtriangle* t = &_GLM_TRI(group->tris[i]);

			if(mode & GLM_SMOOTH && mode & GLM_TEXTURE) {
				fprintf(file, "f %d/%d/%d %d/%d/%d %d/%d/%d\n",
						t->v_inds[0], t->n_inds[0], t->t_inds[0],
						t->v_inds[1], t->n_inds[1], t->t_inds[1],
						t->v_inds[2], t->n_inds[2], t->t_inds[2]);
			}
			else if(mode & GLM_FLAT && mode & GLM_TEXTURE) {
				fprintf(file, "f %d/%d %d/%d %d/%d\n",
						t->v_inds[0], t->findex,
						t->v_inds[1], t->findex,
						t->v_inds[2], t->findex);
			}
			else if(mode & GLM_TEXTURE) {
				fprintf(file, "f %d/%d %d/%d %d/%d\n",
						t->v_inds[0], t->t_inds[0],
						t->v_inds[1], t->t_inds[1],
						t->v_inds[2], t->t_inds[2]);
			}
			else if(mode & GLM_SMOOTH) {
				fprintf(file, "f %d//%d %d//%d %d//%d\n",
						t->v_inds[0], t->n_inds[0],
						t->v_inds[1], t->n_inds[1],
						t->v_inds[2], t->n_inds[2]);
			}
			else if(mode & GLM_FLAT) {
				fprintf(file, "f %d//%d %d//%d %d//%d\n",
						t->v_inds[0], t->findex,
						t->v_inds[1], t->findex,
						t->v_inds[2], t->findex);
			}
			else {
				fprintf(file, "f %d %d %d\n",
						t->v_inds[0], t->v_inds[1], t->v_inds[2]);
			}
		}

		fprintf(file, "\n");
		group = group->next;
	}

	fclose(file);
}

void glmWeld(GLMmodel* model, float epsilon) {
	float* vectors;
	float* copies;
	unsigned numvectors;
	unsigned i;

	/* verts */
	numvectors = model->nverts;
	vectors = model->verts;
	copies = _glmWeldVectors(vectors, &numvectors, epsilon);

	/* TODO: Verbose output. */
	printf("glmWeld(): %d redundant verts.\n", model->nverts - numvectors - 1);

	for(i = 0; i < model->ntris; i++) {
		GLMtriangle* t = &_GLM_TRI(i);

		t->v_inds[0] = (unsigned) vectors[3 * t->v_inds[0] + 0];
		t->v_inds[1] = (unsigned) vectors[3 * t->v_inds[1] + 0];
		t->v_inds[2] = (unsigned) vectors[3 * t->v_inds[2] + 0];
	}

	/* free space for old verts */
	free(vectors);

	/* allocate space for the new verts */
	model->nverts = numvectors;
	/* TODO: Handle OOM. */
	model->verts = malloc(sizeof(float) * 3 * (model->nverts + 1));

	/* copy the optimized verts into the actual vertex list */
	for(i = 1; i <= model->nverts; i++) {
		model->verts[3 * i + 0] = copies[3 * i + 0];
		model->verts[3 * i + 1] = copies[3 * i + 1];
		model->verts[3 * i + 2] = copies[3 * i + 2];
	}

	free(copies);
}


/* TODO: Use this draw code for our models (?) */
#if 0
void glmDraw(const GLMmodel* model, unsigned mode) {
	unsigned i;
	GLMgroup* group;

	if(mode & GLM_COLOR) glEnable(GL_COLOR_MATERIAL);
	if(mode & GLM_MATERIAL) glDisable(GL_COLOR_MATERIAL);

	glPushMatrix();
	glTranslatef(model->position[0], model->position[1], model->position[2]);

	glBegin(GL_TRIANGLES);
	{
		group = model->groups;

		while(group) {
			if(mode & GLM_MATERIAL) {
				GLMmaterial* mat = &model->mats[group->material];

				glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat->ambient);
				glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat->diffuse);
				glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat->specular);
				glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, mat->shininess);
			}

			if(mode & GLM_COLOR) {
				glColor3fv(model->mats[group->material].diffuse);
			}

			for(i = 0; i < group->ntris; i++) {
				const GLMtriangle* t = &_GLM_TRI(group->tris[i]);

				const float* norms = model->norms;
				const float* uvs = model->uvs;
				const float* fnorms = model->fnorms;
				const float* verts = model->verts;

				if(mode & GLM_FLAT) glNormal3fv(&fnorms[3 * t->findex]);
				if(mode & GLM_SMOOTH) glNormal3fv(&norms[3 * t->n_inds[0]]);
				if(mode & GLM_TEXTURE) glTexCoord2fv(&uvs[2 * t->t_inds[0]]);

				glVertex3fv(&verts[3 * t->v_inds[0]]);

#if 0
				printf("%f %f %f\n",
						verts[3 * t->v_inds[0] + 0],
						verts[3 * t->v_inds[0] + 1],
						verts[3 * t->v_inds[0] + 2]);
#endif

				if(mode & GLM_SMOOTH) glNormal3fv(&norms[3 * t->n_inds[1]]);
				if(mode & GLM_TEXTURE) glTexCoord2fv(&uvs[2 * t->t_inds[1]]);

				glVertex3fv(&verts[3 * t->v_inds[1]]);

#if 0
				printf("%f %f %f\n",
						verts[3 * t->v_inds[1] + 0],
						verts[3 * t->v_inds[1] + 1],
						verts[3 * t->v_inds[1] + 2]);
#endif

				if(mode & GLM_SMOOTH) glNormal3fv(&norms[3 * t->n_inds[2]]);
				if(mode & GLM_TEXTURE) glTexCoord2fv(&uvs[2 * t->t_inds[2]]);

				glVertex3fv(&verts[3 * t->v_inds[2]]);

#if 0
				printf("%f %f %f\n",
						verts[3 * t->v_inds[2] + 0],
						verts[3 * t->v_inds[2] + 1],
						verts[3 * t->v_inds[2] + 2]);
#endif

			}

			group = group->next;
		}
	}
	glEnd();

	glPopMatrix();
}
#endif

#if 0
/* glmList: Generates and returns a display list for the model using
* the mode specified.
*
* model    - initialized GLMmodel structure
* mode     - a bitwise OR of values describing what is to be rendered.
*            GLM_NONE     -  render with only verts
*            GLM_FLAT     -  render with facet norms
*            GLM_SMOOTH   -  render with vertex norms
*            GLM_TEXTURE  -  render with texture coords
*            GLM_COLOR    -  render with colors (color material)
*            GLM_MATERIAL -  render with mats
*            GLM_COLOR and GLM_MATERIAL should not both be specified.
*            GLM_FLAT and GLM_SMOOTH should not both be specified.
*/
unsigned
glmList(GLMmodel* model, unsigned mode)
{
unsigned list;

list = glGenLists(1);
glNewList(list, GL_COMPILE);
glmDraw(model, mode);
glEndList();

return list;
}

/* norms */
if(model->nnorms) {
numvectors = model->nnorms;
vectors    = model->norms;
copies = _glmOptimizeVectors(vectors, &numvectors);

printf("glmOptimize(): %d redundant norms.\n",
model->nnorms - numvectors);

for(i = 0; i < model->ntris; i++) {
_GLM_TRI(i).n_inds[0] = (unsigned)vectors[3 * _GLM_TRI(i).n_inds[0] + 0];
_GLM_TRI(i).n_inds[1] = (unsigned)vectors[3 * _GLM_TRI(i).n_inds[1] + 0];
_GLM_TRI(i).n_inds[2] = (unsigned)vectors[3 * _GLM_TRI(i).n_inds[2] + 0];
}

/* free space for old norms */
free(vectors);

/* allocate space for the new norms */
model->nnorms = numvectors;
model->norms = (float*)malloc(sizeof(float) *
3 * (model->nnorms + 1));

/* copy the optimized verts into the actual vertex list */
for(i = 1; i <= model->nnorms; i++) {
model->norms[3 * i + 0] = copies[3 * i + 0];
model->norms[3 * i + 1] = copies[3 * i + 1];
model->norms[3 * i + 2] = copies[3 * i + 2];
}

free(copies);
}

/* uvs */
if(model->nuvs) {
numvectors = model->nuvs;
vectors    = model->uvs;
copies = _glmOptimizeVectors(vectors, &numvectors);

printf("glmOptimize(): %d redundant uvs.\n",
model->nuvs - numvectors);

for(i = 0; i < model->ntris; i++) {
for(j = 0; j < 3; j++) {
_GLM_TRI(i).t_inds[j] = (unsigned)vectors[3 * _GLM_TRI(i).t_inds[j] + 0];
}
}

/* free space for old uvs */
free(vectors);

/* allocate space for the new uvs */
model->nuvs = numvectors;
model->uvs = (float*)malloc(sizeof(float) *
2 * (model->nuvs + 1));

/* copy the optimized verts into the actual vertex list */
for(i = 1; i <= model->nuvs; i++) {
model->uvs[2 * i + 0] = copies[2 * i + 0];
model->uvs[2 * i + 1] = copies[2 * i + 1];
}

free(copies);
}
#endif

#if 0
/* look for unused verts */
/* look for unused norms */
/* look for unused uvs */
for(i = 1; i <= model->nverts; i++) {
for(j = 0; j < model->ntris; i++) {
if(_GLM_TRI(j).v_inds[0] == i ||
_GLM_TRI(j).v_inds[1] == i ||
_GLM_TRI(j).v_inds[1] == i)
break;
}
}
#endif
