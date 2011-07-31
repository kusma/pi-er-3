#define WIN32_LEAN_AND_MEAN

#include <stdio.h>
#include <windows.h>
#include <math.h>
#include <stdlib.h>
#include "mxmplay.h"
#include "leepra.h"
#include "3d.h"

#ifndef M_PI
#define M_PI 3.14159265358979f
#endif

extern char mxmfile[];

#define WIDTH 512
#define HEIGHT 256

void font_print( unsigned int *buffer, int xpos, int ypos, int width, int height, char *name, char* text, int size, int color ){
	HDC hdc_mem;
	HBITMAP bitmap;
	BITMAPINFO bi;
	LOGFONT lf;
	HFONT font;
	HDC hdc;

	unsigned int* temp = malloc(width*height*sizeof(int));
	int x, y;

	hdc = GetDC( NULL );
	bitmap = CreateCompatibleBitmap(hdc, width, height);
	hdc_mem = CreateCompatibleDC(hdc);
	SelectObject(hdc_mem, bitmap);

	for(y=0;y<height;y++){
		for(x=0;x<width;x++){
			temp[x+y*width] = buffer[x+((height-1)-y)*width];
		}
	}

	lf.lfHeight         = size;
	lf.lfWidth          = 0;
	lf.lfEscapement     = 0;
	lf.lfOrientation    = 0;
	lf.lfWeight         = 0;
	lf.lfItalic         = 0;
	lf.lfUnderline      = 0;
	lf.lfStrikeOut      = 0;
	lf.lfCharSet        = 0;
	lf.lfOutPrecision   = 0;
	lf.lfClipPrecision  = 0;
	lf.lfQuality        = ANTIALIASED_QUALITY;
	lf.lfPitchAndFamily = 0;
	strcpy( lf.lfFaceName, name );
	font = CreateFontIndirect( &lf );

	memset( &bi, 0, sizeof(BITMAPINFO) );
	bi.bmiHeader.biSize=sizeof(bi.bmiHeader);
	bi.bmiHeader.biWidth = width;
	bi.bmiHeader.biHeight = height;
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 32;
	bi.bmiHeader.biCompression = BI_RGB;

	SetDIBits( hdc_mem, bitmap, 0, height, temp, &bi, DIB_RGB_COLORS );

	SetBkMode( hdc_mem, TRANSPARENT ); 
	SetTextColor( hdc_mem, ((color&0xFF)<<16)|((color&0xFF0000)>>16)|(color&0xFF00) );
	SelectObject(hdc_mem, font);
	TextOut(hdc_mem, xpos, ypos, text, strlen(text));

	GetDIBits( hdc_mem, bitmap, 0, height, temp, &bi, DIB_RGB_COLORS );

	DeleteDC(hdc_mem);
	DeleteObject(bitmap);
	DeleteObject(font);

	for(y=0;y<height;y++){
		for(x=0;x<width;x++){
			buffer[x+y*width] = temp[x+((height-1)-y)*width];
		}
	}
	free(temp);
}


unsigned int* load_raw( char* filename, int width, int height ){
	unsigned int* buffer = malloc( sizeof(unsigned int)*width*height);
	FILE *fp = fopen(filename, "rb");
	fread( buffer, sizeof(unsigned int), width*height, fp );
	fclose(fp);
	return buffer;
}

void draw_quad( unsigned int* buffer, int xpos, int ypos, float scale, unsigned int* source){
	unsigned int* dst, *ydst;
	unsigned int* ysrc;
	int y, u, v;
	int startx, starty;
	int stopx, stopy;
	int startu, startv;
	int lenx, leny;
	int delta;
	int iscale = (int)(256.0f*scale);
	if(iscale<1) return;

	startx = xpos-(iscale>>1);
	starty = ypos-(iscale>>1);
	stopx = xpos+(iscale>>1);
	stopy = ypos+(iscale>>1);

	delta = ((255<<8)/iscale);

	startu = 0;
	startv = 0;

	lenx = iscale;
	leny = iscale;

	if( startx<0 ){
		startu = delta*-startx;
		lenx += startx;
		startx = 0;
	}

	if( stopx>WIDTH ){
		lenx -= stopx-WIDTH;
	}

	if( starty<0 ){
		startv = delta*-starty;
		leny += starty;
		starty = 0;
	}

	if( stopy>HEIGHT ){
		leny -= stopy-HEIGHT;
	}

	if( leny<1 ) return;
	if( lenx<1 ) return;

	u = startu;
	v = startv;

	dst = buffer+startx+(starty*WIDTH);

	printf( "%i %i\r", lenx, leny );

	for(y=leny;y;y--){
		u=startu;
		ydst = dst;
		ysrc = source+(v&0x00FF00);

		// gidder ikke asm'e mer nå
		_asm{
			push ebp

			mov edi, dst
			mov esi, ysrc
			mov ecx, lenx
			mov ebx, u
			mov ebp, delta
innerloop:
			mov eax, [edi]
			movd mm0, [edi]
			mov edx, ebx
			shr edx, 8
			and edx, 0xff
			movd mm1, [esi+edx*4]

			add ebx, ebp
			paddusb mm0, mm1

			movd [edi], mm0
			add edi, 4

			dec ecx
			jnz innerloop

			pop ebp
		}
		dst += WIDTH;
		v += delta;
	}
	_asm emms;
}

void draw_subquad( unsigned int* buffer, int xpos, int ypos, float scale, unsigned int* source){
	unsigned int* dst, *ydst;
	unsigned int* ysrc;
	int y, u, v;
	int startx, starty;
	int stopx, stopy;
	int startu, startv;
	int lenx, leny;
	int delta;
	int iscale = (int)(256.0f*scale);
	if(iscale<1) return;

	startx = xpos-(iscale>>1);
	starty = ypos-(iscale>>1);
	stopx = xpos+(iscale>>1);
	stopy = ypos+(iscale>>1);

	delta = ((255<<8)/iscale);

	startu = 0;
	startv = 0;

	lenx = iscale;
	leny = iscale;

	if( startx<0 ){
		startu = delta*-startx;
		lenx += startx;
		startx = 0;
	}

	if( stopx>WIDTH ){
		lenx -= stopx-WIDTH;
	}

	if( starty<0 ){
		startv = delta*-starty;
		leny += starty;
		starty = 0;
	}

	if( stopy>HEIGHT ){
		leny -= stopy-HEIGHT;
	}

	if( leny<1 ) return;
	if( lenx<1 ) return;

	u = startu;
	v = startv;

	dst = buffer+startx+(starty*WIDTH);
		
	for(y=leny;y;y--){
		u=startu;
		ydst = dst;
		ysrc = source+(v&0x00FF00);

		// gidder ikke asm'e mer nå
		_asm{
			push ebp

			mov edi, dst
			mov esi, ysrc
			mov ecx, lenx
			mov ebx, u
			mov ebp, delta
innerloop:
			mov eax, [edi]
			movd mm0, [edi]
			mov edx, ebx
			shr edx, 8
			and edx, 0xff
			movd mm1, [esi+edx*4]

			add ebx, ebp
			psubusb mm0, mm1

			movd [edi], mm0
			add edi, 4

			dec ecx
			jnz innerloop

			pop ebp
		}
		dst += WIDTH;
		v += delta;
	}
	_asm emms;
}


unsigned int* generate_phong( int width, int height ){
	unsigned int* buffer = malloc( sizeof(unsigned int)*width*height);
	unsigned int* bufferp = buffer;
	int x, y;
	unsigned char val;
	float dist;
	float fval;

	for(y=0; y<height; y++){
		for(x=0; x<width; x++){
			dist = (float)sqrt( (x-(width>>1))*(x-(width>>1)) + (y-(height>>1))*(y-(height>>1)) )/width*2;
			if( dist>1 ) dist=1;
			fval = ( (1-dist)*255 );

			val = (int)((int)(fval*fval)>>8);
			//val = fval;
			*bufferp++ = (val<<16)|(val<<8)|val;
		}
	}

	return buffer;
}

void invert( unsigned int* buffer, int width, int height ){
	int count = width*height;
	for(;count;count--){
		*buffer = ~*buffer;
		buffer++;
	}
}


void gamma( unsigned int* buffer, int width, int height, float gamma ){
	int i;
	static unsigned char gammatable[256];
	unsigned char *src = (unsigned char*)buffer;
	for( i =0; i<256; i++ ){
		int val = pow(i,gamma);
		if(val<0) val=0;
		if(val>255) val=255;
		gammatable[i] = val;
	}

	for( i=width*height*4; i; i-- ){
		*src++ = gammatable[*src];
	}
}

LARGE_INTEGER timer_freq, timer_count;
float timemultiplyer;
float starttime, time;
float oldtime;
float synctime;
int row, order;
int sync = 0;
BOOL toggle=FALSE;
float toggletime;

void SyncFunc(unsigned char syncval){
	if(syncval==0xFF){
		toggle=!toggle;
		toggletime = time;
	}else{
		sync = syncval;
		synctime = time;
	}
}


static Vector tetrahedron_vertices[] = {

{0.0f,-0.138514f,-0.141216f},{-0.0388514f,-0.146959f,-0.149662f},{-0.128378f,-0.206081f,-0.208784f},
{0.0f,-0.116976f,-0.160051f},{-0.0364759f,-0.123047f,-0.163318f},{-0.109375f,-0.165541f,-0.20114f},
{0.0f,-0.0878379f,-0.231081f},{-0.0325169f,-0.0899493f,-0.210769f},{-0.0777027f,-0.10473f,-0.188176f},
{0.0f,-0.0574324f,-0.380574f},{-0.0306693f,-0.0576964f,-0.316369f},{-0.0629223f,-0.0595439f,-0.210769f},
{0.0f,-0.027027f,-0.47027f},{-0.0304054f,-0.027027f,-0.380574f},{-0.0608108f,-0.027027f,-0.231081f},
{-0.138514f,-0.136402f,-0.20114f},{-0.179054f,-0.155405f,-0.208784f},{-0.0960198f,-0.063503f,-0.163318f},
{-0.119932f,-0.0658784f,-0.149662f},{-0.0899493f,-0.027027f,-0.160051f},{-0.111486f,-0.027027f,-0.141216f},
{-0.283784f,-0.332348f,-0.335051f},{-0.378378f,-0.405405f,-0.408108f},{-0.229519f,-0.256546f,-0.288857f},
{-0.305321f,-0.310811f,-0.335051f},{0.0388514f,-0.146959f,-0.149662f},{0.0364759f,-0.123047f,-0.163318f},
{0.0325169f,-0.0899493f,-0.210769f},{0.128378f,-0.206081f,-0.208784f},{0.109375f,-0.165541f,-0.20114f},
{0.0777027f,-0.10473f,-0.188176f},{0.283784f,-0.332348f,-0.335051f},{0.229519f,-0.256546f,-0.288857f},
{0.138514f,-0.136402f,-0.20114f},{0.378378f,-0.405405f,-0.408108f},{0.305321f,-0.310811f,-0.335051f},
{0.179054f,-0.155405f,-0.208784f},{0.0629223f,-0.0595439f,-0.210769f},{0.0608108f,-0.027027f,-0.231081f},
{0.0960198f,-0.063503f,-0.163318f},{0.0899493f,-0.027027f,-0.160051f},{0.119932f,-0.0658784f,-0.149662f},
{0.111486f,-0.027027f,-0.141216f},{0.0306693f,-0.0576964f,-0.316369f},{0.0304054f,-0.027027f,-0.380574f},
{0.0f,0.00337837f,-0.380574f},{0.0f,0.0337838f,-0.231081f},{0.0306693f,0.0036423f,-0.316369f},
{0.0325169f,0.0358953f,-0.210769f},{0.0629223f,0.00548985f,-0.210769f},{0.0777027f,0.0506757f,-0.188176f},
{0.0960198f,0.00944889f,-0.163318f},{0.138514f,0.082348f,-0.20114f},{0.119932f,0.0118243f,-0.149662f},
{0.179054f,0.101351f,-0.208784f},{0.109375f,0.111486f,-0.20114f},{0.128378f,0.152027f,-0.208784f},
{0.229519f,0.202492f,-0.288857f},{0.283784f,0.278294f,-0.335051f},{0.305321f,0.256757f,-0.335051f},
{0.378378f,0.351351f,-0.408108f},{0.0f,0.0629223f,-0.160051f},{0.0f,0.0844594f,-0.141216f},
{0.0364759f,0.0689928f,-0.163318f},{0.0388514f,0.0929054f,-0.149662f},{-0.0306693f,0.0036423f,-0.316369f},
{-0.0629223f,0.00548985f,-0.210769f},{-0.0325169f,0.0358953f,-0.210769f},{-0.0777027f,0.0506757f,-0.188176f},
{-0.0364759f,0.0689928f,-0.163318f},{-0.109375f,0.111486f,-0.20114f},{-0.0388514f,0.0929054f,-0.149662f},
{-0.128378f,0.152027f,-0.208784f},{-0.138514f,0.082348f,-0.20114f},{-0.179054f,0.101351f,-0.208784f},
{-0.229519f,0.202492f,-0.288857f},{-0.305321f,0.256757f,-0.335051f},{-0.283784f,0.278294f,-0.335051f},
{-0.378378f,0.351351f,-0.408108f},{-0.0960198f,0.00944889f,-0.163318f},{-0.119932f,0.0118243f,-0.149662f},
{0.0f,-0.157517f,-0.119679f},{0.0f,-0.22973f,-0.0905406f},{-0.0364759f,-0.160737f,-0.12575f},
{-0.0325169f,-0.209037f,-0.092652f},{-0.109375f,-0.19848f,-0.168243f},{-0.0777027f,-0.185811f,-0.107432f},
{-0.229519f,-0.286159f,-0.259248f},{-0.138514f,-0.19848f,-0.139105f},{-0.305321f,-0.332348f,-0.313514f},
{-0.179054f,-0.206081f,-0.158108f},{-0.0629223f,-0.209037f,-0.0622466f},{-0.0608108f,-0.22973f,-0.0297298f},
{-0.0960198f,-0.160737f,-0.0662057f},{-0.0899493f,-0.157517f,-0.0297298f},{-0.119932f,-0.146959f,-0.0685811f},
{-0.111486f,-0.138514f,-0.0297298f},{0.0f,-0.381757f,-0.0601352f},{0.0f,-0.472973f,-0.0297298f},
{-0.0306693f,-0.316459f,-0.0603991f},{-0.0304054f,-0.381757f,-0.0297298f},{0.0f,-0.381757f,0.000675656f},
{0.0f,-0.22973f,0.0310811f},{-0.0306693f,-0.316459f,0.000939592f},{-0.0325169f,-0.209037f,0.0331925f},
{-0.0629223f,-0.209037f,0.00278714f},{-0.0777027f,-0.185811f,0.047973f},{-0.0960198f,-0.160737f,0.00674618f},
{-0.138514f,-0.19848f,0.0796453f},{-0.119932f,-0.146959f,0.0091216f},{-0.179054f,-0.206081f,0.0986486f},
{-0.109375f,-0.19848f,0.108784f},{-0.128378f,-0.206081f,0.149324f},{-0.229519f,-0.286159f,0.199789f},
{-0.283784f,-0.332348f,0.275591f},{-0.305321f,-0.332348f,0.254054f},{-0.378378f,-0.405405f,0.348649f},
{0.0f,-0.157517f,0.0602196f},{0.0f,-0.138514f,0.0817567f},{-0.0364759f,-0.160737f,0.0662901f},
{-0.0388514f,-0.146959f,0.0902027f},{0.0304054f,-0.381757f,-0.0297298f},{0.0608108f,-0.22973f,-0.0297298f},
{0.0306693f,-0.316459f,0.000939592f},{0.0629223f,-0.209037f,0.00278714f},{0.0325169f,-0.209037f,0.0331925f},
{0.0777027f,-0.185811f,0.047973f},{0.0364759f,-0.160737f,0.0662901f},{0.109375f,-0.19848f,0.108784f},
{0.0388514f,-0.146959f,0.0902027f},{0.128378f,-0.206081f,0.149324f},{0.138514f,-0.19848f,0.0796453f},
{0.179054f,-0.206081f,0.0986486f},{0.229519f,-0.286159f,0.199789f},{0.305321f,-0.332348f,0.254054f},
{0.283784f,-0.332348f,0.275591f},{0.378378f,-0.405405f,0.348649f},{0.0899493f,-0.157517f,-0.0297298f},
{0.111486f,-0.138514f,-0.0297298f},{0.0960198f,-0.160737f,0.00674618f},{0.119932f,-0.146959f,0.0091216f},
{0.0364759f,-0.160737f,-0.12575f},{0.109375f,-0.19848f,-0.168243f},{0.0325169f,-0.209037f,-0.092652f},
{0.0777027f,-0.185811f,-0.107432f},{0.0306693f,-0.316459f,-0.0603991f},{0.0629223f,-0.209037f,-0.0622466f},
{0.138514f,-0.19848f,-0.139105f},{0.179054f,-0.206081f,-0.158108f},{0.0960198f,-0.160737f,-0.0662057f},
{0.119932f,-0.146959f,-0.0685811f},{0.229519f,-0.286159f,-0.259248f},{0.305321f,-0.332348f,-0.313514f},
{0.259185f,-0.256546f,-0.259248f},{0.171875f,-0.136402f,-0.168243f},{0.171875f,-0.165541f,-0.139105f},
{0.162162f,-0.10473f,-0.107432f},{0.134924f,-0.123047f,-0.0662057f},{0.191723f,-0.0899493f,-0.0622466f},
{0.132179f,-0.116976f,-0.0297298f},{0.216216f,-0.0878379f,-0.0297298f},{0.191723f,-0.0595439f,-0.092652f},
{0.216216f,-0.027027f,-0.0905406f},{0.317356f,-0.0576964f,-0.0603991f},{0.393581f,-0.027027f,-0.0601352f},
{0.393581f,-0.0574324f,-0.0297298f},{0.5f,-0.027027f,-0.0297298f},{0.134924f,-0.063503f,-0.12575f},
{0.132179f,-0.027027f,-0.119679f},{0.134924f,-0.123047f,0.00674618f},{0.191723f,-0.0899493f,0.00278714f},
{0.171875f,-0.165541f,0.0796453f},{0.162162f,-0.10473f,0.047973f},{0.259185f,-0.256546f,0.199789f},
{0.171875f,-0.136402f,0.108784f},{0.305321f,-0.310811f,0.275591f},{0.179054f,-0.155405f,0.149324f},
{0.191723f,-0.0595439f,0.0331925f},{0.216216f,-0.027027f,0.0310811f},{0.134924f,-0.063503f,0.0662901f},
{0.132179f,-0.027027f,0.0602196f},{0.119932f,-0.0658784f,0.0902027f},{0.111486f,-0.027027f,0.0817567f},
{0.317356f,-0.0576964f,0.000939592f},{0.393581f,-0.027027f,0.000675656f},{0.393581f,0.00337837f,-0.0297298f},
{0.216216f,0.0337838f,-0.0297298f},{0.317356f,0.0036423f,0.000939592f},{0.191723f,0.0358953f,0.00278714f},
{0.191723f,0.00548985f,0.0331925f},{0.162162f,0.0506757f,0.047973f},{0.134924f,0.00944889f,0.0662901f},
{0.171875f,0.082348f,0.108784f},{0.119932f,0.0118243f,0.0902027f},{0.179054f,0.101351f,0.149324f},
{0.171875f,0.111486f,0.0796453f},{0.179054f,0.152027f,0.0986486f},{0.259185f,0.202492f,0.199789f},
{0.305321f,0.278294f,0.254054f},{0.305321f,0.256757f,0.275591f},{0.378378f,0.351351f,0.348649f},
{0.132179f,0.0629223f,-0.0297298f},{0.111486f,0.0844594f,-0.0297298f},{0.134924f,0.0689928f,0.00674618f},
{0.119932f,0.0929054f,0.0091216f},{0.134924f,0.00944889f,-0.12575f},{0.171875f,0.082348f,-0.168243f},
{0.191723f,0.00548985f,-0.092652f},{0.162162f,0.0506757f,-0.107432f},{0.317356f,0.0036423f,-0.0603991f},
{0.191723f,0.0358953f,-0.0622466f},{0.171875f,0.111486f,-0.139105f},{0.179054f,0.152027f,-0.158108f},
{0.134924f,0.0689928f,-0.0662057f},{0.119932f,0.0929054f,-0.0685811f},{0.259185f,0.202492f,-0.259248f},
{0.305321f,0.278294f,-0.313514f},{0.0f,0.105152f,-0.119679f},{-0.0364759f,0.107897f,-0.12575f},
{-0.109375f,0.144848f,-0.168243f},{0.0f,0.189189f,-0.0905406f},{-0.0325169f,0.164696f,-0.092652f},
{-0.0777027f,0.135135f,-0.107432f},{0.0f,0.366554f,-0.0601352f},{-0.0306693f,0.290329f,-0.0603991f},
{-0.0629223f,0.164696f,-0.0622466f},{0.0f,0.472973f,-0.0297298f},{-0.0304054f,0.366554f,-0.0297298f},
{-0.0608108f,0.189189f,-0.0297298f},{-0.138514f,0.144848f,-0.139105f},{-0.179054f,0.152027f,-0.158108f},
{-0.0960198f,0.107897f,-0.0662057f},{-0.119932f,0.0929054f,-0.0685811f},{-0.0899493f,0.105152f,-0.0297298f},
{-0.111486f,0.0844594f,-0.0297298f},{-0.229519f,0.232158f,-0.259248f},{-0.305321f,0.278294f,-0.313514f},
{0.0364759f,0.107897f,-0.12575f},{0.0325169f,0.164696f,-0.092652f},{0.109375f,0.144848f,-0.168243f},
{0.0777027f,0.135135f,-0.107432f},{0.229519f,0.232158f,-0.259248f},{0.138514f,0.144848f,-0.139105f},
{0.0629223f,0.164696f,-0.0622466f},{0.0608108f,0.189189f,-0.0297298f},{0.0960198f,0.107897f,-0.0662057f},
{0.0899493f,0.105152f,-0.0297298f},{0.0306693f,0.290329f,-0.0603991f},{0.0304054f,0.366554f,-0.0297298f},
{0.0f,0.366554f,0.000675656f},{0.0f,0.189189f,0.0310811f},{0.0306693f,0.290329f,0.000939592f},
{0.0325169f,0.164696f,0.0331925f},{0.0629223f,0.164696f,0.00278714f},{0.0777027f,0.135135f,0.047973f},
{0.0960198f,0.107897f,0.00674618f},{0.138514f,0.144848f,0.0796453f},{0.109375f,0.144848f,0.108784f},
{0.128378f,0.152027f,0.149324f},{0.229519f,0.232158f,0.199789f},{0.283784f,0.278294f,0.275591f},
{0.0f,0.105152f,0.0602196f},{0.0f,0.0844594f,0.0817567f},{0.0364759f,0.107897f,0.0662901f},
{0.0388514f,0.0929054f,0.0902027f},{-0.0306693f,0.290329f,0.000939592f},{-0.0629223f,0.164696f,0.00278714f},
{-0.0325169f,0.164696f,0.0331925f},{-0.0777027f,0.135135f,0.047973f},{-0.0364759f,0.107897f,0.0662901f},
{-0.109375f,0.144848f,0.108784f},{-0.0388514f,0.0929054f,0.0902027f},{-0.128378f,0.152027f,0.149324f},
{-0.138514f,0.144848f,0.0796453f},{-0.179054f,0.152027f,0.0986486f},{-0.229519f,0.232158f,0.199789f},
{-0.305321f,0.278294f,0.254054f},{-0.283784f,0.278294f,0.275591f},{-0.378378f,0.351351f,0.348649f},
{-0.0960198f,0.107897f,0.00674618f},{-0.119932f,0.0929054f,0.0091216f},{-0.259185f,-0.256546f,-0.259248f},
{-0.171875f,-0.165541f,-0.139105f},{-0.171875f,-0.136402f,-0.168243f},{-0.162162f,-0.10473f,-0.107432f},
{-0.134924f,-0.063503f,-0.12575f},{-0.191723f,-0.0595439f,-0.092652f},{-0.132179f,-0.027027f,-0.119679f},
{-0.216216f,-0.027027f,-0.0905406f},{-0.191723f,-0.0899493f,-0.0622466f},{-0.216216f,-0.0878379f,-0.0297298f},
{-0.317356f,-0.0576964f,-0.0603991f},{-0.393581f,-0.0574324f,-0.0297298f},{-0.393581f,-0.027027f,-0.0601352f},
{-0.5f,-0.027027f,-0.0297298f},{-0.134924f,-0.123047f,-0.0662057f},{-0.132179f,-0.116976f,-0.0297298f},
{-0.134924f,0.00944889f,-0.12575f},{-0.191723f,0.00548985f,-0.092652f},{-0.171875f,0.082348f,-0.168243f},
{-0.162162f,0.0506757f,-0.107432f},{-0.259185f,0.202492f,-0.259248f},{-0.171875f,0.111486f,-0.139105f},
{-0.191723f,0.0358953f,-0.0622466f},{-0.216216f,0.0337838f,-0.0297298f},{-0.134924f,0.0689928f,-0.0662057f},
{-0.132179f,0.0629223f,-0.0297298f},{-0.317356f,0.0036423f,-0.0603991f},{-0.393581f,0.00337837f,-0.0297298f},
{-0.393581f,-0.027027f,0.000675656f},{-0.216216f,-0.027027f,0.0310811f},{-0.317356f,0.0036423f,0.000939592f},
{-0.191723f,0.00548985f,0.0331925f},{-0.191723f,0.0358953f,0.00278714f},{-0.162162f,0.0506757f,0.047973f},
{-0.134924f,0.0689928f,0.00674618f},{-0.171875f,0.111486f,0.0796453f},{-0.171875f,0.082348f,0.108784f},
{-0.179054f,0.101351f,0.149324f},{-0.259185f,0.202492f,0.199789f},{-0.305321f,0.256757f,0.275591f},
{-0.132179f,-0.027027f,0.0602196f},{-0.111486f,-0.027027f,0.0817567f},{-0.134924f,0.00944889f,0.0662901f},
{-0.119932f,0.0118243f,0.0902027f},{-0.134924f,-0.123047f,0.00674618f},{-0.171875f,-0.165541f,0.0796453f},
{-0.191723f,-0.0899493f,0.00278714f},{-0.162162f,-0.10473f,0.047973f},{-0.317356f,-0.0576964f,0.000939592f},
{-0.191723f,-0.0595439f,0.0331925f},{-0.171875f,-0.136402f,0.108784f},{-0.179054f,-0.155405f,0.149324f},
{-0.134924f,-0.063503f,0.0662901f},{-0.119932f,-0.0658784f,0.0902027f},{-0.259185f,-0.256546f,0.199789f},
{-0.305321f,-0.310811f,0.275591f},{0.0f,-0.116976f,0.102449f},{0.0f,-0.0878379f,0.186486f},
{-0.0364759f,-0.123047f,0.105194f},{-0.0325169f,-0.0899493f,0.161993f},{-0.109375f,-0.165541f,0.142145f},
{-0.0777027f,-0.10473f,0.132432f},{-0.229519f,-0.256546f,0.229455f},{-0.138514f,-0.136402f,0.142145f},
{-0.0629223f,-0.0595439f,0.161993f},{-0.0608108f,-0.027027f,0.186486f},{-0.0960198f,-0.063503f,0.105194f},
{-0.0899493f,-0.027027f,0.102449f},{0.0f,-0.0574324f,0.363851f},{0.0f,-0.027027f,0.47027f},
{-0.0306693f,-0.0576964f,0.287627f},{-0.0304054f,-0.027027f,0.363851f},{0.0f,0.00337837f,0.363851f},
{0.0f,0.0337838f,0.186486f},{-0.0306693f,0.0036423f,0.287627f},{-0.0325169f,0.0358953f,0.161993f},
{-0.0629223f,0.00548985f,0.161993f},{-0.0777027f,0.0506757f,0.132432f},{-0.0960198f,0.00944889f,0.105194f},
{-0.138514f,0.082348f,0.142145f},{-0.109375f,0.111486f,0.142145f},{-0.229519f,0.202492f,0.229455f},
{0.0f,0.0629223f,0.102449f},{-0.0364759f,0.0689928f,0.105194f},{0.0304054f,-0.027027f,0.363851f},
{0.0608108f,-0.027027f,0.186486f},{0.0306693f,0.0036423f,0.287627f},{0.0629223f,0.00548985f,0.161993f},
{0.0325169f,0.0358953f,0.161993f},{0.0777027f,0.0506757f,0.132432f},{0.0364759f,0.0689928f,0.105194f},
{0.109375f,0.111486f,0.142145f},{0.138514f,0.082348f,0.142145f},{0.229519f,0.202492f,0.229455f},
{0.0899493f,-0.027027f,0.102449f},{0.0960198f,0.00944889f,0.105194f},{0.0364759f,-0.123047f,0.105194f},
{0.109375f,-0.165541f,0.142145f},{0.0325169f,-0.0899493f,0.161993f},{0.0777027f,-0.10473f,0.132432f},
{0.0306693f,-0.0576964f,0.287627f},{0.0629223f,-0.0595439f,0.161993f},{0.138514f,-0.136402f,0.142145f},
{0.0960198f,-0.063503f,0.105194f},{0.229519f,-0.256546f,0.229455f}

};

static int tetrahedron_faces[] = {

	3,0,4 , 0,1,4 , 4,1,5 , 5,1,2 , 3,7,6 , 3,4,7 , 4,8,7 ,
	8,4,5 , 6,10,9 , 6,7,10 , 7,11,10 , 8,11,7 , 9,13,12 ,
	10,13,9 , 10,14,13 , 11,14,10 , 8,17,11 , 17,8,15 ,
	18,17,15 , 18,15,16 , 11,19,14 , 17,19,11 , 20,19,17 ,
	18,20,17 , 5,2,23 , 23,2,21 , 24,23,21 , 24,21,22 ,
	15,8,5 , 15,5,23 , 16,15,23 , 16,23,24 , 25,0,26 ,
	0,3,26 , 26,3,27 , 27,3,6 , 25,29,28 , 25,26,29 , 26,30,29 ,
	30,26,27 , 28,32,31 , 28,29,32 , 29,33,32 , 30,33,29 ,
	31,35,34 , 32,35,31 , 32,36,35 , 33,36,32 , 30,39,33 ,
	39,30,37 , 40,39,37 , 40,37,38 , 33,41,36 , 39,41,33 ,
	42,41,39 , 40,42,39 , 27,6,43 , 43,6,9 , 44,43,9 ,
	44,9,12 , 37,30,27 , 37,27,43 , 38,37,43 , 38,43,44 ,
	45,44,12 , 45,47,44 , 46,47,45 , 46,48,47 , 47,38,44 ,
	49,38,47 , 48,49,47 , 48,50,49 , 49,40,38 , 51,40,49 ,
	50,51,49 , 51,50,52 , 42,40,51 , 53,42,51 , 53,51,52 ,
	53,52,54 , 50,55,52 , 52,55,57 , 55,56,57 , 57,56,58 ,
	54,52,57 , 54,57,59 , 57,58,59 , 59,58,60 , 61,48,46 ,
	61,63,48 , 61,62,63 , 62,64,63 , 63,50,48 , 50,63,55 ,
	63,64,55 , 55,64,56 , 13,45,12 , 65,45,13 , 14,65,13 ,
	14,66,65 , 65,46,45 , 67,46,65 , 66,67,65 , 68,67,66 ,
	67,61,46 , 69,61,67 , 68,69,67 , 69,68,70 , 62,61,69 ,
	71,62,69 , 71,69,70 , 71,70,72 , 70,68,73 , 70,73,75 ,
	73,74,75 , 75,74,76 , 72,70,75 , 72,75,77 , 77,75,76 ,
	77,76,78 , 19,66,14 , 19,79,66 , 19,20,79 , 20,80,79 ,
	79,68,66 , 68,79,73 , 79,80,73 , 73,80,74 , 0,83,1 ,
	81,83,0 , 81,84,83 , 82,84,81 , 1,85,2 , 1,83,85 ,
	83,86,85 , 84,86,83 , 2,87,21 , 2,85,87 , 85,88,87 ,
	86,88,85 , 21,89,22 , 87,89,21 , 87,90,89 , 88,90,87 ,
	86,93,88 , 91,93,86 , 91,94,93 , 92,94,91 , 88,95,90 ,
	93,95,88 , 93,96,95 , 94,96,93 , 82,99,84 , 97,99,82 ,
	97,100,99 , 98,100,97 , 84,91,86 , 99,91,84 , 99,92,91 ,
	100,92,99 , 101,100,98 , 101,103,100 , 102,103,101 ,
	102,104,103 , 103,92,100 , 103,105,92 , 104,105,103 ,
	104,106,105 , 105,94,92 , 105,107,94 , 106,107,105 ,
	106,108,107 , 107,96,94 , 107,109,96 , 108,109,107 ,
	108,110,109 , 111,108,106 , 111,113,108 , 112,113,111 ,
	114,113,112 , 113,110,108 , 113,115,110 , 114,115,113 ,
	114,116,115 , 117,104,102 , 117,119,104 , 118,119,117 ,
	118,120,119 , 119,106,104 , 119,111,106 , 120,111,119 ,
	112,111,120 , 121,101,98 , 123,101,121 , 122,123,121 ,
	124,123,122 , 123,102,101 , 125,102,123 , 124,125,123 ,
	126,125,124 , 125,117,102 , 127,117,125 , 126,127,125 ,
	128,127,126 , 127,118,117 , 129,118,127 , 128,129,127 ,
	128,130,129 , 131,128,126 , 133,128,131 , 132,133,131 ,
	134,133,132 , 133,130,128 , 133,135,130 , 134,135,133 ,
	136,135,134 , 137,124,122 , 139,124,137 , 138,139,137 ,
	140,139,138 , 139,126,124 , 131,126,139 , 140,131,139 ,
	132,131,140 , 141,81,0 , 141,0,25 , 141,25,142 , 142,25,28 ,
	143,82,81 , 143,81,141 , 144,143,141 , 144,141,142 ,
	145,97,82 , 145,82,143 , 146,145,143 , 146,143,144 ,
	121,98,97 , 121,97,145 , 122,121,145 , 122,145,146 ,
	149,146,144 , 149,144,147 , 150,149,147 , 150,147,148 ,
	137,122,146 , 137,146,149 , 138,137,149 , 138,149,150 ,
	142,28,151 , 151,28,31 , 152,151,31 , 152,31,34 , 147,144,142 ,
	147,142,151 , 148,147,151 , 148,151,152 , 35,152,34 ,
	153,152,35 , 36,153,35 , 36,154,153 , 153,148,152 , 155,148,153 ,
	154,155,153 , 156,155,154 , 155,150,148 , 157,150,155 ,
	156,157,155 , 158,157,156 , 157,138,150 , 159,138,157 ,
	158,159,157 , 160,159,158 , 161,158,156 , 163,158,161 ,
	162,163,161 , 164,163,162 , 163,160,158 , 165,160,163 ,
	164,165,163 , 166,165,164 , 41,154,36 , 41,167,154 ,
	42,167,41 , 168,167,42 , 167,156,154 , 161,156,167 ,
	168,161,167 , 162,161,168 , 169,140,138 , 159,169,138 ,
	170,169,159 , 160,170,159 , 171,132,140 , 171,140,169 ,
	172,171,169 , 172,169,170 , 173,134,132 , 173,132,171 ,
	174,173,171 , 174,171,172 , 175,136,134 , 175,134,173 ,
	175,173,176 , 176,173,174 , 179,174,172 , 179,172,177 ,
	180,179,177 , 180,177,178 , 176,174,181 , 181,174,179 ,
	182,181,179 , 182,179,180 , 183,170,160 , 165,183,160 ,
	184,183,165 , 166,184,165 , 177,172,170 , 177,170,183 ,
	178,177,183 , 178,183,184 , 185,184,166 , 187,184,185 ,
	186,187,185 , 188,187,186 , 187,178,184 , 189,178,187 ,
	188,189,187 , 190,189,188 , 189,180,178 , 191,180,189 ,
	190,191,189 , 192,191,190 , 191,182,180 , 193,182,191 ,
	192,193,191 , 192,194,193 , 195,192,190 , 197,192,195 ,
	196,197,195 , 198,197,196 , 197,194,192 , 197,199,194 ,
	198,199,197 , 200,199,198 , 201,188,186 , 203,188,201 ,
	202,203,201 , 204,203,202 , 203,190,188 , 195,190,203 ,
	204,195,203 , 196,195,204 , 205,168,42 , 205,42,53 ,
	205,53,206 , 206,53,54 , 207,162,168 , 207,168,205 ,
	208,207,205 , 208,205,206 , 209,164,162 , 209,162,207 ,
	210,209,207 , 210,207,208 , 185,166,164 , 185,164,209 ,
	186,185,209 , 186,209,210 , 213,210,208 , 213,208,211 ,
	214,213,211 , 214,211,212 , 201,186,210 , 201,210,213 ,
	202,201,213 , 202,213,214 , 206,54,215 , 215,54,59 ,
	216,215,59 , 216,59,60 , 211,208,206 , 211,206,215 ,
	212,211,215 , 212,215,216 , 218,217,62 , 218,62,71 ,
	218,71,219 , 219,71,72 , 221,220,217 , 221,217,218 ,
	222,221,218 , 222,218,219 , 224,223,220 , 224,220,221 ,
	225,224,221 , 225,221,222 , 227,226,223 , 227,223,224 ,
	228,227,224 , 228,224,225 , 231,225,222 , 231,222,229 ,
	232,231,229 , 232,229,230 , 233,228,225 , 233,225,231 ,
	234,233,231 , 234,231,232 , 219,72,235 , 235,72,77 ,
	236,235,77 , 236,77,78 , 229,222,219 , 229,219,235 ,
	230,229,235 , 230,235,236 , 62,237,64 , 217,237,62 ,
	217,238,237 , 220,238,217 , 64,239,56 , 64,237,239 ,
	237,240,239 , 238,240,237 , 56,241,58 , 56,239,241 ,
	239,242,241 , 240,242,239 , 58,216,60 , 241,216,58 ,
	241,212,216 , 242,212,241 , 240,245,242 , 243,245,240 ,
	243,246,245 , 244,246,243 , 242,214,212 , 245,214,242 ,
	245,202,214 , 246,202,245 , 220,247,238 , 223,247,220 ,
	223,248,247 , 226,248,223 , 238,243,240 , 247,243,238 ,
	247,244,243 , 248,244,247 , 226,249,248 , 249,251,248 ,
	250,251,249 , 250,252,251 , 248,251,244 , 251,253,244 ,
	252,253,251 , 252,254,253 , 244,253,246 , 253,255,246 ,
	254,255,253 , 254,256,255 , 246,255,202 , 255,204,202 ,
	256,204,255 , 256,196,204 , 257,256,254 , 257,259,256 ,
	258,259,257 , 260,259,258 , 259,196,256 , 259,198,196 ,
	260,198,259 , 260,200,198 , 261,252,250 , 261,263,252 ,
	262,263,261 , 262,264,263 , 263,254,252 , 263,257,254 ,
	264,257,263 , 258,257,264 , 249,226,227 , 265,249,227 ,
	265,227,228 , 266,265,228 , 265,250,249 , 267,250,265 ,
	266,267,265 , 268,267,266 , 267,261,250 , 269,261,267 ,
	268,269,267 , 270,269,268 , 269,262,261 , 271,262,269 ,
	270,271,269 , 270,272,271 , 273,270,268 , 275,270,273 ,
	274,275,273 , 276,275,274 , 275,272,270 , 275,277,272 ,
	276,277,275 , 278,277,276 , 266,228,233 , 279,266,233 ,
	279,233,234 , 280,279,234 , 279,268,266 , 273,268,279 ,
	280,273,279 , 274,273,280 , 89,24,22 , 89,281,24 , 90,281,89 ,
	90,282,281 , 281,16,24 , 283,16,281 , 282,283,281 , 282,284,283 ,
	283,18,16 , 285,18,283 , 284,285,283 , 284,286,285 ,
	285,20,18 , 285,287,20 , 286,287,285 , 286,288,287 ,
	289,286,284 , 289,291,286 , 290,291,289 , 290,292,291 ,
	291,288,286 , 291,293,288 , 292,293,291 , 292,294,293 ,
	95,282,90 , 95,295,282 , 96,295,95 , 96,296,295 , 295,284,282 ,
	295,289,284 , 296,289,295 , 296,290,289 , 20,297,80 ,
	287,297,20 , 287,298,297 , 288,298,287 , 80,299,74 ,
	80,297,299 , 297,300,299 , 298,300,297 , 74,301,76 ,
	74,299,301 , 299,302,301 , 300,302,299 , 76,236,78 ,
	301,236,76 , 301,230,236 , 302,230,301 , 300,305,302 ,
	303,305,300 , 303,306,305 , 304,306,303 , 302,232,230 ,
	305,232,302 , 305,234,232 , 306,234,305 , 288,307,298 ,
	293,307,288 , 293,308,307 , 294,308,293 , 298,303,300 ,
	307,303,298 , 307,304,303 , 308,304,307 , 309,308,294 ,
	309,311,308 , 310,311,309 , 310,312,311 , 311,304,308 ,
	311,313,304 , 312,313,311 , 312,314,313 , 313,306,304 ,
	313,315,306 , 314,315,313 , 314,316,315 , 315,234,306 ,
	315,280,234 , 316,280,315 , 316,274,280 , 317,316,314 ,
	317,319,316 , 318,319,317 , 320,319,318 , 319,274,316 ,
	319,276,274 , 320,276,319 , 320,278,276 , 321,312,310 ,
	321,323,312 , 322,323,321 , 322,324,323 , 323,314,312 ,
	323,317,314 , 324,317,323 , 318,317,324 , 325,296,96 ,
	109,325,96 , 109,326,325 , 110,326,109 , 327,290,296 ,
	325,327,296 , 325,328,327 , 326,328,325 , 329,292,290 ,
	327,329,290 , 327,330,329 , 328,330,327 , 309,294,292 ,
	329,309,292 , 329,310,309 , 330,310,329 , 328,333,330 ,
	331,333,328 , 331,334,333 , 331,332,334 , 330,321,310 ,
	333,321,330 , 333,322,321 , 334,322,333 , 110,335,326 ,
	115,335,110 , 115,336,335 , 116,336,115 , 326,331,328 ,
	335,331,326 , 335,332,331 , 335,336,332 , 339,120,118 ,
	337,339,118 , 340,339,337 , 338,340,337 , 341,112,120 ,
	339,341,120 , 342,341,339 , 340,342,339 , 343,114,112 ,
	343,112,341 , 344,343,341 , 344,341,342 , 336,116,114 ,
	336,114,343 , 336,343,332 , 332,343,344 , 344,342,347 ,
	342,345,347 , 347,345,348 , 345,346,348 , 332,344,334 ,
	344,347,334 , 334,347,322 , 347,348,322 , 338,351,340 ,
	349,351,338 , 349,352,351 , 350,352,349 , 340,345,342 ,
	351,345,340 , 351,346,345 , 351,352,346 , 350,353,352 ,
	352,353,355 , 355,353,354 , 355,354,356 , 352,355,346 ,
	346,355,357 , 355,356,357 , 357,356,358 , 346,357,348 ,
	357,359,348 , 357,358,359 , 358,360,359 , 348,359,322 ,
	359,324,322 , 359,360,324 , 360,318,324 , 361,360,358 ,
	361,362,360 , 272,362,361 , 277,362,272 , 362,318,360 ,
	362,320,318 , 277,320,362 , 277,278,320 , 356,354,363 ,
	364,356,363 , 364,363,262 , 271,364,262 , 358,356,364 ,
	361,358,364 , 361,364,271 , 272,361,271 , 353,350,365 ,
	353,365,367 , 367,365,366 , 367,366,368 , 353,367,354 ,
	354,367,369 , 369,367,368 , 369,368,370 , 354,369,363 ,
	369,371,363 , 369,370,371 , 370,372,371 , 363,371,262 ,
	371,264,262 , 371,372,264 , 372,258,264 , 373,372,370 ,
	374,372,373 , 194,374,373 , 199,374,194 , 374,258,372 ,
	374,260,258 , 199,260,374 , 200,260,199 , 368,366,375 ,
	376,368,375 , 376,375,182 , 193,376,182 , 370,368,376 ,
	373,370,376 , 373,376,193 , 194,373,193 , 377,337,118 ,
	129,377,118 , 378,377,129 , 130,378,129 , 379,338,337 ,
	377,379,337 , 380,379,377 , 378,380,377 , 381,349,338 ,
	381,338,379 , 382,381,379 , 382,379,380 , 365,350,349 ,
	365,349,381 , 365,381,366 , 366,381,382 , 382,380,384 ,
	380,383,384 , 384,383,181 , 383,176,181 , 366,382,375 ,
	382,384,375 , 375,384,182 , 384,181,182 , 130,385,378 ,
	135,385,130 , 135,175,385 , 136,175,135 , 378,383,380 ,
	385,383,378 , 385,176,383 , 385,175,176 

};

Object* make_tetrahedron( unsigned int* lightmap ){
	int i;
	Object* temp = malloc( sizeof(Object) );
	Material* jall = malloc( sizeof(Material) );

	temp->face_count = 768;
	temp->faces = malloc( sizeof(Face)*temp->face_count );
	for(i=0; i<temp->face_count; i++){
		temp->faces[i].vertex[0].index = tetrahedron_faces[i*3];
		temp->faces[i].vertex[1].index = tetrahedron_faces[i*3+1];
		temp->faces[i].vertex[2].index = tetrahedron_faces[i*3+2];
		temp->faces[i].mesh = 0;
		temp->faces[i].smooth = 0;
	}
	temp->vertex_count = 386;
	temp->vertices = malloc( sizeof(Vector)*temp->vertex_count );
	memcpy( temp->vertices, tetrahedron_vertices, sizeof(Vector)*temp->vertex_count );

	temp->normal_count = temp->vertex_count;
	temp->normals = malloc( sizeof(Vector)*temp->normal_count );

	jall->lightmap = lightmap;
	temp->meshes = malloc( sizeof(Mesh) );
	temp->meshes->material = jall;

	generate_normals(temp);

	return temp;
}

__inline Vertex2D transform( Vector v ){
	Vertex2D temp;
	float fov = 90;
	float hpc = (WIDTH>>1)/(float)tan((fov*0.5f)*(M_PI/180));
	float muller = 1.0f/(v.z);
	temp.x = (WIDTH>>1)+(int)(hpc*v.x*muller);
	temp.y = (HEIGHT>>1)+(int)(hpc*v.y*muller);
	temp.z = 0xFFFF/(float)fabs(v.z);
	return temp;
}

void draw_particlefield( unsigned int* target, Vector* particles, int particle_count, Matrix matrix, unsigned int* source, float size ){
	int i;
	Vector *particle = particles;
	for(i=particle_count;i;i--){
		Vector trans1 = matrix_transformvector( matrix, *particle );
		if( trans1.z<-0.1f ){
			Vertex2D trans2 = transform( trans1 );
			Vector v = vector_make(1,0,trans1.z);
			draw_quad( target, trans2.x, trans2.y, v.x*0.025f*size, source );
		}
		particle++;
	}
}

void draw_subparticlefield( unsigned int* target, Vector* particles, int particle_count, Matrix matrix, unsigned int* source, float size ){
	int i;
	Vector *particle = particles;
	for(i=particle_count;i;i--){
		Vector trans1 = matrix_transformvector( matrix, *particle );
		if( trans1.z<-0.1f ){
			Vertex2D trans2 = transform( trans1 );
			Vector v = vector_make(1,0,trans1.z);
			draw_subquad( target, trans2.x, trans2.y, v.x*0.025f*size, source );
		}
		particle++;
	}
}

void print_craplayers( unsigned int **buffers, int buffer_count, char **text, int *pos, int *size, unsigned int color, unsigned int bgcolor ){
	int i;
	unsigned int *temp = (unsigned int*)malloc( sizeof(unsigned int)*WIDTH*HEIGHT );
	unsigned int *mordi = temp;
	memset( temp, bgcolor, sizeof(unsigned int)*WIDTH*HEIGHT );

	for(i=0; i<buffer_count; i++){
		buffers[i] = (unsigned int*)malloc( sizeof(unsigned int)*WIDTH*HEIGHT );
		memcpy( buffers[i], mordi, sizeof(unsigned int)*WIDTH*HEIGHT );

		font_print( buffers[i], pos[i*2], pos[i*2+1], WIDTH, HEIGHT, "Arial black", text[i], size[i], color );

		mordi = buffers[i];
	}
	free( temp );
}

float *mothafuckas;
Vector *vertices;
void init_vortex( Object* object ){
	int i;
	mothafuckas = malloc( sizeof(float)*object->vertex_count );
	vertices = malloc( sizeof(Vector)*object->vertex_count );
	for(i=0;i<object->vertex_count;i++){
		vertices[i] = object->vertices[i];
		mothafuckas[i] = vector_magnitude(	object->vertices[i] );
	}
}

void vortex( Object* destination, Vector rot ){
	int i;
	float prevlen=0;
	Matrix mordi;
	matrix_identity( mordi );
	for(i=0;i<destination->vertex_count;i++){
		float len = mothafuckas[i];
		matrix_identity( mordi );
		matrix_rotate( mordi, vector_make( len*sin(rot.x)*2, len*sin(rot.y)*2, len*sin(rot.z)*2 ) );
		destination->vertices[i] = matrix_transformvector( mordi, vertices[i] );
	}
}

#ifdef _DEBUG
int main(){
#else+
int APIENTRY WinMain (HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmd, int nShow){
#endif
	int i;

	unsigned int *screen;
	unsigned int *text[4];
	unsigned int *partikkel;
	unsigned int *kukkentilmorradi;
	unsigned int *jallatekst;
	unsigned int *pikkentilsatan;
	unsigned int *rrrymden;

	unsigned int *textbuffer[16];
	unsigned int *temp;
	char *textstrings[] = {
		"vi","har","smuglet","øl",
		"vi","har","gjort","mye bøll",
		"pi","er","nøyaktig","3",
		"men", "bryr", "vi oss", "egentlig om det?",
		"messer", "du", "med", "oss",
		"så messer", "du med", "wenche", "foss" 
	};
	int pos[] = {
		10,-5,23,-10,85,-8,375,115,
		10,-5,23,-10,85,-8,30,115,
		300,115,10,-5,0,200,400,0,
		300,115, 10,-5, 0,20, 40,0,
		300,115,10,-5,0,200,400,0,
		300,115, 10,-5, 0,20, 40,0
	};
	int size[] = {
		24,48,32,190,
		24,48,32,130,
		24,48,32,130,
		24,48,32,75,
		24,48,32,130,
		24,48,32,75,
	};
	Object *shit;
	Object *mordi;
	Vector *verts;

	Vector* yo_mama;
	int yo_mama_count;

	float morradi_scale = 0.0f;
	leepra_open("pikk",1);
	dsInit(GetForegroundWindow(),mxmfile);
	xmpSetSyncCallback(SyncFunc);

	screen = (unsigned int*)malloc( sizeof(unsigned int)*WIDTH*HEIGHT );

	partikkel = generate_phong( 256, 256 );
	kukkentilmorradi = (unsigned int*)malloc( sizeof(unsigned int)*WIDTH*HEIGHT );
	text[0] = (unsigned int*)malloc( sizeof(unsigned int)*WIDTH*HEIGHT );
	memset( text[0], 0xff, sizeof(unsigned int)*WIDTH*HEIGHT );
	text[1] = (unsigned int*)malloc( sizeof(unsigned int)*WIDTH*HEIGHT );
	memset( text[1], 0xff, sizeof(unsigned int)*WIDTH*HEIGHT );
	text[2] = (unsigned int*)malloc( sizeof(unsigned int)*WIDTH*HEIGHT );
	memset( text[2], 0xff, sizeof(unsigned int)*WIDTH*HEIGHT );
	text[3] = (unsigned int*)malloc( sizeof(unsigned int)*WIDTH*HEIGHT );
	memset( text[3], 0xff, sizeof(unsigned int)*WIDTH*HEIGHT );
	jallatekst = (unsigned int*)malloc( sizeof(unsigned int)*WIDTH*HEIGHT );

	memset( kukkentilmorradi, 0xff, sizeof(unsigned int)*WIDTH*HEIGHT );

	print_craplayers( textbuffer, 4, textstrings, pos, size, 0xffffff, 0 );
	print_craplayers( (textbuffer+4), 4, (textstrings+4), (pos+8), (size+4), 0xffffff, 0 );
	print_craplayers( (textbuffer+8), 4, (textstrings+8), (pos+16), (size+8), 0, 0xff );
	print_craplayers( (textbuffer+12), 4, (textstrings+12), (pos+24), (size+12), 0, 0xff );
//	print_craplayers( (textbuffer+16), 4, (textstrings+16), (pos+32), (size+16), 0, 0xff );
//	print_craplayers( (textbuffer+20), 4, (textstrings+20), (pos+40), (size+20), 0, 0xff );
	pikkentilsatan = (unsigned int*)malloc( sizeof(unsigned int)*WIDTH*HEIGHT );
	memset( pikkentilsatan, 0xff, sizeof(unsigned int)*WIDTH*HEIGHT );
	font_print( pikkentilsatan, 0, 0, WIDTH, HEIGHT, "Arial black", "fuck bugs, jeg retter dem når jeg er edru", 24, 0 );

	rrrymden = (unsigned int*)malloc( sizeof(unsigned int)*WIDTH*HEIGHT );
	memset( rrrymden, 0, sizeof(unsigned int)*WIDTH*HEIGHT );
	font_print( rrrymden, 0, 0, WIDTH, HEIGHT, "Arial black", "excess - vi spiser polygoner til frokost", 24, 0xffffff );


	font_print( kukkentilmorradi, 256, 140, WIDTH, HEIGHT, "Arial", "kode.kusma", 24, 0 );
	font_print( kukkentilmorradi, 256, 158, WIDTH, HEIGHT, "Arial", "music.gloom", 24, 0 );
	font_print( kukkentilmorradi, 256, 176, WIDTH, HEIGHT, "Arial", "moral.support.zycon", 24, 0 );

	font_print( text[0], -100, -200, WIDTH, HEIGHT, "Arial black", "EX", 630, 0 );
	font_print( text[1], -90, -200, WIDTH, HEIGHT, "Arial black", "CE", 630, 0 );
	font_print( text[2], 10, 0, WIDTH, HEIGHT, "Arial", "All work and no joy, makes kusma a dull boy. All work and no joy, makes kusma a dull boy.", 15, 0 );
	font_print( text[2], 340, -23, WIDTH, HEIGHT, "Arial black", "64k", 130, 0 );
	font_print( text[3], -70, -200, WIDTH, HEIGHT, "Arial black", "SS", 630, 0 );

	font_print( jallatekst, -9,-25, WIDTH, HEIGHT, "Arial black", "PI EQU 3", 75, 0xFFFFFF);
	shit = make_tetrahedron( partikkel );

	yo_mama_count = 100;
	yo_mama = malloc( sizeof(Vector)*yo_mama_count );
	for(i=0;i<yo_mama_count;i++){
		Vector fuckshit;
		fuckshit.x = -250+(rand()%500);
		fuckshit.y = -250+(rand()%500);
		fuckshit.z = -250+(rand()%500);
		yo_mama[i] = vector_scale( vector_normalize( fuckshit ), 100 );
	}

	QueryPerformanceFrequency( &timer_freq );
	timemultiplyer = 1.0f/timer_freq.QuadPart;

	init_vortex( shit );
	xmpPlay(0);
	QueryPerformanceCounter( &timer_count );
	starttime = (float)timer_count.QuadPart*timemultiplyer;

	order == 1;
	while(!GetAsyncKeyState(VK_ESCAPE)&&(order<12)){

		QueryPerformanceCounter( &timer_count );
		oldtime = time;
		time = (float)timer_count.QuadPart*timemultiplyer;

		i = xmpGetPos();

		order = i>>8;
		row = i&0xFF;

		if(order==0){
			memcpy( screen, kukkentilmorradi, sizeof(unsigned int)*WIDTH*HEIGHT );
			i = 0;
			for(i=0;i<64;i++){
				float jall = i*0.6f;
				draw_subquad( screen, (WIDTH>>2)+(int)(sin(time+jall)*100), (HEIGHT>>1)+(int)(sin(time+jall+1.3f)*100), (1.2f+(float)cos(time*-jall*0.1f))*0.15f, partikkel );
			}
		}
		if(order>=1&&order<3){
			memcpy( screen, text[sync%4], sizeof(unsigned int)*WIDTH*HEIGHT );
			if(sync==2){
				i = 0;
				for(i=0;i<64;i++){
					float jall = i*0.6f;
					draw_subquad( screen, (WIDTH>>2)+(int)(sin(time+jall)*100), (HEIGHT>>1)+(int)(sin(time+jall+1.3f)*100), (1.2f+(float)cos(time*-jall*0.1f))*0.15f, partikkel );
				}
			}
		}


// matti sanoo :: vittu saatana perkele
// erik sier :: fitte satan faen

		if(order>=3&&order<11){
			static float amount = 10.f;
			Matrix rotation;
			Matrix translation;
			Matrix scale;
			printf("sync %u\r", sync);
			if(order==3)
				memcpy( screen, textbuffer[sync%4], sizeof(unsigned int)*WIDTH*HEIGHT );
			else if(  order==4 )
				memcpy( screen, textbuffer[4+(sync%4)], sizeof(unsigned int)*WIDTH*HEIGHT );
			else if( order==5 || order==6 ){
				memset( screen, 0xFF, sizeof(unsigned int)*WIDTH*HEIGHT );
				draw_subparticlefield( screen, yo_mama, yo_mama_count, shit->matrix, partikkel, 5.0f );
			}else if( order==7 ){
				memcpy( screen, textbuffer[8+(sync%4)], sizeof(unsigned int)*WIDTH*HEIGHT );
				draw_subparticlefield( screen, yo_mama, yo_mama_count, shit->matrix, partikkel, 5.0f );
			}else if( order==8 ){
				memcpy( screen, textbuffer[12+(sync%4)], sizeof(unsigned int)*WIDTH*HEIGHT );
				draw_subparticlefield( screen, yo_mama, yo_mama_count, shit->matrix, partikkel, 5.0f );
			}else if(order==9 || order==10){
				memcpy( screen, pikkentilsatan, sizeof(unsigned int)*WIDTH*HEIGHT );
				draw_subparticlefield( screen, yo_mama, yo_mama_count, shit->matrix, partikkel, 5.0f );
			}
			matrix_identity( shit->matrix );
			matrix_rotate( rotation, vector_make(time,sin(time),cos(time)));
			matrix_translate( translation, vector_make(cos(time)*0.3,sin(time)*0.3,-2.1f+sin(time/3)*0.5f) );
			matrix_multiply( shit->matrix, translation );
			matrix_multiply( shit->matrix, rotation );

			start_frame();
			if( order>=5 ){
				vortex( shit, vector_make(time,time*0.7444444f, time*0.8f) );
			}
			render_object( shit, screen );

			matrix_scale( scale, vector_make( 1.2f,1.2f,1.2f ) );
			matrix_multiply( shit->matrix, scale );

			if(!toggle)
				draw_particlefield( screen, shit->vertices, shit->vertex_count, shit->matrix, partikkel, amount );

			amount -= (time-oldtime)*0.7f;
		}else if(order==11){
			memcpy( screen, rrrymden, sizeof(unsigned int)*WIDTH*HEIGHT );
		}


//		gamma( screen, WIDTH, HEIGHT, 1.0f+(1+sin(time*16))*0.3f );
//		invert( screen, WIDTH, HEIGHT );

// hvis du messer med oss
// messer du med wenche foss
		leepra_update256( screen );

	}

	xmpStop();
	dsClose();
	leepra_close();
	free( screen );
	free( partikkel );
	free( text[0] );
	free( text[1] );
	free( text[2] );
	free(kukkentilmorradi);
	free( jallatekst );
	for(i=0; i<16; i++){
		free(textbuffer[i]);
	}
	
}








