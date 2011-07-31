#include <windows.h>
#include <ddraw.h>

#include <stdio.h>

HWND win;
LPDIRECTDRAW ddraw;
LPDIRECTDRAWSURFACE primarybuffer;
LPDIRECTDRAWSURFACE backbuffer;
DDPIXELFORMAT pixelformat;
LPDIRECTDRAWCLIPPER clipper;


typedef HRESULT (WINAPI * DIRECTDRAWCREATE) (GUID FAR *lpGUID,LPDIRECTDRAW FAR *lplpDD,IUnknown FAR *pUnkOuter);
static HMODULE library = 0;

static LRESULT CALLBACK WndProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);

//LPCLIPPER clipper;

int width, height;
BOOL fullscreen = TRUE;
BOOL fakemode = FALSE;
int real_width;
int real_height;

// Converter-stuff
typedef void* (*CONVERTER) (void *dst,const void *src, unsigned int count);
CONVERTER converter;
CONVERTER get_converter( unsigned int bpp, unsigned int r_mask, unsigned int g_mask, unsigned int b_mask );

int leepra_open( char* title, BOOL fullscreen_){
    DIRECTDRAWCREATE DirectDrawCreate;
	HRESULT result;
	RECT rect;
	WNDCLASS wc;
	DDSURFACEDESC desc;
	DDSCAPS caps;
	HINSTANCE inst = GetModuleHandle(NULL);
	fullscreen = fullscreen_;
	width = 512;
	height = 384;
	real_width = width;
	real_height = height;

    library = (HMODULE) LoadLibrary("ddraw.dll");
    if (!library) return FALSE;

    DirectDrawCreate = (DIRECTDRAWCREATE) GetProcAddress(library,"DirectDrawCreate");
    if (!DirectDrawCreate) return FALSE;


	result = DirectDrawCreate( 0, &ddraw, 0 );
	if(FAILED(result)) return FALSE;

	if(fullscreen){
		result = IDirectDraw_SetDisplayMode( ddraw, real_width, real_height, 32 );
		if(FAILED(result)){
			result = IDirectDraw_SetDisplayMode( ddraw, real_width, real_height, 24 );
			if(FAILED(result)){
				result = IDirectDraw_SetDisplayMode( ddraw, real_width, real_height, 16 );
				if(FAILED(result)){
					fakemode = TRUE;
					real_width = 640;
					real_height = 480;
					result = IDirectDraw_SetDisplayMode( ddraw, real_width, real_height, 32 );
					if(FAILED(result)){
						result = IDirectDraw_SetDisplayMode( ddraw, real_width, real_height, 24 );
						if(FAILED(result)){
							result = IDirectDraw_SetDisplayMode( ddraw, real_width, real_height, 16 );
							if(FAILED(result)){
								real_width = width;
								real_height = height;
								fakemode = FALSE;
								fullscreen = FALSE;
							}
						}
					}
				}
			}
		}
	}

	rect.left = 0;
	rect.top = 0;
	rect.right = real_width;
	rect.bottom = real_height;
	if(!fullscreen) AdjustWindowRect( &rect, WS_OVERLAPPEDWINDOW, FALSE );

    wc.style = CS_VREDRAW | CS_HREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = 0;
    wc.hIcon = 0;
    wc.hCursor = LoadCursor(0,IDC_ARROW);
    wc.hbrBackground = 0;
    wc.lpszMenuName = 0;
    wc.lpszClassName = title;
    RegisterClass(&wc);

	win = CreateWindowEx(0, title, title, WS_VISIBLE | WS_OVERLAPPEDWINDOW, 0, 0, rect.right, rect.bottom, 0, 0, inst, 0);

	if( fullscreen ) result = IDirectDraw_SetCooperativeLevel( ddraw, win, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN );
	else result = IDirectDraw_SetCooperativeLevel( ddraw, win, DDSCL_NORMAL );
	if(FAILED(result)) return FALSE;

	memset( &desc, 0, sizeof(DDSURFACEDESC) );
	desc.dwSize = sizeof(DDSURFACEDESC);

	if(fullscreen){
		ShowCursor(0);
		desc.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
		desc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_VIDEOMEMORY | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
		desc.dwBackBufferCount = 1;
	}else{
		desc.dwFlags = DDSD_CAPS;
		desc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	}

	result = IDirectDraw_CreateSurface( ddraw, &desc, &primarybuffer, NULL );
	if( FAILED(result) ) return FALSE;

	if(fullscreen){
		int i;
		caps.dwCaps = DDSCAPS_BACKBUFFER;
		result = IDirectDrawSurface_GetAttachedSurface( primarybuffer, &caps, &backbuffer );
		if( FAILED(result) ) return FALSE;

		for(i=0; i<2; i++){
			DDSURFACEDESC descriptor;
			int pitch, y;
			char* dst;

			IDirectDrawSurface_Restore(primarybuffer);
			descriptor.dwSize = sizeof(DDSURFACEDESC);
			IDirectDrawSurface_Lock(backbuffer,0,&descriptor,DDLOCK_WAIT,0);
			pitch = descriptor.lPitch;

			dst = (char*)descriptor.lpSurface;
			for( y=real_height; y; y-- ){
				memset(dst, 0, real_width*4);
				dst += pitch;
			}
			IDirectDrawSurface_Unlock( backbuffer, descriptor.lpSurface);
			IDirectDrawSurface_Flip( primarybuffer, NULL, DDFLIP_WAIT );
		}
	}else{
		memset( &desc, 0, sizeof(DDSURFACEDESC) );
		desc.dwSize = sizeof(DDSURFACEDESC);
		desc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
		desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
		desc.dwWidth = real_width;
		desc.dwHeight = real_height;

		result = IDirectDraw_CreateSurface( ddraw, &desc, &backbuffer, NULL );
		if(FAILED(result)) return FALSE;

		IDirectDraw_CreateClipper( ddraw, 0, &clipper, 0 );
		IDirectDrawClipper_SetHWnd( clipper, 0,win);
		IDirectDrawSurface_SetClipper( primarybuffer, clipper );
	}

	pixelformat.dwSize = sizeof(DDPIXELFORMAT);
	IDirectDrawSurface_GetPixelFormat( primarybuffer, &pixelformat );
	if (!(pixelformat.dwFlags & DDPF_RGB)) return FALSE;

#ifdef _DEBUG
	printf("%u\n", pixelformat.dwRGBBitCount );
#endif

	converter = get_converter( pixelformat.dwRGBBitCount, pixelformat.dwRBitMask, pixelformat.dwGBitMask, pixelformat.dwBBitMask );
	if(converter == NULL) return FALSE;

#ifndef _DEBUG
	SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
#endif

	return TRUE;
}

void leepra_update( void* data ){
	MSG message;
	DDSURFACEDESC descriptor;
	int pitch;

	while (PeekMessage(&message,win,0,0,PM_REMOVE)){
		TranslateMessage(&message);
		DispatchMessage(&message);
	}

	if(IDirectDrawSurface_IsLost(primarybuffer))IDirectDrawSurface_Restore(primarybuffer);
	if(!fullscreen&&IDirectDrawSurface_IsLost(backbuffer)) IDirectDrawSurface_Restore(backbuffer);

	descriptor.dwSize = sizeof(DDSURFACEDESC);

	IDirectDrawSurface_Lock( backbuffer, 0,&descriptor,DDLOCK_WAIT,0);

	pitch = descriptor.lPitch;

	if(pitch==width && !fakemode){
		converter(data, descriptor.lpSurface, width*height*4);
	}else{
		int y;
		char* dst = (char*)descriptor.lpSurface;
		char* src = (char*)data;
		if(!fakemode){
			for(y=height; y; y--){
				converter(dst, src, width*4);
				src += width*4;
				dst += pitch;
			}
		}else{
			dst += ((480-384)>>1)*pitch;
			for(y=height; y; y--){
				dst += (640-512)<<1;
				converter(dst, src, width*4);
				src += width*4;
				dst += pitch-((640-512)<<1);
			}
		}
	}

	IDirectDrawSurface_Unlock(backbuffer,descriptor.lpSurface);

	if(!fullscreen){
		RECT srcrect;
		RECT destrect;
		POINT point;

		point.x = 0;
		point.y = 0;
		ClientToScreen(win, &point);
		GetClientRect(win, &destrect);
		OffsetRect(&destrect, point.x, point.y);
		SetRect(&srcrect, 0, 0, width, height);
		IDirectDrawSurface_Blt( primarybuffer, &destrect, backbuffer, &srcrect, DDBLT_WAIT, NULL);
	}else{
		IDirectDrawSurface_Flip( primarybuffer, NULL, DDFLIP_WAIT );
	}
}

void leepra_update256( void* data ){
	MSG message;
	DDSURFACEDESC descriptor;
	int pitch;

	while (PeekMessage(&message,win,0,0,PM_REMOVE)){
		TranslateMessage(&message);
		DispatchMessage(&message);
	}

	if(IDirectDrawSurface_IsLost(primarybuffer))IDirectDrawSurface_Restore(primarybuffer);
	if(!fullscreen&&IDirectDrawSurface_IsLost(backbuffer)) IDirectDrawSurface_Restore(backbuffer);

	descriptor.dwSize = sizeof(DDSURFACEDESC);

	IDirectDrawSurface_Lock( backbuffer, 0,&descriptor,DDLOCK_WAIT,0);

	pitch = descriptor.lPitch;

	if(pitch==width && !fakemode){
		char* dst = (char*)descriptor.lpSurface;
		char* src = (char*)data;
		dst += ((384-256)>>1)*pitch;
		converter(dst, src, width*256*4);
	}else{
		int y;
		char* dst = (char*)descriptor.lpSurface;
		char* src = (char*)data;
		if(!fakemode){
			dst += ((384-256)>>1)*pitch;
			for(y=256; y; y--){
				converter(dst, src, width*4);
				src += width*4;
				dst += pitch;
			}
		}else{
			dst += ((480-256)>>1)*pitch;
			for(y=256; y; y--){
				dst += (640-512)<<1;
				converter(dst, src, width*4);
				src += width*4;
				dst += pitch-((640-512)<<1);
			}
		}
	}

	IDirectDrawSurface_Unlock(backbuffer,descriptor.lpSurface);

	if(!fullscreen){
		RECT srcrect;
		RECT destrect;
		POINT point;

		point.x = 0;
		point.y = 0;
		ClientToScreen(win, &point);
		GetClientRect(win, &destrect);
		OffsetRect(&destrect, point.x, point.y);
		SetRect(&srcrect, 0, 0, width, height);
		IDirectDrawSurface_Blt( primarybuffer, &destrect, backbuffer, &srcrect, DDBLT_WAIT, NULL);
	}else{
		IDirectDrawSurface_Flip( primarybuffer, NULL, DDFLIP_WAIT );
	}
}


void leepra_close(){
	if(primarybuffer!=NULL) IDirectDrawSurface_Release(primarybuffer);
	IDirectDraw_Release(ddraw);
	ddraw = NULL;
	DestroyWindow( win );
	ShowCursor( TRUE );
}


// pixelkonvertering
void* convert_argb32_argb888(void *dst, const void *src, unsigned int count);
void* convert_argb32_rgb888(void *dst, const void *src, unsigned int count);
void* convert_argb32_rgb565(void *dst, const void *src, unsigned int count);
void* convert_argb32_rgb555(void *dst, const void *src, unsigned int count);

CONVERTER get_converter( unsigned int bpp, unsigned int r_mask, unsigned int g_mask, unsigned int b_mask ){
	if( (bpp==32) ) return &memcpy;
	if( (bpp==24) ) return &convert_argb32_rgb888;
	if( (bpp==16) ) return &convert_argb32_rgb565;
	if( (bpp==15) ) return &convert_argb32_rgb555;

#ifdef _DEBUG
	printf("bpp:%u rmask:%x gmask:%x bmask:%x\n", bpp, r_mask, g_mask, b_mask);
#endif
	return NULL;
}

void* convert_argb32_argb888(void *dst, const void *src, unsigned int count){
	unsigned int *in = (unsigned int*)src;
	unsigned int *out = (unsigned int*)dst;
	count >>= 2;
	for (;count;count--){
			*(out++) = *(in++);
	}
	return NULL;
}

void* convert_argb32_rgb565(void *dst, const void *src, unsigned int num){
	_asm
	{
		push ebp
		mov esi,src
		mov edi,dst
		mov ecx,num
		xor edx,edx
	lup:
		mov eax,[esi+edx*4]
		mov ebx,eax
		shr ebx,8
		and ebx,1111100000000000b
		mov ebp,eax
		shr ebp,3
		and ebp,0000000000011111b
		add ebx,ebp
		shr eax,5
		and eax,0000011111100000b
		add ebx,eax
		mov [edi+edx*2],bx
		inc edx
		dec ecx
		jnz lup
		pop ebp
	}
	return NULL;
}

void* convert_argb32_rgb555(void *dst, const void *src, unsigned int num){
	_asm
	{
		push ebp
		mov esi,src
		mov edi,dst
		mov ecx,num
		xor edx,edx
	lup:
		mov eax,[esi+edx*4]
		mov ebx,eax
		shr ebx,9
		and ebx,0111110000000000b
		mov ebp,eax
		shr ebp,3
		and ebp,0000000000011111b
		add ebx,ebp
		shr eax,6
		and eax,0000001111100000b
		add ebx,eax
		mov [edi+edx*2],bx
		inc edx
		dec ecx
		jnz lup
		pop ebp
	}
	return NULL;
}


void* convert_argb32_rgb888(void *dst, const void *src, unsigned int num){
	_asm
	{
		push ebp
		mov esi,src
		mov edi,dst
		mov ecx,num
		xor edx,edx
	lup:
		mov eax,[esi]
		mov [edi],eax
		add esi,4
		add edi,3
		dec ecx
		jnz lup
		pop ebp
	}
	return NULL;
}

static LRESULT CALLBACK WndProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam){
	switch (message){
		case WM_PAINT:
			if(!fullscreen){
			RECT srcrect;
			RECT destrect;
			POINT point;

			point.x = 0;
			point.y = 0;
			ClientToScreen(win, &point);
			GetClientRect(win, &destrect);
			OffsetRect(&destrect, point.x, point.y);
			SetRect(&srcrect, 0, 0, width, height);
			IDirectDrawSurface_Blt( primarybuffer, &destrect, backbuffer, &srcrect, DDBLT_WAIT, NULL);
		}else{
			IDirectDrawSurface_Flip( primarybuffer, NULL, DDFLIP_WAIT );
		}
	}
    return DefWindowProc(hWnd,message,wParam,lParam);
}