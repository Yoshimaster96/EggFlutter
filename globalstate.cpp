#include "globalstate.h"

//General variables
HICON hiconMain;
bool isRomOpen = false,isRomSaved = false;
BYTE romBuf[0x400000];
DWORD allocOffs[0x2000],allocSizes[0x2000];
//Variables for levels
BYTE curLevel = 0;
BYTE levelHeader[10];

///////////////////////////////
//ROM PATCHING AND ALLOCATION//
///////////////////////////////
bool patchRom() {
	//TODO
	return true;
}
bool checkRom() {
	if(romBuf[0x7FD7]==11) {
		return patchRom();
	} else if(romBuf[0x7FD7]==12) {
		return true;
	}
	return false;
}
void initAllocTable() {
	for(int i = 0; i < 0x2000; i++) {
		allocOffs[i] = romBuf[0x3F0000+(i<<3)]|(romBuf[0x3F0001+(i<<3)]<<8)|(romBuf[0x3F0002+(i<<3)]<<16)|(romBuf[0x3F0003+(i<<3)]<<24);
		allocSizes[i] = romBuf[0x3F0004+(i<<3)]|(romBuf[0x3F0005+(i<<3)]<<8)|(romBuf[0x3F0006+(i<<3)]<<16)|(romBuf[0x3F0007+(i<<3)]<<24);
	}
}
DWORD romAlloc(DWORD size) {
	//Goal: Find smallest contiguous block of ROM that is at least `size` bytes
	DWORD smallestBufSize = 0x200000;
	DWORD smallestBufOffset = 0x200000;
	//Check region immediately at start of extended region
	DWORD smallestAllocOffset0 = 0x3F0000;
	for(int i=0; i<0x2000; i++) {
		if(allocOffs[i] && allocOffs[i]<smallestAllocOffset0) {
			smallestAllocOffset0 = allocOffs[i];
		}
	}
	DWORD thisSize0 = smallestAllocOffset0-0x200000;
	if(thisSize0<smallestBufSize && thisSize0>=size) {
		smallestBufSize = thisSize0;
		smallestBufOffset = 0x200000;
	}
    //Check region at end of ROM
	DWORD smallestAllocOffset1 = 0x200000;
	DWORD smallestAllocSize1 = 0;
	for(int i=0; i<0x2000; i++) {
		if(allocOffs[i]>smallestAllocOffset1) {
			smallestAllocOffset1 = allocOffs[i];
			smallestAllocSize1 = allocSizes[i];
		}
	}
	DWORD thisSize1 = 0x3F0000-(smallestAllocOffset1+smallestAllocSize1);
	if(thisSize1<smallestBufSize && thisSize1>=size) {
		smallestBufSize = thisSize1;
		smallestBufOffset = smallestAllocOffset1;
	}
    //Check inbetweens
	//TODO
    //Return result
	return smallestBufOffset;
}
DWORD readRomFile(WORD fileNum,BYTE * data) {
	DWORD offset = allocOffs[fileNum];
	DWORD size = allocSizes[fileNum];
	memcpy(data,&romBuf[offset],size);
	return size;
}
void writeRomFile(WORD fileNum,BYTE * data,DWORD size) {
	DWORD offset = romAlloc(size);
	if(offset != 0) {
		allocOffs[fileNum] = offset;
		allocSizes[fileNum] = size;
		romBuf[0x3F0000+(fileNum<<3)] = offset&0xFF;
		romBuf[0x3F0001+(fileNum<<3)] = (offset>>8)&0xFF;
		romBuf[0x3F0002+(fileNum<<3)] = (offset>>16)&0xFF;
		romBuf[0x3F0003+(fileNum<<3)] = (offset>>24)&0xFF;
		romBuf[0x3F0004+(fileNum<<3)] = size&0xFF;
		romBuf[0x3F0005+(fileNum<<3)] = (size>>8)&0xFF;
		romBuf[0x3F0006+(fileNum<<3)] = (size>>16)&0xFF;
		romBuf[0x3F0007+(fileNum<<3)] = (size>>24)&0xFF;
		memcpy(&romBuf[offset],data,size);
	}
}

