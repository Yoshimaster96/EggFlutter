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
//Helper function for drawing HDMA sprites from polygon data
void drawSpriteHDMAPolygon(sprite_t * s,BYTE * data,int coneBase,int numPoints,bool ux,bool ny) {
	int lineLeft[0x100],lineRight[0x100];
	memset(lineLeft,0,0x100*sizeof(int));
	memset(lineRight,0,0x100*sizeof(int));
	for(int n=0; n<numPoints; n++) {
		int curX = data[(n<<1)];
		int curY = data[(n<<1)+1];
		int nextX = data[(n<<1)+2];
		int nextY = data[(n<<1)+3];
		if((n+1)==numPoints) {
			nextX = data[0];
			nextY = data[1];
		}
		if(!ux) {
			if(curX&0x80) curX -= 0x100;
			if(nextX&0x80) nextX -= 0x100;
		}
		if(!ny) {
			curY ^= 0x80;
			nextY ^= 0x80;
		}
		if(curY<nextY) {
			int dx = (nextX-curX)<<16;
			int dy = nextY-curY;
			int dxdy = dx/dy;
			int x = curX<<16;
			for(int j=curY; j<=nextY; j++) {
				lineRight[j] = x>>16;
				x += dxdy;
			}
		} else if(curY>nextY) {
			int dx = (nextX-curX)<<16;
			int dy = curY-nextY;
			int dxdy = dx/dy;
			int x = curX<<16;
			for(int j=curY; j>=nextY; j--) {
				lineLeft[j] = x>>16;
				x += dxdy;
			}
		}
	}
	for(int i=0; i<0x100; i++) {
		if(lineLeft[i]<lineRight[i]) {
			int width = lineRight[i]-lineLeft[i];
			if(width>0xFF) width = 0xFF;
			int offsX = (lineRight[i]+lineLeft[i])>>1;
			if(width&1) offsX++;
			int offsY = ny?0x100:0x80;
			addSpriteTile(s,0,coneBase+width,offsX-0x80,i-offsY);
		}
	}
}

//Helper function for finding GFX files
inline int findSpGfxFile(BYTE file) {
	for(int i=5; i>=0; i--) {
		if(spGfxFiles[i]==file) return 0x100+(i<<5);
	}
	return 0;
}

//SuperFX texture displayer function
void dispSuperFXTexture(DWORD * pixelBuf,int width,int height,BYTE props,WORD tile,POINT offs) {
	int offsX = offs.x;
	int offsY = offs.y;
	bool flipV = props&0x80;
	bool flipH = props&0x40;
	int palette = (props&0x3C)<<2;
	int tx = (tile&0xF)<<4;
	int ty = tile&0x3F0;
	for(int j=0; j<16; j++) {
		for(int i=0; i<16; i++) {
			int sx = flipH?(0xF-i):i;
			int sy = flipV?(0xF-j):j;
			if(tile&0x2000) {
				int temp = sx;
				sx = sy;
				sy = temp;
			}
			sx += tx;
			sy += ty;
			int dx = offsX+i;
			int dy = offsY+j;
			int pixShift = (tile&0x400)?4:0;
			int idx = getIndexFromTexture(&romBuf[0x140000],{sx,sy});
			idx = (idx>>pixShift)&0xF;
			if(idx) {
				putPixel(pixelBuf,width,height,paletteBuffer[palette|idx],{dx,dy});
			}
		}
	}
}
//HDMA displayer function
void dispBackgroundRow(DWORD * pixelBuf,int width,int height,int row,POINT offs) {
	int offsX = offs.x;
	int offsY = offs.y;
	int base = ((row&0x1FF)<<10)|((row&0x400)>>1);
	for(int i=0; i<0x100; i++) {
		DWORD color = bmpDataBg[base+i];
		if(color) {
			putPixel(pixelBuf,width,height,color,{offsX+i,offsY});
		}
	}
}

//Floating log
void drawSprite_000(sprite_t * s) {
	addSpriteTile(s,(0xC<<2),0x4046,-8,-8);
	addSpriteTile(s,(0xC<<2),0x4047,8,-8);
	addSpriteTile(s,(0xC<<2),0x4056,-8,8);
	addSpriteTile(s,(0xC<<2),0x4057,8,8);
}
//Closed door
void drawSprite_001(sprite_t * s) {
	addSpriteTile(s,(0x8<<2),0x4502,-8,-16);
	addSpriteTile(s,(0x8<<2)|0x40,0x4502,8,-16);
	addSpriteTile(s,(0x8<<2),0x4512,-8,0);
	addSpriteTile(s,(0x8<<2)|0x40,0x4512,8,0);
}
//Naval Piranha stalk
void drawSprite_002(sprite_t * s) {
	int base = findSpGfxFile(0x5A);
	addSpriteTile(s,(0x9<<2)|0x41,base,-8,-8);
	addSpriteTile(s,(0x9<<2)|1,base,8,-8);
	addSpriteTile(s,(0x9<<2)|0x41,base+0x08,-16,2);
	addSpriteTile(s,(0x9<<2)|1,base+0x08,16,2);
}
//Crate
void drawSprite_003(sprite_t * s) {
	addSpriteTile(s,(0xC<<2),0x4128,-8,-16);
	addSpriteTile(s,(0xC<<2),0x4129,8,-16);
	addSpriteTile(s,(0xC<<2),0x4138,-8,0);
	addSpriteTile(s,(0xC<<2),0x4139,8,0);
}
//Star Mario block item
void drawSprite_004(sprite_t * s) {
	addSpriteTile(s,(0x9<<2)|1,0x00C0,0,0);
}
//Icy watermelon
void drawSprite_005(sprite_t * s) {
	addSpriteTile(s,(0xB<<2)|1,0x0068,0,0);
}
//Icy watermelon projectile
void drawSprite_006(sprite_t * s) {
	addSpriteTile(s,(0x9<<2),0x25D0,0,0);
	addSpriteTile(s,(0x9<<2),0x25D1,8,0);
	addSpriteTile(s,(0x9<<2),0x25D8,0,8);
	addSpriteTile(s,(0x9<<2),0x25D9,8,8);
}
//Watermelon
void drawSprite_007(sprite_t * s) {
	addSpriteTile(s,(0x8<<2)|1,0x0068,0,0);
}
//Rubble from final boss
void drawSprite_008(sprite_t * s) {
	addSpriteTile(s,(0xE<<2),0x408E,-8,-8);
	addSpriteTile(s,(0xE<<2),0x408F,8,-8);
	addSpriteTile(s,(0xE<<2),0x409E,-8,8);
	addSpriteTile(s,(0xE<<2),0x409F,8,8);
}
//Fire watermelon
void drawSprite_009(sprite_t * s) {
	addSpriteTile(s,(0x9<<2)|1,0x0068,0,0);
}
//Kaboomba
void drawSprite_00A(sprite_t * s) {
	int base = findSpGfxFile(0x4A);
	addSpriteTile(s,(0xA<<2),base+0x03,0,-8);
	addSpriteTile(s,(0xA<<2),base+0x0F,0,8);
	addSpriteTile(s,(0xA<<2)|1,base,-8,0);
	addSpriteTile(s,(0xA<<2)|1,base+0x01,0,0);
	addSpriteTile(s,(0xA<<2),base+0x0F,4,8);
}
//Kaboomba projectile
void drawSprite_00B(sprite_t * s) {
	int base = findSpGfxFile(0x4A);
	addSpriteTile(s,(0xA<<2)|1,base+0x0C,0,0);
}
//Raphael Raven
void drawSprite_00C(sprite_t * s) {
	for(int j=0; j<4; j++) {
		for(int i=0; i<4; i++) {
			int offsX = -24+(i<<4);
			int offsY = -24+(j<<4);
			addSpriteTile(s,(0xE<<2),0x4508+i+(j<<4),offsX,offsY);
		}
	}
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
	for(int i=0; i<5; i++) {
		int i2 = 4-i;
		int base = romBuf[0x04AD6B+(i2<<1)]|(romBuf[0x04AD6C+(i2<<1)]<<8);
		base += 0x040000;
		int numTiles = romBuf[0x04AD66+i2];
		int baseX = romBuf[0x04AD61+i];
		for(int n=0; n<numTiles; n++) {
			int offsX = romBuf[base+(n*5)];
			int offsY = romBuf[base+(n*5)+1];
			if(offsX&0x80) offsX -= 0x100;
			if(offsY&0x80) offsY -= 0x100;
			int tileRef = romBuf[base+(n*5)+2]|(romBuf[base+(n*5)+3]<<8);
			WORD tile = tileRef&0x01FF;
			BYTE sz = (tileRef>>8)&0xC0;
			if(romBuf[base+(n*5)+4]) sz++;
			addSpriteTile(s,(0xB<<2)|sz,tile,baseX+offsX,offsY);
		}
	}
}
//BONUS CHALLENGE
void drawSprite_00F(sprite_t * s) {
	for(int n=0; n<46; n++) {
		int offsX = romBuf[0x078276+(n*5)];
		int offsY = romBuf[0x078277+(n*5)];
		if(offsX&0x80) offsX -= 0x100;
		if(offsY&0x80) offsY -= 0x100;
		int tileRef = romBuf[0x078278+(n*5)]|(romBuf[0x078279+(n*5)]<<8);
		WORD tile = tileRef&0x01FF;
		BYTE sz = (tileRef>>8)&0xC0;
		if(romBuf[0x07827A+(n*5)]) sz++;
		addSpriteTile(s,(0xE<<2)|sz,tile,offsX,offsY);
	}
}
//Caged Ghost round mound
void drawSprite_010(sprite_t * s) {
	drawSpriteHDMAPolygon(s,&romBuf[0x036407],0x8400,20,false,false);
	int base = findSpGfxFile(0x42);
	addSpriteTile(s,(0x9<<2)|1,base+0x26,-5,-105);
	addSpriteTile(s,(0x9<<2)|1,base+0x24,-17,-105);
	addSpriteTile(s,(0x9<<2)|1,base+0x00,-16,-88);
	addSpriteTile(s,(0x9<<2)|1,base+0x02,0,-88);
	addSpriteTile(s,(0x9<<2)|1,base+0x22,0,-76);
	addSpriteTile(s,(0x9<<2)|1,base+0x20,-16,-76);
	addSpriteTile(s,(0x9<<2),base+0x3B,0,-99);
	addSpriteTile(s,(0x9<<2),base+0x3B,-10,-99);
}
//Minigame prize card
void drawSprite_011(sprite_t * s) {
	addSpriteTile(s,(0xE<<2),0x4177,-8,-8);
	addSpriteTile(s,(0xE<<2),0x4178,8,-8);
	addSpriteTile(s,(0xE<<2),0x4187,-8,8);
	addSpriteTile(s,(0xE<<2),0x4188,8,8);
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
	addSpriteTile(s,(0xB<<2),0x0055,0,0);
	addSpriteTile(s,(0xB<<2),0x0055,8,0);
	addSpriteTile(s,(0xB<<2),0x0055,0,8);
	addSpriteTile(s,(0xB<<2),0x0055,8,8);
}
//Boss key
void drawSprite_014(sprite_t * s) {
	addSpriteTile(s,(0x9<<2),0x40CE,-8,-8);
	addSpriteTile(s,(0x9<<2),0x40CF,8,-8);
	addSpriteTile(s,(0x9<<2),0x40DE,-8,8);
	addSpriteTile(s,(0x9<<2),0x40DF,8,8);
}
//Submarine Yoshi projectile
void drawSprite_015(sprite_t * s) {
	addSpriteTile(s,(0xD<<2)|0x80,0x603B,0,0);
}
//Bigger Boo
void drawSprite_016(sprite_t * s) {
	for(int i=0; i<120; i++) {
		int offsX = romBuf[0x023D8E +(i<<1)];
		int width = romBuf[0x023D8F+(i<<1)];
		if(offsX&0x80) offsX -= 0x100;
		if(width&1) offsX++;
		addSpriteTile(s,0,0x8000+width,offsX-0x80,i-60);
	}
	addSpriteTile(s,(0x9<<2)|0x40,0x4545,0,-48);
	addSpriteTile(s,(0x9<<2)|0x40,0x4544,16,-48);
	addSpriteTile(s,(0x9<<2)|0x40,0x4555,0,-32);
	addSpriteTile(s,(0x9<<2)|0x40,0x4554,16,-32);
	addSpriteTile(s,(0x9<<2)|0x40,0x4565,0,-16);
	addSpriteTile(s,(0x9<<2)|0x40,0x4564,16,-16);
	addSpriteTile(s,(0x9<<2)|0x40,0x4575,0,0);
	addSpriteTile(s,(0x9<<2)|0x40,0x4574,16,0);
}
//Frog Pirate
void drawSprite_017(sprite_t * s) {
	int base = findSpGfxFile(0x68);
	addSpriteTile(s,(0x9<<2),base+0x08,-2,2);
	addSpriteTile(s,(0x9<<2),base+0x18,-2,10);
	addSpriteTile(s,(0x9<<2)|1,base+0x00,-8,0);
	addSpriteTile(s,(0x9<<2)|1,base+0x01,0,0);
	addSpriteTile(s,(0x9<<2),base+0x0C,-3,-5);
	addSpriteTile(s,(0x9<<2),base+0x0C,4,-5);
	addSpriteTile(s,(0x9<<2),base+0x08,14,2);
	addSpriteTile(s,(0x9<<2),base+0x18,14,10);
	addSpriteTile(s,(0x9<<2),base+0x06,5,10);
	addSpriteTile(s,(0x9<<2),base+0x06,-4,10);
}
//Fire watermelon projectile
void drawSprite_018(sprite_t * s) {
	addSpriteTile(s,(0x9<<2),0x25F0,0,0);
	addSpriteTile(s,(0x9<<2),0x25F1,8,0);
	addSpriteTile(s,(0x9<<2),0x25F8,0,8);
	addSpriteTile(s,(0x9<<2),0x25F9,8,8);
}
//Bubble
void drawSprite_019(sprite_t * s) {
	addSpriteTile(s,(0x9<<2)|0x41,0x00A2,0,0);
}
//Ski lift
void drawSprite_01A(sprite_t * s) {
	int base = findSpGfxFile(0x2E);
	addSpriteTile(s,(0x8<<2)|1,base+0x02,-8,34);
	addSpriteTile(s,(0x8<<2)|0x41,base+0x02,8,34);
	addSpriteTile(s,(0x8<<2)|1,base+0x04,8,18);
	addSpriteTile(s,(0x8<<2)|1,base+0x00,0,2);
}
//Vertical log on lava
void drawSprite_01B(sprite_t * s) {
	int base = findSpGfxFile(0x48);
	addSpriteTile(s,(0xC<<2)|1,base+0x04,0,-8);
	addSpriteTile(s,(0xC<<2)|0x81,base+0x04,0,8);
}
//Dr. Freezegood
void drawSprite_01C(sprite_t * s) {
	addSpriteTile(s,(0x9<<2),0x4424,-8,-8);
	addSpriteTile(s,(0x9<<2),0x4425,8,-8);
	addSpriteTile(s,(0x9<<2),0x4434,-8,8);
	addSpriteTile(s,(0x9<<2),0x4435,8,8);
}
//Dr. Freezegood with ski lift
void drawSprite_01D(sprite_t * s) {
	int base = findSpGfxFile(0x2E);
	addSpriteTile(s,(0x8<<2)|1,base+0x02,-8,34);
	addSpriteTile(s,(0x8<<2)|0x41,base+0x02,8,34);
	addSpriteTile(s,(0x8<<2)|1,base+0x04,8,18);
	addSpriteTile(s,(0x8<<2)|1,base+0x00,0,2);
	addSpriteTile(s,(0x9<<2),0x4424,-8,8);
	addSpriteTile(s,(0x9<<2),0x4425,8,8);
	addSpriteTile(s,(0x9<<2),0x4434,-8,24);
	addSpriteTile(s,(0x9<<2),0x4435,8,24);
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
	addSpriteTile(s,(0x8<<2),0x440F,-8,-48);
	addSpriteTile(s,(0x8<<2)|0x40,0x440F,8,-48);
	addSpriteTile(s,(0x8<<2),0x441F,-8,-32);
	addSpriteTile(s,(0x8<<2)|0x40,0x441F,8,-32);
	addSpriteTile(s,(0x8<<2),0x440F,-40,-16);
	addSpriteTile(s,(0x8<<2)|0x40,0x440F,-24,-16);
	addSpriteTile(s,(0x8<<2),0x441F,-40,0);
	addSpriteTile(s,(0x8<<2)|0x40,0x441F,-24,0);
	addSpriteTile(s,(0x8<<2),0x440F,-8,16);
	addSpriteTile(s,(0x8<<2)|0x40,0x440F,8,16);
	addSpriteTile(s,(0x8<<2),0x441F,-8,32);
	addSpriteTile(s,(0x8<<2)|0x40,0x441F,8,32);
	addSpriteTile(s,(0x8<<2),0x440F,24,-16);
	addSpriteTile(s,(0x8<<2)|0x40,0x440F,40,-16);
	addSpriteTile(s,(0x8<<2),0x441F,24,0);
	addSpriteTile(s,(0x8<<2)|0x40,0x441F,40,0);
	int base = findSpGfxFile(0x31);
	addSpriteTile(s,(0x9<<2)|1,base+0x0C,0,-40);
	addSpriteTile(s,(0x9<<2)|1,base+0x0E,-32,-8);
	addSpriteTile(s,(0x9<<2)|1,base+0x28,0,24);
	addSpriteTile(s,(0x9<<2)|1,base+0x2A,32,-8);
}
//Bandit
void drawSprite_020(sprite_t * s) {
	int base = findSpGfxFile(0x4E);
	addSpriteTile(s,(0x8<<2),base+0x0A,0,5);
	addSpriteTile(s,(0x8<<2),base+0x0B,8,5);
	addSpriteTile(s,(0x8<<2),base+0x1E,11,8);
	addSpriteTile(s,(0x8<<2),base+0x1E,0,8);
	addSpriteTile(s,(0x8<<2)|1,base+0x08,0,-10);
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
	addSpriteTile(s,(0x9<<2)|1,0x0187,4,-8);
	addSpriteTile(s,(0x9<<2)|1,0x0185,-4,0);
	addSpriteTile(s,(0x9<<2),0x0197,12,8);
	addSpriteTile(s,(0x9<<2),0x0194,-4,-8);
}
//Key
void drawSprite_027(sprite_t * s) {
	addSpriteTile(s,(0x9<<2)|1,0x00EE,0,0);
}
//Huffin' Puffin
void drawSprite_028(sprite_t * s) {
	addSpriteTile(s,(0x9<<2),0x009E,4,8);
	addSpriteTile(s,(0x9<<2)|0x40,0x4460,0,-2);
}
//Giant egg which follows Superstar Mario
void drawSprite_029(sprite_t * s) {
	addSpriteTile(s,(0xD<<2),0x25B0,-7,-15);
	addSpriteTile(s,(0xD<<2),0x25B1,1,-15);
	addSpriteTile(s,(0xD<<2),0x25B8,-7,-7);
	addSpriteTile(s,(0xD<<2),0x25B9,1,-7);
	addSpriteTile(s,(0xD<<2),0x25B2,7,-15);
	addSpriteTile(s,(0xD<<2),0x25B3,15,-15);
	addSpriteTile(s,(0xD<<2),0x25BA,7,-7);
	addSpriteTile(s,(0xD<<2),0x25BB,15,-7);
	addSpriteTile(s,(0xD<<2),0x25B4,-7,0);
	addSpriteTile(s,(0xD<<2),0x25B5,1,0);
	addSpriteTile(s,(0xD<<2),0x25BC,-7,8);
	addSpriteTile(s,(0xD<<2),0x25BD,1,8);
	addSpriteTile(s,(0xD<<2),0x25B6,7,0);
	addSpriteTile(s,(0xD<<2),0x25B7,15,0);
	addSpriteTile(s,(0xD<<2),0x25BE,7,8);
	addSpriteTile(s,(0xD<<2),0x25BF,15,8);
}
//Giant egg
void drawSprite_02A(sprite_t * s) {
	int spRef = (s->data[0]-0x2A)<<2;
	int pal = ((0x9<<2)-spRef)|1;
	int base = findSpGfxFile(0x2F);
	addSpriteTile(s,pal,base+0x06,-6,-14);
	addSpriteTile(s,pal,base+0x0A,6,-14);
	addSpriteTile(s,pal,base+0x0C,-6,0);
	addSpriteTile(s,pal,base+0x0E,6,0);
}
//Lunge Fish
void drawSprite_02C(sprite_t * s) {
	int base = findSpGfxFile(0x32);
	addSpriteTile(s,(0x9<<2)|1,base+0x4A,16,16);
	addSpriteTile(s,(0x9<<2)|1,base+0x48,0,16);
	addSpriteTile(s,(0x9<<2)|1,base+0x46,-16,16);
	addSpriteTile(s,(0x9<<2)|1,base+0x2A,16,0);
	addSpriteTile(s,(0x9<<2)|1,base+0x28,0,0);
	addSpriteTile(s,(0x9<<2)|1,base+0x26,-16,0);
	addSpriteTile(s,(0x9<<2)|1,base+0x0A,16,-16);
	addSpriteTile(s,(0x9<<2)|1,base+0x08,0,-16);
	addSpriteTile(s,(0x9<<2)|1,base+0x06,-16,-16);
}
//Salvo
void drawSprite_02D(sprite_t * s) {
	for(int i=0; i<96; i++) {
		int width = romBuf[0x05060F+i];
		addSpriteTile(s,0,0x8000+width,-0x80,i-96);
	}
	int base = findSpGfxFile(0x45);
	addSpriteTile(s,(0x8<<2),base+0x0D,-40,-56);
	addSpriteTile(s,(0x8<<2),base+0x0D,-32,-56);
	addSpriteTile(s,(0x8<<2),base+0x1D,-40,-48);
	addSpriteTile(s,(0x8<<2),base+0x1D,-32,-48);
}
//Salvo eyes
void drawSprite_02E(sprite_t * s) {
	int base = findSpGfxFile(0x45);
	addSpriteTile(s,(0x8<<2),base+0x0D,0,0);
	addSpriteTile(s,(0x8<<2),base+0x0D,8,0);
	addSpriteTile(s,(0x8<<2),base+0x1D,0,8);
	addSpriteTile(s,(0x8<<2),base+0x1D,8,8);
}
//Little Mouser nest
void drawSprite_02F(sprite_t * s) {
	int base = findSpGfxFile(0x28);
	addSpriteTile(s,(0xE<<2)|1,base+0x06,0,0);
}
//Little Mouser
void drawSprite_030(sprite_t * s) {
	int base = findSpGfxFile(0x28);
	addSpriteTile(s,(0xE<<2),base+0x02,0,8);
	addSpriteTile(s,(0xE<<2)|1,base+0x00,-2,-1);
	addSpriteTile(s,(0xE<<2),base+0x14,14,4);
	addSpriteTile(s,(0xE<<2),base+0x02,8,8);
}
//Potted Spiked Fun Guy
void drawSprite_031(sprite_t * s) {
	int base = findSpGfxFile(0x36);
	addSpriteTile(s,(0x8<<2)|0x41,base+0x08,0,-10);
	addSpriteTile(s,(0xC<<2)|1,base+0x0E,0,0);
}
//Roger the Potted Ghost
void drawSprite_034(sprite_t * s) {
	drawSpriteHDMAPolygon(s,&romBuf[0x010E7D],0x8000,64,false,false);
	int base = findSpGfxFile(0x42);
	addSpriteTile(s,(0x9<<2)|1,base+0x20,-5,-86);
	addSpriteTile(s,(0x9<<2)|1,base+0x22,11,-86);
	addSpriteTile(s,(0x9<<2)|1,base+0x02,11,-102);
	addSpriteTile(s,(0x9<<2)|1,base+0x00,-5,-102);
	addSpriteTile(s,(0x9<<2),base+0x3B,1,-111);
	addSpriteTile(s,(0x9<<2),base+0x3B,11,-111);
	addSpriteTile(s,(0x9<<2)|1,base+0x24,-5,-120);
	addSpriteTile(s,(0x9<<2)|1,base+0x26,8,-120);
	addSpriteTile(s,(0x9<<2)|1,base+0x44,6,-143);
	addSpriteTile(s,(0x9<<2)|1,base+0x42,-10,-143);
	addSpriteTile(s,(0x9<<2)|1,base+0x40,-2,-159);
	if(s->data[0]==0x34) {
		addSpriteTile(s,(0x9<<2)|1,base+0x49,4,0);
		addSpriteTile(s,(0x9<<2)|1,base+0x48,-4,0);
		addSpriteTile(s,(0x9<<2),base+0x4F,12,-8);
		addSpriteTile(s,(0x9<<2),base+0x4E,4,-8);
		addSpriteTile(s,(0x9<<2),base+0x4D,-4,-8);
	}
}
//Fake falling wall
void drawSprite_036(sprite_t * s) {
	for(int j=-96; j<16; j++) {
		addSpriteTile(s,0,0x8560,-0x80,j);
	}
}
//Grim Leecher
void drawSprite_037(sprite_t * s) {
	int base = findSpGfxFile(0x46);
	addSpriteTile(s,(0xA<<2)|1,base+0x0A,0,0);
}
//Roger the Potted Ghost projectile
void drawSprite_038(sprite_t * s) {
	int base = findSpGfxFile(0x42);
	addSpriteTile(s,(0xB<<2)|1,base+0x4B,0,0);
}
//Spinning platform
void drawSprite_039(sprite_t * s) {
	
	
	
}
//3 Mini Ravens
void drawSprite_03A(sprite_t * s) {
	int base = findSpGfxFile(0x4F);
	
	
	
}
//Mini Raven
void drawSprite_03B(sprite_t * s) {
	int base = findSpGfxFile(0x4F);
	
	
	
}
//Tap-Tap the Red Nose
void drawSprite_03C(sprite_t * s) {
	
	
	
}
//Seesaw
void drawSprite_03D(sprite_t * s) {
	
	
	
}
//Skinny platform
void drawSprite_03E(sprite_t * s) {
	
	
	
}
//Slime
void drawSprite_03F(sprite_t * s) {
	
	
	
}
//Baby Luigi
void drawSprite_040(sprite_t * s) {
	
	
	
}
//Stork
void drawSprite_041(sprite_t * s) {
	
	
	
}
//Vertical pipe entrance
void drawSprite_042(sprite_t * s) {
	
	
	
}
//Giant Shy-Guy
void drawSprite_043(sprite_t * s) {
	int spRef = (s->data[0]-0x43)<<2;
	int pal = ((0x9<<2)-spRef)|1;
	int base = findSpGfxFile(0x70);
	
	
	
}
//Prince Froggy
void drawSprite_045(sprite_t * s) {
	int base = findSpGfxFile(0x70);
	
	
	
}
//Burt the Bashful
void drawSprite_046(sprite_t * s) {
	
	
	
}
//Shy Guy from Roger the Potted Ghost
void drawSprite_047(sprite_t * s) {
	
	
	
}
//Kamek cutscene
void drawSprite_048(sprite_t * s) {
	int base = findSpGfxFile(0x6A);
	
	
	
}
//TODO
//Red coin
void drawSprite_065(sprite_t * s) {
	addSpriteTile(s,(0x9<<2)|1,0x00A0,0,0);
}
//Wild Piranha
void drawSprite_066(sprite_t * s) {
	
	
	
}
//Winged cloud
void drawSprite_067(sprite_t * s) {
	addSpriteTile(s,(0x8<<2)|0x40,0x00D7,-7,2);
	addSpriteTile(s,(0x8<<2)|0x40,0x00C7,-10,-1);
	addSpriteTile(s,(0x8<<2),0x00D7,15,2);
	addSpriteTile(s,(0x8<<2),0x00C7,18,-1);
	addSpriteTile(s,(0x8<<2)|1,0x00CC,0,0);
}
//Flashing egg block
void drawSprite_068(sprite_t * s) {
	addSpriteTile(s,(0xB<<2)|1,0x00C0,0,0);
}
//Red egg block
void drawSprite_069(sprite_t * s) {
	addSpriteTile(s,(0x9<<2)|1,0x00C0,0,0);
}
//Yellow egg block
void drawSprite_06A(sprite_t * s) {
	addSpriteTile(s,(0xA<<2)|1,0x00C0,0,0);
}
//Hit egg block
void drawSprite_06B(sprite_t * s) {
	addSpriteTile(s,(0x8<<2)|1,0x4074,0,0);
}
//Large spring
void drawSprite_06C(sprite_t * s) {
	addSpriteTile(s,(0x8<<2),0x414E,-8,-8);
	addSpriteTile(s,(0x8<<2),0x414F,8,-8);
	addSpriteTile(s,(0x8<<2),0x415E,-8,8);
	addSpriteTile(s,(0x8<<2),0x415F,8,8);
}
//TODO
//Pot for Potted Spiked Fun Guy
void drawSprite_0A1(sprite_t * s) {
	int base = findSpGfxFile(0x36);
	addSpriteTile(s,(0xC<<2)|1,base+0x0E,0,0);
}
//TODO
//Nep-Enut/Gargantua Blargg
void drawSprite_0A5(sprite_t * s) {
	drawSpriteHDMAPolygon(s,&romBuf[0x0146E4],0x8400,62,false,false);
	
	
	
}
//TODO
//Caged Ghost squeezed in tunnel
void drawSprite_193(sprite_t * s) {
	BYTE tempBuf[26*2];
	memcpy(tempBuf,&romBuf[0x035A63],26*2);
	for(int i=0; i<26; i++) {
		tempBuf[(i<<1)+1] += 0xD0;
	}
	drawSpriteHDMAPolygon(s,tempBuf,0x8400,26,true,false);
	
	
	
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
	drawSprite_028,drawSprite_029,drawSprite_02A,drawSprite_02A,
	drawSprite_02C,drawSprite_02D,drawSprite_02E,drawSprite_02F,
	//030
	drawSprite_030,drawSprite_031,drawSprite_030,drawSprite_030,
	drawSprite_034,drawSprite_034,drawSprite_036,drawSprite_037,
	drawSprite_038,drawSprite_039,drawSprite_03A,drawSprite_03B,
	drawSprite_03C,drawSprite_03D,drawSprite_03E,drawSprite_03F,
	//040
	drawSprite_040,drawSprite_041,drawSprite_042,drawSprite_043,
	drawSprite_043,drawSprite_045,drawSprite_046,drawSprite_047,
	drawSprite_048,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	//050
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_048,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	//060
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_065,drawSprite_066,drawSprite_067,
	drawSprite_068,drawSprite_069,drawSprite_06A,drawSprite_06B,
	drawSprite_06C,drawSprite_unused,drawSprite_unused,drawSprite_unused,
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
	drawSprite_unused,drawSprite_0A1,drawSprite_unused,drawSprite_020,
	drawSprite_020,drawSprite_0A5,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	//0B0
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_067,drawSprite_067,drawSprite_067,
	drawSprite_067,drawSprite_067,drawSprite_067,drawSprite_067,
	drawSprite_067,drawSprite_067,drawSprite_067,drawSprite_067,
	//0C0
	drawSprite_067,drawSprite_067,drawSprite_067,drawSprite_067,
	drawSprite_067,drawSprite_067,drawSprite_067,drawSprite_067,
	drawSprite_067,drawSprite_067,drawSprite_067,drawSprite_067,
	drawSprite_067,drawSprite_unused,drawSprite_unused,drawSprite_unused,
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
	drawSprite_06C,drawSprite_unused,drawSprite_unused,drawSprite_unused,
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
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_193,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	//1A0
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	//1B0
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_020,
	drawSprite_unused,drawSprite_020,drawSprite_unused,drawSprite_020,
	drawSprite_unused,drawSprite_020,drawSprite_unused,drawSprite_unused,
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
					if(tile&0x2000) dispMap8Tile(pixelBuf,width,height,props,tile,{sptX,sptY});
					else dispMap8Tile(pixelBuf,width,height,props,tile+0x480,{sptX,sptY});
					break;
				}
				case 0x4000: {
					dispSuperFXTexture(pixelBuf,width,height,props,tile&0x3FFF,{sptX,sptY});
					break;
				}
				case 0x8000: {
					dispBackgroundRow(pixelBuf,width,height,tile&0x7FF,{sptX,sptY});
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

