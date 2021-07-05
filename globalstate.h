#ifndef GLOBALSTATE_H
#define GLOBALSTATE_H

//Includes
#include "sneslib.h"

//General variables
extern HICON hiconMain;
extern HINSTANCE hinstMain;
extern bool isRomOpen,isRomSaved;
#ifdef YI_4MB_MODE
extern BYTE romBuf[0x400000];
#else
extern BYTE romBuf[0x800000];
#endif
extern BYTE patchBuf[0x10000];
extern DWORD patchBufSize;
//Variables for levels
extern BYTE curLevel;
extern BYTE levelHeader[10];
extern BYTE screenExits[0x200];

//Functions
bool checkRom();
DWORD findFreespace(DWORD size);

#endif

