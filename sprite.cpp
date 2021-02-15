#include "sprite.h"

HWND hwndSprite;
bool wvisSprite = false;
level_sprite_data_ctx_t spriteContexts[2];
int curSpCtx = 0;

////////////////////////////////
//SPRITE OCCUPY REGISTRATION &//
// 8x8/16x16 SECTION HANDLING //
////////////////////////////////
void addSpriteTile(sprite_t * s,BYTE props,DWORD tile,int offsX,int offsY) {
	//Simple setup
	sprite_tile_t entry;
	entry.props		 = props;
	entry.tile		 = tile;
	entry.offsX		 = offsX;
	entry.offsY		 = offsY;
	//Calculate occupied tile data
	int spX = s->data[2]<<4;
	int spY = (s->data[1]&0xFE)<<3;
	entry.numOccupiedTiles = 0;
	switch(tile&0xC000) {
		case 0x0000: {
			if(props&1) {
				//16x16
				entry.numOccupiedTiles = 1;
				if(offsX&0xF) entry.numOccupiedTiles <<= 1;
				if(offsY&0xF) entry.numOccupiedTiles <<= 1;
				//TODO
			} else {
				//8x8
				entry.numOccupiedTiles = 1;
				if((offsX&0xF)>8) entry.numOccupiedTiles <<= 1;
				if((offsY&0xF)>8) entry.numOccupiedTiles <<= 1;
				//TODO
			}
			break;
		}
		case 0x4000: {
			//16x16
			entry.numOccupiedTiles = 1;
			if(offsX&0xF) entry.numOccupiedTiles <<= 1;
			if(offsY&0xF) entry.numOccupiedTiles <<= 1;
			//TODO
			break;
		}
		case 0x8000: {
			//256x1
			entry.numOccupiedTiles = 16;
			if(offsX&0xF) entry.numOccupiedTiles++;
			//TODO
			break;
		}
		case 0xC000: {
			entry.numOccupiedTiles = 1;
			if((offsX&0xF)>8) entry.numOccupiedTiles <<= 1;
			if((offsY&0xF)>8) entry.numOccupiedTiles <<= 1;
			//TODO
			break;
		}
	}
	//Store in sprite
	s->tiles.push_back(entry);
}

////////////////////////////
//SPRITE DRAWING FUNCTIONS//
////////////////////////////
BYTE spGfxFiles[6];

//Helper function for drawing text
void drawSpriteText(sprite_t * s,char * text) {
	int offsX = 0;
	int offsY = 0;
	while(true) {
		char c = *(text++);
		if(c=='\0') break;
		else if(c=='\n') {
			offsX = 0;
			offsY += 8;
			continue;
		} else {
			addSpriteTile(s,0,0xC000|c,offsX,offsY);
			offsX += 8;
		}
	}
}

//SuperFX texture displayer function
void dispSuperFXTexture(DWORD * pixelBuf,int width,int height,BYTE props,WORD tile,POINT offs) {
	int offsX = offs.x;
	int offsY = offs.y;
	bool flipV = props&0x80;
	bool flipH = props&0x40;
	int palette = (props&0x3C)<<2;
	int tx = (tile&0xF)<<4;
	int ty = tile&0x1F0;
	for(int j=0; j<16; j++) {
		for(int i=0; i<16; i++) {
			int sx = flipH?(0xF-i):i;
			int sy = flipV?(0xF-j):j;
			sx += tx;
			sy += ty;
			int dx = offsX+i;
			int dy = offsY+j;
			int pixShift = (tile&0x200)?4:0;
			int idx = getIndexFromTexture(&romBuf[0x140000],{sx,sy});
			idx = (idx>>pixShift)&0xF;
			if(idx) {
				putPixel(pixelBuf,width,height,paletteBuffer[palette|idx],{dx,dy});
			}
		}
	}
}
//HDMA displayer function
void dispBackgroundRow(DWORD * pixelBuf,int width,int height,BYTE props,int row,POINT offs) {
	int offsX = offs.x;
	int offsY = offs.y;
	bool flipH = props&0x40;
	//TODO
	for(int i=0; i<256; i++) {
		//TODO
	}
}

//Floating log
void drawSprite_000(sprite_t * s) {
	addSpriteTile(s,(0xC<<2),0x4046,0,0);
	addSpriteTile(s,(0xC<<2),0x4047,16,0);
	addSpriteTile(s,(0xC<<2),0x4056,0,16);
	addSpriteTile(s,(0xC<<2),0x4057,16,16);
}
//Closed door
void drawSprite_001(sprite_t * s) {
	addSpriteTile(s,(0x8<<2),0x4302,-8,-16);
	addSpriteTile(s,(0x8<<2)|0x40,0x4302,8,-16);
	addSpriteTile(s,(0x8<<2),0x4312,-8,0);
	addSpriteTile(s,(0x8<<2)|0x40,0x4312,8,0);
}
//Naval Piranha stalk
void drawSprite_002(sprite_t * s) {
	//TODO
}
//Crate
void drawSprite_003(sprite_t * s) {
	addSpriteTile(s,(0xC<<2),0x4128,0,0);
	addSpriteTile(s,(0xC<<2),0x4129,16,0);
	addSpriteTile(s,(0xC<<2),0x4138,0,16);
	addSpriteTile(s,(0xC<<2),0x4139,16,16);
}
//Star Mario block item
void drawSprite_004(sprite_t * s) {
	//TODO
}
//Icy watermelon
void drawSprite_005(sprite_t * s) {
	addSpriteTile(s,(0xB<<2)|1,0x0068,0,0);
}
//Icy watermelon projectile
void drawSprite_006(sprite_t * s) {
	//TODO
}
//Watermelon
void drawSprite_007(sprite_t * s) {
	addSpriteTile(s,(0x8<<2)|1,0x0068,0,0);
}
//Rubble from final boss
void drawSprite_008(sprite_t * s) {
	//TODO
}
//Fire watermelon
void drawSprite_009(sprite_t * s) {
	addSpriteTile(s,(0x9<<2)|1,0x0068,0,0);
}
//Kaboomba
void drawSprite_00A(sprite_t * s) {
	//TODO
}
//Kaboomba projectile
void drawSprite_00B(sprite_t * s) {
	//TODO
}
//Raphael Raven
void drawSprite_00C(sprite_t * s) {
	//TODO
}
//Goal Ring
void drawSprite_00D(sprite_t * s) {
	addSpriteTile(s,(0xA<<2),0x4034,24,-16);
	addSpriteTile(s,(0x9<<2),0x4041,13,-26);
	addSpriteTile(s,(0xA<<2),0x4034,6,-50);
	addSpriteTile(s,(0x9<<2),0x4041,6,-79);
	addSpriteTile(s,(0xA<<2),0x4034,13,-103);
	addSpriteTile(s,(0x9<<2),0x4041,24,-112);
	addSpriteTile(s,(0xA<<2),0x4034,34,-103);
	addSpriteTile(s,(0x9<<2),0x4041,41,-79);
	addSpriteTile(s,(0xA<<2),0x4034,41,-49);
	addSpriteTile(s,(0x9<<2),0x4041,34,-25);
}
//GOAL!
void drawSprite_00E(sprite_t * s) {
	//TODO
}
//BONUS CHALLENGE
void drawSprite_00F(sprite_t * s) {
	//TODO
}
//Sewer ghost blob
void drawSprite_010(sprite_t * s) {
	//TODO
}
//Minigame prize card
void drawSprite_011(sprite_t * s) {
	//TODO
}
//Boss door
void drawSprite_012(sprite_t * s) {
	addSpriteTile(s,(0x9<<2),0x416C,-8,-16);
	addSpriteTile(s,(0x9<<2)|0x40,0x416C,8,-16);
	addSpriteTile(s,(0x9<<2),0x417C,-8,0);
	addSpriteTile(s,(0x9<<2)|0x40,0x417C,8,0);
}
//Boss explosion
void drawSprite_013(sprite_t * s) {
	//TODO
}
//Boss key
void drawSprite_014(sprite_t * s) {
	//TODO
}
//Submarine Yoshi projectile
void drawSprite_015(sprite_t * s) {
	//TODO
}
//Bigger Boo
void drawSprite_016(sprite_t * s) {
	//TODO
}
//Frog Pirate
void drawSprite_017(sprite_t * s) {
	//TODO
}
//Fire watermelon projectile
void drawSprite_018(sprite_t * s) {
	//TODO
}
//Bubble
void drawSprite_019(sprite_t * s) {
	//TODO
}
//Ski lift
void drawSprite_01A(sprite_t * s) {
	//TODO
}
//Vertical log on lava
void drawSprite_01B(sprite_t * s) {
	//TODO
}
//Dr. Freezegood
void drawSprite_01C(sprite_t * s) {
	//TODO
}
//Dr. Freezegood with ski lift
void drawSprite_01D(sprite_t * s) {
	drawSprite_01A(s);
	drawSprite_01C(s);
}
//Shy-Guy
void drawSprite_01E(sprite_t * s) {
	int offset = ((s->data[2]&1)|(s->data[1]&2))<<1;
	int pal = ((romBuf[0x0209B8+offset]-1)<<1)|0x21;
	addSpriteTile(s,pal|1,0x0088,0,-4);
	addSpriteTile(s,(0x8<<2),0x009E,1,8);
	addSpriteTile(s,(0x8<<2),0x009E,7,8);
}
//Rotating door roulette
void drawSprite_01F(sprite_t * s) {
	//TODO
}
//Bandit
void drawSprite_020(sprite_t * s) {
	//TODO
}
//Bucket
void drawSprite_021(sprite_t * s) {
	addSpriteTile(s,(0xA<<2),0x412C,-8,-8);
	addSpriteTile(s,(0xA<<2),0x412D,8,-8);
	addSpriteTile(s,(0xA<<2),0x413C,-8,8);
	addSpriteTile(s,(0xA<<2),0x413D,8,8);
}
//Egg
void drawSprite_022(sprite_t * s) {
	int spRef = (s->data[0]-0x22)<<2;
	int pal = (0xB<<2)-spRef;
	addSpriteTile(s,pal|1,0x0082,0,0);
}
//Giant egg from final boss
void drawSprite_026(sprite_t * s) {
	//TODO
}
//Key
void drawSprite_027(sprite_t * s) {
	addSpriteTile(s,(0x9<<2)|1,0x00EE,8,8);
}
//TODO
//Red coin
void drawSprite_065(sprite_t * s) {
	addSpriteTile(s,(0x9<<2)|1,0x00A0,0,0);
}
//TODO

//Sprite function pointer table and updater
void drawSprite_unused(sprite_t * s) {}
void (*spriteDrawFunc[0x200])(sprite_t * s) = {
	//000
	drawSprite_000,drawSprite_001,drawSprite_002,drawSprite_003,
	drawSprite_004,drawSprite_005,drawSprite_006,drawSprite_007,
	drawSprite_008,drawSprite_009,drawSprite_00A,drawSprite_00B,
	drawSprite_00C,drawSprite_00D,drawSprite_00E,drawSprite_00F,
	//010
	drawSprite_010,drawSprite_011,drawSprite_012,drawSprite_013,
	drawSprite_014,drawSprite_015,drawSprite_016,drawSprite_017,
	drawSprite_018,drawSprite_019,drawSprite_01A,drawSprite_01B,
	drawSprite_01C,drawSprite_01D,drawSprite_01E,drawSprite_01F,
	//020
	drawSprite_020,drawSprite_021,drawSprite_022,drawSprite_022,
	drawSprite_022,drawSprite_022,drawSprite_026,drawSprite_027,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	//030
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	//040
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	//050
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	//060
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_065,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	//070
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	//080
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	//090
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	//0A0
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	//0B0
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	//0C0
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	//0D0
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	//0E0
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	//0F0
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	//100
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_003,drawSprite_unused,
	//110
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	//120
	drawSprite_unused,drawSprite_unused,drawSprite_021,drawSprite_021,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	//130
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	//140
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	//150
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	//160
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	//170
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	//180
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	//190
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	//1A0
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	//1B0
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	//1C0
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	//1D0
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	//1E0
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	//1F0
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused};
int setSpriteContext(int ctx) {
	int prevCtx = curSpCtx;
	curSpCtx = ctx;
	return prevCtx;
}
void drawSprites() {
	for(int n = 0; n < spriteContexts[curSpCtx].sprites.size(); n++) {
		sprite_t * thisSprite = &spriteContexts[curSpCtx].sprites[n];
		int id = thisSprite->data[0]|(thisSprite->data[1]<<8);
		id &= 0x1FF;
		spriteDrawFunc[id](thisSprite);
	}
}
void drawSingleSprite(int n) {
	sprite_t * thisSprite = &spriteContexts[curSpCtx].sprites[n];
	int id = thisSprite->data[0]|(thisSprite->data[1]<<8);
	id &= 0x1FF;
	spriteDrawFunc[id](thisSprite);
}
void dispSprites(DWORD * pixelBuf,int width,int height,RECT rect) {
	for(int n = 0; n < spriteContexts[curSpCtx].sprites.size(); n++) {
		sprite_t * thisSprite = &spriteContexts[curSpCtx].sprites[n];
		int spX = thisSprite->data[2]<<4;
		int spY = (thisSprite->data[1]&0xFE)<<3;
		for(int i=0; i<thisSprite->tiles.size(); i++) {
			sprite_tile_t * thisSpriteTile = &thisSprite->tiles[i];
			BYTE props = thisSpriteTile->props;
			WORD tile = thisSpriteTile->tile;
			int sptX = spX+thisSpriteTile->offsX;
			int sptY = spY+thisSpriteTile->offsY;
			switch(tile&0xC000) {
				case 0x0000: {
					dispMap8Tile(pixelBuf,width,height,props,tile+0x480,{sptX,sptY});
					break;
				}
				case 0x4000: {
					dispSuperFXTexture(pixelBuf,width,height,props,tile&0x3FF,{sptX,sptY});
					break;
				}
				case 0x8000: {
					dispBackgroundRow(pixelBuf,width,height,props,tile&0x3FF,{sptX,sptY});
					break;
				}
				case 0xC000: {
					dispMap8Char(pixelBuf,width,height,0xFF,0xFFFFFF,tile&0x7F,{sptX,sptY});
					break;
				}
			}
		}
	}
}
void initOtherSpriteBuffers() {
	int spTs = ((levelHeader[4]&7)<<4)|(levelHeader[5]>>4);
	for(int i=0; i<6; i++) {
		spGfxFiles[i] = romBuf[0x003039+i+(spTs*6)];
	}
}

/////////////////////
//SPRITE MANAGEMENT//
/////////////////////
//Predicate for deletion
bool sprite_delPred(sprite_t & un) {
	return un.selected;
}

//Load/save
void loadSprites(BYTE * data) {
	//Clear buffers
	for(int i=0; i<0x8000; i++) {
		spriteContexts[curSpCtx].assocSprites[i].clear();
		spriteContexts[curSpCtx].invalidSprites[i] = false;
	}
	spriteContexts[curSpCtx].sprites.clear();
	//Reload buffer with sprite data
	while(true) {
		//Create entry
		sprite_t entry;
		//Get data bytes and check for end
		entry.data[0] = *data++;
		entry.data[1] = *data++;
		if(entry.data[0]==0xFF && entry.data[1]==0xFF) break;
		entry.data[2] = *data++;
		entry.dataSize = 3;
		//Init other elements to sane values
		entry.selected = false;
		entry.tiles.clear();
		//Push back
		spriteContexts[curSpCtx].sprites.push_back(entry);
	}
}
int saveSprites(BYTE * data) {
	//Init stuff
	int curSz = 0;
	//Save sprite data
	for(int n=0; n<spriteContexts[curSpCtx].sprites.size(); n++) {
		//Shorten the names of these for convenience
		BYTE * thisData = spriteContexts[curSpCtx].sprites[n].data;
		int thisDataSz = spriteContexts[curSpCtx].sprites[n].dataSize;
		//Copy data
		memcpy(&data[curSz],thisData,thisDataSz);
		//Increment stuff
		curSz += thisDataSz;
	}
}

//Manipulation (internal)
void addToSpriteSelection(int index) {
	//Select sprite
	sprite_t thisSprite = spriteContexts[0].sprites[index];
	thisSprite.selected = true;
	//Mark occupied tiles as invalid
	for(int i=0; i<thisSprite.tiles.size(); i++) {
		sprite_tile_t thisTile = thisSprite.tiles[i];
		for(int j=0; j<thisTile.numOccupiedTiles; j++) {
			spriteContexts[0].invalidSprites[thisTile.occupiedTiles[j]] = true;
		}
	}
}
void removeFromSpriteSelection(int index) {
	//Deselect sprite
	sprite_t thisSprite = spriteContexts[0].sprites[index];
	thisSprite.selected = false;
	//Mark occupied tiles as invalid
	for(int i=0; i<thisSprite.tiles.size(); i++) {
		sprite_tile_t thisTile = thisSprite.tiles[i];
		for(int j=0; j<thisTile.numOccupiedTiles; j++) {
			spriteContexts[0].invalidSprites[thisTile.occupiedTiles[j]] = true;
		}
	}
}
void clearSpriteSelection() {
	//Deselect all sprites
	for(int n=0; n<spriteContexts[0].sprites.size(); n++) {
		sprite_t thisSprite = spriteContexts[0].sprites[n];
		thisSprite.selected = false;
		//Mark occupied tiles as invalid
		for(int i=0; i<thisSprite.tiles.size(); i++) {
			sprite_tile_t thisTile = thisSprite.tiles[i];
			for(int j=0; j<thisTile.numOccupiedTiles; j++) {
				spriteContexts[0].invalidSprites[thisTile.occupiedTiles[j]] = true;
			}
		}
	}
}

//Manipulation
int selectSprites(RECT rect,bool ctrl) {
	if(ctrl) {
		//TODO
		for(int n=0; n<spriteContexts[0].sprites.size(); n++) {
			//TODO
		}
		//TODO
	} else {
		//TODO
		for(int n=0; n<spriteContexts[0].sprites.size(); n++) {
			//TODO
		}
		//TODO
	}
}
void insertSprites(int x,int y) {
	int numSelectedSprites = 0;
	int minX = 0x8000,minY = 0x8000;
	int maxX = 0,maxY = 0;
	//Check if any sprites are to be pasted
	for(int n=0; n<spriteContexts[0].sprites.size(); n++) {
		sprite_t thisSprite = spriteContexts[0].sprites[n];
		if(thisSprite.selected) {
			int xpos = thisSprite.data[2];
			int ypos = thisSprite.data[1]>>1;
			if(xpos<minX) minX = xpos;
			if(ypos<minY) minY = ypos;
			if(xpos>maxX) maxX = xpos;
			if(ypos>maxY) maxY = ypos;
			numSelectedSprites++;
		}
	}
	if(numSelectedSprites) {
		//Determine if any sprites will be out of bounds after this operation,
		//and if so, terminate
		//TODO
		//Paste selected sprites
		//TODO
	} else {
		//Determine if any sprites will be out of bounds after this operation,
		//and if so, terminate
		//TODO
		//Insert current sprite in selection dialog
		//TODO
	}
}
void deleteSprites() {
	//Invalidate occupied tiles
	for(int n=0; n<spriteContexts[0].sprites.size(); n++) {
		sprite_t thisSprite = spriteContexts[0].sprites[n];
		if(thisSprite.selected) {
			//Mark occupied tiles as invalid
			for(int i=0; i<thisSprite.tiles.size(); i++) {
				sprite_tile_t thisTile = thisSprite.tiles[i];
				for(int j=0; j<thisTile.numOccupiedTiles; j++) {
					spriteContexts[0].invalidSprites[thisTile.occupiedTiles[j]] = true;
				}
			}
		}
	}
	//Delete selected sprites
	remove_if(spriteContexts[0].sprites.begin(),spriteContexts[0].sprites.end(),sprite_delPred);
}
void moveSprites(int dx,int dy) {
	int numSelectedSprites = 0;
	int minX = 0x8000,minY = 0x8000;
	int maxX = 0,maxY = 0;
	//Check if any sprites are to be pasted
	for(int n=0; n<spriteContexts[0].sprites.size(); n++) {
		sprite_t thisSprite = spriteContexts[0].sprites[n];
		if(thisSprite.selected) {
			int xpos = thisSprite.data[2];
			int ypos = thisSprite.data[1]>>1;
			if(xpos<minX) minX = xpos;
			if(ypos<minY) minY = ypos;
			if(xpos>maxX) maxX = xpos;
			if(ypos>maxY) maxY = ypos;
			numSelectedSprites++;
		}
	}
	if(numSelectedSprites) {
		//Determine if any sprites will be out of bounds after this operation,
		//and if so, terminate
		//TODO
		//Move selected sprites
		//TODO
	}
}

///////////////////
//WINDOW FUNCTION//
///////////////////
HDC				hdcSp;
HBITMAP			hbmpSp;
DWORD *			bmpDataSp;

LRESULT CALLBACK WndProc_Sprite(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {
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
			wvisSprite = false;
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

