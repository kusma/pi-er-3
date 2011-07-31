#ifndef __3D_H__
#define __3D_H__

#ifdef WIDTH
#undef WIDTH
#endif
#ifdef HEIGHT
#undef HEIGHT
#endif

#define WIDTH 512
#define HEIGHT 256

typedef struct{
	float x, y, z;
}Vector;

typedef float Matrix[16];

typedef struct{
	int index;
	int normal;
	float u, v;
}Vertex;

typedef struct{
	unsigned int *texture;
	unsigned int *lightmap;
}Material;

typedef struct{
	Vertex vertex[3];
	Vector normal;
	int smooth;
	int mesh;
}Face;

typedef struct{
	Material *material;
}Mesh;

typedef struct{
	Matrix matrix;
	Vector *vertices;
	int vertex_count;
	Vector *normals;
	int normal_count;
	Face *faces;
	int face_count;
	Mesh *meshes;
	int mesh_count;
}Object;

typedef struct{
	int x, y;
	unsigned int z;
	int u,v;
	int u2, v2;
}Vertex2D;

#include <math.h>

//matrix-routines
void matrix_identity( Matrix m );
void matrix_rotate( Matrix m, Vector v );
void matrix_translate( Matrix m, Vector v );
void matrix_scale( Matrix m, Vector v );

Vector matrix_transformvector( Matrix m, Vector v );
Vector matrix_rotatevector( Matrix m, Vector v );

//vector-routines

__inline Vector vector_make( float x, float y, float z ){
	Vector temp;
	temp.x = x;
	temp.y = y;
	temp.z = z;
	return temp;
}

__inline Vector vector_add( Vector v1, Vector v2 ){
	Vector temp;
	temp.x = v1.x + v2.x;
	temp.y = v1.y + v2.y;
	temp.z = v1.z + v2.z;
	return temp;
}

__inline Vector vector_sub( Vector v1, Vector v2 ){
	Vector temp;
	temp.x = v1.x - v2.x;
	temp.y = v1.y - v2.y;
	temp.z = v1.z - v2.z;
	return temp;
}

__inline Vector vector_scale( Vector v, float scalar ){
	Vector temp;
	temp.x = v.x * scalar;
	temp.y = v.y * scalar;
	temp.z = v.z * scalar;
	return temp;
}

__inline float vector_magnitude( Vector v ){
	return (float)sqrt(v.x*v.x+v.y*v.y+v.z*v.z);
}

__inline Vector vector_normalize(Vector v){
	Vector temp;
	float scale = 1.f/vector_magnitude(v);
	temp.x = v.x*scale;
	temp.y = v.y*scale;
	temp.z = v.z*scale;
	return temp;
}

__inline float vector_dotproduct( Vector v1, Vector v2 ){
	return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
}

__inline Vector vector_crossproduct( const Vector v1, const Vector v2 ){
	Vector temp;
	temp.x = (v1.y*v2.z)-(v1.z*v2.y);
	temp.y = (v1.z*v2.x)-(v1.x*v2.z);
	temp.z = (v1.x*v2.y)-(v1.y*v2.x);
	return temp;
}


// lowlevel
void start_frame();
void generate_normals( Object *object );
void render_object( Object *object, unsigned int *buffer );

// lowest level
void flat_triangle( unsigned int *target, Vertex2D v[3], unsigned int color );
void texture_triangle( unsigned int* target, Vertex2D v[3], unsigned int* texture );

#endif