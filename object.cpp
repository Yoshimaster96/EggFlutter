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
WORD tilesetBuffer[0x1000];

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
			WORD tileRef = romBuf[0x09250B+offset];
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
			WORD tileRef = romBuf[0x092759+offset];
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
			WORD tileRef = romBuf[base+offset];
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
			WORD tileRef = romBuf[0x092B29+offset];
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
				int feelOff = offsetMap16Right(mtOff);
				addObjectTile(o,0x015C,feelOff);
			} else if(orig==0x015B) {
				int feelOff = offsetMap16Left(mtOff);
				addObjectTile(o,0x015C,feelOff);
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
	WORD tileRef = romBuf[0x092BB9+offset]|(romBuf[0x092BBA+offset]<<8);
	addObjectTile(o,tilesetBuffer[tileRef>>1],mtOff);
}
//Random egg block
void drawObject_extended46(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int offset = noiseTilemap[mtOff]&6;
	WORD tileRef = romBuf[0x092BF7+offset]|(romBuf[0x092BF8+offset]<<8);
	addObjectTile(o,tilesetBuffer[tileRef>>1],mtOff);
}
//Bandit's house
void drawObject_extended47(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
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
			if(orig==tilesetBuffer[0x1C5C>>1] || orig==tilesetBuffer[0x1C5E>>1] || orig==tilesetBuffer[0x1DB4>>1] || orig==tilesetBuffer[0x1DB6>>1]) tile = romBuf[0x092D7D+offset]|(romBuf[0x092D7E +offset]<<8);
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
			WORD tileRef = romBuf[base+offset]|(romBuf[base+1+offset]<<8);
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
//TODO
//Standard objects
//TODO

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
	drawObject_extended5F,drawObject_extended5F,drawObject_extended5F,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
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
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused};
void (*objectDrawFunc[0x100])(object_t * o) = {
	//00
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	//10
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	//20
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
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
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
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
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
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
				//TODO
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
	memset(tilesetBuffer,0,0x1000*sizeof(WORD));
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
	//Draw object data
	drawObjects();
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

