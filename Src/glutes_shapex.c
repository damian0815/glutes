/*
 * glutes_shape.c
 *
 * Copyright (c) 2005  Joachim Pouderoux   All Rights Reserved.
 * Copyright (c) 2003  David Blythe   All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "../Inc/GLES/glutes.h"
#include "glutes_internal.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

#define PI_ 3.14159265358979323846

#define F1 (1<<16)
#define FM1 ((-1)<<16)


void FGAPIENTRY
glutSolidTorusx(GLfixed ir, GLfixed or, GLint sides, GLint rings)
{
    GLint i, j, k, triangles;
    float s, t, x, y, z, twopi, nx, ny, nz;
	float sin_s, cos_s, cos_t, sin_t, twopi_s, twopi_t;
	float twopi_sides, twopi_rings;
    static GLfixed* v, *n;
    static GLfixed parms[4];
	float irf, orf;
    GLfixed *p, *q;

    if (v) 
	{
		if (parms[0] != ir || parms[1] != or || parms[2] != sides || parms[3] != rings) 
		{
			free(v);
			free(n);
			n = v = 0;

			glVertexPointer(3, GL_FIXED, 0, 0);
			glNormalPointer(GL_FIXED, 0, 0);
		}
    }

    if (!v) 
	{
		irf = _FIXED2FLOAT(ir);
		orf = _FIXED2FLOAT(or);
		parms[0] = ir; 
		parms[1] = or; 
		parms[2] = (GLfixed)sides; 
		parms[3] = (GLfixed)rings;

		p = v = (GLfixed*)malloc(sides*(rings+1)*2*3*sizeof *v);
		q = n = (GLfixed*)malloc(sides*(rings+1)*2*3*sizeof *n);

        twopi = 2.0f * (float)PI_;
		twopi_sides = twopi / sides;
		twopi_rings = twopi / rings;

        for (i = 0; i < sides; i++) 
		{
			for (j = 0; j <= rings; j++) 
			{
				for (k = 1; k >= 0; k--) 
				{
					s = (i + k) % sides + 0.5f;
					t = (float)( j % rings);

					twopi_s = s * twopi_sides;
					twopi_t = t * twopi_rings;

					cos_s = (float)cos(twopi_s);
					sin_s = (float)sin(twopi_s);

					cos_t = (float)cos(twopi_t);
					sin_t = (float)sin(twopi_t);

					x = (orf + irf * cos_s) * cos_t;
					y = (orf + irf * cos_s) * sin_t;
					z = irf * sin_s;

					*p++ = _FLOAT2FIXED(x);
					*p++ = _FLOAT2FIXED(y);
					*p++ = _FLOAT2FIXED(z);

					nx = cos_s * cos_t;
					ny = cos_s * sin_t;
					nz = sin_s;

					*q++ = _FLOAT2FIXED(nx);
					*q++ = _FLOAT2FIXED(ny);
					*q++ = _FLOAT2FIXED(nz);
				}
			}
		}
    }

    glVertexPointer(3, GL_FIXED, 0, v);
    glNormalPointer(GL_FIXED, 0, n);

    glEnableClientState (GL_VERTEX_ARRAY);
    glEnableClientState (GL_NORMAL_ARRAY);

	triangles = (rings + 1) * 2;

    for(i = 0; i < sides; i++)
		glDrawArrays(GL_TRIANGLE_STRIP, triangles * i, triangles);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

}


//-------------------------------------------------------------------------------------------------------------
static GLfixed boxvec[6][3] =
{
   {FM1, 0, 0},
   {0, F1, 0},
   {F1, 0, 0},
   {0, FM1, 0},
   {0, 0, F1},
   {0, 0, FM1}
};

static GLushort boxndex [12][3] = 
{
	{0, 1, 2},
	{0, 2, 3},
	{3, 2, 6},
	{3, 6, 7},
	{6, 4, 7},
	{6, 5, 4},
	{4, 5, 1},
	{4, 1, 0},
	{2, 1, 5},
	{2, 5, 6},
	{3, 7, 4},
	{3, 4, 0}
};

static GLushort wireboxndex[6][4] = 
{
   {0, 1, 2, 3},
   {3, 2, 6, 7},
   {7, 6, 5, 4},
   {4, 5, 1, 0},
   {5, 6, 2, 1},
   {7, 4, 0, 3}
};

											
void FGAPIENTRY		   //x						 y						 z
glutSolidBoxx(GLfixed Width, GLfixed Depth, GLfixed Height)
{
	int i;
	GLfixed v[8][3];

	v[0][0] = v[1][0] = v[2][0] = v[3][0] = - Width/ 2;
	v[4][0] = v[5][0] = v[6][0] = v[7][0] = Width / 2;
	v[0][1] = v[1][1] = v[4][1] = v[5][1] = -Depth / 2;
	v[2][1] = v[3][1] = v[6][1] = v[7][1] = Depth / 2;
	v[0][2] = v[3][2] = v[4][2] = v[7][2] = -Height / 2;
	v[1][2] = v[2][2] = v[5][2] = v[6][2] = Height / 2;

	glVertexPointer(3, GL_FIXED, 0, v);
	glEnableClientState (GL_VERTEX_ARRAY);

	for (i = 0; i < 6; i++)
	{
		glNormal3x(boxvec[i][0], boxvec[i][1], boxvec[i][2]);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, boxndex[i*2]);
	}

	glDisableClientState (GL_VERTEX_ARRAY);
}


void FGAPIENTRY      //x						 y						 z
glutWireBoxx(GLfixed Width, GLfixed Depth, GLfixed Height)
{
	GLfixed v[8][3];
	int i;

	v[0][0] = v[1][0] = v[2][0] = v[3][0] = - Width/ 2;
	v[4][0] = v[5][0] = v[6][0] = v[7][0] = Width / 2;
	v[0][1] = v[1][1] = v[4][1] = v[5][1] = -Depth / 2;
	v[2][1] = v[3][1] = v[6][1] = v[7][1] = Depth / 2;
	v[0][2] = v[3][2] = v[4][2] = v[7][2] = -Height / 2;
	v[1][2] = v[2][2] = v[5][2] = v[6][2] = Height / 2;

	glVertexPointer(3, GL_FIXED, 0, v);
	glEnableClientState (GL_VERTEX_ARRAY);

	for ( i = 0; i < 6; i++)
	{
		glNormal3x(boxvec[i][0], boxvec[i][1], boxvec[i][2]);
		glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_SHORT, wireboxndex[i]);
	}
	glDisableClientState (GL_VERTEX_ARRAY);
}

//-------------------------------------------------------------------------------------------------------------
void PlotSpherePointsx(float radius, GLint stacks, GLint slices, GLfixed* v, GLfixed* n)
{

    GLint i, j; 
	float slicestep, stackstep;
	float tmp;

	stackstep = ((float)PI_) / stacks;
	slicestep = 2.0f * ((float)PI_) / slices;

	for (i = 0; i < stacks; ++i)		
	{
		float a = i * stackstep;
		float b = a + stackstep;

		float s0 =  (float)sin(a);
		float s1 =  (float)sin(b);

		float c0 =  (float)cos(a);
		float c1 =  (float)cos(b);

		for (j = 0; j <= slices; ++j)		
		{
			float c = j * slicestep;
			float x = (float)cos(c);
			float y = (float)sin(c);
			
			tmp = x * s0;
			*n = _FLOAT2FIXED(tmp);
			*v = _FLOAT2FIXED(tmp * radius);

			n++;
			v++;

			tmp = y * s0;
			*n = _FLOAT2FIXED(tmp);
			*v = _FLOAT2FIXED(tmp * radius);

			n++;
			v++;

			tmp = c0;
			*n = _FLOAT2FIXED(tmp);
			*v = _FLOAT2FIXED(tmp * radius);

			n++;
			v++;

			tmp = x * s1;
			*n = _FLOAT2FIXED(tmp);
			*v = _FLOAT2FIXED(tmp * radius);

			n++;
			v++;

			tmp = y * s1;
			*n = _FLOAT2FIXED(tmp);
			*v = _FLOAT2FIXED(tmp * radius);

			n++;
			v++;

			tmp = c1;
			*n = _FLOAT2FIXED(tmp);
			*v = _FLOAT2FIXED(tmp * radius);

			n++;
			v++;

		}
	}
}


void FGAPIENTRY
glutSolidSpherex(GLfixed radius, GLint slices, GLint stacks) 
{
    GLint i, triangles; 
    static GLfixed* v, *n;
    static GLfixed parms[3];
	float radiusf = _FIXED2FLOAT(radius);

    if (v) 
	{
		if (parms[0] != radius || parms[1] != slices || parms[2] != stacks) 
		{
			free(v); 
			free(n);

			n = v = 0;

			glVertexPointer(3, GL_FIXED, 0, 0);
			glNormalPointer(GL_FIXED, 0, 0);
		}
    }

    if (!v) 
	{
		parms[0] = radius; 
		parms[1] = (GLfixed)slices; 
		parms[2] = (GLfixed)stacks;

		v = (GLfixed*)malloc(stacks*(slices+1)*2*3*sizeof *v);
		n = (GLfixed*)malloc(stacks*(slices+1)*2*3*sizeof *n);

		PlotSpherePointsx(radiusf, stacks, slices, v, n);

	}

    glVertexPointer(3, GL_FIXED, 0, v);
    glNormalPointer(GL_FIXED, 0, n);

    glEnableClientState (GL_VERTEX_ARRAY);
    glEnableClientState (GL_NORMAL_ARRAY);

	triangles = (slices + 1) * 2;

    for(i = 0; i < stacks; i++)
		glDrawArrays(GL_TRIANGLE_STRIP, i * triangles, triangles);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

}


void FGAPIENTRY
glutWireSpherex(GLfixed radius, GLint slices, GLint stacks) 
{
    GLint i, j, f; 
    static GLfixed* v, *n;
    static GLfixed parms[3];
	float radiusf = _FIXED2FLOAT(radius);

    if (v) 
	{
		if (parms[0] != radius || parms[1] != slices || parms[2] != stacks) 
		{
			free(v); 
			free(n);

			n = v = 0;

			glVertexPointer(3, GL_FIXED, 0, 0);
			glNormalPointer(GL_FIXED, 0, 0);
		}
    }

    if (!v) 
	{
		parms[0] = radius; 
		parms[1] = (GLfixed)slices; 
		parms[2] = (GLfixed)stacks;

		v = (GLfixed*)malloc(stacks*(slices+1)*2*3*sizeof *v);
		n = (GLfixed*)malloc(stacks*(slices+1)*2*3*sizeof *n);

		PlotSpherePointsx(radiusf, stacks, slices, v, n);

	}

    glVertexPointer(3, GL_FIXED, 0, v);
    glNormalPointer(GL_FIXED, 0, n);

    glEnableClientState (GL_VERTEX_ARRAY);
    glEnableClientState (GL_NORMAL_ARRAY);

    for(i = 0; i < stacks; ++i)
	{
		f = i * (slices + 1);

		for (j = 0; j <= slices; ++j)
			glDrawArrays(GL_LINE_LOOP, (f + j)*2, 3);
	}

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

}

void FGAPIENTRY
glutSolidConex(GLfixed base, GLfixed height, GLint slices, GLint stacks) 
{
    GLint i, j;
    float twopi, nx, ny, nz;
    static GLfixed* v, *n;
    static GLfixed parms[4];
    GLfixed* p, *q;
	float basef = _FIXED2FLOAT(base);
	float heightf = _FIXED2FLOAT(height);

    if (v) 
	{
		if (parms[0] != base || parms[1] != height || parms[2] != slices || parms[3] != stacks) 
		{
			free(v); 
			free(n);

			n = v = 0;

			glVertexPointer(3, GL_FIXED, 0, 0);
			glNormalPointer(GL_FIXED, 0, 0);
		}
    }

    if ((!v) && (height != 0))
	{
		float phi = (float)atan(basef/heightf);
		float cphi = (float)cos(phi);
		float sphi= (float)sin(phi);

		parms[0] = base; 
		parms[1] = height; 
		parms[2] = (GLfixed)slices;
		parms[3] = (GLfixed)stacks;

		p = v = (GLfixed*)malloc(stacks*(slices+1)*2*3*sizeof *v);
		q = n = (GLfixed*)malloc(stacks*(slices+1)*2*3*sizeof *n);

        twopi = 2.0f * ((float)PI_);

        for (i = 0; i < stacks; i++) 
		{
			float r = basef * (1.f - (float) i / stacks);
			float r1 = basef * (1.f - (float) (i + 1.f) / stacks);
			float z = heightf * i /stacks;
			float z1 = heightf * (1.f + i) / stacks;

			for (j = 0; j <= slices; j++) 
			{
				float theta = j == slices ? 0.f : (float) j / slices * twopi;
				float ctheta = (float)cos(theta);
				float stheta = (float)sin(theta);

				nx = ctheta;
				ny = stheta;
				nz = sphi;

				*p++ = _FLOAT2FIXED(r1 * nx);
				*p++ = _FLOAT2FIXED(r1 * ny);
				*p++ = _FLOAT2FIXED(z1);

				*q++ = _FLOAT2FIXED(nx * cphi);
				*q++ = _FLOAT2FIXED(ny * cphi);
				*q++ = _FLOAT2FIXED(nz);

				*p++ = _FLOAT2FIXED(r * nx);
				*p++ = _FLOAT2FIXED(r * ny);
				*p++ = _FLOAT2FIXED(z);

				*q++ = _FLOAT2FIXED(nx * cphi);
				*q++ = _FLOAT2FIXED(ny * cphi);
				*q++ = _FLOAT2FIXED(nz);
			}
		}
    }

    glVertexPointer(3, GL_FIXED, 0, v);
    glNormalPointer(GL_FIXED, 0, n);

    glEnableClientState (GL_VERTEX_ARRAY);
    glEnableClientState (GL_NORMAL_ARRAY);

    for(i = 0; i < stacks; i++)
		glDrawArrays(GL_TRIANGLE_STRIP, i*(slices+1)*2, (slices+1)*2);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
}

void FGAPIENTRY
glutSolidCubex(GLfixed size) 
{
    static GLfixed v[108];	   // 108 =  6*18

    static const int cubev[108] = 
	{
		-1, -1, 1,	/* front */
		 1, -1, 1,
		-1,  1, 1,

		 1, -1, 1,
		 1,  1, 1,
		-1,  1, 1,

		-1,  1, -1,	/* back */
		 1, -1, -1,
		-1, -1, -1,

		-1,  1, -1,
		 1,  1, -1,
		 1, -1, -1,

		-1, -1, -1,	/* left */
		-1, -1,  1,
		-1,  1, -1,

		-1, -1,  1,
		-1,  1,  1,
		-1,  1, -1,

		 1, -1,  1,	/* right */
		 1, -1, -1,
		 1,  1,  1,

		 1, -1, -1,
		 1,  1, -1,
		 1,  1,  1,

		-1,  1,  1,	/* top */
		 1,  1,  1,
		-1,  1, -1,

		 1,  1,  1,
		 1,  1, -1,
		-1,  1, -1,

		-1, -1, -1,	/* bottom */
		 1, -1, -1,
		-1, -1,  1,

		 1, -1, -1,
		 1, -1,  1,
		-1, -1,  1,
    };

    static const GLfixed cuben[108] = 
	{
		0, 0, F1,	/* front */
		0, 0, F1,
		0, 0, F1,

		0, 0, F1,
		0, 0, F1,
		0, 0, F1,

		0, 0, FM1,	/* back */
		0, 0, FM1,
		0, 0, FM1,

		0, 0, FM1,
		0, 0, FM1,
		0, 0, FM1,

		FM1, 0, 0,	/* left */
		FM1, 0, 0,
		FM1, 0, 0,

		FM1, 0, 0,
		FM1, 0, 0,
		FM1, 0, 0,

		F1, 0, 0,	/* right */
		F1, 0, 0,
		F1, 0, 0,

		F1, 0, 0,
		F1, 0, 0,
		F1, 0, 0,

		0, F1, 0,	/* top */
		0, F1, 0,
		0, F1, 0,

		0, F1, 0,
		0, F1, 0,
		0, F1, 0,

		0, FM1, 0,	/* bottom */
		0, FM1, 0,
		0, FM1, 0,

		0, FM1, 0,
		0, FM1, 0,
		0, FM1, 0,
    };

    int i;
	size /= 2;

    for(i = 0; i < 108; i++) 
		v[i] = cubev[i] * size;

    glVertexPointer(3, GL_FIXED, 0, v);
    glNormalPointer(GL_FIXED, 0, cuben);

    glEnableClientState (GL_VERTEX_ARRAY);
    glEnableClientState (GL_NORMAL_ARRAY);

    glDrawArrays(GL_TRIANGLES, 0, 36);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

}

void FGAPIENTRY
glutWireCubex(GLfixed size) 
{
    static GLfixed v[72];

    static const int cubev[72] = 	  // 72 = 3*6*4
	{
		-1, -1, 1,	/* front */
		 1, -1, 1,
		 1,  1, 1,
		-1,  1, 1,

		-1,  1, -1,	/* back */
		 1,  1, -1,
		 1, -1, -1,
		-1, -1, -1,

		-1, -1, -1,	/* left */
		-1, -1,  1,
		-1,  1,  1,
		-1,  1, -1,

		 1, -1,  1,	/* right */
		 1, -1, -1,
		 1,  1, -1,
		 1,  1,  1,

		-1,  1,  1,	/* top */
		 1,  1,  1,
		 1,  1, -1,
		-1,  1, -1,

		-1, -1, -1,	/* bottom */
		 1, -1, -1,
		 1, -1,  1,
		-1, -1,  1,
    };

    static const float cuben[72] = 
	{
		0, 0, F1,	/* front */
		0, 0, F1,
		0, 0, F1,
		0, 0, F1,

		0, 0, FM1,	/* back */
		0, 0, FM1,
		0, 0, FM1,
		0, 0, FM1,

		FM1, 0, 0,	/* left */
		FM1, 0, 0,
		FM1, 0, 0,
		FM1, 0, 0,

		F1, 0, 0,	/* right */
		F1, 0, 0,
		F1, 0, 0,
		F1, 0, 0,

		0, F1, 0,	/* top */
		0, F1, 0,
		0, F1, 0,
		0, F1, 0,

		0, FM1, 0,	/* bottom */
		0, FM1, 0,
		0, FM1, 0,
		0, FM1, 0,
    };

    int i;
	size /= 2;

    for(i = 0; i < 72; i++) 
		v[i] = cubev[i] * size;

    glVertexPointer(3, GL_FIXED, 0, v);
    glNormalPointer(GL_FIXED, 0, cuben);

    glEnableClientState (GL_VERTEX_ARRAY);
    glEnableClientState (GL_NORMAL_ARRAY);

    for(i = 0; i < 6; i++)
		glDrawArrays(GL_LINE_LOOP, 4*i, 4);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

}
