#include "globalstate.h"

//General variables
HICON hiconMain;
bool isRomOpen = false,isRomSaved = false;
BYTE romBuf[0x800000];
DWORD allocOffs[0x2000],allocSizes[0x2000];
//Variables for levels
BYTE curLevel = 0;
BYTE levelHeader[10];
BYTE screenExits[0x200];

///////////////////////////////
//ROM PATCHING AND ALLOCATION//
///////////////////////////////
void patchRom() {
	//Expand ROM to 8MB and SRAM to 128KB
	romBuf[0x7FBD] = 0x07;
	romBuf[0x7FD7] = 0x0D;
	//Add more midpoint indices
	//TODO
}
bool checkRom() {
	if(romBuf[0x7FD7]==11) {
		patchRom();
		return true;
	} else if(romBuf[0x7FD7]==12) {
		return true;
	}
	return false;
}
DWORD findFreespace(DWORD size) {
	//TODO
}

