#ifndef GLOBALSTATE_H
#define GLOBALSTATE_H

//Includes
#include "sneslib.h"

//General variables
extern HICON hiconMain;
extern bool isRomOpen,isRomSaved;
extern BYTE romBuf[0x400000];
extern DWORD allocOffs[0x2000],allocSizes[0x2000];
//Variables for levels
extern BYTE curLevel;
extern BYTE levelHeader[10];

//Functions
bool checkRom();
void initAllocTable();
DWORD readRomFile(WORD fileNum,BYTE * data);
void writeRomFile(WORD fileNum,BYTE * data,DWORD size);

#endif

