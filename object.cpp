#include "object.h"

HWND hwndObject;
bool wvisObject = false;
level_object_data_ctx_t objectContexts[2];
int curObjCtx = 0;

//////////////////////////////
//OBJECT OCCUPY REGISTRATION//
//////////////////////////////
void addObjectTile(object_t * o,WORD tile,int offset) {
	if(offset >= 0 && offset < 0x8000) {
		//Store in object
		o->occupiedTiles.push_back(offset);
		//Set tile in tilemap for current context
		objectContexts[curObjCtx].tilemap[offset] = tile;
		objectContexts[curObjCtx].assocObjects[offset].push_back(o);
	}
}

////////////////////////////
//OBJECT DRAWING FUNCTIONS//
////////////////////////////
int noiseTilemap[0x8000];
WORD tilesetBuffer[0x4000];

//Helper function for drawing text
void drawObjectText(object_t * o,char * text) {
	//TODO
}

//Helper functions (for calculating tile offsets)
inline int getBaseMap16Offset(object_t * o) {
	BYTE hi = o->data[1];
	BYTE lo = o->data[2];
	return (lo&0xF)|((hi&0xF)<<4)|((lo&0xF0)<<4)|((hi&0xF0)<<8);
}
inline int offsetMap16Right(int curOffs) {
	return curOffs+1;
}
inline int offsetMap16Left(int curOffs) {
	return curOffs-1;
}
inline int offsetMap16Down(int curOffs) {
	return curOffs+0x100;
}
inline int offsetMap16Up(int curOffs) {
	return curOffs-0x100;
}

//Overlapped tile handlers
inline WORD overlapTile_grassTrees(int offs,WORD tile) {
	if(tile==0) return 0;
	WORD orig = objectContexts[curObjCtx].tilemap[offs];
	if(tile==0x3D9F && orig==0x3D72) return 0x3DA9;
	else if(tile==0x3DA0 && orig==0x3D71) return 0x3DA8;
	else return tile;
}
inline void overlapTile_stdLedgeMid(object_t * o,int mtOff,WORD orig) {
	int mtOff2 = offsetMap16Left(mtOff);
	WORD orig2 = objectContexts[curObjCtx].tilemap[mtOff2];
	if(orig2==tilesetBuffer[0x1CAC>>1]) {
		addObjectTile(o,tilesetBuffer[0x1CCA>>1],mtOff2);
		mtOff2 = offsetMap16Down(mtOff2);
		addObjectTile(o,tilesetBuffer[0x1CC4>>1],mtOff2);
	}
	mtOff2 = offsetMap16Right(mtOff);
	orig2 = objectContexts[curObjCtx].tilemap[mtOff2];
	if(orig2==tilesetBuffer[0x1CAC>>1]) {
		addObjectTile(o,tilesetBuffer[0x1CCC>>1],mtOff2);
		mtOff2 = offsetMap16Down(mtOff2);
		addObjectTile(o,tilesetBuffer[0x1CC2>>1],mtOff2);
	}
	int offset = noiseTilemap[mtOff]&0x0E;
	int tileRef = romBuf[0x098121+offset]|(romBuf[0x098122+offset]<<8);
	addObjectTile(o,tilesetBuffer[tileRef>>1],mtOff);
}
inline void overlapTile_stdLedgeBottom(object_t * o,int mtOff,WORD orig) {
	if(orig==tilesetBuffer[0x1CC2>>1] || orig==tilesetBuffer[0x1CC4>>1] ||
	orig==tilesetBuffer[0x1CCA>>1] || orig==tilesetBuffer[0x1CCC>>1]) return;
	if(orig==tilesetBuffer[0x1CB6>>1] || orig==tilesetBuffer[0x1CB8>>1]) {
		int mtOff2 = offsetMap16Left(mtOff);
		WORD orig2 = objectContexts[curObjCtx].tilemap[mtOff2];
		if(orig2==tilesetBuffer[0x1CE8>>1] || orig2==tilesetBuffer[0x1CEA>>1] ||
		(orig2>=tilesetBuffer[0x1CAE>>1]) && orig2<tilesetBuffer[0x1CB6>>1]) {
			addObjectTile(o,tilesetBuffer[0x1CCC>>1],mtOff);
			mtOff2 = offsetMap16Down(mtOff);
			addObjectTile(o,tilesetBuffer[0x1CC2>>1],mtOff2);
			return;
		}
		mtOff2 = offsetMap16Right(mtOff);
		orig2 = objectContexts[curObjCtx].tilemap[mtOff2];
		if(orig2==tilesetBuffer[0x1CE8>>1] || orig2==tilesetBuffer[0x1CEA>>1] ||
		(orig2>=tilesetBuffer[0x1CAE>>1]) && orig2<tilesetBuffer[0x1CB6>>1]) {
			addObjectTile(o,tilesetBuffer[0x1CCA>>1],mtOff);
			mtOff2 = offsetMap16Down(mtOff);
			addObjectTile(o,tilesetBuffer[0x1CC4>>1],mtOff2);
			return;
		}
		addObjectTile(o,tilesetBuffer[0x1CAC>>1],mtOff);
		return;
	}
	int offset = noiseTilemap[mtOff]&0x0E;
	int tileRef = romBuf[0x098121+offset]|(romBuf[0x098122+offset]<<8);
	addObjectTile(o,tilesetBuffer[tileRef>>1],mtOff);
}

//Extended objects
//Jungle leaf
void drawObject_extended00(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	for(int j=0; j<3; j++) {
		for(int i=0; i<2; i++) {
			int offset = (o->data[3]*12)+(j<<2)+(i<<1);
			WORD tile = romBuf[0x0923F6+offset]|(romBuf[0x0923F7+offset]<<8);
			if(tile) addObjectTile(o,tile,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Small jungle leaf
void drawObject_extended04(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	for(int j=0; j<3; j++) {
		int offset = ((o->data[3]-0x04)*6)+(j<<1);
		WORD tile = romBuf[0x092426+offset]|(romBuf[0x092427+offset]<<8);
		if(tile) addObjectTile(o,tile,mtOff);
		mtOff = offsetMap16Down(mtOff);
	}
}
//3 jungle leaves
void drawObject_extended08(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	for(int j=0; j<3; j++) {
		for(int i=0; i<3; i++) {
			int offset = (j*6)+(i<<1);
			WORD tile = romBuf[0x09243E +offset]|(romBuf[0x09243F+offset]<<8);
			if(tile) addObjectTile(o,tile,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//2 jungle leaves
void drawObject_extended09(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	for(int j=0; j<3; j++) {
		for(int i=0; i<2; i++) {
			int offset = (j*6)+(i<<1);
			WORD tile = romBuf[0x092450+offset]|(romBuf[0x092451+offset]<<8);
			addObjectTile(o,tile,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Bush on mud
void drawObject_extended0A(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	for(int j=0; j<2; j++) {
		for(int i=0; i<2; i++) {
			int offset = ((o->data[3]-0x0A)<<3)+(j<<2)+(i<<1);
			WORD tile = romBuf[0x0924B5+offset]|(romBuf[0x0924B6+offset]<<8);
			addObjectTile(o,tile,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Vine on mud
void drawObject_extended0C(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	for(int j=0; j<4; j++) {
		int offset = (j<<1);
		WORD tile = romBuf[0x0924E4+offset]|(romBuf[0x0924E5+offset]<<8);
		addObjectTile(o,tile,mtOff);
		mtOff = offsetMap16Right(mtOff);
		tile++;
		addObjectTile(o,tile,mtOff);
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Kamek's room piece
void drawObject_extended0D(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	for(int j=0; j<0x10; j++) {
		for(int i=0; i<8; i++) {
			int offset = ((o->data[3]-0x0D)<<7)+(j<<3)+i;
			int tileRef = romBuf[0x09250B+offset];
			if(tileRef!=0x5B) {
				if(tileRef<0x46) addObjectTile(o,0x9684+tileRef,mtOff);
				else if(tileRef<0x54) addObjectTile(o,0x9D46+tileRef,mtOff);
				else addObjectTile(o,0x9D30+tileRef,mtOff);
			}
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Lily tile
void drawObject_extended0F(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	addObjectTile(o,0x00B6,mtOff);
}
//Prince Froggy's stomach background
void drawObject_extended10(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	for(int j=0; j<0x20; j++) {
		for(int i=0; i<0x10; i++) {
			int offset = (i&1)+((j&1)<<1);
			WORD tile = 0x84C2+offset;
			addObjectTile(o,tile,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Seesaw holder
void drawObject_extended11(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	addObjectTile(o,0x7797,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0x7798,mtOff);
}
//Red platform guide, right
void drawObject_extended12(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	addObjectTile(o,0x96D1,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0x96D1,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0x96D1,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0x96D2,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0x96D2,mtOff);
}
//Red platform guide, left
void drawObject_extended13(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	addObjectTile(o,0x96D3,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0x96D3,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0x96D1,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0x96D1,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0x96D1,mtOff);
}
//Red stairs guide, right
void drawObject_extended14(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	addObjectTile(o,0x96D6,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0x96D7,mtOff);
	mtOff = offsetMap16Up(mtOff);
	addObjectTile(o,0x96D6,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0x96D7,mtOff);
	mtOff = offsetMap16Up(mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0x96D4,mtOff);
	mtOff = offsetMap16Up(mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0x96D4,mtOff);
}
//Red stairs guide, left
void drawObject_extended15(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	mtOff = offsetMap16Down(mtOff);
	addObjectTile(o,0x96D5,mtOff);
	mtOff = offsetMap16Right(mtOff);
	mtOff = offsetMap16Down(mtOff);
	addObjectTile(o,0x96D5,mtOff);
	mtOff = offsetMap16Right(mtOff);
	mtOff = offsetMap16Down(mtOff);
	addObjectTile(o,0x96D8,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0x96D9,mtOff);
	mtOff = offsetMap16Down(mtOff);
	addObjectTile(o,0x96D8,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0x96D9,mtOff);
}
//Stake red coin tile
void drawObject_extended16(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	WORD orig = objectContexts[curObjCtx].tilemap[mtOff];
	if(orig==0x6B09) addObjectTile(o,0xA309,mtOff);
	else if(orig==0x6B0A) addObjectTile(o,0xA30A,mtOff);
}
//Green coin
void drawObject_extended17(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	addObjectTile(o,0xA400,mtOff);
}
//Intro background
void drawObject_extended18(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	for(int j=0; j<0x10; j++) {
		for(int i=0; i<0x10; i++) {
			int offset = (j<<4)+i;
			int tileRef = romBuf[0x092759+offset];
			if(tileRef>=0x0C) addObjectTile(o,0x9D00+tileRef,mtOff);
			else addObjectTile(o,0xA500+tileRef,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Final Bowser broken ground piece
void drawObject_extended19(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int height = (o->data[3]==0x19)?3:12;
	int base = (o->data[3]==0x19)?0x092897:0x0928F7;
	for(int j=0; j<height; j++) {
		for(int i=0; i<0x20; i++) {
			int offset = (j<<5)+i;
			int tileRef = romBuf[base+offset];
			if(tileRef!=0xFF) {
				if(tileRef>=0x2A) {
					if(tileRef>=0x35) {
						addObjectTile(o,0xA55A+tileRef-0x35,mtOff);
					} else {
						tileRef = (tileRef-0x2A)<<1;
						WORD tile = romBuf[0x092881+tileRef]|(romBuf[0x092882+tileRef]<<8);
						addObjectTile(o,tile,mtOff);
					}
				} else {
					addObjectTile(o,0x9D65+tileRef,mtOff);
				}
			}
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Bones
void drawObject_extended1B(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	for(int j=0; j<2; j++) {
		for(int i=0; i<2; i++) {
			int offset = ((o->data[3]-0x1B)<<3)+(j<<2)+(i<<1);
			WORD tile = romBuf[0x092AC7+offset]|(romBuf[0x092AC8+offset]<<8);
			if(tile) addObjectTile(o,tile,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Hole in Kamek's room
void drawObject_extended1E(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	WORD tile = 0x9D9A;
	for(int j=0; j<4; j++) {
		addObjectTile(o,tile++,mtOff);
		mtOff = offsetMap16Right(mtOff);
		for(int i=0; i<6; i++) {
			addObjectTile(o,0,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		addObjectTile(o,tile++,mtOff);
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Boss door background
void drawObject_extended1F(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	for(int j=0; j<4; j++) {
		for(int i=0; i<4; i++) {
			int offset = (j<<2)+i;
			int tileRef = romBuf[0x092B29+offset];
			addObjectTile(o,0x9600+tileRef,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Unused castle block
void drawObject_extended30(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	for(int j=0; j<2; j++) {
		for(int i=0; i<2; i++) {
			int offset = (j<<2)+(i<<1);
			WORD tile = romBuf[0x092B56+offset]|(romBuf[0x092B57+offset]<<8);
			WORD orig = objectContexts[curObjCtx].tilemap[mtOff];
			if(i && orig==0x015A) {
				int mtOff2 = offsetMap16Right(mtOff);
				addObjectTile(o,0x015C,mtOff2);
			} else if(orig==0x015B) {
				int mtOff2 = offsetMap16Left(mtOff);
				addObjectTile(o,0x015C,mtOff2);
			}
			addObjectTile(o,tile,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Wooden wall
void drawObject_extended31(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	for(int j=0; j<7; j++) {
		addObjectTile(o,0x00BB,mtOff);
		mtOff = offsetMap16Right(mtOff);
		addObjectTile(o,0x00BC,mtOff);
		mtOff = offsetMap16Right(mtOff);
		addObjectTile(o,0x00BD,mtOff);
		mtOff = offsetMap16Right(mtOff);
		addObjectTile(o,0x00BC,mtOff);
		mtOff = offsetMap16Right(mtOff);
		addObjectTile(o,0x00BD,mtOff);
		mtOff = offsetMap16Right(mtOff);
		addObjectTile(o,0x00BC,mtOff);
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Graffiti tile
void drawObject_extended32(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int offset = (o->data[3]-0x32)<<1;
	int tileRef = romBuf[0x092BB9+offset]|(romBuf[0x092BBA+offset]<<8);
	addObjectTile(o,tilesetBuffer[tileRef>>1],mtOff);
}
//Random egg block
void drawObject_extended46(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int offset = noiseTilemap[mtOff]&6;
	int tileRef = romBuf[0x092BF7+offset]|(romBuf[0x092BF8+offset]<<8);
	addObjectTile(o,tilesetBuffer[tileRef>>1],mtOff);
}
//Bandit's house
void drawObject_extended47(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	mtOff = offsetMap16Up(mtOff);
	mtOff = offsetMap16Up(mtOff);
	mtOff = offsetMap16Up(mtOff);
	int preserve = mtOff;
	for(int j=0; j<4; j++) {
		for(int i=0; i<4; i++) {
			int offset = (j<<3)+(i<<1);
			WORD tile = romBuf[0x092C39+offset]|(romBuf[0x092C3A+offset]<<8);
			if(tile) addObjectTile(o,tile,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Unknown
void drawObject_extended48(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	addObjectTile(o,0x00DE,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0x00DE,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0x00E3,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0x00E4,mtOff);
	mtOff = preserve = offsetMap16Up(preserve);
	mtOff = offsetMap16Right(mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0x00E1,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0x00E2,mtOff);
	mtOff = preserve = offsetMap16Up(preserve);
	mtOff = offsetMap16Right(mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0x00E1,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0x00E2,mtOff);
	mtOff = preserve = offsetMap16Up(preserve);
	for(int j=0; j<8; j++) {
		mtOff = offsetMap16Right(mtOff);
		mtOff = offsetMap16Right(mtOff);
		addObjectTile(o,0x00E1,mtOff);
		mtOff = offsetMap16Right(mtOff);
		addObjectTile(o,0x00E2,mtOff);
		mtOff = preserve = offsetMap16Up(preserve);
		mtOff = offsetMap16Right(mtOff);
		addObjectTile(o,0x00E5,mtOff);
		mtOff = offsetMap16Right(mtOff);
		addObjectTile(o,0x00E1,mtOff);
		mtOff = offsetMap16Right(mtOff);
		addObjectTile(o,0x00E2,mtOff);
		mtOff = preserve = offsetMap16Up(preserve);
	}
	mtOff = offsetMap16Right(mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0x00DF,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0x00E0,mtOff);
}
//Red treetop
void drawObject_extended49(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	addObjectTile(o,0x3D4D,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0x3D4E,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0x3D4F,mtOff);
}
//Red leaf, right
void drawObject_extended4A(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	addObjectTile(o,0x3D4C,mtOff);
	mtOff = offsetMap16Left(mtOff);
	WORD orig = objectContexts[curObjCtx].tilemap[mtOff];
	if(orig==0x3D3B || orig==0x3D49 || orig==0x3D4A) {
		addObjectTile(o,0x3D3C,mtOff);
	}
}
//Red leaf, left
void drawObject_extended4B(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	addObjectTile(o,0x3D41,mtOff);
	mtOff = offsetMap16Left(mtOff);
	WORD orig = objectContexts[curObjCtx].tilemap[mtOff];
	if(orig==0x3D3B || orig==0x3D3C || orig==0x3D49) {
		addObjectTile(o,0x3D4A,mtOff);
	}
}
//Mouser nest
void drawObject_extended4C(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	addObjectTile(o,tilesetBuffer[0x1D1A>>1],mtOff);
}
//Small ground bush 1
void drawObject_extended4D(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	addObjectTile(o,0x0080,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0x0081,mtOff);
	mtOff = preserve = offsetMap16Down(preserve);
	addObjectTile(o,0x014B,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0x014C,mtOff);
}
//Small ground bush 2
void drawObject_extended4E(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	addObjectTile(o,0x0082,mtOff);
	mtOff = offsetMap16Down(mtOff);
	addObjectTile(o,0x014D,mtOff);
}
//Small ground bush 3
void drawObject_extended4F(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	addObjectTile(o,0x014A,mtOff);
}
//Arrow sign, right
void drawObject_extended50(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int bg1Ts = ((levelHeader[0]&7)<<1)|(levelHeader[1]>>7);
	for(int j=0; j<2; j++) {
		for(int i=0; i<2; i++) {
			int offset = (j<<2)+(i<<1);
			WORD tile = romBuf[0x092D79+offset]|(romBuf[0x092D7A+offset]<<8);
			WORD orig = objectContexts[curObjCtx].tilemap[mtOff];
			if(orig==tilesetBuffer[0x1C5C>>1] || orig==tilesetBuffer[0x1C5E>>1] || 
			orig==tilesetBuffer[0x1DB4>>1] || orig==tilesetBuffer[0x1DB6>>1]) {
				tile = romBuf[0x092D7D+offset]|(romBuf[0x092D7E +offset]<<8);
				tile = tilesetBuffer[tile>>1];
			}
			if(bg1Ts==4 && j==0) {
				offset = (tile-0x000C)<<1;
				tile = romBuf[0x092D95+offset]|(romBuf[0x092D96+offset]<<8);
			} else if(bg1Ts==12) {
				if((orig&0xFF00)!=0x8500 && j==1) offset += 4;
				tile = romBuf[0x092D9D+offset]|(romBuf[0x092D9E +offset]<<8);
			}
			addObjectTile(o,tile,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Spike mace center tile
void drawObject_extended51(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	addObjectTile(o,0x0183,mtOff);
}
//Spike mace room, rotating
void drawObject_extended52(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	for(int j=0; j<2; j++) {
		for(int i=0; i<3; i++) {
			int offset = (j<<3)+(i<<1);
			WORD tile = romBuf[0x092E2C+offset]|(romBuf[0x092E2D+offset]<<8);
			addObjectTile(o,tile,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Spike mace room, falling
void drawObject_extended53(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	for(int j=0; j<3; j++) {
		for(int i=0; i<3; i++) {
			int offset = (j<<3)+(i<<1);
			WORD tile = romBuf[0x092E72+offset]|(romBuf[0x092E73+offset]<<8);
			if(tile==0x8000) {
				WORD orig = objectContexts[curObjCtx].tilemap[mtOff];
				if(orig==0x015A) tile = 0x015C;
			}
			addObjectTile(o,tile,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Small slanted treetop
void drawObject_extended54(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	for(int j=0; j<3; j++) {
		for(int i=0; i<3; i++) {
			int offset = ((o->data[3]-0x54)*18)+(j*6)+(i<<1);
			WORD tile = romBuf[0x092ECE +offset]|(romBuf[0x092ECF+offset]<<8);
			tile = overlapTile_grassTrees(mtOff,tile);
			if(tile) addObjectTile(o,tile,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Large slanted treetop
void drawObject_extended56(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	for(int j=0; j<3; j++) {
		for(int i=0; i<5; i++) {
			int offset = ((o->data[3]-0x56)*30)+(j*10)+(i<<1);
			WORD tile = romBuf[0x092F08+offset]|(romBuf[0x092F09+offset]<<8);
			tile = overlapTile_grassTrees(mtOff,tile);
			if(tile) addObjectTile(o,tile,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Tree branches, left
void drawObject_extended58(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	for(int j=0; j<2; j++) {
		for(int i=0; i<3; i++) {
			int offset = ((o->data[3]-0x58)*12)+(j*6)+(i<<1);
			WORD tile = romBuf[0x092F5A+offset]|(romBuf[0x092F5B+offset]<<8);
			tile = overlapTile_grassTrees(mtOff,tile);
			if(tile) addObjectTile(o,tile,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Tree branches, right
void drawObject_extended5B(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	for(int j=0; j<2; j++) {
		for(int i=0; i<3; i++) {
			int offset = ((o->data[3]-0x5B)*12)+(j*6)+(i<<1);
			WORD tile = romBuf[0x092F95+offset]|(romBuf[0x092F96+offset]<<8);
			tile = overlapTile_grassTrees(mtOff,tile);
			if(tile) addObjectTile(o,tile,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Donut lift
void drawObject_extended5E(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	addObjectTile(o,0x7502,mtOff);
}
//Rock
const int rockHeightTable[8] = {2,3,2,2,4,4,3,2};
void drawObject_extended5F(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int objRef = (o->data[3]-0x5F)<<1;
	int width = romBuf[0x0930F1+objRef]|(romBuf[0x0930F2+objRef]<<8);
	width >>= 1;
	int height = rockHeightTable[objRef>>1];
	int base = romBuf[0x0930E1+objRef]|(romBuf[0x0930E2+objRef]<<8);
	base += 0x088000;
	for(int j=0; j<height; j++) {
		for(int i=0; i<width; i++) {
			int offset = ((j*width)+i)<<1;
			int tileRef = romBuf[base+offset]|(romBuf[base+1+offset]<<8);
			if(tileRef&0x8000) {
				WORD tile = romBuf[0x88000+tileRef]|(romBuf[0x88001+tileRef]<<8);
				addObjectTile(o,tile,mtOff);
			} else if(tileRef!=0) {
				addObjectTile(o,tilesetBuffer[tileRef>>1],mtOff);
			}
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Stuck tree branch
void drawObject_extended67(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	WORD orig = objectContexts[curObjCtx].tilemap[mtOff];
	for(int n=0; n<0x10; n+=2) {
		int tileRef = romBuf[0x09312E +n]|(romBuf[0x09312F+n]<<8);
		WORD tile = 0;
		if(tileRef&0x8000) tile = romBuf[0x88000+tileRef]|(romBuf[0x88001+tileRef]<<8);
		else tile = tilesetBuffer[tileRef>>1];
		if(tile==orig) {
			tileRef = romBuf[0x09313E +n]|(romBuf[0x09313F+n]<<8);
			if(tileRef&0x8000) tile = romBuf[0x88000+tileRef]|(romBuf[0x88001+tileRef]<<8);
			else tilesetBuffer[tileRef>>1];
			addObjectTile(o,tile,mtOff);
			break;
		}
	}
}
//Waterfall stones
void drawObject_extended68(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int offset = o->data[3]-0x68;
	addObjectTile(o,0x775E +offset,mtOff);
}
//Small bushes
void drawObject_extended6A(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	for(int j=0; j<2; j++) {
		for(int i=0; i<3; i++) {
			int offset = (j<<3)+(i<<1);
			WORD tile = romBuf[0x0931B1+offset]|(romBuf[0x0931B2+offset]<<8);
			addObjectTile(o,tile,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Medium bushes
void drawObject_extended6B(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	for(int j=0; j<3; j++) {
		for(int i=0; i<4; i++) {
			int offset = (j<<3)+(i<<1);
			WORD tile = romBuf[0x0931C3+offset]|(romBuf[0x0931C4+offset]<<8);
			addObjectTile(o,tile,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Large bushes
void drawObject_extended6C(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	for(int j=0; j<3; j++) {
		for(int i=0; i<5; i++) {
			int offset = (j*10)+(i<<1);
			WORD tile = romBuf[0x0931DF+offset]|(romBuf[0x0931E0+offset]<<8);
			addObjectTile(o,tile,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Sewer entrance
void drawObject_extended6D(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int offset = (o->data[3]-0x6D)<<1;
	for(int j=0; j<2; j++) {
		for(int i=0; i<2; i++) {
			WORD tile = romBuf[0x093212+offset]|(romBuf[0x093213+offset]<<8);
			tile += i+(j<<1);
			addObjectTile(o,tile,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Sewer size change piece, vertical
void drawObject_extended71(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int base = (o->data[3]-0x71)*0x1E;
	for(int i=0; i<6; i++) {
		int offset = i<<1;
		WORD tile = romBuf[0x093230+base+offset]|(romBuf[0x093231+base+offset]<<8);
		addObjectTile(o,tile,mtOff);
		mtOff = offsetMap16Right(mtOff);
	}
}
//Sewer size change piece, horizontal
void drawObject_extended73(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int base = (o->data[3]-0x73)*0x17;
	for(int j=0; j<6; j++) {
		int offset = j<<1;
		WORD tile = romBuf[0x093265+base+offset]|(romBuf[0x093266+base+offset]<<8);
		addObjectTile(o,tile,mtOff);
		mtOff = offsetMap16Down(mtOff);
	}
}
//Sewer small joint, horizontal
void drawObject_extended75(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int base = (o->data[3]-0x75)*0x28;
	for(int j=0; j<4; j++) {
		for(int i=0; i<2; i++) {
			int offset = (i<<3)+(j<<1);
			WORD tile = romBuf[0x093293+base+offset]|(romBuf[0x093294+base+offset]<<8);
			if(tile) addObjectTile(o,tile,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Sewer large joint, horizontal
void drawObject_extended77(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int base = (o->data[3]-0x77)*0x2E;
	for(int j=0; j<6; j++) {
		for(int i=0; i<2; i++) {
			int offset = (i<<4)+(j<<1);
			WORD tile = romBuf[0x0932DC+base+offset]|(romBuf[0x0932DD+base+offset]<<8);
			if(tile) addObjectTile(o,tile,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Sewer small joint, vertical
void drawObject_extended79(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int base = (o->data[3]-0x79)*0x21;
	for(int j=0; j<2; j++) {
		for(int i=0; i<4; i++) {
			int offset = (i<<2)+(j<<1);
			WORD tile = romBuf[0x093339+base+offset]|(romBuf[0x09333A+base+offset]<<8);
			if(tile) addObjectTile(o,tile,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Sewer large joint, vertical
void drawObject_extended7B(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int base = (o->data[3]-0x7B)*0x29;
	for(int j=0; j<2; j++) {
		for(int i=0; i<6; i++) {
			int offset = (i<<2)+(j<<1);
			WORD tile = romBuf[0x09337B+base+offset]|(romBuf[0x09337C+base+offset]<<8);
			if(tile) addObjectTile(o,tile,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Sewer grate
void drawObject_extended7D(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	addObjectTile(o,0x77C6,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0x77C7,mtOff);
}
//Sewer wall shadow tile
void drawObject_extended7E(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int offset = (o->data[3]-0x7E)<<1;
	WORD tile = romBuf[0x0933DD+offset]|(romBuf[0x0933DE +offset]<<8);
	addObjectTile(o,tile,mtOff);
}
//Invisible tile, for Wall Lakitu
void drawObject_extended80(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	addObjectTile(o,0x0010,mtOff);
}
//Old goal platform
void drawObject_extended81(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	addObjectTile(o,0x6F00,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0x6F01,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0x6F02,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0x6F03,mtOff);
}
//Yoshi's house
void drawObject_extended82(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	mtOff = offsetMap16Up(mtOff);
	mtOff = offsetMap16Up(mtOff);
	mtOff = offsetMap16Up(mtOff);
	mtOff = offsetMap16Up(mtOff);
	int preserve = mtOff;
	for(int j=0; j<5; j++) {
		for(int i=0; i<8; i++) {
			int offset = (j<<4)+(i<<1);
			WORD tile = romBuf[0x09340C+offset]|(romBuf[0x09340D+offset]<<8);
			addObjectTile(o,tile,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Cloud platform
const int cloudVOffTable[5] = {9,5,3,3,3};
const int cloudHeightTable[5] = {22,11,7,7,8};
void drawObject_extended83(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int objRef = (o->data[3]-0x83)<<1;
	int width = romBuf[0x0938F9+objRef]|(romBuf[0x0938FA+objRef]<<8);
	int height = cloudHeightTable[objRef>>1];
	int voff = cloudVOffTable[objRef>>1];
	int base = romBuf[0x0938EF+objRef]|(romBuf[0x0938F0+objRef]<<8);
	base += 0x088000;
	for(int j=0; j<voff; j++) {
		mtOff = preserve = offsetMap16Up(preserve);
	}
	for(int j=0; j<height; j++) {
		for(int i=0; i<width; i++) {
			int offset = (j*width)+i;
			int tileRef = romBuf[base+offset];
			if(tileRef>=0xD0) {
				tileRef = (tileRef-0xD0)<<1;
				tileRef = romBuf[0x093903+tileRef]|(romBuf[0x093904+tileRef]<<8);
				WORD tile = tilesetBuffer[tileRef>>1];
				addObjectTile(o,tile,mtOff);
			} else if(tileRef) {
				addObjectTile(o,0x8413+tileRef,mtOff);
			}
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Sewer door hole
void drawObject_extended88(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	for(int i=0; i<4; i++) {
		WORD orig = objectContexts[curObjCtx].tilemap[mtOff];
		int offset = i<<1;
		if((orig&0xFF00)==0x8500) {
			WORD orig2 = orig-0x854B;
			WORD tile = romBuf[0x0939C0+offset]|(romBuf[0x0939C1+offset]<<8);
			if(tile) {
				tile += orig2;
				addObjectTile(o,tile,mtOff);
			}
		} else {
			WORD orig2 = (orig-0x7799)&0x00FE;
			WORD tile = romBuf[0x0939B8+offset]|(romBuf[0x0939B9+offset]<<8);
			if(tile && (orig&0xFF00)!=0x7900) {
				tile += romBuf[0x0939C8+orig2];
				addObjectTile(o,tile,mtOff);
			}
		}
		mtOff = offsetMap16Right(mtOff);
	}
	mtOff = preserve = offsetMap16Down(preserve);
	for(int i=0; i<4; i++) {
		WORD orig = objectContexts[curObjCtx].tilemap[mtOff];
		int offset = i<<1;
		if((orig&0xFF00)==0x8500) {
			WORD orig2 = orig-0x854B;
			WORD tile = romBuf[0x093A16+offset]|(romBuf[0x093A17+offset]<<8);
			if(i==0 || i==3) {
				tile += orig2;
			}
			addObjectTile(o,tile,mtOff);
		} else {
			WORD orig2 = (orig-0x7799)&0x00FE;
			WORD tile = romBuf[0x093A0E +offset]|(romBuf[0x093A0F+offset]<<8);
			if(i==0 || i==3) {
				tile += romBuf[0x093A1E + orig2];
			}
			addObjectTile(o,tile,mtOff);
		}
		mtOff = offsetMap16Right(mtOff);
	}
	mtOff = preserve = offsetMap16Down(preserve);
	for(int i=0; i<4; i++) {
		WORD orig = objectContexts[curObjCtx].tilemap[mtOff];
		int offset = i<<1;
		if((orig&0xFF00)==0x8500) {
			WORD orig2 = orig-0x854B;
			WORD tile = romBuf[0x093A6C+offset]|(romBuf[0x093A6D+offset]<<8);
			if(i==0 || i==3) {
				tile += orig2;
			}
			addObjectTile(o,tile,mtOff);
		} else {
			WORD orig2 = (orig-0x7799)&0x00FE;
			WORD tile = romBuf[0x093A64+offset]|(romBuf[0x093A65+offset]<<8);
			if(i==0 || i==3) {
				tile += romBuf[0x093A1E + orig2];
			}
			addObjectTile(o,tile,mtOff);
		}
		mtOff = offsetMap16Right(mtOff);
	}
	mtOff = preserve = offsetMap16Down(preserve);
	for(int i=0; i<4; i++) {
		WORD orig = objectContexts[curObjCtx].tilemap[mtOff];
		int offset = i<<1;
		if((orig&0xFF00)==0x8500) {
			WORD orig2 = orig-0x854B;
			WORD tile = romBuf[0x093AAA+offset]|(romBuf[0x093AAB+offset]<<8);
			if(tile) {
				tile += orig2;
				addObjectTile(o,tile,mtOff);
			}
		} else {
			WORD orig2 = (orig-0x7799)&0x00FE;
			WORD tile = romBuf[0x093AA2+offset]|(romBuf[0x093AA3+offset]<<8);
			if(tile && (orig&0xFF00)!=0x7900 && (orig&0xFF00)!=0x1500) {
				tile += romBuf[0x0939C8+orig2];
				addObjectTile(o,tile,mtOff);
			}
		}
		mtOff = offsetMap16Right(mtOff);
	}
}
//Sewer arrow sign, horizontal
void drawObject_extended89(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int objRef = (o->data[3]-0x89)<<1;
	for(int i=0; i<2; i++) {
		WORD orig = objectContexts[curObjCtx].tilemap[mtOff];
		orig = (orig-0x77A9)&0x000E;
		WORD tile = romBuf[0x093AE5+objRef]|(romBuf[0x093AE6+objRef]<<8);
		tile += orig+i;
		addObjectTile(o,tile,mtOff);
		if(orig==0) {
			int mtOff2 = offsetMap16Down(mtOff);
			tile = romBuf[0x093AE9+objRef]|(romBuf[0x093AEA+objRef]<<8);
			tile += i;
			addObjectTile(o,tile,mtOff2);
		}
		mtOff = offsetMap16Right(mtOff);
	}
}
//Sewer arrow sign, vertical
void drawObject_extended8B(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int objRef = (o->data[3]-0x8B)<<1;
	for(int j=0; j<2; j++) {
		WORD orig = objectContexts[curObjCtx].tilemap[mtOff];
		orig = (orig-0x7799)&0x000E;
		WORD tile = romBuf[0x093B22+objRef]|(romBuf[0x093B23+objRef]<<8);
		tile += orig+j;
		addObjectTile(o,tile,mtOff);
		if(orig==0) {
			int mtOff2 = offsetMap16Right(mtOff);
			tile = romBuf[0x093B26+objRef]|(romBuf[0x093B27+objRef]<<8);
			tile += j;
			addObjectTile(o,tile,mtOff2);
		}
		mtOff = offsetMap16Down(mtOff);
	}
}
//Ground edge top piece
void drawObject_extended8D(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	WORD orig = objectContexts[curObjCtx].tilemap[mtOff];
	int objRef = (orig-tilesetBuffer[0x1CD0>>1])&0x0001;
	WORD tile = objRef+tilesetBuffer[0x1D0E>>1];
	addObjectTile(o,tile,mtOff);
	if(objRef) {
		int mtOff2 = offsetMap16Up(mtOff);
		addObjectTile(o,tilesetBuffer[0x1C60>>1],mtOff2);
		mtOff = offsetMap16Right(mtOff);
		addObjectTile(o,0,mtOff);
		mtOff2 = offsetMap16Up(mtOff);
		addObjectTile(o,0,mtOff2);
	} else {
		int mtOff2 = offsetMap16Up(mtOff);
		addObjectTile(o,tilesetBuffer[0x1C66>>1],mtOff2);
		mtOff = offsetMap16Left(mtOff);
		addObjectTile(o,0,mtOff);
		mtOff2 = offsetMap16Up(mtOff);
		addObjectTile(o,0,mtOff2);
	}
}
//Line guide quarter-circle, small
void drawObject_extended8E(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int objRef = o->data[3]-0x8E;
	addObjectTile(o,0x8710+objRef,mtOff);
}
//Line guide quarter-circle, medium
void drawObject_extended92(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int objRef = (o->data[3]-0x92)<<2;
	for(int j=0; j<2; j++) {
		for(int i=0; i<2; i++) {
			int offset = (j<<1)+i;
			WORD tile = romBuf[0x093C12+objRef+offset]|0x8700;
			if(tile!=0x8700) addObjectTile(o,tile,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Line guide quarter-circle, large
void drawObject_extended96(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int objRef = (o->data[3]-0x96)<<6;
	for(int j=0; j<8; j++) {
		for(int i=0; i<8; i++) {
			int offset = (j<<3)+i;
			WORD tile = romBuf[0x093C4D+objRef+offset]|0x8700;
			if(tile!=0x8700) addObjectTile(o,tile,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Line guide end, horizontal
void drawObject_extended9A(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int objRef = (o->data[3]-0x9A)<<1;
	WORD tile = romBuf[0x093D7A+objRef]|(romBuf[0x093D7B+objRef]<<8);
	addObjectTile(o,tile,mtOff);
	mtOff = offsetMap16Up(mtOff);
	tile = romBuf[0x093D82+objRef]|(romBuf[0x093D83+objRef]<<8);
	addObjectTile(o,tile,mtOff);
}
//Line guide end, vertical
void drawObject_extended9C(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int objRef = (o->data[3]-0x9C)<<1;
	WORD tile = romBuf[0x093D7E +objRef]|(romBuf[0x093D7F+objRef]<<8);
	addObjectTile(o,tile,mtOff);
	mtOff = offsetMap16Left(mtOff);
	tile = romBuf[0x093D86+objRef]|(romBuf[0x093D87+objRef]<<8);
	addObjectTile(o,tile,mtOff);
}
//TODO
//Screen copy command
void drawObject_extendedFB(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	addObjectTile(o,0xB000,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0xB001,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0xB002,mtOff);
}
//Screen scroll enable command
void drawObject_extendedFD(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	addObjectTile(o,0xB003,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0xB004,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0xB005,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0xB006,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0xB007,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0xB008,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0xB009,mtOff);
}
//Screen scroll disable command
void drawObject_extendedFE(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	addObjectTile(o,0xB003,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0xB004,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0xB00A,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0xB00B,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0xB00C,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0xB00D,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0xB009,mtOff);
}
//Screen erase command
void drawObject_extendedFF(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	addObjectTile(o,0xB00E,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0xB00F,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0xB010,mtOff);
}
//Standard objects
//Ledge
void drawObject_01(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	mtOff = offsetMap16Up(mtOff);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+2;
	for(int i=0; i<width; i++) {
		for(int j=0; j<height; j++) {
			if(j<3) {
				WORD orig = objectContexts[curObjCtx].tilemap[mtOff];
				bool overWall = ((orig&0xFF00)==tilesetBuffer[0x1C92>>1]);
				if(overWall) {
					if(orig==(i?tilesetBuffer[0x1CD4>>1]:tilesetBuffer[0x1CD6>>1])) {
						WORD tile = i?tilesetBuffer[0x1CFE>>1]:tilesetBuffer[0x1D00>>1];
						addObjectTile(o,tile,mtOff);
						mtOff = offsetMap16Down(mtOff);
						continue;
					}
				}
				int base = (overWall)?0x0980A4:0x098045;
				int offset = (overWall)?(i?1:0):(i&1);
				offset = (j<<1)+(offset<<3);
				if(j==2) {
					int mtOff2 = offsetMap16Up(mtOff);
					WORD orig2 = objectContexts[curObjCtx].tilemap[mtOff2];
					if(orig2==tilesetBuffer[0x1CA0>>1] || orig2==tilesetBuffer[0x1CA2>>1]) offset+=2;
				}
				int tileRef = romBuf[base+offset]|(romBuf[base+1+offset]<<8);
				addObjectTile(o,tilesetBuffer[tileRef>>1],mtOff);
			} else {
				WORD orig = objectContexts[curObjCtx].tilemap[mtOff];
				if(orig==tilesetBuffer[0x1CF4>>1] || orig==tilesetBuffer[0x1CF6>>1] ||
				(orig>=tilesetBuffer[0x1CD4>>1] && tilesetBuffer[0x1CE8>>1])) {
					mtOff = offsetMap16Down(mtOff);
					continue;
				}
				if((j+1)==height) {
					overlapTile_stdLedgeBottom(o,mtOff,orig);
				} else {
					overlapTile_stdLedgeMid(o,mtOff,orig);
				}
			}
			mtOff = offsetMap16Down(mtOff);
		}
		mtOff = preserve = offsetMap16Right(preserve);
	}
}
//Ledge edge with top, left
void drawObject_02(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	mtOff = offsetMap16Up(mtOff);
	int height = o->data[3]+2;
	for(int j=0; j<height; j++) {
		if(j==0) {
			addObjectTile(o,tilesetBuffer[0x1C5E>>1],mtOff);
			int mtOff2 = offsetMap16Left(mtOff);
			addObjectTile(o,tilesetBuffer[0x1D12>>1],mtOff2);
		} else if(j==1) {
			addObjectTile(o,tilesetBuffer[0x1CD0>>1],mtOff);
			int mtOff2 = offsetMap16Left(mtOff);
			addObjectTile(o,tilesetBuffer[0x1D16>>1],mtOff2);
		} else if(j==2) {
			WORD orig = objectContexts[curObjCtx].tilemap[mtOff];
			if(orig==tilesetBuffer[0x1C5C>>1] || orig==tilesetBuffer[0x1C5E>>1]) addObjectTile(o,tilesetBuffer[0x1CFE>>1],mtOff);
			else addObjectTile(o,tilesetBuffer[0x1CD4>>1],mtOff);
		} else {
			int offset = noiseTilemap[mtOff]&0x06;
			int tileRef = romBuf[0x0982A8+offset]|(romBuf[0x0982A9+offset]<<8);
			WORD tile = tilesetBuffer[tileRef>>1];
			WORD orig = objectContexts[curObjCtx].tilemap[mtOff];
			if(orig==tilesetBuffer[0x1C5C>>1] || orig==tilesetBuffer[0x1C5E>>1]) {
				tile = tilesetBuffer[0x1CE4>>1];
			} else if(orig==tilesetBuffer[0x1CAC>>1] || orig==tilesetBuffer[0x1CB6>>1] ||
			orig==tilesetBuffer[0x1CB8>>1]) {
				int mtOff2 = offsetMap16Up(mtOff);
				WORD orig2 = objectContexts[curObjCtx].tilemap[mtOff2];
				if(orig2!=tilesetBuffer[0x1CFE>>1] && orig2!=tilesetBuffer[0x1D00>>1]) {
					addObjectTile(o,tilesetBuffer[0x1CE4>>1],mtOff2);
				}
				mtOff2 = offsetMap16Right(mtOff);
				tile = tilesetBuffer[0x1CA8>>1];
				orig2 = objectContexts[curObjCtx].tilemap[mtOff2];
				if(orig2!=tilesetBuffer[0x1CAC>>1]) {
					mtOff2 = offsetMap16Down(mtOff);
					addObjectTile(o,tilesetBuffer[0x1CC4>>1],mtOff2);
					tile = tilesetBuffer[0x1CA2>>1];
				}
			}
			addObjectTile(o,tile,mtOff);
		}
		mtOff = offsetMap16Down(mtOff);
	}
}
//Ledge edge with top, right
void drawObject_03(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	mtOff = offsetMap16Up(mtOff);
	int height = o->data[3]+2;
	for(int j=0; j<height; j++) {
		if(j==0) {
			addObjectTile(o,tilesetBuffer[0x1C5E>>1],mtOff);
			int mtOff2 = offsetMap16Right(mtOff);
			addObjectTile(o,tilesetBuffer[0x1D14>>1],mtOff2);
		} else if(j==1) {
			addObjectTile(o,tilesetBuffer[0x1CD2>>1],mtOff);
			int mtOff2 = offsetMap16Right(mtOff);
			addObjectTile(o,tilesetBuffer[0x1D18>>1],mtOff2);
		} else if(j==2) {
			WORD orig = objectContexts[curObjCtx].tilemap[mtOff];
			if(orig==tilesetBuffer[0x1C5C>>1] || orig==tilesetBuffer[0x1C5E>>1]) addObjectTile(o,tilesetBuffer[0x1D00>>1],mtOff);
			else addObjectTile(o,tilesetBuffer[0x1CD6>>1],mtOff);
		} else {
			int offset = noiseTilemap[mtOff]&0x06;
			int tileRef = romBuf[0x0982B0+offset]|(romBuf[0x0982B1+offset]<<8);
			WORD tile = tilesetBuffer[tileRef>>1];
			WORD orig = objectContexts[curObjCtx].tilemap[mtOff];
			if(orig==tilesetBuffer[0x1C5C>>1] || orig==tilesetBuffer[0x1C5E>>1]) {
				tile = tilesetBuffer[0x1CE6>>1];
			} else if(orig==tilesetBuffer[0x1CAC>>1] || orig==tilesetBuffer[0x1CB6>>1] ||
			orig==tilesetBuffer[0x1CB8>>1]) {
				int mtOff2 = offsetMap16Up(mtOff);
				WORD orig2 = objectContexts[curObjCtx].tilemap[mtOff2];
				if(orig2!=tilesetBuffer[0x1CFE>>1] && orig2!=tilesetBuffer[0x1D00>>1]) {
					addObjectTile(o,tilesetBuffer[0x1CE6>>1],mtOff2);
				}
				mtOff2 = offsetMap16Left(mtOff);
				tile = tilesetBuffer[0x1CAA>>1];
				orig2 = objectContexts[curObjCtx].tilemap[mtOff2];
				if(orig2!=tilesetBuffer[0x1CAC>>1]) {
					mtOff2 = offsetMap16Down(mtOff);
					addObjectTile(o,tilesetBuffer[0x1CC2>>1],mtOff2);
					tile = tilesetBuffer[0x1CA0>>1];
				}
			}
			addObjectTile(o,tile,mtOff);
		}
		mtOff = offsetMap16Down(mtOff);
	}
}
//Gentle slope, right
void drawObject_04(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	mtOff = offsetMap16Up(mtOff);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+2;
	for(int i=0; i<width; i++) {
		for(int j=0; j<height; j++) {
			if(j<4) {
				int offset = ((i&1)*0x1D)+(j<<1);
				WORD tileRef = romBuf[0x098338+offset]|(romBuf[0x098339+offset]<<8);
				WORD tile = tilesetBuffer[tileRef>>1];
				addObjectTile(o,tile,mtOff);
			} else {
				WORD orig = objectContexts[curObjCtx].tilemap[mtOff];
				if(orig==tilesetBuffer[0x1CF4>>1] || orig==tilesetBuffer[0x1CF6>>1] ||
				(orig>=tilesetBuffer[0x1CD4>>1] && tilesetBuffer[0x1CE8>>1])) {
					mtOff = offsetMap16Down(mtOff);
					continue;
				}
				if((j+1)==height) {
					overlapTile_stdLedgeBottom(o,mtOff,orig);
				} else {
					overlapTile_stdLedgeMid(o,mtOff,orig);
				}
			}
			mtOff = offsetMap16Down(mtOff);
		}
		mtOff = preserve = offsetMap16Right(preserve);
		if(i&1) {
			mtOff = preserve = offsetMap16Down(preserve);
			height--;
		}
	}
}
//Gentle slope, left
void drawObject_05(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	mtOff = offsetMap16Up(mtOff);
	mtOff = offsetMap16Up(mtOff);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+3;
	for(int i=0; i<width; i++) {
		for(int j=0; j<height; j++) {
			if(j<4) {
				int offset = ((i&1)*0x1D)+(j<<1);
				WORD tileRef = romBuf[0x098340+offset]|(romBuf[0x098341+offset]<<8);
				WORD tile = tilesetBuffer[tileRef>>1];
				addObjectTile(o,tile,mtOff);
			} else {
				WORD orig = objectContexts[curObjCtx].tilemap[mtOff];
				if(orig==tilesetBuffer[0x1CF4>>1] || orig==tilesetBuffer[0x1CF6>>1] ||
				(orig>=tilesetBuffer[0x1CD4>>1] && tilesetBuffer[0x1CE8>>1])) {
					mtOff = offsetMap16Down(mtOff);
					continue;
				}
				if((j+1)==height) {
					overlapTile_stdLedgeBottom(o,mtOff,orig);
				} else {
					overlapTile_stdLedgeMid(o,mtOff,orig);
				}
			}
			mtOff = offsetMap16Down(mtOff);
		}
		mtOff = preserve = offsetMap16Right(preserve);
		if(i&1) {
			mtOff = preserve = offsetMap16Up(preserve);
			height++;
		}
	}
}
//Steep slope, right
void drawObject_06(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	mtOff = offsetMap16Up(mtOff);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+2;
	for(int i=0; i<width; i++) {
		for(int j=0; j<height; j++) {
			if(j<4) {
				int offset = j<<1;
				WORD tileRef = romBuf[0x0983E9+offset]|(romBuf[0x0983EA+offset]<<8);
				WORD tile = tilesetBuffer[tileRef>>1];
				addObjectTile(o,tile,mtOff);
			} else {
				WORD orig = objectContexts[curObjCtx].tilemap[mtOff];
				if(orig==tilesetBuffer[0x1CF4>>1] || orig==tilesetBuffer[0x1CF6>>1] ||
				(orig>=tilesetBuffer[0x1CD4>>1] && tilesetBuffer[0x1CE8>>1])) {
					mtOff = offsetMap16Down(mtOff);
					continue;
				}
				if((j+1)==height) {
					overlapTile_stdLedgeBottom(o,mtOff,orig);
				} else {
					overlapTile_stdLedgeMid(o,mtOff,orig);
				}
			}
			mtOff = offsetMap16Down(mtOff);
		}
		mtOff = preserve = offsetMap16Right(preserve);
		mtOff = preserve = offsetMap16Down(preserve);
		height--;
	}
}
//Steep slope, left
void drawObject_07(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	mtOff = offsetMap16Up(mtOff);
	mtOff = offsetMap16Up(mtOff);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+3;
	for(int i=0; i<width; i++) {
		for(int j=0; j<height; j++) {
			if(j<4) {
				int offset = j<<1;
				WORD tileRef = romBuf[0x098401+offset]|(romBuf[0x098402+offset]<<8);
				WORD tile = tilesetBuffer[tileRef>>1];
				addObjectTile(o,tile,mtOff);
			} else {
				WORD orig = objectContexts[curObjCtx].tilemap[mtOff];
				if(orig==tilesetBuffer[0x1CF4>>1] || orig==tilesetBuffer[0x1CF6>>1] ||
				(orig>=tilesetBuffer[0x1CD4>>1] && tilesetBuffer[0x1CE8>>1])) {
					mtOff = offsetMap16Down(mtOff);
					continue;
				}
				if((j+1)==height) {
					overlapTile_stdLedgeBottom(o,mtOff,orig);
				} else {
					overlapTile_stdLedgeMid(o,mtOff,orig);
				}
			}
			mtOff = offsetMap16Down(mtOff);
		}
		mtOff = preserve = offsetMap16Right(preserve);
		mtOff = preserve = offsetMap16Up(preserve);
		height++;
	}
}
//Very steep slope, right
void drawObject_08(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	mtOff = offsetMap16Up(mtOff);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+2;
	for(int i=0; i<width; i++) {
		for(int j=0; j<height; j++) {
			if(j<5) {
				int offset = j<<1;
				WORD tileRef = romBuf[0x0983F1+offset]|(romBuf[0x0983F2+offset]<<8);
				WORD tile = tilesetBuffer[tileRef>>1];
				addObjectTile(o,tile,mtOff);
			} else {
				WORD orig = objectContexts[curObjCtx].tilemap[mtOff];
				if(orig==tilesetBuffer[0x1CF4>>1] || orig==tilesetBuffer[0x1CF6>>1] ||
				(orig>=tilesetBuffer[0x1CD4>>1] && tilesetBuffer[0x1CE8>>1])) {
					mtOff = offsetMap16Down(mtOff);
					continue;
				}
				if((j+1)==height) {
					overlapTile_stdLedgeBottom(o,mtOff,orig);
				} else {
					overlapTile_stdLedgeMid(o,mtOff,orig);
				}
			}
			mtOff = offsetMap16Down(mtOff);
		}
		mtOff = preserve = offsetMap16Right(preserve);
		mtOff = preserve = offsetMap16Down(preserve);
		mtOff = preserve = offsetMap16Down(preserve);
		height -= 2;
	}
}
//Very steep slope, left
void drawObject_09(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	mtOff = offsetMap16Up(mtOff);
	mtOff = offsetMap16Up(mtOff);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+3;
	for(int i=0; i<width; i++) {
		for(int j=0; j<height; j++) {
			if(j<5) {
				int offset = j<<1;
				WORD tileRef = romBuf[0x098409+offset]|(romBuf[0x09840A+offset]<<8);
				WORD tile = tilesetBuffer[tileRef>>1];
				addObjectTile(o,tile,mtOff);
			} else {
				WORD orig = objectContexts[curObjCtx].tilemap[mtOff];
				if(orig==tilesetBuffer[0x1CF4>>1] || orig==tilesetBuffer[0x1CF6>>1] ||
				(orig>=tilesetBuffer[0x1CD4>>1] && tilesetBuffer[0x1CE8>>1])) {
					mtOff = offsetMap16Down(mtOff);
					continue;
				}
				if((j+1)==height) {
					overlapTile_stdLedgeBottom(o,mtOff,orig);
				} else {
					overlapTile_stdLedgeMid(o,mtOff,orig);
				}
			}
			mtOff = offsetMap16Down(mtOff);
		}
		mtOff = preserve = offsetMap16Right(preserve);
		mtOff = preserve = offsetMap16Up(preserve);
		mtOff = preserve = offsetMap16Up(preserve);
		height += 2;
	}
}
//Left edge
void drawObject_0A(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int height = o->data[3]+1;
	for(int j=0; j<height; j++) {
		if(j==0) {
			addObjectTile(o,tilesetBuffer[0x1CA4>>1],mtOff);
		} else {
			int offset = noiseTilemap[mtOff]&0x06;
			int tileRef = romBuf[0x0982A8+offset]|(romBuf[0x0982A9+offset]<<8);
			WORD tile = tilesetBuffer[tileRef>>1];
			WORD orig = objectContexts[curObjCtx].tilemap[mtOff];
			if(orig==tilesetBuffer[0x1C5C>>1] || orig==tilesetBuffer[0x1C5E>>1]) {
				tile = tilesetBuffer[0x1CE4>>1];
			} else if(orig==tilesetBuffer[0x1CAC>>1] || orig==tilesetBuffer[0x1CB6>>1] ||
			orig==tilesetBuffer[0x1CB8>>1]) {
				int mtOff2 = offsetMap16Up(mtOff);
				WORD orig2 = objectContexts[curObjCtx].tilemap[mtOff2];
				if(orig2!=tilesetBuffer[0x1CFE>>1] && orig2!=tilesetBuffer[0x1D00>>1]) {
					addObjectTile(o,tilesetBuffer[0x1CE4>>1],mtOff2);
				}
				mtOff2 = offsetMap16Right(mtOff);
				tile = tilesetBuffer[0x1CA8>>1];
				orig2 = objectContexts[curObjCtx].tilemap[mtOff2];
				if(orig2!=tilesetBuffer[0x1CAC>>1]) {
					mtOff2 = offsetMap16Down(mtOff);
					addObjectTile(o,tilesetBuffer[0x1CC4>>1],mtOff2);
					tile = tilesetBuffer[0x1CA2>>1];
				}
			}
			addObjectTile(o,tile,mtOff);
		}
		mtOff = offsetMap16Down(mtOff);
	}
}
//Right edge
void drawObject_0B(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int height = o->data[3]+1;
	for(int j=0; j<height; j++) {
		if(j==0) {
			addObjectTile(o,tilesetBuffer[0x1CA6>>1],mtOff);
		} else {
			int offset = noiseTilemap[mtOff]&0x06;
			int tileRef = romBuf[0x0982B0+offset]|(romBuf[0x0982B1+offset]<<8);
			WORD tile = tilesetBuffer[tileRef>>1];
			WORD orig = objectContexts[curObjCtx].tilemap[mtOff];
			if(orig==tilesetBuffer[0x1C5C>>1] || orig==tilesetBuffer[0x1C5E>>1]) {
				tile = tilesetBuffer[0x1CE6>>1];
			} else if(orig==tilesetBuffer[0x1CAC>>1] || orig==tilesetBuffer[0x1CB6>>1] ||
			orig==tilesetBuffer[0x1CB8>>1]) {
				int mtOff2 = offsetMap16Up(mtOff);
				WORD orig2 = objectContexts[curObjCtx].tilemap[mtOff2];
				if(orig2!=tilesetBuffer[0x1CFE>>1] && orig2!=tilesetBuffer[0x1D00>>1]) {
					addObjectTile(o,tilesetBuffer[0x1CE6>>1],mtOff2);
				}
				mtOff2 = offsetMap16Left(mtOff);
				tile = tilesetBuffer[0x1CAA>>1];
				orig2 = objectContexts[curObjCtx].tilemap[mtOff2];
				if(orig2!=tilesetBuffer[0x1CAC>>1]) {
					mtOff2 = offsetMap16Down(mtOff);
					addObjectTile(o,tilesetBuffer[0x1CC2>>1],mtOff2);
					tile = tilesetBuffer[0x1CA0>>1];
				}
			}
			addObjectTile(o,tile,mtOff);
		}
		mtOff = offsetMap16Down(mtOff);
	}
}
//Poundable stake/ski lift prop
void drawObject_0C(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int height = o->data[3]+1;
	int objRef = (o->data[0]-0x0C)<<1;
	if(o->data[0]==0x2B) objRef = 0;
	for(int j=0; j<height; j++) {
		int offset = objRef;
		if(j==0) offset += 0x08;
		else if((j+1)==height) offset += 0x10;
		int tileRef = romBuf[0x098458+offset]|(romBuf[0x098459+offset]<<8);
		if(tileRef&0x8000) {
			WORD tile = romBuf[0x90000+tileRef]|(romBuf[0x90001+tileRef]<<8);
			addObjectTile(o,tile,mtOff);
		} else {
			addObjectTile(o,tilesetBuffer[tileRef>>1],mtOff);
		}
		mtOff = offsetMap16Down(mtOff);
	}
}
//Skinny horizontal platform
void drawObject_0D(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int width = o->data[3]+1;
	for(int i=0; i<width; i++) {
		if(i==0) {
			WORD orig = objectContexts[curObjCtx].tilemap[mtOff];
			if((orig&0xFF00)==tilesetBuffer[0x1C92>>1]) addObjectTile(o,tilesetBuffer[0x1C9A>>1],mtOff);
			else addObjectTile(o,tilesetBuffer[0x1C7A>>1],mtOff);
		} else if((i+1)==width) {
			WORD orig = objectContexts[curObjCtx].tilemap[mtOff];
			if((orig&0xFF00)==tilesetBuffer[0x1C92>>1]) addObjectTile(o,tilesetBuffer[0x1C98>>1],mtOff);
			else addObjectTile(o,tilesetBuffer[0x1C7E>>1],mtOff);
		} else {
			addObjectTile(o,tilesetBuffer[0x1C7C>>1],mtOff);
		}
		mtOff = offsetMap16Right(mtOff);
	}
}
//TODO
//Transparent water
void drawObject_16(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	for(int j=0; j<height; j++) {
		for(int i=0; i<width; i++) {
			addObjectTile(o,0x1600,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//TODO
//Brown platform
void drawObject_63(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int width = o->data[3]+1;
	for(int i=0; i<width; i++) {
		if(i==0) {
			addObjectTile(o,0x151E,mtOff);
		} else if((i+1)==width) {
			addObjectTile(o,0x1520,mtOff);
		} else {
			addObjectTile(o,0x151F,mtOff);
		}
		mtOff = offsetMap16Right(mtOff);
	}
}
//Meltable ice block
void drawObject_66(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	for(int j=0; j<height; j++) {
		for(int i=0; i<width; i++) {
			int offset = ((j&1)<<1)+(i&1);
			addObjectTile(o,0x8900+offset,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Ground
void drawObject_67(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	int bg1Ts = ((levelHeader[0]&7)<<1)|(levelHeader[1]>>7);
	for(int j=0; j<height; j++) {
		for(int i=0; i<width; i++) {
			if(bg1Ts==12) {
				int offset = noiseTilemap[mtOff]&0x3F;
				WORD tile = 0x79E0;
				if(offset<0xB) {
					tile = 0x79BB+offset;
				}
				addObjectTile(o,tile,mtOff);
			} else {
				//TODO: Fix use of tiles and tileset buffer
				int offset = noiseTilemap[mtOff]&0x0E;
				int tileRef = romBuf[0x098121+offset]|(romBuf[0x098122+offset]<<8);
				addObjectTile(o,tilesetBuffer[tileRef>>1],mtOff);
				if(i==0) {
					int mtOff2 = offsetMap16Left(mtOff);
					if(j==0) {
						mtOff2 = offsetMap16Up(mtOff2);
						WORD orig2 = objectContexts[curObjCtx].tilemap[mtOff2];
						if((orig2&0xFF00)==0x1D00) {
							int offset2 = (orig2&0xFF)<<1;
							WORD tile2 = romBuf[0x09C311+orig2]|(romBuf[0x09C312+orig2]<<8);
							addObjectTile(o,tile2,mtOff2);
						}
					} else if((j+1)==height) {
						mtOff2 = offsetMap16Down(mtOff2);
						WORD orig2 = objectContexts[curObjCtx].tilemap[mtOff2];
						if((orig2&0xFF00)==0x1D00) {
							int offset2 = (orig2&0xFF)<<1;
							WORD tile2 = romBuf[0x09C36D+orig2]|(romBuf[0x09C36E +orig2]<<8);
							addObjectTile(o,tile2,mtOff2);
						}
					} else {
						WORD orig2 = objectContexts[curObjCtx].tilemap[mtOff2];
						if((orig2&0xFF00)==0x1D00) {
							int offset2 = (orig2&0xFF)<<1;
							WORD tile2 = romBuf[0x09C194+orig2]|(romBuf[0x09C195+orig2]<<8);
							addObjectTile(o,tile2,mtOff2);
						}
					}
				} else if((i+1)==width) {
					int mtOff2 = offsetMap16Right(mtOff);
					if(j==0) {
						mtOff2 = offsetMap16Up(mtOff2);
						WORD orig2 = objectContexts[curObjCtx].tilemap[mtOff2];
						if((orig2&0xFF00)==0x1D00) {
							int offset2 = (orig2&0xFF)<<1;
							WORD tile2 = romBuf[0x09C481+orig2]|(romBuf[0x09C482+orig2]<<8);
							addObjectTile(o,tile2,mtOff2);
						}
					} else if((j+1)==height) {
						mtOff2 = offsetMap16Down(mtOff2);
						WORD orig2 = objectContexts[curObjCtx].tilemap[mtOff2];
						if((orig2&0xFF00)==0x1D00) {
							int offset2 = (orig2&0xFF)<<1;
							WORD tile2 = romBuf[0x09C4DD+orig2]|(romBuf[0x09C4DE +orig2]<<8);
							addObjectTile(o,tile2,mtOff2);
						}
					} else {
						WORD orig2 = objectContexts[curObjCtx].tilemap[mtOff2];
						if((orig2&0xFF00)==0x1D00) {
							int offset2 = (orig2&0xFF)<<1;
							WORD tile2 = romBuf[0x09C20F+orig2]|(romBuf[0x09C210+orig2]<<8);
							addObjectTile(o,tile2,mtOff2);
						}
					}
				} else if(j==0) {
					int mtOff2 = offsetMap16Up(mtOff);
					WORD orig2 = objectContexts[curObjCtx].tilemap[mtOff2];
					if((orig2&0xFF00)==0x1D00) {
						int offset2 = (orig2&0xFF)<<1;
						WORD tile2 = romBuf[0x09C3C9+orig2]|(romBuf[0x09C3CA+orig2]<<8);
						addObjectTile(o,tile2,mtOff2);
					}
				} else if((j+1)==height) {
					int mtOff2 = offsetMap16Down(mtOff);
					WORD orig2 = objectContexts[curObjCtx].tilemap[mtOff2];
					if((orig2&0xFF00)==0x1D00) {
						int offset2 = (orig2&0xFF)<<1;
						WORD tile2 = romBuf[0x09C425+orig2]|(romBuf[0x09C426+orig2]<<8);
						addObjectTile(o,tile2,mtOff2);
					}
				}
			}
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Coins
void drawObject_68(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	for(int j=0; j<height; j++) {
		for(int i=0; i<width; i++) {
			addObjectTile(o,0x6000,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Squishy block
void drawObject_69(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	for(int j=0; j<height; j++) {
		for(int i=0; i<width; i++) {
			if(j==0) {
				if(i==0) {
					addObjectTile(o,0x6100,mtOff);
				} else if((i+1)==width) {
					addObjectTile(o,0x6102,mtOff);
				} else {
					addObjectTile(o,0x6101,mtOff);
				}
			} else if((j+1)==height) {
				if(i==0) {
					addObjectTile(o,0x6103,mtOff);
				} else if((i+1)==width) {
					addObjectTile(o,0x6105,mtOff);
				} else {
					addObjectTile(o,0x6104,mtOff);
				}
			} else {
				if(i==0) {
					addObjectTile(o,0x0185,mtOff);
				} else if((i+1)==width) {
					addObjectTile(o,0x0187,mtOff);
				} else {
					addObjectTile(o,0x0186,mtOff);
				}
			}
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Unknown horizontal object
void drawObject_6A(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int width = o->data[3]+1;
	for(int i=0; i<width; i++) {
		if(i==0) {
			addObjectTile(o,0x6400,mtOff);
		} else if((i+1)==width) {
			addObjectTile(o,0x6402,mtOff);
		} else {
			addObjectTile(o,0x6401,mtOff);
		}
		mtOff = offsetMap16Right(mtOff);
	}
}
//Goal platform
void drawObject_6B(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	for(int j=0; j<height; j++) {
		for(int i=0; i<width; i++) {
			if(j==0) {
				if(i==0) {
					addObjectTile(o,0x0188,mtOff);
				} else if((i+1)==width) {
					addObjectTile(o,0x018A,mtOff);
				} else {
					addObjectTile(o,0x0189,mtOff);
				}
			} else {
				if(i==0) {
					addObjectTile(o,0x018B,mtOff);
				} else if((i+1)==width) {
					addObjectTile(o,0x018D,mtOff);
				} else {
					addObjectTile(o,0x018C,mtOff);
				}
			}
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Cement blocks, gray
void drawObject_6C(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	for(int j=0; j<height; j++) {
		for(int i=0; i<width; i++) {
			//TODO: Handle castle wall shadowing
			addObjectTile(o,0x0184,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Spiky stake
void drawObject_6D(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int height = o->data[3]+1;
	for(int j=0; j<height; j++) {
		if(j==0) {
			addObjectTile(o,tilesetBuffer[0x1DD6>>1],mtOff);
		} else if((j+1)==height) {
			addObjectTile(o,tilesetBuffer[0x1DD2>>1],mtOff);
		} else {
			addObjectTile(o,tilesetBuffer[0x1DD0>>1],mtOff);
		}
		mtOff = offsetMap16Down(mtOff);
	}
}
//Cement blocks, colorful
void drawObject_6E(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	for(int j=0; j<height; j++) {
		for(int i=0; i<width; i++) {
			int tileRef = noiseTilemap[mtOff]&0xE;
			WORD tile = romBuf[0x09C80C+tileRef]|(romBuf[0x09C80D+tileRef]<<8);
			addObjectTile(o,tile,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//TODO
//Skinny spikes
void drawObject_F5(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	for(int j=0; j<height; j++) {
		for(int i=0; i<width; i++) {
			addObjectTile(o,j?0x2910:0x8413,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Kamek's room blocks
void drawObject_F6(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	for(int j=0; j<height; j++) {
		for(int i=0; i<width; i++) {
			addObjectTile(o,0x9D8B,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}


//Object function pointer table and updater
void drawObject_unused(object_t * o) {}
void (*objectDrawFunc_extended[0x100])(object_t * o) = {
	//00
	drawObject_extended00,drawObject_extended00,drawObject_extended00,drawObject_extended00,
	drawObject_extended04,drawObject_extended04,drawObject_extended04,drawObject_extended04,
	drawObject_extended08,drawObject_extended09,drawObject_extended0A,drawObject_extended0A,
	drawObject_extended0C,drawObject_extended0D,drawObject_extended0D,drawObject_extended0F,
	//10
	drawObject_extended10,drawObject_extended11,drawObject_extended12,drawObject_extended13,
	drawObject_extended14,drawObject_extended15,drawObject_extended16,drawObject_extended17,
	drawObject_extended18,drawObject_extended19,drawObject_extended19,drawObject_extended1B,
	drawObject_extended1B,drawObject_extended1B,drawObject_extended1E,drawObject_extended1F,
	//20
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	//30
	drawObject_extended30,drawObject_extended31,drawObject_extended32,drawObject_extended32,
	drawObject_extended32,drawObject_extended32,drawObject_extended32,drawObject_extended32,
	drawObject_extended32,drawObject_extended32,drawObject_extended32,drawObject_extended32,
	drawObject_extended32,drawObject_extended32,drawObject_extended32,drawObject_extended32,
	//40
	drawObject_extended32,drawObject_extended32,drawObject_extended32,drawObject_extended32,
	drawObject_extended32,drawObject_extended32,drawObject_extended46,drawObject_extended47,
	drawObject_extended48,drawObject_extended49,drawObject_extended4A,drawObject_extended4B,
	drawObject_extended4C,drawObject_extended4D,drawObject_extended4E,drawObject_extended4F,
	//50
	drawObject_extended50,drawObject_extended51,drawObject_extended52,drawObject_extended53,
	drawObject_extended54,drawObject_extended54,drawObject_extended56,drawObject_extended56,
	drawObject_extended58,drawObject_extended58,drawObject_extended58,drawObject_extended5B,
	drawObject_extended5B,drawObject_extended5B,drawObject_extended5E,drawObject_extended5F,
	//60
	drawObject_extended5F,drawObject_extended5F,drawObject_extended5F,drawObject_extended5F,
	drawObject_extended5F,drawObject_extended5F,drawObject_extended5F,drawObject_extended67,
	drawObject_extended68,drawObject_extended68,drawObject_extended6A,drawObject_extended6B,
	drawObject_extended6C,drawObject_extended6D,drawObject_extended6D,drawObject_extended6D,
	//70
	drawObject_extended6D,drawObject_extended71,drawObject_extended71,drawObject_extended73,
	drawObject_extended73,drawObject_extended75,drawObject_extended75,drawObject_extended77,
	drawObject_extended77,drawObject_extended79,drawObject_extended79,drawObject_extended7B,
	drawObject_extended7B,drawObject_extended7D,drawObject_extended7E,drawObject_extended7E,
	//80
	drawObject_extended80,drawObject_extended81,drawObject_extended82,drawObject_extended83,
	drawObject_extended83,drawObject_extended83,drawObject_extended83,drawObject_extended83,
	drawObject_extended88,drawObject_extended89,drawObject_extended89,drawObject_extended8B,
	drawObject_extended8B,drawObject_extended8D,drawObject_extended8E,drawObject_extended8E,
	//90
	drawObject_extended8E,drawObject_extended8E,drawObject_extended92,drawObject_extended92,
	drawObject_extended92,drawObject_extended92,drawObject_extended96,drawObject_extended96,
	drawObject_extended96,drawObject_extended96,drawObject_extended9A,drawObject_extended9A,
	drawObject_extended9C,drawObject_extended9C,drawObject_unused,drawObject_unused,
	//A0
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	//B0
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	//C0
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	//D0
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	//E0
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	//F0
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_extendedFB,
	drawObject_unused,drawObject_extendedFD,drawObject_extendedFE,drawObject_extendedFF};
void (*objectDrawFunc[0x100])(object_t * o) = {
	//00
	drawObject_unused,drawObject_01,drawObject_02,drawObject_03,
	drawObject_04,drawObject_05,drawObject_06,drawObject_07,
	drawObject_08,drawObject_09,drawObject_0A,drawObject_0B,
	drawObject_0C,drawObject_0D,drawObject_0C,drawObject_0C,
	//10
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_16,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	//20
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_0C,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	//30
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	//40
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	//50
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	//60
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_63,
	drawObject_63,drawObject_63,drawObject_66,drawObject_67,
	drawObject_68,drawObject_69,drawObject_6A,drawObject_6B,
	drawObject_6C,drawObject_6D,drawObject_6E,drawObject_unused,
	//70
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	//80
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	//90
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	//A0
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	//B0
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	//C0
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	//D0
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	//E0
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	//F0
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_F5,drawObject_F6,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused};
int setObjectContext(int ctx) {
	int prevCtx = curObjCtx;
	curObjCtx = ctx;
	return prevCtx;
}
void drawObjects() {
	for(int n = 0; n < objectContexts[curObjCtx].objects.size(); n++) {
		object_t thisObject = objectContexts[curObjCtx].objects[n];
		int id = thisObject.data[0];
		if(id) objectDrawFunc[id](&thisObject);
		else {
			id = thisObject.data[3];
			objectDrawFunc_extended[id](&thisObject);
		}
	}
}
void drawSingleObject(int n) {
	object_t thisObject = objectContexts[curObjCtx].objects[n];
	int id = thisObject.data[0];
	if(id) objectDrawFunc[id](&thisObject);
	else {
		id = thisObject.data[3];
		objectDrawFunc_extended[id](&thisObject);
	}
}
const char textTiles_object[4*(3+7+4+3)] = {
	'S','c',' ','C',
	'r','e','o','p',
	'e','n','y',' ',
	'S','c',' ',' ',
	'r','e',' ',' ',
	'e','n','E','n',
	' ',' ','a','b',
	'S','c','l','e',
	'r','o',' ',' ',
	'l','l',' ',' ',
	'e','n','D','i',
	' ',' ','s','a',
	'S','c','b','l',
	'r','o','e',' ',
	'S','c',' ','E',
	'r','e','r','a',
	'e','n','s','e'};
void dispObjects(DWORD * pixelBuf,int width,int height,RECT rect) {
	int minx = rect.left&0xFFF0;
	int miny = rect.top&0x7FF0;
	int maxx = (rect.right&0xFFF0)+0x10;
	int maxy = (rect.bottom&0x7FF0)+0x10;
	for(int j=miny; j<maxy; j+=0x10) {
		for(int i=minx; i<maxx; i+=0x10) {
			int tileIdx = (i>>4)|((j>>4)<<8);
			WORD tile = objectContexts[curObjCtx].tilemap[tileIdx];
			if(tile>=0xB000) {
				int idx = (tile-0xB000)<<2;
				dispMap8Char(pixelBuf,width,height,0xFF,0xFFFFFF,textTiles_object[idx],{i,j});
				dispMap8Char(pixelBuf,width,height,0xFF,0xFFFFFF,textTiles_object[idx+1],{i+8,j});
				dispMap8Char(pixelBuf,width,height,0xFF,0xFFFFFF,textTiles_object[idx+2],{i,j+8});
				dispMap8Char(pixelBuf,width,height,0xFF,0xFFFFFF,textTiles_object[idx+3],{i+8,j+8});
			} else {
				dispMap16Tile(pixelBuf,width,height,tile,{i,j});
			}
			//Check object selection to highlight/invert
			//TODO
		}
	}
}
void initOtherObjectBuffers() {
	//Setup noise tilemap
	srand(time(NULL));
	for(int i=0; i<0x8000; i++) {
		noiseTilemap[i] = rand();
	}
	//Setup tileset buffer
	memset(tilesetBuffer,0,0x4000*sizeof(WORD));
	int bg1Ts = ((levelHeader[0]&7)<<1)|(levelHeader[1]>>7);
	int base = 0x0CD61A;
	while(true) {
		BYTE count = romBuf[base++];
		if(count==0) break;
		int offset = romBuf[base++]|(romBuf[base++]<<8);
		offset >>= 1;
		WORD init = romBuf[(bg1Ts<<1)+base]|(romBuf[(bg1Ts<<1)+base+1]<<8);
		for(int i=0; i<count; i++) {
			tilesetBuffer[offset++] = init++;
		}
		base += 0x20;
	}
	
}

/////////////////////
//OBJECT MANAGEMENT//
/////////////////////
//Predicate for deletion
bool object_delPred(object_t & un) {
	return un.selected;
}

//Load/save
int loadObjects(BYTE * data) {
	//Clear buffers
	for(int i=0; i<0x8000; i++) {
		objectContexts[curObjCtx].assocObjects[i].clear();
		objectContexts[curObjCtx].tilemap[i] = 0;
		objectContexts[curObjCtx].invalidObjects[i] = false;
	}
	objectContexts[curObjCtx].objects.clear();
	//Init stuff
	int curSz = 0;
	//Reload buffer with object data
	while(true) {
		//Create entry
		object_t entry;
		//Get data bytes and check for end
		entry.data[0] = *data++;
		if(entry.data[0]==0xFF) break;
		entry.data[1] = *data++;
		entry.data[2] = *data++;
		entry.data[3] = *data++;
		entry.dataSize = 4;
		curSz += 4;
		if(entry.data[0] && romBuf[0x0904EC+entry.data[0]]&2) {
			entry.data[4] = *data++;
			entry.dataSize = 5;
			curSz++;
		}
		//Init other elements to sane values
		entry.selected = false;
		entry.occupiedTiles.clear();
		//Push back
		objectContexts[curObjCtx].objects.push_back(entry);
	}
	return curSz;
}
int saveObjects(BYTE * data) {
	//Init stuff
	int curSz = 0;
	//Save sprite data
	for(int n=0; n<objectContexts[curObjCtx].objects.size(); n++) {
		//Shorten the names of these for convenience
		BYTE * thisData = objectContexts[curObjCtx].objects[n].data;
		int thisDataSz = objectContexts[curObjCtx].objects[n].dataSize;
		//Copy data
		memcpy(&data[curSz],thisData,thisDataSz);
		//Increment stuff
		curSz += thisDataSz;
	}
}

//Manipulation (internal)
void addToObjectSelection(int index) {
	//Select object
	object_t thisObject = objectContexts[0].objects[index];
	thisObject.selected = true;
	//Mark occupied tiles as invalid
	for(int i = 0; i < thisObject.occupiedTiles.size(); i++) {
		objectContexts[0].invalidObjects[thisObject.occupiedTiles[i]] = true;
	}
}
void removeFromObjectSelection(int index) {
	//Deselect object
	object_t thisObject = objectContexts[0].objects[index];
	thisObject.selected = false;
	//Mark occupied tiles as invalid
	for(int i = 0; i < thisObject.occupiedTiles.size(); i++) {
		objectContexts[0].invalidObjects[thisObject.occupiedTiles[i]] = true;
	}
}
void clearObjectSelection() {
	//Deselect all objects
	for(int n = 0; n < objectContexts[0].objects.size(); n++) {
		object_t thisObject = objectContexts[0].objects[n];
		thisObject.selected = false;
		//Mark occupied tiles as invalid
		for(int i = 0; i < thisObject.occupiedTiles.size(); i++) {
			objectContexts[0].invalidObjects[thisObject.occupiedTiles[i]] = true;
		}
	}
}

//Manipulation
int selectObjects(RECT rect,bool ctrl) {
	//TODO
}
void insertObjects(int x,int y) {
	//TODO
}
void deleteObjects() {
	//Invalidate occupied tiles
	for(int n = 0; n < objectContexts[0].objects.size(); n++) {
		object_t thisObject = objectContexts[0].objects[n];
		if(thisObject.selected) {
			//Mark occupied tiles as invalid
			for(int i = 0; i < thisObject.occupiedTiles.size(); i++) {
				objectContexts[0].invalidObjects[thisObject.occupiedTiles[i]] = true;
			}
		}
	}
	//Delete selected sprites
	remove_if(objectContexts[0].objects.begin(),objectContexts[0].objects.end(),object_delPred);
}
void moveObjects(int dx,int dy) {
	//TODO
}
void resizeObjects(int dx,int dy) {
	//TODO
}
void increaseObjectZ() {
	//TODO
}
void decreaseObjectZ() {
	//TODO
}

///////////////////
//WINDOW FUNCTION//
///////////////////
HDC				hdcObj;
HBITMAP			hbmpObj;
DWORD *			bmpDataObj;

LRESULT CALLBACK WndProc_Object(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {
	switch(msg) {
		//Creation and destruction of window(s)
		case WM_CREATE: {
			//Add controls
			//TODO
			//Create objects
			//TODO
			break;
		}
		case WM_DESTROY: {
			//Free objects
			//TODO
			break;
		}
		case WM_CLOSE: {
			//Simply hide the window
			ShowWindow(hwnd,SW_HIDE);
			wvisObject = false;
			break;
		}
		//Updating
		case WM_PAINT: {
			//TODO
			break;
		}
		//Input
		case WM_COMMAND: {
			//TODO
			break;
		}
		default: {
			return DefWindowProc(hwnd,msg,wParam,lParam);
		}
	}
	return 0;
}

