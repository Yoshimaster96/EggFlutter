#ifndef GLOBALSTATE_H
#define GLOBALSTATE_H

//Includes
#include "sneslib.h"

//General variables
extern HICON hiconMain;
extern HINSTANCE hinstMain;
extern bool isRomOpen,isRomSaved;
extern BYTE romBuf[0x800000];
extern BYTE patchBuf[0x200000];
extern DWORD patchBufSize;
//Variables for levels
extern BYTE curLevel;
extern BYTE levelHeader[10];
extern BYTE screenExits[0x200];

//Functions
bool checkRom();
DWORD findFreespace(DWORD size);

#endif

