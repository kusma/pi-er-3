//----------------------------------------------------------------------------
//
//                            E L I T E G R O U P
//                             we are very good.
//
// MXMPlay/Win32 v0.9 (based on MXMPlay 1.6)
// (C) 1999 The Artist Formerly Known As Doctor Roole in 1999
//
// -> DirectSound player include files
//
//----------------------------------------------------------------------------

#ifndef __DSIO_H
#define __DSIO_H

#include <windef.h>

#ifdef __cplusplus
extern "C"
{
#endif

// Initialises DirectSound and MXMPlay
// Output format is fixed to 44KHz 16bit stereo for size reasons
// You'll need to start playing with xmpPlay() yourself

// hwnd: handle of your window (use GetForegroundWindow if you don't have one)
// mxm : ptr to MXM data

DWORD __stdcall ds_init(HWND hwnd, void *mxm);



// Shuts down DirectSound and MXMPlay

void __stdcall ds_close();



#ifdef __cplusplus
};
#endif

#endif

