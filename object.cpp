#include "object.h"

HWND hwndObject;
HWND hwndObjectLb,hwndObjectCb;
bool wvisObject = false;
level_object_data_ctx_t objectContexts[2];
int curObjCtx = 0;

//////////////////////////////
//OBJECT OCCUPY REGISTRATION//
//////////////////////////////
void addObjectTile(object_t * o,WORD tile,int offset) {
	if(offset>=0 && offset<0x8000) {
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

//Helper functions (for calculating tile offsets)
inline int getBaseMap16Offset(object_t * o) {
	BYTE hi = o->data[1];
	BYTE lo = o->data[2];
	return (lo&0xF)|((hi&0xF)<<4)|((lo&0xF0)<<4)|((hi&0xF0)<<8);
}
inline WORD getOriginalMap16Tile(int offset) {
	if(offset>=0 && offset<0x8000) return objectContexts[curObjCtx].tilemap[offset];
	else return 0;
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
inline void overlapTile_grassTrees(object_t * o,int mtOff,WORD tile) {
	WORD orig = getOriginalMap16Tile(mtOff);
	if(tile==0x3D9F && orig==0x3D72) tile = 0x3DA9;
	else if(tile==0x3DA0 && orig==0x3D71) tile = 0x3DA8;
	if(tile) addObjectTile(o,tile,mtOff);
}
inline void overlapTile_stdLedgeMid(object_t * o,int mtOff,WORD orig) {
	if(orig==tilesetBuffer[0x1CF4>>1] || orig==tilesetBuffer[0x1CF6>>1] ||
	(orig>=tilesetBuffer[0x1CD4>>1] && orig<tilesetBuffer[0x1CE8>>1])) return;
	int mtOff2 = offsetMap16Left(mtOff);
	WORD orig2 = getOriginalMap16Tile(mtOff2);
	if(orig2==tilesetBuffer[0x1CAC>>1]) {
		addObjectTile(o,tilesetBuffer[0x1CCA>>1],mtOff2);
		mtOff2 = offsetMap16Down(mtOff2);
		addObjectTile(o,tilesetBuffer[0x1CC4>>1],mtOff2);
	}
	mtOff2 = offsetMap16Right(mtOff);
	orig2 = getOriginalMap16Tile(mtOff2);
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
	if(orig==tilesetBuffer[0x1CF4>>1] || orig==tilesetBuffer[0x1CF6>>1] ||
	(orig>=tilesetBuffer[0x1CD4>>1] && orig<tilesetBuffer[0x1CE8>>1]) ||
	orig==tilesetBuffer[0x1CC2>>1] || orig==tilesetBuffer[0x1CC4>>1] ||
	orig==tilesetBuffer[0x1CCA>>1] || orig==tilesetBuffer[0x1CCC>>1]) return;
	if(orig==tilesetBuffer[0x1CB6>>1] || orig==tilesetBuffer[0x1CB8>>1]) {
		int mtOff2 = offsetMap16Left(mtOff);
		WORD orig2 = getOriginalMap16Tile(mtOff2);
		if(orig2==tilesetBuffer[0x1CE8>>1] || orig2==tilesetBuffer[0x1CEA>>1] ||
		(orig2>=tilesetBuffer[0x1CAE>>1]) && orig2<tilesetBuffer[0x1CB6>>1]) {
			addObjectTile(o,tilesetBuffer[0x1CCC>>1],mtOff);
			mtOff2 = offsetMap16Down(mtOff);
			addObjectTile(o,tilesetBuffer[0x1CC2>>1],mtOff2);
			return;
		}
		mtOff2 = offsetMap16Right(mtOff);
		orig2 = getOriginalMap16Tile(mtOff2);
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
inline void overlapTile_hookbillStone2(object_t * o,int mtOff,WORD * tile,int mtOff2,int mtOff3,int mtOff4) {
	WORD orig2 = getOriginalMap16Tile(mtOff2);
	WORD orig3 = getOriginalMap16Tile(mtOff3);
	WORD orig4 = getOriginalMap16Tile(mtOff4);
	if((orig2&0xFF00)==0x9D00 && (orig3&0xFF00)==0x9D00 && (orig4&0xFF00)==0x9D00) {
		*tile = 0x9D00|romBuf[0x099ACB+((*tile)&0xFF)];
		WORD tile2 = 0x9D00|romBuf[0x099ACB+(orig2&0xFF)];
		addObjectTile(o,tile2,mtOff2);
	}
}
inline void overlapTile_hookbillStone4(object_t * o,int mtOff,WORD * tile,int mtOff2,int mtOff3,int mtOff4) {
	WORD orig2 = getOriginalMap16Tile(mtOff2);
	WORD orig3 = getOriginalMap16Tile(mtOff3);
	WORD orig4 = getOriginalMap16Tile(mtOff4);
	if((orig2&0xFF00)==0x9D00 && (orig3&0xFF00)==0x9D00 && (orig4&0xFF00)==0x9D00) {
		*tile = 0x9D00|romBuf[0x099ACB+((*tile)&0xFF)];
		WORD tile2 = 0x9D00|romBuf[0x099ACB+(orig2&0xFF)];
		WORD tile3 = 0x9D00|romBuf[0x099ACB+(orig3&0xFF)];
		WORD tile4 = 0x9D00|romBuf[0x099ACB+(orig4&0xFF)];
		addObjectTile(o,tile2,mtOff2);
		addObjectTile(o,tile3,mtOff3);
		addObjectTile(o,tile4,mtOff4);
	}
}
inline void overlapTile_castleWallShadowDL(object_t * o, int mtOff2) {
	WORD orig2 = getOriginalMap16Tile(mtOff2);
	for(int n=0; n<0x12; n+=2) {
		int tileRef = romBuf[0x09A1AF+n]|(romBuf[0x09A1B0+n]<<8);
		if(orig2==tileRef) {
			WORD tile = romBuf[0x09A1C1+n]|(romBuf[0x09A1C2+n]<<8);
			addObjectTile(o,tile,mtOff2);
			return;
		}
	}
}
inline void overlapTile_castleWallShadowD(object_t * o, int mtOff2) {
	WORD orig2 = getOriginalMap16Tile(mtOff2);
	for(int n=0; n<0x26; n+=2) {
		int tileRef = romBuf[0x09A226+n]|(romBuf[0x09A227+n]<<8);
		if(orig2==tileRef) {
			WORD tile = romBuf[0x09A24C+n]|(romBuf[0x09A24D+n]<<8);
			addObjectTile(o,tile,mtOff2);
			return;
		}
	}
}
inline void overlapTile_castleWallShadowDR(object_t * o, int mtOff2) {
	WORD orig2 = getOriginalMap16Tile(mtOff2);
	for(int n=0; n<0x12; n+=2) {
		int tileRef = romBuf[0x09A1AF+n]|(romBuf[0x09A1B0+n]<<8);
		if(orig2==tileRef) {
			WORD tile = romBuf[0x09A321+n]|(romBuf[0x09A322+n]<<8);
			addObjectTile(o,tile,mtOff2);
			return;
		}
	}
}
inline void overlapTile_castleWallShadowR(object_t * o, int mtOff2) {
	WORD orig2 = getOriginalMap16Tile(mtOff2);
	for(int n=0; n<0x26; n+=2) {
		int tileRef = romBuf[0x09A226+n]|(romBuf[0x09A227+n]<<8);
		if(orig2==tileRef) {
			WORD tile = romBuf[0x09A2AB+n]|(romBuf[0x09A2AC+n]<<8);
			addObjectTile(o,tile,mtOff2);
			return;
		}
	}
}
inline void overlapTile_castleWallShadowUR(object_t * o, int mtOff2) {
	WORD orig2 = getOriginalMap16Tile(mtOff2);
	for(int n=0; n<0x12; n+=2) {
		int tileRef = romBuf[0x09A1AF+n]|(romBuf[0x09A1B0+n]<<8);
		if(orig2==tileRef) {
			WORD tile = romBuf[0x09A1F8+n]|(romBuf[0x09A1F9+n]<<8);
			addObjectTile(o,tile,mtOff2);
			return;
		}
	}
}
const int sandBlockIdxToDir[16] = {0,4,5,1,2,10,8,6,7,3,14,15,11,12,13,9};
const int sandBlockDirToIdx[16] = {0,3,4,9,1,2,7,8,6,15,5,12,13,14,10,11};
inline void overlapTile_sandBlock(object_t * o,int mtOff,int baseIdx) {
	int base = 0x09AC7B+(baseIdx*0x3E);
	WORD orig = getOriginalMap16Tile(mtOff);
	if(orig==0 || orig==0x00C2) {
		int tileRef = romBuf[base]|(romBuf[base+1]<<8);
		addObjectTile(o,tilesetBuffer[tileRef>>1],mtOff);
	} else if((orig&0xFF00)==tilesetBuffer[0x1A62>>1]) {
		for(int i=0; i<0x3E0; i+=2) {
			int tileRef = romBuf[0x09AC7B+i]|(romBuf[0x09AC7C+i]<<8);
			if(tilesetBuffer[tileRef>>1]==orig) {
				int baseIdx2 = i/0x3E;
				i %= 0x3E;
				baseIdx = sandBlockDirToIdx[sandBlockIdxToDir[baseIdx]|sandBlockIdxToDir[baseIdx2]];
				base = 0x09AC7B+(baseIdx*0x3E);
				tileRef = romBuf[base+i]|(romBuf[base+1+i]<<8);
				addObjectTile(o,tilesetBuffer[tileRef>>1],mtOff);
				return;
			}
		}
	} else {
		for(int i=2; i<0x3E; i+=2) {
			int tileRef = romBuf[0x09AC3D+i]|(romBuf[0x09AC3E +i]<<8);
			if(tilesetBuffer[tileRef>>1]==orig) {
				tileRef = romBuf[base+i]|(romBuf[base+1+i]<<8);
				addObjectTile(o,tilesetBuffer[tileRef>>1],mtOff);
				return;
			}
		}
	}
}
inline void overlapTile_slopeXSectEdgeL(object_t * o,int mtOff) {
	int mtOff2 = offsetMap16Left(mtOff);
	WORD orig2 = getOriginalMap16Tile(mtOff2);
	if((orig2&0xFF00)==tilesetBuffer[0x1BE0>>1]) {
		int offset = (orig2&0xFF)<<1;
		int tileRef = romBuf[0x09C194+offset]|(romBuf[0x09C195+offset]<<8);
		addObjectTile(o,tilesetBuffer[tileRef>>1],mtOff2);
	}
}
inline void overlapTile_slopeXSectEdgeR(object_t * o,int mtOff) {
	int mtOff2 = offsetMap16Right(mtOff);
	WORD orig2 = getOriginalMap16Tile(mtOff2);
	if((orig2&0xFF00)==tilesetBuffer[0x1BE0>>1]) {
		int offset = (orig2&0xFF)<<1;
		int tileRef = romBuf[0x09C20F+offset]|(romBuf[0x09C210+offset]<<8);
		addObjectTile(o,tilesetBuffer[tileRef>>1],mtOff2);
	}
}
inline void overlapTile_groundXSectEdge(object_t * o,int mtOff2,int base) {
	WORD orig2 = getOriginalMap16Tile(mtOff2);
	if((orig2&0xFF00)==tilesetBuffer[0x1BE0>>1]) {
		int offset2 = (orig2&0xFF)<<1;
		int tileRef = romBuf[base+offset2]|(romBuf[base+1+offset2]<<8);
		addObjectTile(o,tilesetBuffer[tileRef>>1],mtOff2);
	}
}
inline void overlapTile_sewerPipeWall(object_t * o,int mtOff,WORD tile,int row) {
	WORD orig = getOriginalMap16Tile(mtOff);
	for(int n=0; n<0x20; n+=2) {
		int tileRef = romBuf[0x09DF94+n]|(romBuf[0x09DF95+n]<<8);
		if(tile==tileRef) {
			tileRef = (n&8)<<1;
			for(int m=0; m<0x10; m+=2) {
				int tileRef2 = romBuf[0x09DFD4+tileRef+m]|(romBuf[0x09DFD5+tileRef+m]<<8);
				if(orig==tileRef2) {
					tileRef2 = ((m&0xC)<<1)|row;
					tileRef = (n&0x1C)<<3;
					WORD tile2 = romBuf[0x09DFF4+tileRef+tileRef2]|(romBuf[0x09DFF5+tileRef+tileRef2]<<8);
					if(tile2==0xFFFF) addObjectTile(o,tile,mtOff);
					else if(tile2) addObjectTile(o,tile2,mtOff);
					return;
				}
			}
		}
	}
	addObjectTile(o,tile,mtOff);
}

//Extended objects
//Jungle leaf
void drawObject_extended00(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int objRef = o->data[3]*12;
	for(int j=0; j<3; j++) {
		for(int i=0; i<2; i++) {
			int offset = (j<<2)+(i<<1);
			WORD tile = romBuf[0x0923F6+objRef+offset]|(romBuf[0x0923F7+objRef+offset]<<8);
			if(tile) addObjectTile(o,tile,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Small jungle leaf
void drawObject_extended04(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int objRef = (o->data[3]-0x04)*6;
	for(int j=0; j<3; j++) {
		int offset = (j<<1);
		WORD tile = romBuf[0x092426+objRef+offset]|(romBuf[0x092427+objRef+offset]<<8);
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
			int offset = (j*4)+(i<<1);
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
	int objRef = (o->data[3]-0x0A)<<3;
	for(int j=0; j<2; j++) {
		for(int i=0; i<2; i++) {
			int offset = (j<<2)+(i<<1);
			WORD tile = romBuf[0x0924B5+objRef+offset]|(romBuf[0x0924B6+objRef+offset]<<8);
			addObjectTile(o,tile,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Vine on mud
void drawObject_extended0C(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	for(int j=0; j<4; j++) {
		int offset = (j<<1);
		WORD tile = romBuf[0x0924E4+offset]|(romBuf[0x0924E5+offset]<<8);
		addObjectTile(o,tile,mtOff);
		int mtOff2 = offsetMap16Right(mtOff);
		tile++;
		addObjectTile(o,tile,mtOff2);
		mtOff = offsetMap16Down(mtOff);
	}
}
//Kamek's room piece
void drawObject_extended0D(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int objRef = (o->data[3]-0x0D)<<7;
	for(int j=0; j<0x10; j++) {
		for(int i=0; i<8; i++) {
			int offset = (j<<3)+i;
			int tileRef = romBuf[0x09250B+objRef+offset];
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
	WORD orig = getOriginalMap16Tile(mtOff);
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
			if(j>=0xC) addObjectTile(o,0x9D00+tileRef,mtOff);
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
	int objRef = o->data[3]-0x19;
	int height = objRef?12:3;
	int base = objRef?0x0928F7:0x092897;
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
	int objRef = (o->data[3]-0x1B)<<3;
	for(int j=0; j<2; j++) {
		for(int i=0; i<2; i++) {
			int offset = (j<<2)+(i<<1);
			WORD tile = romBuf[0x092AC7+objRef+offset]|(romBuf[0x092AC8+objRef+offset]<<8);
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
			WORD orig = getOriginalMap16Tile(mtOff);
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
	mtOff = offsetMap16Left(mtOff);
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
	WORD orig = getOriginalMap16Tile(mtOff);
	if(orig==0x3D3B || orig==0x3D49 || orig==0x3D4A) {
		addObjectTile(o,0x3D3C,mtOff);
	}
}
//Red leaf, left
void drawObject_extended4B(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	addObjectTile(o,0x3D41,mtOff);
	mtOff = offsetMap16Left(mtOff);
	WORD orig = getOriginalMap16Tile(mtOff);
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
			WORD orig = getOriginalMap16Tile(mtOff);
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
			if(i==0) {
				int mtOff2 = offsetMap16Left(mtOff);
				WORD orig2 = getOriginalMap16Tile(mtOff2);
				if(orig2==0x015A) addObjectTile(o,0x015C,mtOff2);
			} else if(i==2) {
				int mtOff2 = offsetMap16Right(mtOff);
				WORD orig2 = getOriginalMap16Tile(mtOff2);
				if(orig2==0x015B) addObjectTile(o,0x015C,mtOff2);
			}
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
				WORD orig = getOriginalMap16Tile(mtOff);
				if(orig==0x015A) tile = 0x015C;
			}
			if(tile!=0x8000) addObjectTile(o,tile,mtOff);
			if(i==0 && j!=2) {
				int mtOff2 = offsetMap16Left(mtOff);
				WORD orig2 = getOriginalMap16Tile(mtOff2);
				if(orig2==0x015A) addObjectTile(o,0x015C,mtOff2);
			} else if(i==2) {
				int mtOff2 = offsetMap16Right(mtOff);
				WORD orig2 = getOriginalMap16Tile(mtOff2);
				if(orig2==0x015B) addObjectTile(o,0x015C,mtOff2);
			}
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Small slanted treetop
void drawObject_extended54(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int objRef = (o->data[3]-0x54)*18;
	for(int j=0; j<3; j++) {
		for(int i=0; i<3; i++) {
			int offset = (j*6)+(i<<1);
			WORD tile = romBuf[0x092ECE +objRef+offset]|(romBuf[0x092ECF+objRef+offset]<<8);
			overlapTile_grassTrees(o,mtOff,tile);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Large slanted treetop
void drawObject_extended56(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int objRef = (o->data[3]-0x56)*30;
	for(int j=0; j<3; j++) {
		for(int i=0; i<5; i++) {
			int offset = (j*10)+(i<<1);
			WORD tile = romBuf[0x092F08+objRef+offset]|(romBuf[0x092F09+objRef+offset]<<8);
			overlapTile_grassTrees(o,mtOff,tile);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Tree branches, left
void drawObject_extended58(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int objRef = (o->data[3]-0x58)*12;
	for(int j=0; j<2; j++) {
		for(int i=0; i<3; i++) {
			int offset = (j*6)+(i<<1);
			WORD tile = romBuf[0x092F5A+objRef+offset]|(romBuf[0x092F5B+objRef+offset]<<8);
			overlapTile_grassTrees(o,mtOff,tile);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Tree branches, right
void drawObject_extended5B(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int objRef = (o->data[3]-0x5B)*12;
	for(int j=0; j<2; j++) {
		for(int i=0; i<3; i++) {
			int offset = (j*6)+(i<<1);
			WORD tile = romBuf[0x092F95+objRef+offset]|(romBuf[0x092F96+objRef+offset]<<8);
			overlapTile_grassTrees(o,mtOff,tile);
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
	WORD orig = getOriginalMap16Tile(mtOff);
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
void drawObject_extended83(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int objRef = (o->data[3]-0x83)<<1;
	int width = romBuf[0x090DAE +objRef];
	int height = romBuf[0x090DB8+objRef];
	int voff = romBuf[0x090DA4+objRef]>>4;
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
		WORD orig = getOriginalMap16Tile(mtOff);
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
		WORD orig = getOriginalMap16Tile(mtOff);
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
		WORD orig = getOriginalMap16Tile(mtOff);
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
		WORD orig = getOriginalMap16Tile(mtOff);
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
		WORD orig = getOriginalMap16Tile(mtOff);
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
		WORD orig = getOriginalMap16Tile(mtOff);
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
	WORD orig = getOriginalMap16Tile(mtOff);
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
//Sewer waterfall
void drawObject_extended9E(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int objRef = o->data[3]-0x9E;
	WORD orig = getOriginalMap16Tile(mtOff);
	addObjectTile(o,orig+0x17+(objRef<<2),mtOff);
	mtOff = offsetMap16Down(mtOff);
	addObjectTile(o,0x8104+objRef,mtOff);
}
//Sewer rock corner, top left
void drawObject_extendedA0(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	mtOff = preserve = offsetMap16Left(preserve);
	mtOff = preserve = offsetMap16Up(preserve);
	for(int j=0; j<2; j++) {
		for(int i=0; i<2; i++) {
			int offset = i+(j<<1);
			int mtOff2 = offsetMap16Up(mtOff);
			WORD orig2 = getOriginalMap16Tile(mtOff2);
			if(orig2==0x7942 || orig2==0x7943) addObjectTile(o,orig2+3,mtOff2);
			mtOff2 = offsetMap16Left(mtOff);
			orig2 = getOriginalMap16Tile(mtOff2);
			if(orig2==0x7944 || orig2==0x794B) addObjectTile(o,orig2+4,mtOff2);
			else if(orig2==0x7946 || orig2==0x794D) addObjectTile(o,orig2+3,mtOff2);
			addObjectTile(o,0x7970+offset,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Sewer rock corner, top right
void drawObject_extendedA1(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	mtOff = preserve = offsetMap16Up(preserve);
	for(int j=0; j<2; j++) {
		for(int i=0; i<2; i++) {
			int offset = i+(j<<1);
			int mtOff2 = offsetMap16Down(mtOff);
			WORD orig2 = getOriginalMap16Tile(mtOff2);
			if(orig2==0x7948 || orig2==0x7949) addObjectTile(o,orig2+3,mtOff2);
			mtOff2 = offsetMap16Right(mtOff);
			orig2 = getOriginalMap16Tile(mtOff2);
			if(orig2==0x7955 || orig2==0x795C) addObjectTile(o,orig2+3,mtOff2);
			else if(orig2==0x7957 || orig2==0x795E) addObjectTile(o,orig2+2,mtOff2);
			addObjectTile(o,0x7974+offset,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Sewer rock corner, bottom left
void drawObject_extendedA2(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	mtOff = preserve = offsetMap16Left(preserve);
	for(int j=0; j<2; j++) {
		for(int i=0; i<2; i++) {
			int offset = i+(j<<1);
			int mtOff2 = offsetMap16Down(mtOff);
			WORD orig2 = getOriginalMap16Tile(mtOff2);
			if(orig2==0x793D || orig2==0x793E) addObjectTile(o,orig2+3,mtOff2);
			mtOff2 = offsetMap16Left(mtOff);
			orig2 = getOriginalMap16Tile(mtOff2);
			if(orig2==0x7962 || orig2==0x7969) addObjectTile(o,orig2+4,mtOff2);
			else if(orig2==0x7964 || orig2==0x796B) addObjectTile(o,orig2+3,mtOff2);
			addObjectTile(o,0x7978+offset,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Sewer rock corner, bottom right
void drawObject_extendedA3(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	for(int j=0; j<2; j++) {
		for(int i=0; i<2; i++) {
			int offset = i+(j<<1);
			int mtOff2 = offsetMap16Down(mtOff);
			WORD orig2 = getOriginalMap16Tile(mtOff2);
			if(orig2==0x794F || orig2==0x7950) addObjectTile(o,orig2+2,mtOff2);
			mtOff2 = offsetMap16Right(mtOff);
			orig2 = getOriginalMap16Tile(mtOff2);
			if(orig2==0x7963 || orig2==0x796A) addObjectTile(o,orig2+3,mtOff2);
			else if(orig2==0x7965 || orig2==0x796C) addObjectTile(o,orig2+2,mtOff2);
			addObjectTile(o,0x797C+offset,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Small yellow flowers
void drawObject_extendedA4(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	addObjectTile(o,0x000A,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0x000B,mtOff);
	mtOff = preserve = offsetMap16Down(preserve);
	addObjectTile(o,0x8800,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0x8801,mtOff);
}
//Large snowy tree
void drawObject_extendedA5(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	mtOff = preserve = offsetMap16Up(preserve);
	mtOff = preserve = offsetMap16Up(preserve);
	mtOff = preserve = offsetMap16Up(preserve);
	mtOff = preserve = offsetMap16Up(preserve);
	mtOff = preserve = offsetMap16Up(preserve);
	mtOff = preserve = offsetMap16Up(preserve);
	mtOff = preserve = offsetMap16Up(preserve);
	mtOff = preserve = offsetMap16Up(preserve);
	mtOff = preserve = offsetMap16Left(preserve);
	mtOff = preserve = offsetMap16Left(preserve);
	for(int j=0; j<9; j++) {
		for(int i=0; i<5; i++) {
			int offset = (i<<1)+(j*10);
			WORD tile = romBuf[0x093F7E +offset]|(romBuf[0x093F7F+offset]<<8);
			if(tile) addObjectTile(o,tile,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Small snowy tree
void drawObject_extendedA6(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	mtOff = preserve = offsetMap16Up(preserve);
	mtOff = preserve = offsetMap16Up(preserve);
	mtOff = preserve = offsetMap16Up(preserve);
	mtOff = preserve = offsetMap16Up(preserve);
	mtOff = preserve = offsetMap16Left(preserve);
	for(int j=0; j<5; j++) {
		for(int i=0; i<3; i++) {
			int offset = (i<<1)+(j*6);
			WORD tile = romBuf[0x093F60+offset]|(romBuf[0x093F61+offset]<<8);
			if(tile) addObjectTile(o,tile,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Frozen Shy-Guy
void drawObject_extendedA7(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	addObjectTile(o,0x799C,mtOff);
}
//Arrow sign, left
void drawObject_extendedA8(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int bg1Ts = ((levelHeader[0]&7)<<1)|(levelHeader[1]>>7);
	for(int j=0; j<2; j++) {
		for(int i=0; i<2; i++) {
			int offset = (j<<2)+(i<<1);
			WORD tile = romBuf[0x092D89+offset]|(romBuf[0x092D8A+offset]<<8);
			WORD orig = getOriginalMap16Tile(mtOff);
			if(orig==tilesetBuffer[0x1C5C>>1] || orig==tilesetBuffer[0x1C5E>>1] || 
			orig==tilesetBuffer[0x1DB4>>1] || orig==tilesetBuffer[0x1DB6>>1]) {
				tile = romBuf[0x092D8D+offset]|(romBuf[0x092D8E +offset]<<8);
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
//Icicle
void drawObject_extendedA9(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int objRef = o->data[3]-0xA9;
	addObjectTile(o,0x799D,mtOff);
	mtOff = offsetMap16Down(mtOff);
	if(objRef==0) {
		addObjectTile(o,0x8E00,mtOff);
		mtOff = offsetMap16Down(mtOff);
	}
	if(objRef<2) {
		addObjectTile(o,0x8E01,mtOff);
		mtOff = offsetMap16Down(mtOff);
	}
	addObjectTile(o,0x8E02,mtOff);
	mtOff = offsetMap16Down(mtOff);
	addObjectTile(o,0x8D95,mtOff);
}
//Broken icicle
void drawObject_extendedAC(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	addObjectTile(o,0x799D,mtOff);
	mtOff = offsetMap16Down(mtOff);
	addObjectTile(o,0x799E,mtOff);
	mtOff = offsetMap16Down(mtOff);
	addObjectTile(o,0x8D94,mtOff);
}
//Crystal
void drawObject_extendedAD(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int objRef = (o->data[3]-0xAD)<<1;
	int seed = (noiseTilemap[mtOff]&3)*0xE;
	int height = romBuf[0x090FAD+objRef];
	int base = romBuf[0x0940A5+objRef]|(romBuf[0x0940A6+objRef]<<8);
	base += 0x088000;
	for(int j=0; j<height; j++) {
		for(int i=0; i<2; i++) {
			int offset = (i<<1)+(j<<2);
			WORD tile = romBuf[base+offset]|(romBuf[base+offset+1]<<8);
			addObjectTile(o,tile+seed,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Rocks on lava
void drawObject_extendedB3(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	addObjectTile(o,0x8D8E,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0x8D8F,mtOff);
}
//Small mushroom
void drawObject_extendedB4(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int objRef = (o->data[3]-0xB4)<<4;
	int seed = (noiseTilemap[mtOff]&8)+objRef;
	for(int j=0; j<2; j++) {
		for(int i=0; i<2; i++) {
			int offset = (i<<1)+(j<<2);
			int tileRef = romBuf[0x0940E8+seed+offset]|(romBuf[0x0940E9+seed+offset]<<8);
			if(j==0) addObjectTile(o,tilesetBuffer[tileRef>>1],mtOff);
			else addObjectTile(o,tileRef,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Large mushroom
void drawObject_extendedB6(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int objRef = (o->data[3]-0xB6)<<1;
	int seed = (noiseTilemap[mtOff]&1)+objRef;
	seed *= 0x16;
	for(int j=0; j<3; j++) {
		for(int i=0; i<3; i++) {
			int offset = (i<<1)+(j*6);
			int tileRef = romBuf[0x094131+seed+offset]|(romBuf[0x094132+seed+offset]<<8);
			if(j!=2) {
				if(tileRef&0x8000) {
					tileRef = romBuf[0x088000+tileRef]|(romBuf[0x088001+tileRef]<<8);
					addObjectTile(o,tileRef,mtOff);
				}
				else addObjectTile(o,tilesetBuffer[tileRef>>1],mtOff);
			}
			else addObjectTile(o,tileRef,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Group of 3 or 4 mushrooms
void drawObject_extendedB8(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int objRef = o->data[3]-0xB8;
	int width = objRef?5:4;
	int height = objRef?6:4;
	int base = objRef?0x0941E6:0x0941BA;
	for(int j=0; j<height; j++) {
		for(int i=0; i<width; i++) {
			int offset = (i<<1)+((j<<1)*width);
			int tileRef = romBuf[base+offset]|(romBuf[base+offset+1]<<8);
			if(j!=(height-1)) {
				if(tileRef&0x8000) {
					tileRef = romBuf[0x088000+tileRef]|(romBuf[0x088001+tileRef]<<8);
					addObjectTile(o,tileRef,mtOff);
				}
				else if(tileRef) addObjectTile(o,tilesetBuffer[tileRef>>1],mtOff);
			}
			else addObjectTile(o,tileRef,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Dandelion
void drawObject_extendedBA(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int objRef = (o->data[3]-0xBA)<<1;
	int seed = (noiseTilemap[mtOff]&3);
	if(seed==3) seed = 0;
	int height = romBuf[0x09105B+objRef];
	int base = romBuf[0x094290+objRef]|(romBuf[0x094291+objRef]<<8);
	base += 0x088000;
	for(int j=0; j<height; j++) {
		int offset = (j<<1);
		WORD tile = romBuf[base+offset]|(romBuf[base+offset+1]<<8);
		if(j==0) tile = (objRef>4)?0x8D45:0x8D36;
		addObjectTile(o,tile+seed,mtOff);
		mtOff = offsetMap16Down(mtOff);
	}
}
//Small snowy platform
void drawObject_extendedC0(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	addObjectTile(o,0x8DA7,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0x8DA8,mtOff);
	mtOff = preserve = offsetMap16Down(preserve);
	int mtOff2 = offsetMap16Down(mtOff);
	WORD orig2 = getOriginalMap16Tile(mtOff2);
	if(orig2==0x8DA5 || orig2==0x8DA6) addObjectTile(o,0x8F04,mtOff);
	else addObjectTile(o,0x152A,mtOff);
	mtOff = offsetMap16Right(mtOff);
	mtOff2 = offsetMap16Down(mtOff);
	orig2 = getOriginalMap16Tile(mtOff2);
	if(orig2==0x8DA5 || orig2==0x8DA6) addObjectTile(o,0x8F05,mtOff);
	else addObjectTile(o,0x152B,mtOff);
}
//Snowy platform support tip
void drawObject_extendedC1(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int mtOff2 = offsetMap16Up(mtOff);
	WORD orig2 = getOriginalMap16Tile(mtOff2);
	if(orig2==0x152A || orig2==0x152B) addObjectTile(o,0x8F04,mtOff2);
	addObjectTile(o,0x8DA5,mtOff);
	mtOff = offsetMap16Right(mtOff);
	mtOff2 = offsetMap16Down(mtOff);
	orig2 = getOriginalMap16Tile(mtOff2);
	if(orig2==0x152A || orig2==0x152B) addObjectTile(o,0x8F05,mtOff2);
	addObjectTile(o,0x8DA6,mtOff);
}
//Snowy platform
void drawObject_extendedC2(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int objRef = (o->data[3]-0xC2)<<5;
	for(int j=0; j<4; j++) {
		for(int i=0; i<4; i++) {
			int offset = (i<<1)+(j<<3);
			WORD tile = romBuf[0x094335+objRef+offset]|(romBuf[0x094336+objRef+offset]<<8);
			addObjectTile(o,tile,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Egg block
void drawObject_extendedC4(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	addObjectTile(o,0x5F04,mtOff);
}
//Flower garden background piece
void drawObject_extendedC5(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int objRef = (o->data[3]-0xC5)<<1;
	int width = romBuf[0x0910D9+objRef];
	int height = romBuf[0x0910E3+objRef];
	int base = romBuf[0x0943BF+objRef]|(romBuf[0x0943C0+objRef]<<8);
	base += 0x088000;
	for(int j=0; j<height; j++) {
		for(int i=0; i<width; i++) {
			int offset = i+(j*width);
			int tileRef = romBuf[base+offset];
			if(tileRef) addObjectTile(o,0x7900+tileRef,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Flower garden tile
void drawObject_extendedCA(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int objRef = o->data[3]-0xCA;
	addObjectTile(o,0x79BB+objRef,mtOff);
}
//Flower garden rock
void drawObject_extendedD4(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int objRef = o->data[3]-0xD4;
	int width = romBuf[0x09111F+objRef];
	int height = romBuf[0x09112B+objRef];
	int base = romBuf[0x094658+(objRef<<1)]|(romBuf[0x094659+(objRef<<1)]<<8);
	base += 0x088000;
	for(int j=0; j<height; j++) {
		for(int i=0; i<width; i++) {
			int offset = (i<<1)+((j<<1)*width);
			WORD tile = romBuf[base+offset]|(romBuf[base+offset+1]<<8);
			if(tile) {
				addObjectTile(o,tile,mtOff);
				if((j+1)==height) {
					int mtOff2 = offsetMap16Down(mtOff);
					WORD orig2 = getOriginalMap16Tile(mtOff2);
					if(orig2==0x100F) addObjectTile(o,0x100E,mtOff2);
					else if(orig2==0x0C0B) addObjectTile(o,0x0C0C,mtOff2);
				}
			}
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//3D pipe lock
void drawObject_extendedE0(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	addObjectTile(o,0x7D24,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0x7D25,mtOff);
	mtOff = preserve = offsetMap16Down(preserve);
	addObjectTile(o,0x0118,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0x0119,mtOff);
}
//Special screen copy
void drawObject_extendedFB(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	addObjectTile(o,0xBCB0,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0xBCB1,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0xBCB2,mtOff);
}
//Special screen scroll enable
void drawObject_extendedFD(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	addObjectTile(o,0xBCD0,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0xBCD1,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0xBCD2,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0xBCD3,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0xBCD4,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0xBCD5,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0xBCD6,mtOff);
}
//Special screen scroll disable
void drawObject_extendedFE(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	addObjectTile(o,0xBCE0,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0xBCE1,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0xBCE2,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0xBCE3,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0xBCE4,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0xBCE5,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0xBCE6,mtOff);
}
//Special screen clear
void drawObject_extendedFF(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	addObjectTile(o,0xBCF0,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0xBCF1,mtOff);
	mtOff = offsetMap16Right(mtOff);
	addObjectTile(o,0xBCF2,mtOff);
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
				WORD orig = getOriginalMap16Tile(mtOff);
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
					WORD orig2 = getOriginalMap16Tile(mtOff2);
					if(orig2==tilesetBuffer[0x1CA0>>1] || orig2==tilesetBuffer[0x1CA2>>1]) offset+=2;
				}
				int tileRef = romBuf[base+offset]|(romBuf[base+1+offset]<<8);
				addObjectTile(o,tilesetBuffer[tileRef>>1],mtOff);
			} else {
				WORD orig = getOriginalMap16Tile(mtOff);
				if((j+1)==height) overlapTile_stdLedgeBottom(o,mtOff,orig);
				else overlapTile_stdLedgeMid(o,mtOff,orig);
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
			WORD orig = getOriginalMap16Tile(mtOff);
			if(orig==tilesetBuffer[0x1C5C>>1] || orig==tilesetBuffer[0x1C5E>>1]) addObjectTile(o,tilesetBuffer[0x1CFE>>1],mtOff);
			else addObjectTile(o,tilesetBuffer[0x1CD4>>1],mtOff);
		} else {
			int offset = noiseTilemap[mtOff]&0x06;
			int tileRef = romBuf[0x0982A8+offset]|(romBuf[0x0982A9+offset]<<8);
			WORD tile = tilesetBuffer[tileRef>>1];
			WORD orig = getOriginalMap16Tile(mtOff);
			if(orig==tilesetBuffer[0x1C5C>>1] || orig==tilesetBuffer[0x1C5E>>1]) {
				tile = tilesetBuffer[0x1CE4>>1];
			} else if(orig==tilesetBuffer[0x1CAC>>1] || orig==tilesetBuffer[0x1CB6>>1] ||
			orig==tilesetBuffer[0x1CB8>>1]) {
				int mtOff2 = offsetMap16Up(mtOff);
				WORD orig2 = getOriginalMap16Tile(mtOff2);
				if(orig2!=tilesetBuffer[0x1CFE>>1] && orig2!=tilesetBuffer[0x1D00>>1]) {
					addObjectTile(o,tilesetBuffer[0x1CE4>>1],mtOff2);
				}
				mtOff2 = offsetMap16Right(mtOff);
				tile = tilesetBuffer[0x1CA8>>1];
				orig2 = getOriginalMap16Tile(mtOff2);
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
			WORD orig = getOriginalMap16Tile(mtOff);
			if(orig==tilesetBuffer[0x1C5C>>1] || orig==tilesetBuffer[0x1C5E>>1]) addObjectTile(o,tilesetBuffer[0x1D00>>1],mtOff);
			else addObjectTile(o,tilesetBuffer[0x1CD6>>1],mtOff);
		} else {
			int offset = noiseTilemap[mtOff]&0x06;
			int tileRef = romBuf[0x0982B0+offset]|(romBuf[0x0982B1+offset]<<8);
			WORD tile = tilesetBuffer[tileRef>>1];
			WORD orig = getOriginalMap16Tile(mtOff);
			if(orig==tilesetBuffer[0x1C5C>>1] || orig==tilesetBuffer[0x1C5E>>1]) {
				tile = tilesetBuffer[0x1CE6>>1];
			} else if(orig==tilesetBuffer[0x1CAC>>1] || orig==tilesetBuffer[0x1CB6>>1] ||
			orig==tilesetBuffer[0x1CB8>>1]) {
				int mtOff2 = offsetMap16Up(mtOff);
				WORD orig2 = getOriginalMap16Tile(mtOff2);
				if(orig2!=tilesetBuffer[0x1CFE>>1] && orig2!=tilesetBuffer[0x1D00>>1]) {
					addObjectTile(o,tilesetBuffer[0x1CE6>>1],mtOff2);
				}
				mtOff2 = offsetMap16Left(mtOff);
				tile = tilesetBuffer[0x1CAA>>1];
				orig2 = getOriginalMap16Tile(mtOff2);
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
				WORD orig = getOriginalMap16Tile(mtOff);
				if((j+1)==height) overlapTile_stdLedgeBottom(o,mtOff,orig);
				else overlapTile_stdLedgeMid(o,mtOff,orig);
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
				WORD orig = getOriginalMap16Tile(mtOff);
				if((j+1)==height) overlapTile_stdLedgeBottom(o,mtOff,orig);
				else overlapTile_stdLedgeMid(o,mtOff,orig);
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
				WORD orig = getOriginalMap16Tile(mtOff);
				if((j+1)==height) overlapTile_stdLedgeBottom(o,mtOff,orig);
				else overlapTile_stdLedgeMid(o,mtOff,orig);
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
				WORD orig = getOriginalMap16Tile(mtOff);
				if((j+1)==height) overlapTile_stdLedgeBottom(o,mtOff,orig);
				else overlapTile_stdLedgeMid(o,mtOff,orig);
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
				WORD orig = getOriginalMap16Tile(mtOff);
				if((j+1)==height) overlapTile_stdLedgeBottom(o,mtOff,orig);
				else overlapTile_stdLedgeMid(o,mtOff,orig);
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
				WORD orig = getOriginalMap16Tile(mtOff);
				if((j+1)==height) overlapTile_stdLedgeBottom(o,mtOff,orig);
				else overlapTile_stdLedgeMid(o,mtOff,orig);
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
			WORD orig = getOriginalMap16Tile(mtOff);
			if(orig==tilesetBuffer[0x1C5C>>1] || orig==tilesetBuffer[0x1C5E>>1]) {
				tile = tilesetBuffer[0x1CE4>>1];
			} else if(orig==tilesetBuffer[0x1CAC>>1] || orig==tilesetBuffer[0x1CB6>>1] ||
			orig==tilesetBuffer[0x1CB8>>1]) {
				int mtOff2 = offsetMap16Up(mtOff);
				WORD orig2 = getOriginalMap16Tile(mtOff2);
				if(orig2!=tilesetBuffer[0x1CFE>>1] && orig2!=tilesetBuffer[0x1D00>>1]) {
					addObjectTile(o,tilesetBuffer[0x1CE4>>1],mtOff2);
				}
				mtOff2 = offsetMap16Right(mtOff);
				tile = tilesetBuffer[0x1CA8>>1];
				orig2 = getOriginalMap16Tile(mtOff2);
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
			WORD orig = getOriginalMap16Tile(mtOff);
			if(orig==tilesetBuffer[0x1C5C>>1] || orig==tilesetBuffer[0x1C5E>>1]) {
				tile = tilesetBuffer[0x1CE6>>1];
			} else if(orig==tilesetBuffer[0x1CAC>>1] || orig==tilesetBuffer[0x1CB6>>1] ||
			orig==tilesetBuffer[0x1CB8>>1]) {
				int mtOff2 = offsetMap16Up(mtOff);
				WORD orig2 = getOriginalMap16Tile(mtOff2);
				if(orig2!=tilesetBuffer[0x1CFE>>1] && orig2!=tilesetBuffer[0x1D00>>1]) {
					addObjectTile(o,tilesetBuffer[0x1CE6>>1],mtOff2);
				}
				mtOff2 = offsetMap16Left(mtOff);
				tile = tilesetBuffer[0x1CAA>>1];
				orig2 = getOriginalMap16Tile(mtOff2);
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
		WORD orig = getOriginalMap16Tile(mtOff);
		if(i==0) {
			if((orig&0xFF00)==tilesetBuffer[0x1C92>>1]) addObjectTile(o,tilesetBuffer[0x1C9A>>1],mtOff);
			else addObjectTile(o,tilesetBuffer[0x1C7A>>1],mtOff);
		} else if((i+1)==width) {
			if((orig&0xFF00)==tilesetBuffer[0x1C92>>1]) addObjectTile(o,tilesetBuffer[0x1C98>>1],mtOff);
			else addObjectTile(o,tilesetBuffer[0x1C7E>>1],mtOff);
		} else {
			addObjectTile(o,tilesetBuffer[0x1C7C>>1],mtOff);
		}
		mtOff = offsetMap16Right(mtOff);
	}
}
//Diagonal ski lift wire, gentle slope
void drawObject_10(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int width = o->data[3];
	if(width&0x80) {
		width = 0x101-width;
		for(int i=0; i<width; i++) {
			WORD orig = getOriginalMap16Tile(mtOff);
			if(i==0) {
				if(orig==0x00B4 || orig==0x00A7) addObjectTile(o,0x00A7,mtOff);
				else addObjectTile(o,0x0092,mtOff);
			} else if((i+1)==width) {
				if(orig==0x00B4 || orig==0x00A7) addObjectTile(o,0x00A7,mtOff);
				else addObjectTile(o,0x0093,mtOff);
			} else {
				if(i&1) {
					addObjectTile(o,0x009E,mtOff);
				} else {
					addObjectTile(o,0x009D,mtOff);
					mtOff = offsetMap16Down(mtOff);
					addObjectTile(o,0x009F,mtOff);
				}
			}
			mtOff = offsetMap16Left(mtOff);
		}
	} else {
		width++;
		for(int i=0; i<width; i++) {
			WORD orig = getOriginalMap16Tile(mtOff);
			if(i==0) {
				if(orig==0x00B4 || orig==0x00A7) addObjectTile(o,0x00A7,mtOff);
				else addObjectTile(o,0x0093,mtOff);
			} else if((i+1)==width) {
				if(orig==0x00B4 || orig==0x00A7) addObjectTile(o,0x00A7,mtOff);
				else addObjectTile(o,0x0092,mtOff);
			} else {
				if(i&1) {
					addObjectTile(o,0x009B,mtOff);
				} else {
					addObjectTile(o,0x009C,mtOff);
					mtOff = offsetMap16Down(mtOff);
					addObjectTile(o,0x009A,mtOff);
				}
			}
			mtOff = offsetMap16Right(mtOff);
		}
	}
}
//Diagonal ski lift wire, steep slope
void drawObject_11(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int width = o->data[3];
	if(width&0x80) {
		width = 0x101-width;
		for(int i=0; i<width; i++) {
			WORD orig = getOriginalMap16Tile(mtOff);
			if(i==0) {
				if(orig==0x00B4 || orig==0x00A7) addObjectTile(o,0x00A7,mtOff);
				else addObjectTile(o,0x0092,mtOff);
			} else if((i+1)==width) {
				if(orig==0x00B4 || orig==0x00A7) addObjectTile(o,0x00A7,mtOff);
				else addObjectTile(o,0x0093,mtOff);
			} else {
				addObjectTile(o,0x0098,mtOff);
				mtOff = offsetMap16Down(mtOff);
				addObjectTile(o,0x0099,mtOff);
			}
			mtOff = offsetMap16Left(mtOff);
		}
	} else {
		width++;
		for(int i=0; i<width; i++) {
			WORD orig = getOriginalMap16Tile(mtOff);
			if(i==0) {
				if(orig==0x00B4 || orig==0x00A7) addObjectTile(o,0x00A7,mtOff);
				else addObjectTile(o,0x0093,mtOff);
			} else if((i+1)==width) {
				if(orig==0x00B4 || orig==0x00A7) addObjectTile(o,0x00A7,mtOff);
				else addObjectTile(o,0x0092,mtOff);
			} else {
				addObjectTile(o,0x0097,mtOff);
				mtOff = offsetMap16Down(mtOff);
				addObjectTile(o,0x0096,mtOff);
			}
			mtOff = offsetMap16Right(mtOff);
		}
	}
}
//Diagonal ski lift wire, very steep slope
void drawObject_12(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int width = o->data[3];
	if(width&0x80) {
		width = 0x101-width;
		for(int i=0; i<width; i++) {
			WORD orig = getOriginalMap16Tile(mtOff);
			if(i==0) {
				if(orig==0x00B4 || orig==0x00A7) addObjectTile(o,0x00A7,mtOff);
				else addObjectTile(o,0x0092,mtOff);
			} else if((i+1)==width) {
				if(orig==0x00B4 || orig==0x00A7) addObjectTile(o,0x00A7,mtOff);
				else addObjectTile(o,0x0093,mtOff);
			} else {
				addObjectTile(o,0x00A0,mtOff);
				mtOff = offsetMap16Down(mtOff);
				addObjectTile(o,0x00A2,mtOff);
				mtOff = offsetMap16Down(mtOff);
				addObjectTile(o,0x00A1,mtOff);
			}
			mtOff = offsetMap16Left(mtOff);
		}
	} else {
		width++;
		for(int i=0; i<width; i++) {
			WORD orig = getOriginalMap16Tile(mtOff);
			if(i==0) {
				if(orig==0x00B4 || orig==0x00A7) addObjectTile(o,0x00A7,mtOff);
				else addObjectTile(o,0x0093,mtOff);
			} else if((i+1)==width) {
				if(orig==0x00B4 || orig==0x00A7) addObjectTile(o,0x00A7,mtOff);
				else addObjectTile(o,0x0092,mtOff);
			} else {
				addObjectTile(o,0x00A5,mtOff);
				mtOff = offsetMap16Down(mtOff);
				addObjectTile(o,0x00A3,mtOff);
				mtOff = offsetMap16Down(mtOff);
				addObjectTile(o,0x00A4,mtOff);
			}
			mtOff = offsetMap16Right(mtOff);
		}
	}
}
//Horizontal ski lift wire
void drawObject_13(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int width = o->data[3]+1;
	for(int i=0; i<width; i++) {
		WORD orig = getOriginalMap16Tile(mtOff);
		if(i==0) {
			if(orig==0x00B4 || orig==0x00A7) addObjectTile(o,0x00A7,mtOff);
			else addObjectTile(o,0x0093,mtOff);
		} else if((i+1)==width) {
			if(orig==0x00B4 || orig==0x00A7) addObjectTile(o,0x00A7,mtOff);
			else addObjectTile(o,0x0092,mtOff);
		} else {
			if(orig==0x00B4 || orig==0x00A7) addObjectTile(o,0x00A7,mtOff);
			else addObjectTile(o,0x00A6,mtOff);
		}
		mtOff = offsetMap16Right(mtOff);
	}
}
//Cross section
void drawObject_14(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	if(width==1) {
		for(int j=0; j<height; j++) {
			WORD orig = getOriginalMap16Tile(mtOff);
			int tileRef = 0;
			if((orig&0xFF00)==tilesetBuffer[0x1BE0>>1]) {
				tileRef = (orig&0xFF)<<1;
				tileRef += 2;
			} else {
				for(int n=0; n<0x24; n+=2) {
					int tileRef2 = romBuf[0x098760+n]|(romBuf[0x098761+n]<<8);
					if(orig==tilesetBuffer[tileRef2>>1]) {
						tileRef = n+0x28;
						break;
					}
				}
			}
			if(j==0) {
				tileRef = romBuf[0x0987E2+tileRef]|(romBuf[0x0987E3+tileRef]<<8);
				addObjectTile(o,tilesetBuffer[tileRef>>1],mtOff);
			} else if((j+1)==height) {
				tileRef = romBuf[0x098840+tileRef]|(romBuf[0x098841+tileRef]<<8);
				addObjectTile(o,tilesetBuffer[tileRef>>1],mtOff);
			} else {
				tileRef = romBuf[0x098784+tileRef]|(romBuf[0x098785+tileRef]<<8);
				addObjectTile(o,tilesetBuffer[tileRef>>1],mtOff);
			}
			mtOff = offsetMap16Down(mtOff);
		}
	} else if(height==1) {
		for(int i=0; i<width; i++) {
			WORD orig = getOriginalMap16Tile(mtOff);
			int tileRef = 0;
			if((orig&0xFF00)==tilesetBuffer[0x1BE0>>1]) {
				tileRef = (orig&0xFF)<<1;
				tileRef += 2;
			} else {
				for(int n=0; n<0x24; n+=2) {
					int tileRef2 = romBuf[0x098760+n]|(romBuf[0x098761+n]<<8);
					if(orig==tilesetBuffer[tileRef2>>1]) {
						tileRef = n+0x28;
						break;
					}
				}
			}
			if(i==0) {
				tileRef = romBuf[0x0988FC+tileRef]|(romBuf[0x0988FD+tileRef]<<8);
				addObjectTile(o,tilesetBuffer[tileRef>>1],mtOff);
			} else if((i+1)==width) {
				tileRef = romBuf[0x09895A+tileRef]|(romBuf[0x09895B+tileRef]<<8);
				addObjectTile(o,tilesetBuffer[tileRef>>1],mtOff);
			} else {
				tileRef = romBuf[0x09889E +tileRef]|(romBuf[0x09889F+tileRef]<<8);
				addObjectTile(o,tilesetBuffer[tileRef>>1],mtOff);
			}
			mtOff = offsetMap16Right(mtOff);
		}
	} else {
		for(int j=0; j<height; j++) {
			for(int i=0; i<width; i++) {
				WORD orig = getOriginalMap16Tile(mtOff);
				int tileRef = 0;
				if((orig&0xFF00)==tilesetBuffer[0x1BE0>>1]) {
					tileRef = (orig&0xFF)<<1;
					tileRef += 2;
				} else {
					for(int n=0; n<0x24; n+=2) {
						int tileRef2 = romBuf[0x098760+n]|(romBuf[0x098761+n]<<8);
						if(orig==tilesetBuffer[tileRef2>>1]) {
							tileRef = n+0x28;
							break;
						}
					}
				}
				if(j==0) {
					if(i==0) {
						tileRef = romBuf[0x0989B8+tileRef]|(romBuf[0x0989B9+tileRef]<<8);
						addObjectTile(o,tilesetBuffer[tileRef>>1],mtOff);
						int mtOff2 = offsetMap16Up(mtOff);
						WORD orig2 = getOriginalMap16Tile(mtOff2);
						if(orig2==tilesetBuffer[0x1C5C>>1] || orig2==tilesetBuffer[0x1C5E>>1]) {
							addObjectTile(o,0x007E,mtOff2);
						}
					} else if((i+1)==width) {
						tileRef = romBuf[0x098B8E +tileRef]|(romBuf[0x098B8F+tileRef]<<8);
						addObjectTile(o,tilesetBuffer[tileRef>>1],mtOff);
						int mtOff2 = offsetMap16Up(mtOff);
						WORD orig2 = getOriginalMap16Tile(mtOff2);
						if(orig2==tilesetBuffer[0x1C5C>>1] || orig2==tilesetBuffer[0x1C5E>>1]) {
							addObjectTile(o,0x007F,mtOff2);
						}
					} else {
						tileRef = romBuf[0x098AD2+tileRef]|(romBuf[0x098AD3+tileRef]<<8);
						addObjectTile(o,tilesetBuffer[tileRef>>1],mtOff);
						int mtOff2 = offsetMap16Up(mtOff);
						WORD orig2 = getOriginalMap16Tile(mtOff2);
						if(orig2==tilesetBuffer[0x1C5C>>1] || orig2==tilesetBuffer[0x1C5E>>1]) {
							addObjectTile(o,0,mtOff2);
						}
					}
				} else if((j+1)==height) {
					if(i==0) {
						tileRef = romBuf[0x098A74+tileRef]|(romBuf[0x098A75+tileRef]<<8);
						addObjectTile(o,tilesetBuffer[tileRef>>1],mtOff);
					} else if((i+1)==width) {
						tileRef = romBuf[0x098C4A+tileRef]|(romBuf[0x098C4B+tileRef]<<8);
						addObjectTile(o,tilesetBuffer[tileRef>>1],mtOff);
					} else {
						tileRef = romBuf[0x098B30+tileRef]|(romBuf[0x098B31+tileRef]<<8);
						addObjectTile(o,tilesetBuffer[tileRef>>1],mtOff);
					}
				} else if(i==0) {
					tileRef = romBuf[0x098A16+tileRef]|(romBuf[0x098A17+tileRef]<<8);
					addObjectTile(o,tilesetBuffer[tileRef>>1],mtOff);
				} else if((i+1)==width) {
					tileRef = romBuf[0x098BEC+tileRef]|(romBuf[0x098BED+tileRef]<<8);
					addObjectTile(o,tilesetBuffer[tileRef>>1],mtOff);
				} else {
					addObjectTile(o,tilesetBuffer[0x1C04>>1],mtOff);
				}
				mtOff = offsetMap16Right(mtOff);
			}
			mtOff = preserve = offsetMap16Down(preserve);
		}
	}
}
//Cloud platform
void drawObject_15(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int width = o->data[3]+1;
	for(int i=0; i<width; i++) {
		if(i==0) {
			addObjectTile(o,0x00DB,mtOff);
			int mtOff2 = offsetMap16Down(mtOff);
			addObjectTile(o,0x150F,mtOff2);
		} else if((i+1)==width) {
			addObjectTile(o,0x00DC,mtOff);
			int mtOff2 = offsetMap16Down(mtOff);
			addObjectTile(o,0x1510,mtOff2);
		} else {
			addObjectTile(o,0x00DD,mtOff);
			int mtOff2 = offsetMap16Down(mtOff);
			addObjectTile(o,0x1511,mtOff2);
		}
		mtOff = offsetMap16Right(mtOff);
	}
}
//Transparent water
void drawObject_16(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	for(int j=0; j<height; j++) {
		for(int i=0; i<width; i++) {
			WORD orig = getOriginalMap16Tile(mtOff);
			if(orig==0) addObjectTile(o,0x1600,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Stone with grass
void drawObject_17(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	mtOff = offsetMap16Up(mtOff);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+2;
	for(int j=0; j<height; j++) {
		for(int i=0; i<width; i++) {
			WORD orig = getOriginalMap16Tile(mtOff);
			bool overWater = (orig&0xFF00)==0x1600;
			if(j==0) {
				addObjectTile(o,0x0021+(i&1),mtOff);
				if(i==0) {
					int mtOff2 = offsetMap16Left(mtOff);
					WORD orig2 = getOriginalMap16Tile(mtOff2);
					if(orig2==0) addObjectTile(o,0x0020,mtOff2);
				}
				if((i+1)==width) {
					int mtOff2 = offsetMap16Right(mtOff);
					WORD orig2 = getOriginalMap16Tile(mtOff2);
					if(orig2==0) addObjectTile(o,0x0023,mtOff2);
				}
			} else if((j+1)==height) {
				if(i==0) {
					addObjectTile(o,overWater?0x0137:0x013A,mtOff);
				} else if((i+1)==width) {
					addObjectTile(o,overWater?0x0139:0x013C,mtOff);
				} else {
					addObjectTile(o,overWater?0x0138:0x013B,mtOff);
				}
			} else if(j==1) {
				addObjectTile(o,0x011A+(i&1),mtOff);
				if(i==0) {
					int mtOff2 = offsetMap16Left(mtOff);
					WORD orig2 = getOriginalMap16Tile(mtOff2);
					if(orig2==0) addObjectTile(o,0x001F,mtOff2);
				}
				if((i+1)==width) {
					int mtOff2 = offsetMap16Right(mtOff);
					WORD orig2 = getOriginalMap16Tile(mtOff2);
					if(orig2==0) addObjectTile(o,0x0024,mtOff2);
				}
			} else {
				if(i==0) {
					addObjectTile(o,overWater?0x0122:0x011C,mtOff);
				} else if((i+1)==width) {
					addObjectTile(o,overWater?0x0124:0x011E,mtOff);
				} else {
					addObjectTile(o,overWater?0x0123:0x011D,mtOff);
				}
			}
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Stone with grass
void drawObject_18(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	for(int j=0; j<height; j++) {
		for(int i=0; i<width; i++) {
			WORD orig = getOriginalMap16Tile(mtOff);
			bool overWater = (orig&0xFF00)==0x1600;
			WORD base = overWater?0x012E:0x0125;
			int offset = 4;
			if(j==0) offset -=3;
			else if((j+1)==height) offset += 3;
			if(i==0) offset--;
			else if((i+1)==width) offset++;
			addObjectTile(o,base+offset,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Water with rock background
void drawObject_19(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	for(int j=0; j<height; j++) {
		for(int i=0; i<width; i++) {
			WORD tile = 0x1601+(i&3)+((j&3)<<2);
			if(j>=3) tile = 0x160D+(i&3)+(((j-3)&1)<<2);
			addObjectTile(o,tile,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Horizontal log platform
void drawObject_1A(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int width = o->data[3]+1;
	for(int i=0; i<width; i++) {
		if(i==0) {
			addObjectTile(o,0x1505,mtOff);
		} else if((i+1)==width) {
			addObjectTile(o,0x1506,mtOff);
		} else {
			WORD orig = getOriginalMap16Tile(mtOff);
			if(orig==0x0019) addObjectTile(o,0x1509,mtOff);
			else addObjectTile(o,0x1501+(i&1),mtOff);
		}
		mtOff = offsetMap16Right(mtOff);
	}
}
//Vertical log platform
void drawObject_1B(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int height = o->data[3]+1;
	for(int j=0; j<height; j++) {
		if(j==0) {
			addObjectTile(o,0x1500,mtOff);
		} else if((j+1)==height) {
			addObjectTile(o,0x001A,mtOff);
		} else {
			WORD orig = getOriginalMap16Tile(mtOff);
			if(orig==0x1501 || orig==0x1502) addObjectTile(o,0x1509,mtOff);
			else addObjectTile(o,0x0019,mtOff);
		}
		mtOff = offsetMap16Down(mtOff);
	}
}
//Vertical tied log platform
void drawObject_1C(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int height = o->data[3]+1;
	for(int j=0; j<height; j++) {
		if(j==0) {
			addObjectTile(o,0x1507,mtOff);
			int mtOff2 = offsetMap16Right(mtOff);
			addObjectTile(o,0x1508,mtOff2);
		} else if((j+1)==height) {
			addObjectTile(o,0x1503,mtOff);
			int mtOff2 = offsetMap16Right(mtOff);
			addObjectTile(o,0x1504,mtOff2);
		} else {
			addObjectTile(o,0x001B,mtOff);
			int mtOff2 = offsetMap16Right(mtOff);
			addObjectTile(o,0x001C,mtOff2);
		}
		mtOff = offsetMap16Down(mtOff);
	}
}
//Red mushroom/white flower decoration
void drawObject_1D(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int width = o->data[3]+1;
	for(int i=0; i<width; i++) {
		addObjectTile(o,o->data[0],mtOff);
		mtOff = offsetMap16Right(mtOff);
	}
}
//Wavy lava
void drawObject_1F(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	for(int j=0; j<height; j++) {
		for(int i=0; i<width; i++) {
			if(j<5) {
				int offset = (j<<1)+((i&1)*10);
				WORD tile = romBuf[0x098FAD+offset]|(romBuf[0x098FAE +offset]<<8);
				addObjectTile(o,tile,mtOff);
			} else {
				addObjectTile(o,0x7E04,mtOff);
			}
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//3D stone platform
void drawObject_20(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	for(int j=0; j<height; j++) {
		for(int i=0; i<width; i++) {
			bool rhs = (mtOff&1)^((mtOff>>8)&1);
			if(j==0) {
				if((i==0 && rhs) || (i!=0 && (i+1)==width && !rhs)) addObjectTile(o,0x002D,mtOff);
				else addObjectTile(o,rhs?0x0029:0x0028,mtOff);
			} else if(j==1) {
				if((i==0 && !rhs) || (i!=0 && (i+1)==width && rhs)) addObjectTile(o,0x010A,mtOff);
				else addObjectTile(o,rhs?0x0100:0x0101,mtOff);
			} else if(j==2) {
				if((i==0 && rhs) || (i!=0 && (i+1)==width && !rhs)) addObjectTile(o,0x0105,mtOff);
				else addObjectTile(o,rhs?0x0104:0x0103,mtOff);
			} else {
				if((i==0 && rhs) || ((i+1)==width && !rhs)) addObjectTile(o,0x0106,mtOff);
				else addObjectTile(o,rhs?0x0109:0x0108,mtOff);
			}
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Mud ground with grass
void drawObject_21(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	for(int i=0; i<width; i++) {
		int seed = noiseTilemap[mtOff]&3;
		for(int j=0; j<height; j++) {
			if(j<3) {
				WORD orig = getOriginalMap16Tile(mtOff);
				if(orig!=0x9072 && orig!=0x9073 && orig!=0x907F &&
				orig!=0x908F && orig!=0x90A2 && orig!=0x90A3) {
					int offset = j<<1;
					WORD tile = romBuf[0x098FDB+offset]|(romBuf[0x098FDC+offset]<<8);
					if((orig&0xFF00)==0x9400 || (orig&0xFF00)==0x9500) {
						if(j==0) {
							if(i==0) {
								addObjectTile(o,0x9500,mtOff);
								mtOff = offsetMap16Down(mtOff);
								addObjectTile(o,0x90A3,mtOff);
								mtOff = offsetMap16Down(mtOff);
								addObjectTile(o,0x9073,mtOff);
								j = 2;
							} else if((i+1)==width) {
								addObjectTile(o,0x9402,mtOff);
								mtOff = offsetMap16Down(mtOff);
								addObjectTile(o,0x90A2,mtOff);
								mtOff = offsetMap16Down(mtOff);
								addObjectTile(o,0x9072,mtOff);
								j = 2;
							} else {
								addObjectTile(o,tile+seed,mtOff);
							}
						} else if(j==1) {
							if(i==0) {
								addObjectTile(o,0x330D,mtOff);
								int mtOff2 = offsetMap16Up(mtOff);
								addObjectTile(o,0x9204,mtOff2);
								mtOff2 = offsetMap16Left(mtOff);
								addObjectTile(o,0x964D,mtOff2);
								mtOff = offsetMap16Down(mtOff);
								addObjectTile(o,0x908F,mtOff);
								j = 3;
							} else if((i+1)==width) {
								addObjectTile(o,0x3512,mtOff);
								int mtOff2 = offsetMap16Up(mtOff);
								addObjectTile(o,0x9205,mtOff2);
								mtOff2 = offsetMap16Right(mtOff);
								addObjectTile(o,0x964E,mtOff2);
								mtOff = offsetMap16Down(mtOff);
								addObjectTile(o,0x907F,mtOff);
								j = 3;
							} else {
								addObjectTile(o,tile+seed,mtOff);
							}
						}
					}
					else addObjectTile(o,tile+seed,mtOff);
				}
			} else {
				int offset = noiseTilemap[mtOff]&0x1E;
				WORD tile = romBuf[0x098FE1+offset]|(romBuf[0x098FE2+offset]<<8);
				addObjectTile(o,tile,mtOff);
			}
			mtOff = offsetMap16Down(mtOff);
		}
		mtOff = preserve = offsetMap16Right(preserve);
	}
}
//Left edge of mud ground with grass
void drawObject_22(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int height = o->data[3]+1;
	for(int j=0; j<height; j++) {
		int mtOff2 = offsetMap16Right(mtOff);
		if(j==0) {
			addObjectTile(o,0x9204,mtOff2);
		} else if(j==1) {
			addObjectTile(o,0x964D,mtOff);
			addObjectTile(o,0x330D,mtOff2);
		} else if(j==2) {
			addObjectTile(o,0x909C,mtOff2);
		} else {
			int offset = noiseTilemap[mtOff]&1;
			WORD tile = 0x909E +offset;
			WORD orig = getOriginalMap16Tile(mtOff);
			if((orig&0xFFFC)==0x9200) tile = 0x90A0;
			else if((orig&0xFFFC)==0x9080) tile = 0x90A2;
			else if((orig&0xFFFC)==0x9090) tile = 0x9072;
			addObjectTile(o,tile,mtOff2);
		}
		mtOff = offsetMap16Down(mtOff);
	}
}
//Right edge of mud ground with grass
void drawObject_23(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int height = o->data[3]+1;
	for(int j=0; j<height; j++) {
		if(j==0) {
			addObjectTile(o,0x9205,mtOff);
		} else if(j==1) {
			addObjectTile(o,0x3512,mtOff);
			int mtOff2 = offsetMap16Right(mtOff);
			addObjectTile(o,0x964E,mtOff2);
		} else if(j==2) {
			addObjectTile(o,0x909D,mtOff);
		} else {
			int offset = noiseTilemap[mtOff]&1;
			WORD tile = 0x9062+offset;
			WORD orig = getOriginalMap16Tile(mtOff);
			if((orig&0xFFFC)==0x9200) tile = 0x90A1;
			else if((orig&0xFFFC)==0x9080) tile = 0x90A3;
			else if((orig&0xFFFC)==0x9090) tile = 0x9073;
			addObjectTile(o,tile,mtOff);
		}
		mtOff = offsetMap16Down(mtOff);
	}
}
//Mud ground
void drawObject_24(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	for(int j=0; j<height; j++) {
		for(int i=0; i<width; i++) {
			if(j<2) {
				int offset = j<<1;
				WORD tile = romBuf[0x099224+offset]|(romBuf[0x099225+offset]<<8);
				addObjectTile(o,tile+(noiseTilemap[mtOff]&3),mtOff);
			} else {
				int offset = noiseTilemap[mtOff]&0x1E;
				WORD tile = romBuf[0x098FE1+offset]|(romBuf[0x098FE2+offset]<<8);
				addObjectTile(o,tile,mtOff);
			}
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Left edge of mud ground
void drawObject_25(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int height = o->data[3]+1;
	for(int j=0; j<height; j++) {
		if(j==0) {
			addObjectTile(o,0x9400,mtOff);
			int mtOff2 = offsetMap16Right(mtOff);
			WORD orig2 = getOriginalMap16Tile(mtOff2);
			if((orig2&0xFFFC)==0x9090) {
				addObjectTile(o,0x908F,mtOff2);
				mtOff2 = offsetMap16Up(mtOff2);
				addObjectTile(o,0x330D,mtOff2);
				mtOff2 = offsetMap16Up(mtOff2);
				addObjectTile(o,0x9204,mtOff2);
				mtOff2 = offsetMap16Up(mtOff);
				addObjectTile(o,0x964D,mtOff2);
			}
		} else {
			int offset = noiseTilemap[mtOff]&1;
			WORD tile = 0x909E +offset;
			WORD orig = getOriginalMap16Tile(mtOff);
			if((orig&0xFFFC)==0x9200) tile = 0x90A0;
			else if((orig&0xFFFC)==0x9080) tile = 0x90A2;
			else if((orig&0xFFFC)==0x9090) tile = 0x9072;
			addObjectTile(o,tile,mtOff);
		}
		mtOff = offsetMap16Down(mtOff);
	}
}
//Right edge of mud ground
void drawObject_26(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int height = o->data[3]+1;
	for(int j=0; j<height; j++) {
		if(j==0) {
			addObjectTile(o,0x9502,mtOff);
			int mtOff2 = offsetMap16Left(mtOff);
			WORD orig2 = getOriginalMap16Tile(mtOff2);
			if((orig2&0xFFFC)==0x9090) {
				addObjectTile(o,0x907F,mtOff2);
				mtOff2 = offsetMap16Up(mtOff2);
				addObjectTile(o,0x3512,mtOff2);
				mtOff2 = offsetMap16Up(mtOff2);
				addObjectTile(o,0x9205,mtOff2);
				mtOff2 = offsetMap16Up(mtOff);
				addObjectTile(o,0x964E,mtOff2);
			}
		} else {
			int offset = noiseTilemap[mtOff]&1;
			WORD tile = 0x9062+offset;
			WORD orig = getOriginalMap16Tile(mtOff);
			if((orig&0xFFFC)==0x9200) tile = 0x90A1;
			else if((orig&0xFFFC)==0x9080) tile = 0x90A3;
			else if((orig&0xFFFC)==0x9090) tile = 0x9073;
			addObjectTile(o,tile,mtOff);
		}
		mtOff = offsetMap16Down(mtOff);
	}
}
//Mud ground steep slope, left
void drawObject_27(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = (0x101-o->data[3])&0xFF;
	int height = o->data[4]+1;
	for(int i=0; i<width; i++) {
		for(int j=0; j<height; j++) {
			if(j<2) {
				int offset = j<<1;
				WORD tile = romBuf[0x099348+offset]|(romBuf[0x099349+offset]<<8);
				tile += noiseTilemap[mtOff]&1;
				WORD orig = getOriginalMap16Tile(mtOff);
				if(i==0) {
					if((orig&0xFFFC)==0x9080) {
						int mtOff2 = offsetMap16Up(mtOff);
						addObjectTile(o,0x9204,mtOff2);
						mtOff2 = offsetMap16Left(mtOff);
						addObjectTile(o,0x964D,mtOff2);
						tile = 0x330D;
					} else if((orig&0xFFFC)==0x9090) tile = 0x908F;
				} else if((i+1)==width) {
					if((orig&0xFFFC)==0x9200) tile = 0x9402;
					else if((orig&0xFFFC)==0x9080) tile = 0x90A2;
					else if((orig&0xFFFC)==0x9090) tile = 0x9072;
				}
				addObjectTile(o,tile,mtOff);
			} else if(j==2) {
				int offset = noiseTilemap[mtOff]&0x1E;
				WORD tile = romBuf[0x098FE1+offset]|(romBuf[0x098FE2+offset]<<8);
				WORD orig = getOriginalMap16Tile(mtOff);
				if((orig&0xFFFC)==0x9200) tile = 0x9402;
				else if((orig&0xFFFC)==0x9080) tile = 0x90A2;
				else if((orig&0xFFFC)==0x9090) tile = 0x9072;
				addObjectTile(o,tile,mtOff);
			} else {
				int offset = noiseTilemap[mtOff]&0x1E;
				WORD tile = romBuf[0x098FE1+offset]|(romBuf[0x098FE2+offset]<<8);
				addObjectTile(o,tile,mtOff);
			}
			mtOff = offsetMap16Down(mtOff);
		}
		mtOff = preserve = offsetMap16Left(preserve);
		mtOff = preserve = offsetMap16Down(preserve);
		height--;
	}
}
//Mud ground steep slope, right
void drawObject_28(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	for(int i=0; i<width; i++) {
		for(int j=0; j<height; j++) {
			if(j<2) {
				int offset = j<<1;
				WORD tile = romBuf[0x0993E0+offset]|(romBuf[0x0993E1+offset]<<8);
				tile += noiseTilemap[mtOff]&1;
				WORD orig = getOriginalMap16Tile(mtOff);
				if(i==0) {
					if((orig&0xFFFC)==0x9080) {
						int mtOff2 = offsetMap16Up(mtOff);
						addObjectTile(o,0x9205,mtOff2);
						mtOff2 = offsetMap16Right(mtOff);
						addObjectTile(o,0x964E,mtOff2);
						tile = 0x3512;
					} else if((orig&0xFFFC)==0x9090) tile = 0x907F;
				} else if((i+1)==width) {
					if((orig&0xFFFC)==0x9200) tile = 0x9500;
					else if((orig&0xFFFC)==0x9080) tile = 0x90A3;
					else if((orig&0xFFFC)==0x9090) tile = 0x9073;
				}
				addObjectTile(o,tile,mtOff);
			} else if(j==2) {
				int offset = noiseTilemap[mtOff]&0x1E;
				WORD tile = romBuf[0x098FE1+offset]|(romBuf[0x098FE2+offset]<<8);
				WORD orig = getOriginalMap16Tile(mtOff);
				if((orig&0xFFFC)==0x9200) tile = 0x9500;
				else if((orig&0xFFFC)==0x9080) tile = 0x90A3;
				else if((orig&0xFFFC)==0x9090) tile = 0x9073;
				addObjectTile(o,tile,mtOff);
			} else {
				int offset = noiseTilemap[mtOff]&0x1E;
				WORD tile = romBuf[0x098FE1+offset]|(romBuf[0x098FE2+offset]<<8);
				addObjectTile(o,tile,mtOff);
			}
			mtOff = offsetMap16Down(mtOff);
		}
		mtOff = preserve = offsetMap16Right(preserve);
		mtOff = preserve = offsetMap16Down(preserve);
		height--;
	}
}
//Large leaf cover, left
void drawObject_29(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = 0x101-o->data[3];
	int height = 0x101-o->data[4];
	int base = 0;
	for(int i=0; i<width; i++) {
		if((i&1)==0) {
			base = noiseTilemap[mtOff]&2;
			base = romBuf[0x0994C8+base]|(romBuf[0x0994C9+base]<<8);
			base += 0x090000;
		}
		for(int j=0; j<height; j++) {
			if(j<4) {
				int offset = ((i&1)<<1)+(j<<2);
				WORD tile = romBuf[base+offset]|(romBuf[base+offset+1]<<8);
				addObjectTile(o,tile,mtOff);
			} else {
				addObjectTile(o,0x961B,mtOff);
			}
			mtOff = offsetMap16Up(mtOff);
		}
		mtOff = preserve = offsetMap16Left(preserve);
		if(i&1) {
			mtOff = preserve = offsetMap16Up(preserve);
			mtOff = preserve = offsetMap16Up(preserve);
			height -= 2;
		}
	}
}
//Large leaf cover, left
void drawObject_2A(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = 0x101-o->data[4];
	int base = 0;
	for(int i=0; i<width; i++) {
		if((i&1)==0) {
			base = noiseTilemap[mtOff]&2;
			base = romBuf[0x0994CC+base]|(romBuf[0x0994CD+base]<<8);
			base += 0x090000;
		}
		for(int j=0; j<height; j++) {
			if(j<4) {
				int offset = ((i&1)<<1)+(j<<2);
				WORD tile = romBuf[base+offset]|(romBuf[base+offset+1]<<8);
				addObjectTile(o,tile,mtOff);
			} else {
				addObjectTile(o,0x961B,mtOff);
			}
			mtOff = offsetMap16Up(mtOff);
		}
		mtOff = preserve = offsetMap16Right(preserve);
		if(i&1) {
			mtOff = preserve = offsetMap16Up(preserve);
			mtOff = preserve = offsetMap16Up(preserve);
			height -= 2;
		}
	}
}
//Stone pillar
void drawObject_2C(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int height = o->data[3]+1;
	for(int j=0; j<height; j++) {
		if(j==0) {
			addObjectTile(o,0x330E,mtOff);
			int mtOff2 = offsetMap16Right(mtOff);
			addObjectTile(o,0x3511,mtOff2);
		} else {
			WORD tile = 0x90DA+(noiseTilemap[mtOff]&6);
			addObjectTile(o,tile,mtOff);
			int mtOff2 = offsetMap16Right(mtOff);
			addObjectTile(o,tile+1,mtOff2);
		}
		mtOff = offsetMap16Down(mtOff);
	}
}
//Hanging vine on mud
void drawObject_2D(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int height = o->data[3]+1;
	int seed = noiseTilemap[mtOff]&1;
	for(int j=0; j<height; j++) {
		if((height-j)<3) {
			int offset = (seed<<2)+((height-j-1)<<1);
			WORD tile = romBuf[0x09957A+offset]|(romBuf[0x09957B+offset]<<8);
			addObjectTile(o,tile,mtOff);
		} else if(j<4) {
			int offset = (seed*0x1C)+(j<<1);
			WORD tile = romBuf[0x0995C6+offset]|(romBuf[0x0995C7+offset]<<8);
			addObjectTile(o,tile,mtOff);
		} else {
			int offset = ((j+seed)&1)<<1;
			WORD tile = romBuf[0x099586+offset]|(romBuf[0x099587+offset]<<8);
			addObjectTile(o,tile,mtOff);
		}
		mtOff = offsetMap16Down(mtOff);
	}
}
//Hanging vine on mud, with leaves
void drawObject_2E(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int height = o->data[3]+1;
	int seed = noiseTilemap[mtOff]&1;
	for(int j=0; j<height; j++) {
		if((height-j)<3) {
			int offset = (seed<<2)+((height-j-1)<<1);
			WORD tile = romBuf[0x09957A+offset]|(romBuf[0x09957B+offset]<<8);
			addObjectTile(o,tile,mtOff);
		} else if(j<4) {
			int offset = (seed*0x1C)+(j<<1);
			WORD tile = romBuf[0x0995C6+offset]|(romBuf[0x0995C7+offset]<<8);
			addObjectTile(o,tile,mtOff);
		} else {
			if(noiseTilemap[mtOff]&1) {
				if((j+seed)&1) {
					addObjectTile(o,0x9089,mtOff);
					int mtOff2 = offsetMap16Right(mtOff);
					addObjectTile(o,0x908A,mtOff2);
				} else {
					addObjectTile(o,0x907B,mtOff);
					int mtOff2 = offsetMap16Left(mtOff);
					addObjectTile(o,0x907A,mtOff2);
				}
			} else {
				int offset = ((j+seed)&1)<<1;
				WORD tile = romBuf[0x099586+offset]|(romBuf[0x099587+offset]<<8);
				addObjectTile(o,tile,mtOff);
			}
		}
		mtOff = offsetMap16Down(mtOff);
	}
}
//Jungle tree
void drawObject_2F(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int height = o->data[3]+1;
	for(int j=0; j<height; j++) {
		if(j<2) {
			int offset = j<<1;
			WORD tile = romBuf[0x099656+offset]|(romBuf[0x099657+offset]<<8);
			addObjectTile(o,tile,mtOff);
			int mtOff2 = offsetMap16Right(mtOff);
			tile = romBuf[0x09965A+offset]|(romBuf[0x09965B+offset]<<8);
			addObjectTile(o,tile,mtOff2);
			mtOff2 = offsetMap16Left(mtOff);
			tile = romBuf[0x099652+offset]|(romBuf[0x099653+offset]<<8);
			addObjectTile(o,tile,mtOff2);
		} else if((j+1)==height) {
			addObjectTile(o,0x9206,mtOff);
		} else if(j==2) {
			addObjectTile(o,0x990A,mtOff);
		} else {
			addObjectTile(o,0x990B,mtOff);
		}
		mtOff = offsetMap16Down(mtOff);
	}
}
//Vine
void drawObject_30(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int height = o->data[3]+1;
	int seed = noiseTilemap[mtOff]&1;
	for(int j=0; j<height; j++) {
		if((j+1)==height) {
			WORD orig = getOriginalMap16Tile(mtOff);
			if((orig&0xFF00)==0x9200) {
				if(orig>=0x920F) {
					int offset = (orig-0x920F)<<1;
					WORD tile = romBuf[0x0996AA+offset]|(romBuf[0x0996AB+offset]<<8);
					addObjectTile(o,tile,mtOff);
				} else addObjectTile(o,0x9215,mtOff);
			} else addObjectTile(o,0x00AC+(noiseTilemap[mtOff]&1)+(seed?2:0),mtOff);
		} else {
			WORD orig = getOriginalMap16Tile(mtOff);
			if(j==0) {
				if(orig>=0x9B00 && orig<0x9B04) {
					mtOff = offsetMap16Down(mtOff);
					continue;
				}
				int mtOff2 = offsetMap16Up(mtOff);
				WORD orig2 = getOriginalMap16Tile(mtOff2);
				if(orig2==0x963B) addObjectTile(o,0x9B04,mtOff2);
				else if(orig2==0x963C) addObjectTile(o,0x9B05,mtOff2);
				else if(orig2==0x960E) addObjectTile(o,0x9B06,mtOff2);
				else if(orig2==0x961D) addObjectTile(o,0x9B07,mtOff2);
			} 
			if(orig==0x960F) addObjectTile(o,0x9900,mtOff);
			else if(orig==0x961C) addObjectTile(o,0x9901,mtOff);
			else addObjectTile(o,0x9908+(noiseTilemap[mtOff]&1)+(seed?0xB:0),mtOff);
		}
		mtOff = offsetMap16Down(mtOff);
	}
}
//Vine with leaves
void drawObject_31(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int height = o->data[3]+1;
	int seed = noiseTilemap[mtOff]&1;
	for(int j=0; j<height; j++) {
		if((j+1)==height) {
			WORD orig = getOriginalMap16Tile(mtOff);
			if((orig&0xFF00)==0x9200) {
				if(orig>=0x9210) {
					int offset = (orig-0x9210)<<1;
					WORD tile = romBuf[0x0996AA+offset]|(romBuf[0x0996AB+offset]<<8);
					addObjectTile(o,tile,mtOff);
				} else addObjectTile(o,0x9216,mtOff);
			} else addObjectTile(o,0x00AC+(noiseTilemap[mtOff]&1)+(seed?2:0),mtOff);
		} else {
			WORD orig = getOriginalMap16Tile(mtOff);
			if(j==0) {
				if(orig>=0x9B00 && orig<0x9B04) {
					mtOff = offsetMap16Down(mtOff);
					continue;
				}
				int mtOff2 = offsetMap16Up(mtOff);
				WORD orig2 = getOriginalMap16Tile(mtOff2);
				if(orig2==0x963B) addObjectTile(o,0x9B04,mtOff2);
				else if(orig2==0x963C) addObjectTile(o,0x9B05,mtOff2);
				else if(orig2==0x960E) addObjectTile(o,0x9B06,mtOff2);
				else if(orig2==0x961D) addObjectTile(o,0x9B07,mtOff2);
			} 
			if(orig==0x960F) addObjectTile(o,0x9900,mtOff);
			else if(orig==0x961C) addObjectTile(o,0x9901,mtOff);
			else {
				if(j==0 || j==1) addObjectTile(o,0x9908+(noiseTilemap[mtOff]&1)+(seed?0xB:0),mtOff);
				else {
					int mode = noiseTilemap[mtOff]&6;
					if(mode==0) addObjectTile(o,0x9908+(noiseTilemap[mtOff]&1)+(seed?0xB:0),mtOff);
					else {
						addObjectTile(o,0x9900+(noiseTilemap[mtOff]&7)+(seed?0xB:0),mtOff);
						if(mode==2 || mode==4) {
							int mtOff2 = offsetMap16Left(mtOff);
							addObjectTile(o,(noiseTilemap[mtOff]&1)?0x9674:0x9672,mtOff2);
						}
						if(mode==2 || mode==6) {
							int mtOff2 = offsetMap16Right(mtOff);
							addObjectTile(o,(noiseTilemap[mtOff]&1)?0x9675:0x9673,mtOff2);
						}
					}
				}
			}
		}
		mtOff = offsetMap16Down(mtOff);
	}
}
//Stone with moss
void drawObject_32(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	for(int j=0; j<height; j++) {
		for(int i=0; i<width; i++) {
			if(j==0) {
				if(i==0) addObjectTile(o,0x90A8,mtOff);
				else if((i+1)==width) addObjectTile(o,0x90A9,mtOff);
				else addObjectTile(o,(noiseTilemap[mtOff]&1)+0x90BE,mtOff);
			} else if((j+1)==height) {
				WORD orig = getOriginalMap16Tile(mtOff);
				int tileRef = 0x90AE;
				int tileRef2 = 0x90B2;
				if((orig&0xFF00)==0x9200) {
					tileRef = 0x90CC;
					tileRef2 = 0x90CE;
				}
				if(i==0) addObjectTile(o,tileRef,mtOff);
				else if((i+1)==width) addObjectTile(o,tileRef+1,mtOff);
				else addObjectTile(o,(noiseTilemap[mtOff]&3)+tileRef2,mtOff);
			} else {
				if(i==0) {
					WORD tile = (noiseTilemap[mtOff]&3)+0x90B6;
					int mtOff2 = offsetMap16Left(mtOff);
					WORD orig2 = getOriginalMap16Tile(mtOff2);
					if(orig2>=0x90C4 && orig2<0x90C8) tile += 4;
					addObjectTile(o,tile,mtOff);
				} else if((i+1)==width) {
					WORD tile = (noiseTilemap[mtOff]&3)+0x90C4;
					int mtOff2 = offsetMap16Right(mtOff);
					WORD orig2 = getOriginalMap16Tile(mtOff2);
					if(orig2>=0x90B6 && orig2<0x90BA) tile += 4;
					addObjectTile(o,tile,mtOff);
				} else addObjectTile(o,(noiseTilemap[mtOff]&7)+0x90D2,mtOff);
			}
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Stone without moss
void drawObject_33(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	for(int j=0; j<height; j++) {
		for(int i=0; i<width; i++) {
			if(j==0) {
				if(i==0) addObjectTile(o,0x90AA,mtOff);
				else if((i+1)==width) addObjectTile(o,0x90AB,mtOff);
				else addObjectTile(o,(noiseTilemap[mtOff]&1)+0x90C0,mtOff);
			} else if((j+1)==height) {
				WORD orig = getOriginalMap16Tile(mtOff);
				int tileRef = 0x90AE;
				int tileRef2 = 0x90B2;
				if((orig&0xFF00)==0x9200) {
					tileRef = 0x90CC;
					tileRef2 = 0x90CE;
				}
				if(i==0) addObjectTile(o,tileRef,mtOff);
				else if((i+1)==width) addObjectTile(o,tileRef+1,mtOff);
				else addObjectTile(o,(noiseTilemap[mtOff]&3)+tileRef2,mtOff);
			} else {
				if(i==0) {
					WORD tile = (noiseTilemap[mtOff]&3)+0x90B6;
					int mtOff2 = offsetMap16Left(mtOff);
					WORD orig2 = getOriginalMap16Tile(mtOff2);
					if(orig2>=0x90C4 && orig2<0x90C8) tile += 4;
					addObjectTile(o,tile,mtOff);
				} else if((i+1)==width) {
					WORD tile = (noiseTilemap[mtOff]&3)+0x90C4;
					int mtOff2 = offsetMap16Right(mtOff);
					WORD orig2 = getOriginalMap16Tile(mtOff2);
					if(orig2>=0x90B6 && orig2<0x90BA) tile += 4;
					addObjectTile(o,tile,mtOff);
				} else addObjectTile(o,(noiseTilemap[mtOff]&7)+0x90D2,mtOff);
			}
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Cattail
void drawObject_34(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int width = o->data[3]+1;
	for(int i=0; i<width; i++) {
		int offset = noiseTilemap[mtOff]&0x3C;
		WORD tile = romBuf[0x09992A+offset]|(romBuf[0x09992B+offset]<<8);
		if(tile) addObjectTile(o,tile,mtOff);
		int mtOff2 = offsetMap16Down(mtOff);
		tile = romBuf[0x09992C+offset]|(romBuf[0x09992D+offset]<<8);
		addObjectTile(o,tile,mtOff2);
		mtOff = offsetMap16Right(mtOff);
	}
}
//Animated water
void drawObject_35(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	int seed = 0;
	for(int i=0; i<width; i++) {
		if((i&1)==0) seed = noiseTilemap[mtOff]&8;
		for(int j=0; j<height; j++) {
			WORD tile = 0x1628;
			if(j<2) {
				int offset = (j<<2)+((i&1)<<1);
				tile = romBuf[0x0999D9+seed+offset]|(romBuf[0x0999DA+seed+offset]<<8);
			}
			WORD orig = getOriginalMap16Tile(mtOff);
			if((orig&0xFF00)==0x6B00 || (orig&0xFF00)==0x9000 || (orig&0xFF00)==0x9300) {
				if(j==0) tile = 0x9061;
				else if(j==1) {
					int offset = noiseTilemap[mtOff]&6;
					tile = romBuf[0x0999F1+offset]|(romBuf[0x0999F2+offset]<<8);
				}
				else tile = 0x909B;
			} else if((orig&0xFF00)==0x9400) tile = j?0x9701:0x9700;
			else if((orig&0xFF00)==0x9500) tile = j?0x9801:0x9800;
			addObjectTile(o,tile,mtOff);
			mtOff = offsetMap16Down(mtOff);
		}
		mtOff = preserve = offsetMap16Right(preserve);
	}
}
//Red platform, horizontal
void drawObject_37(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int width = o->data[3]+1;
	for(int i=0; i<width; i++) {
		addObjectTile(o,0x1512,mtOff);
		mtOff = offsetMap16Right(mtOff);
	}
}
//Stone in Hookbill's stage
void drawObject_38(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	int seed = (noiseTilemap[mtOff]&1)*0x18;
	for(int j=0; j<height; j++) {
		for(int i=0; i<width; i++) {
			int offset = 0xC;
			if(j==0) offset -= 8;
			else if((j+1)==height) offset += 8;
			if(i==0) offset -= 4;
			else if((i+1)==width) offset += 2;
			else if(i&1) offset -= 2;
			WORD tile = romBuf[0x099A9B+seed+offset]|(romBuf[0x099A9C+seed+offset]<<8);
			if(j==0) {
				if(i==0) {
					int mtOff2 = offsetMap16Up(mtOff);
					int mtOff3 = offsetMap16Left(mtOff);
					int mtOff4 = offsetMap16Up(mtOff3);
					overlapTile_hookbillStone4(o,mtOff,&tile,mtOff2,mtOff3,mtOff4);
				} else if((i+1)==width) {
					int mtOff2 = offsetMap16Up(mtOff);
					int mtOff3 = offsetMap16Right(mtOff);
					int mtOff4 = offsetMap16Up(mtOff3);
					overlapTile_hookbillStone4(o,mtOff,&tile,mtOff2,mtOff3,mtOff4);
				} else {
					int mtOff2 = offsetMap16Up(mtOff);
					int mtOff3 = offsetMap16Left(mtOff2);
					int mtOff4 = offsetMap16Right(mtOff2);
					overlapTile_hookbillStone2(o,mtOff,&tile,mtOff2,mtOff3,mtOff4);
				}
			} else if((j+1)==height) {
				if(i==0) {
					int mtOff2 = offsetMap16Down(mtOff);
					int mtOff3 = offsetMap16Left(mtOff);
					int mtOff4 = offsetMap16Down(mtOff3);
					overlapTile_hookbillStone4(o,mtOff,&tile,mtOff2,mtOff3,mtOff4);
				} else if((i+1)==width) {
					int mtOff2 = offsetMap16Down(mtOff);
					int mtOff3 = offsetMap16Right(mtOff);
					int mtOff4 = offsetMap16Down(mtOff3);
					overlapTile_hookbillStone4(o,mtOff,&tile,mtOff2,mtOff3,mtOff4);
				} else {
					int mtOff2 = offsetMap16Down(mtOff);
					int mtOff3 = offsetMap16Left(mtOff2);
					int mtOff4 = offsetMap16Right(mtOff2);
					overlapTile_hookbillStone2(o,mtOff,&tile,mtOff2,mtOff3,mtOff4);
				}
			} else if(i==0) {
				int mtOff2 = offsetMap16Left(mtOff);
				int mtOff3 = offsetMap16Up(mtOff2);
				int mtOff4 = offsetMap16Down(mtOff2);
				overlapTile_hookbillStone2(o,mtOff,&tile,mtOff2,mtOff3,mtOff4);
			} else if((i+1)==width) {
				int mtOff2 = offsetMap16Right(mtOff);
				int mtOff3 = offsetMap16Up(mtOff2);
				int mtOff4 = offsetMap16Down(mtOff2);
				overlapTile_hookbillStone2(o,mtOff,&tile,mtOff2,mtOff3,mtOff4);
			}
			addObjectTile(o,tile,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Red stone in Hookbill's stage
void drawObject_39(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	if(width&1) width++;
	int height = 2;
	for(int j=0; j<height; j++) {
		for(int i=0; i<width; i++) {
			int offset = (j<<2)+((i&1)<<1);
			WORD tile = romBuf[0x099F73+offset]|(romBuf[0x099F74+offset]<<8);
			if(j==0) {
				if(i&1) {
					int mtOff2 = offsetMap16Up(mtOff);
					int mtOff3 = offsetMap16Right(mtOff);
					int mtOff4 = offsetMap16Up(mtOff3);
					overlapTile_hookbillStone4(o,mtOff,&tile,mtOff2,mtOff3,mtOff4);
				} else {
					int mtOff2 = offsetMap16Up(mtOff);
					int mtOff3 = offsetMap16Left(mtOff);
					int mtOff4 = offsetMap16Up(mtOff3);
					overlapTile_hookbillStone4(o,mtOff,&tile,mtOff2,mtOff3,mtOff4);
				}
			} else {
				if(i&1) {
					int mtOff2 = offsetMap16Down(mtOff);
					int mtOff3 = offsetMap16Right(mtOff);
					int mtOff4 = offsetMap16Down(mtOff3);
					overlapTile_hookbillStone4(o,mtOff,&tile,mtOff2,mtOff3,mtOff4);
				} else {
					int mtOff2 = offsetMap16Down(mtOff);
					int mtOff3 = offsetMap16Left(mtOff);
					int mtOff4 = offsetMap16Down(mtOff3);
					overlapTile_hookbillStone4(o,mtOff,&tile,mtOff2,mtOff3,mtOff4);
				}
			}
			addObjectTile(o,tile,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Upside-down very steep slope, right
void drawObject_3A(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	for(int i=0; i<width; i++) {
		for(int j=0; j<height; j++) {
			if((j+1)==height) addObjectTile(o,tilesetBuffer[0x1DF4>>1],mtOff);
			else if((j+2)==height) addObjectTile(o,tilesetBuffer[0x1DF0>>1],mtOff);
			else {
				int offset = noiseTilemap[mtOff]&0x0E;
				int tileRef = romBuf[0x098121+offset]|(romBuf[0x098122+offset]<<8);
				addObjectTile(o,tilesetBuffer[tileRef>>1],mtOff);
			}
			mtOff = offsetMap16Down(mtOff);
		}
		mtOff = preserve = offsetMap16Right(preserve);
		height -= 2;
	}
}
//Upside-down very steep slope, left
void drawObject_3B(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]-1-(o->data[3]<<1);
	for(int i=0; i<width; i++) {
		for(int j=0; j<height; j++) {
			if((j+1)==height) addObjectTile(o,tilesetBuffer[0x1DEC>>1],mtOff);
			else if((j+2)==height) addObjectTile(o,tilesetBuffer[0x1DE8>>1],mtOff);
			else {
				int offset = noiseTilemap[mtOff]&0x0E;
				int tileRef = romBuf[0x098121+offset]|(romBuf[0x098122+offset]<<8);
				addObjectTile(o,tilesetBuffer[tileRef>>1],mtOff);
			}
			mtOff = offsetMap16Down(mtOff);
		}
		mtOff = preserve = offsetMap16Right(preserve);
		height += 2;
	}
}
//Vertical pipe, enterable
void drawObject_3C(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int height = o->data[3];
	if(height&0x80) {
		height = 0x101-height;
		for(int j=0; j<height; j++) {
			int mtOff2 = offsetMap16Right(mtOff);
			if(j==0) {
				addObjectTile(o,0x7D0A,mtOff);
				addObjectTile(o,0x7D0B,mtOff2);
			} else if((j+1)==height) {
				addObjectTile(o,0x9D36,mtOff);
				addObjectTile(o,0x9D37,mtOff2);
			} else {
				addObjectTile(o,0x9D32,mtOff);
				addObjectTile(o,0x9D33,mtOff2);
			}
			mtOff = offsetMap16Up(mtOff);
		}
	} else {
		height++;
		for(int j=0; j<height; j++) {
			int mtOff2 = offsetMap16Right(mtOff);
			if(j==0) {
				addObjectTile(o,0x7D08,mtOff);
				addObjectTile(o,0x7D09,mtOff2);
			} else if((j+1)==height) {
				addObjectTile(o,0x9D34,mtOff);
				addObjectTile(o,0x9D35,mtOff2);
			} else {
				addObjectTile(o,0x9D32,mtOff);
				addObjectTile(o,0x9D33,mtOff2);
			}
			mtOff = offsetMap16Down(mtOff);
		}
	}
}
//Unknown
void drawObject_3D(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int width = o->data[3]+1;
	for(int i=0; i<width; i++) {
		int mtOff2 = offsetMap16Down(mtOff);
		int mtOff3 = offsetMap16Down(mtOff2);
		if(i==0) {
			WORD orig = getOriginalMap16Tile(mtOff);
			bool overUnk = (orig==0x00A8 || orig==0x00A9);
			addObjectTile(o,overUnk?0x00A7:0x00B5,mtOff);
			addObjectTile(o,0x3C00,mtOff2);
			addObjectTile(o,0x00AB,mtOff3);
		} else if((i+1)==width) {
			addObjectTile(o,0x00AA,mtOff);
			addObjectTile(o,0x3C03,mtOff2);
			addObjectTile(o,0x00B2,mtOff3);
		} else {
			int offset = (i&1)^1;
			addObjectTile(o,0x00A8+offset,mtOff);
			addObjectTile(o,0x3C01+offset,mtOff2);
			addObjectTile(o,0x00B0+offset,mtOff3);
		}
		mtOff = offsetMap16Right(mtOff);
	}
}
//Ski lift prop, double
void drawObject_3E(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int height = o->data[3]+1;
	for(int j=0; j<height; j++) {
		WORD orig = getOriginalMap16Tile(mtOff);
		if(orig==0x0092 || orig==0x0093 || orig==0x00A6) addObjectTile(o,0x00A7,mtOff);
		else if(j==0) addObjectTile(o,0x00B3,mtOff);
		else if((j+1)==height) addObjectTile(o,tilesetBuffer[0x1C74>>1],mtOff);
		else addObjectTile(o,0x00B4,mtOff);
		mtOff = offsetMap16Down(mtOff);
	}
}
//Vertical spikes
void drawObject_3F(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int height = o->data[3]+1;
	int objRef = (o->data[0]-0x3F)^1;
	for(int j=0; j<height; j++) {
		if(j==0) addObjectTile(o,0x0114+objRef,mtOff);
		else addObjectTile(o,0x2904+objRef,mtOff);
		mtOff = offsetMap16Down(mtOff);
	}
}
//Horizontal platform for castles
void drawObject_41(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int width = o->data[3]+1;
	if(width==1) {
		addObjectTile(o,0x0156,mtOff);
		int mtOff2 = offsetMap16Down(mtOff);
		overlapTile_castleWallShadowDL(o,mtOff2);
		mtOff2 = offsetMap16Right(mtOff2);
		overlapTile_castleWallShadowDR(o,mtOff2);
		mtOff2 = offsetMap16Right(mtOff);
		overlapTile_castleWallShadowUR(o,mtOff2);
	} else {
		for(int i=0; i<width; i++) {
			if(i==0) {
				addObjectTile(o,0x0153,mtOff);
				int mtOff2 = offsetMap16Down(mtOff);
				overlapTile_castleWallShadowDL(o,mtOff2);
			} else if((i+1)==width) {
				addObjectTile(o,0x0155,mtOff);
				int mtOff2 = offsetMap16Down(mtOff);
				overlapTile_castleWallShadowD(o,mtOff2);
				mtOff2 = offsetMap16Right(mtOff2);
				overlapTile_castleWallShadowDR(o,mtOff2);
				mtOff2 = offsetMap16Right(mtOff);
				overlapTile_castleWallShadowUR(o,mtOff2);
			} else {
				addObjectTile(o,0x0154,mtOff);
				int mtOff2 = offsetMap16Down(mtOff);
				overlapTile_castleWallShadowD(o,mtOff2);
			}
			mtOff = offsetMap16Right(mtOff);
		}
	}
}
//Castle pillar
void drawObject_42(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int height = o->data[3]+1;
	for(int j=0; j<height; j++) {
		WORD tile = 0x00B6+((j&1)<<1);
		if((height-j)<4) {
			int offset = (height-j-1)<<1;
			tile = romBuf[0x09A36C+offset]|(romBuf[0x09A36D+offset]<<8);
		}
		WORD orig = getOriginalMap16Tile(mtOff);
		if((orig&0xFF00)!=0x7E00) {
			if(orig>=0x0084 && orig<0x008E) {
				int offset = (tile-0x00B6)<<1;
				tile = romBuf[0x09A3A5+offset]|(romBuf[0x09A3A6+offset]<<8);
			}
			addObjectTile(o,tile,mtOff);
			int mtOff2 = offsetMap16Right(mtOff);
			if(j==0) overlapTile_castleWallShadowUR(o,mtOff2);
			else if((j+1)==height) {
				overlapTile_castleWallShadowR(o,mtOff2);
				mtOff2 = offsetMap16Down(mtOff);
				overlapTile_castleWallShadowDL(o,mtOff2);
				mtOff2 = offsetMap16Right(mtOff2);
				overlapTile_castleWallShadowDR(o,mtOff2);
			} else overlapTile_castleWallShadowR(o,mtOff2);
		}
		mtOff = offsetMap16Down(mtOff);
	}
}
//Castle pillar 2
void drawObject_43(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int height = o->data[3]+1;
	for(int j=0; j<height; j++) {
		WORD tile = 0x00B7;
		if(j==0) tile = 0x00B6;
		else if((j+1)==height) tile = 0x00B8;
		WORD orig = getOriginalMap16Tile(mtOff);
		if((orig&0xFF00)!=0x7E00) {
			if(orig>=0x0084 && orig<0x008E) {
				int offset = (tile-0x00B6)<<1;
				tile = romBuf[0x09A3A5+offset]|(romBuf[0x09A3A6+offset]<<8);
			}
			addObjectTile(o,tile,mtOff);
			int mtOff2 = offsetMap16Right(mtOff);
			if(j==0) overlapTile_castleWallShadowUR(o,mtOff2);
			else if((j+1)==height) {
				overlapTile_castleWallShadowR(o,mtOff2);
				mtOff2 = offsetMap16Down(mtOff);
				overlapTile_castleWallShadowDL(o,mtOff2);
				mtOff2 = offsetMap16Right(mtOff2);
				overlapTile_castleWallShadowDR(o,mtOff2);
			} else overlapTile_castleWallShadowR(o,mtOff2);
		}
		mtOff = offsetMap16Down(mtOff);
	}
}
//Castle wall
void drawObject_44(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	bool shadowFlag = false;
	for(int i=0; i<width; i++) {
		for(int j=0; j<height; j++) {
			WORD tile = 0x00C2;
			if(i==0) {
				int mtOff2 = offsetMap16Left(mtOff);
				WORD orig2 = getOriginalMap16Tile(mtOff2);
				if(orig2==0x015A) addObjectTile(o,0x0152,mtOff2);
			} else if((i+1)==width) {
				int mtOff2 = offsetMap16Right(mtOff);
				WORD orig2 = getOriginalMap16Tile(mtOff2);
				if(orig2==0x015B) addObjectTile(o,0x0151,mtOff2);
			}
			if(i==0) {
				int mtOff2 = offsetMap16Left(mtOff);
				WORD orig2 = getOriginalMap16Tile(mtOff2);
				if(orig2>=0x0151 && orig2<0x0161) {
					mtOff2 = offsetMap16Up(mtOff);
					orig2 = getOriginalMap16Tile(mtOff2);
					if(orig2>=0x0151 && orig2<0x0161) tile = 0x00D5;
					else if(orig2==0x00C2 || orig2==0x77E6 || orig2==0x77E7) tile = 0x00C4;
					else tile = 0x00C5;
				} else {
					mtOff2 = offsetMap16Up(mtOff);
					orig2 = getOriginalMap16Tile(mtOff2);
					if(orig2==0x00C5) tile = 0x00C7;
				}
			}
			if(j==0) {
				int mtOff2 = offsetMap16Up(mtOff);
				WORD orig2 = getOriginalMap16Tile(mtOff2);
				if(orig2>=0x0151 && orig2<0x0161) {
					if(shadowFlag) tile = 0x00C6;
					else if(tile!=0x00D5) {
						mtOff2 = offsetMap16Left(mtOff);
						orig2 = getOriginalMap16Tile(mtOff2);
						if(orig2==0x00C6) tile = 0x00C6;
						else tile = 0x00C3;
					}
					shadowFlag = true;
				} else if(shadowFlag) {
					tile = 0x00C7;
					shadowFlag = false;
				}
			}
			addObjectTile(o,tile,mtOff);
			mtOff = offsetMap16Down(mtOff);
		}
		mtOff = preserve = offsetMap16Right(preserve);
	}
}
//Diagonal castle wall, right
void drawObject_45(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	mtOff = offsetMap16Up(mtOff);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+2;
	for(int i=0; i<width; i++) {
		for(int j=0; j<height; j++) {
			WORD orig = getOriginalMap16Tile(mtOff);
			bool overSide = false;
			if(orig==0x00D6 || orig==0x00D7 || orig==0x77D8 || orig==0x77D9) overSide = true;
			WORD tile = 0x00C2;
			if(j==0) {
				tile = 0x00C1;
				int mtOff2 = offsetMap16Up(mtOff);
				WORD orig2 = getOriginalMap16Tile(mtOff2);
				if(orig2==0x0151 || orig2==0x0152 ||
				orig2==0x015A || orig2==0x015B) tile = 0x77E1;
				else {
					mtOff2 = offsetMap16Left(mtOff);
					orig2 = getOriginalMap16Tile(mtOff2);
					if(orig2==0x0151 || orig2==0x0152 ||
					orig2==0x015A || orig2==0x015B) tile = 0x77E6;
				}
			} else if(j==1) {
				tile = 0x00C0;
				int mtOff2 = offsetMap16Left(mtOff);
				WORD orig2 = getOriginalMap16Tile(mtOff2);
				if(orig2==0x0151 || orig2==0x0152 ||
				orig2==0x015A || orig2==0x015B) tile = 0x77E8;
			} else if(i==0) {
				int mtOff2 = offsetMap16Left(mtOff);
				WORD orig2 = getOriginalMap16Tile(mtOff2);
				if(orig2>=0x0151 && orig2<0x0161) {
					mtOff2 = offsetMap16Up(mtOff);
					orig2 = getOriginalMap16Tile(mtOff2);
					if(orig2>=0x0151 && orig2<0x0161) tile = 0x00D5;
					else if(orig2==0x00C2 || orig2==0x77E6 || orig2==0x77E7) tile = 0x00C4;
					else tile = 0x00C5;
				} else {
					mtOff2 = offsetMap16Up(mtOff);
					orig2 = getOriginalMap16Tile(mtOff2);
					if(orig2==0x00C5) tile = 0x00C7;
				}
			}
			if(j || !overSide) addObjectTile(o,tile,mtOff);
			mtOff = offsetMap16Down(mtOff);
		}
		mtOff = preserve = offsetMap16Right(preserve);
		mtOff = preserve = offsetMap16Down(preserve);
		height--;
	}
}
//Diagonal castle wall, left
void drawObject_46(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	mtOff = offsetMap16Up(mtOff);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+2;
	for(int i=0; i<width; i++) {
		for(int j=0; j<height; j++) {
			WORD orig = getOriginalMap16Tile(mtOff);
			bool overSide = false;
			if(orig==0x00D6 || orig==0x00D7 || orig==0x77D8 || orig==0x77D9) overSide = true;
			WORD tile = 0x00C2;
			if(j==0) {
				tile = 0x00BE;
				int mtOff2 = offsetMap16Up(mtOff);
				WORD orig2 = getOriginalMap16Tile(mtOff2);
				if(orig2==0x0151 || orig2==0x0152 ||
				orig2==0x015A || orig2==0x015B) tile = 0x77DE;
				else {
					mtOff2 = offsetMap16Left(mtOff);
					orig2 = getOriginalMap16Tile(mtOff2);
					if(orig2==0x0151 || orig2==0x0152 ||
					orig2==0x015A || orig2==0x015B) tile = 0x77E7;
				}
			} else if(j==1) {
				tile = 0x00BF;
				int mtOff2 = offsetMap16Left(mtOff);
				WORD orig2 = getOriginalMap16Tile(mtOff2);
				if(orig2==0x0151 || orig2==0x0152 ||
				orig2==0x015A || orig2==0x015B) tile = 0x77E9;
			} else if(i==0) {
				int mtOff2 = offsetMap16Left(mtOff);
				WORD orig2 = getOriginalMap16Tile(mtOff2);
				if(orig2>=0x0151 && orig2<0x0161) {
					mtOff2 = offsetMap16Up(mtOff);
					orig2 = getOriginalMap16Tile(mtOff2);
					if(orig2>=0x0151 && orig2<0x0161) tile = 0x00D5;
					else if(orig2==0x00C2 || orig2==0x77E6 || orig2==0x77E7) tile = 0x00C4;
					else tile = 0x00C5;
				} else {
					mtOff2 = offsetMap16Up(mtOff);
					orig2 = getOriginalMap16Tile(mtOff2);
					if(orig2==0x00C5) tile = 0x00C7;
				}
			}
			if(j || !overSide) addObjectTile(o,tile,mtOff);
			mtOff = offsetMap16Down(mtOff);
		}
		mtOff = preserve = offsetMap16Right(preserve);
		mtOff = preserve = offsetMap16Up(preserve);
		height++;
	}
}
//Lava in castle
void drawObject_47(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	for(int j=0; j<height; j++) {
		for(int i=0; i<width; i++) {
			if(j==0) {
				int offset = noiseTilemap[mtOff]&0xE;
				WORD tile = romBuf[0x09A638+offset]|(romBuf[0x09A639+offset]<<8);
				tile += 5;
				int mtOff2 = offsetMap16Up(mtOff);
				WORD orig2 = getOriginalMap16Tile(mtOff2);
				if(orig2==0x00C2) {
					addObjectTile(o,0x002E,mtOff2);
					tile -= 5;
				} else if(orig2==0x00C5) {
					addObjectTile(o,0x002F,mtOff2);
					tile = 0x0031;
				} else if(orig2==0x00C4) {
					addObjectTile(o,0x0030,mtOff2);
					tile = 0x0031;
				}
				addObjectTile(o,tile,mtOff);
			} else addObjectTile(o,0x7E00+(i&1),mtOff);
			if(i==0) {
				int mtOff2 = offsetMap16Left(mtOff);
				WORD orig2 = getOriginalMap16Tile(mtOff2);
				if(orig2==0x015B) addObjectTile(o,0x01A1,mtOff2);
				else if(orig2==0x0151 || orig2==0x152 || orig2==0x015A) addObjectTile(o,0x01A3,mtOff2);
			} else if((i+1)==width) {
				int mtOff2 = offsetMap16Right(mtOff);
				WORD orig2 = getOriginalMap16Tile(mtOff2);
				if(orig2==0x015A) addObjectTile(o,0x01A2,mtOff2);
				else if(orig2==0x0151 || orig2==0x152 || orig2==0x015B) addObjectTile(o,0x01A4,mtOff2);
			}
			if((j+1)==height) {
				int mtOff2 = offsetMap16Down(mtOff);
				WORD orig2 = getOriginalMap16Tile(mtOff2);
				if(orig2==0x0151 || orig2==0x015A) addObjectTile(o,0x01A5,mtOff2);
				else if(orig2==0x0152 || orig2==0x015B) addObjectTile(o,0x01A6,mtOff2);
			}
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Brick
void drawObject_48(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	for(int j=0; j<height; j++) {
		for(int i=0; i<width; i++) {
			bool rhs = (mtOff&1)^((mtOff>>8)&1);
			WORD tile = rhs?0x015B:0x015A;
			if(i==0 && rhs) {
				int mtOff2 = offsetMap16Left(mtOff);
				WORD orig2 = getOriginalMap16Tile(mtOff2);
				if(orig2==0x0152) addObjectTile(o,0x015A,mtOff2);
				else tile = 0x0151;
			} else if((i+1)==width && !rhs) {
				WORD orig = getOriginalMap16Tile(mtOff);
				int mtOff2 = offsetMap16Right(mtOff);
				WORD orig2 = getOriginalMap16Tile(mtOff2);
				if(orig2==0x0151) addObjectTile(o,0x015B,mtOff2);
				else if(orig!=0x015A) tile = 0x0152;
			}
			if(i==0) {
				int mtOff2 = offsetMap16Left(mtOff);
				WORD orig2 = getOriginalMap16Tile(mtOff2);
				if((orig2>=0x002E && orig2<0x0033) ||
				(orig2>=0x0084 && orig2<0x008E) ||
				orig2==0x7E00 || orig2==0x7E01) {
					if(tile==0x015A) tile = 0x01A2;
					else tile = 0x01A4;
				}
			} else if((i+1)==width) {
				int mtOff2 = offsetMap16Right(mtOff);
				WORD orig2 = getOriginalMap16Tile(mtOff2);
				if((orig2>=0x002E && orig2<0x0033) ||
				(orig2>=0x0084 && orig2<0x008E) ||
				orig2==0x7E00 || orig2==0x7E01) {
					if(tile==0x015B) tile = 0x01A1;
					else tile = 0x01A3;
				}
			}
			if(j==0) {
				int mtOff2 = offsetMap16Up(mtOff);
				WORD orig2 = getOriginalMap16Tile(mtOff2);
				if(orig2==0x7E00 || orig2==0x7E01) tile += (0x01A5-0x015A);
			}
			addObjectTile(o,tile,mtOff);
			if((j+1)==height) {
				int mtOff2 = offsetMap16Down(mtOff);
				if(i==0) overlapTile_castleWallShadowDL(o,mtOff2);
				else overlapTile_castleWallShadowD(o,mtOff2);
				if((i+1)==width) {
					mtOff2 = offsetMap16Right(mtOff2);
					overlapTile_castleWallShadowDR(o,mtOff2);
				}
			}
			if((i+1)==width) {
				int mtOff2 = offsetMap16Right(mtOff);
				if(j==0) overlapTile_castleWallShadowUR(o,mtOff2);
				else overlapTile_castleWallShadowR(o,mtOff2);
			}
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Castle window
void drawObject_49(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int height = o->data[3]+1;
	for(int j=0; j<height; j++) {
		int mtOff2 = offsetMap16Right(mtOff);
		if(j==0) {
			addObjectTile(o,0x00C8,mtOff);
			addObjectTile(o,0x00CD,mtOff2);
		} else {
			addObjectTile(o,0x00CE,mtOff);
			addObjectTile(o,0x00CF,mtOff2);
		}
		mtOff = offsetMap16Down(mtOff);
	}
}
//Castle pillar 3
void drawObject_4A(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int height = o->data[3]+1;
	for(int j=0; j<height; j++) {
		addObjectTile(o,0x00D3,mtOff);
		int mtOff2 = offsetMap16Right(mtOff);
		addObjectTile(o,0x00D4,mtOff2);
		mtOff = offsetMap16Down(mtOff);
	}
}
//Checker block 4x
void drawObject_4B(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int height = o->data[3]+1;
	for(int j=0; j<height; j++) {
		if(j==0) {
			addObjectTile(o,0x0174,mtOff);
			int mtOff2 = offsetMap16Right(mtOff);
			addObjectTile(o,0x0175,mtOff2);
			mtOff2 = offsetMap16Right(mtOff2);
			addObjectTile(o,0x0175,mtOff2);
			mtOff2 = offsetMap16Right(mtOff2);
			addObjectTile(o,0x0178,mtOff2);
		} else if((j+1)==height) {
			addObjectTile(o,0x017E,mtOff);
			int mtOff2 = offsetMap16Right(mtOff);
			addObjectTile(o,0x017F,mtOff2);
			mtOff2 = offsetMap16Right(mtOff2);
			addObjectTile(o,0x017F,mtOff2);
			mtOff2 = offsetMap16Right(mtOff2);
			addObjectTile(o,0x0182,mtOff2);
		} else {
			addObjectTile(o,0x0179,mtOff);
			int mtOff2 = offsetMap16Right(mtOff);
			addObjectTile(o,0x017A,mtOff2);
			mtOff2 = offsetMap16Right(mtOff2);
			addObjectTile(o,0x017A,mtOff2);
			mtOff2 = offsetMap16Right(mtOff2);
			addObjectTile(o,0x017D,mtOff2);
		}
		mtOff = offsetMap16Down(mtOff);
	}
}
//Checker block 6x
void drawObject_4C(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int height = o->data[3]+1;
	for(int j=0; j<height; j++) {
		if(j==0) {
			addObjectTile(o,0x0174,mtOff);
			int mtOff2 = offsetMap16Right(mtOff);
			addObjectTile(o,0x0175,mtOff2);
			mtOff2 = offsetMap16Right(mtOff2);
			addObjectTile(o,0x0175,mtOff2);
			mtOff2 = offsetMap16Right(mtOff2);
			addObjectTile(o,0x0175,mtOff2);
			mtOff2 = offsetMap16Right(mtOff2);
			addObjectTile(o,0x0176,mtOff2);
			mtOff2 = offsetMap16Right(mtOff2);
			addObjectTile(o,0x0178,mtOff2);
		} else if((j+1)==height) {
			addObjectTile(o,0x017E,mtOff);
			int mtOff2 = offsetMap16Right(mtOff);
			addObjectTile(o,0x017F,mtOff2);
			mtOff2 = offsetMap16Right(mtOff2);
			addObjectTile(o,0x017F,mtOff2);
			mtOff2 = offsetMap16Right(mtOff2);
			addObjectTile(o,0x017F,mtOff2);
			mtOff2 = offsetMap16Right(mtOff2);
			addObjectTile(o,0x0180,mtOff2);
			mtOff2 = offsetMap16Right(mtOff2);
			addObjectTile(o,0x0182,mtOff2);
		} else {
			addObjectTile(o,0x0179,mtOff);
			int mtOff2 = offsetMap16Right(mtOff);
			addObjectTile(o,0x017A,mtOff2);
			mtOff2 = offsetMap16Right(mtOff2);
			addObjectTile(o,0x017A,mtOff2);
			mtOff2 = offsetMap16Right(mtOff2);
			addObjectTile(o,0x017A,mtOff2);
			mtOff2 = offsetMap16Right(mtOff2);
			addObjectTile(o,0x017B,mtOff2);
			mtOff2 = offsetMap16Right(mtOff2);
			addObjectTile(o,0x017D,mtOff2);
		}
		mtOff = offsetMap16Down(mtOff);
	}
}
//Checker block 8x
void drawObject_4D(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int height = o->data[3]+1;
	for(int j=0; j<height; j++) {
		if(j==0) {
			addObjectTile(o,0x0174,mtOff);
			int mtOff2 = offsetMap16Right(mtOff);
			addObjectTile(o,0x0175,mtOff2);
			mtOff2 = offsetMap16Right(mtOff2);
			addObjectTile(o,0x0175,mtOff2);
			mtOff2 = offsetMap16Right(mtOff2);
			addObjectTile(o,0x0175,mtOff2);
			mtOff2 = offsetMap16Right(mtOff2);
			addObjectTile(o,0x0175,mtOff2);
			mtOff2 = offsetMap16Right(mtOff2);
			addObjectTile(o,0x0175,mtOff2);
			mtOff2 = offsetMap16Right(mtOff2);
			addObjectTile(o,0x0177,mtOff2);
			mtOff2 = offsetMap16Right(mtOff2);
			addObjectTile(o,0x0178,mtOff2);
		} else if((j+1)==height) {
			addObjectTile(o,0x017E,mtOff);
			int mtOff2 = offsetMap16Right(mtOff);
			addObjectTile(o,0x017F,mtOff2);
			mtOff2 = offsetMap16Right(mtOff2);
			addObjectTile(o,0x017F,mtOff2);
			mtOff2 = offsetMap16Right(mtOff2);
			addObjectTile(o,0x017F,mtOff2);
			mtOff2 = offsetMap16Right(mtOff2);
			addObjectTile(o,0x017F,mtOff2);
			mtOff2 = offsetMap16Right(mtOff2);
			addObjectTile(o,0x017F,mtOff2);
			mtOff2 = offsetMap16Right(mtOff2);
			addObjectTile(o,0x0181,mtOff2);
			mtOff2 = offsetMap16Right(mtOff2);
			addObjectTile(o,0x0182,mtOff2);
		} else {
			addObjectTile(o,0x0179,mtOff);
			int mtOff2 = offsetMap16Right(mtOff);
			addObjectTile(o,0x017A,mtOff2);
			mtOff2 = offsetMap16Right(mtOff2);
			addObjectTile(o,0x017A,mtOff2);
			mtOff2 = offsetMap16Right(mtOff2);
			addObjectTile(o,0x017A,mtOff2);
			mtOff2 = offsetMap16Right(mtOff2);
			addObjectTile(o,0x017A,mtOff2);
			mtOff2 = offsetMap16Right(mtOff2);
			addObjectTile(o,0x017A,mtOff2);
			mtOff2 = offsetMap16Right(mtOff2);
			addObjectTile(o,0x017C,mtOff2);
			mtOff2 = offsetMap16Right(mtOff2);
			addObjectTile(o,0x017D,mtOff2);
		}
		mtOff = offsetMap16Down(mtOff);
	}
}
//Sand block
void drawObject_4E(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	if(width==1) {
		if(height==1) {
			overlapTile_sandBlock(o,mtOff,0);
		} else {
			for(int j=0; j<height; j++) {
				if(j==0) overlapTile_sandBlock(o,mtOff,1);
				else if((j+1)==height) overlapTile_sandBlock(o,mtOff,3);
				else overlapTile_sandBlock(o,mtOff,2);
				mtOff = offsetMap16Down(mtOff);
			}
		}
	} else {
		if(height==1) {
			for(int i=0; i<width; i++) {
				if(i==0) overlapTile_sandBlock(o,mtOff,4);
				else if((i+1)==width) overlapTile_sandBlock(o,mtOff,6);
				else overlapTile_sandBlock(o,mtOff,5);
				mtOff = offsetMap16Right(mtOff);
			}
		} else {
			for(int j=0; j<height; j++) {
				for(int i=0; i<width; i++) {
					if(j==0) {
						if(i==0) overlapTile_sandBlock(o,mtOff,7);
						else if((i+1)==width) overlapTile_sandBlock(o,mtOff,13);
						else overlapTile_sandBlock(o,mtOff,10);
					} else if((j+1)==height) {
						if(i==0) overlapTile_sandBlock(o,mtOff,9);
						else if((i+1)==width) overlapTile_sandBlock(o,mtOff,15);
						else overlapTile_sandBlock(o,mtOff,12);
					} else {
						if(i==0) overlapTile_sandBlock(o,mtOff,8);
						else if((i+1)==width) overlapTile_sandBlock(o,mtOff,14);
						else overlapTile_sandBlock(o,mtOff,11);
					}
					mtOff = offsetMap16Right(mtOff);
				}
				mtOff = preserve = offsetMap16Down(preserve);
			}
		}
	}
}
//Sand block remover
void drawObject_4F(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	for(int j=0; j<height; j++) {
		for(int i=0; i<width; i++) {
			WORD orig = getOriginalMap16Tile(mtOff);
			int tileRef = (orig&0xFF)<<1;
			tileRef = romBuf[0x09B7A8+tileRef]|(romBuf[0x09B7A9+tileRef]<<8);
			if(tileRef) addObjectTile(o,tilesetBuffer[tileRef>>1],mtOff);
			else addObjectTile(o,0,mtOff);
			int mtOff2 = offsetMap16Up(mtOff);
			WORD orig2 = getOriginalMap16Tile(mtOff2);
			if((orig2&0xFF00)==tilesetBuffer[0x1A62>>1]) {
				tileRef = (orig2&0xFF)<<1;
				tileRef = romBuf[0x09B1B0+tileRef]|(romBuf[0x09B1B1+tileRef]<<8);
				addObjectTile(o,tilesetBuffer[tileRef>>1],mtOff2);
			}
			mtOff2 = offsetMap16Down(mtOff);
			orig2 = getOriginalMap16Tile(mtOff2);
			if((orig2&0xFF00)==tilesetBuffer[0x1A62>>1]) {
				tileRef = (orig2&0xFF)<<1;
				tileRef = romBuf[0x09B32E +tileRef]|(romBuf[0x09B32F+tileRef]<<8);
				addObjectTile(o,tilesetBuffer[tileRef>>1],mtOff2);
			}
			mtOff2 = offsetMap16Right(mtOff);
			orig2 = getOriginalMap16Tile(mtOff2);
			if((orig2&0xFF00)==tilesetBuffer[0x1A62>>1]) {
				tileRef = (orig2&0xFF)<<1;
				tileRef = romBuf[0x09B4AC+tileRef]|(romBuf[0x09B4AD+tileRef]<<8);
				addObjectTile(o,tilesetBuffer[tileRef>>1],mtOff2);
			}
			mtOff2 = offsetMap16Left(mtOff);
			orig2 = getOriginalMap16Tile(mtOff2);
			if((orig2&0xFF00)==tilesetBuffer[0x1A62>>1]) {
				tileRef = (orig2&0xFF)<<1;
				tileRef = romBuf[0x09B62A+tileRef]|(romBuf[0x09B62B+tileRef]<<8);
				addObjectTile(o,tilesetBuffer[tileRef>>1],mtOff2);
			}
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Vertical train rail
void drawObject_50(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int height = o->data[3]+1;
	for(int j=0; j<height; j++) {
		WORD orig = getOriginalMap16Tile(mtOff);
		if(orig==tilesetBuffer[0x1C5C>>1] || orig==tilesetBuffer[0x1C5E>>1] ||
		orig==tilesetBuffer[0x1D94>>1] || orig==tilesetBuffer[0x1D96>>1]) addObjectTile(o,tilesetBuffer[0x1C48>>1],mtOff);
		else addObjectTile(o,tilesetBuffer[0x1C46>>1],mtOff);
		mtOff = offsetMap16Down(mtOff);
	}
}
//Horizontal train rail
void drawObject_51(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int width = o->data[3]+1;
	for(int i=0; i<width; i++) {
		WORD orig = getOriginalMap16Tile(mtOff);
		if(orig==tilesetBuffer[0x1C5C>>1] || orig==tilesetBuffer[0x1C5E>>1] ||
		orig==tilesetBuffer[0x1D94>>1] || orig==tilesetBuffer[0x1D96>>1]) addObjectTile(o,tilesetBuffer[0x1C48>>1],mtOff);
		else addObjectTile(o,tilesetBuffer[0x1C52>>1],mtOff);
		mtOff = offsetMap16Right(mtOff);
	}
}
//Diagonal train rail
void drawObject_52(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3];
	int height = o->data[4]+1;
	if(width&0x80) {
		width = 0x101-width;
		for(int i=0; i<width; i++) {
			for(int j=0; j<height; j++) {
				if(j==0) addObjectTile(o,tilesetBuffer[0x1C4A>>1],mtOff);
				else if(j==1) addObjectTile(o,tilesetBuffer[0x1C4C>>1],mtOff);
				mtOff = offsetMap16Down(mtOff);
			}
			mtOff = preserve = offsetMap16Left(preserve);
			mtOff = preserve = offsetMap16Down(preserve);
			height--;
		}
	} else {
		width++;
		for(int i=0; i<width; i++) {
			for(int j=0; j<height; j++) {
				if(j==0) addObjectTile(o,tilesetBuffer[0x1C50>>1],mtOff);
				else if(j==1) addObjectTile(o,tilesetBuffer[0x1C4E>>1],mtOff);
				mtOff = offsetMap16Down(mtOff);
			}
			mtOff = preserve = offsetMap16Right(preserve);
			mtOff = preserve = offsetMap16Down(preserve);
			height--;
		}
	}
}
//Castle wall platform, horizontal
void drawObject_53(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int width = o->data[3]+1;
	for(int i=0; i<width; i++) {
		WORD orig = getOriginalMap16Tile(mtOff);
		if(orig>=0x00C2 && orig<0x00C8) {
			WORD tile = (i&1)?0x150D:0x150E;
			if(i==0) {
				if(orig<tilesetBuffer[0x19DA>>1] || orig>=tilesetBuffer[0x1A5E>>1]) {
					if(orig==0x00D1 || orig==0x00D2) tile = 0x150D;
					else tile = 0x00D1;
				}
			} else if((i+1)==width) {
				if(orig<tilesetBuffer[0x19DA>>1] || orig>=tilesetBuffer[0x1A5E>>1]) {
					if(orig==0x00D1 || orig==0x00D2) tile = 0x150D;
					else tile = 0x00D2;
					if(orig>=0x00C4) {
						int tileRef = (orig-0x00C4)<<1;
						tile = romBuf[0x09BA18+tileRef]|(romBuf[0x09BA19+tileRef]<<8);
					}
				}
			} else {
				if(orig>=0x00C4) {
					int tileRef = (orig-0x00C4)<<1;
					tile = romBuf[0x09BA18+tileRef]|(romBuf[0x09BA19+tileRef]<<8);
				}
			}
			addObjectTile(o,tile,mtOff);
		}
		mtOff = offsetMap16Right(mtOff);
	}
}
//Castle wall platform, gentle slope
void drawObject_54(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3];
	int height = o->data[4]+1;
	if(width&0x80) {
		width = 0x101-width;
		for(int i=0; i<width; i++) {
			for(int j=0; j<height; j++) {
				WORD orig = getOriginalMap16Tile(mtOff);
				if(i==0) {
					if(j==0 && orig!=0x150D && orig!=0x150E &&
					(orig<tilesetBuffer[0x19DA>>1] || orig>=tilesetBuffer[0x1A5E>>1])) {
						if(orig==0x00D1) addObjectTile(o,0x150D,mtOff);
						else if(orig==0x00D2) addObjectTile(o,0x150E,mtOff);
						else if(orig==0x00C5) addObjectTile(o,0x151B,mtOff);
						else addObjectTile(o,0x00D2,mtOff);
					}
				} else if((i+1)==width) {
					if(j==0 && orig!=0x150D && orig!=0x150E &&
					(orig<tilesetBuffer[0x19DA>>1] || orig>=tilesetBuffer[0x1A5E>>1])) {
						if(orig==0x00D1) addObjectTile(o,0x150E,mtOff);
						else if(orig==0x00D2) addObjectTile(o,0x150D,mtOff);
						else if(orig==0x00C5) addObjectTile(o,0x151B,mtOff);
						else addObjectTile(o,0x00D1,mtOff);
					}
				} else {
					if(j==0) addObjectTile(o,(i&1)?tilesetBuffer[0x1A34>>1]:tilesetBuffer[0x1A2A>>1],mtOff);
					else if(j==1) addObjectTile(o,(i&1)?tilesetBuffer[0x1A5E>>1]:tilesetBuffer[0x1A40>>1],mtOff);
				}
				mtOff = offsetMap16Down(mtOff);
			}
			mtOff = preserve = offsetMap16Left(preserve);
			if(i && (i&1)==0) {
				mtOff = preserve = offsetMap16Down(preserve);
				height--;
			}
		}
	} else {
		width++;
		for(int i=0; i<width; i++) {
			for(int j=0; j<height; j++) {
				WORD orig = getOriginalMap16Tile(mtOff);
				if(i==0) {
					if(j==0 && orig!=0x150D && orig!=0x150E &&
					(orig<tilesetBuffer[0x19DA>>1] || orig>=tilesetBuffer[0x1A5E>>1])) {
						if(orig==0x00D1) addObjectTile(o,0x150D,mtOff);
						else if(orig==0x00D2) addObjectTile(o,0x150E,mtOff);
						else if(orig==0x00C5) addObjectTile(o,0x151B,mtOff);
						else addObjectTile(o,0x00D1,mtOff);
					}
				} else if((i+1)==width) {
					if(j==0 && orig!=0x150D && orig!=0x150E &&
					(orig<tilesetBuffer[0x19DA>>1] || orig>=tilesetBuffer[0x1A5E>>1])) {
						if(orig==0x00D1) addObjectTile(o,0x150E,mtOff);
						else if(orig==0x00D2) addObjectTile(o,0x150D,mtOff);
						else if(orig==0x00C5) addObjectTile(o,0x151B,mtOff);
						else addObjectTile(o,0x00D2,mtOff);
					}
				} else {
					if(j==0) addObjectTile(o,(i&1)?tilesetBuffer[0x1A42>>1]:tilesetBuffer[0x1A50>>1],mtOff);
					else if(j==1) addObjectTile(o,(i&1)?tilesetBuffer[0x1A60>>1]:tilesetBuffer[0x1A5C>>1],mtOff);
				}
				mtOff = offsetMap16Down(mtOff);
			}
			mtOff = preserve = offsetMap16Right(preserve);
			if(i && (i&1)==0) {
				mtOff = preserve = offsetMap16Down(preserve);
				height--;
			}
		}
	}
}
//Castle wall platform, steep slope
void drawObject_55(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3];
	int height = o->data[4]+1;
	if(width&0x80) {
		width = 0x101-width;
		for(int i=0; i<width; i++) {
			for(int j=0; j<height; j++) {
				WORD orig = getOriginalMap16Tile(mtOff);
				if(i==0) {
					if(j==0 && orig!=0x150D && orig!=0x150E &&
					(orig<tilesetBuffer[0x19DA>>1] || orig>=tilesetBuffer[0x1A5E>>1])) {
						if(orig==0x00D1) addObjectTile(o,0x150D,mtOff);
						else if(orig==0x00D2) addObjectTile(o,0x150E,mtOff);
						else if(orig==0x00C5) addObjectTile(o,0x151B,mtOff);
						else addObjectTile(o,0x00D2,mtOff);
					}
				} else if((i+1)==width) {
					if(j==0 && orig!=0x150D && orig!=0x150E &&
					(orig<tilesetBuffer[0x19DA>>1] || orig>=tilesetBuffer[0x1A5E>>1])) {
						if(orig==0x00D1) addObjectTile(o,0x150E,mtOff);
						else if(orig==0x00D2) addObjectTile(o,0x150D,mtOff);
						else if(orig==0x00C5) addObjectTile(o,0x151B,mtOff);
						else addObjectTile(o,0x00D1,mtOff);
					}
				} else {
					if(j==0) addObjectTile(o,tilesetBuffer[0x1A02>>1],mtOff);
					else if(j==1) addObjectTile(o,tilesetBuffer[0x1A14>>1],mtOff);
				}
				mtOff = offsetMap16Down(mtOff);
			}
			mtOff = preserve = offsetMap16Left(preserve);
			if(i) {
				mtOff = preserve = offsetMap16Down(preserve);
				height--;
			}
		}
	} else {
		width++;
		for(int i=0; i<width; i++) {
			for(int j=0; j<height; j++) {
				WORD orig = getOriginalMap16Tile(mtOff);
				if(i==0) {
					if(j==0 && orig!=0x150D && orig!=0x150E &&
					(orig<tilesetBuffer[0x19DA>>1] || orig>=tilesetBuffer[0x1A5E>>1])) {
						if(orig==0x00D1) addObjectTile(o,0x150D,mtOff);
						else if(orig==0x00D2) addObjectTile(o,0x150E,mtOff);
						else if(orig==0x00C5) addObjectTile(o,0x151B,mtOff);
						else addObjectTile(o,0x00D1,mtOff);
					}
				} else if((i+1)==width) {
					if(j==0 && orig!=0x150D && orig!=0x150E &&
					(orig<tilesetBuffer[0x19DA>>1] || orig>=tilesetBuffer[0x1A5E>>1])) {
						if(orig==0x00D1) addObjectTile(o,0x150E,mtOff);
						else if(orig==0x00D2) addObjectTile(o,0x150D,mtOff);
						else if(orig==0x00C5) addObjectTile(o,0x151B,mtOff);
						else addObjectTile(o,0x00D2,mtOff);
					}
				} else {
					if(j==0) addObjectTile(o,tilesetBuffer[0x1A16>>1],mtOff);
					else if(j==1) addObjectTile(o,tilesetBuffer[0x1A28>>1],mtOff);
				}
				mtOff = offsetMap16Down(mtOff);
			}
			mtOff = preserve = offsetMap16Right(preserve);
			if(i) {
				mtOff = preserve = offsetMap16Down(preserve);
				height--;
			}
		}
	}
}
//Castle wall platform, very steep slope
void drawObject_56(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3];
	int height = o->data[4]+1;
	if(width&0x80) {
		width = 0x101-width;
		for(int i=0; i<width; i++) {
			for(int j=0; j<height; j++) {
				WORD orig = getOriginalMap16Tile(mtOff);
				if(i==0) {
					if(j==0 && orig!=0x150D && orig!=0x150E &&
					(orig<tilesetBuffer[0x19DA>>1] || orig>=tilesetBuffer[0x1A5E>>1])) {
						if(orig==0x00D1) addObjectTile(o,0x150D,mtOff);
						else if(orig==0x00D2) addObjectTile(o,0x150E,mtOff);
						else if(orig==0x00C5) addObjectTile(o,0x151B,mtOff);
						else addObjectTile(o,0x00D2,mtOff);
					}
				} else if((i+1)==width) {
					if(j==0 && orig!=0x150D && orig!=0x150E &&
					(orig<tilesetBuffer[0x19DA>>1] || orig>=tilesetBuffer[0x1A5E>>1])) {
						if(orig==0x00D1) addObjectTile(o,0x150E,mtOff);
						else if(orig==0x00D2) addObjectTile(o,0x150D,mtOff);
						else if(orig==0x00C5) addObjectTile(o,0x151B,mtOff);
						else addObjectTile(o,0x00D1,mtOff);
					}
				} else {
					if(j==0) addObjectTile(o,tilesetBuffer[0x19DA>>1],mtOff);
					else if(j==1) addObjectTile(o,tilesetBuffer[0x19E2>>1],mtOff);
					else if(j==2) addObjectTile(o,tilesetBuffer[0x19EC>>1],mtOff);
				}
				mtOff = offsetMap16Down(mtOff);
			}
			mtOff = preserve = offsetMap16Left(preserve);
			if(i) {
				mtOff = preserve = offsetMap16Down(preserve);
				mtOff = preserve = offsetMap16Down(preserve);
				height -= 2;
			}
		}
	} else {
		width++;
		for(int i=0; i<width; i++) {
			for(int j=0; j<height; j++) {
				WORD orig = getOriginalMap16Tile(mtOff);
				if(i==0) {
					if(j==0 && orig!=0x150D && orig!=0x150E &&
					(orig<tilesetBuffer[0x19DA>>1] || orig>=tilesetBuffer[0x1A5E>>1])) {
						if(orig==0x00D1) addObjectTile(o,0x150D,mtOff);
						else if(orig==0x00D2) addObjectTile(o,0x150E,mtOff);
						else if(orig==0x00C5) addObjectTile(o,0x151B,mtOff);
						else addObjectTile(o,0x00D1,mtOff);
					}
				} else if((i+1)==width) {
					if(j==0 && orig!=0x150D && orig!=0x150E &&
					(orig<tilesetBuffer[0x19DA>>1] || orig>=tilesetBuffer[0x1A5E>>1])) {
						if(orig==0x00D1) addObjectTile(o,0x150E,mtOff);
						else if(orig==0x00D2) addObjectTile(o,0x150D,mtOff);
						else if(orig==0x00C5) addObjectTile(o,0x151B,mtOff);
						else addObjectTile(o,0x00D2,mtOff);
					}
				} else {
					if(j==0) addObjectTile(o,tilesetBuffer[0x19EE>>1],mtOff);
					else if(j==1) addObjectTile(o,tilesetBuffer[0x19F6>>1],mtOff);
					else if(j==2) addObjectTile(o,tilesetBuffer[0x1A00>>1],mtOff);
				}
				mtOff = offsetMap16Down(mtOff);
			}
			mtOff = preserve = offsetMap16Right(preserve);
			if(i) {
				mtOff = preserve = offsetMap16Down(preserve);
				mtOff = preserve = offsetMap16Down(preserve);
				height -= 2;
			}
		}
	}
}
//Cross section platform
void drawObject_57(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int width = o->data[3]+1;
	for(int i=0; i<width; i++) {
		if(i==0) {
			WORD orig = getOriginalMap16Tile(mtOff);
			if(orig==tilesetBuffer[0x1BF8>>1]) addObjectTile(o,tilesetBuffer[0x1D36>>1],mtOff);
			else addObjectTile(o,tilesetBuffer[0x1D30>>1],mtOff);
		} else if((i+1)==width) {
			WORD orig = getOriginalMap16Tile(mtOff);
			if(orig==tilesetBuffer[0x1BFA>>1]) addObjectTile(o,tilesetBuffer[0x1D38>>1],mtOff);
			else addObjectTile(o,tilesetBuffer[0x1D34>>1],mtOff);
		} else addObjectTile(o,tilesetBuffer[0x1D32>>1],mtOff);
		mtOff = offsetMap16Right(mtOff);
	}
}
//Ceiling
void drawObject_58(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	for(int i=0; i<width; i++) {
		if(i==0) {
			WORD orig = getOriginalMap16Tile(mtOff);
			int mtOff2 = offsetMap16Down(mtOff);
			WORD orig2 = getOriginalMap16Tile(mtOff2);
			if(orig2==tilesetBuffer[0x1CE8>>1] || orig2==tilesetBuffer[0x1CEA>>1] ||
			(orig2>=tilesetBuffer[0x1CAE>>1] && orig2<tilesetBuffer[0x1CCA>>1])) {
				addObjectTile(o,tilesetBuffer[0x1CF6>>1],mtOff);
				mtOff = offsetMap16Down(mtOff);
				for(int j=1; j<height; j++) {
					int offset = noiseTilemap[mtOff]&0x06;
					int tileRef = romBuf[0x0982B0+offset]|(romBuf[0x0982B1+offset]<<8);
					WORD tile = tilesetBuffer[tileRef>>1];
					orig2 = getOriginalMap16Tile(mtOff);
					if(orig2==tilesetBuffer[0x1C5C>>1] || orig2==tilesetBuffer[0x1C5E>>1]) tile = tilesetBuffer[0x1CE6>>1];
					else if(orig2==tilesetBuffer[0x1CAC>>1] || orig2==tilesetBuffer[0x1CB6>>1] ||
					orig2==tilesetBuffer[0x1CB8>>1]) {
						mtOff2 = offsetMap16Up(mtOff);
						orig2 = getOriginalMap16Tile(mtOff2);
						if(orig2!=tilesetBuffer[0x1CFE>>1] && orig2!=tilesetBuffer[0x1D00>>1]) {
							addObjectTile(o,tilesetBuffer[0x1CE6>>1],mtOff2);
						}
						mtOff2 = offsetMap16Left(mtOff);
						tile = tilesetBuffer[0x1CAA>>1];
						orig2 = getOriginalMap16Tile(mtOff2);
						if(orig2!=tilesetBuffer[0x1CAC>>1]) {
							mtOff2 = offsetMap16Down(mtOff);
							addObjectTile(o,tilesetBuffer[0x1CC2>>1],mtOff2);
							tile = tilesetBuffer[0x1CA0>>1];
						}
					}
					mtOff2 = offsetMap16Right(mtOff);
					orig2 = getOriginalMap16Tile(mtOff2);
					if(orig2==0x007D || orig2==0x007E ||
					orig2==tilesetBuffer[0x1C5C>>1] || orig2==tilesetBuffer[0x1C5E>>1]) tile = tilesetBuffer[0x1CE6>>1];
					addObjectTile(o,tile,mtOff);
					mtOff = offsetMap16Down(mtOff);
				}
			} else if(orig==tilesetBuffer[0x1CF0>>1] || orig==tilesetBuffer[0x1CF2>>1] ||
			orig==tilesetBuffer[0x1CF4>>1] || orig==tilesetBuffer[0x1CF6>>1] ||
			orig==tilesetBuffer[0x1CF8>>1] || orig==tilesetBuffer[0x1CFA>>1]) addObjectTile(o,tilesetBuffer[0x1CF2>>1],mtOff);
			else if(orig==tilesetBuffer[0x1BF6>>1]) addObjectTile(o,tilesetBuffer[0x1C28>>1],mtOff);
			else if(orig!=tilesetBuffer[0x1C28>>1]) addObjectTile(o,tilesetBuffer[0x1CF0>>1],mtOff);
		} else if((i+1)==width) {
			WORD orig = getOriginalMap16Tile(mtOff);
			int mtOff2 = offsetMap16Down(mtOff);
			WORD orig2 = getOriginalMap16Tile(mtOff2);
			if(orig2==tilesetBuffer[0x1CE8>>1] || orig2==tilesetBuffer[0x1CEA>>1] ||
			(orig2>=tilesetBuffer[0x1CAE>>1] && orig2<tilesetBuffer[0x1CCA>>1])) {
				addObjectTile(o,tilesetBuffer[0x1CF4>>1],mtOff);
				mtOff = offsetMap16Down(mtOff);
				for(int j=1; j<height; j++) {
					int offset = noiseTilemap[mtOff]&0x06;
					int tileRef = romBuf[0x0982A8+offset]|(romBuf[0x0982A9+offset]<<8);
					WORD tile = tilesetBuffer[tileRef>>1];
					orig2 = getOriginalMap16Tile(mtOff);
					if(orig2==tilesetBuffer[0x1C5C>>1] || orig2==tilesetBuffer[0x1C5E>>1]) tile = tilesetBuffer[0x1CE4>>1];
					else if(orig2==tilesetBuffer[0x1CAC>>1] || orig2==tilesetBuffer[0x1CB6>>1] ||
					orig2==tilesetBuffer[0x1CB8>>1]) {
						mtOff2 = offsetMap16Up(mtOff);
						orig2 = getOriginalMap16Tile(mtOff2);
						if(orig2!=tilesetBuffer[0x1CFE>>1] && orig2!=tilesetBuffer[0x1D00>>1]) {
							addObjectTile(o,tilesetBuffer[0x1CE4>>1],mtOff2);
						}
						mtOff2 = offsetMap16Right(mtOff);
						tile = tilesetBuffer[0x1CA8>>1];
						orig2 = getOriginalMap16Tile(mtOff2);
						if(orig2!=tilesetBuffer[0x1CAC>>1]) {
							mtOff2 = offsetMap16Down(mtOff);
							addObjectTile(o,tilesetBuffer[0x1CC4>>1],mtOff2);
							tile = tilesetBuffer[0x1CA2>>1];
						}
					}
					mtOff2 = offsetMap16Left(mtOff);
					orig2 = getOriginalMap16Tile(mtOff2);
					if(orig2==0x007D || orig2==0x007F ||
					orig2==tilesetBuffer[0x1C5C>>1] || orig2==tilesetBuffer[0x1C5E>>1]) tile = tilesetBuffer[0x1CE4>>1];
					addObjectTile(o,tile,mtOff);
					mtOff = offsetMap16Down(mtOff);
				}
			} else if(orig==tilesetBuffer[0x1CF0>>1] || orig==tilesetBuffer[0x1CF2>>1] ||
			orig==tilesetBuffer[0x1CF4>>1] || orig==tilesetBuffer[0x1CF6>>1] ||
			orig==tilesetBuffer[0x1CF8>>1] || orig==tilesetBuffer[0x1CFA>>1]) addObjectTile(o,tilesetBuffer[0x1CF2>>1],mtOff);
			else if(orig==tilesetBuffer[0x1BF6>>1]) addObjectTile(o,tilesetBuffer[0x1C28>>1],mtOff);
			else if(orig!=tilesetBuffer[0x1C28>>1]) addObjectTile(o,tilesetBuffer[0x1CFA>>1],mtOff);
		} else {
			WORD orig = getOriginalMap16Tile(mtOff);
			if(orig!=tilesetBuffer[0x1C28>>1]) {
				WORD tile = tilesetBuffer[0x1CF2>>1];
				for(int n=0; n<0x18; n+=2) {
					int tileRef = romBuf[0x09BCA4+n]|(romBuf[0x09BCA5+n]<<8);
					if(orig==tilesetBuffer[tileRef>>1]) {
						tileRef = romBuf[0x09BCBC+n]|(romBuf[0x09BCBD+n]<<8);
						tile = tilesetBuffer[tileRef>>1];
						break;
					}
				}
				addObjectTile(o,tile,mtOff);
			}
		}
		mtOff = preserve = offsetMap16Right(preserve);
	}
}
//Cross section gentle slope right
void drawObject_59(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	mtOff = offsetMap16Up(mtOff);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+2;
	for(int i=0; i<width; i++) {
		for(int j=0; j<height; j++) {
			if(j==0) addObjectTile(o,(i&1)?tilesetBuffer[0x1D44>>1]:tilesetBuffer[0x1D42>>1],mtOff);
			else if(j==1) addObjectTile(o,(i&1)?tilesetBuffer[0x1CB4>>1]:tilesetBuffer[0x1CB2>>1],mtOff);
			else {
				int offset = noiseTilemap[mtOff]&0x0E;
				int tileRef = romBuf[0x098121+offset]|(romBuf[0x098122+offset]<<8);
				addObjectTile(o,tilesetBuffer[tileRef>>1],mtOff);
			}
			if(i==0) overlapTile_slopeXSectEdgeL(o,mtOff);
			if((i+1)==width && j) overlapTile_slopeXSectEdgeR(o,mtOff);
			mtOff = offsetMap16Down(mtOff);
		}
		mtOff = preserve = offsetMap16Right(preserve);
		if(i&1) {
			mtOff = preserve = offsetMap16Down(preserve);
			height--;
		}
	}
}
//Cross section steep slope right
void drawObject_5A(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	mtOff = offsetMap16Up(mtOff);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+2;
	for(int i=0; i<width; i++) {
		for(int j=0; j<height; j++) {
			if(j==0) addObjectTile(o,tilesetBuffer[0x1D5A>>1],mtOff);
			else if(j==1) addObjectTile(o,tilesetBuffer[0x1CB2>>1],mtOff);
			else {
				int offset = noiseTilemap[mtOff]&0x0E;
				int tileRef = romBuf[0x098121+offset]|(romBuf[0x098122+offset]<<8);
				addObjectTile(o,tilesetBuffer[tileRef>>1],mtOff);
			}
			if(i==0) overlapTile_slopeXSectEdgeL(o,mtOff);
			if((i+1)==width && j) overlapTile_slopeXSectEdgeR(o,mtOff);
			mtOff = offsetMap16Down(mtOff);
		}
		mtOff = preserve = offsetMap16Right(preserve);
		mtOff = preserve = offsetMap16Down(preserve);
		height--;
	}
}
//Cross section very steep slope right
void drawObject_5B(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	mtOff = offsetMap16Up(mtOff);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+2;
	for(int i=0; i<width; i++) {
		for(int j=0; j<height; j++) {
			if(j==0) addObjectTile(o,tilesetBuffer[0x1D50>>1],mtOff);
			else if(j==1) addObjectTile(o,tilesetBuffer[0x1D52>>1],mtOff);
			else if(j==2) addObjectTile(o,tilesetBuffer[0x1CB2>>1],mtOff);
			else {
				int offset = noiseTilemap[mtOff]&0x0E;
				int tileRef = romBuf[0x098121+offset]|(romBuf[0x098122+offset]<<8);
				addObjectTile(o,tilesetBuffer[tileRef>>1],mtOff);
			}
			if(i==0) overlapTile_slopeXSectEdgeL(o,mtOff);
			if((i+1)==width && j>1) overlapTile_slopeXSectEdgeR(o,mtOff);
			mtOff = offsetMap16Down(mtOff);
		}
		mtOff = preserve = offsetMap16Right(preserve);
		mtOff = preserve = offsetMap16Down(preserve);
		mtOff = preserve = offsetMap16Down(preserve);
		height -= 2;
	}
}
//Cross section gentle slope left
void drawObject_5C(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	mtOff = offsetMap16Up(mtOff);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+2;
	for(int i=0; i<width; i++) {
		for(int j=0; j<height; j++) {
			if(j==0) addObjectTile(o,(i&1)?tilesetBuffer[0x1D3C>>1]:tilesetBuffer[0x1D3A>>1],mtOff);
			else if(j==1) addObjectTile(o,(i&1)?tilesetBuffer[0x1CB4>>1]:tilesetBuffer[0x1CB2>>1],mtOff);
			else {
				int offset = noiseTilemap[mtOff]&0x0E;
				int tileRef = romBuf[0x098121+offset]|(romBuf[0x098122+offset]<<8);
				addObjectTile(o,tilesetBuffer[tileRef>>1],mtOff);
			}
			if(i==0 && j) overlapTile_slopeXSectEdgeL(o,mtOff);
			if((i+1)==width) overlapTile_slopeXSectEdgeR(o,mtOff);
			mtOff = offsetMap16Down(mtOff);
		}
		mtOff = preserve = offsetMap16Right(preserve);
		if(i&1) {
			mtOff = preserve = offsetMap16Up(preserve);
			height++;
		}
	}
}
//Cross section steep slope left
void drawObject_5D(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	mtOff = offsetMap16Up(mtOff);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+2;
	for(int i=0; i<width; i++) {
		for(int j=0; j<height; j++) {
			if(j==0) addObjectTile(o,tilesetBuffer[0x1D56>>1],mtOff);
			else if(j==1) addObjectTile(o,tilesetBuffer[0x1CB4>>1],mtOff);
			else {
				int offset = noiseTilemap[mtOff]&0x0E;
				int tileRef = romBuf[0x098121+offset]|(romBuf[0x098122+offset]<<8);
				addObjectTile(o,tilesetBuffer[tileRef>>1],mtOff);
			}
			if(i==0 && j) overlapTile_slopeXSectEdgeL(o,mtOff);
			if((i+1)==width) overlapTile_slopeXSectEdgeR(o,mtOff);
			mtOff = offsetMap16Down(mtOff);
		}
		mtOff = preserve = offsetMap16Right(preserve);
		mtOff = preserve = offsetMap16Up(preserve);
		height++;
	}
}
//Cross section very steep slope left
void drawObject_5E(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	mtOff = offsetMap16Up(mtOff);
	mtOff = offsetMap16Up(mtOff);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+3;
	for(int i=0; i<width; i++) {
		for(int j=0; j<height; j++) {
			if(j==0) addObjectTile(o,tilesetBuffer[0x1D4A>>1],mtOff);
			else if(j==1) addObjectTile(o,tilesetBuffer[0x1D4C>>1],mtOff);
			else if(j==2) addObjectTile(o,tilesetBuffer[0x1CB4>>1],mtOff);
			else {
				int offset = noiseTilemap[mtOff]&0x0E;
				int tileRef = romBuf[0x098121+offset]|(romBuf[0x098122+offset]<<8);
				addObjectTile(o,tilesetBuffer[tileRef>>1],mtOff);
			}
			if(i==0 && j>1) overlapTile_slopeXSectEdgeL(o,mtOff);
			if((i+1)==width && j>1) overlapTile_slopeXSectEdgeR(o,mtOff);
			mtOff = offsetMap16Down(mtOff);
		}
		mtOff = preserve = offsetMap16Right(preserve);
		mtOff = preserve = offsetMap16Up(preserve);
		mtOff = preserve = offsetMap16Up(preserve);
		height += 2;
	}
}
//Cross section upside-down gentle slope right
void drawObject_5F(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	if(height<(width>>1)) height = width>>1;
	height++;
	int bg1Ts = ((levelHeader[0]&7)<<1)|(levelHeader[1]>>7);
	for(int i=0; i<width; i++) {
		for(int j=0; j<height; j++) {
			if((height-j)==1) {
				if(bg1Ts==8) {
					if(i&1) addObjectTile(o,0x5903,mtOff);
				} else if(i&1) addObjectTile(o,tilesetBuffer[0x1D72>>1],mtOff);
				else {
					WORD orig = getOriginalMap16Tile(mtOff);
					if(orig==tilesetBuffer[0x1C18>>1] || orig==tilesetBuffer[0x1C04>>1]) addObjectTile(o,tilesetBuffer[0x1C1E>>1],mtOff);
				}
			} else if((height-j)==2 && (i&1)==0) {
				if(bg1Ts==8) addObjectTile(o,0x5703,mtOff);
				else addObjectTile(o,tilesetBuffer[0x1D6C>>1],mtOff);
			} else {
				int offset = noiseTilemap[mtOff]&0x0E;
				int tileRef = romBuf[0x098121+offset]|(romBuf[0x098122+offset]<<8);
				addObjectTile(o,tilesetBuffer[tileRef>>1],mtOff);
			}
			if(i==0 && (height-j)>1) overlapTile_slopeXSectEdgeL(o,mtOff);
			if((i+1)==width) {
				int mtOff2 = offsetMap16Right(mtOff);
				if((height-j)==1) {
					WORD orig = getOriginalMap16Tile(mtOff2);
					if(orig==tilesetBuffer[0x1C04>>1]) addObjectTile(o,tilesetBuffer[0x1C1E>>1],mtOff2);
				} else overlapTile_slopeXSectEdgeR(o,mtOff);
			}
			mtOff = offsetMap16Down(mtOff);
		}
		mtOff = preserve = offsetMap16Right(preserve);
		if((i&1)==0) height--;
	}
}
//Cross section upside-down steep slope right
void drawObject_60(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	if(height<width) height = width;
	height++;
	int bg1Ts = ((levelHeader[0]&7)<<1)|(levelHeader[1]>>7);
	for(int i=0; i<width; i++) {
		for(int j=0; j<height; j++) {
			if((height-j)==1) {
				if(bg1Ts!=8) {
					WORD orig = getOriginalMap16Tile(mtOff);
					if(orig==tilesetBuffer[0x1C18>>1] || orig==tilesetBuffer[0x1C04>>1]) addObjectTile(o,tilesetBuffer[0x1C1E>>1],mtOff);
				}
			} else if((height-j)==2) {
				if(bg1Ts==8) addObjectTile(o,0x5D04,mtOff);
				else addObjectTile(o,tilesetBuffer[0x1D82>>1],mtOff);
			} else {
				int offset = noiseTilemap[mtOff]&0x0E;
				int tileRef = romBuf[0x098121+offset]|(romBuf[0x098122+offset]<<8);
				addObjectTile(o,tilesetBuffer[tileRef>>1],mtOff);
			}
			if(i==0 && (height-j)>1) overlapTile_slopeXSectEdgeL(o,mtOff);
			if((i+1)==width && (height-j)>1) {
				int mtOff2 = offsetMap16Right(mtOff);
				if((height-j)==2) {
					WORD orig = getOriginalMap16Tile(mtOff2);
					if(orig==tilesetBuffer[0x1C04>>1]) addObjectTile(o,tilesetBuffer[0x1C1E>>1],mtOff2);
				} else overlapTile_slopeXSectEdgeR(o,mtOff);
			}
			mtOff = offsetMap16Down(mtOff);
		}
		mtOff = preserve = offsetMap16Right(preserve);
		height--;
	}
}
//Cross section upside-down gentle slope left
void drawObject_61(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	height -= (width-1)>>1;
	if(height<1) height = 1;
	int bg1Ts = ((levelHeader[0]&7)<<1)|(levelHeader[1]>>7);
	for(int i=0; i<width; i++) {
		for(int j=0; j<height; j++) {
			if((height-j)==1) {
				if(bg1Ts==8) {
					if((i&1)==0) addObjectTile(o,0x5303,mtOff);
				} else if(i&1) {
					WORD orig = getOriginalMap16Tile(mtOff);
					if(orig==tilesetBuffer[0x1C04>>1]) addObjectTile(o,tilesetBuffer[0x1C1C>>1],mtOff);
					else if(orig==tilesetBuffer[0x1BFA>>1]) addObjectTile(o,tilesetBuffer[0x1C2E>>1],mtOff);
					else if(orig==tilesetBuffer[0x1C02>>1]) addObjectTile(o,tilesetBuffer[0x1C30>>1],mtOff);
				} else addObjectTile(o,tilesetBuffer[0x1D60>>1],mtOff);
			} else if((height-j)==2 && (i&1)) {
				if(bg1Ts==8) addObjectTile(o,0x5503,mtOff);
				else addObjectTile(o,tilesetBuffer[0x1D66>>1],mtOff);
			} else {
				int offset = noiseTilemap[mtOff]&0x0E;
				int tileRef = romBuf[0x098121+offset]|(romBuf[0x098122+offset]<<8);
				addObjectTile(o,tilesetBuffer[tileRef>>1],mtOff);
			}
			if(i==0 && (height-j)>1) overlapTile_slopeXSectEdgeL(o,mtOff);
			if((i+1)==width) {
				int mtOff2 = offsetMap16Right(mtOff);
				if((height-j)==1) {
					WORD orig = getOriginalMap16Tile(mtOff2);
					if(orig==tilesetBuffer[0x1C04>>1]) addObjectTile(o,tilesetBuffer[0x1C1E>>1],mtOff2);
				} else overlapTile_slopeXSectEdgeR(o,mtOff);
			}
			mtOff = offsetMap16Down(mtOff);
		}
		mtOff = preserve = offsetMap16Right(preserve);
		if((i&1)==0) height++;
	}
}
//Cross section upside-down steep slope left
void drawObject_62(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	height -= (width-1);
	if(height<2) height = 2;
	int bg1Ts = ((levelHeader[0]&7)<<1)|(levelHeader[1]>>7);
	for(int i=0; i<width; i++) {
		for(int j=0; j<height; j++) {
			if((height-j)==1) {
				if(bg1Ts!=8) {
					WORD orig = getOriginalMap16Tile(mtOff);
					if(orig==tilesetBuffer[0x1C04>>1]) addObjectTile(o,tilesetBuffer[0x1C1C>>1],mtOff);
					else if(orig==tilesetBuffer[0x1BFA>>1]) addObjectTile(o,tilesetBuffer[0x1C2E>>1],mtOff);
					else if(orig==tilesetBuffer[0x1C02>>1]) addObjectTile(o,tilesetBuffer[0x1C30>>1],mtOff);
				}
			} else if((height-j)==2) {
				if(bg1Ts==8) addObjectTile(o,0x5B05,mtOff);
				else addObjectTile(o,tilesetBuffer[0x1D78>>1],mtOff);
			} else {
				int offset = noiseTilemap[mtOff]&0x0E;
				int tileRef = romBuf[0x098121+offset]|(romBuf[0x098122+offset]<<8);
				addObjectTile(o,tilesetBuffer[tileRef>>1],mtOff);
			}
			if(i==0 && (height-j)>2) overlapTile_slopeXSectEdgeL(o,mtOff);
			if((i+1)==width) {
				int mtOff2 = offsetMap16Right(mtOff);
				if((height-j)==1) {
					WORD orig = getOriginalMap16Tile(mtOff2);
					if(orig==tilesetBuffer[0x1C04>>1]) addObjectTile(o,tilesetBuffer[0x1C1E>>1],mtOff2);
				} else overlapTile_slopeXSectEdgeR(o,mtOff);
			}
			mtOff = offsetMap16Down(mtOff);
		}
		mtOff = preserve = offsetMap16Right(preserve);
		height++;
	}
}
//Brown platform
void drawObject_63(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int width = o->data[3]+1;
	for(int i=0; i<width; i++) {
		if(i==0) addObjectTile(o,0x151E,mtOff);
		else if((i+1)==width) addObjectTile(o,0x1520,mtOff);
		else addObjectTile(o,0x151F,mtOff);
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
				if(offset<0xB) tile = 0x79BB+offset;
				addObjectTile(o,tile,mtOff);
			} else {
				int offset = noiseTilemap[mtOff]&0x0E;
				int tileRef = romBuf[0x098121+offset]|(romBuf[0x098122+offset]<<8);
				addObjectTile(o,tilesetBuffer[tileRef>>1],mtOff);
				if(i==0) {
					int mtOff2 = offsetMap16Left(mtOff);
					overlapTile_groundXSectEdge(o,mtOff2,0x09C194);
					if(j==0) {
						int mtOff3 = offsetMap16Up(mtOff2);
						overlapTile_groundXSectEdge(o,mtOff3,0x09C311);
					}
					if((j+1)==height) {
						int mtOff3 = offsetMap16Down(mtOff2);
						overlapTile_groundXSectEdge(o,mtOff3,0x09C36D);
					}
				}
				if((i+1)==width) {
					int mtOff2 = offsetMap16Right(mtOff);
					overlapTile_groundXSectEdge(o,mtOff2,0x09C20F);
					if(j==0) {
						int mtOff3 = offsetMap16Up(mtOff2);
						overlapTile_groundXSectEdge(o,mtOff3,0x09C481);
					}
					if((j+1)==height) {
						int mtOff3 = offsetMap16Down(mtOff2);
						overlapTile_groundXSectEdge(o,mtOff3,0x09C4DD);
					}
				}
				if(j==0) {
					int mtOff2 = offsetMap16Up(mtOff);
					overlapTile_groundXSectEdge(o,mtOff2,0x09C3C9);
				}
				if((j+1)==height) {
					int mtOff2 = offsetMap16Down(mtOff);
					overlapTile_groundXSectEdge(o,mtOff2,0x09C425);
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
				if(i==0) addObjectTile(o,0x6100,mtOff);
				else if((i+1)==width) addObjectTile(o,0x6102,mtOff);
				else addObjectTile(o,0x6101,mtOff);
			} else if((j+1)==height) {
				if(i==0) addObjectTile(o,0x6103,mtOff);
				else if((i+1)==width) addObjectTile(o,0x6105,mtOff);
				else addObjectTile(o,0x6104,mtOff);
			} else {
				if(i==0) addObjectTile(o,0x0185,mtOff);
				else if((i+1)==width) addObjectTile(o,0x0187,mtOff);
				else addObjectTile(o,0x0186,mtOff);
			}
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Unknown
void drawObject_6A(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int width = o->data[3]+1;
	for(int i=0; i<width; i++) {
		if(i==0) addObjectTile(o,0x6400,mtOff);
		else if((i+1)==width) addObjectTile(o,0x6402,mtOff);
		else addObjectTile(o,0x6401,mtOff);
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
					int mtOff2 = offsetMap16Left(mtOff);
					WORD orig2 = getOriginalMap16Tile(mtOff2);
					if(orig2==tilesetBuffer[0x1CB6>>1] || orig2==tilesetBuffer[0x1CB8>>1] ||
					orig2==tilesetBuffer[0x1CD2>>1] || orig2==tilesetBuffer[0x1CE6>>1]) addObjectTile(o,tilesetBuffer[0x1CFC>>1],mtOff);
					else addObjectTile(o,0x0188,mtOff);
					mtOff2 = offsetMap16Up(mtOff2);
					orig2 = getOriginalMap16Tile(mtOff2);
					if(orig2==tilesetBuffer[0x1C5C>>1] || orig2==tilesetBuffer[0x1C5E>>1]) {
						mtOff2 = offsetMap16Up(mtOff);
						addObjectTile(o,tilesetBuffer[0x1D14>>1],mtOff2);
					}
				}
				else if((i+1)==width) addObjectTile(o,0x018A,mtOff);
				else addObjectTile(o,0x0189,mtOff);
			} else {
				if(i==0) addObjectTile(o,0x018B,mtOff);
				else if((i+1)==width) addObjectTile(o,0x018D,mtOff);
				else addObjectTile(o,0x018C,mtOff);
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
			addObjectTile(o,0x0184,mtOff);
			if((j+1)==height) {
				int mtOff2 = offsetMap16Down(mtOff);
				if(i==0) overlapTile_castleWallShadowDL(o,mtOff2);
				else overlapTile_castleWallShadowD(o,mtOff2);
				if((i+1)==width) {
					mtOff2 = offsetMap16Right(mtOff2);
					overlapTile_castleWallShadowDR(o,mtOff2);
				}
			}
			if((i+1)==width) {
				int mtOff2 = offsetMap16Right(mtOff);
				if(j==0) overlapTile_castleWallShadowUR(o,mtOff2);
				else overlapTile_castleWallShadowR(o,mtOff2);
			}
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
		if(j==0) addObjectTile(o,tilesetBuffer[0x1DD6>>1],mtOff);
		else if((j+1)==height) addObjectTile(o,tilesetBuffer[0x1DD2>>1],mtOff);
		else addObjectTile(o,tilesetBuffer[0x1DD0>>1],mtOff);
		mtOff = offsetMap16Down(mtOff);
	}
}
//Cement blocks, colored
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
//Twisted tree
void drawObject_6F(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int height = o->data[3]+1;
	for(int j=0; j<height; j++) {
		int tileRef = noiseTilemap[mtOff]&6;
		WORD tile = romBuf[0x09C848+tileRef]|(romBuf[0x09C849+tileRef]<<8);
		WORD orig = getOriginalMap16Tile(mtOff);
		if((j+1)==height && (orig==tilesetBuffer[0x1C5C>>1] || orig==tilesetBuffer[0x1C5E>>1])) tile = 0x3D4B;
		addObjectTile(o,tile,mtOff);
		mtOff = offsetMap16Down(mtOff);
	}
}
//Forest plants
void drawObject_70(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int width = o->data[3]+1;
	if(width&1) width++;
	int objRef = (o->data[0]-0x70)*0x15;
	for(int i=0; i<width; i++) {
		int offset = (i&1)<<1;
		WORD tile = romBuf[0x09C877+objRef+offset]|(romBuf[0x09C878+objRef+offset]<<8);
		addObjectTile(o,tile,mtOff);
		int mtOff2 = offsetMap16Down(mtOff);
		tile = romBuf[0x09C87B+objRef+offset]|(romBuf[0x09C87C+objRef+offset]<<8);
		addObjectTile(o,tile,mtOff2);
		mtOff = offsetMap16Right(mtOff);
	}
}
//Twisted tree leaves 3x2
void drawObject_73(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int height = o->data[3]+1;
	for(int j=0; j<height; j++) {
		addObjectTile(o,(j&1)?0x3D50:0x3D42,mtOff);
		int mtOff2 = offsetMap16Right(mtOff);
		addObjectTile(o,(j&1)?0x3D51:0x3D43,mtOff2);
		mtOff2 = offsetMap16Right(mtOff2);
		addObjectTile(o,(j&1)?0x3D52:0x3D44,mtOff2);
		mtOff = offsetMap16Down(mtOff);
	}
}
//Twisted tree leaves 3x1
void drawObject_74(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int height = o->data[3]+1;
	for(int j=0; j<height; j++) {
		addObjectTile(o,0x3D53,mtOff);
		int mtOff2 = offsetMap16Right(mtOff);
		addObjectTile(o,0x3D54,mtOff2);
		mtOff2 = offsetMap16Right(mtOff2);
		addObjectTile(o,0x3D55,mtOff2);
		mtOff = offsetMap16Down(mtOff);
	}
}
//Twisted tree leaves 2x1, type 1
void drawObject_75(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int height = o->data[3]+1;
	for(int j=0; j<height; j++) {
		addObjectTile(o,0x3D53,mtOff);
		int mtOff2 = offsetMap16Right(mtOff);
		addObjectTile(o,0x3D57,mtOff2);
		mtOff = offsetMap16Down(mtOff);
	}
}
//Twisted tree leaves 2x1, type 1
void drawObject_76(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int height = o->data[3]+1;
	for(int j=0; j<height; j++) {
		addObjectTile(o,0x3D56,mtOff);
		int mtOff2 = offsetMap16Right(mtOff);
		addObjectTile(o,0x3D55,mtOff2);
		mtOff = offsetMap16Down(mtOff);
	}
}
//Twisted tree leaves 1x1
void drawObject_77(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int height = o->data[3]+1;
	for(int j=0; j<height; j++) {
		addObjectTile(o,0x3D58,mtOff);
		mtOff = offsetMap16Down(mtOff);
	}
}
//Diagonal twisted tree
void drawObject_78(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int width = o->data[3];
	if(width&0x80) {
		width = 0x101-width;
		for(int i=0; i<width; i++) {
			if(i==0) addObjectTile(o,0x3D40,mtOff);
			else if((i+1)==width) addObjectTile(o,0x3D3F,mtOff);
			else {
				addObjectTile(o,0x3D3F,mtOff);
				mtOff = offsetMap16Down(mtOff);
				addObjectTile(o,0x3D40,mtOff);
			}
			mtOff = offsetMap16Left(mtOff);
		}
	} else {
		width++;
		for(int i=0; i<width; i++) {
			if(i==0) addObjectTile(o,0x3D3D,mtOff);
			else if((i+1)==width) addObjectTile(o,0x3D3E,mtOff);
			else {
				addObjectTile(o,0x3D3E,mtOff);
				mtOff = offsetMap16Down(mtOff);
				addObjectTile(o,0x3D3D,mtOff);
			}
			mtOff = offsetMap16Right(mtOff);
		}
	}
}
//Red platform stairs
void drawObject_79(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int width = o->data[3];
	if(width&0x80) {
		width = 0x101-width;
		for(int i=0; i<width; i++) {
			if(i==0) addObjectTile(o,0x6600,mtOff);
			else if((i+1)==width) addObjectTile(o,0x3D59,mtOff);
			else {
				addObjectTile(o,0x3D59,mtOff);
				mtOff = offsetMap16Down(mtOff);
				addObjectTile(o,0x6600,mtOff);
			}
			mtOff = offsetMap16Left(mtOff);
		}
	} else {
		width++;
		for(int i=0; i<width; i++) {
			if(i==0) addObjectTile(o,0x6700,mtOff);
			else if((i+1)==width) addObjectTile(o,0x3D5A,mtOff);
			else {
				addObjectTile(o,0x3D5A,mtOff);
				mtOff = offsetMap16Down(mtOff);
				addObjectTile(o,0x6700,mtOff);
			}
			mtOff = offsetMap16Right(mtOff);
		}
	}
}
//Wall
void drawObject_7A(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	for(int i=0; i<width; i++) {
		for(int j=0; j<height; j++) {
			WORD orig = getOriginalMap16Tile(mtOff);
			if(j==0) {
				if(i==0) {
					if(orig==tilesetBuffer[0x1D90>>1] || orig==tilesetBuffer[0x1DAC>>1]) addObjectTile(o,tilesetBuffer[0x1D9E>>1],mtOff);
					else {
						int mtOff2 = offsetMap16Down(mtOff);
						WORD orig2 = getOriginalMap16Tile(mtOff2);
						if(orig2!=tilesetBuffer[0x1D9C>>1]) addObjectTile(o,tilesetBuffer[0x1DAE>>1],mtOff);
					}
				} else if((i+1)==width) {
					if(orig==tilesetBuffer[0x1D8A>>1] || orig==tilesetBuffer[0x1DAA>>1]) addObjectTile(o,tilesetBuffer[0x1D9C>>1],mtOff);
					else {
						int mtOff2 = offsetMap16Down(mtOff);
						WORD orig2 = getOriginalMap16Tile(mtOff2);
						if(orig2!=tilesetBuffer[0x1D9E>>1]) addObjectTile(o,tilesetBuffer[0x1DAE>>1],mtOff);
					}
				} else addObjectTile(o,tilesetBuffer[0x1DAE>>1],mtOff);
			} else if((j+1)==height) {
				if(i==0) {
					if(orig==tilesetBuffer[0x1D94>>1] || orig==tilesetBuffer[0x1D96>>1] ||
					orig==tilesetBuffer[0x1D98>>1]) {
						if(orig) addObjectTile(o,(i&1)?tilesetBuffer[0x1D96>>1]:tilesetBuffer[0x1D94>>1],mtOff);
						else addObjectTile(o,tilesetBuffer[0x1DB0>>1],mtOff);
					} else if(orig==tilesetBuffer[0x1D90>>1] || orig==tilesetBuffer[0x1DAC>>1]) addObjectTile(o,tilesetBuffer[0x1DA6>>1],mtOff);
					else if(orig!=tilesetBuffer[0x1DA2>>1]) addObjectTile(o,tilesetBuffer[0x1D92>>1],mtOff);
				} else if((i+1)==width) {
					if(orig==tilesetBuffer[0x1D92>>1] || orig==tilesetBuffer[0x1D94>>1] ||
					orig==tilesetBuffer[0x1D96>>1]) {
						if(orig) addObjectTile(o,(i&1)?tilesetBuffer[0x1D96>>1]:tilesetBuffer[0x1D94>>1],mtOff);
						else addObjectTile(o,tilesetBuffer[0x1DB0>>1],mtOff);
					} else if(orig==tilesetBuffer[0x1D8A>>1] || orig==tilesetBuffer[0x1DAA>>1]) addObjectTile(o,tilesetBuffer[0x1DA4>>1],mtOff);
					else if(orig!=tilesetBuffer[0x1DA8>>1]) addObjectTile(o,tilesetBuffer[0x1D98>>1],mtOff);
				} else {
					if(orig) addObjectTile(o,(i&1)?tilesetBuffer[0x1D96>>1]:tilesetBuffer[0x1D94>>1],mtOff);
					else addObjectTile(o,tilesetBuffer[0x1DB0>>1],mtOff);
				}
			} else {
				if(i==0) {
					if(orig==tilesetBuffer[0x1D8C>>1] || orig==tilesetBuffer[0x1D8E>>1] ||
					orig==tilesetBuffer[0x1D90>>1] || orig==tilesetBuffer[0x1DAC>>1]) addObjectTile(o,tilesetBuffer[0x1D8C>>1],mtOff);
					else if(orig==tilesetBuffer[0x1DAE>>1]) addObjectTile(o,tilesetBuffer[0x1D9C>>1],mtOff);
					else if(orig!=tilesetBuffer[0x1D9C>>1] && orig!=tilesetBuffer[0x1D9E>>1] &&
					orig!=tilesetBuffer[0x1DA4>>1]) addObjectTile(o,tilesetBuffer[0x1D8A>>1],mtOff);
				} else if((i+1)==width) {
					if(orig==tilesetBuffer[0x1D8A>>1] || orig==tilesetBuffer[0x1D8C>>1] ||
					orig==tilesetBuffer[0x1D8E>>1] || orig==tilesetBuffer[0x1DAA>>1]) addObjectTile(o,(i&1)?tilesetBuffer[0x1D8E>>1]:tilesetBuffer[0x1D8C>>1],mtOff);
					else if(orig==tilesetBuffer[0x1DAE>>1]) addObjectTile(o,tilesetBuffer[0x1D9E>>1],mtOff);
					else if(orig!=tilesetBuffer[0x1D9C>>1] && orig!=tilesetBuffer[0x1D9E>>1] &&
					orig!=tilesetBuffer[0x1DA6>>1]) addObjectTile(o,(i&1)?tilesetBuffer[0x1DAC>>1]:tilesetBuffer[0x1D90>>1],mtOff);
				} else addObjectTile(o,(i&1)?tilesetBuffer[0x1D8E>>1]:tilesetBuffer[0x1D8C>>1],mtOff);
			}
			mtOff = offsetMap16Down(mtOff);
		}
		mtOff = preserve = offsetMap16Right(preserve);
	}
}
//Diagonal wall with edge
void drawObject_7B(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3];
	int height = o->data[4]+1;
	if(width&0x80) {
		width = 0x101-width;
		for(int i=0; i<width; i++) {
			for(int j=0; j<height; j++) {
				WORD orig = getOriginalMap16Tile(mtOff);
				if(j==0) {
					if(orig==tilesetBuffer[0x1DAE>>1] || (orig&0xFF00)!=tilesetBuffer[0x1D8A>>1]) addObjectTile(o,tilesetBuffer[0x1D9A>>1],mtOff);
				} else if(j==1) {
					if((i+1)==width) {
						if(orig==tilesetBuffer[0x1DAE>>1]) addObjectTile(o,tilesetBuffer[0x1D9C>>1],mtOff);
						else {
							if(orig==tilesetBuffer[0x1C5C>>1] || orig==tilesetBuffer[0x1C5E>>1] ||
							orig==tilesetBuffer[0x1C92>>1] || orig==tilesetBuffer[0x1C98>>1]) addObjectTile(o,tilesetBuffer[0x1D92>>1],mtOff);
							else if(orig!=tilesetBuffer[0x1D8C>>1] && orig!=tilesetBuffer[0x1D8E>>1] &&
							orig!=tilesetBuffer[0x1D9C>>1] && orig!=tilesetBuffer[0x1D9E>>1]) addObjectTile(o,tilesetBuffer[0x1DAA>>1],mtOff);
						}
					} else {
						if(orig==tilesetBuffer[0x1C5C>>1] || orig==tilesetBuffer[0x1C5E>>1] ||
						orig==tilesetBuffer[0x1C92>>1] || orig==tilesetBuffer[0x1C98>>1]) addObjectTile(o,tilesetBuffer[0x1D92>>1],mtOff);
						else if(orig!=tilesetBuffer[0x1D8C>>1] && orig!=tilesetBuffer[0x1D8E>>1] &&
						orig!=tilesetBuffer[0x1D9C>>1] && orig!=tilesetBuffer[0x1D9E>>1]) addObjectTile(o,tilesetBuffer[0x1D9C>>1],mtOff);
					}
				} else if((j+1)==height) {
					if(orig==tilesetBuffer[0x1C5C>>1] || orig==tilesetBuffer[0x1C5E>>1] ||
					orig==tilesetBuffer[0x1D92>>1] || orig==tilesetBuffer[0x1D94>>1] ||
					orig==tilesetBuffer[0x1D96>>1] || orig==tilesetBuffer[0x1D98>>1]) {
						if((i+1)==width) {
							if(orig==tilesetBuffer[0x1C5C>>1] || orig==tilesetBuffer[0x1C5E>>1]) addObjectTile(o,tilesetBuffer[0x1D92>>1],mtOff);
							else if((orig&0xFF00)==tilesetBuffer[0x1D8A>>1]) addObjectTile(o,(i&1)?tilesetBuffer[0x1D96>>1]:tilesetBuffer[0x1D94>>1],mtOff);
							else addObjectTile(o,tilesetBuffer[0x1DAA>>1],mtOff);
						} else addObjectTile(o,(i&1)?tilesetBuffer[0x1D96>>1]:tilesetBuffer[0x1D94>>1],mtOff);
					} else {
						if((i+1)==width) {
							if(orig==tilesetBuffer[0x1C5C>>1] || orig==tilesetBuffer[0x1C5E>>1]) addObjectTile(o,tilesetBuffer[0x1D92>>1],mtOff);
							else if((orig&0xFF00)==tilesetBuffer[0x1D8A>>1]) addObjectTile(o,(i&1)?tilesetBuffer[0x1D8E>>1]:tilesetBuffer[0x1D8C>>1],mtOff);
							else addObjectTile(o,tilesetBuffer[0x1DAA>>1],mtOff);
						} else addObjectTile(o,(i&1)?tilesetBuffer[0x1D8E>>1]:tilesetBuffer[0x1D8C>>1],mtOff);
					}
				} else {
					if((i+1)==width) {
						if(orig==tilesetBuffer[0x1D8A>>1] || orig==tilesetBuffer[0x1DAA>>1] ||
						orig==tilesetBuffer[0x1D90>>1] || orig==tilesetBuffer[0x1DAC>>1] ||
						orig==tilesetBuffer[0x1D8C>>1] || orig==tilesetBuffer[0x1D8E>>1]) addObjectTile(o,(i&1)?tilesetBuffer[0x1D8E>>1]:tilesetBuffer[0x1D8C>>1],mtOff);
						else if(orig!=tilesetBuffer[0x1D9C>>1] && orig!=tilesetBuffer[0x1D9E>>1]) {
							if(orig==tilesetBuffer[0x1DAE>>1]) addObjectTile(o,tilesetBuffer[0x1D9C>>1],mtOff);
							else addObjectTile(o,(i&1)?tilesetBuffer[0x1DAA>>1]:tilesetBuffer[0x1D8A>>1],mtOff);
						}
					} else addObjectTile(o,(i&1)?tilesetBuffer[0x1D8E>>1]:tilesetBuffer[0x1D8C>>1],mtOff);
				}
				mtOff = offsetMap16Down(mtOff);
			}
			mtOff = preserve = offsetMap16Left(preserve);
			mtOff = preserve = offsetMap16Down(preserve);
			height--;
		}
	} else {
		width++;
		for(int i=0; i<width; i++) {
			for(int j=0; j<height; j++) {
				WORD orig = getOriginalMap16Tile(mtOff);
				if(j==0) {
					if(orig==tilesetBuffer[0x1DAE>>1] || (orig&0xFF00)!=tilesetBuffer[0x1D8A>>1]) addObjectTile(o,tilesetBuffer[0x1DA0>>1],mtOff);
				} else if(j==1) {
					if((i+1)==width) {
						if(orig==tilesetBuffer[0x1DAE>>1]) addObjectTile(o,tilesetBuffer[0x1D9E>>1],mtOff);
						else {
							if(orig==tilesetBuffer[0x1C5C>>1] || orig==tilesetBuffer[0x1C5E>>1] ||
							orig==tilesetBuffer[0x1C92>>1] || orig==tilesetBuffer[0x1C98>>1]) addObjectTile(o,tilesetBuffer[0x1D98>>1],mtOff);
							else if(orig!=tilesetBuffer[0x1D8C>>1] && orig!=tilesetBuffer[0x1D8E>>1] &&
							orig!=tilesetBuffer[0x1D9C>>1] && orig!=tilesetBuffer[0x1D9E>>1]) addObjectTile(o,tilesetBuffer[0x1DAC>>1],mtOff);
						}
					} else {
						if(orig==tilesetBuffer[0x1C5C>>1] || orig==tilesetBuffer[0x1C5E>>1] ||
						orig==tilesetBuffer[0x1C92>>1] || orig==tilesetBuffer[0x1C98>>1]) addObjectTile(o,tilesetBuffer[0x1D98>>1],mtOff);
						else if(orig!=tilesetBuffer[0x1D8C>>1] && orig!=tilesetBuffer[0x1D8E>>1] &&
						orig!=tilesetBuffer[0x1D9C>>1] && orig!=tilesetBuffer[0x1D9E>>1]) addObjectTile(o,tilesetBuffer[0x1D9E>>1],mtOff);
					}
				} else if((j+1)==height) {
					if(orig==tilesetBuffer[0x1C5C>>1] || orig==tilesetBuffer[0x1C5E>>1] ||
					orig==tilesetBuffer[0x1D92>>1] || orig==tilesetBuffer[0x1D94>>1] ||
					orig==tilesetBuffer[0x1D96>>1] || orig==tilesetBuffer[0x1D98>>1]) {
						if((i+1)==width) {
							if(orig==tilesetBuffer[0x1C5C>>1] || orig==tilesetBuffer[0x1C5E>>1]) addObjectTile(o,tilesetBuffer[0x1D98>>1],mtOff);
							else if((orig&0xFF00)==tilesetBuffer[0x1D8A>>1]) addObjectTile(o,(i&1)?tilesetBuffer[0x1D96>>1]:tilesetBuffer[0x1D94>>1],mtOff);
							else addObjectTile(o,tilesetBuffer[0x1DAC>>1],mtOff);
						} else addObjectTile(o,(i&1)?tilesetBuffer[0x1D96>>1]:tilesetBuffer[0x1D94>>1],mtOff);
					} else {
						if((i+1)==width) {
							if(orig==tilesetBuffer[0x1C5C>>1] || orig==tilesetBuffer[0x1C5E>>1]) addObjectTile(o,tilesetBuffer[0x1D98>>1],mtOff);
							else if((orig&0xFF00)==tilesetBuffer[0x1D8A>>1]) addObjectTile(o,(i&1)?tilesetBuffer[0x1D8E>>1]:tilesetBuffer[0x1D8C>>1],mtOff);
							else addObjectTile(o,tilesetBuffer[0x1DAC>>1],mtOff);
						} else addObjectTile(o,(i&1)?tilesetBuffer[0x1D8E>>1]:tilesetBuffer[0x1D8C>>1],mtOff);
					}
				} else {
					if((i+1)==width) {
						if(orig==tilesetBuffer[0x1D8A>>1] || orig==tilesetBuffer[0x1DAA>>1] ||
						orig==tilesetBuffer[0x1D90>>1] || orig==tilesetBuffer[0x1DAC>>1] ||
						orig==tilesetBuffer[0x1D8C>>1] || orig==tilesetBuffer[0x1D8E>>1]) addObjectTile(o,(i&1)?tilesetBuffer[0x1D8E>>1]:tilesetBuffer[0x1D8C>>1],mtOff);
						else if(orig!=tilesetBuffer[0x1D9C>>1] && orig!=tilesetBuffer[0x1D9E>>1]) {
							if(orig==tilesetBuffer[0x1DAE>>1]) addObjectTile(o,tilesetBuffer[0x1D9E>>1],mtOff);
							else addObjectTile(o,(i&1)?tilesetBuffer[0x1DAC>>1]:tilesetBuffer[0x1D90>>1],mtOff);
						}
					} else addObjectTile(o,(i&1)?tilesetBuffer[0x1D8E>>1]:tilesetBuffer[0x1D8C>>1],mtOff);
				}
				mtOff = offsetMap16Down(mtOff);
			}
			mtOff = preserve = offsetMap16Right(preserve);
			mtOff = preserve = offsetMap16Down(preserve);
			height--;
		}
	}
}
//Upside-down diagonal wall without edge
void drawObject_7C(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3];
	if(width&0x80) {
		width = 0x101-width;
		int height = width;
		for(int i=0; i<width; i++) {
			for(int j=0; j<height; j++) {
				WORD orig = getOriginalMap16Tile(mtOff);
				if((j+1)==height) {
					if(orig==tilesetBuffer[0x1D92>>1] || orig==tilesetBuffer[0x1D98>>1] ||
					(orig&0xFF00)!=tilesetBuffer[0x1D8A>>1]) addObjectTile(o,tilesetBuffer[0x1DA2>>1],mtOff);
				} else if((j+2)==height) addObjectTile(o,tilesetBuffer[0x1DA4>>1],mtOff);
				else addObjectTile(o,(j&1)?tilesetBuffer[0x1D8E>>1]:tilesetBuffer[0x1D8C>>1],mtOff);
				mtOff = offsetMap16Down(mtOff);
			}
			mtOff = preserve = offsetMap16Left(preserve);
			height--;
		}
	} else {
		width++;
		int height = width;
		for(int i=0; i<width; i++) {
			for(int j=0; j<height; j++) {
				WORD orig = getOriginalMap16Tile(mtOff);
				if((j+1)==height) {
					if(orig==tilesetBuffer[0x1D92>>1] || orig==tilesetBuffer[0x1D98>>1] ||
					(orig&0xFF00)!=tilesetBuffer[0x1D8A>>1]) addObjectTile(o,tilesetBuffer[0x1DA8>>1],mtOff);
				} else if((j+2)==height) addObjectTile(o,tilesetBuffer[0x1DA6>>1],mtOff);
				else addObjectTile(o,(j&1)?tilesetBuffer[0x1D8E>>1]:tilesetBuffer[0x1D8C>>1],mtOff);
				mtOff = offsetMap16Down(mtOff);
			}
			mtOff = preserve = offsetMap16Right(preserve);
			height--;
		}
	}
}
//Wall ledge
void drawObject_7D(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	for(int j=0; j<2; j++) {
		for(int i=0; i<width; i++) {
			WORD orig = getOriginalMap16Tile(mtOff);
			int tileRef = (orig==tilesetBuffer[0x1D8C>>1] || orig==tilesetBuffer[0x1D8E>>1])?0x20:0;
			if((i+1)==width) tileRef += 6;
			else if(i!=0) tileRef += (i&1)?2:4;
			if(j) tileRef += 8;
			if(orig==tilesetBuffer[0x1D8A>>1] || orig==tilesetBuffer[0x1DAA>>1] ||
			orig==tilesetBuffer[0x1D90>>1] || orig==tilesetBuffer[0x1DAC>>1]) tileRef += 0x10;
			tileRef = romBuf[0x09CD97+tileRef]|(romBuf[0x09CD98+tileRef]<<8);
			addObjectTile(o,tilesetBuffer[tileRef>>1],mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Wall platform
void drawObject_7E(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int width = o->data[3]+1;
	for(int i=0; i<width; i++) {
		if(i==0) addObjectTile(o,tilesetBuffer[0x1C8C>>1],mtOff);
		else if((i+1)==width) addObjectTile(o,tilesetBuffer[0x1C90>>1],mtOff);
		else addObjectTile(o,tilesetBuffer[0x1C8E>>1],mtOff);
		mtOff = offsetMap16Right(mtOff);
	}
}
//Wall hole
void drawObject_7F(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	if(width==1) width++;
	if(height==1) height++;
	for(int i=0; i<width; i++) {
		for(int j=0; j<height; j++) {
			WORD orig = getOriginalMap16Tile(mtOff);
			if((orig&0xFF00)==tilesetBuffer[0x1BE0>>1]) {
				int tileRef = 8;
				if(i==0) tileRef -= 2;
				else if((i+1)==width) tileRef += 2;
				if(j==0) tileRef -= 6;
				else if((j+1)==height) tileRef += 6;
				if(orig==tilesetBuffer[0x1C0C>>1] || orig==tilesetBuffer[0x1C0E>>1]) tileRef = romBuf[0x09CFC4+tileRef]|(romBuf[0x09CFC5+tileRef]<<8);
				else tileRef = romBuf[0x09CFB2+tileRef]|(romBuf[0x09CFB3+tileRef]<<8);
				if(tileRef==0xCFB0) addObjectTile(o,0,mtOff);
				else addObjectTile(o,tilesetBuffer[tileRef>>1],mtOff);
			} else if((orig&0xFF00)==tilesetBuffer[0x1D8A>>1]) {
				int tileRef = 0x09CEE6;
				if(i==0) tileRef -= 0x78;
				else if((i+1)==width) tileRef += 0x78;
				if(j==0) tileRef -= 0x28;
				else if((j+1)==height) tileRef += 0x28;
				tileRef += (orig&0xFF)<<1;
				tileRef = romBuf[tileRef]|(romBuf[tileRef+1]<<8);
				if(tileRef==0xCFB0) addObjectTile(o,0,mtOff);
				else if(tileRef!=0xCFAE) addObjectTile(o,tilesetBuffer[tileRef>>1],mtOff);
			}
			mtOff = offsetMap16Down(mtOff);
		}
		mtOff = preserve = offsetMap16Right(preserve);
	}
}
//Upside-down diagonal wall
void drawObject_80(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3];
	if(width&0x80) {
		width = 0x101-width;
		int height = width;
		for(int i=0; i<width; i++) {
			for(int j=0; j<height; j++) {
				WORD orig = getOriginalMap16Tile(mtOff);
				if((j+1)==height) {
					if((orig&0xFF00)!=tilesetBuffer[0x1D8A>>1]) addObjectTile(o,tilesetBuffer[0x1DA2>>1],mtOff);
				} else if((j+2)==height) addObjectTile(o,tilesetBuffer[0x1DA4>>1],mtOff);
				else if(orig==0 || (orig&0xFF00)==tilesetBuffer[0x1D8A>>1]) addObjectTile(o,(i&1)?tilesetBuffer[0x1D8E>>1]:tilesetBuffer[0x1D8C>>1],mtOff);
				mtOff = offsetMap16Down(mtOff);
			}
			mtOff = preserve = offsetMap16Left(preserve);
			height--;
		}
	} else {
		width++;
		int height = width;
		for(int i=0; i<width; i++) {
			for(int j=0; j<height; j++) {
				WORD orig = getOriginalMap16Tile(mtOff);
				if((j+1)==height) {
					if((orig&0xFF00)!=tilesetBuffer[0x1D8A>>1]) addObjectTile(o,tilesetBuffer[0x1DA8>>1],mtOff);
				} else if((j+2)==height) addObjectTile(o,tilesetBuffer[0x1DA6>>1],mtOff);
				else if(orig==0 || (orig&0xFF00)==tilesetBuffer[0x1D8A>>1]) addObjectTile(o,(i&1)?tilesetBuffer[0x1D8E>>1]:tilesetBuffer[0x1D8C>>1],mtOff);
				mtOff = offsetMap16Down(mtOff);
			}
			mtOff = preserve = offsetMap16Right(preserve);
			height--;
		}
	}
}
//Diagonal wall
void drawObject_81(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3];
	if(width&0x80) {
		width = 0x101-width;
		int height = width;
		for(int i=0; i<width; i++) {
			for(int j=0; j<height; j++) {
				WORD orig = getOriginalMap16Tile(mtOff);
				if(j==0) {
					if((orig&0xFF00)!=tilesetBuffer[0x1D8A>>1]) addObjectTile(o,tilesetBuffer[0x1D9A>>1],mtOff);
				} else if(j==1) addObjectTile(o,tilesetBuffer[0x1D9C>>1],mtOff);
				else if(orig==0 || (orig&0xFF00)==tilesetBuffer[0x1D8A>>1]) addObjectTile(o,(i&1)?tilesetBuffer[0x1D8E>>1]:tilesetBuffer[0x1D8C>>1],mtOff);
				mtOff = offsetMap16Down(mtOff);
			}
			mtOff = preserve = offsetMap16Left(preserve);
			mtOff = preserve = offsetMap16Down(preserve);
			height--;
		}
	} else {
		width++;
		int height = width;
		for(int i=0; i<width; i++) {
			for(int j=0; j<height; j++) {
				WORD orig = getOriginalMap16Tile(mtOff);
				if(j==0) {
					if((orig&0xFF00)!=tilesetBuffer[0x1D8A>>1]) addObjectTile(o,tilesetBuffer[0x1DA0>>1],mtOff);
				} else if(j==1) addObjectTile(o,tilesetBuffer[0x1D9E>>1],mtOff);
				else if(orig==0 || (orig&0xFF00)==tilesetBuffer[0x1D8A>>1]) addObjectTile(o,(i&1)?tilesetBuffer[0x1D8E>>1]:tilesetBuffer[0x1D8C>>1],mtOff);
				mtOff = offsetMap16Down(mtOff);
			}
			mtOff = preserve = offsetMap16Right(preserve);
			mtOff = preserve = offsetMap16Down(preserve);
			height--;
		}
	}
}
//Green coin
void drawObject_82(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	for(int j=0; j<height; j++) {
		for(int i=0; i<width; i++) {
			addObjectTile(o,0xA400,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Green coin, horizontal
void drawObject_83(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int width = o->data[3]+1;
	for(int i=0; i<width; i++) {
		if((i&1)==0) addObjectTile(o,0xA400,mtOff);
		mtOff = offsetMap16Right(mtOff);
	}
}
//Green coin, diagonal
void drawObject_84(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int width = o->data[3];
	if(width&0x80) {
		width = 0x101-width;
		for(int i=0; i<width; i++) {
			if((i&1)==0) addObjectTile(o,0xA400,mtOff);
			mtOff = offsetMap16Left(mtOff);
			mtOff = offsetMap16Down(mtOff);
		}
	} else {
		width++;
		for(int i=0; i<width; i++) {
			if((i&1)==0) addObjectTile(o,0xA400,mtOff);
			mtOff = offsetMap16Right(mtOff);
			mtOff = offsetMap16Down(mtOff);
		}
	}
}
//Cross section upside-down very steep slope right
void drawObject_85(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	for(int i=0; i<width; i++) {
		for(int j=0; j<height; j++) {
			WORD orig = getOriginalMap16Tile(mtOff);
			if((height-j)==1) addObjectTile(o,tilesetBuffer[0x1DF6>>1],mtOff);
			else if((height-j)==2) addObjectTile(o,tilesetBuffer[0x1DF2>>1],mtOff);
			else {
				int offset = noiseTilemap[mtOff]&0x0E;
				int tileRef = romBuf[0x098121+offset]|(romBuf[0x098122+offset]<<8);
				addObjectTile(o,tilesetBuffer[tileRef>>1],mtOff);
			}
			if(i==0) {
				int mtOff2 = offsetMap16Left(mtOff);
				overlapTile_groundXSectEdge(o,mtOff2,0x09C194);
				if((height-j)==1) {
					mtOff2 = offsetMap16Down(mtOff2);
					WORD orig2 = getOriginalMap16Tile(mtOff2);
					if(orig2==tilesetBuffer[0x1C04>>1]) addObjectTile(o,tilesetBuffer[0x1D2C>>1],mtOff2);
				}
			}
			if((i+1)==width) {
				if((height-j)==2) {
					int mtOff2 = offsetMap16Right(mtOff);
					WORD orig2 = getOriginalMap16Tile(mtOff2);
					if(orig2==tilesetBuffer[0x1C04>>1]) addObjectTile(o,tilesetBuffer[0x1D2E>>1],mtOff2);
				} else if((height-j)>2) {
					int mtOff2 = offsetMap16Right(mtOff);
					overlapTile_groundXSectEdge(o,mtOff2,0x09C20F);
				}
			}
			if((height-j)==1) {
				int mtOff2 = offsetMap16Down(mtOff);
				WORD orig2 = getOriginalMap16Tile(mtOff2);
				if(orig2==tilesetBuffer[0x1C04>>1]) addObjectTile(o,tilesetBuffer[0x1D2E>>1],mtOff2);
			}
			mtOff = offsetMap16Down(mtOff);
		}
		mtOff = preserve = offsetMap16Right(preserve);
		height -= 2;
	}
}
//Cross section upside-down very steep slope left
void drawObject_86(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	height -= width;
	if(height<1) height = 1;
	for(int i=0; i<width; i++) {
		for(int j=0; j<height; j++) {
			WORD orig = getOriginalMap16Tile(mtOff);
			if((height-j)==1) addObjectTile(o,tilesetBuffer[0x1DEE>>1],mtOff);
			else if((height-j)==2) addObjectTile(o,tilesetBuffer[0x1DEA>>1],mtOff);
			else {
				int offset = noiseTilemap[mtOff]&0x0E;
				int tileRef = romBuf[0x098121+offset]|(romBuf[0x098122+offset]<<8);
				addObjectTile(o,tilesetBuffer[tileRef>>1],mtOff);
			}
			if(i==0 && (height-j)>2) {
				int mtOff2 = offsetMap16Left(mtOff);
				overlapTile_groundXSectEdge(o,mtOff2,0x09C194);
			}
			if((i+1)==width) {
				int mtOff2 = offsetMap16Right(mtOff);
				overlapTile_groundXSectEdge(o,mtOff2,0x09C20F);
				if((height-j)==1) {
					mtOff2 = offsetMap16Down(mtOff2);
					WORD orig2 = getOriginalMap16Tile(mtOff2);
					if(orig2==tilesetBuffer[0x1C04>>1]) addObjectTile(o,tilesetBuffer[0x1D2E>>1],mtOff2);
				}
			}
			if((height-j)==1) {
				int mtOff2 = offsetMap16Down(mtOff);
				WORD orig2 = getOriginalMap16Tile(mtOff2);
				if(orig2==tilesetBuffer[0x1C04>>1]) addObjectTile(o,tilesetBuffer[0x1D2C>>1],mtOff2);
			}
			mtOff = offsetMap16Down(mtOff);
		}
		mtOff = preserve = offsetMap16Right(preserve);
		height += 2;
	}
}
//Ground without grass
void drawObject_87(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	int objRef = (o->data[0]-0x87)<<3;
	for(int i=0; i<width; i++) {
		for(int j=0; j<height; j++) {
			WORD orig = getOriginalMap16Tile(mtOff);
			if(j==0) {
				if(i==0) {
					if(orig==tilesetBuffer[0x1C5C>>1] || orig==tilesetBuffer[0x1C5E>>1] ||
					orig==tilesetBuffer[0x1CB6>>1] || orig==tilesetBuffer[0x1CB8>>1]) addObjectTile(o,tilesetBuffer[0x1D14>>1],mtOff);
				} else if((i+1)==width) {
					if(orig==tilesetBuffer[0x1C5C>>1] || orig==tilesetBuffer[0x1C5E>>1] ||
					orig==tilesetBuffer[0x1CB6>>1] || orig==tilesetBuffer[0x1CB8>>1]) addObjectTile(o,tilesetBuffer[0x1D12>>1],mtOff);
				} else if(orig==tilesetBuffer[0x1C5C>>1] || orig==tilesetBuffer[0x1C5E>>1]) addObjectTile(o,0,mtOff);
			} else if(j==1) {
				if(i==0) {
					if(orig==tilesetBuffer[0x1C5C>>1] || orig==tilesetBuffer[0x1C5E>>1] ||
					orig==tilesetBuffer[0x1CB6>>1] || orig==tilesetBuffer[0x1CB8>>1]) addObjectTile(o,0x0145,mtOff);
					else {
						int offset = noiseTilemap[mtOff]&0xE;
						offset |= objRef;
						WORD tile = romBuf[0x09D246+offset]|(romBuf[0x09D247+offset]<<8);
						addObjectTile(o,tile,mtOff);
					}
				} else if((i+1)==width) {
					if(orig==tilesetBuffer[0x1C5C>>1] || orig==tilesetBuffer[0x1C5E>>1] ||
					orig==tilesetBuffer[0x1CB6>>1] || orig==tilesetBuffer[0x1CB8>>1]) addObjectTile(o,0x0150,mtOff);
					else {
						int offset = noiseTilemap[mtOff]&0xE;
						offset |= objRef;
						WORD tile = romBuf[0x09D246+offset]|(romBuf[0x09D247+offset]<<8);
						addObjectTile(o,tile,mtOff);
					}
				} else {
					int offset = noiseTilemap[mtOff]&0xE;
					offset |= objRef;
					WORD tile = romBuf[0x09D246+offset]|(romBuf[0x09D247+offset]<<8);
					addObjectTile(o,tile,mtOff);
				}
			} else {
				if((j+1)==height) overlapTile_stdLedgeBottom(o,mtOff,orig);
				else overlapTile_stdLedgeMid(o,mtOff,orig);
			}
			mtOff = offsetMap16Down(mtOff);
		}
		mtOff = preserve = offsetMap16Right(preserve);
	}
}
//Falling rock platform
void drawObject_89(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	if(width==1) {
		if(height==1) addObjectTile(o,0x720D,mtOff);
		else {
			for(int j=0; j<height; j++) {
				if(j==0) addObjectTile(o,0x720C,mtOff);
				else if((j+1)==height) addObjectTile(o,0x720F,mtOff);
				else addObjectTile(o,(j&1)?0x7213:0x720E,mtOff);
				mtOff = offsetMap16Down(mtOff);
			}
		}
	} else {
		if(height==1) {
			for(int i=0; i<width; i++) {
				if(i==0) addObjectTile(o,0x7209,mtOff);
				else if((i+1)==width) addObjectTile(o,0x720B,mtOff);
				else addObjectTile(o,0x720A,mtOff);
				mtOff = offsetMap16Right(mtOff);
			}
		} else {
			for(int j=0; j<height; j++) {
				for(int i=0; i<width; i++) {
					int offset = 14;
					if(i==0) offset -= 2;
					else if((i+1)==width) offset += 2;
					if(j==0) offset -= 12;
					else if((j+1)==height) offset += 6;
					else if(j&1) offset -= 6;
					WORD tile = romBuf[0x09D33C+offset]|(romBuf[0x09D33D+offset]<<8);
					addObjectTile(o,tile,mtOff);
					mtOff = offsetMap16Right(mtOff);
				}
				mtOff = preserve = offsetMap16Down(preserve);
			}
		}
	}
}
//Switch coin
void drawObject_8A(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	for(int j=0; j<height; j++) {
		for(int i=0; i<width; i++) {
			addObjectTile(o,0x7400,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Switch block
void drawObject_8B(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	for(int j=0; j<height; j++) {
		for(int i=0; i<width; i++) {
			addObjectTile(o,0x7300,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Bomb brigade room
void drawObject_8C(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int width = o->data[3]+1;
	for(int i=0; i<width; i++) {
		int offset = i&1;
		WORD orig = getOriginalMap16Tile(mtOff);
		if(orig<0x00B6 || orig>0x00BA) addObjectTile(o,0x016F+offset,mtOff);
		int mtOff2 = offsetMap16Down(mtOff);
		WORD orig2 = getOriginalMap16Tile(mtOff2);
		if(orig2<0x00B6 || orig2>0x00BA) addObjectTile(o,0x0171+offset,mtOff2);
		mtOff2 = offsetMap16Down(mtOff2);
		orig2 = getOriginalMap16Tile(mtOff2);
		if((orig2==0x00C3 && i) || orig2==0x00C7 ||
		orig2==0x00C2) addObjectTile(o,0x00C6,mtOff2);
		else if(orig2==0x00C5) addObjectTile(o,0x00D5,mtOff2);
		mtOff = offsetMap16Right(mtOff);
	}
}
//Tree trunk
void drawObject_8D(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int height = o->data[3]+1;
	for(int j=0; j<height; j++) {
		if((j+1)==height) addObjectTile(o,0x3D6F,mtOff);
		else addObjectTile(o,(noiseTilemap[mtOff]&1)?0x3DA7:0x3D70,mtOff);
		mtOff = offsetMap16Down(mtOff);
	}
}
//Giant donut lift
void drawObject_8E(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int width = o->data[3]+1;
	if(width&1) width++;
	for(int i=0; i<width; i++) {
		int offset = i&1;
		addObjectTile(o,0x7500+offset,mtOff);
		int mtOff2 = offsetMap16Down(mtOff);
		addObjectTile(o,0x3DAA+offset,mtOff2);
		mtOff = offsetMap16Right(mtOff);
	}
}
//Stuck log, gentle slope
void drawObject_8F(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3];
	int height = o->data[4]+1;
	if(width&0x80) {
		width = 0x101-width;
		for(int i=0; i<width; i++) {
			for(int j=0; j<height; j++) {
				WORD orig = getOriginalMap16Tile(mtOff);
				int tileRef = 0;
				if(orig==tilesetBuffer[0x1C5C>>1] || orig==tilesetBuffer[0x1C5E>>1]) tileRef = 1;
				else if(orig==0x3DB0 || orig==0x3DB1 || orig==0x3DB9 || orig==0x3DBA) tileRef = 2;
				else if(orig && orig!=tilesetBuffer[0x1A0C>>1] && orig!=tilesetBuffer[0x1A18>>1]) tileRef = 3;
				if(i==0) {
					if(j==0) addObjectTile(o,tilesetBuffer[0x1A22>>1],mtOff);
					else if(j==1) addObjectTile(o,tileRef?0x3DDA:0x3DBE,mtOff);
				} else if(i&1) {
					if(j==0) {
						if(tileRef<2) addObjectTile(o,tileRef?tilesetBuffer[0x1A38>>1]:tilesetBuffer[0x1A36>>1],mtOff);
					} else if(j==1) {
						if(tileRef<2) addObjectTile(o,tileRef?0x3DD9:0x3DBD,mtOff);
					}
				} else {
					if(j==0) {
						if(tileRef<2) addObjectTile(o,tileRef?tilesetBuffer[0x1A32>>1]:tilesetBuffer[0x1A2C>>1],mtOff);
					} else if(j==1) {
						if(tileRef<2) addObjectTile(o,tileRef?0x3DD8:0x3DBC,mtOff);
					} else if(j==2) {
						if(tileRef<2) addObjectTile(o,tileRef?0x3DDA:0x3DBE,mtOff);
					}
				}
				mtOff = offsetMap16Down(mtOff);
			}
			mtOff = preserve = offsetMap16Left(preserve);
			if(i && (i&1)==0) {
				mtOff = preserve = offsetMap16Down(preserve);
				height--;
			}
		}
	} else {
		width++;
		for(int i=0; i<width; i++) {
			for(int j=0; j<height; j++) {
				WORD orig = getOriginalMap16Tile(mtOff);
				int tileRef = 0;
				if(orig==tilesetBuffer[0x1C5C>>1] || orig==tilesetBuffer[0x1C5E>>1]) tileRef = 1;
				else if(orig==0x3DB0 || orig==0x3DB1 || orig==0x3DB9 || orig==0x3DBA) tileRef = 2;
				else if(orig && orig!=tilesetBuffer[0x1A0C>>1] && orig!=tilesetBuffer[0x1A18>>1]) tileRef = 3;
				if(i==0) {
					if(j==0) addObjectTile(o,tilesetBuffer[0x1A0E>>1],mtOff);
					else if(j==1) addObjectTile(o,tileRef?0x3DDB:0x3DBF,mtOff);
				} else if(i&1) {
					if(j==0) {
						if(tileRef<2) addObjectTile(o,tileRef?tilesetBuffer[0x1A48>>1]:tilesetBuffer[0x1A46>>1],mtOff);
					} else if(j==1) {
						if(tileRef<2) addObjectTile(o,tileRef?0x3DDC:0x3DC0,mtOff);
					}
				} else {
					if(j==0) {
						if(tileRef<2) addObjectTile(o,tileRef?tilesetBuffer[0x1A58>>1]:tilesetBuffer[0x1A56>>1],mtOff);
					} else if(j==1) {
						if(tileRef<2) addObjectTile(o,tileRef?0x3DDD:0x3DC1,mtOff);
					} else if(j==2) {
						if(tileRef<2) addObjectTile(o,tileRef?0x3DDB:0x3DBF,mtOff);
					}
				}
				mtOff = offsetMap16Down(mtOff);
			}
			mtOff = preserve = offsetMap16Right(preserve);
			if(i && (i&1)==0) {
				mtOff = preserve = offsetMap16Down(preserve);
				height--;
			}
		}
	}
}
//Stuck log, steep slope
void drawObject_90(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3];
	int height = o->data[4]+1;
	if(width&0x80) {
		width = 0x101-width;
		for(int i=0; i<width; i++) {
			for(int j=0; j<height; j++) {
				WORD orig = getOriginalMap16Tile(mtOff);
				int tileRef = 0;
				if(orig==tilesetBuffer[0x1C5C>>1] || orig==tilesetBuffer[0x1C5E>>1]) tileRef = 1;
				else if(orig) {
					mtOff = offsetMap16Down(mtOff);
					continue;
				}
				if(i==0) {
					if(j==0) addObjectTile(o,tilesetBuffer[0x1A18>>1],mtOff);
					else if(j==1) addObjectTile(o,tileRef?0x3DB5:0x3DB0,mtOff);
				} else {
					if(j==0) addObjectTile(o,tileRef?tilesetBuffer[0x1A04>>1]:tilesetBuffer[0x1A06>>1],mtOff);
					else if(j==1) addObjectTile(o,tileRef?0x3DB4:0x3DB8,mtOff);
					else if(j==2) addObjectTile(o,tileRef?0x3DB5:0x3DB9,mtOff);
				}
				mtOff = offsetMap16Down(mtOff);
			}
			mtOff = preserve = offsetMap16Left(preserve);
			if(i) {
				mtOff = preserve = offsetMap16Down(preserve);
				height--;
			}
		}
	} else {
		width++;
		for(int i=0; i<width; i++) {
			for(int j=0; j<height; j++) {
				WORD orig = getOriginalMap16Tile(mtOff);
				int tileRef = 0;
				if(orig==tilesetBuffer[0x1C5C>>1] || orig==tilesetBuffer[0x1C5E>>1]) tileRef = 1;
				else if(orig) {
					mtOff = offsetMap16Down(mtOff);
					continue;
				}
				if(i==0) {
					if(j==0) addObjectTile(o,tilesetBuffer[0x1A0C>>1],mtOff);
					else if(j==1) addObjectTile(o,tileRef?0x3DB6:0x3DB1,mtOff);
				} else {
					if(j==0) addObjectTile(o,tileRef?tilesetBuffer[0x1A20>>1]:tilesetBuffer[0x1A1E>>1],mtOff);
					else if(j==1) addObjectTile(o,tileRef?0x3DB7:0x3DBB,mtOff);
					else if(j==2) addObjectTile(o,tileRef?0x3DB6:0x3DBA,mtOff);
				}
				mtOff = offsetMap16Down(mtOff);
			}
			mtOff = preserve = offsetMap16Right(preserve);
			if(i) {
				mtOff = preserve = offsetMap16Down(preserve);
				height--;
			}
		}
	}
}
//Tree wth spore, left
void drawObject_91(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int height = o->data[3]+1;
	for(int j=0; j<height; j++) {
		if(j==0) {
			addObjectTile(o,0x3DC3,mtOff);
			int mtOff2 = offsetMap16Left(mtOff);
			addObjectTile(o,0x3DC2,mtOff2);
			mtOff2 = offsetMap16Right(mtOff);
			addObjectTile(o,0x3DC4,mtOff2);
		} else if(j==1) {
			addObjectTile(o,0x3DC9,mtOff);
			int mtOff2 = offsetMap16Left(mtOff);
			addObjectTile(o,0x3DC8,mtOff2);
			mtOff2 = offsetMap16Right(mtOff);
			addObjectTile(o,0x3DCA,mtOff2);
		} else if((j+1)==height) {
			WORD orig = getOriginalMap16Tile(mtOff);
			if(orig==tilesetBuffer[0x1A06>>1]) addObjectTile(o,tilesetBuffer[0x1A0A>>1],mtOff);
			else if(orig==tilesetBuffer[0x1A2C>>1]) addObjectTile(o,tilesetBuffer[0x1A30>>1],mtOff);
			else addObjectTile(o,0x3DAC,mtOff);
		} else addObjectTile(o,0x3DB2,mtOff);
		mtOff = offsetMap16Down(mtOff);
	}
}
//Tree wth spore, right
void drawObject_92(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int height = o->data[3]+1;
	for(int j=0; j<height; j++) {
		if(j==0) {
			addObjectTile(o,0x3DC6,mtOff);
			int mtOff2 = offsetMap16Left(mtOff);
			addObjectTile(o,0x3DC5,mtOff2);
			mtOff2 = offsetMap16Right(mtOff);
			addObjectTile(o,0x3DC7,mtOff2);
		} else if(j==1) {
			addObjectTile(o,0x3DAE,mtOff);
			int mtOff2 = offsetMap16Left(mtOff);
			addObjectTile(o,0x3DCB,mtOff2);
			mtOff2 = offsetMap16Right(mtOff);
			addObjectTile(o,0x3DAF,mtOff2);
		} else if((j+1)==height) {
			WORD orig = getOriginalMap16Tile(mtOff);
			if(orig==tilesetBuffer[0x1A1E>>1]) addObjectTile(o,tilesetBuffer[0x1A1A>>1],mtOff);
			else if(orig==tilesetBuffer[0x1A56>>1]) addObjectTile(o,tilesetBuffer[0x1A52>>1],mtOff);
			else addObjectTile(o,0x3DAD,mtOff);
		} else addObjectTile(o,0x3DB3,mtOff);
		mtOff = offsetMap16Down(mtOff);
	}
}
//Palm tree
void drawObject_93(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int height = o->data[3]+1;
	for(int j=0; j<height; j++) {
		if(j==0) {
			addObjectTile(o,0x3DCF,mtOff);
			int mtOff2 = offsetMap16Left(mtOff);
			addObjectTile(o,0x3DCE,mtOff2);
			mtOff2 = offsetMap16Right(mtOff);
			addObjectTile(o,0x3DD0,mtOff2);
		} else if(j==1) {
			addObjectTile(o,0x3DD2,mtOff);
			int mtOff2 = offsetMap16Left(mtOff);
			addObjectTile(o,0x3DD1,mtOff2);
			mtOff2 = offsetMap16Right(mtOff);
			addObjectTile(o,0x3DD3,mtOff2);
			mtOff2 = offsetMap16Right(mtOff2);
			addObjectTile(o,0x3DD4,mtOff2);
		} else if((j+1)==height) {
			addObjectTile(o,0x3DD6,mtOff);
			int mtOff2 = offsetMap16Right(mtOff);
			addObjectTile(o,0x3DD7,mtOff2);
		} else addObjectTile(o,0x3DD5,mtOff);
		mtOff = offsetMap16Down(mtOff);
	}
}
//Number platform
void drawObject_94(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	if(width&1) width++;
	if(height&1) height++;
	int objRef = (o->data[0]-0x94)<<1;
	for(int j=0; j<height; j++) {
		for(int i=0; i<width; i++) {
			int offset = (i&1);
			if(j&1) addObjectTile(o,0x7775+objRef+offset,mtOff);
			else addObjectTile(o,0x7600+objRef+offset,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Waterfall
void drawObject_98(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	for(int j=0; j<height; j++) {
		for(int i=0; i<width; i++) {
			if(j==0) addObjectTile(o,(i&1)?0x7754:0x7750,mtOff);
			else if(j==1) {
				if(i==0) {
					if((i+1)==width) addObjectTile(o,0x7804,mtOff);
					else addObjectTile(o,0x7800,mtOff);
				} else if((i+1)==width) addObjectTile(o,0x7803,mtOff);
				else addObjectTile(o,(i&1)?0x7802:0x7801,mtOff);
			} else {
				int offset = (i&1)^(j&1);
				addObjectTile(o,offset?0x01B8:0x01B7,mtOff);
			}
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Waterfall with rock platform
void drawObject_99(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int height = o->data[3]+1;
	for(int j=0; j<height; j++) {
		if(j==0) {
			addObjectTile(o,0x01BA,mtOff);
			int mtOff2 = offsetMap16Left(mtOff);
			addObjectTile(o,0x01B9,mtOff2);
			mtOff2 = offsetMap16Right(mtOff);
			addObjectTile(o,0x01BB,mtOff2);
		} else if(j==1) {
			addObjectTile(o,0x01BD,mtOff);
			int mtOff2 = offsetMap16Left(mtOff);
			addObjectTile(o,0x01BC,mtOff2);
			mtOff2 = offsetMap16Right(mtOff);
			addObjectTile(o,0x01BE,mtOff2);
		} else {
			WORD orig = getOriginalMap16Tile(mtOff);
			overlapTile_stdLedgeMid(o,mtOff,orig);
		}
		mtOff = offsetMap16Down(mtOff);
	}
}
//Curly cave plant 1
void drawObject_9A(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int height = o->data[3]+1;
	int seed = noiseTilemap[mtOff]&0xC;
	int seed2 = seed^0xC;
	for(int j=0; j<height; j++) {
		if(j==0) {
			WORD tile = (height&1)?0x7700:0x7730;
			tile |= seed;
			addObjectTile(o,tile+2,mtOff);
			int mtOff2 = offsetMap16Left(mtOff);
			addObjectTile(o,tile+1,mtOff2);
			mtOff2 = offsetMap16Left(mtOff2);
			if((height&1)==0) addObjectTile(o,tile,mtOff2);
			mtOff2 = offsetMap16Right(mtOff);
			if(height&1) addObjectTile(o,tile+3,mtOff2);
		} else if(j==1) {
			WORD tile = (height&1)?0x7710:0x7740;
			tile |= seed;
			addObjectTile(o,tile+2,mtOff);
			int mtOff2 = offsetMap16Left(mtOff);
			addObjectTile(o,tile+1,mtOff2);
			mtOff2 = offsetMap16Left(mtOff2);
			addObjectTile(o,tile,mtOff2);
			mtOff2 = offsetMap16Right(mtOff);
			addObjectTile(o,tile+3,mtOff2);
		} else if((j+1)==height) {
			int mtOff2 = mtOff;
			if((height&1)==0) mtOff2 = offsetMap16Left(mtOff);
			addObjectTile(o,0x7723|seed2,mtOff2);
		} else {
			int mtOff2 = mtOff;
			if((height&1)==0) mtOff2 = offsetMap16Left(mtOff);
			WORD tile = ((height-j)&1)?0x7700:0x7733;
			addObjectTile(o,tile|seed2,mtOff2);
		}
		mtOff = offsetMap16Down(mtOff);
	}
}
//Curly cave plant 2
void drawObject_9B(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int height = o->data[3]+1;
	int seed = noiseTilemap[mtOff]&0xC;
	int seed2 = seed^0xC;
	for(int j=0; j<height; j++) {
		if(j==0) {
			int offset = (seed>>1)+((height&1)<<3);
			WORD tile = romBuf[0x09D935+offset]|(romBuf[0x09D936+offset]<<8);
			addObjectTile(o,tile,mtOff);
		} else if((j+1)==height) {
			addObjectTile(o,0x7723|seed2,mtOff);
		} else {
			WORD tile = ((height-j)&1)?0x7700:0x7733;
			addObjectTile(o,tile|seed2,mtOff);
		}
		mtOff = offsetMap16Down(mtOff);
	}
}
//Curly cave plant 3
void drawObject_9C(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int height = o->data[3]+1;
	int seed = noiseTilemap[mtOff]&0xC;
	int seed2 = seed^0xC;
	for(int j=0; j<height; j++) {
		if(j<2) {
			int offset = (seed>>1)+(j<<3)+((height&1)<<4);
			WORD tile = romBuf[0x09D9A8+offset]|(romBuf[0x09D9A9+offset]<<8);
			addObjectTile(o,tile,mtOff);
		} else if((j+1)==height) {
			addObjectTile(o,0x7723|seed2,mtOff);
		} else {
			WORD tile = ((height-j)&1)?0x7700:0x7733;
			addObjectTile(o,tile|seed2,mtOff);
		}
		mtOff = offsetMap16Down(mtOff);
	}
}
//Rock platform
void drawObject_9D(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	for(int j=0; j<height; j++) {
		for(int i=0; i<width; i++) {
			int offset = 7;
			if(i==0) offset--;
			else if((i+1)==width) offset++;
			if(j==0) offset -= 6;
			else if((j+1)==height) offset += 3;
			else if(j&1) offset -= 3;
			if((j+1)==height) {
				WORD orig = getOriginalMap16Tile(mtOff);
				if(orig==tilesetBuffer[0x1C5C>>1] || orig==tilesetBuffer[0x1C5E>>1]) offset += 3;
			}
			addObjectTile(o,0x7900+offset,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Donut platform
void drawObject_9E(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int width = o->data[3]+1;
	for(int i=0; i<width; i++) {
		addObjectTile(o,0x7502,mtOff);
		mtOff = offsetMap16Right(mtOff);
	}
}
//Raven platform
void drawObject_9F(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int width = o->data[3]+1;
	if((width&3)==1) width++;
	for(int i=0; i<width; i++) {
		if((i&2)==0) {
			addObjectTile(o,(i&1)?0x3508:0x3308,mtOff);
			int mtOff2 = offsetMap16Down(mtOff);
			addObjectTile(o,(i&1)+0x0004,mtOff2);
		}
		mtOff = offsetMap16Right(mtOff);
	}
}
//Colored breakable castle blocks
void drawObject_A0(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	if(width&1) width++;
	int objRef = (o->data[0]-0xA0)<<1;
	for(int j=0; j<height; j++) {
		for(int i=0; i<width; i++) {
			addObjectTile(o,0x7A00+objRef+(i&1),mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Stone breakable castle blocks
void drawObject_A3(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	if(width&1) width++;
	if(height&1) height++;
	int objRef = (o->data[0]-0xA3)<<2;
	for(int j=0; j<height; j++) {
		for(int i=0; i<width; i++) {
			int offset = (i&1)+((j&1)<<1);
			addObjectTile(o,0x7B00+objRef+offset,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Double-ended vertical pipe for castles (broken in 3D)
void drawObject_A5(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int height = o->data[3]+1;
	int bg1Ts = ((levelHeader[0]&7)<<1)|(levelHeader[1]>>7);
	if(bg1Ts==3) {
		for(int j=0; j<height; j++) {
			int mtOff2 = offsetMap16Right(mtOff);
			if(j==0) {
				addObjectTile(o,0x3D29,mtOff);
				addObjectTile(o,0x3D2A,mtOff2);
			} else if((j+1)==height) {
				addObjectTile(o,0x7D1C,mtOff);
				addObjectTile(o,0x7D1D,mtOff2);
			} else if((j+2)==height) {
				addObjectTile(o,0x9050,mtOff);
				addObjectTile(o,0x9051,mtOff2);
			} else {
				addObjectTile(o,0x00A0,mtOff);
				addObjectTile(o,0x00A1,mtOff2);
			}
			mtOff = offsetMap16Down(mtOff);
		}
	} else {
		for(int j=0; j<height; j++) {
			int mtOff2 = offsetMap16Right(mtOff);
			WORD orig = getOriginalMap16Tile(mtOff);
			if(j==0) {
				if(orig==0 || orig==0x1600) {
					addObjectTile(o,0x7D02,mtOff);
					addObjectTile(o,0x7D03,mtOff2);
				}
			} else if((j+1)==height) {
				if(orig==0 || orig==0x1600) {
					addObjectTile(o,0x7D06,mtOff);
					addObjectTile(o,0x7D07,mtOff2);
				}
			} else {
				WORD tile = (j&1)?0x01C7:0x01C9;
				addObjectTile(o,tile,mtOff);
				addObjectTile(o,tile+1,mtOff2);
			}
			mtOff = offsetMap16Down(mtOff);
		}
	}
}
//Double-ended horizontal pipe for castles and 3D
void drawObject_A6(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int width = o->data[3]+1;
	int bg1Ts = ((levelHeader[0]&7)<<1)|(levelHeader[1]>>7);
	if(bg1Ts==3) {
		for(int i=0; i<width; i++) {
			int mtOff2 = offsetMap16Down(mtOff);
			int mtOff3 = offsetMap16Down(mtOff2);
			int mtOff4 = offsetMap16Down(mtOff3);
			if(i==0) {
				addObjectTile(o,0x3D2B,mtOff);
				addObjectTile(o,0x7D1E,mtOff2);
				addObjectTile(o,0x7D1F,mtOff3);
				addObjectTile(o,0x9056,mtOff4);
			} else if((i+1)==width) {
				addObjectTile(o,0x3D2E,mtOff);
				addObjectTile(o,0x7D20,mtOff2);
				addObjectTile(o,0x7D21,mtOff3);
				addObjectTile(o,0x9059,mtOff4);
			} else {
				int offset = (i&1);
				addObjectTile(o,offset?0x3D2D:0x3D2C,mtOff);
				addObjectTile(o,offset?0x9053:0x9052,mtOff2);
				addObjectTile(o,offset?0x9055:0x9054,mtOff3);
				addObjectTile(o,offset?0x9058:0x9057,mtOff4);
			}
			mtOff = offsetMap16Right(mtOff);
		}
	} else {
		for(int i=0; i<width; i++) {
			int mtOff2 = offsetMap16Down(mtOff);
			WORD orig = getOriginalMap16Tile(mtOff);
			if(i==0) {
				if(orig==0 || orig==0x1600) {
					addObjectTile(o,0x7D00,mtOff);
					addObjectTile(o,0x7D01,mtOff2);
				}
			} else if((i+1)==width) {
				if(orig==0 || orig==0x1600) {
					addObjectTile(o,0x7D04,mtOff);
					addObjectTile(o,0x7D05,mtOff2);
				}
			} else {
				int offset = (i&1);
				addObjectTile(o,offset?0x01C3:0x01C4,mtOff);
				addObjectTile(o,offset?0x01C6:0x01C5,mtOff2);
			}
			mtOff = offsetMap16Right(mtOff);
		}
	}
}
//Spike blocks
void drawObject_A7(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	for(int j=0; j<height; j++) {
		for(int i=0; i<width; i++) {
			addObjectTile(o,0x7C00,mtOff);
			if(i==0) {
				int mtOff2 = offsetMap16Left(mtOff);
				WORD orig2 = getOriginalMap16Tile(mtOff2);
				if(orig2==0) addObjectTile(o,0x777E,mtOff2);
				else if(orig2>0x777C && orig2<0x778C) {
					WORD tile = (orig2-0x777C)|2;
					addObjectTile(o,0x777C+tile,mtOff2);
				}
			}
			if((i+1)==width) {
				int mtOff2 = offsetMap16Right(mtOff);
				WORD orig2 = getOriginalMap16Tile(mtOff2);
				if(orig2==0) addObjectTile(o,0x777D,mtOff2);
				else if(orig2>0x777C && orig2<0x778C) {
					WORD tile = (orig2-0x777C)|1;
					addObjectTile(o,0x777C+tile,mtOff2);
				}
			}
			if(j==0) {
				int mtOff2 = offsetMap16Up(mtOff);
				WORD orig2 = getOriginalMap16Tile(mtOff2);
				if(orig2==0) addObjectTile(o,0x7780,mtOff2);
				else if(orig2>0x777C && orig2<0x778C) {
					WORD tile = (orig2-0x777C)|4;
					addObjectTile(o,0x777C+tile,mtOff2);
				}
			}
			if((j+1)==height) {
				int mtOff2 = offsetMap16Down(mtOff);
				WORD orig2 = getOriginalMap16Tile(mtOff2);
				if(orig2==0) addObjectTile(o,0x7784,mtOff2);
				else if(orig2>0x777C && orig2<0x778C) {
					WORD tile = (orig2-0x777C)|8;
					addObjectTile(o,0x777C+tile,mtOff2);
				}
			}
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Spike blocks remover
void drawObject_A8(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	for(int j=0; j<height; j++) {
		for(int i=0; i<width; i++) {
			int mtOff2 = offsetMap16Left(mtOff);
			WORD orig2 = getOriginalMap16Tile(mtOff2);
			int mtOff3 = offsetMap16Right(mtOff);
			WORD orig3 = getOriginalMap16Tile(mtOff3);
			int mtOff4 = offsetMap16Up(mtOff);
			WORD orig4 = getOriginalMap16Tile(mtOff4);
			int mtOff5 = offsetMap16Down(mtOff);
			WORD orig5 = getOriginalMap16Tile(mtOff5);
			WORD tile = 0;
			if(orig2==0x7C00) tile |= 1;
			if(orig3==0x7C00 && (i+1)==width) tile |= 2;
			if(orig4==0x7C00) tile |= 8;
			if(orig5==0x7C00 && (j+1)==height) tile |= 4;
			if(tile) tile += 0x777C;
			addObjectTile(o,tile,mtOff);
			if(i==0) {
				if(orig2>0x777C && orig2<0x778C) {
					if((orig2-0x777C)&2) {
						tile = (orig2-0x777C)^2;
						if(tile) addObjectTile(o,0x777C+tile,mtOff2);
						else addObjectTile(o,0,mtOff2);
					}
				}
			}
			if((i+1)==width) {
				if(orig3>0x777C && orig3<0x778C) {
					if((orig3-0x777C)&1) {
						tile = (orig3-0x777C)^1;
						if(tile) addObjectTile(o,0x777C+tile,mtOff3);
						else addObjectTile(o,0,mtOff3);
					}
				}
			}
			if(j==0) {
				if(orig4>0x777C && orig4<0x778C) {
					if((orig4-0x777C)&4) {
						tile = (orig4-0x777C)^4;
						if(tile) addObjectTile(o,0x777C+tile,mtOff4);
						else addObjectTile(o,0,mtOff4);
					}
				}
			}
			if((j+1)==height) {
				if(orig5>0x777C && orig5<0x778C) {
					if((orig5-0x777C)&8) {
						tile = (orig5-0x777C)^8;
						if(tile) addObjectTile(o,0x777C+tile,mtOff5);
						else addObjectTile(o,0,mtOff5);
					}
				}
			}
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Chomp signpost/vertical pipe for 3D
void drawObject_A9(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int height = o->data[3]+1;
	int bg1Ts = ((levelHeader[0]&7)<<1)|(levelHeader[1]>>7);
	if(bg1Ts==3) {
		for(int j=0; j<height; j++) {
			int mtOff2 = offsetMap16Right(mtOff);
			if(j<4) {
				int offset = j<<1;
				WORD tile = romBuf[0x09DE1A+offset]|(romBuf[0x09DE1B+offset]<<8);
				addObjectTile(o,tile,mtOff);
				addObjectTile(o,tile+1,mtOff2);
			} else if((height-j)<4) {
				int offset = (height-j-1)<<1;
				WORD tile = romBuf[0x09DE45+offset]|(romBuf[0x09DE46+offset]<<8);
				addObjectTile(o,tile,mtOff);
				addObjectTile(o,tile+1,mtOff2);
			} else {
				int offset = (j%3)<<1;
				WORD tile = romBuf[0x09DE2B+offset]|(romBuf[0x09DE2C+offset]<<8);
				addObjectTile(o,tile,mtOff);
				addObjectTile(o,tile+1,mtOff2);
			}
			mtOff = offsetMap16Down(mtOff);
		}
	} else {
		for(int j=0; j<height; j++) {
			WORD orig = getOriginalMap16Tile(mtOff);
			if(orig==tilesetBuffer[0x1C5C>>1] || orig==tilesetBuffer[0x1C5E>>1]) addObjectTile(o,tilesetBuffer[0x1C78>>1],mtOff);
			else if(orig==0) addObjectTile(o,0x0083,mtOff);
			mtOff = offsetMap16Down(mtOff);
		}
	}
}
//Sewer pipe wall left
void drawObject_AA(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int height = o->data[3]+1;
	for(int j=0; j<height; j++) {
		int offset = (j&1);
		int row = 4;
		if(j<2) row = 0;
		else if((height-j)<3) row = 2;
		overlapTile_sewerPipeWall(o,mtOff,offset?0x791F:0x790F,row);
		int mtOff2 = offsetMap16Right(mtOff);
		overlapTile_sewerPipeWall(o,mtOff2,0x7799+offset,row);
		mtOff = offsetMap16Down(mtOff);
	}
}
//Sewer pipe wall right
void drawObject_AB(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int height = o->data[3]+1;
	for(int j=0; j<height; j++) {
		int offset = (j&1);
		int row = 4;
		if(j<2) row = 0;
		else if((height-j)<3) row = 2;
		overlapTile_sewerPipeWall(o,mtOff,0x779F+offset,row);
		int mtOff2 = offsetMap16Right(mtOff);
		overlapTile_sewerPipeWall(o,mtOff2,offset?0x7910:0x7920,row);
		mtOff = offsetMap16Down(mtOff);
	}
}
//Sewer pipe wall top
void drawObject_AC(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int width = o->data[3]+1;
	int bg1Ts = ((levelHeader[0]&7)<<1)|(levelHeader[1]>>7);
	if(bg1Ts==11) {
		for(int i=0; i<width; i++) {
			int offset = (i&1);
			int row = 4;
			if(i<2) row = 0;
			else if((width-i)<3) row = 2;
			overlapTile_sewerPipeWall(o,mtOff,0x7915+offset,row);
			int mtOff2 = offsetMap16Down(mtOff);
			overlapTile_sewerPipeWall(o,mtOff2,0x77A9+offset,row);
			mtOff = offsetMap16Right(mtOff);
		}
	} else {
		for(int i=0; i<width; i++) {
			int offset = noiseTilemap[mtOff]&6;
			int tileRef = romBuf[0x092BF7+offset]|(romBuf[0x092BF8+offset]<<8);
			addObjectTile(o,tilesetBuffer[tileRef>>1],mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
	}
}
//Sewer pipe wall bottom
void drawObject_AD(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int width = o->data[3]+1;
	for(int i=0; i<width; i++) {
		int offset = (i&1);
		int row = 4;
		if(i<2) row = 0;
		else if((width-i)<3) row = 2;
		overlapTile_sewerPipeWall(o,mtOff,0x77AF+offset,row);
		int mtOff2 = offsetMap16Down(mtOff);
		overlapTile_sewerPipeWall(o,mtOff2,0x7925+offset,row);
		mtOff = offsetMap16Right(mtOff);
	}
}
//Sewer background vertical
void drawObject_AE(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int height = o->data[3]+1;
	for(int j=0; j<height; j++) {
		int offset = (j&1);
		addObjectTile(o,0x779B+offset,mtOff);
		int mtOff2 = offsetMap16Right(mtOff);
		addObjectTile(o,0x779D+offset,mtOff2);
		mtOff = offsetMap16Down(mtOff);
	}
}
//Sewer background horizontal
void drawObject_AF(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int width = o->data[3]+1;
	for(int i=0; i<width; i++) {
		int offset = (i&1);
		addObjectTile(o,0x77AB+offset,mtOff);
		int mtOff2 = offsetMap16Down(mtOff);
		addObjectTile(o,0x77AD+offset,mtOff2);
		mtOff = offsetMap16Right(mtOff);
	}
}
//Sewer background
void drawObject_B0(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	for(int j=0; j<height; j++) {
		for(int i=0; i<width; i++) {
			WORD orig = getOriginalMap16Tile(mtOff);
			if(orig==0) {
				int offset = 12;
				if(i==0) offset -= 4;
				else if((i+1)==width) offset += 2;
				else if(i&1) offset -= 2;
				if(j==0) offset -= 8;
				else if((j+1)==height) offset += 16;
				else if(j&1) offset += 8;
				WORD tile = romBuf[0x09E190+offset]|(romBuf[0x09E191+offset]<<8);
				addObjectTile(o,tile,mtOff);
			}
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Sewer platform
void drawObject_B1(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int width = o->data[3]+1;
	for(int i=0; i<width; i++) {
		WORD orig = getOriginalMap16Tile(mtOff);
		if(orig<0x77B9) {
			int tileRef = (orig-9)&0xE;
			WORD tile = romBuf[0x09E22E +tileRef]|(romBuf[0x09E22F+tileRef]<<8);
			addObjectTile(o,tile,mtOff);
		} else {
			for(int n=0; n<0x18; n+=2) {
				int tileRef = romBuf[0x09E1FE +n]|(romBuf[0x09E1FF+n]<<8);
				if(orig==tileRef) {
					WORD tile = romBuf[0x09E216+n]|(romBuf[0x09E217+n]<<8);
					addObjectTile(o,tile,mtOff);
				}
			}
		}
		mtOff = offsetMap16Right(mtOff);
	}
}
//Sewer pipe upside-down steep slope right
void drawObject_B2(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int width = 0x101-o->data[3];
	if(width==0x101) width = 1;
	for(int i=0; i<width; i++) {
		WORD orig = getOriginalMap16Tile(mtOff);
		if(orig==0) addObjectTile(o,0x792E,mtOff);
		int mtOff2 = offsetMap16Down(mtOff);
		WORD orig2 = getOriginalMap16Tile(mtOff2);
		if(orig2==0) addObjectTile(o,0x5D09,mtOff2);
		mtOff2 = offsetMap16Down(mtOff2);
		orig2 = getOriginalMap16Tile(mtOff2);
		if(orig2==0) addObjectTile(o,0x77B9,mtOff2);
		mtOff = offsetMap16Left(mtOff);
		mtOff = offsetMap16Down(mtOff);
	}
}
//Sewer pipe steep slope left
void drawObject_B3(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int width = 0x101-o->data[3];
	if(width==0x101) width = 1;
	for(int i=0; i<width; i++) {
		WORD orig = getOriginalMap16Tile(mtOff);
		if(orig==0) addObjectTile(o,0x77BA,mtOff);
		int mtOff2 = offsetMap16Down(mtOff);
		WORD orig2 = getOriginalMap16Tile(mtOff2);
		if(orig2==0) addObjectTile(o,0x082D,mtOff2);
		mtOff2 = offsetMap16Down(mtOff2);
		orig2 = getOriginalMap16Tile(mtOff2);
		if(orig2==0) addObjectTile(o,0x791D,mtOff2);
		mtOff = offsetMap16Left(mtOff);
		mtOff = offsetMap16Down(mtOff);
	}
}
//Sewer pipe upside-down steep slope right (large)
void drawObject_B4(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int width = 0x101-o->data[3];
	if(width==0x101) width = 1;
	for(int i=0; i<width; i++) {
		WORD orig = getOriginalMap16Tile(mtOff);
		if(orig==0) addObjectTile(o,0x792E,mtOff);
		int mtOff2 = offsetMap16Down(mtOff);
		WORD orig2 = getOriginalMap16Tile(mtOff2);
		if(orig2==0) addObjectTile(o,0x5D09,mtOff2);
		mtOff2 = offsetMap16Down(mtOff2);
		orig2 = getOriginalMap16Tile(mtOff2);
		if(orig2==0) addObjectTile(o,0x77B9,mtOff2);
		mtOff2 = offsetMap16Down(mtOff2);
		orig2 = getOriginalMap16Tile(mtOff2);
		if(orig2==0) addObjectTile(o,0x77AB,mtOff2);
		mtOff = offsetMap16Left(mtOff);
		mtOff = offsetMap16Down(mtOff);
	}
}
//Sewer pipe steep slope left (large)
void drawObject_B5(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int width = 0x101-o->data[3];
	if(width==0x101) width = 1;
	for(int i=0; i<width; i++) {
		WORD orig = getOriginalMap16Tile(mtOff);
		if(orig==0) addObjectTile(o,0x77AE,mtOff);
		int mtOff2 = offsetMap16Down(mtOff);
		WORD orig2 = getOriginalMap16Tile(mtOff2);
		if(orig2==0) addObjectTile(o,0x77BA,mtOff2);
		mtOff2 = offsetMap16Down(mtOff2);
		orig2 = getOriginalMap16Tile(mtOff2);
		if(orig2==0) addObjectTile(o,0x082D,mtOff2);
		mtOff2 = offsetMap16Down(mtOff2);
		orig2 = getOriginalMap16Tile(mtOff2);
		if(orig2==0) addObjectTile(o,0x791D,mtOff2);
		mtOff = offsetMap16Left(mtOff);
		mtOff = offsetMap16Down(mtOff);
	}
}
//Sewer pipe upside-down steep slope left
void drawObject_B6(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int width = o->data[3]+1;
	for(int i=0; i<width; i++) {
		WORD orig = getOriginalMap16Tile(mtOff);
		if(orig==0) addObjectTile(o,0x792D,mtOff);
		int mtOff2 = offsetMap16Down(mtOff);
		WORD orig2 = getOriginalMap16Tile(mtOff2);
		if(orig2==0) {
			int mtOff3 = offsetMap16Down(mtOff2);
			WORD orig3 = getOriginalMap16Tile(mtOff3);
			if(orig3==0x779F || orig3==0x77A0) addObjectTile(o,0x5B0D,mtOff2);
			else addObjectTile(o,0x5B0C,mtOff2);
		}
		mtOff2 = offsetMap16Down(mtOff2);
		orig2 = getOriginalMap16Tile(mtOff2);
		if(orig2==0) addObjectTile(o,0x77C9,mtOff2);
		mtOff = offsetMap16Right(mtOff);
		mtOff = offsetMap16Down(mtOff);
	}
}
//Sewer pipe steep slope right
void drawObject_B7(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int width = o->data[3]+1;
	for(int i=0; i<width; i++) {
		WORD orig = getOriginalMap16Tile(mtOff);
		if(orig==0) addObjectTile(o,0x77CA,mtOff);
		int mtOff2 = offsetMap16Down(mtOff);
		WORD orig2 = getOriginalMap16Tile(mtOff2);
		if(orig2==0) {
			int mtOff3 = offsetMap16Up(mtOff2);
			WORD orig3 = getOriginalMap16Tile(mtOff3);
			if(orig3==0x779F || orig3==0x77A0) addObjectTile(o,0x0A2F,mtOff2);
			else addObjectTile(o,0x0A2E,mtOff2);
		}
		mtOff2 = offsetMap16Down(mtOff2);
		orig2 = getOriginalMap16Tile(mtOff2);
		if(orig2==0) addObjectTile(o,0x791E,mtOff2);
		mtOff = offsetMap16Right(mtOff);
		mtOff = offsetMap16Down(mtOff);
	}
}
//Sewer pipe upside-down steep slope left (large)
void drawObject_B8(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int width = o->data[3]+1;
	for(int i=0; i<width; i++) {
		WORD orig = getOriginalMap16Tile(mtOff);
		if(orig==0) addObjectTile(o,0x792D,mtOff);
		int mtOff2 = offsetMap16Down(mtOff);
		WORD orig2 = getOriginalMap16Tile(mtOff2);
		if(orig2==0) {
			int mtOff3 = offsetMap16Down(mtOff2);
			WORD orig3 = getOriginalMap16Tile(mtOff3);
			if(orig3==0x779F || orig3==0x77A0) addObjectTile(o,0x5B0D,mtOff2);
			else addObjectTile(o,0x5B0C,mtOff2);
		}
		mtOff2 = offsetMap16Down(mtOff2);
		orig2 = getOriginalMap16Tile(mtOff2);
		if(orig2==0) addObjectTile(o,0x77C9,mtOff2);
		mtOff2 = offsetMap16Down(mtOff2);
		orig2 = getOriginalMap16Tile(mtOff2);
		if(orig2==0) addObjectTile(o,0x77AC,mtOff2);
		mtOff = offsetMap16Right(mtOff);
		mtOff = offsetMap16Down(mtOff);
	}
}
//Sewer pipe steep slope right (large)
void drawObject_B9(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int width = o->data[3]+1;
	for(int i=0; i<width; i++) {
		WORD orig = getOriginalMap16Tile(mtOff);
		if(orig==0) addObjectTile(o,0x77AD,mtOff);
		int mtOff2 = offsetMap16Down(mtOff);
		WORD orig2 = getOriginalMap16Tile(mtOff2);
		if(orig2==0) addObjectTile(o,0x77CA,mtOff2);
		mtOff2 = offsetMap16Down(mtOff2);
		orig2 = getOriginalMap16Tile(mtOff2);
		if(orig2==0) {
			int mtOff3 = offsetMap16Up(mtOff2);
			WORD orig3 = getOriginalMap16Tile(mtOff3);
			if(orig3==0x779F || orig3==0x77A0) addObjectTile(o,0x0A2F,mtOff2);
			else addObjectTile(o,0x0A2E,mtOff2);
		}
		mtOff2 = offsetMap16Down(mtOff2);
		orig2 = getOriginalMap16Tile(mtOff2);
		if(orig2==0) addObjectTile(o,0x791E,mtOff2);
		mtOff = offsetMap16Right(mtOff);
		mtOff = offsetMap16Down(mtOff);
	}
}
//Sewer pipe end top
void drawObject_BA(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int width = o->data[3]+1;
	for(int i=0; i<width; i++) {
		if(i==0) addObjectTile(o,0x792F,mtOff);
		else if((i+1)==width) addObjectTile(o,0x7930,mtOff);
		else addObjectTile(o,(i&1)?0x7915:0x7916,mtOff);
		mtOff = offsetMap16Right(mtOff);
	}
}
//Sewer pipe end bottom
void drawObject_BB(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int width = o->data[3]+1;
	for(int i=0; i<width; i++) {
		if(i==0) addObjectTile(o,0x7932,mtOff);
		else if((i+1)==width) addObjectTile(o,0x7933,mtOff);
		else addObjectTile(o,(i&1)?0x7925:0x7926,mtOff);
		mtOff = offsetMap16Right(mtOff);
	}
}
//Sewer pipe end right
void drawObject_BC(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int height = o->data[3]+1;
	for(int j=0; j<height; j++) {
		if(j==0) addObjectTile(o,0x7930,mtOff);
		else if((j+1)==height) addObjectTile(o,0x7933,mtOff);
		else addObjectTile(o,(j&1)?0x7910:0x7920,mtOff);
		mtOff = offsetMap16Down(mtOff);
	}
}
//Sewer pipe end left
void drawObject_BD(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int height = o->data[3]+1;
	for(int j=0; j<height; j++) {
		if(j==0) addObjectTile(o,0x792F,mtOff);
		else if((j+1)==height) addObjectTile(o,0x7932,mtOff);
		else addObjectTile(o,(j&1)?0x790F:0x791F,mtOff);
		mtOff = offsetMap16Down(mtOff);
	}
}
//Vertical sewage from pipe
void drawObject_BE(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int height = o->data[3]+1;
	for(int j=0; j<height; j++) {
		WORD orig = getOriginalMap16Tile(mtOff);
		if(j==0) {
			for(int n=0; n<0x58; n+=2) {
				int tileRef = romBuf[0x09E46F+n]|(romBuf[0x09E470+n]<<8);
				if(orig==tileRef) {
					WORD tile = romBuf[0x09E4C7+n]|(romBuf[0x09E4C8+n]<<8);
					addObjectTile(o,tile,mtOff);
				}
			}
		} else if((j+2)==height) {
			WORD tile = 0x8103;
			for(int n=0; n<0x1C; n+=2) {
				int tileRef = romBuf[0x09E5F5+n]|(romBuf[0x09E5F6+n]<<8);
				if(orig==tileRef) tile = romBuf[0x09E611+n]|(romBuf[0x09E612+n]<<8);
			}
			addObjectTile(o,tile,mtOff);
		} else if((j+1)==height) {
			for(int n=0; n<0x30; n++) {
				int tileRef = romBuf[0x09E51F+n]|(romBuf[0x09E520+n]<<8);
				if(orig==tileRef) {
					WORD tile = romBuf[0x09E54F+n]|(romBuf[0x09E550+n]<<8);
					addObjectTile(o,tile,mtOff);
				}
			}
		} else {
			WORD tile = 0x8101;
			for(int n=0; n<0xC; n++) {
				int tileRef = romBuf[0x09E577+n]|(romBuf[0x09E578+n]<<8);
				if(orig==tileRef) tile = 0x1517;
			}
			addObjectTile(o,tile,mtOff);
		}
		mtOff = offsetMap16Down(mtOff);
	}
}
//Vertical sewage going downward
void drawObject_BF(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int height = o->data[3]+1;
	for(int j=0; j<height; j++) {
		WORD orig = getOriginalMap16Tile(mtOff);
		if(j==0) {
			if(orig==0x77BA) addObjectTile(o,0x77BF,mtOff);
			else if((orig&0xFF00)!=0x8500) addObjectTile(o,0x77C0,mtOff);
		} else if(j==1) {
			if(orig==0x779F || orig==0x77A0) addObjectTile(o,0x8100,mtOff);
			else if(orig==0x1513 || orig==0x1516) addObjectTile(o,0x1517,mtOff);
			else addObjectTile(o,0x8102,mtOff);
		} else if((j+1)==height) {
			WORD tile = 0x8101;
			for(int n=0; n<0x1C; n+=2) {
				int tileRef = romBuf[0x09E5F5+n]|(romBuf[0x09E5F6+n]<<8);
				if(orig==tileRef) tile = romBuf[0x09E611+n]|(romBuf[0x09E612+n]<<8);
			}
			if(tile==0x8101) {
				for(int n=0; n<0x30; n++) {
					int tileRef = romBuf[0x09E51F+n]|(romBuf[0x09E520+n]<<8);
					if(orig==tileRef) tile = romBuf[0x09E54F+n]|(romBuf[0x09E550+n]<<8);
				}
			}
			addObjectTile(o,tile,mtOff);
		} else if((j+2)==height) {
			WORD tile = 0x8101;
			int mtOff2 = offsetMap16Down(mtOff);
			WORD orig2 = getOriginalMap16Tile(mtOff2);
			for(int n=0; n<0x30; n++) {
				int tileRef = romBuf[0x09E51F+n]|(romBuf[0x09E520+n]<<8);
				if(orig2==tileRef) tile = romBuf[0x09E54F+n]|(romBuf[0x09E550+n]<<8);
			}
			addObjectTile(o,tile,mtOff);
		} else {
			WORD tile = 0x8101;
			for(int n=0; n<0xC; n++) {
				int tileRef = romBuf[0x09E577+n]|(romBuf[0x09E578+n]<<8);
				if(orig==tileRef) tile = 0x1517;
			}
			addObjectTile(o,tile,mtOff);
		}
		mtOff = offsetMap16Down(mtOff);
	}
}
//Horizontal sewage going leftward
void drawObject_C0(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int width = o->data[3]+1;
	for(int i=0; i<width; i++) {
		WORD orig = getOriginalMap16Tile(mtOff);
		if(orig==0x854A) addObjectTile(o,0x8550,mtOff);
		else {
			if(i==0) {
				int mtOff2 = offsetMap16Left(mtOff);
				WORD orig2 = getOriginalMap16Tile(mtOff2);
				if(orig2==0x8101) {
					if(orig>=0x854B && orig<0x854F) addObjectTile(o,orig+0xB,mtOff);
					else addObjectTile(o,0x77EB,mtOff);
				} else {
					for(int n=0; n<0x1A; n+=2) {
						int tileRef = romBuf[0x09E6E8+n]|(romBuf[0x09E6E9+n]<<8);
						if(orig==tileRef) {
							WORD tile = romBuf[0x09E702+n]|(romBuf[0x09E703+n]<<8);
							addObjectTile(o,tile,mtOff);
						}
					}
				}
			} else {
				int mtOff2 = offsetMap16Right(mtOff);
				WORD orig2 = getOriginalMap16Tile(mtOff2);
				if(orig2==0x8101) {
					if(orig>=0x854B && orig<0x854F) addObjectTile(o,orig+0x1F,mtOff);
					else addObjectTile(o,0x77D0,mtOff);
				} else {
					for(int n=0; n<0x1A; n+=2) {
						int tileRef = romBuf[0x09E6E8+n]|(romBuf[0x09E6E9+n]<<8);
						if(orig==tileRef) {
							WORD tile = romBuf[0x09E702+n]|(romBuf[0x09E703+n]<<8);
							addObjectTile(o,tile,mtOff);
						}
					}
				}
			}
		}
		int mtOff2 = offsetMap16Down(mtOff);
		WORD orig2 = getOriginalMap16Tile(mtOff2);
		for(int n=0; n<0x30; n++) {
			int tileRef = romBuf[0x09E51F+n]|(romBuf[0x09E520+n]<<8);
			if(orig2==tileRef) {
				WORD tile = romBuf[0x09E71C+n]|(romBuf[0x09E71D+n]<<8);
				addObjectTile(o,tile,mtOff2);
			}
		}
		mtOff = offsetMap16Right(mtOff);
	}
}
//Horizontal sewage going rightward
void drawObject_C1(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int width = o->data[3]+1;
	for(int i=0; i<width; i++) {
		WORD orig = getOriginalMap16Tile(mtOff);
		if(orig==0x8546) addObjectTile(o,0x8551,mtOff);
		else {
			if(i==0) {
				int mtOff2 = offsetMap16Left(mtOff);
				WORD orig2 = getOriginalMap16Tile(mtOff2);
				if(orig2==0x8101) {
					if(orig>=0x854B && orig<0x854F) addObjectTile(o,orig+0x13,mtOff);
					else addObjectTile(o,0x77EB,mtOff);
				} else {
					for(int n=0; n<0x1A; n+=2) {
						int tileRef = romBuf[0x09E6E8+n]|(romBuf[0x09E6E9+n]<<8);
						if(orig==tileRef) {
							WORD tile = romBuf[0x09E7D3+n]|(romBuf[0x09E7D4+n]<<8);
							addObjectTile(o,tile,mtOff);
						}
					}
				}
			} else {
				int mtOff2 = offsetMap16Right(mtOff);
				WORD orig2 = getOriginalMap16Tile(mtOff2);
				if(orig2==0x8101) {
					if(orig>=0x854B && orig<0x854F) addObjectTile(o,orig+7,mtOff);
					else addObjectTile(o,0x77D0,mtOff);
				} else {
					for(int n=0; n<0x1A; n+=2) {
						int tileRef = romBuf[0x09E6E8+n]|(romBuf[0x09E6E9+n]<<8);
						if(orig==tileRef) {
							WORD tile = romBuf[0x09E7D3+n]|(romBuf[0x09E7D4+n]<<8);
							addObjectTile(o,tile,mtOff);
						}
					}
				}
			}
		}
		int mtOff2 = offsetMap16Down(mtOff);
		WORD orig2 = getOriginalMap16Tile(mtOff2);
		for(int n=0; n<0x30; n++) {
			int tileRef = romBuf[0x09E51F+n]|(romBuf[0x09E520+n]<<8);
			if(orig2==tileRef) {
				WORD tile = romBuf[0x09E7ED+n]|(romBuf[0x09E7EE +n]<<8);
				addObjectTile(o,tile,mtOff2);
			}
		}
		mtOff = offsetMap16Right(mtOff);
	}
}
//Diagonal sewage going leftward
void drawObject_C2(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int width = 0x101-o->data[3];
	if(width==0x101) width = 1;
	for(int i=0; i<width; i++) {
		addObjectTile(o,0x77BF,mtOff);
		int mtOff2 = offsetMap16Down(mtOff);
		WORD orig2 = getOriginalMap16Tile(mtOff2);
		if(orig2==0x082D || orig2==0x082E) addObjectTile(o,0x7F00,mtOff2);
		mtOff = offsetMap16Left(mtOff);
		mtOff = offsetMap16Down(mtOff);
	}
}
//Diagonal sewage going rightward
void drawObject_C3(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int width = o->data[3]+1;
	for(int i=0; i<width; i++) {
		addObjectTile(o,0x77C0,mtOff);
		int mtOff2 = offsetMap16Down(mtOff);
		WORD orig2 = getOriginalMap16Tile(mtOff2);
		if(orig2>=0x0A2D && orig2<=0x0A30) addObjectTile(o,0x8000,mtOff2);
		mtOff = offsetMap16Right(mtOff);
		mtOff = offsetMap16Down(mtOff);
	}
}
//Horizontal coin
void drawObject_C4(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int width = o->data[3]+1;
	for(int i=0; i<width; i++) {
		if((i&1)==0) addObjectTile(o,0x6000,mtOff);
		mtOff = offsetMap16Right(mtOff);
	}
}
//Vertical coin
void drawObject_C5(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int height = o->data[3]+1;
	for(int j=0; j<height; j++) {
		if((j&1)==0) addObjectTile(o,0x6000,mtOff);
		mtOff = offsetMap16Down(mtOff);
	}
}
//Diagonal coin
void drawObject_C6(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int width = o->data[3];
	if(width&0x80) {
		width = 0x101-width;
		for(int i=0; i<width; i++) {
			if((i&1)==0) addObjectTile(o,0x6000,mtOff);
			mtOff = offsetMap16Left(mtOff);
			mtOff = offsetMap16Down(mtOff);
		}
	} else {
		width++;
		for(int i=0; i<width; i++) {
			if((i&1)==0) addObjectTile(o,0x6000,mtOff);
			mtOff = offsetMap16Right(mtOff);
			mtOff = offsetMap16Down(mtOff);
		}
	}
}
//Horizontal switch coin
void drawObject_C7(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int width = o->data[3]+1;
	for(int i=0; i<width; i++) {
		if((i&1)==0) addObjectTile(o,0x7400,mtOff);
		mtOff = offsetMap16Right(mtOff);
	}
}
//Vertical switch coin
void drawObject_C8(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int height = o->data[3]+1;
	for(int j=0; j<height; j++) {
		if((j&1)==0) addObjectTile(o,0x7400,mtOff);
		mtOff = offsetMap16Down(mtOff);
	}
}
//Diagonal switch coin
void drawObject_C9(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int width = o->data[3];
	if(width&0x80) {
		width = 0x101-width;
		for(int i=0; i<width; i++) {
			if((i&1)==0) addObjectTile(o,0x7400,mtOff);
			mtOff = offsetMap16Left(mtOff);
			mtOff = offsetMap16Down(mtOff);
		}
	} else {
		width++;
		for(int i=0; i<width; i++) {
			if((i&1)==0) addObjectTile(o,0x7400,mtOff);
			mtOff = offsetMap16Right(mtOff);
			mtOff = offsetMap16Down(mtOff);
		}
	}
}
//Sewer water
void drawObject_CA(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	for(int i=0; i<width; i++) {
		for(int j=0; j<height; j++) {
			if(j==0) {
				WORD orig = getOriginalMap16Tile(mtOff);
				if(orig==0x8101) addObjectTile(o,0x8103,mtOff);
				else {
					int mtOff2 = offsetMap16Left(mtOff);
					WORD orig2 = getOriginalMap16Tile(mtOff2);
					if(orig2==0x8103) {
						for(int n=0; n<0x58; n++) {
							int tileRef = romBuf[0x09E46F+n]|(romBuf[0x09E470+n]<<8);
							if(orig==tileRef) {
								WORD tile = romBuf[0x09E99A+n]|(romBuf[0x09E99B+n]<<8);
								if(tile) addObjectTile(o,tile,mtOff);
							}
						}
					} else {
						mtOff2 = offsetMap16Right(mtOff);
						orig2 = getOriginalMap16Tile(mtOff2);
						if(orig2==0x8101 || orig2==0x8103) {
							for(int n=0; n<0x58; n++) {
								int tileRef = romBuf[0x09E46F+n]|(romBuf[0x09E470+n]<<8);
								if(orig==tileRef) {
									WORD tile = romBuf[0x09E942+n]|(romBuf[0x09E943+n]<<8);
									if(tile) addObjectTile(o,tile,mtOff);
								}
							}
						}
					}
				}
			} else if(j==1) addObjectTile(o,0x161F,mtOff);
			else addObjectTile(o,0x1620,mtOff);
			mtOff = offsetMap16Down(mtOff);
		}
		mtOff = preserve = offsetMap16Right(preserve);
	}
}
//Castle wall with sides
void drawObject_CB(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	bool shadowFlag = false;
	for(int i=0; i<width; i++) {
		for(int j=0; j<height; j++) {
			WORD orig = getOriginalMap16Tile(mtOff);
			WORD tile = 0x00C2;
			if(i==0) tile = 0x00D6;
			else if((i+1)==width) tile = 0x00D7;
			if(j==0) {
				if(tile==0x00C2) {
					int mtOff2 = offsetMap16Up(mtOff);
					WORD orig2 = getOriginalMap16Tile(mtOff2);
					if(orig2>=0x0151 && orig2<0x0161) {
						if(shadowFlag) tile = 0x00C6;
						else if(tile!=0x00D5) {
							mtOff2 = offsetMap16Left(mtOff);
							orig2 = getOriginalMap16Tile(mtOff2);
							if(orig2==0x00C6) tile = 0x00C6;
							else tile = 0x00C3;
						}
						shadowFlag = true;
					} else if(shadowFlag) {
						tile = 0x00C7;
						shadowFlag = false;
					}
				} else {
					int mtOff2 = offsetMap16Up(mtOff);
					WORD orig2 = getOriginalMap16Tile(mtOff2);
					if(orig2>=0x0153 && orig2<0x0161) {
						if(tile==0x00D6) tile = 0x77D8;
						else if(tile==0x00D7) tile = 0x77D9;
						shadowFlag = true;
					} else {
						mtOff2 = offsetMap16Up(mtOff);
						orig2 = getOriginalMap16Tile(mtOff2);
						if(orig2>=0x0151 && orig2<0x0161) {
							if(shadowFlag) tile = 0x00C6;
							else if(tile!=0x00D5) {
								mtOff2 = offsetMap16Left(mtOff);
								orig2 = getOriginalMap16Tile(mtOff2);
								if(orig2==0x00C6) tile = 0x00C6;
								else tile = 0x00C3;
							}
							shadowFlag = true;
						} else if(shadowFlag) {
							tile = 0x00C7;
							shadowFlag = false;
						}
					}
				}
			}
			addObjectTile(o,tile,mtOff);
			mtOff = offsetMap16Down(mtOff);
		}
		mtOff = preserve = offsetMap16Right(preserve);
	}
}
//Upside-down castle wall, left
void drawObject_CC(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = 0x101-o->data[3];
	int height = 0x101-o->data[4];
	if(width==0x101) width = 1;
	if(height==0x101) height = 1;
	bool shadowFlag = false;
	for(int i=0; i<width; i++) {
		for(int j=0; j<height; j++) {
			WORD orig = getOriginalMap16Tile(mtOff);
			WORD tile = 0x00C2;
			if(j==0) tile = 0x00C9;
			else if(j==1) tile = 0x00CA;
			if(j<2) {
				int mtOff2 = offsetMap16Up(mtOff);
				WORD orig2 = getOriginalMap16Tile(mtOff2);
				if(orig2>=0x0153 && orig2<0x0161) {
					if(j==0) tile = 0x77DA;
					else tile = 0x77DB;
				} else {
					mtOff2 = offsetMap16Left(mtOff);
					orig2 = getOriginalMap16Tile(mtOff2);
					if(orig2>=0x0153 && orig2<0x0161) {
						if(j==0) tile = 0x77E5;
						else tile = 0x77E3;
					}
				}
			} else if((i+1)==width) {
				int mtOff2 = offsetMap16Left(mtOff);
				WORD orig2 = getOriginalMap16Tile(mtOff2);
				if(orig2<0x0153 || orig2>=0x0161) {
					int mtOff3 = offsetMap16Up(mtOff2);
					WORD orig3 = getOriginalMap16Tile(mtOff3);
					if(orig3>=0x0153 && orig3<0x0161) tile = 0x00C7;
				}
				if(orig2>=0x0151 && orig2<0x0161) {
					mtOff2 = offsetMap16Up(mtOff);
					orig2 = getOriginalMap16Tile(mtOff2);
					if(orig2>=0x0151 && orig2<0x0161) tile = 0x00D5;
					else if(orig2==0x00C2 || orig2==0x77E6 || orig2==0x77E7) tile = 0x00C4;
					else tile = 0x00C5;
				} else {
					mtOff2 = offsetMap16Up(mtOff);
					orig2 = getOriginalMap16Tile(mtOff2);
					if(orig2==0x00C5) tile = 0x00C7;
				}
			}
			if((j+1)==height && j>1) {
				if(orig==0x00DE) shadowFlag = false;
				else {
					int mtOff2 = offsetMap16Left(mtOff);
					mtOff2 = offsetMap16Up(mtOff2);
					WORD orig2 = getOriginalMap16Tile(mtOff2);
					if(orig2>=0x0153 && orig2<0x0161) shadowFlag = true;
					else shadowFlag = false;
				}
				int mtOff2 = offsetMap16Up(mtOff);
				WORD orig2 = getOriginalMap16Tile(mtOff2);
				if(orig2>=0x0151 && orig2<0x0161) {
					if(shadowFlag) tile = 0x00C6;
					else if(tile!=0x00D5) {
						mtOff2 = offsetMap16Left(mtOff);
						orig2 = getOriginalMap16Tile(mtOff2);
						if(orig2==0x00C6) tile = 0x00C6;
						else tile = 0x00C3;
					}
					shadowFlag = true;
				} else if(shadowFlag) {
					tile = 0x00C7;
					shadowFlag = false;
				}
			}
			if(tile!=0x00C9 || orig==0) addObjectTile(o,tile,mtOff);
			mtOff = offsetMap16Up(mtOff);
		}
		mtOff = preserve = offsetMap16Left(preserve);
		mtOff = preserve = offsetMap16Up(preserve);
		height--;
	}
}
//Upside-down castle wall, right
void drawObject_CD(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = 0x101-o->data[4];
	if(height==0x101) height = 1;
	bool shadowFlag = false;
	for(int i=0; i<width; i++) {
		for(int j=0; j<height; j++) {
			WORD orig = getOriginalMap16Tile(mtOff);
			WORD tile = 0x00C2;
			if(j==0) tile = 0x00CC;
			else if(j==1) tile = 0x00CB;
			if(j<2) {
				int mtOff2 = offsetMap16Up(mtOff);
				WORD orig2 = getOriginalMap16Tile(mtOff2);
				if(orig2>=0x0153 && orig2<0x0161) {
					if(j==0) tile = 0x77DD;
					else tile = 0x77DC;
				} else {
					mtOff2 = offsetMap16Left(mtOff);
					orig2 = getOriginalMap16Tile(mtOff2);
					if(orig2>=0x0153 && orig2<0x0161) {
						if(j==0) tile = 0x77E4;
						else tile = 0x77E2;
					}
				}
			} else if(i==0) {
				int mtOff2 = offsetMap16Left(mtOff);
				WORD orig2 = getOriginalMap16Tile(mtOff2);
				if(orig2<0x0153 || orig2>=0x0161) {
					int mtOff3 = offsetMap16Up(mtOff2);
					WORD orig3 = getOriginalMap16Tile(mtOff3);
					if(orig3>=0x0153 && orig3<0x0161) tile = 0x00C7;
				}
				if(orig2>=0x0151 && orig2<0x0161) {
					mtOff2 = offsetMap16Up(mtOff);
					orig2 = getOriginalMap16Tile(mtOff2);
					if(orig2>=0x0151 && orig2<0x0161) tile = 0x00D5;
					else if(orig2==0x00C2 || orig2==0x77E6 || orig2==0x77E7) tile = 0x00C4;
					else tile = 0x00C5;
				} else {
					mtOff2 = offsetMap16Up(mtOff);
					orig2 = getOriginalMap16Tile(mtOff2);
					if(orig2==0x00C5) tile = 0x00C7;
				}
			}
			if((j+1)==height && j>1) {
				if(orig==0x00DE) shadowFlag = false;
				else {
					int mtOff2 = offsetMap16Left(mtOff);
					mtOff2 = offsetMap16Up(mtOff2);
					WORD orig2 = getOriginalMap16Tile(mtOff2);
					if(orig2>=0x0153 && orig2<0x0161) shadowFlag = true;
				}
				int mtOff2 = offsetMap16Up(mtOff);
				WORD orig2 = getOriginalMap16Tile(mtOff2);
				if(orig2>=0x0151 && orig2<0x0161) {
					if(shadowFlag) tile = 0x00C6;
					else if(tile!=0x00D5) {
						mtOff2 = offsetMap16Left(mtOff);
						orig2 = getOriginalMap16Tile(mtOff2);
						if(orig2==0x00C6) tile = 0x00C6;
						else tile = 0x00C3;
					}
					shadowFlag = true;
				} else if(shadowFlag) {
					tile = 0x00C7;
					shadowFlag = false;
				}
			}
			if(tile!=0x00CC || orig==0) addObjectTile(o,tile,mtOff);
			mtOff = offsetMap16Up(mtOff);
		}
		mtOff = preserve = offsetMap16Right(preserve);
		mtOff = preserve = offsetMap16Up(preserve);
		height--;
	}
}
//Line guide, steep slope
void drawObject_CE(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int width = o->data[3];
	if(width&0x80) {
		width = 0x101-width;
		for(int i=0; i<width; i++) {
			addObjectTile(o,0x8700,mtOff);
			mtOff = offsetMap16Left(mtOff);
			mtOff = offsetMap16Down(mtOff);
		}
	} else {
		width++;
		for(int i=0; i<width; i++) {
			addObjectTile(o,0x8701,mtOff);
			mtOff = offsetMap16Right(mtOff);
			mtOff = offsetMap16Down(mtOff);
		}
	}
}
//Line guide, gentle slope
void drawObject_CF(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int width = o->data[3];
	if(width&0x80) {
		width = 0x101-width;
		for(int i=0; i<width; i++) {
			addObjectTile(o,0x8702+(i&1),mtOff);
			mtOff = offsetMap16Left(mtOff);
			if(i&1) mtOff = offsetMap16Down(mtOff);
		}
	} else {
		width++;
		for(int i=0; i<width; i++) {
			addObjectTile(o,0x8704+(i&1),mtOff);
			mtOff = offsetMap16Right(mtOff);
			if(i&1) mtOff = offsetMap16Down(mtOff);
		}
	}
}
//Line guide, very gentle slope
void drawObject_D0(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int width = o->data[3];
	if(width&0x80) {
		width = 0x101-width;
		for(int i=0; i<width; i++) {
			addObjectTile(o,0x8706+(i&3),mtOff);
			mtOff = offsetMap16Left(mtOff);
			if((i&3)==3) mtOff = offsetMap16Down(mtOff);
		}
	} else {
		width++;
		for(int i=0; i<width; i++) {
			addObjectTile(o,0x870A+(i&3),mtOff);
			mtOff = offsetMap16Right(mtOff);
			if((i&3)==3) mtOff = offsetMap16Down(mtOff);
		}
	}
}
//Line guide, vertical
void drawObject_D1(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int height = o->data[3]+1;
	for(int j=0; j<height; j++) {
		addObjectTile(o,0x870F,mtOff);
		mtOff = offsetMap16Down(mtOff);
	}
}
//Line guide, horizontal
void drawObject_D2(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int width = o->data[3]+1;
	for(int i=0; i<width; i++) {
		addObjectTile(o,0x870E,mtOff);
		mtOff = offsetMap16Right(mtOff);
	}
}
//Sewer rock background
void drawObject_D3(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	for(int j=0; j<height; j++) {
		for(int i=0; i<width; i++) {
			int offset = (mtOff&3)^((mtOff>>7)&2);
			addObjectTile(o,0x854B+offset,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Sewer rock wall left
void drawObject_D4(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int height = o->data[3]+1;
	int prevIdx = 0;
	for(int j=0; j<height; j++) {
		if(j==0) {
			addObjectTile(o,0x7982,mtOff);
			int mtOff2 = offsetMap16Left(mtOff);
			addObjectTile(o,0x7981,mtOff2);
			mtOff2 = offsetMap16Up(mtOff);
			addObjectTile(o,0x7980,mtOff2);
		} else if((j+1)==height) {
			addObjectTile(o,0x7987,mtOff);
			int mtOff2 = offsetMap16Left(mtOff);
			addObjectTile(o,0x7986,mtOff2);
			mtOff2 = offsetMap16Down(mtOff);
			addObjectTile(o,0x7988,mtOff2);
		} else {
			WORD orig = getOriginalMap16Tile(mtOff);
			WORD tile = 0;
			for(int n=0; n<0x18; n+=2) {
				int tileRef = romBuf[0x09ECF7+n]|(romBuf[0x09ECF8+n]<<8);
				if(orig==tileRef) tile = romBuf[0x09ED0F+n]|(romBuf[0x09ED10+n]<<8);
			}
			if(tile) {
				addObjectTile(o,tile,mtOff);
				mtOff = offsetMap16Down(mtOff);
				continue;
			}
			if(prevIdx==0xC) {
				addObjectTile(o,0x793E,mtOff);
				int mtOff2 = offsetMap16Left(mtOff);
				addObjectTile(o,0x793D,mtOff2);
				prevIdx = 0;
			} else if(prevIdx==0xE) {
				addObjectTile(o,0x7944,mtOff);
				prevIdx = 0;
			} else {
				int index = noiseTilemap[mtOff]&0xE;
				tile = romBuf[0x09ECDF+index]|(romBuf[0x09ECE0+index]<<8);
				addObjectTile(o,tile,mtOff);
				if(index&8) {
					int mtOff2 = offsetMap16Left(mtOff);
					tile = romBuf[0x09ECE7+index]|(romBuf[0x09ECE8+index]<<8);
					addObjectTile(o,tile,mtOff2);
				}
				prevIdx = index;
			}
		}
		mtOff = offsetMap16Down(mtOff);
	}
}
//Sewer rock wall right
void drawObject_D5(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int height = o->data[3]+1;
	int prevIdx = 0;
	for(int j=0; j<height; j++) {
		if(j==0) {
			addObjectTile(o,0x7984,mtOff);
			int mtOff2 = offsetMap16Right(mtOff);
			addObjectTile(o,0x7985,mtOff2);
			mtOff2 = offsetMap16Up(mtOff);
			addObjectTile(o,0x7983,mtOff2);
		} else if((j+1)==height) {
			addObjectTile(o,0x7989,mtOff);
			int mtOff2 = offsetMap16Right(mtOff);
			addObjectTile(o,0x798A,mtOff2);
			mtOff2 = offsetMap16Down(mtOff);
			addObjectTile(o,0x798B,mtOff2);
		} else {
			WORD orig = getOriginalMap16Tile(mtOff);
			WORD tile = 0;
			for(int n=0; n<0x18; n+=2) {
				int tileRef = romBuf[0x09ECF7+n]|(romBuf[0x09ECF8+n]<<8);
				if(orig==tileRef) tile = romBuf[0x09EE19+n]|(romBuf[0x09EE1A+n]<<8);
			}
			if(tile) {
				addObjectTile(o,tile,mtOff);
				mtOff = offsetMap16Down(mtOff);
				continue;
			}
			if(prevIdx==0xC) {
				addObjectTile(o,0x794F,mtOff);
				int mtOff2 = offsetMap16Right(mtOff);
				addObjectTile(o,0x7950,mtOff2);
				prevIdx = 0;
			} else if(prevIdx==0xE) {
				addObjectTile(o,0x794A,mtOff);
				prevIdx = 0;
			} else {
				int index = noiseTilemap[mtOff]&0xE;
				tile = romBuf[0x09EE01+index]|(romBuf[0x09EE02+index]<<8);
				addObjectTile(o,tile,mtOff);
				if(index&8) {
					int mtOff2 = offsetMap16Right(mtOff);
					tile = romBuf[0x09EE09+index]|(romBuf[0x09EE0A+index]<<8);
					addObjectTile(o,tile,mtOff2);
				}
				prevIdx = index;
			}
		}
		mtOff = offsetMap16Down(mtOff);
	}
}
//Sewer rock wall top
void drawObject_D6(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int width = o->data[3]+1;
	int prevIdx = 0;
	for(int i=0; i<width; i++) {
		if(i==0) {
			addObjectTile(o,0x7982,mtOff);
			int mtOff2 = offsetMap16Up(mtOff);
			addObjectTile(o,0x7980,mtOff2);
			mtOff2 = offsetMap16Left(mtOff);
			addObjectTile(o,0x7981,mtOff2);
		} else if((i+1)==width) {
			addObjectTile(o,0x7984,mtOff);
			int mtOff2 = offsetMap16Up(mtOff);
			addObjectTile(o,0x7983,mtOff2);
			mtOff2 = offsetMap16Right(mtOff);
			addObjectTile(o,0x7985,mtOff2);
		} else {
			WORD orig = getOriginalMap16Tile(mtOff);
			WORD tile = 0;
			for(int n=0; n<0x1A; n+=2) {
				int tileRef = romBuf[0x09EF23+n]|(romBuf[0x09EF24+n]<<8);
				if(orig==tileRef) tile = romBuf[0x09EF3D+n]|(romBuf[0x09EF3E +n]<<8);
			}
			if(tile) {
				addObjectTile(o,tile,mtOff);
				mtOff = offsetMap16Right(mtOff);
				continue;
			}
			if(prevIdx==0xC) {
				addObjectTile(o,0x7957,mtOff);
				int mtOff2 = offsetMap16Up(mtOff);
				addObjectTile(o,0x7955,mtOff2);
				prevIdx = 0;
			} else if(prevIdx==0xE) {
				addObjectTile(o,0x795E,mtOff);
				int mtOff2 = offsetMap16Up(mtOff);
				addObjectTile(o,0x795C,mtOff2);
				prevIdx = 0;
			} else {
				int index = noiseTilemap[mtOff]&0xE;
				tile = romBuf[0x09EF0B+index]|(romBuf[0x09EF0C+index]<<8);
				addObjectTile(o,tile,mtOff);
				if(index&8) {
					int mtOff2 = offsetMap16Up(mtOff);
					tile = romBuf[0x09EF13+index]|(romBuf[0x09EF14+index]<<8);
					addObjectTile(o,tile,mtOff2);
				}
				prevIdx = index;
			}
		}
		mtOff = offsetMap16Right(mtOff);
	}
}
//Sewer rock wall bottom
void drawObject_D7(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int width = o->data[3]+1;
	int prevIdx = 0;
	for(int i=0; i<width; i++) {
		if(i==0) {
			addObjectTile(o,0x7987,mtOff);
			int mtOff2 = offsetMap16Down(mtOff);
			addObjectTile(o,0x7988,mtOff2);
			mtOff2 = offsetMap16Left(mtOff);
			addObjectTile(o,0x7986,mtOff2);
		} else if((i+1)==width) {
			addObjectTile(o,0x7989,mtOff);
			int mtOff2 = offsetMap16Down(mtOff);
			addObjectTile(o,0x798B,mtOff2);
			mtOff2 = offsetMap16Right(mtOff);
			addObjectTile(o,0x798A,mtOff2);
		} else {
			WORD orig = getOriginalMap16Tile(mtOff);
			WORD tile = 0;
			for(int n=0; n<0x1A; n+=2) {
				int tileRef = romBuf[0x09EF23+n]|(romBuf[0x09EF24+n]<<8);
				if(orig==tileRef) tile = romBuf[0x09F052+n]|(romBuf[0x09F053+n]<<8);
			}
			if(tile) {
				addObjectTile(o,tile,mtOff);
				mtOff = offsetMap16Right(mtOff);
				continue;
			}
			if(prevIdx==0xC) {
				addObjectTile(o,0x7963,mtOff);
				int mtOff2 = offsetMap16Down(mtOff);
				addObjectTile(o,0x7965,mtOff2);
				prevIdx = 0;
			} else if(prevIdx==0xE) {
				addObjectTile(o,0x796A,mtOff);
				int mtOff2 = offsetMap16Down(mtOff);
				addObjectTile(o,0x796C,mtOff2);
				prevIdx = 0;
			} else {
				int index = noiseTilemap[mtOff]&0xE;
				tile = romBuf[0x09F03A+index]|(romBuf[0x09F03B+index]<<8);
				addObjectTile(o,tile,mtOff);
				if(index&8) {
					int mtOff2 = offsetMap16Down(mtOff);
					tile = romBuf[0x09F042+index]|(romBuf[0x09F043+index]<<8);
					addObjectTile(o,tile,mtOff2);
				}
				prevIdx = index;
			}
		}
		mtOff = offsetMap16Right(mtOff);
	}
}
//Giant log bridge
void drawObject_D8(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int width = o->data[3]+1;
	for(int i=0; i<width; i++) {
		int offset = (i&1);
		addObjectTile(o,0x84BA+offset,mtOff);
		int mtOff2 = offsetMap16Down(mtOff);
		addObjectTile(o,offset?0x3510:0x330C,mtOff2);
		mtOff2 = offsetMap16Down(mtOff2);
		addObjectTile(o,0x84BC+offset,mtOff2);
		mtOff2 = offsetMap16Down(mtOff2);
		addObjectTile(o,0x84BE +offset,mtOff2);
		mtOff = offsetMap16Right(mtOff);
	}
}
//Log bridge
void drawObject_D9(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int width = o->data[3]+1;
	for(int i=0; i<width; i++) {
		addObjectTile(o,0x84C0,mtOff);
		int mtOff2 = offsetMap16Down(mtOff);
		addObjectTile(o,0x8600,mtOff2);
		mtOff2 = offsetMap16Down(mtOff2);
		addObjectTile(o,0x84C1,mtOff2);
		mtOff = offsetMap16Right(mtOff);
	}
}
//Star Mario blocks
void drawObject_DA(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	for(int j=0; j<height; j++) {
		for(int i=0; i<width; i++) {
			addObjectTile(o,0x8A00,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Icy floor
void drawObject_DB(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	for(int j=0; j<height; j++) {
		for(int i=0; i<width; i++) {
			if(j==0) {
				WORD orig = getOriginalMap16Tile(mtOff);
				if(orig==0) {
					int offset = noiseTilemap[mtOff]&6;
					WORD tile = romBuf[0x09F194+offset]|(romBuf[0x09F195+offset]<<8);
					addObjectTile(o,tile,mtOff);
				}
			} else if(j==1) addObjectTile(o,(i&1)+0x8C01,mtOff);
			else if(j==2) addObjectTile(o,(i&1)+0x8C05,mtOff);
			else if(j==3) addObjectTile(o,(i&1)+0x8C09,mtOff);
			else addObjectTile(o,0x8C0D,mtOff);
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Icy floor hole with water
void drawObject_DC(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	for(int j=0; j<height; j++) {
		for(int i=0; i<width; i++) {
			if(i==0) {
				if(j==0) addObjectTile(o,0x8C03,mtOff);
				else if(j==1) addObjectTile(o,0x8C07,mtOff);
				else if(j==2) addObjectTile(o,0x8C0B,mtOff);
				else addObjectTile(o,0x8C0E,mtOff);
			} else if((i+1)==width) {
				if(j==0) addObjectTile(o,0x8C00,mtOff);
				else if(j==1) addObjectTile(o,0x8C04,mtOff);
				else if(j==2) addObjectTile(o,0x8C08,mtOff);
				else addObjectTile(o,0x8C0C,mtOff);
			} else {
				if(j==0) {
					addObjectTile(o,0,mtOff);
					int mtOff2 = offsetMap16Up(mtOff);
					addObjectTile(o,0,mtOff2);
				} else if(j==1) addObjectTile(o,(i&1)+0x0015,mtOff);
				else if(j==2) addObjectTile(o,(i&1)+0x1621,mtOff);
				else if(j==3) addObjectTile(o,(i&1)+0x1623,mtOff);
				else addObjectTile(o,0x1625,mtOff);
			}
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Icy cave floor
void drawObject_DD(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	int seed = noiseTilemap[mtOff]&7;
	for(int j=0; j<height; j++) {
		for(int i=0; i<width; i++) {
			if(j==0) addObjectTile(o,(i&1)+0x8D8C,mtOff);
			else {
				if(j<6) {
					int offset = (((seed+i)&7)<<1)+((j-1)<<4);
					WORD tile = romBuf[0x09F26E +offset]|(romBuf[0x09F26F+offset]<<8);
					addObjectTile(o,tile,mtOff);
				} else addObjectTile(o,0x7997,mtOff);
				if(i==0) {
					int mtOff2 = offsetMap16Left(mtOff);
					if(j==1) addObjectTile(o,0x7998,mtOff2);
					else addObjectTile(o,0x7999,mtOff2);
				} else if((i+1)==width) {
					int mtOff2 = offsetMap16Right(mtOff);
					if(j==1) addObjectTile(o,0x799A,mtOff2);
					else addObjectTile(o,0x799B,mtOff2);
				}
			}
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Icy cave floor rock
void drawObject_DE(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int height = o->data[3]+1;
	for(int j=0; j<height; j++) {
		int mtOff2 = offsetMap16Right(mtOff);
		if(j==0) {
			addObjectTile(o,0x79A4,mtOff);
			addObjectTile(o,0x79A5,mtOff2);
		} else if(j==1) {
			addObjectTile(o,0x79A6,mtOff);
			addObjectTile(o,0x79A7,mtOff2);
		} else {
			addObjectTile(o,0x799B,mtOff);
			addObjectTile(o,0x7999,mtOff2);
		}
		mtOff = offsetMap16Down(mtOff);
	}
}
//Cave lava
void drawObject_DF(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int width = o->data[3]+1;
	for(int i=0; i<width; i++) {
		WORD orig = getOriginalMap16Tile(mtOff);
		if(orig>=0x8D2E && orig<0x8D32) addObjectTile(o,orig+4,mtOff);
		else {
			if(i==0) addObjectTile(o,0x8D92,mtOff);
			else if((i+1)==width) addObjectTile(o,0x8D93,mtOff);
			else addObjectTile(o,(i&1)+0x8D90,mtOff);
		}
		int mtOff2 = offsetMap16Down(mtOff);
		WORD orig2 = getOriginalMap16Tile(mtOff2);
		if(orig2>=0x8D2E && orig2<0x8D32) addObjectTile(o,orig2+4,mtOff2);
		else {
			if(i==0) addObjectTile(o,0xA602,mtOff2);
			else if((i+1)==width) addObjectTile(o,0xA603,mtOff2);
			else addObjectTile(o,(i&1)+0xA600,mtOff2);
		}
		mtOff = offsetMap16Right(mtOff);
	}
}
//Cave lavafall
void drawObject_E0(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int height = o->data[3]+1;
	for(int j=0; j<height; j++) {
		if(j==0) addObjectTile(o,0xA605,mtOff);
		else addObjectTile(o,0xA606,mtOff);
		mtOff = offsetMap16Down(mtOff);
	}
}
//Cave mushroom
void drawObject_E1(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	int seed = noiseTilemap[mtOff]&0x18;
	if(seed==0x18) seed = 0;
	for(int j=0; j<height; j++) {
		for(int i=0; i<width; i++) {
			if(j==0) {
				int offset = ((i%3)<<1)+seed;
				WORD tile = romBuf[0x09F3C8+offset]|(romBuf[0x09F3C9+offset]<<8);
				addObjectTile(o,tile,mtOff);
			} else if((i%3)==1) {
				if((j+1)==height) {
					int offset = (j-2)&3;
					WORD orig = getOriginalMap16Tile(mtOff);
					if(orig>=0x8D90 && orig<0x8D93) addObjectTile(o,0x8D32+offset,mtOff);
					else addObjectTile(o,0x8D2E +offset,mtOff);
				} else if(j==1) addObjectTile(o,0x8D29,mtOff);
				else {
					int offset = (j-2)&3;
					addObjectTile(o,0x8D2A+offset,mtOff);
				}
			}
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Snowy platform support
void drawObject_E2(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int height = o->data[3]+1;
	for(int j=0; j<height; j++) {
		int mtOff2 = offsetMap16Right(mtOff);
		int mtOff3 = offsetMap16Right(mtOff2);
		int mtOff4 = offsetMap16Right(mtOff3);
		int offset = ((j&7)<<3);
		if(j<8) {
			WORD tile = romBuf[0x09F464+offset]|(romBuf[0x09F465+offset]<<8);
			if(tile) addObjectTile(o,tile,mtOff);
			tile = romBuf[0x09F466+offset]|(romBuf[0x09F467+offset]<<8);
			addObjectTile(o,tile,mtOff2);
			tile = romBuf[0x09F468+offset]|(romBuf[0x09F469+offset]<<8);
			addObjectTile(o,tile,mtOff3);
			tile = romBuf[0x09F46A+offset]|(romBuf[0x09F46B+offset]<<8);
			if(tile) addObjectTile(o,tile,mtOff4);
		} else {
			WORD tile = romBuf[0x09F4A4+offset]|(romBuf[0x09F4A5+offset]<<8);
			addObjectTile(o,tile,mtOff);
			tile = romBuf[0x09F4A6+offset]|(romBuf[0x09F4A7+offset]<<8);
			addObjectTile(o,tile,mtOff2);
			tile = romBuf[0x09F4A8+offset]|(romBuf[0x09F4A9+offset]<<8);
			addObjectTile(o,tile,mtOff3);
			tile = romBuf[0x09F4AA+offset]|(romBuf[0x09F4AB+offset]<<8);
			addObjectTile(o,tile,mtOff4);
		}
		mtOff = offsetMap16Down(mtOff);
	}
}
//Icy floor hole
void drawObject_E3(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	for(int j=0; j<height; j++) {
		for(int i=0; i<width; i++) {
			if(i==0) {
				if(j==0) addObjectTile(o,0x8C03,mtOff);
				else if(j==1) addObjectTile(o,0x8C07,mtOff);
				else addObjectTile(o,0x8C0B,mtOff);
			} else if((i+1)==width) {
				if(j==0) addObjectTile(o,0x8C00,mtOff);
				else if(j==1) addObjectTile(o,0x8C04,mtOff);
				else addObjectTile(o,0x8C08,mtOff);
			} else addObjectTile(o,0,mtOff);
			if(j==0) {
				int mtOff2 = offsetMap16Up(mtOff);
				addObjectTile(o,0,mtOff2);
			}
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Flower garden ground
void drawObject_E4(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	int seed = 0;
	for(int i=0; i<width; i++) {
		if((i&1)==0) {
			seed = noiseTilemap[mtOff]&0x1E;
			seed = romBuf[0x09F5CE +seed]|(romBuf[0x09F5CF+seed]<<8);
			seed += 0x090000;
		}
		for(int j=0; j<height; j++) {
			if(j==0) {
				int mtOff2 = offsetMap16Up(mtOff);
				int mtOff3 = offsetMap16Up(mtOff2);
				int offset = ((i&1)<<1);
				WORD tile = romBuf[seed+offset]|(romBuf[seed+offset+1]<<8);
				addObjectTile(o,tile,mtOff3);
				tile = romBuf[seed+offset+4]|(romBuf[seed+offset+5]<<8);
				addObjectTile(o,tile,mtOff2);
				tile = romBuf[seed+offset+8]|(romBuf[seed+offset+9]<<8);
				addObjectTile(o,tile,mtOff);
			} else {
				int offset = noiseTilemap[mtOff]&0x1E;
				offset += ((j-1)<<2);
				if(offset>0x1E) offset = 0x1E;
				WORD tile = romBuf[0x09F634+offset]|(romBuf[0x09F635+offset]<<8);
				addObjectTile(o,tile,mtOff);
			}
			mtOff = offsetMap16Down(mtOff);
		}
		mtOff = preserve = offsetMap16Right(preserve);
	}
}
//Flower garden gentle slope left
void drawObject_E5(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = 0x101-o->data[3];
	int height = o->data[4]+1;
	if(width==0x101) width = 1;
	int seed = 0;
	for(int i=0; i<width; i++) {
		if((i&1)==0) {
			seed = noiseTilemap[mtOff]&0xE;
			seed = romBuf[0x09F6C2+seed]|(romBuf[0x09F6C3+seed]<<8);
			seed += 0x090000;
		}
		for(int j=0; j<height; j++) {
			if(j==0) {
				int mtOff2 = offsetMap16Up(mtOff);
				int mtOff3 = offsetMap16Up(mtOff2);
				int offset = ((i&1)<<1);
				WORD tile = romBuf[seed+offset]|(romBuf[seed+offset+1]<<8);
				addObjectTile(o,tile,mtOff3);
				tile = romBuf[seed+offset+4]|(romBuf[seed+offset+5]<<8);
				addObjectTile(o,tile,mtOff2);
				tile = romBuf[seed+offset+8]|(romBuf[seed+offset+9]<<8);
				addObjectTile(o,tile,mtOff);
			} else if(j==1) {
				int offset = ((i&1)<<1);
				WORD tile = romBuf[seed+offset+12]|(romBuf[seed+offset+13]<<8);
				addObjectTile(o,tile,mtOff);
			} else {
				int offset = noiseTilemap[mtOff]&0x1E;
				offset += ((j-2)<<2);
				if(offset>0x1E) offset = 0x1E;
				WORD tile = romBuf[0x09F634+offset]|(romBuf[0x09F635+offset]<<8);
				addObjectTile(o,tile,mtOff);
			}
			mtOff = offsetMap16Down(mtOff);
		}
		if(i&1) {
			mtOff = preserve = offsetMap16Down(preserve);
			height--;
		}
		mtOff = preserve = offsetMap16Left(preserve);
	}
}
//Flower garden steep slope left
void drawObject_E6(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = 0x101-o->data[3];
	int height = o->data[4]+1;
	if(width==0x101) width = 1;
	int seed = 0;
	for(int i=0; i<width; i++) {
		seed = noiseTilemap[mtOff]&0xE;
		seed = romBuf[0x09F767+seed]|(romBuf[0x09F768+seed]<<8);
		seed += 0x090000;
		for(int j=0; j<height; j++) {
			if(j==0) {
				int mtOff2 = offsetMap16Up(mtOff);
				int mtOff3 = offsetMap16Up(mtOff2);
				WORD tile = romBuf[seed]|(romBuf[seed+1]<<8);
				addObjectTile(o,tile,mtOff3);
				tile = romBuf[seed+2]|(romBuf[seed+3]<<8);
				addObjectTile(o,tile,mtOff2);
				tile = romBuf[seed+4]|(romBuf[seed+5]<<8);
				addObjectTile(o,tile,mtOff);
			} else if(j==1) {
				WORD tile = romBuf[seed+6]|(romBuf[seed+7]<<8);
				addObjectTile(o,tile,mtOff);
			} else {
				int offset = noiseTilemap[mtOff]&0x1E;
				offset += ((j-2)<<2);
				if(offset>0x1E) offset = 0x1E;
				WORD tile = romBuf[0x09F634+offset]|(romBuf[0x09F635+offset]<<8);
				addObjectTile(o,tile,mtOff);
			}
			mtOff = offsetMap16Down(mtOff);
		}
		mtOff = preserve = offsetMap16Left(preserve);
		mtOff = preserve = offsetMap16Down(preserve);
		height--;
	}
}
//Flower garden very steep slope left
void drawObject_E7(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = 0x101-o->data[3];
	int height = o->data[4]+1;
	if(width==0x101) width = 1;
	int seed = 0;
	for(int i=0; i<width; i++) {
		seed = noiseTilemap[mtOff]&0xE;
		seed = romBuf[0x09F7CF+seed]|(romBuf[0x09F7D0+seed]<<8);
		seed += 0x090000;
		for(int j=0; j<height; j++) {
			if(j==0) {
				int mtOff2 = offsetMap16Up(mtOff);
				WORD tile = romBuf[seed]|(romBuf[seed+1]<<8);
				addObjectTile(o,tile,mtOff2);
				tile = romBuf[seed+2]|(romBuf[seed+3]<<8);
				addObjectTile(o,tile,mtOff);
			} else if(j==1) {
				WORD tile = romBuf[seed+4]|(romBuf[seed+5]<<8);
				addObjectTile(o,tile,mtOff);
			} else if(j==2) {
				WORD tile = romBuf[seed+6]|(romBuf[seed+7]<<8);
				addObjectTile(o,tile,mtOff);
			} else {
				int offset = noiseTilemap[mtOff]&0x1E;
				offset += ((j-3)<<2);
				if(offset>0x1E) offset = 0x1E;
				WORD tile = romBuf[0x09F634+offset]|(romBuf[0x09F635+offset]<<8);
				addObjectTile(o,tile,mtOff);
			}
			mtOff = offsetMap16Down(mtOff);
		}
		mtOff = preserve = offsetMap16Left(preserve);
		mtOff = preserve = offsetMap16Down(preserve);
		mtOff = preserve = offsetMap16Down(preserve);
		height -= 2;
	}
}
//Flower garden gentle slope right
void drawObject_E8(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	int seed = 0;
	for(int i=0; i<width; i++) {
		if((i&1)==0) {
			seed = noiseTilemap[mtOff]&0xE;
			seed = romBuf[0x09F878+seed]|(romBuf[0x09F879+seed]<<8);
			seed += 0x090000;
		}
		for(int j=0; j<height; j++) {
			if(j==0) {
				int mtOff2 = offsetMap16Up(mtOff);
				int mtOff3 = offsetMap16Up(mtOff2);
				int offset = ((i&1)<<1);
				WORD tile = romBuf[seed+offset]|(romBuf[seed+offset+1]<<8);
				addObjectTile(o,tile,mtOff3);
				tile = romBuf[seed+offset+4]|(romBuf[seed+offset+5]<<8);
				addObjectTile(o,tile,mtOff2);
				tile = romBuf[seed+offset+8]|(romBuf[seed+offset+9]<<8);
				addObjectTile(o,tile,mtOff);
			} else if(j==1) {
				int offset = ((i&1)<<1);
				WORD tile = romBuf[seed+offset+12]|(romBuf[seed+offset+13]<<8);
				addObjectTile(o,tile,mtOff);
			} else {
				int offset = noiseTilemap[mtOff]&0x1E;
				offset += ((j-2)<<2);
				if(offset>0x1E) offset = 0x1E;
				WORD tile = romBuf[0x09F634+offset]|(romBuf[0x09F635+offset]<<8);
				addObjectTile(o,tile,mtOff);
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
//Flower garden steep slope right
void drawObject_E9(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	int seed = 0;
	for(int i=0; i<width; i++) {
		seed = noiseTilemap[mtOff]&0xE;
		seed = romBuf[0x09F93F+seed]|(romBuf[0x09F940+seed]<<8);
		seed += 0x090000;
		for(int j=0; j<height; j++) {
			if(j==0) {
				int mtOff2 = offsetMap16Up(mtOff);
				int mtOff3 = offsetMap16Up(mtOff2);
				WORD tile = romBuf[seed]|(romBuf[seed+1]<<8);
				addObjectTile(o,tile,mtOff3);
				tile = romBuf[seed+2]|(romBuf[seed+3]<<8);
				addObjectTile(o,tile,mtOff2);
				tile = romBuf[seed+4]|(romBuf[seed+5]<<8);
				addObjectTile(o,tile,mtOff);
			} else if(j==1) {
				WORD tile = romBuf[seed+6]|(romBuf[seed+7]<<8);
				addObjectTile(o,tile,mtOff);
			} else {
				int offset = noiseTilemap[mtOff]&0x1E;
				offset += ((j-2)<<2);
				if(offset>0x1E) offset = 0x1E;
				WORD tile = romBuf[0x09F634+offset]|(romBuf[0x09F635+offset]<<8);
				addObjectTile(o,tile,mtOff);
			}
			mtOff = offsetMap16Down(mtOff);
		}
		mtOff = preserve = offsetMap16Right(preserve);
		mtOff = preserve = offsetMap16Down(preserve);
		height--;
	}
}
//Flower garden very slope right
void drawObject_EA(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	int seed = 0;
	for(int i=0; i<width; i++) {
		seed = noiseTilemap[mtOff]&0xE;
		seed = romBuf[0x09F9A7+seed]|(romBuf[0x09F9A8+seed]<<8);
		seed += 0x090000;
		for(int j=0; j<height; j++) {
			if(j==0) {
				int mtOff2 = offsetMap16Up(mtOff);
				WORD tile = romBuf[seed]|(romBuf[seed+1]<<8);
				addObjectTile(o,tile,mtOff2);
				tile = romBuf[seed+2]|(romBuf[seed+3]<<8);
				addObjectTile(o,tile,mtOff);
			} else if(j==1) {
				WORD tile = romBuf[seed+4]|(romBuf[seed+5]<<8);
				addObjectTile(o,tile,mtOff);
			} else if(j==2) {
				WORD tile = romBuf[seed+6]|(romBuf[seed+7]<<8);
				addObjectTile(o,tile,mtOff);
			} else {
				int offset = noiseTilemap[mtOff]&0x1E;
				offset += ((j-3)<<2);
				if(offset>0x1E) offset = 0x1E;
				WORD tile = romBuf[0x09F634+offset]|(romBuf[0x09F635+offset]<<8);
				addObjectTile(o,tile,mtOff);
			}
			mtOff = offsetMap16Down(mtOff);
		}
		mtOff = preserve = offsetMap16Right(preserve);
		mtOff = preserve = offsetMap16Down(preserve);
		mtOff = preserve = offsetMap16Down(preserve);
		height -= 2;
	}
}
//Flower garden left edge
void drawObject_EB(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	int seed = 0;
	for(int i=0; i<width; i++) {
		seed = noiseTilemap[mtOff]&0xE;
		seed = romBuf[0x09F7CF+seed]|(romBuf[0x09F7D0+seed]<<8);
		seed += 0x090000;
		for(int j=0; j<height; j++) {
			if(j==0) {
				int mtOff2 = offsetMap16Up(mtOff);
				WORD tile = romBuf[seed]|(romBuf[seed+1]<<8);
				addObjectTile(o,tile,mtOff2);
				tile = romBuf[seed+2]|(romBuf[seed+3]<<8);
				addObjectTile(o,tile,mtOff);
			} else if(j==1) {
				WORD tile = romBuf[seed+4]|(romBuf[seed+5]<<8);
				addObjectTile(o,tile,mtOff);
			} else {
				int mtOff2 = offsetMap16Right(mtOff);
				WORD orig2 = getOriginalMap16Tile(mtOff2);
				if((orig2&0xFF00)==0x7900) {
					int offset = noiseTilemap[mtOff2]&6;
					WORD tile = romBuf[0x09F9F9+offset]|(romBuf[0x09F9FA+offset]<<8);
					addObjectTile(o,tile,mtOff2);
				}
				WORD orig = getOriginalMap16Tile(mtOff);
				mtOff2 = offsetMap16Left(mtOff);
				orig2 = getOriginalMap16Tile(mtOff2);
				if((orig2&0xFF00)==0x0300 || (orig2&0xFF00)==0x0600 ||
				(orig2&0xFF00)==0x0800 || (orig2&0xFF00)==0x0A00 ||
				(orig2&0xFF00)==0x0C00 || (orig2&0xFF00)==0x1000 ||
				(orig>=0x85A8 && orig<0x85B0)) addObjectTile(o,0x79C8,mtOff);
				else addObjectTile(o,(j&1)?0x79D7:0x79D6,mtOff);
			}
			mtOff = offsetMap16Down(mtOff);
		}
		mtOff = preserve = offsetMap16Right(preserve);
	}
}
//Flower garden right edge
void drawObject_EC(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	int seed = 0;
	for(int i=0; i<width; i++) {
		seed = noiseTilemap[mtOff]&0xE;
		seed = romBuf[0x09F9A7+seed]|(romBuf[0x09F9A8+seed]<<8);
		seed += 0x090000;
		for(int j=0; j<height; j++) {
			if(j==0) {
				int mtOff2 = offsetMap16Up(mtOff);
				WORD tile = romBuf[seed]|(romBuf[seed+1]<<8);
				addObjectTile(o,tile,mtOff2);
				tile = romBuf[seed+2]|(romBuf[seed+3]<<8);
				addObjectTile(o,tile,mtOff);
			} else if(j==1) {
				WORD tile = romBuf[seed+4]|(romBuf[seed+5]<<8);
				addObjectTile(o,tile,mtOff);
			} else {
				int mtOff2 = offsetMap16Left(mtOff);
				WORD orig2 = getOriginalMap16Tile(mtOff2);
				if((orig2&0xFF00)==0x7900) {
					int offset = noiseTilemap[mtOff2]&6;
					WORD tile = romBuf[0x09F9F9+offset]|(romBuf[0x09F9FA+offset]<<8);
					addObjectTile(o,tile,mtOff2);
				}
				WORD orig = getOriginalMap16Tile(mtOff);
				mtOff2 = offsetMap16Right(mtOff);
				orig2 = getOriginalMap16Tile(mtOff2);
				if((orig2&0xFF00)==0x0300 || (orig2&0xFF00)==0x0600 ||
				(orig2&0xFF00)==0x0800 || (orig2&0xFF00)==0x0A00 ||
				(orig2&0xFF00)==0x0C00 || (orig2&0xFF00)==0x1000 ||
				(orig>=0x85A8 && orig<0x85B0)) addObjectTile(o,0x79C9,mtOff);
				else addObjectTile(o,(j&1)?0x79D9:0x79D8,mtOff);
			}
			mtOff = offsetMap16Down(mtOff);
		}
		mtOff = preserve = offsetMap16Right(preserve);
	}
}
//3D stone wall
void drawObject_ED(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = 0x101-o->data[4];
	if(height==0x101) height = 1;
	for(int j=0; j<height; j++) {
		int shadeOffs = 0;
		for(int i=0; i<width; i++) {
			bool rhs = (mtOff&1)^((mtOff>>8)&1);
			if(j==0) {
				if((i==0 && rhs) || (i!=0 & (i+1)==width && !rhs)) addObjectTile(o,0x3D09,mtOff);
				else addObjectTile(o,rhs?0x3D0B:0x3D0A,mtOff);
			} else if(j==1) {
				if(i==0 || !rhs) shadeOffs = (noiseTilemap[mtOff]&1)*3;
				if((i==0 && rhs) || (i!=0 & (i+1)==width && !rhs)) addObjectTile(o,0x79E8+shadeOffs,mtOff);
				else addObjectTile(o,rhs?(0x79EA+shadeOffs):(0x79E9+shadeOffs),mtOff);
			} else if(j==2) {
				if((i==0 && rhs) || (i!=0 & (i+1)==width && !rhs)) addObjectTile(o,0x79EB,mtOff);
				else addObjectTile(o,rhs?0x79ED:0x79EC,mtOff);
			} else if(j==3) {
				if(i==0 || !rhs) shadeOffs = (noiseTilemap[mtOff]&1)*3;
				if((i==0 && rhs) || (i!=0 & (i+1)==width && !rhs)) addObjectTile(o,0x79EB+shadeOffs,mtOff);
				else addObjectTile(o,rhs?(0x79ED+shadeOffs):(0x79EC+shadeOffs),mtOff);
			} else {
				if((i==0 && rhs) || (i!=0 & (i+1)==width && !rhs)) addObjectTile(o,0x79EE,mtOff);
				else addObjectTile(o,rhs?0x79F0:0x79EF,mtOff);
			}
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Up(preserve);
	}
}
//3D stone
void drawObject_EE(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	for(int j=0; j<height; j++) {
		int shadeOffs = 0;
		for(int i=0; i<width; i++) {
			bool rhs = (mtOff&1)^((mtOff>>8)&1);
			if(j==0) {
				if((i==0 && rhs) || (i!=0 && (i+1)==width && !rhs)) addObjectTile(o,0x002D,mtOff);
				else addObjectTile(o,rhs?0x0029:0x0028,mtOff);
			} else if(j==1) {
				if((i==0 && !rhs) || (i!=0 && (i+1)==width && rhs)) addObjectTile(o,0x010A,mtOff);
				else addObjectTile(o,rhs?0x0100:0x0101,mtOff);
			} else if(j==2) {
				if((i==0 && rhs) || (i!=0 && (i+1)==width && !rhs)) addObjectTile(o,0x0105,mtOff);
				else addObjectTile(o,rhs?0x0104:0x0103,mtOff);
			} else {
				if(i==0 || !rhs) {
					shadeOffs = (noiseTilemap[mtOff]&3)+(j-3);
					if(shadeOffs>7) shadeOffs = (noiseTilemap[mtOff]&1)+6;
				}
				if(shadeOffs<3) {
					if((i==0 && rhs) || ((i+1)==width && !rhs)) addObjectTile(o,0x0106,mtOff);
					else addObjectTile(o,rhs?0x7793:0x7792,mtOff);
				} else if(shadeOffs<5) {
					if((i==0 && rhs) || ((i+1)==width && !rhs)) addObjectTile(o,0x778C,mtOff);
					else addObjectTile(o,rhs?0x778E:0x778D,mtOff);
				} else if(shadeOffs<7) {
					if((i==0 && rhs) || ((i+1)==width && !rhs)) addObjectTile(o,0x778F,mtOff);
					else addObjectTile(o,rhs?0x7791:0x7790,mtOff);
				} else {
					addObjectTile(o,0,mtOff);
				}
			}
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//3D stone with solid bottom
void drawObject_EF(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int preserve = mtOff;
	int width = o->data[3]+1;
	int height = o->data[4]+1;
	for(int j=0; j<height; j++) {
		int shadeOffs = 0;
		for(int i=0; i<width; i++) {
			bool rhs = (mtOff&1)^((mtOff>>8)&1);
			if(j==0) {
				if((i==0 && rhs) || (i!=0 && (i+1)==width && !rhs)) addObjectTile(o,0x002D,mtOff);
				else addObjectTile(o,rhs?0x0029:0x0028,mtOff);
			} else if(j==1) {
				if((i==0 && !rhs) || (i!=0 && (i+1)==width && rhs)) addObjectTile(o,0x010A,mtOff);
				else addObjectTile(o,rhs?0x0100:0x0101,mtOff);
			} else if(j==2) {
				if((i==0 && rhs) || (i!=0 && (i+1)==width && !rhs)) addObjectTile(o,0x0105,mtOff);
				else addObjectTile(o,rhs?0x0104:0x0103,mtOff);
			} else {
				if(i==0 || !rhs) {
					shadeOffs = (noiseTilemap[mtOff]&3)+(j-3);
					if(shadeOffs>7) shadeOffs = (noiseTilemap[mtOff]&1)+6;
				}
				if(shadeOffs<3) {
					if((i==0 && rhs) || ((i+1)==width && !rhs)) addObjectTile(o,0x0106,mtOff);
					else addObjectTile(o,rhs?0x0109:0x0108,mtOff);
				} else if(shadeOffs<5) {
					if((i==0 && rhs) || ((i+1)==width && !rhs)) addObjectTile(o,0x79E1,mtOff);
					else addObjectTile(o,rhs?0x79E3:0x79E2,mtOff);
				} else if(shadeOffs<7) {
					if((i==0 && rhs) || ((i+1)==width && !rhs)) addObjectTile(o,0x79E4,mtOff);
					else addObjectTile(o,rhs?0x79E6:0x79E5,mtOff);
				} else {
					addObjectTile(o,0x79E7,mtOff);
				}
			}
			mtOff = offsetMap16Right(mtOff);
		}
		mtOff = preserve = offsetMap16Down(preserve);
	}
}
//Vertical pipe
void drawObject_F4(object_t * o) {
	int mtOff = getBaseMap16Offset(o);
	int height = o->data[3];
	if(height&0x80) {
		height = 0x101-height;
		for(int j=0; j<height; j++) {
			int mtOff2 = offsetMap16Right(mtOff);
			if(j==0) {
				addObjectTile(o,0x79A8,mtOff);
				addObjectTile(o,0x79A9,mtOff2);
			} else if((j+1)==height) {
				addObjectTile(o,0x79A0,mtOff);
				addObjectTile(o,0x79A1,mtOff2);
			} else {
				addObjectTile(o,0x79F3,mtOff);
				addObjectTile(o,0x79F4,mtOff2);
			}
			mtOff = offsetMap16Up(mtOff);
		}
	} else {
		height++;
		for(int j=0; j<height; j++) {
			int mtOff2 = offsetMap16Right(mtOff);
			if(j==0) {
				addObjectTile(o,0x79F1,mtOff);
				addObjectTile(o,0x79F2,mtOff2);
			} else if((j+1)==height) {
				addObjectTile(o,0x79F5,mtOff);
				addObjectTile(o,0x79F6,mtOff2);
			} else {
				addObjectTile(o,0x79F3,mtOff);
				addObjectTile(o,0x79F4,mtOff2);
			}
			mtOff = offsetMap16Down(mtOff);
		}
	}
}
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
	drawObject_extended9C,drawObject_extended9C,drawObject_extended9E,drawObject_extended9E,
	//A0
	drawObject_extendedA0,drawObject_extendedA1,drawObject_extendedA2,drawObject_extendedA3,
	drawObject_extendedA4,drawObject_extendedA5,drawObject_extendedA6,drawObject_extendedA7,
	drawObject_extendedA8,drawObject_extendedA9,drawObject_extendedA9,drawObject_extendedA9,
	drawObject_extendedAC,drawObject_extendedAD,drawObject_extendedAD,drawObject_extendedAD,
	//B0
	drawObject_extendedAD,drawObject_extendedAD,drawObject_extendedAD,drawObject_extendedB3,
	drawObject_extendedB4,drawObject_extendedB4,drawObject_extendedB6,drawObject_extendedB6,
	drawObject_extendedB8,drawObject_extendedB8,drawObject_extendedBA,drawObject_extendedBA,
	drawObject_extendedBA,drawObject_extendedBA,drawObject_extendedBA,drawObject_extendedBA,
	//C0
	drawObject_extendedC0,drawObject_extendedC1,drawObject_extendedC2,drawObject_extendedC2,
	drawObject_extendedC4,drawObject_extendedC5,drawObject_extendedC5,drawObject_extendedC5,
	drawObject_extendedC5,drawObject_extendedC5,drawObject_extendedCA,drawObject_extendedCA,
	drawObject_extendedCA,drawObject_extendedCA,drawObject_extendedCA,drawObject_extendedCA,
	//D0
	drawObject_extendedCA,drawObject_extendedCA,drawObject_extendedCA,drawObject_extendedCA,
	drawObject_extendedD4,drawObject_extendedD4,drawObject_extendedD4,drawObject_extendedD4,
	drawObject_extendedD4,drawObject_extendedD4,drawObject_extendedD4,drawObject_extendedD4,
	drawObject_extendedD4,drawObject_extendedD4,drawObject_extendedD4,drawObject_extendedD4,
	//E0
	drawObject_extendedE0,drawObject_unused,drawObject_unused,drawObject_unused,
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
	drawObject_10,drawObject_11,drawObject_12,drawObject_13,
	drawObject_14,drawObject_15,drawObject_16,drawObject_17,
	drawObject_18,drawObject_19,drawObject_1A,drawObject_1B,
	drawObject_1C,drawObject_1D,drawObject_1D,drawObject_1F,
	//20
	drawObject_20,drawObject_21,drawObject_22,drawObject_23,
	drawObject_24,drawObject_25,drawObject_26,drawObject_27,
	drawObject_28,drawObject_29,drawObject_2A,drawObject_0C,
	drawObject_2C,drawObject_2D,drawObject_2E,drawObject_2F,
	//30
	drawObject_30,drawObject_31,drawObject_32,drawObject_33,
	drawObject_34,drawObject_35,drawObject_31,drawObject_37,
	drawObject_38,drawObject_39,drawObject_3A,drawObject_3B,
	drawObject_3C,drawObject_3D,drawObject_3E,drawObject_3F,
	//40
	drawObject_3F,drawObject_41,drawObject_42,drawObject_43,
	drawObject_44,drawObject_45,drawObject_46,drawObject_47,
	drawObject_48,drawObject_49,drawObject_4A,drawObject_4B,
	drawObject_4C,drawObject_4D,drawObject_4E,drawObject_4F,
	//50
	drawObject_50,drawObject_51,drawObject_52,drawObject_53,
	drawObject_54,drawObject_55,drawObject_56,drawObject_57,
	drawObject_58,drawObject_59,drawObject_5A,drawObject_5B,
	drawObject_5C,drawObject_5D,drawObject_5E,drawObject_5F,
	//60
	drawObject_60,drawObject_61,drawObject_62,drawObject_63,
	drawObject_63,drawObject_63,drawObject_66,drawObject_67,
	drawObject_68,drawObject_69,drawObject_6A,drawObject_6B,
	drawObject_6C,drawObject_6D,drawObject_6E,drawObject_6F,
	//70
	drawObject_70,drawObject_70,drawObject_70,drawObject_73,
	drawObject_74,drawObject_75,drawObject_76,drawObject_77,
	drawObject_78,drawObject_79,drawObject_7A,drawObject_7B,
	drawObject_7C,drawObject_7D,drawObject_7E,drawObject_7F,
	//80
	drawObject_80,drawObject_81,drawObject_82,drawObject_83,
	drawObject_84,drawObject_85,drawObject_86,drawObject_87,
	drawObject_87,drawObject_89,drawObject_8A,drawObject_8B,
	drawObject_8C,drawObject_8D,drawObject_8E,drawObject_8F,
	//90
	drawObject_90,drawObject_91,drawObject_92,drawObject_93,
	drawObject_94,drawObject_94,drawObject_94,drawObject_94,
	drawObject_98,drawObject_99,drawObject_9A,drawObject_9B,
	drawObject_9C,drawObject_9D,drawObject_9E,drawObject_9F,
	//A0
	drawObject_A0,drawObject_A0,drawObject_A0,drawObject_A3,
	drawObject_A3,drawObject_A5,drawObject_A6,drawObject_A7,
	drawObject_A8,drawObject_A9,drawObject_AA,drawObject_AB,
	drawObject_AC,drawObject_AD,drawObject_AE,drawObject_AF,
	//B0
	drawObject_B0,drawObject_B1,drawObject_B2,drawObject_B3,
	drawObject_B4,drawObject_B5,drawObject_B6,drawObject_B7,
	drawObject_B8,drawObject_B9,drawObject_BA,drawObject_BB,
	drawObject_BC,drawObject_BD,drawObject_BE,drawObject_BF,
	//C0
	drawObject_C0,drawObject_C1,drawObject_C2,drawObject_C3,
	drawObject_C4,drawObject_C5,drawObject_C6,drawObject_C7,
	drawObject_C8,drawObject_C9,drawObject_CA,drawObject_CB,
	drawObject_CC,drawObject_CD,drawObject_CE,drawObject_CF,
	//D0
	drawObject_D0,drawObject_D1,drawObject_D2,drawObject_D3,
	drawObject_D4,drawObject_D5,drawObject_D6,drawObject_D7,
	drawObject_D8,drawObject_D9,drawObject_DA,drawObject_DB,
	drawObject_DC,drawObject_DD,drawObject_DE,drawObject_DF,
	//E0
	drawObject_E0,drawObject_E1,drawObject_E2,drawObject_E3,
	drawObject_E4,drawObject_E5,drawObject_E6,drawObject_E7,
	drawObject_E8,drawObject_E9,drawObject_EA,drawObject_EB,
	drawObject_EC,drawObject_ED,drawObject_EE,drawObject_EF,
	//F0
	drawObject_EF,drawObject_EF,drawObject_EF,drawObject_EF,
	drawObject_F4,drawObject_F5,drawObject_F6,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused,
	drawObject_unused,drawObject_unused,drawObject_unused,drawObject_unused};
int setObjectContext(int ctx) {
	int prevCtx = curObjCtx;
	curObjCtx = ctx;
	return prevCtx;
}
void drawObjects() {
	//Clear buffers
	for(int i=0; i<0x8000; i++) {
		objectContexts[curObjCtx].assocObjects[i].clear();
		objectContexts[curObjCtx].invalidObjects[i] = false;
	}
	memset(objectContexts[curObjCtx].tilemap,0,0x10000);
	//Draw objects
	for(int n=0; n<objectContexts[curObjCtx].objects.size(); n++) {
		object_t * thisObject = &objectContexts[curObjCtx].objects[n];
		thisObject->occupiedTiles.clear();
		int id = thisObject->data[0];
		if(id) {
			objectDrawFunc[id](thisObject);
			//Draw text for objects which have no tiles
			if(thisObject->occupiedTiles.size()==0) {
				int mtOff = getBaseMap16Offset(thisObject);
				addObjectTile(thisObject,0xBF00+id,mtOff);
			}
		}
		else {
			id = thisObject->data[3];
			objectDrawFunc_extended[id](thisObject);
			//Draw text for objects which have no tiles
			if(thisObject->occupiedTiles.size()==0) {
				int mtOff = getBaseMap16Offset(thisObject);
				addObjectTile(thisObject,0xBE00+id,mtOff);
			}
		}
	}
}
void dispObjects(DWORD * pixelBuf,int width,int height,RECT rect) {
	int minx = std::max((int)(rect.left&0x7FF0),0);
	int miny = std::max((int)(rect.top&0x7FF0),0);
	int maxx = std::min((int)((rect.right&0x7FF0)+0x10),0x1000);
	int maxy = std::min((int)((rect.bottom&0x7FF0)+0x10),0x800);
	char obStr[256];
	for(int j=miny; j<maxy; j+=0x10) {
		for(int i=minx; i<maxx; i+=0x10) {
			int tileIdx = (i>>4)|((j>>4)<<8);
			WORD tile = objectContexts[curObjCtx].tilemap[tileIdx];
			if((tile&0xFF00)==0xBC00) {
				switch(tile&0xFF) {
					//Screen copy
					case 0xB0: {
						obStr[0] = 'S';
						obStr[1] = 'c';
						obStr[2] = ' ';
						obStr[3] = 'C';
						break;
					}
					case 0xB1: {
						obStr[0] = 'r';
						obStr[1] = 'e';
						obStr[2] = 'o';
						obStr[3] = 'p';
						break;
					}
					case 0xB2: {
						obStr[0] = 'e';
						obStr[1] = 'n';
						obStr[2] = 'y';
						obStr[3] = ' ';
						break;
					}
					//Screen scroll enable
					case 0xD0: 
					case 0xE0: {
						obStr[0] = 'S';
						obStr[1] = 'c';
						obStr[2] = ' ';
						obStr[3] = ' ';
						break;
					}
					case 0xD1: {
						obStr[0] = 'r';
						obStr[1] = 'e';
						obStr[2] = ' ';
						obStr[3] = ' ';
						break;
					}
					case 0xD2: {
						obStr[0] = 'e';
						obStr[1] = 'n';
						obStr[2] = 'E';
						obStr[3] = 'n';
						break;
					}
					case 0xD3: 
					case 0xE3: {
						obStr[0] = ' ';
						obStr[1] = ' ';
						obStr[2] = 'a';
						obStr[3] = 'b';
						break;
					}
					case 0xD4: 
					case 0xE4: {
						obStr[0] = 'S';
						obStr[1] = 'c';
						obStr[2] = 'l';
						obStr[3] = 'e';
						break;
					}
					case 0xD5: 
					case 0xE5 :{
						obStr[0] = 'r';
						obStr[1] = 'o';
						obStr[2] = ' ';
						obStr[3] = ' ';
						break;
					}
					case 0xD6: 
					case 0xE6: {
						obStr[0] = 'l';
						obStr[1] = 'l';
						obStr[2] = ' ';
						obStr[3] = ' ';
						break;
					}
					//Screen scroll disable
					case 0xE1: {
						obStr[0] = 'r';
						obStr[1] = 'e';
						obStr[2] = ' ';
						obStr[3] = 'D';
						break;
					}
					case 0xE2: {
						obStr[0] = 'e';
						obStr[1] = 'n';
						obStr[2] = 'i';
						obStr[3] = 's';
						break;
					}
					//Screen erase
					case 0xF0: {
						obStr[0] = 'S';
						obStr[1] = 'c';
						obStr[2] = 'E';
						obStr[3] = 'r';
						break;
					}
					case 0xF1: {
						obStr[0] = 'r';
						obStr[1] = 'e';
						obStr[2] = 'a';
						obStr[3] = 's';
						break;
					}
					case 0xF2: {
						obStr[0] = 'e';
						obStr[1] = 'n';
						obStr[2] = 'e';
						obStr[3] = ' ';
						break;
					}
				}
				dispMap8Char(pixelBuf,width,height,0xFF,0xFFFFFF,obStr[0],{i,j},false);
				dispMap8Char(pixelBuf,width,height,0xFF,0xFFFFFF,obStr[1],{i+8,j},false);
				dispMap8Char(pixelBuf,width,height,0xFF,0xFFFFFF,obStr[2],{i,j+8},false);
				dispMap8Char(pixelBuf,width,height,0xFF,0xFFFFFF,obStr[3],{i+8,j+8},false);
			} else if((tile&0xFF00)==0xBE00) {
				snprintf(obStr,256,"OX%02X",tile&0xFF);
				dispMap8Char(pixelBuf,width,height,0xFF,0xFFFFFF,obStr[0],{i,j},false);
				dispMap8Char(pixelBuf,width,height,0xFF,0xFFFFFF,obStr[1],{i+8,j},false);
				dispMap8Char(pixelBuf,width,height,0xFF,0xFFFFFF,obStr[2],{i,j+8},false);
				dispMap8Char(pixelBuf,width,height,0xFF,0xFFFFFF,obStr[3],{i+8,j+8},false);
			} else if((tile&0xFF00)==0xBF00) {
				snprintf(obStr,256,"O %02X",tile&0xFF);
				dispMap8Char(pixelBuf,width,height,0xFF,0xFFFFFF,obStr[0],{i,j},false);
				dispMap8Char(pixelBuf,width,height,0xFF,0xFFFFFF,obStr[1],{i+8,j},false);
				dispMap8Char(pixelBuf,width,height,0xFF,0xFFFFFF,obStr[2],{i,j+8},false);
				dispMap8Char(pixelBuf,width,height,0xFF,0xFFFFFF,obStr[3],{i+8,j+8},false);
			} else if(tile!=0x0000) {
				dispMap16Tile(pixelBuf,width,height,tile,{i,j},false);
			}
			//Check object selection to highlight/invert
			int hiliteInvertFlag = 0;
			for(int n=0; n<objectContexts[curObjCtx].assocObjects[tileIdx].size(); n++) {
				if(objectContexts[curObjCtx].assocObjects[tileIdx][n]->selected) {
					hiliteInvertFlag = 1;
				} else if(hiliteInvertFlag==1) {
					hiliteInvertFlag = 2;
				}
			}
			if(hiliteInvertFlag==1) {
				for(int l=0; l<16; l++) {
					for(int k=0; k<16; k++) {
						invertPixel(pixelBuf,width,height,{i+k,j+l});
					}
				}
			} else if(hiliteInvertFlag==2) {
				for(int l=0; l<16; l++) {
					for(int k=0; k<16; k++) {
						hilitePixel(pixelBuf,width,height,0xFF0000,{i+k,j+l});
					}
				}
			}
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

//Manipulation
int selectObjects(RECT rect) {
	//Select nothing by default
	clearObjectSelection();
	//Get tile region
	int minx = std::max((int)(rect.left&0x7FF0),0);
	int miny = std::max((int)(rect.top&0x7FF0),0);
	int maxx = std::min((int)((rect.right&0x7FF0)+0x10),0x1000);
	int maxy = std::min((int)((rect.bottom&0x7FF0)+0x10),0x800);
	//For each tile, mark all occupied objects as selected
	for(int j=miny; j<maxy; j+=0x10) {
		for(int i=minx; i<maxx; i+=0x10) {
			int tileIdx = (i>>4)|(j<<4);
			for(int n=0; n<objectContexts[0].assocObjects[tileIdx].size(); n++) {
				object_t * thisObject = objectContexts[0].assocObjects[tileIdx][n];
				thisObject->selected = true;
			}
		}
	}
}
void clearObjectSelection() {
	//Deselect all objects
	for(int n=0; n<objectContexts[0].objects.size(); n++) {
		object_t * thisObject = &objectContexts[0].objects[n];
		thisObject->selected = false;
	}
}
void insertObjects(int x,int y) {
	x >>= 4;
	y >>= 4;
	int numSelectedObjects = 0;
	int minX = 0x8000,minY = 0x8000;
	int maxX = 0,maxY = 0;
	//Check if any objects are to be pasted
	for(int n=0; n<objectContexts[0].objects.size(); n++) {
		object_t * thisObject = &objectContexts[0].objects[n];
		if(thisObject->selected) {
			BYTE hi = thisObject->data[1];
			BYTE lo = thisObject->data[2];
			int xpos = (lo&0xF)|((hi&0xF)<<4);
			int ypos = ((lo&0xF0)>>4)|(hi&0xF0);
			minX = std::min(xpos,minX);
			minY = std::min(ypos,minY);
			maxX = std::max(xpos,maxX);
			maxY = std::max(ypos,maxY);
			numSelectedObjects++;
		}
	}
	if(numSelectedObjects) {
		//Determine if any objects will be out of bounds after this operation,
		//and if so, terminate
		//TODO
		//Paste selected objects
		//TODO
	} else {
		//Determine if any objects will be out of bounds after this operation,
		//and if so, terminate
		//TODO
		//Insert current objects in selection dialog
		//TODO
	}
}
void deleteObjects() {
	//Delete selected objects
	remove_if(objectContexts[0].objects.begin(),objectContexts[0].objects.end(),object_delPred);
}
void selectTopObject(int x,int y) {
	//Select top object
	int tileIdx = (x>>4)|((y&0x7FF0)<<4);
	int topIdx = objectContexts[0].assocObjects[tileIdx].size()-1;
	if(topIdx>=0) {
		object_t * thisObject = objectContexts[0].assocObjects[tileIdx][topIdx];
		if(!thisObject->selected) {
			clearObjectSelection();
			thisObject->selected = true;
		}
	}
}
void moveObjects(int dx,int dy) {
	dx >>= 4;
	dy >>= 4;
	int numSelectedObjects = 0;
	int minX = 0x8000,minY = 0x8000;
	int maxX = 0,maxY = 0;
	//Check if any objects are to be moved
	for(int n=0; n<objectContexts[0].objects.size(); n++) {
		object_t * thisObject = &objectContexts[0].objects[n];
		if(thisObject->selected) {
			BYTE hi = thisObject->data[1];
			BYTE lo = thisObject->data[2];
			int xpos = (lo&0xF)|((hi&0xF)<<4);
			int ypos = ((lo&0xF0)>>4)|(hi&0xF0);
			minX = std::min(xpos,minX);
			minY = std::min(ypos,minY);
			maxX = std::max(xpos,maxX);
			maxY = std::max(ypos,maxY);
			numSelectedObjects++;
		}
	}
	if(numSelectedObjects) {
		//Determine if any objects will be out of bounds after this operation,
		//and if so, terminate
		if((dx<0 && (minX+dx)<0) || (dx>0 && (maxX+dx)>=0x100) || (dy<0 && (minY+dy)<0) || (dy>0 && (maxY+dy)>=0x80)) return;
		//Move selected objects
		for(int n=0; n<objectContexts[0].objects.size(); n++) {
			object_t * thisObject = &objectContexts[0].objects[n];
			if(thisObject->selected) {
				BYTE hi = thisObject->data[1];
				BYTE lo = thisObject->data[2];
				int xpos = (lo&0xF)|((hi&0xF)<<4);
				int ypos = ((lo&0xF0)>>4)|(hi&0xF0);
				xpos += dx;
				ypos += dy;
				thisObject->data[1] = ((xpos&0xF0)>>4)|(ypos&0xF0);
				thisObject->data[2] = (xpos&0xF)|((ypos&0xF)<<4);
			}
		}
	}
}
void resizeObjects(int dx,int dy) {
	dx >>= 4;
	dy >>= 4;
	int numSelectedObjects = 0;
	int minX = 0x8000,minY = 0x8000;
	int maxX = 0,maxY = 0;
	//Check if any objects are to be resized
	for(int n=0; n<objectContexts[0].objects.size(); n++) {
		object_t * thisObject = &objectContexts[0].objects[n];
		if(thisObject->selected) {
			//TODO
		}
	}
	if(numSelectedObjects) {
		//Determine if any objects will be oversized after this operation,
		//and if so, terminate
		//TODO
		//Resize selected objects
		for(int n=0; n<objectContexts[0].objects.size(); n++) {
			object_t * thisObject = &objectContexts[0].objects[n];
			if(thisObject->selected) {
				//TODO
			}
		}
	}
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
HWND			hwndCbObject,hwndLbObject;
RECT invRect_object = {0,0,0x100,0x100};

BYTE objectDlgData_t0[] = {
	//00
	0x01,0x00,0x66,0x03,0x03,0xFF,
	0x02,0x00,0x67,0x03,0xFF,0xFF,
	0x03,0x00,0x68,0x03,0xFF,0xFF,
	
	
	
};
LPCTSTR objectDlgNames_t0[] = {
	//00
	"01\tNormal ground",
	"02\tNormal ground left edge",
	"03\tNormal ground left edge",
	
	
	
};
BYTE objectDlgData_t1[] = {
	//00
	0x00,0x00,0x56,0x00,0xFF,0xFF,
	0x00,0x00,0x58,0x01,0xFF,0xFF,
	0x00,0x00,0x56,0x02,0xFF,0xFF,
	0x00,0x00,0x58,0x03,0xFF,0xFF,
	0x00,0x00,0x57,0x04,0xFF,0xFF,
	0x00,0x00,0x58,0x05,0xFF,0xFF,
	0x00,0x00,0x57,0x06,0xFF,0xFF,
	0x00,0x00,0x58,0x07,0xFF,0xFF,
	0x00,0x00,0x57,0x08,0xFF,0xFF,
	0x00,0x00,0x56,0x09,0xFF,0xFF,
	
	
	
};
LPCTSTR objectDlgNames_t1[] = {
	//00
	"00\tJungle leaf facing left",
	"01\tJungle leaf facing right",
	"02\tJungle leaf facing left, short",
	"03\tJungle leaf facing right, short",
	"04\tSmall jungle leaf facing left",
	"05\tSmall jungle leaf facing right",
	"06\tSmall jungle leaf facing left short",
	"07\tSmall jungle leaf facing right short",
	"08\t3 jungle leaves",
	"09\t2 jungle leaves",
	
	
	
};
LPCTSTR whatsThisObjectExt[0x100] = {
	//00
	
	
	
};
LPCTSTR whatsThisObject[0x100] = {
	//00
	"An extended object command. This description should not appear.\r\nObject ID: 00",
	"A normal ground.\r\nObject ID: 01",
	"A normal ground left edge.\r\nObject ID: 02",
	"A normal ground right edge.\r\nObject ID: 03",
	
	
	
};

int focusObject(int x,int y,UINT * cursor,TCHAR * text) {
	//Get top object
	int tileIdx = (x>>4)|((y&0x7FF0)<<4);
	int topIdx = objectContexts[0].assocObjects[tileIdx].size()-1;
	if(topIdx<0) {
		//Return default
		return 4;
	} else {
		object_t * thisObject = objectContexts[0].assocObjects[tileIdx][topIdx];
		int id = thisObject->data[0];
		int idExt = thisObject->data[3];
		*cursor = 0x7F86; //IDC_SIZEALL
		if(id) {
			//_tcscpy(text,whatsThisObject[id]);
		} else {
			//_tcscpy(text,whatsThisObjectExt[idExt]);
		}
		int retval = 5;
		if(thisObject->selected) {
			int resizeCaps = romBuf[0x0904EC+id]+1;
			//Check for horizontal resize
			bool horizNeg = false;
			if(resizeCaps&1) {
				int width = thisObject->data[3];
				if((resizeCaps&0x80) && (width&0x80) && ((x&0xF)<4)) {
					bool edgeFlag = true;
					for(int n=0; n<thisObject->occupiedTiles.size(); n++) {
						if(thisObject->occupiedTiles[n]==(tileIdx-1)) {
							edgeFlag = false;
							break;
						}
					}
					if(edgeFlag) {
						*cursor = 0x7F84; //IDC_SIZEWE
						retval = 1;
						horizNeg = true;
					}
				} else if((x&0xF)>=0xC) {
					bool edgeFlag = true;
					for(int n=0; n<thisObject->occupiedTiles.size(); n++) {
						if(thisObject->occupiedTiles[n]==(tileIdx+1)) {
							edgeFlag = false;
							break;
						}
					}
					if(edgeFlag) {
						*cursor = 0x7F84; //IDC_SIZEWE
						retval = 1;
					}
				}
			}
			//Check for vertical resize
			if(resizeCaps&2) {
				int height = thisObject->data[3];
				if((resizeCaps&3)==3) height = thisObject->data[4];
				if((resizeCaps&0x40) && (height&0x80) && ((y&0xF)<4)) {
					bool edgeFlag = true;
					for(int n=0; n<thisObject->occupiedTiles.size(); n++) {
						if(thisObject->occupiedTiles[n]==(tileIdx-0x100)) {
							edgeFlag = false;
							break;
						}
					}
					if(edgeFlag) {
						retval |= 2;
						if(retval==3) {
							if(horizNeg) {
								*cursor = 0x7F82; //IDC_SIZENWSE
							} else {
								*cursor = 0x7F83; //IDC_SIZENESW
							}
						} else {
							*cursor = 0x7F85; //IDC_SIZENS
						}
					}
				} else if((y&0xF)>=0xC) {
					bool edgeFlag = true;
					for(int n=0; n<thisObject->occupiedTiles.size(); n++) {
						if(thisObject->occupiedTiles[n]==(tileIdx+0x100)) {
							edgeFlag = false;
							break;
						}
					}
					if(edgeFlag) {
						retval |= 2;
						if(retval==3) {
							if(horizNeg) {
								*cursor = 0x7F83; //IDC_SIZENESW
							} else {
								*cursor = 0x7F82; //IDC_SIZENWSE
							}
						} else {
							*cursor = 0x7F85; //IDC_SIZENS
						}
					}
				}
			}
		}
		return retval;
	}
}

//Control updaters
void updateWindowSub_object() {
	int prevCtx = setObjectContext(1);
	int idx = SendMessage(hwndCbObject,CB_GETCURSEL,0,0);
	int idx2 = SendMessage(hwndLbObject,LB_GETCURSEL,0,0);
	if(idx==0) {
		loadObjects(&objectDlgData_t0[idx2*6]);
	} else if(idx==1) {
		loadObjects(&objectDlgData_t1[idx2*6]);
	}
	drawObjects();
	setObjectContext(prevCtx);
}
void updateWindow_object() {
	//Remove previous elements
	int prevSize = SendMessage(hwndLbObject,LB_GETCOUNT,0,0);
	for(int i=0; i<prevSize; i++) {
		SendMessage(hwndLbObject,LB_DELETESTRING,0,0);
	}
	//Add new elements and select first
	int idx = SendMessage(hwndCbObject,CB_GETCURSEL,0,0);
	if(idx==0) {
		int arrSize = sizeof(objectDlgNames_t0)/sizeof(LPCTSTR);
		for(int i=0; i<arrSize; i++) {
			SendMessage(hwndLbObject,LB_ADDSTRING,0,(LPARAM)objectDlgNames_t0[i]);
		}
	} else if(idx==1) {
		int arrSize = sizeof(objectDlgNames_t1)/sizeof(LPCTSTR);
		for(int i=0; i<arrSize; i++) {
			SendMessage(hwndLbObject,LB_ADDSTRING,0,(LPARAM)objectDlgNames_t1[i]);
		}
	}
	SendMessage(hwndLbObject,LB_SETCURSEL,0,0);
	updateWindowSub_object();
}
//Main drawing code
void updateEntireScreen_obj() {
	//memset(bmpDataObj,1,0x10000*sizeof(DWORD));
	for(int i=0; i<0x10000; i++) {
		if((i&0x10)^((i&0x1000)>>8)) {
			bmpDataObj[i] = 0x01010101;
		} else {
			bmpDataObj[i] = 0x81818181;
		}
	}
	int prevCtx = setObjectContext(1);
	drawObjects();
	dispObjects(bmpDataObj,0x100,0x100,{0,0,0x100,0x100});
	setObjectContext(prevCtx);
}

LRESULT CALLBACK WndProc_Object(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {
	switch(msg) {
		//Creation and destruction of window(s)
		case WM_CREATE: {
			//Add controls
			hwndCbObject = CreateWindow(WC_COMBOBOX,NULL,CBS_DROPDOWNLIST|WS_CHILD|WS_VISIBLE|WS_TABSTOP|WS_VSCROLL,
				0,256,256,100,
				hwnd,(HMENU)20,hinstMain,NULL);
			hwndLbObject = CreateWindow(WC_LISTBOX,NULL,LBS_NOTIFY|LBS_NOINTEGRALHEIGHT|LBS_USETABSTOPS|WS_CHILD|WS_VISIBLE|WS_TABSTOP|WS_VSCROLL,
				0,282,256,102,
				hwnd,(HMENU)25,hinstMain,NULL);
			//Create objects
			hdcObj = GetDC(hwnd);
			BITMAPINFO bmi;
			memset(&bmi.bmiHeader,0,sizeof(BITMAPINFOHEADER));
			bmi.bmiHeader.biSize			= sizeof(BITMAPINFOHEADER);
			bmi.bmiHeader.biPlanes			= 1;
			bmi.bmiHeader.biBitCount		= 32;
			bmi.bmiHeader.biCompression		= BI_RGB;
			bmi.bmiHeader.biWidth			= 0x100;
			bmi.bmiHeader.biHeight			= -0x100;
			hbmpObj = CreateDIBSection(hdcObj,&bmi,DIB_RGB_COLORS,(void**)&bmpDataObj,NULL,0);
			memset(bmpDataObj,0,0x10000*sizeof(DWORD));
			//Setup font
			int ppi = GetDeviceCaps(hdcObj,LOGPIXELSY);
			int height = -ppi/9;
			HFONT hfont = CreateFont(height,0,0,0,
				FW_NORMAL,FALSE,FALSE,FALSE,
				DEFAULT_CHARSET,
				OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,
				DEFAULT_PITCH|FF_SWISS,"MS Shell Dlg");
			SendMessage(hwndCbObject,WM_SETFONT,(WPARAM)hfont,FALSE);
			SendMessage(hwndLbObject,WM_SETFONT,(WPARAM)hfont,FALSE);
			//Init combo boxes
			SendMessage(hwndCbObject,CB_ADDSTRING,0,(LPARAM)"Standard Objects");
			SendMessage(hwndCbObject,CB_ADDSTRING,0,(LPARAM)"Extended Objects");
			SendMessage(hwndCbObject,CB_SETCURSEL,0,0);
			//Init control values
			updateWindow_object();
			break;
		}
		case WM_DESTROY: {
			//Free objects
			DeleteDC(hdcObj);
			DeleteObject(hbmpObj);
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
			//Update
			updateEntireScreen_obj();
			//Blit screen
			PAINTSTRUCT ps;
			BeginPaint(hwnd,&ps);
			HDC hdcMem = CreateCompatibleDC(hdcObj);
			HBITMAP hbmpOld = (HBITMAP)SelectObject(hdcMem,hbmpObj);
			BitBlt(hdcObj,0,0,0x100,0x100,hdcMem,0,0,SRCCOPY);
			SelectObject(hdcMem,hbmpOld);
			DeleteDC(hdcMem);
			EndPaint(hwnd,&ps);
			break;
		}
		//Input
		case WM_COMMAND: {
			switch(LOWORD(wParam)) {
				case 20: {
					if(HIWORD(wParam)==CBN_SELCHANGE) {
						updateWindow_object();
						InvalidateRect(hwnd,&invRect_object,false);
						UpdateWindow(hwnd);
						break;
					}
				}
				case 25: {
					if(HIWORD(wParam)==LBN_SELCHANGE) {
						updateWindowSub_object();
						InvalidateRect(hwnd,&invRect_object,false);
						UpdateWindow(hwnd);
						break;
					}
				}
			}
			break;
		}
		default: {
			return DefWindowProc(hwnd,msg,wParam,lParam);
		}
	}
	return 0;
}

