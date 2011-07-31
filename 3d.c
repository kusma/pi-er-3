#include "3d.h"

#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979f
#endif

#include <assert.h>
#include <stdlib.h>

unsigned int zbuffer[WIDTH*HEIGHT];
float fov = 90;

__inline Vertex2D transform( Vector v ){
	Vertex2D temp;
	float hpc = (WIDTH>>1)/(float)tan((fov*0.5f)*(M_PI/180));
	float muller = 1.0f/(v.z);
	temp.x = (WIDTH>>1)+(int)(hpc*v.x*muller);
	temp.y = (HEIGHT>>1)+(int)(hpc*v.y*muller);
	temp.z = 0xFFFF/(float)fabs(v.z);
	return temp;
}

void generate_normals( Object *object ){
	int i;
	Vector* vert;

	Face* face = object->faces;
	for( i=object->face_count; i; i-- ){
		face->normal = vector_normalize(
							vector_crossproduct(
							vector_sub( object->vertices[face->vertex[2].index], object->vertices[face->vertex[0].index] ),
								vector_sub( object->vertices[face->vertex[1].index], object->vertices[face->vertex[0].index] )
							)
						);
		face++;
	}

	face = object->faces;
	for(i=object->face_count; i; i--){
		object->normals[face->vertex[0].index] =  vector_add( object->normals[face->vertex[0].index], face->normal );
		object->normals[face->vertex[1].index] =  vector_add( object->normals[face->vertex[1].index], face->normal );
		object->normals[face->vertex[2].index] =  vector_add( object->normals[face->vertex[2].index], face->normal );
		face++;
	}

	vert = object->normals;
	for( i=0; i<object->normal_count; i++ ){
		*vert = vector_normalize( *vert );
		vert++;
	}


}

void start_frame(){
/*
	int i;
	float *zbufferp=zbuffer;
	for(i=WIDTH*HEIGHT; i; i--)
		*zbufferp++ = 0.0f;
*/
	memset( zbuffer, 0, sizeof(float)*WIDTH*HEIGHT );
}

__inline void envmap( Vertex2D* v, Vector normal ){
	v->u = ((float)(1.f+normal.x)*127.f);
	v->v = ((float)(1.f+normal.y)*127.f);
}

void render_object( Object *object, unsigned int* buffer ){
	Face* face = object->faces;
	int i;
	for(i=object->face_count; i; i--){
		Vector v3d[3];
		Vector normals[3];
		Vertex2D v[3];
		Vector normal;
		unsigned int color;

		v3d[0] = matrix_transformvector( object->matrix, object->vertices[face->vertex[0].index] );
		v3d[1] = matrix_transformvector( object->matrix, object->vertices[face->vertex[1].index] );
		v3d[2] = matrix_transformvector( object->matrix, object->vertices[face->vertex[2].index] );

		normals[0] = matrix_rotatevector( object->matrix, object->normals[face->vertex[0].index] );
		normals[1] = matrix_rotatevector( object->matrix, object->normals[face->vertex[1].index] );
		normals[2] = matrix_rotatevector( object->matrix, object->normals[face->vertex[2].index] );

		v[0] = transform(v3d[0]);
		v[1] = transform(v3d[1]);
		v[2] = transform(v3d[2]);

		envmap( &v[0], normals[0] );
		envmap( &v[1], normals[1] );
		envmap( &v[2], normals[2] );

		//color = object->meshes[ face->mesh ].material->lightmap[ v[2].u+(v[2].v<<8) ];
		//flat_triangle( buffer, v, color );
		texture_triangle( buffer, v, object->meshes[ face->mesh ].material->lightmap );
		face++;
	}
}

__inline void float_xchg( float *a, float *b ){
	float c = *a;
	*a = *b;
	*b = c;
}

__inline void int_xchg( int *a, int *b ){
	int c = *a;
	*a = *b;
	*b = c;
}

__inline void vertex2d_xchg( Vertex2D *a, Vertex2D *b ){
	Vertex2D c = *a;
	*a = *b;
	*b = c;
}

#define SHIFT 8
__inline void hline( unsigned int* target, int x1, int x2, int y, float z1, float z2, unsigned int color ){
	int offset, counter;
	unsigned int *bufferp;
	float *zbufferp, z, zdelta;
	assert(target);

	if( x1==x2 ) return;

	if(y>=HEIGHT) return;
	assert(!(y>WIDTH));
	if(y<0) return;
	assert(!(y<0));

	if( x2 < x1 ){
		int_xchg( &x1, &x2 );
		float_xchg( &z1, &z2 );
	}

	if( x2<1 ) return;
	if( x1>WIDTH ) return;
	if( x1<1 ) x1 = 1;
	if( x2>WIDTH-2 ) x2 = WIDTH-2;
	if( x1==x2 ) return;

	offset = y*WIDTH+x1;

	zdelta = (z2-z1)/(x2-x1);
	z = z1+(zdelta/2);
	bufferp = target+offset;
	zbufferp = zbuffer+offset;

	for( counter=x2-x1; counter; counter-- ){
		if( *zbufferp<z ){
			*bufferp = color;
			*zbufferp = z;
		}
		z+=zdelta;
		zbufferp++;
		bufferp++;
	}
}

void flat_triangle( unsigned int* target, Vertex2D v[3], unsigned int color ){
	int y, x1, x2, delta1, delta2;
	float z1, z2;
	float zdelta1, zdelta2;
	unsigned int* bufferp;

	assert(target);

	if(v[1].y > v[2].y) vertex2d_xchg(&v[1], &v[2]);
	if(v[0].y > v[2].y) vertex2d_xchg(&v[0], &v[2]);
	if(v[0].y > v[1].y) vertex2d_xchg(&v[0], &v[1]);
	if(v[0].y == v[2].y) return;

	bufferp=target;

	if( v[1].y>v[0].y ){
		int len1 = (v[2].y-v[0].y);
		int len2 = (v[1].y-v[0].y);
		int prediv1 = 0xFFFF/len1;
		int prediv2 = 0xFFFF/len2;
		delta1 = (((v[2].x-v[0].x)<<SHIFT)*prediv1)>>16;
		delta2 = (((v[1].x-v[0].x)<<SHIFT)*prediv2)>>16;
		zdelta1 = (v[2].z-v[0].z)/len1;
		zdelta2 = (v[1].z-v[0].z)/len2;

		x1 = v[0].x<<SHIFT;
		x2 = x1;
		z1 = v[0].z;
		z2 = z1;
		bufferp += v[0].y;
		for( y=v[0].y; y<v[1].y; y++ ){
			hline(target, (x1>>SHIFT), (x2>>SHIFT), y, z1, z2, color );
			z1+=zdelta1;
			z2+=zdelta2;
			x1+=delta1;
			x2+=delta2;
			bufferp+=WIDTH;
		}
	}else{
		delta1 = ((v[2].x-v[0].x)<<SHIFT) / (v[2].y-v[0].y);
		x1 = v[0].x<<SHIFT;
		zdelta1 = (v[2].z-v[0].z)/(v[2].y-v[0].y);
		z1 = v[0].z;
		bufferp += (v[2].y-v[0].y)*WIDTH;
	}

	if( v[2].y > v[1].y){
		delta2 = ((v[2].x-v[1].x)<<SHIFT)/(v[2].y-v[1].y);
		zdelta2 = (v[2].z-v[1].z)/(v[2].y-v[1].y);
		x2 = v[1].x<<SHIFT;
		z2 = v[1].z;
		for( y=v[1].y; y<v[2].y; y++ ){
			hline(target, (x1>>SHIFT), (x2>>SHIFT), y, z1, z2, color );
			z1+=zdelta1;
			z2+=zdelta2;
			x1+=delta1;
			x2+=delta2;
		}
	}
}


void texture_hline( unsigned int* target, int x1, int x2, int y, int z1, int z2, int u1, int u2, int v1, int v2, unsigned int* texture ){
	int counter;
	int offset, precalc, zdelta, udelta, vdelta;
	unsigned int z, *bufferp, *zbufferp;
	if( x1==x2 ) return;

	if(y>(HEIGHT-1)) return;
	assert(!(y>(HEIGHT-1)));
	if(y<0) return;
	assert(!(y<0));

	if( x2 < x1 ){
		int_xchg( &x1, &x2 );
		int_xchg( &z1, &z2 );
		int_xchg( &u1, &u2 );
		int_xchg( &v1, &v2 );
	}

	if( x2<1 ) return;
	if( x1>(WIDTH-2) ) return;
	if( x1<0 ) x1 = 0;
	if( x2>(WIDTH-1) ) x2 = WIDTH-1;
	if( x1==x2 ) return;

	offset = y*WIDTH+x1;

	precalc = 65535/(x2-x1+1);
	zdelta = (z2-z1)/(x2-x1);
	udelta = ((u2-u1)*precalc)>>16;
	vdelta = ((v2-v1)*precalc)>>16;

	z = z1+(zdelta>>1);
	u1 += (udelta>>1);
	v1 += (vdelta>>1);
	bufferp = target+offset;
	zbufferp = zbuffer+offset;

	for( counter=x2-x1; counter; counter-- ){
		if( *zbufferp<(z>>SHIFT) ){
			*bufferp = texture[((u1>>8)&0xFF)+(v1&0xFF00)]|0xFF000000;//u1>>SHIFT;
			*zbufferp = (z>>SHIFT);
		}
		u1+=udelta;
		v1+=vdelta;
		z+=zdelta;
		zbufferp++;
		bufferp++;
	}
}

void texture_triangle( unsigned int* target, Vertex2D v[3], unsigned int* texture ){
	int y, x1, x2, delta1, delta2;
	unsigned int z1, z2;
	int zdelta1, zdelta2;

	unsigned int u1, u2;
	int udelta1, udelta2;

	unsigned int v1, v2;
	int vdelta1, vdelta2;

	if(v[1].y > v[2].y) vertex2d_xchg(&v[1], &v[2]);
	if(v[0].y > v[2].y) vertex2d_xchg(&v[0], &v[2]);
	if(v[0].y > v[1].y) vertex2d_xchg(&v[0], &v[1]);

	if(v[0].y == v[2].y) return;

	if( (int)v[1].y > (int)v[0].y ){
		int precalc1 = 65535/((int)v[2].y - (int)v[0].y+1);
		int precalc2 = 65535/((int)v[1].y - (int)v[0].y+1);
		delta1 =  (((v[2].x - v[0].x)<<SHIFT) * precalc1)>>16;
		delta2 =  (((v[1].x - v[0].x)<<SHIFT) * precalc2)>>16;
		zdelta1 = ((int)(v[2].z - v[0].z)<<SHIFT) / ((int)v[2].y - (int)v[0].y+1);
		zdelta2 = ((int)(v[1].z - v[0].z)<<SHIFT) / ((int)v[1].y - (int)v[0].y+1);
		udelta1 = (((v[2].u - v[0].u)<<SHIFT) * precalc1)>>16;
		udelta2 = (((v[1].u - v[0].u)<<SHIFT) * precalc2)>>16;
		vdelta1 = (((v[2].v - v[0].v)<<SHIFT) * precalc1)>>16;
		vdelta2 = (((v[1].v - v[0].v)<<SHIFT) * precalc2)>>16;

		x1 = (v[0].x<<SHIFT)+(delta1>>1);
		x2 = (v[0].x<<SHIFT)+(delta2>>1);
		z1 = (v[0].z<<SHIFT)+(zdelta1>>1);
		z2 = (v[0].z<<SHIFT)+(zdelta2>>1);
		u1 = (v[0].u<<SHIFT)+(udelta1>>1);
		u2 = (v[0].u<<SHIFT)+(udelta2>>1);
		v1 = (v[0].v<<SHIFT)+(vdelta1>>1);
		v2 = (v[0].v<<SHIFT)+(vdelta2>>1);

		for( y=v[0].y; y<v[1].y; y++ ){
			texture_hline(target, (x1>>SHIFT), (x2>>SHIFT), y, z1, z2,u1,u2,v1,v2, texture);
			u1+=udelta1;
			u2+=udelta2;
			v1+=vdelta1;
			v2+=vdelta2;
			z1+=zdelta1;
			z2+=zdelta2;
			x1+=delta1;
			x2+=delta2;
		}
	}else{
		int precalc1 = 65535/(v[2].y-v[0].y+1);
		delta1 =  (((v[2].x - v[0].x)<<SHIFT) * precalc1)>>16;
		zdelta1 = ((int)(v[2].z - v[0].z)<<SHIFT) / (v[2].y-v[0].y+1);
		udelta1 = (((v[2].u - v[0].u)<<SHIFT) * precalc1)>>16;
		vdelta1 = (((v[2].v - v[0].v)<<SHIFT) * precalc1)>>16;
		x1 = (v[0].x<<SHIFT)+(delta1>>1);
		z1 = (v[0].z<<SHIFT)+(zdelta1>>1);
		u1 = (v[0].u<<SHIFT)+(udelta1>>1);
		v1 = (v[0].v<<SHIFT)+(vdelta1>>1);
	}

	if( (int)v[2].y > (int)v[1].y){
		int precalc2 = 65535/(v[2].y-v[1].y+1);
		delta2 =  (((v[2].x - v[1].x)<<SHIFT)*precalc2)>>16;
		zdelta2 = (((int)v[2].z - (int)v[1].z)<<SHIFT) / (v[2].y-v[1].y+1);
		udelta2 = (((v[2].u - v[1].u)<<SHIFT)*precalc2)>>16;
		vdelta2 = (((v[2].v - v[1].v)<<SHIFT)*precalc2)>>16;

		x2 = (v[1].x<<SHIFT)+(delta2>>1);
		z2 = (v[1].z<<SHIFT)+(zdelta2>>1);
		u2 = (v[1].u<<SHIFT)+(udelta2>>1);
		v2 = (v[1].v<<SHIFT)+(vdelta2>>1);
		for( y=v[1].y; y<v[2].y; y++ ){
			texture_hline(target, (x1>>SHIFT), (x2>>SHIFT), y, z1, z2,u1,u2,v1,v2, texture);
			u1+=udelta1;
			u2+=udelta2;
			v1+=vdelta1;
			v2+=vdelta2;
			z1+=zdelta1;
			z2+=zdelta2;
			x1+=delta1;
			x2+=delta2;
		}
	}
}



void matrix_identity( Matrix m ){
	m[0 ] = 1; m[1 ] = 0; m[2 ] = 0; m[3 ] = 0;
	m[4 ] = 0; m[5 ] = 1; m[6 ] = 0; m[7 ] = 0;
	m[8 ] = 0; m[9 ] = 0; m[10] = 1; m[11] = 0;
	m[12] = 0; m[13] = 0; m[14] = 0; m[15] = 1;
}

void matrix_rotate( Matrix m, Vector v ){
	float cx, cy, cz, sx, sy, sz;
	cx = (float)cos(v.x);
	cy = (float)cos(v.y);
	cz = (float)cos(v.z);
	sx = (float)sin(v.x);
	sy = (float)sin(v.y);
	sz = (float)sin(v.z);
	m[0 ] = cy*cz;			m[1 ] = cy*sz;			m[2 ] = -sy;	m[3 ] = 0;
	m[4 ] = sx*sy*cz-cx*sz;	m[5 ] = sx*sy*sz+cx*cz;	m[6 ] = sx*cy;	m[7 ] = 0;
	m[8 ] = cx*sy*cz+sx*sz;	m[9 ] = cx*sy*sz-sx*cz;	m[10] = cx*cy;	m[11] = 0;
	m[12] = 0;				m[13] = 0;				m[14] = 0;		m[15] = 1;
}

void matrix_translate( Matrix m, Vector v ){
	m[0 ] = 1;		m[1 ] = 0;		m[2 ] = 0;		m[3 ] = 0;
	m[4 ] = 0;		m[5 ] = 1;		m[6 ] = 0;		m[7 ] = 0;
	m[8 ] = 0;		m[9 ] = 0;		m[10] = 1;		m[11] = 0;
	m[12] = v.x;	m[13] = v.y;	m[14] = v.z;	m[15] = 1;
}

void matrix_scale( Matrix m, Vector v ){
	m[0 ] = v.x;	m[1 ] = 0;		m[2 ] = 0;		m[3 ] = 0;
	m[4 ] = 0;		m[5 ] = v.y;	m[6 ] = 0;		m[7 ] = 0;
	m[8 ] = 0;		m[9 ] = 0;		m[10] = v.z;	m[11] = 0;
	m[12] = 0;		m[13] = 0;		m[14] = 0;		m[15] = 1;
}

Vector matrix_transformvector( Matrix m, Vector v ){
	Vector temp;
	temp.x = (v.x*m[0]) + (v.y*m[4]) + (v.z*m[8]) + m[12];
	temp.y = (v.x*m[1]) + (v.y*m[5]) + (v.z*m[9]) + m[13];
	temp.z = (v.x*m[2]) + (v.y*m[6]) + (v.z*m[10]) + m[14];
	return temp;
}

Vector matrix_rotatevector( Matrix m, Vector v ){
	Vector temp;
	temp.x = (v.x*m[0]) + (v.y*m[4]) + (v.z*m[8]);
	temp.y = (v.x*m[1]) + (v.y*m[5]) + (v.z*m[9]);
	temp.z = (v.x*m[2]) + (v.y*m[6]) + (v.z*m[10]);
	return temp;
}

void matrix_multiply(Matrix m1, Matrix m2){
	int i, j, counter;
	float temp[16];
	for( i=0; i<4; i++ )
		for ( j=0; j<4; j++ )
			temp[i+(j<<2)] = (m1[i]*m2[(j<<2)])
							+(m1[i+(1<<2)]*m2[1+(j<<2)])
							+(m1[i+(2<<2)]*m2[2+(j<<2)])
							+(m1[i+(3<<2)]*m2[3+(j<<2)]);

	for( counter=0; counter<(4*4); counter++ ) m1[counter] = temp[counter];
}

