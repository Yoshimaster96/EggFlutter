#include "sprite.h"

HWND hwndSprite;
HWND hwndSpriteLb,hwndSpriteCb;
bool wvisSprite = false;
level_sprite_data_ctx_t spriteContexts[2];
int curSpCtx = 0;

////////////////////////////////
//SPRITE OCCUPY REGISTRATION &//
// 8x8/16x16 SECTION HANDLING //
////////////////////////////////
void occupySpriteTile(sprite_t * s,int offset) {
	if(offset>=0 && offset<0x8000) {
		//Store in sprite
		s->occupiedTiles.push_back(offset);
		//Set tile in tilemap for current context
		spriteContexts[curSpCtx].assocSprites[offset].push_back(s);
	}
}
void addSpriteTile(sprite_t * s,BYTE props,DWORD tile,int offsX,int offsY) {
	//Simple setup
	sprite_tile_t entry;
	entry.props		 = props;
	entry.tile		 = tile;
	entry.offsX		 = offsX;
	entry.offsY		 = offsY;
	//Determine occupied tiles
	int spX = s->data[2];
	int spY = s->data[1]&0xFE;
	int spTileIdx = spX|(spY<<7);
	switch(tile&0xC000) {
		case 0x0000: {
			if(props&1) {
				//16x16
				occupySpriteTile(s,spTileIdx);
				if(offsX&0xF) {
					occupySpriteTile(s,spTileIdx+1);
				}
				if(offsY&0xF) {
					occupySpriteTile(s,spTileIdx+0x100);
					if(offsX&0xF) {
						occupySpriteTile(s,spTileIdx+0x101);
					}
				}
			} else {
				//8x8
				occupySpriteTile(s,spTileIdx);
				if((offsX&0xF)>7) {
					occupySpriteTile(s,spTileIdx+1);
				}
				if((offsY&0xF)>7) {
					occupySpriteTile(s,spTileIdx+0x100);
					if((offsX&0xF)>7) {
						occupySpriteTile(s,spTileIdx+0x101);
					}
				}
			}
			break;
		}
		case 0x4000: {
			//16x16
			occupySpriteTile(s,spTileIdx);
			if(offsX&0xF) {
				occupySpriteTile(s,spTileIdx+1);
			}
			if(offsY&0xF) {
				occupySpriteTile(s,spTileIdx+0x100);
				if(offsX&0xF) {
					occupySpriteTile(s,spTileIdx+0x101);
				}
			}
			break;
		}
		case 0x8000: {
			//256x1
			for(int i=0; i<16; i++) {
				occupySpriteTile(s,spTileIdx+i);
			}
			if(offsX&0xF) {
				occupySpriteTile(s,spTileIdx+16);
			}
			break;
		}
		case 0xC000: {
			//8x8
			occupySpriteTile(s,spTileIdx);
			if((offsX&0xF)>7) {
				occupySpriteTile(s,spTileIdx+1);
			}
			if((offsY&0xF)>7) {
				occupySpriteTile(s,spTileIdx+0x100);
				if((offsX&0xF)>7) {
					occupySpriteTile(s,spTileIdx+0x101);
				}
			}
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
void drawSpriteText(sprite_t * s,const char * text) {
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
void drawSpriteHDMAPolygon(sprite_t * s,BYTE * data,int coneBase,int numPoints,bool ux,bool uy,int offsX,int offsY) {
	int lineLeft[0x100],lineRight[0x100];
	for(int j=0; j<0x100; j++) {
		lineLeft[j] = 1000;
		lineRight[j] = -1000;
	}
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
		if(!uy) {
			curY ^= 0x80;
			nextY ^= 0x80;
		}
		if(curY<nextY) {
			int dx = (nextX-curX)<<16;
			int dy = nextY-curY;
			int dxdy = dx/dy;
			int x = curX<<16;
			for(int j=curY; j<=nextY; j++) {
				int rhs = x>>16;
				if(rhs>lineRight[j]) lineRight[j] = rhs;
				x += dxdy;
			}
		} else if(curY>nextY) {
			int dx = (nextX-curX)<<16;
			int dy = curY-nextY;
			int dxdy = dx/dy;
			int x = curX<<16;
			for(int j=curY; j>=nextY; j--) {
				int lhs = x>>16;
				if(lhs<lineLeft[j]) lineLeft[j] = lhs;
				x += dxdy;
			}
		}
	}
	for(int i=0; i<0x100; i++) {
		if(lineLeft[i]<lineRight[i]) {
			int width = lineRight[i]-lineLeft[i];
			if(width>0xFF) width = 0xFF;
			int offsX2 = (lineRight[i]+lineLeft[i])>>1;
			if(width&1) offsX2++;
			int offsY2 = uy?0:0x80;
			addSpriteTile(s,0,coneBase+width,offsX+offsX2-0x80,offsY+i-offsY2);
		}
	}
}
void drawSpriteHDMASpline(sprite_t * s,BYTE * data,int coneBase,int numPoints,bool ux,bool uy,int offsX,int offsY) {
	BYTE * tempBuf = (BYTE*)malloc(numPoints<<4);
	for(int n=0; n<numPoints; n++) {
		int px0 = data[n<<1];
		int py0 = data[1+(n<<1)];
		int px1 = data[((n+1)%numPoints)<<1];
		int py1 = data[1+(((n+1)%numPoints)<<1)];
		int px2 = data[((n+2)%numPoints)<<1];
		int py2 = data[1+(((n+2)%numPoints)<<1)];
		if(px0&0x80) px0 -= 0x100;
		if(py0&0x80) py0 -= 0x100;
		if(px1&0x80) px1 -= 0x100;
		if(py1&0x80) py1 -= 0x100;
		if(px2&0x80) px2 -= 0x100;
		if(py2&0x80) py2 -= 0x100;
		for(int m=0; m<8; m++) {
			int offset = ((n<<3)+m)<<1;
			int c0 = (8-m)*(8-m);
			int c2 = m*m;
			int c1 = 128-c0-c2;
			tempBuf[offset] = ((px0*c0)+(px1*c1)+(px2*c2))>>7;
			tempBuf[offset+1] = ((py0*c0)+(py1*c1)+(py2*c2))>>7;
		}
	}
	drawSpriteHDMAPolygon(s,tempBuf,coneBase,numPoints<<3,ux,uy,offsX,offsY);
	free(tempBuf);
}

//Helper function for finding GFX files
inline int findSpGfxFile(BYTE file) {
	for(int i=5; i>=0; i--) {
		if(spGfxFiles[i]==file) return 0x100+(i<<5);
	}
	return 0;
}

//SuperFX texture displayer function
void dispSuperFXTexture(DWORD * pixelBuf,int width,int height,BYTE props,WORD tile,POINT offs,bool inv) {
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
				if(inv) invertPixel(pixelBuf,width,height,{dx,dy});
			}
		}
	}
}
//HDMA displayer function
void dispBackgroundRow(DWORD * pixelBuf,int width,int height,int row,POINT offs,bool inv) {
	int offsX = offs.x;
	int offsY = offs.y;
	int base = ((row&0x3FF)<<10)|((row&0x400)>>1);
	for(int i=0; i<0x100; i++) {
		DWORD color = bmpDataBg[base+i];
		if(color!=0x80808080) {
			putPixel(pixelBuf,width,height,color,{offsX+i,offsY});
			if(inv) invertPixel(pixelBuf,width,height,{offsX+i,offsY});
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
	addSpriteTile(s,(0xE<<2)|0x41,base+0x08,-16,2);
	addSpriteTile(s,(0xE<<2)|1,base+0x08,16,2);
	addSpriteTile(s,(0xE<<2)|0x41,base+0x00,-8,-8);
	addSpriteTile(s,(0xE<<2)|1,base+0x00,8,-8);
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
	addSpriteTile(s,(0xA<<2),base+0x0F,0,12);
	addSpriteTile(s,(0x8<<2)|1,base+0x01,0,1);
	addSpriteTile(s,(0x8<<2)|1,base+0x00,-8,1);
	addSpriteTile(s,(0xA<<2),base+0x0F,2,12);
	addSpriteTile(s,(0x8<<2),base+0x03,0,-7);
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
	drawSpriteHDMAPolygon(s,&romBuf[0x036407],0x8400,20,false,false,0,0);
	int base = findSpGfxFile(0x42);
	addSpriteTile(s,(0x9<<2)|1,base+0x20,-24,-73);
	addSpriteTile(s,(0x9<<2)|1,base+0x22,-8,-73);
	addSpriteTile(s,(0x9<<2)|1,base+0x02,-8,-89);
	addSpriteTile(s,(0x9<<2)|1,base+0x00,-24,-89);
	addSpriteTile(s,(0x9<<2),base+0x3B,-18,-98);
	addSpriteTile(s,(0x9<<2),base+0x3B,-8,-98);
	addSpriteTile(s,(0x9<<2)|1,base+0x24,-24,-107);
	addSpriteTile(s,(0x9<<2)|1,base+0x26,-11,-107);
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
//Shy Guy
void drawSprite_01E(sprite_t * s) {
	int offset = ((s->data[2]&1)|(s->data[1]&2))<<1;
	int pal = ((romBuf[0x0209B8+offset]-1)<<1)|0x20;
	addSpriteTile(s,pal|1,0x0088,0,-3);
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
	addSpriteTile(s,(0x9<<2)|1,0x0102,0,-4);
	addSpriteTile(s,(0x9<<2),0x009E,4,8);
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
	drawSpriteHDMAPolygon(s,&romBuf[0x010E7D],0x8000,66,false,false,0,0);
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
		addSpriteTile(s,(0x8<<2)|1,0x012C,63,-3);
		addSpriteTile(s,(0x8<<2),0x010B,73,-8);
		addSpriteTile(s,(0x8<<2),0x010C,72,8);
		addSpriteTile(s,(0x8<<2)|1,0x012C,127,-3);
		addSpriteTile(s,(0x8<<2),0x010B,137,-8);
		addSpriteTile(s,(0x8<<2),0x010C,136,8);
	}
}
//Fake falling wall
void drawSprite_036(sprite_t * s) {
	for(int j=-96; j<16; j++) {
		addSpriteTile(s,0,0x855F,-0x50,j);
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
	for(int j=0; j<48; j++) {
		addSpriteTile(s,0,0x84D0,-0x70,j);
	}
}
//Mini Raven
void drawSprite_03A(sprite_t * s) {
	int base = findSpGfxFile(0x4F);
	addSpriteTile(s,(0x9<<2)|1,base+0x00,-2,-2);
	addSpriteTile(s,(0x9<<2),base+0x12,3,-1);
	addSpriteTile(s,(0x9<<2),base+0x02,-2,3);
	addSpriteTile(s,(0x9<<2),base+0x16,11,-5);
	addSpriteTile(s,(0x9<<2),base+0x05,0,13);
	if(s->data[0]==0x3A) {
		addSpriteTile(s,(0x9<<2)|1,base+0x00,-18,-2);
		addSpriteTile(s,(0x9<<2),base+0x12,-13,-1);
		addSpriteTile(s,(0x9<<2),base+0x02,-18,3);
		addSpriteTile(s,(0x9<<2),base+0x16,-5,-5);
		addSpriteTile(s,(0x9<<2),base+0x05,-16,13);
		addSpriteTile(s,(0x9<<2)|0x40,base+0x05,-8,13);
		addSpriteTile(s,(0x9<<2)|0x40,base+0x05,8,13);
		addSpriteTile(s,(0x9<<2)|1,base+0x00,14,-2);
		addSpriteTile(s,(0x9<<2),base+0x12,19,-1);
		addSpriteTile(s,(0x9<<2),base+0x02,14,3);
		addSpriteTile(s,(0x9<<2),base+0x16,27,-5);
		addSpriteTile(s,(0x9<<2),base+0x05,16,13);
		addSpriteTile(s,(0x9<<2)|0x40,base+0x05,24,13);
	}
}
//Tap-Tap the Red Nose
void drawSprite_03C(sprite_t * s) {
	for(int j=0; j<4; j++) {
		for(int i=0; i<4; i++) {
			int offsX = -24+(i<<4);
			int offsY = -32+(j<<4);
			addSpriteTile(s,(0xE<<2),0x450C+i+(j<<4),offsX,offsY);
		}
	}
	addSpriteTile(s,(0xE<<2),0x4548,-14,-24);
	addSpriteTile(s,(0xE<<2),0x4549,2,-24);
	addSpriteTile(s,(0xE<<2),0x4558,-14,-8);
	addSpriteTile(s,(0xE<<2),0x4559,2,-8);
	addSpriteTile(s,(0xE<<2),0x456A,-25,10);
	addSpriteTile(s,(0xE<<2),0x456B,-9,10);
	addSpriteTile(s,(0xE<<2),0x457A,-25,26);
	addSpriteTile(s,(0xE<<2),0x457B,-9,26);
	addSpriteTile(s,(0xE<<2),0x456A,9,10);
	addSpriteTile(s,(0xE<<2),0x456B,25,10);
	addSpriteTile(s,(0xE<<2),0x457A,9,26);
	addSpriteTile(s,(0xE<<2),0x457B,25,26);
	addSpriteTile(s,(0xE<<2)|1,0x010E,-21,-17);
}
//Seesaw
void drawSprite_03D(sprite_t * s) {
	drawSpriteHDMAPolygon(s,&romBuf[0x023330],0x8400,16,false,false,0x80,0x10);
}
//Skinny platform
void drawSprite_03E(sprite_t * s) {
	addSpriteTile(s,(0x9<<2),0x0066,-32,0);
	addSpriteTile(s,(0x9<<2),0x0067,-24,0);
	addSpriteTile(s,(0x9<<2),0x0067,-16,0);
	addSpriteTile(s,(0x9<<2),0x0067,-8,0);
	addSpriteTile(s,(0x9<<2),0x0067,0,0);
	addSpriteTile(s,(0x9<<2),0x0067,8,0);
	addSpriteTile(s,(0x9<<2),0x0067,16,0);
	addSpriteTile(s,(0x9<<2)|0x40,0x0066,24,0);
}
//Slime
void drawSprite_03F(sprite_t * s) {
	drawSpriteHDMAPolygon(s,&romBuf[0x030D80],0x8400,64,false,false,0,0);
	addSpriteTile(s,(0x8<<2),0x012D,-40,-56);
	addSpriteTile(s,(0x8<<2),0x012D,-32,-56);
	addSpriteTile(s,(0x8<<2),0x013D,-40,-48);
	addSpriteTile(s,(0x8<<2),0x013D,-32,-48);
}
//Baby Luigi
void drawSprite_040(sprite_t * s) {
	addSpriteTile(s,(0xE<<2),0x40AA,-8,-10);
	addSpriteTile(s,(0xE<<2)|1,0x01A4,0,-14);
}
//Stork
void drawSprite_041(sprite_t * s) {
	addSpriteTile(s,(0xE<<2)|0x40,0x019D,-27,72);
	addSpriteTile(s,(0xE<<2)|0x41,0x018B,-19,64);
	addSpriteTile(s,(0xE<<2)|0x41,0x018E,-24,80);
	addSpriteTile(s,(0xE<<2)|1,0x0187,-8,55);
	addSpriteTile(s,(0xE<<2)|1,0x0189,-8,40);
	addSpriteTile(s,(0xE<<2)|1,0x014C,-12,71);
	addSpriteTile(s,(0xE<<2)|1,0x018B,3,64);
	addSpriteTile(s,(0xE<<2)|1,0x018E,8,80);
	addSpriteTile(s,(0xE<<2),0x019D,21,72);
	addSpriteTile(s,(0xE<<2)|0x80,0x0190,-20,80);
	addSpriteTile(s,(0xE<<2),0x018D,-4,16);
	addSpriteTile(s,(0xE<<2),0x018D,-4,24);
	addSpriteTile(s,(0xE<<2),0x018D,-4,32);
	addSpriteTile(s,(0xE<<2)|1,0x014A,-8,0);
}
//Vertical pipe entrance
void drawSprite_042(sprite_t * s) {
	drawSpriteText(s,"Vertical Pipe\n  Exit Down  ");
}
//Giant Shy-Guy
void drawSprite_043(sprite_t * s) {
	int spRef = (s->data[0]-0x43)<<2;
	int pal = (0x9<<2)-spRef;
	int base = findSpGfxFile(0x70);
	addSpriteTile(s,pal|1,base+0x00,-8,-10);
	addSpriteTile(s,pal|1,base+0x02,8,-10);
	addSpriteTile(s,pal|1,base+0x04,-8,6);
	addSpriteTile(s,pal|1,base+0x06,8,6);
	addSpriteTile(s,pal|0x40,base+0x1A,-9,12);
	addSpriteTile(s,pal|0x40,base+0x0A,-11,8);
	addSpriteTile(s,pal,base+0x1A,16,12);
	addSpriteTile(s,pal,base+0x0A,18,8);
}
//Prince Froggy
void drawSprite_045(sprite_t * s) {
	int offsY = (s->data[1]&2);
	if(s->data[2]&1) {
		int base = findSpGfxFile(0x68);
		if(offsY) {
			addSpriteTile(s,(0x9<<2)|0x40,base+0x0C,43,29);
			addSpriteTile(s,(0x9<<2)|0xC0,base+0x19,22,9);
			addSpriteTile(s,(0x9<<2)|0xC0,base+0x19,31,7);
			addSpriteTile(s,(0x9<<2)|0x40,base+0x03,48,26);
			addSpriteTile(s,(0x9<<2)|0x80,base+0x19,41,12);
			addSpriteTile(s,(0x9<<2),base+0x09,36,12);
			addSpriteTile(s,(0x9<<2)|0xC0,base+0x04,40,25);
			addSpriteTile(s,(0x9<<2)|0xC0,base+0x05,32,22);
			addSpriteTile(s,(0x9<<2)|0xC0,base+0x12,32,15);
			addSpriteTile(s,(0x9<<2),base+0x16,41,18);
			addSpriteTile(s,(0x9<<2)|0xC0,base+0x14,40,16);
			addSpriteTile(s,(0x9<<2),base+0x09,26,13);
			addSpriteTile(s,(0x9<<2),base+0x0C,36,27);
			addSpriteTile(s,(0x9<<2)|0xC0,base+0x19,31,14);
			addSpriteTile(s,(0x9<<2),base+0x0D,46,16);
		} else {
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
	} else {
		drawSpriteHDMASpline(s,&romBuf[0x014774],0x8000,16,false,false,0,0);
		if(offsY) {
			int base = findSpGfxFile(0x70);
			addSpriteTile(s,(0xF<<2),base+0x0B,-8,-79);
			addSpriteTile(s,(0xF<<2)|0x40,base+0x0B,0,-79);
			addSpriteTile(s,(0xF<<2),0x4664,-16,-75);
			addSpriteTile(s,(0xF<<2),0x4665,0,-75);
			addSpriteTile(s,(0xF<<2),0x4674,-16,-59);
			addSpriteTile(s,(0xF<<2),0x4675,0,-59);
		}
	}
}
//Burt the Bashful
void drawSprite_046(sprite_t * s) {
	addSpriteTile(s,(0xE<<2),0x4263,17,-122);
	for(int j=0; j<64; j++) {
		int row = romBuf[0x050A51+j];
		addSpriteTile(s,0,0x8200|row,-0x80,j-0x78);
	}
	addSpriteTile(s,(0xE<<2),0x4264,0,-108);
	addSpriteTile(s,(0xE<<2),0x4265,16,-108);
	addSpriteTile(s,(0xE<<2),0x4274,0,-92);
	addSpriteTile(s,(0xE<<2),0x4275,16,-92);
	addSpriteTile(s,(0xE<<2),0x4273,17,-64);
	addSpriteTile(s,(0xF<<2)|0x40,0x4273,33,-64);
	addSpriteTile(s,(0xE<<2),0x4272,17,-48);
	addSpriteTile(s,(0xF<<2)|0x40,0x4272,33,-48);
	for(int j=0; j<64; j++) {
		int row = romBuf[0x050A90+j];
		addSpriteTile(s,0,0x8100|row,-0x80,j-0x38);
	}
	addSpriteTile(s,(0xE<<2),0x4262,-18,-122);
	addSpriteTile(s,(0xE<<2),0x4262,-3,-122);
	addSpriteTile(s,(0xE<<2),0x4260,-65,-118);
	addSpriteTile(s,(0xE<<2),0x4261,-49,-118);
	addSpriteTile(s,(0xE<<2),0x4270,-65,-102);
	addSpriteTile(s,(0xE<<2),0x4271,-49,-102);
	addSpriteTile(s,(0xF<<2)|0xC0,0x4260,-25,-102);
	addSpriteTile(s,(0xF<<2)|0xC0,0x4261,-41,-102);
	addSpriteTile(s,(0xF<<2)|0xC0,0x4270,-25,-118);
	addSpriteTile(s,(0xF<<2)|0xC0,0x4271,-41,-118);
	addSpriteTile(s,(0xE<<2)|0x80,0x4260,-56,8);
	addSpriteTile(s,(0xE<<2)|0x80,0x4261,-40,8);
	addSpriteTile(s,(0xE<<2)|0x80,0x4270,-56,-8);
	addSpriteTile(s,(0xE<<2)|0x80,0x4271,-40,-8);
	addSpriteTile(s,(0xF<<2)|0x40,0x4260,-16,-8);
	addSpriteTile(s,(0xF<<2)|0x40,0x4261,-32,-8);
	addSpriteTile(s,(0xF<<2)|0x40,0x4270,-16,8);
	addSpriteTile(s,(0xF<<2)|0x40,0x4271,-32,8);
	addSpriteTile(s,(0xE<<2)|0xC0,0x4260,40,8);
	addSpriteTile(s,(0xE<<2)|0xC0,0x4261,24,8);
	addSpriteTile(s,(0xE<<2)|0xC0,0x4270,40,-8);
	addSpriteTile(s,(0xE<<2)|0xC0,0x4271,24,-8);
	addSpriteTile(s,(0xF<<2),0x4260,0,-8);
	addSpriteTile(s,(0xF<<2),0x4261,16,-8);
	addSpriteTile(s,(0xF<<2),0x4270,0,8);
	addSpriteTile(s,(0xF<<2),0x4271,16,8);
}
//Shy Guy from Roger the Potted Ghost
void drawSprite_047(sprite_t * s) {
	addSpriteTile(s,(0x8<<2)|1,0x012C,-1,-3);
	addSpriteTile(s,(0x8<<2),0x010B,9,-8);
	addSpriteTile(s,(0x8<<2),0x010C,8,8);
}
//Kamek cutscene
void drawSprite_048(sprite_t * s) {
	int base = findSpGfxFile(0x6A);
	addSpriteTile(s,(0x9<<2)|1,base+0x0E,16,4);
	addSpriteTile(s,(0x9<<2),base+0x1A,9,10);
	addSpriteTile(s,(0x9<<2)|1,base+0x00,-9,-8);
	addSpriteTile(s,(0x9<<2)|1,base+0x02,-13,0);
	addSpriteTile(s,(0x9<<2)|1,base+0x04,3,0);
	addSpriteTile(s,(0x9<<2)|0x40,base+0x10,5,-8);
	addSpriteTile(s,(0x9<<2),base+0x0A,-5,8);
}
//Fire from Thunder Lakitu
void drawSprite_049(sprite_t * s) {
	addSpriteTile(s,(0xA<<2)|0x41,0x01AE,0,0);
	addSpriteTile(s,(0xA<<2)|0x41,0x01AC,0,-16);
}
//Upside-down Blow Hard
void drawSprite_04C(sprite_t * s) {
	int base = findSpGfxFile(0x36);
	addSpriteTile(s,(8<<2)|0x81,base+0x00,-8,0);
	addSpriteTile(s,(8<<2)|0x81,base+0x02,8,0);
	addSpriteTile(s,(8<<2)|0x80,0x4122,-16,20);
	addSpriteTile(s,(8<<2)|0x80,0x4123,0,20);
	addSpriteTile(s,(8<<2)|0x80,0x4132,-16,4);
	addSpriteTile(s,(8<<2)|0x80,0x4133,0,4);
}
//Unused sprite
void drawSprite_04D(sprite_t * s) {
	addSpriteTile(s,(0x8<<2)|0x41,0x0182,-8,-8);
	addSpriteTile(s,(0x8<<2)|0x41,0x0180,8,-8);
	addSpriteTile(s,(0x8<<2)|0x41,0x01A2,-8,8);
	addSpriteTile(s,(0x8<<2)|0x41,0x01A0,8,8);
}
//Locked door
void drawSprite_04E(sprite_t * s) {
	addSpriteTile(s,(0x8<<2),0x4160,-8,0);
	addSpriteTile(s,(0x8<<2)|0x40,0x4160,8,0);
	addSpriteTile(s,(0x8<<2),0x4170,-8,16);
	addSpriteTile(s,(0x8<<2)|0x40,0x4170,8,16);
}
//Middle Ring
void drawSprite_04F(sprite_t * s) {
	addSpriteTile(s,(0xB<<2),0x0055,8,39);
	addSpriteTile(s,(0xB<<2),0x0045,1,35);
	addSpriteTile(s,(0xB<<2),0x0054,-5,27);
	addSpriteTile(s,(0xB<<2),0x0044,-8,14);
	addSpriteTile(s,(0xB<<2),0x0054,-8,0);
	addSpriteTile(s,(0xB<<2),0x0045,-5,-13);
	addSpriteTile(s,(0xB<<2),0x0055,0,-22);
	addSpriteTile(s,(0xB<<2),0x0045,8,-25);
	addSpriteTile(s,(0xB<<2),0x0054,14,-22);
	addSpriteTile(s,(0xB<<2),0x0044,20,-14);
	addSpriteTile(s,(0xB<<2),0x0054,23,-1);
	addSpriteTile(s,(0xB<<2),0x0045,23,14);
	addSpriteTile(s,(0xB<<2),0x0055,20,26);
	addSpriteTile(s,(0xB<<2),0x0045,15,35);
}
//Wide plank
void drawSprite_050(sprite_t * s) {
	for(int j=0; j<16; j++) {
		addSpriteTile(s,0,0xA45A,-0x58,j);
	}
}
//Octagonal log
void drawSprite_051(sprite_t * s) {
	int x = 95<<16;
	for(int j=-55; j<-45; j++) {
		int width = x>>16;
		x += (16<<16)/10;
		addSpriteTile(s,0,0xA400+width,-0x58,j+0x10);
	}
	x = 111<<16;
	for(int j=-45; j<0; j++) {
		int width = x>>16;
		x += (9<<16)/45;
		addSpriteTile(s,0,0xA500+width,-0x58,j+0x10);
	}
	x = 120<<16;
	for(int j=0; j<45; j++) {
		int width = x>>16;
		x -= (9<<16)/45;
		addSpriteTile(s,0,0xA480+width,-0x58,j+0x10);
	}
	x = 111<<16;
	for(int j=45; j<55; j++) {
		int width = x>>16;
		x -= (16<<16)/10;
		addSpriteTile(s,0,0xA580+width,-0x58,j+0x10);
	}
}
//Balloon platform, randomly colored
void drawSprite_052(sprite_t * s) {
	addSpriteTile(s,(0x8<<2),0x4504,-8,-8);
	addSpriteTile(s,(0x8<<2),0x4505,8,-8);
	addSpriteTile(s,(0x8<<2),0x4514,-8,8);
	addSpriteTile(s,(0x8<<2),0x4515,8,8);
}
//Upside-down Wild Piranha
void drawSprite_054(sprite_t * s) {
	int base = findSpGfxFile(0x29);
	addSpriteTile(s,(0x9<<2)|0x80,base+0x1A,0,0);
	addSpriteTile(s,(0x9<<2)|0xC0,base+0x1A,8,0);
	addSpriteTile(s,(0x9<<2)|0x80,base+0x0A,4,8);
	addSpriteTile(s,(0x9<<2)|0x80,0x442A,-23,20);
	addSpriteTile(s,(0x9<<2)|0x80,0x442B,-7,20);
	addSpriteTile(s,(0x9<<2)|0x80,0x443A,-23,4);
	addSpriteTile(s,(0x9<<2)|0x80,0x443B,-7,4);
}
//Flatbed Ferry pinwheel, green
void drawSprite_055(sprite_t * s) {
	addSpriteTile(s,(0x8<<2),0x00BD,4,-4);
	addSpriteTile(s,(0x8<<2),0x00BD,8,-16);
	addSpriteTile(s,(0x8<<2),0x00BD,12,-28);
	addSpriteTile(s,(0x8<<2),0x00BD,16,0);
	addSpriteTile(s,(0x8<<2),0x00BD,28,4);
	addSpriteTile(s,(0x8<<2),0x00BD,0,8);
	addSpriteTile(s,(0x8<<2),0x00BD,-4,20);
	addSpriteTile(s,(0x8<<2),0x00BD,-8,-8);
	addSpriteTile(s,(0x8<<2),0x00BD,-20,-12);
	addSpriteTile(s,(0x8<<2)|1,0x0028,7,-45);
	addSpriteTile(s,(0x8<<2)|1,0x002A,23,-45);
	addSpriteTile(s,(0x8<<2)|1,0x0028,29,7);
	addSpriteTile(s,(0x8<<2)|1,0x002A,45,7);
	addSpriteTile(s,(0x8<<2)|1,0x0028,-23,29);
	addSpriteTile(s,(0x8<<2)|1,0x002A,-7,29);
	addSpriteTile(s,(0x8<<2)|1,0x0028,-45,-23);
	addSpriteTile(s,(0x8<<2)|1,0x002A,-29,-23);
}
//Flatbed Ferry pinwheel, pink
void drawSprite_056(sprite_t * s) {
	addSpriteTile(s,(0x8<<2),0x00BD,4,-4);
	addSpriteTile(s,(0x8<<2),0x00BD,8,-16);
	addSpriteTile(s,(0x8<<2),0x00BD,16,0);
	addSpriteTile(s,(0x8<<2),0x00BD,0,8);
	addSpriteTile(s,(0x8<<2),0x00BD,-8,-8);
	addSpriteTile(s,(0xC<<2)|1,0x0028,5,-30);
	addSpriteTile(s,(0xC<<2)|1,0x002A,13,-30);
	addSpriteTile(s,(0xC<<2)|1,0x0028,18,1);
	addSpriteTile(s,(0xC<<2)|1,0x002A,26,1);
	addSpriteTile(s,(0xC<<2)|1,0x0028,-13,14);
	addSpriteTile(s,(0xC<<2)|1,0x002A,-5,14);
	addSpriteTile(s,(0xC<<2)|1,0x0028,-26,-17);
	addSpriteTile(s,(0xC<<2)|1,0x002A,-18,-17);
}
//Roger Lift
void drawSprite_057(sprite_t * s) {
	drawSpriteHDMAPolygon(s,&romBuf[0x0378BB],0x8400,50,true,true,-0x80,-0xD8);
	addSpriteTile(s,(0x8<<2)|1,0x0028,76,-232);
	addSpriteTile(s,(0x8<<2)|1,0x0029,92,-232);
	addSpriteTile(s,(0x8<<2)|1,0x002A,108,-232);
	addSpriteTile(s,(0x8<<2)|0x40,0x010D,90,-224);
	addSpriteTile(s,(0x8<<2)|0x40,0x010D,106,-224);
	addSpriteTile(s,(0x8<<2)|0x40,0x011D,90,-216);
	addSpriteTile(s,(0x8<<2)|0x40,0x011D,106,-216);
}
//Solo Toady
void drawSprite_058(sprite_t * s) {
	int spRef = (s->data[0]-0x58)<<2;
	int pal = (0x8<<2)+spRef;
	addSpriteTile(s,pal,0x002C,0,-6);
	addSpriteTile(s,pal|0x40,0x002C,8,-6);
	addSpriteTile(s,pal|0x40,0x002F,6,14);
	addSpriteTile(s,pal|0x40,0x002F,1,14);
	addSpriteTile(s,pal|1,0x000E,0,0);
}
//Super Star
void drawSprite_059(sprite_t * s) {
	addSpriteTile(s,(0xA<<2),0x416F,0,0);
}
//Raphael Raven projectile
void drawSprite_05A(sprite_t * s) {
	int base = findSpGfxFile(0x1B);
	addSpriteTile(s,(9<<2)|1,base+0x20,-8,-8);
	addSpriteTile(s,(9<<2)|1,base+0x22,8,-8);
	addSpriteTile(s,(9<<2)|1,base+0x24,-8,8);
	addSpriteTile(s,(9<<2)|0x41,base+0x24,8,8);
}
//Coin Bandit
void drawSprite_05B(sprite_t * s) {
	int base = findSpGfxFile(0x4E);
	addSpriteTile(s,(0xC<<2),base+0x0A,0,5);
	addSpriteTile(s,(0xC<<2),base+0x0B,8,5);
	addSpriteTile(s,(0xC<<2),base+0x1E,11,8);
	addSpriteTile(s,(0xC<<2),base+0x1E,0,8);
	addSpriteTile(s,(0xC<<2)|1,base+0x08,0,-10);
	addSpriteTile(s,(0x9<<2)|1,0x00A0,0,-26);
}
//Short plank
void drawSprite_05E(sprite_t * s) {
	for(int j=0; j<16; j++) {
		addSpriteTile(s,0,0xA434,-0x78,j);
	}
}
//Bomb
void drawSprite_060(sprite_t * s) {
	addSpriteTile(s,(0xC<<2)|1,0x00EC,0,0);
}
//Baby Mario
void drawSprite_061(sprite_t * s) {
	addSpriteTile(s,(0xD<<2)|1,0x0064,0,3);
	addSpriteTile(s,(0xD<<2)|1,0x0062,1,-6);
}
//Goomba
void drawSprite_062(sprite_t * s) {
	int base = findSpGfxFile(0x26);
	addSpriteTile(s,(0xC<<2)|1,base+0x0C,0,-1);
	addSpriteTile(s,(0x9<<2),base+0x0E,2,12);
	addSpriteTile(s,(0x9<<2)|0x40,base+0x0E,7,12);
	addSpriteTile(s,(0xC<<2),base+0x1F,4,-6);
}
//Muddy Buddy
void drawSprite_063(sprite_t * s) {
	addSpriteTile(s,(0xC<<2),0x009E,8,9);
	addSpriteTile(s,(0xC<<2),0x009E,0,9);
	addSpriteTile(s,(0xC<<2),0x4040,0,-1);
}
//Flatbed Ferry pinwheel, pink
void drawSprite_064(sprite_t * s) {
	if(s->data[1]&2) {
		addSpriteTile(s,(0x8<<2),0x00BD,4,-4);
		addSpriteTile(s,(0x8<<2),0x00BD,8,-16);
		addSpriteTile(s,(0x8<<2),0x00BD,16,0);
		addSpriteTile(s,(0x8<<2),0x00BD,0,8);
		addSpriteTile(s,(0x8<<2),0x00BD,-8,-8);
		addSpriteTile(s,(0xC<<2)|1,0x0028,5,-30);
		addSpriteTile(s,(0xC<<2)|1,0x002A,13,-30);
		addSpriteTile(s,(0xC<<2)|1,0x0028,18,1);
		addSpriteTile(s,(0xC<<2)|1,0x002A,26,1);
		addSpriteTile(s,(0xC<<2)|1,0x0028,-13,14);
		addSpriteTile(s,(0xC<<2)|1,0x002A,-5,14);
		addSpriteTile(s,(0xC<<2)|1,0x0028,-26,-17);
		addSpriteTile(s,(0xC<<2)|1,0x002A,-18,-17);
	} else {
		addSpriteTile(s,(0x8<<2),0x00BD,4,-4);
		addSpriteTile(s,(0x8<<2),0x00BD,8,-16);
		addSpriteTile(s,(0x8<<2),0x00BD,12,-28);
		addSpriteTile(s,(0x8<<2),0x00BD,16,0);
		addSpriteTile(s,(0x8<<2),0x00BD,28,4);
		addSpriteTile(s,(0x8<<2),0x00BD,0,8);
		addSpriteTile(s,(0x8<<2),0x00BD,-4,20);
		addSpriteTile(s,(0x8<<2),0x00BD,-8,-8);
		addSpriteTile(s,(0x8<<2),0x00BD,-20,-12);
		addSpriteTile(s,(0xC<<2)|1,0x0028,7,-45);
		addSpriteTile(s,(0xC<<2)|1,0x002A,23,-45);
		addSpriteTile(s,(0xC<<2)|1,0x0028,29,7);
		addSpriteTile(s,(0xC<<2)|1,0x002A,45,7);
		addSpriteTile(s,(0xC<<2)|1,0x0028,-23,29);
		addSpriteTile(s,(0xC<<2)|1,0x002A,-7,29);
		addSpriteTile(s,(0xC<<2)|1,0x0028,-45,-23);
		addSpriteTile(s,(0xC<<2)|1,0x002A,-29,-23);
	}
}
//Red coin
void drawSprite_065(sprite_t * s) {
	addSpriteTile(s,(0x9<<2)|1,0x00A0,0,0);
}
//Wild Piranha
void drawSprite_066(sprite_t * s) {
	int base = findSpGfxFile(0x29);
	addSpriteTile(s,(0x9<<2),base+0x1A,0,8);
	addSpriteTile(s,(0x9<<2)|0x40,base+0x1A,8,8);
	addSpriteTile(s,(0x9<<2),base+0x0A,4,0);
	addSpriteTile(s,(0x9<<2),0x442A,-23,-20);
	addSpriteTile(s,(0x9<<2),0x442B,-7,-20);
	addSpriteTile(s,(0x9<<2),0x443A,-23,-4);
	addSpriteTile(s,(0x9<<2),0x443B,-7,-4);
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
//Hit green egg block
void drawSprite_06B(sprite_t * s) {
	addSpriteTile(s,(0x8<<2)|1,0x4074,0,0);
}
//Large spring ball
void drawSprite_06C(sprite_t * s) {
	addSpriteTile(s,(0x8<<2),0x414E,-8,-8);
	addSpriteTile(s,(0x8<<2),0x414F,8,-8);
	addSpriteTile(s,(0x8<<2),0x415E,-8,8);
	addSpriteTile(s,(0x8<<2),0x415F,8,8);
}
//Propeller Piranha
void drawSprite_06D(sprite_t * s) {
	addSpriteTile(s,(0xA<<2),0x4446,-8,-8);
	addSpriteTile(s,(0xA<<2),0x4447,8,-8);
	addSpriteTile(s,(0xA<<2),0x4456,-8,8);
	addSpriteTile(s,(0xA<<2),0x4457,8,8);
}
//Spring ball
void drawSprite_06F(sprite_t * s) {
	addSpriteTile(s,(0x8<<2)|1,0x00C2,0,0);
}
//Clawdaddy
void drawSprite_070(sprite_t * s) {
	int base = findSpGfxFile(0x35);
	addSpriteTile(s,(0x9<<2)|1,base+0x08,-8,-16);
	addSpriteTile(s,(0x9<<2)|1,base+0x0A,8,-16);
	addSpriteTile(s,(0x9<<2)|1,base+0x0C,-8,0);
	addSpriteTile(s,(0x9<<2)|1,base+0x0E,8,0);
}
//Boo
void drawSprite_071(sprite_t * s) {
	int base = findSpGfxFile(0x61);
	if(s->data[2]&1) {
		if(s->data[1]&2) {
			addSpriteTile(s,(0x9<<2)|1,base+0x26,0,0);
		} else {
			addSpriteTile(s,(0x9<<2)|1,base+0x00,-8,-8);
			addSpriteTile(s,(0x9<<2)|1,base+0x02,8,-8);
			addSpriteTile(s,(0x9<<2)|1,base+0x08,-8,8);
			addSpriteTile(s,(0x9<<2)|1,base+0x0A,8,8);
		}
	} else {
		addSpriteTile(s,(0x9<<2)|1,base+0x00,-8,-8);
		addSpriteTile(s,(0x9<<2)|1,base+0x02,8,-8);
		addSpriteTile(s,(0x9<<2)|1,base+0x08,-8,8);
		addSpriteTile(s,(0x9<<2)|1,base+0x0A,8,8);
		addSpriteTile(s,(0x9<<2)|1,base+0x26,24,0);
		addSpriteTile(s,(0x9<<2)|1,base+0x26,40,0);
		addSpriteTile(s,(0x9<<2)|1,base+0x26,56,0);
	}
}
//Train Bandit
void drawSprite_072(sprite_t * s) {
	int base = findSpGfxFile(0x2E);
	addSpriteTile(s,(0xC<<2)|1,base+0x08,0,0);
}
//Pump with large balloon platform
void drawSprite_073(sprite_t * s) {
	addSpriteTile(s,(0x9<<2),0x4084,-8,-22);
	addSpriteTile(s,(0x9<<2),0x4085,8,-22);
	addSpriteTile(s,(0x9<<2),0x4094,-8,-6);
	addSpriteTile(s,(0x9<<2),0x4095,8,-6);
	addSpriteTile(s,(0x8<<2),0x003E,-8,8);
	addSpriteTile(s,(0x8<<2),0x003F,-0,8);
	addSpriteTile(s,(0x8<<2)|0x40,0x003F,8,8);
	addSpriteTile(s,(0x8<<2)|0x40,0x003E,16,8);
	addSpriteTile(s,(0x8<<2),0x003E,-8,-28);
	addSpriteTile(s,(0x8<<2),0x003F,-0,-28);
	addSpriteTile(s,(0x8<<2)|0x40,0x003F,8,-28);
	addSpriteTile(s,(0x8<<2)|0x40,0x003E,16,-28);
	int base = findSpGfxFile(0x65);
	addSpriteTile(s,(9<<2)|1,base+0x0E,14,6);
	BYTE tempBuf[44*2];
	for(int n=0; n<88; n++) {
		tempBuf[n] = romBuf[0x066E64+(n<<1)];
	}
	drawSpriteHDMAPolygon(s,tempBuf,0x8400,44,true,false,0x1C,0x0C);
}
//Spike
void drawSprite_074(sprite_t * s) {
	int base = findSpGfxFile(0x26);
	addSpriteTile(s,(0xA<<2)|1,base+0x00,0,0);
}
//Spike projectile
void drawSprite_075(sprite_t * s) {
	addSpriteTile(s,(0xA<<2),0x4060,0,0);
}
//Piro Dangle
void drawSprite_076(sprite_t * s) {
	int base = findSpGfxFile(0x45);
	addSpriteTile(s,(0x9<<2),0x4063,0,0);
	addSpriteTile(s,(0x9<<2),base+0x02,0,4);
	addSpriteTile(s,(0x9<<2)|0x40,base+0x02,8,4);
}
//Bullet Bill blaster
void drawSprite_078(sprite_t * s) {
	int pal = ((romBuf[0x02D1D1+((s->data[0]-0x78)<<1)]&0xE)<<1)|0x20;
	int base = findSpGfxFile(0x31);
	addSpriteTile(s,pal|1,base+0x02,0,-8);
	addSpriteTile(s,pal|1,base+0x00,0,0);
}
//Bullet Bill
void drawSprite_07B(sprite_t * s) {
	int pal = ((romBuf[0x02D1D1+((s->data[0]-0x7B)<<1)]&0xE)<<1)|0x20;
	addSpriteTile(s,pal,0x4082,-8,-8);
	addSpriteTile(s,pal,0x4083,8,-8);
	addSpriteTile(s,pal,0x4092,-8,8);
	addSpriteTile(s,pal,0x4093,8,8);
}
//Squishy block dent
void drawSprite_07E(sprite_t * s) {
	addSpriteTile(s,(0x2<<2),0x40A3,-16,0);
	addSpriteTile(s,(0x2<<2),0x40A4,0,0);
	addSpriteTile(s,(0x2<<2),0x40A5,16,0);
}
//Log seesaw
void drawSprite_07F(sprite_t * s) {
	addSpriteTile(s,(0xC<<2),0x4046,18,-8);
	addSpriteTile(s,(0xC<<2),0x4047,34,-8);
	addSpriteTile(s,(0xC<<2),0x4056,18,8);
	addSpriteTile(s,(0xC<<2),0x4057,34,8);
	addSpriteTile(s,(0xC<<2),0x4046,-8,-8);
	addSpriteTile(s,(0xC<<2),0x4047,8,-8);
	addSpriteTile(s,(0xC<<2),0x4056,-8,8);
	addSpriteTile(s,(0xC<<2),0x4057,8,8);
	addSpriteTile(s,(0xC<<2),0x4046,-34,-8);
	addSpriteTile(s,(0xC<<2),0x4047,-18,-8);
	addSpriteTile(s,(0xC<<2),0x4056,-34,8);
	addSpriteTile(s,(0xC<<2),0x4057,-18,8);
}
//Lava Bubble
void drawSprite_080(sprite_t * s) {
	addSpriteTile(s,(0x9<<2),0x408B,0,0);
}
//Chain Chomp
void drawSprite_082(sprite_t * s) {
	addSpriteTile(s,(0x8<<2),0x00BD,4,8);
	addSpriteTile(s,(0x8<<2),0x00BD,-4,8);
	addSpriteTile(s,(0x8<<2),0x00BD,-12,8);
	addSpriteTile(s,(0x8<<2),0x00BD,-20,8);
	addSpriteTile(s,(0x8<<2),0x4099,-36,0);
}
//Ambient cloud for final boss
void drawSprite_083(sprite_t * s) {
	addSpriteTile(s,(0xF<<2)|1,0x012C,-16,0);
	addSpriteTile(s,(0xF<<2)|1,0x0142,0,8);
	addSpriteTile(s,(0xF<<2)|0xC1,0x0142,0,8);
	addSpriteTile(s,(0xF<<2)|0xC1,0x012C,16,0);
}
//Teleport screen exit
void drawSprite_084(sprite_t * s) {
	drawSpriteText(s,"Teleport\n  Exit  ");
}
//Harry Hedgehog
void drawSprite_085(sprite_t * s) {
	int base = findSpGfxFile(0x1E);
	addSpriteTile(s,(0x8<<2)|1,base+0x0A,0,0);
}
//Red Egg with 1-UP
void drawSprite_087(sprite_t * s) {
	addSpriteTile(s,(0x9<<2)|1,0x0082,0,0);
}
//Flatbed Ferry, red
void drawSprite_089(sprite_t * s) {
	addSpriteTile(s,(0x9<<2)|1,0x0028,-16,0);
	addSpriteTile(s,(0x9<<2)|1,0x0029,0,0);
	addSpriteTile(s,(0x9<<2)|1,0x002A,16,0);
}
//Flatbed Ferry, pink
void drawSprite_08A(sprite_t * s) {
	addSpriteTile(s,(0xC<<2)|1,0x0028,-16,0);
	addSpriteTile(s,(0xC<<2)|1,0x0029,0,0);
	addSpriteTile(s,(0xC<<2)|1,0x002A,16,0);
}
//Mock Up
void drawSprite_08B(sprite_t * s) {
	int pal = (s->data[2]&1);
	pal = (pal+0x8)<<2;
	addSpriteTile(s,pal,0x44C4,-8,-8);
	addSpriteTile(s,pal,0x44C5,8,-8);
	addSpriteTile(s,pal,0x44D4,-8,8);
	addSpriteTile(s,pal,0x44D5,8,8);
}
//Yoshi at goal
void drawSprite_08C(sprite_t * s) {
	addSpriteTile(s,(0xF<<2),0x4168,-8,-8);
	addSpriteTile(s,(0xF<<2),0x4169,8,-8);
	addSpriteTile(s,(0xF<<2),0x4178,-8,8);
	addSpriteTile(s,(0xF<<2),0x4179,8,8);
}
//Fly Guy
void drawSprite_08D(sprite_t * s) {
	if(s->data[1]&2) {
		addSpriteTile(s,(0xB<<2)|1,0x00E3,0,17);
		addSpriteTile(s,(0x9<<2),0x0049,-2,22);
		addSpriteTile(s,(0x9<<2),0x006E,0,22);
		addSpriteTile(s,(0x9<<2),0x006F,8,22);
	} else if(s->data[2]&1) {
		addSpriteTile(s,(0x9<<2)|1,0x00A0,0,16);
	} else {
		addSpriteTile(s,(0x8<<2)|1,0x00EA,0,16);
	}
	addSpriteTile(s,(0x9<<2),0x002C,0,-6);
	addSpriteTile(s,(0x9<<2)|0x40,0x002C,8,-6);
	addSpriteTile(s,(0x9<<2)|0x40,0x002F,6,14);
	addSpriteTile(s,(0x9<<2)|0x40,0x002F,1,14);
	addSpriteTile(s,(0x9<<2)|1,0x0088,0,0);
}
//Kamek in Bowser's Room
void drawSprite_08E(sprite_t * s) {
	int base = findSpGfxFile(0x67);
	addSpriteTile(s,(0x9<<2)|1,base+0x04,-1,3);
	addSpriteTile(s,(0x9<<2)|0x80,base+0x1F,-9,8);
	addSpriteTile(s,(0x9<<2)|1,base+0x02,-1,4);
	addSpriteTile(s,(0x9<<2)|1,base+0x04,8,3);
	addSpriteTile(s,(0x9<<2)|1,base+0x00,0,-10);
}
//Grinder, swing
void drawSprite_08F(sprite_t * s) {
	addSpriteTile(s,(0x8<<2),0x40C6,-78,-8);
	addSpriteTile(s,(0x8<<2),0x40C7,-62,-8);
	addSpriteTile(s,(0x8<<2),0x40D6,-78,8);
	addSpriteTile(s,(0x8<<2),0x40D7,-62,8);
	addSpriteTile(s,(0x8<<2),0x40C6,-58,-8);
	addSpriteTile(s,(0x8<<2),0x40C7,-42,-8);
	addSpriteTile(s,(0x8<<2),0x40D6,-58,8);
	addSpriteTile(s,(0x8<<2),0x40D7,-42,8);
	addSpriteTile(s,(0x8<<2),0x40C6,-38,-8);
	addSpriteTile(s,(0x8<<2),0x40C7,-22,-8);
	addSpriteTile(s,(0x8<<2),0x40D6,-38,8);
	addSpriteTile(s,(0x8<<2),0x40D7,-22,8);
	addSpriteTile(s,(0x8<<2),0x40C6,-18,-8);
	addSpriteTile(s,(0x8<<2),0x40C7,-2,-8);
	addSpriteTile(s,(0x8<<2),0x40D6,-18,8);
	addSpriteTile(s,(0x8<<2),0x40D7,-2,8);
}
//Dangling Ghost
void drawSprite_090(sprite_t * s) {
	BYTE tempBuf[18*2];
	memcpy(tempBuf,&romBuf[0x0352E3],36);
	tempBuf[9] += 0x10;
	tempBuf[11] += 0x20;
	tempBuf[13] += 0x30;
	tempBuf[15] += 0x40;
	tempBuf[17] += 0x40;
	tempBuf[19] += 0x40;
	tempBuf[21] += 0x40;
	tempBuf[23] += 0x40;
	tempBuf[25] += 0x30;
	tempBuf[27] += 0x20;
	tempBuf[29] += 0x10;
	drawSpriteHDMASpline(s,tempBuf,0x8400,18,false,false,0,0);
	int base = findSpGfxFile(0x42);
	addSpriteTile(s,(0x8<<2),base+0x0A,-18,80);
	addSpriteTile(s,(0x8<<2),base+0x0A,-10,80);
	addSpriteTile(s,(0x8<<2)|1,base+0x24,-24,73);
	addSpriteTile(s,(0x8<<2)|1,base+0x26,-12,73);
}
//Group of 4 Toadies
void drawSprite_091(sprite_t * s) {
	addSpriteTile(s,(0x9<<2)|1,0x000E,-10,0);
	addSpriteTile(s,(0x9<<2),0x002C,-10,-8);
	addSpriteTile(s,(0x9<<2)|0x40,0x002C,-2,-8);
	addSpriteTile(s,(0x9<<2)|0x41,0x000E,10,0);
	addSpriteTile(s,(0x9<<2),0x002C,10,-8);
	addSpriteTile(s,(0x9<<2)|0x40,0x002C,18,-8);
	addSpriteTile(s,(0x9<<2)|1,0x000E,-6,8);
	addSpriteTile(s,(0x9<<2),0x002C,-6,0);
	addSpriteTile(s,(0x9<<2)|0x40,0x002C,2,0);
	addSpriteTile(s,(0x9<<2)|0x41,0x000E,6,8);
	addSpriteTile(s,(0x9<<2),0x002C,6,0);
	addSpriteTile(s,(0x9<<2)|0x40,0x002C,14,0);
}
//Melon Bug
void drawSprite_092(sprite_t * s) {
	int base = findSpGfxFile(0x1E);
	addSpriteTile(s,(0x9<<2)|1,base+0x00,0,-8);
	addSpriteTile(s,(0x9<<2),base+0x04,0,8);
	addSpriteTile(s,(0x9<<2),base+0x05,8,8);
}
//Door
void drawSprite_093(sprite_t * s) {
	addSpriteTile(s,(0x8<<2),0x440F,-8,-16);
	addSpriteTile(s,(0x8<<2)|0x40,0x440F,8,-16);
	addSpriteTile(s,(0x8<<2),0x441F,-8,0);
	addSpriteTile(s,(0x8<<2)|0x40,0x441F,8,0);
}
//Expansion Block
void drawSprite_094(sprite_t * s) {
	addSpriteTile(s,(0xC<<2),0x4074,0,0);
}
//Checkered platform
void drawSprite_095(sprite_t * s) {
	int spRef = (s->data[0]-0x95)<<4;
	spRef |= (0x8<<2)|1;
	addSpriteTile(s,spRef,0x409B,0,0);
}
//POW block
void drawSprite_097(sprite_t * s) {
	addSpriteTile(s,(0x9<<2),0x40EA,-8,-16);
	addSpriteTile(s,(0x9<<2),0x40EB,8,-16);
	addSpriteTile(s,(0x9<<2),0x40FA,-8,0);
	addSpriteTile(s,(0x9<<2),0x40FB,8,0);
}
//Yoshi block
void drawSprite_098(sprite_t * s) {
	addSpriteTile(s,(0xD<<2),0x40EC,-8,-16);
	addSpriteTile(s,(0xD<<2),0x40ED,8,-16);
	addSpriteTile(s,(0xD<<2),0x40FC,-8,0);
	addSpriteTile(s,(0xD<<2),0x40FD,8,0);
}
//Wall Lakitu projectile
void drawSprite_099(sprite_t * s) {
	int base = findSpGfxFile(0x4F);
	addSpriteTile(s,(0xA<<2)|1,base+0x0E,0,0);
}
//Flatbed Ferry, green chained
void drawSprite_09A(sprite_t * s) {
	addSpriteTile(s,(0x8<<2)|1,0x00C0,0,0);
	addSpriteTile(s,(0x8<<2),0x00BD,-14,-9);
	addSpriteTile(s,(0x8<<2),0x00BD,-31,-22);
	addSpriteTile(s,(0x8<<2),0x00BD,-49,-35);
	addSpriteTile(s,(0x8<<2)|1,0x0028,-79,-48);
	addSpriteTile(s,(0x8<<2)|1,0x002A,-63,-48);
}
//Mace Guy
void drawSprite_09B(sprite_t * s) {
	int base = findSpGfxFile(0x59);
	addSpriteTile(s,(0x9<<2)|1,base+0x0C,0,0);
	addSpriteTile(s,(0x8<<2),0x00BD,0,2);
	addSpriteTile(s,(0x8<<2),0x00BD,-8,-3);
	addSpriteTile(s,(0x8<<2),0x00BD,-16,-8);
	addSpriteTile(s,(0xA<<2),0x4060,-32,-17);
}
//Mace Guy projectile
void drawSprite_09C(sprite_t * s) {
	addSpriteTile(s,(0x8<<2),0x00BD,0,2);
	addSpriteTile(s,(0x8<<2),0x00BD,-8,-3);
	addSpriteTile(s,(0x8<<2),0x00BD,-16,-8);
	addSpriteTile(s,(0xA<<2),0x4060,-32,-17);
}
//!-switch
void drawSprite_09D(sprite_t * s) {
	addSpriteTile(s,(0x9<<2),0x440C,-8,-16);
	addSpriteTile(s,(0x9<<2),0x440D,8,-16);
	addSpriteTile(s,(0x9<<2),0x441C,-8,0);
	addSpriteTile(s,(0x9<<2),0x441D,8,0);
}
//Chomp Rock
void drawSprite_09E(sprite_t * s) {
	addSpriteTile(s,(0xA<<2),0x4162,-8,-16);
	addSpriteTile(s,(0xA<<2),0x4163,8,-16);
	addSpriteTile(s,(0xA<<2),0x4172,-8,0);
	addSpriteTile(s,(0xA<<2),0x4173,8,0);
}
//Wild Ptooie Piranha
void drawSprite_09F(sprite_t * s) {
	int base = findSpGfxFile(0x29);
	addSpriteTile(s,(0x9<<2),base+0x1A,0,8);
	addSpriteTile(s,(0x9<<2)|0x40,base+0x1A,8,8);
	addSpriteTile(s,(0x9<<2),base+0x0A,4,0);
	addSpriteTile(s,(0x8<<2),0x442A,-23,-20);
	addSpriteTile(s,(0x8<<2),0x442B,-7,-20);
	addSpriteTile(s,(0x8<<2),0x443A,-23,-4);
	addSpriteTile(s,(0x8<<2),0x443B,-7,-4);
}
//Tulip
void drawSprite_0A0(sprite_t * s) {
	addSpriteTile(s,(0x9<<2),0x4506,-8,-8);
	addSpriteTile(s,(0x9<<2),0x4516,-8,8);
	addSpriteTile(s,(0x9<<2)|0x40,0x4506,8,-8);
	addSpriteTile(s,(0x9<<2)|0x40,0x4516,8,8);
	addSpriteTile(s,(0x9<<2),0x4503,0,-8);
	addSpriteTile(s,(0x9<<2),0x4513,0,8);
}
//Pot for Potted Spiked Fun Guy
void drawSprite_0A1(sprite_t * s) {
	int base = findSpGfxFile(0x36);
	addSpriteTile(s,(0xC<<2)|1,base+0x0E,0,0);
}
//Thunder Lakitu projectile
void drawSprite_0A2(sprite_t * s) {
	int base = findSpGfxFile(0x4B);
	addSpriteTile(s,(0xA<<2)|0x40,base+0x19,0,0);
	addSpriteTile(s,(0xA<<2),base+0x19,8,0);
	addSpriteTile(s,(0xA<<2)|0xC0,base+0x19,0,8);
	addSpriteTile(s,(0xA<<2)|0x80,base+0x19,8,8);
}
//Nep-Enut/Gargantua Blargg
void drawSprite_0A5(sprite_t * s) {
	int base = findSpGfxFile(0x53);
	addSpriteTile(s,(0x8<<2)|0x41,base+0x00,-11,-100);
	addSpriteTile(s,(0x8<<2)|0x41,base+0x00,26,-113);
	drawSpriteHDMAPolygon(s,&romBuf[0x0146E4],0x8400,62,false,false,8,-8);
	addSpriteTile(s,(0x8<<2)|0x41,base+0x0C,10,-142);
	addSpriteTile(s,(0x8<<2)|0x81,base+0x0C,10,-126);
	addSpriteTile(s,(0x8<<2)|0x41,base+0x0C,-10,-142);
	addSpriteTile(s,(0x8<<2)|0x81,base+0x0C,-10,-126);
	addSpriteTile(s,(0x8<<2)|0x41,base+0x08,-8,-152);
	addSpriteTile(s,(0x8<<2)|1,base+0x08,8,-152);
	addSpriteTile(s,(0x8<<2)|0x41,base+0x0A,-17,-162);
	addSpriteTile(s,(0x8<<2)|1,base+0x0A,16,-161);
	addSpriteTile(s,(0x8<<2)|0x40,base+0x07,14,-139);
	addSpriteTile(s,(0x8<<2)|0x40,base+0x07,-6,-139);
}
//Incoming Chomp, in background
void drawSprite_0A6(sprite_t * s) {
	int base = findSpGfxFile(0x1B);
	addSpriteTile(s,(0xE<<2)|1,base+0x05,0,0);
}
//Incoming Chomp, flock in background
void drawSprite_0A7(sprite_t * s) {
	int base = findSpGfxFile(0x1B);
	addSpriteTile(s,(0xE<<2)|1,base+0x05,-8,-8);
	addSpriteTile(s,(0xE<<2)|1,base+0x05,8,-8);
	addSpriteTile(s,(0xE<<2)|1,base+0x05,-16,0);
	addSpriteTile(s,(0xE<<2)|1,base+0x05,0,0);
	addSpriteTile(s,(0xE<<2)|1,base+0x05,16,0);
}
//Falling Incoming Chomp
void drawSprite_0A8(sprite_t * s) {
	addSpriteTile(s,(0x9<<2),0x40C2,-8,-8);
	addSpriteTile(s,(0x9<<2)|0x40,0x40C2,8,-8);
	addSpriteTile(s,(0x9<<2),0x40D2,-8,8);
	addSpriteTile(s,(0x9<<2)|0x40,0x40D2,8,8);
}
//Incoming Chomp shadow
void drawSprite_0A9(sprite_t * s) {
	int base = findSpGfxFile(0x1B);
	addSpriteTile(s,(0xD<<2)|1,base+0x00,-24,1);
	addSpriteTile(s,(0xD<<2)|1,base+0x02,-8,1);
	addSpriteTile(s,(0xD<<2)|0x41,base+0x02,8,1);
	addSpriteTile(s,(0xD<<2)|0x41,base+0x00,24,1);
}
//Shy Guy in background
void drawSprite_0AA(sprite_t * s) {
	int base = findSpGfxFile(0x1B);
	addSpriteTile(s,(0xE<<2),base+0x04,4,0);
}
//Fill Eggs
void drawSprite_0AB(sprite_t * s) {
	addSpriteTile(s,(0x8<<2)|1,0x0082,0,0);
}
//Signal arrow from final boss
void drawSprite_0AC(sprite_t * s) {
	addSpriteTile(s,(0x9<<2),0x00C4,11,-32);
	addSpriteTile(s,(0x9<<2)|0x40,0x00C4,13,-32);
	addSpriteTile(s,(0x9<<2)|0xC0,0x00C5,8,-24);
	addSpriteTile(s,(0x9<<2)|0x80,0x00C5,16,-24);
}
//Message Block
void drawSprite_0AD(sprite_t * s) {
	addSpriteTile(s,(0x9<<2),0x4161,0,0);
}
//Hookbill Koopa
void drawSprite_0AE(sprite_t * s) {
	addSpriteTile(s,(0xF<<2)|1,0x010E,25,11);
	addSpriteTile(s,(0xF<<2)|1,0x010C,17,19);
	addSpriteTile(s,(0xF<<2),0x011E,33,27);
	addSpriteTile(s,(0xF<<2),0x0120,17,11);
	addSpriteTile(s,(0xF<<2),0x0120,-18,18);
	addSpriteTile(s,(0xF<<2)|1,0x0120,-18,18);
	addSpriteTile(s,(0xF<<2),0x012C,-26,26);
	addSpriteTile(s,(0xF<<2)|1,0x0122,-18,10);
	addSpriteTile(s,(0xE<<2),0x44EC,-45,-7);
	addSpriteTile(s,(0xE<<2),0x44ED,-29,-7);
	addSpriteTile(s,(0xE<<2),0x44FC,-45,9);
	addSpriteTile(s,(0xE<<2),0x44FD,-29,9);
	addSpriteTile(s,(0xE<<2),0x44EA,32,1);
	addSpriteTile(s,(0xE<<2),0x0124,-8,11);
	addSpriteTile(s,(0xE<<2)|1,0x0124,-8,11);
	addSpriteTile(s,(0xE<<2)|1,0x0126,0,19);
	addSpriteTile(s,(0xE<<2),0x0135,-8,27);
	addSpriteTile(s,(0xE<<2),0x44E8,-1,6);
	addSpriteTile(s,(0xE<<2),0x44E9,15,6);
	addSpriteTile(s,(0xE<<2),0x44F8,-1,22);
	addSpriteTile(s,(0xE<<2),0x44F9,15,22);
}
//Morph Bubble, Car
void drawSprite_0AF(sprite_t * s) {
	addSpriteTile(s,(0xD<<2),0x4566,0,0);
	addSpriteTile(s,(0x8<<2)|1,0x009C,-7,-7);
	addSpriteTile(s,(0x8<<2)|1,0x007E,7,-7);
	addSpriteTile(s,(0x8<<2)|0xC1,0x007E,-7,7);
	addSpriteTile(s,(0x8<<2)|0x81,0x007E,7,7);
}
//Morph Bubble, Mole Tank
void drawSprite_0B0(sprite_t * s) {
	addSpriteTile(s,(0xD<<2),0x4567,0,0);
	addSpriteTile(s,(0x8<<2)|1,0x009C,-7,-7);
	addSpriteTile(s,(0x8<<2)|1,0x007E,7,-7);
	addSpriteTile(s,(0x8<<2)|0xC1,0x007E,-7,7);
	addSpriteTile(s,(0x8<<2)|0x81,0x007E,7,7);
}
//Morph Bubble, Helicopter
void drawSprite_0B1(sprite_t * s) {
	addSpriteTile(s,(0xD<<2),0x4576,0,0);
	addSpriteTile(s,(0x8<<2)|1,0x009C,-7,-7);
	addSpriteTile(s,(0x8<<2)|1,0x007E,7,-7);
	addSpriteTile(s,(0x8<<2)|0xC1,0x007E,-7,7);
	addSpriteTile(s,(0x8<<2)|0x81,0x007E,7,7);
}
//Morph Bubble, Train
void drawSprite_0B2(sprite_t * s) {
	addSpriteTile(s,(0xD<<2),0x4577,0,0);
	addSpriteTile(s,(0x8<<2)|1,0x009C,-7,-7);
	addSpriteTile(s,(0x8<<2)|1,0x007E,7,-7);
	addSpriteTile(s,(0x8<<2)|0xC1,0x007E,-7,7);
	addSpriteTile(s,(0x8<<2)|0x81,0x007E,7,7);
}
//Fuzzy cloud
void drawSprite_0B3(sprite_t * s) {
	addSpriteTile(s,(0xB<<2)|1,0x00E3,0,0);
}
//Morph Bubble, Submarine
void drawSprite_0B4(sprite_t * s) {
	addSpriteTile(s,(0xD<<2),0x417F,0,0);
	addSpriteTile(s,(0x8<<2)|1,0x009C,-7,-7);
	addSpriteTile(s,(0x8<<2)|1,0x007E,7,-7);
	addSpriteTile(s,(0x8<<2)|0xC1,0x007E,-7,7);
	addSpriteTile(s,(0x8<<2)|0x81,0x007E,7,7);
}
//Baron Von Zeppelin, Giant Egg
void drawSprite_0CD(sprite_t * s) {
	addSpriteTile(s,(0x8<<2)|1,0x00A8,0,-32);
	addSpriteTile(s,(0x8<<2),0x00BC,4,-18);
	addSpriteTile(s,(0x8<<2),0x00BC,4,-14);
	addSpriteTile(s,(0x8<<2)|1,0x0182,4,-8);
	addSpriteTile(s,(0x8<<2)|1,0x0180,-4,0);
	addSpriteTile(s,(0x8<<2),0x0192,12,8);
	addSpriteTile(s,(0x8<<2),0x0184,-4,-8);
}
//Baby Bowser projectile
void drawSprite_0CE(sprite_t * s) {
	addSpriteTile(s,(0x9<<2),0x408C,-8,-24);
	addSpriteTile(s,(0x9<<2)|0x40,0x408C,8,-24);
	addSpriteTile(s,(0x9<<2)|0x80,0x408C,-8,-8);
	addSpriteTile(s,(0x9<<2)|0xC0,0x408C,8,-8);
}
//Baby Bowser quake
void drawSprite_0CF(sprite_t * s) {
	drawSpriteText(s,"Baby Bowser\n   Quake   ");
}
//Horizontal pipe exit right
void drawSprite_0D0(sprite_t * s) {
	drawSpriteText(s,"Horizontal  Pipe\n   Exit Right   ");
}
//Hidden vertical pipe exit
void drawSprite_0D1(sprite_t * s) {
	drawSpriteText(s,"  Vertical  Pipe  \nExit Down (Hidden)");
}
//Marching Milde
void drawSprite_0D2(sprite_t * s) {
	for(int j=0; j<64; j++) {
		int row = romBuf[0x050A51+j];
		addSpriteTile(s,0,0x8100|row,-0x80,j-0x78);
		row = romBuf[0x050A90+j];
		addSpriteTile(s,0,0x8100|row,-0x80,j-0x38);
	}
	addSpriteTile(s,(0xF<<2),0x4264,0,-110);
	addSpriteTile(s,(0xF<<2),0x4265,16,-110);
	addSpriteTile(s,(0xF<<2),0x4274,0,-94);
	addSpriteTile(s,(0xF<<2),0x4275,16,-94);
	addSpriteTile(s,(0xF<<2),0x4268,-52,-112);
	addSpriteTile(s,(0xF<<2),0x4269,-36,-112);
	addSpriteTile(s,(0xF<<2),0x4278,-52,-96);
	addSpriteTile(s,(0xF<<2),0x4279,-36,-96);
	addSpriteTile(s,(0xE<<2)|0x40,0x4268,-11,-112);
	addSpriteTile(s,(0xE<<2)|0x40,0x4269,-27,-112);
	addSpriteTile(s,(0xE<<2)|0x40,0x4278,-11,-96);
	addSpriteTile(s,(0xE<<2)|0x40,0x4279,-27,-96);
	addSpriteTile(s,(0xF<<2)|0xC0,0x4266,13,2);
	addSpriteTile(s,(0xF<<2)|0xC0,0x4267,-3,2);
	addSpriteTile(s,(0xF<<2)|0xC0,0x4276,13,-14);
	addSpriteTile(s,(0xF<<2)|0xC0,0x4277,-3,-14);
	addSpriteTile(s,(0xE<<2),0x4266,-20,-13);
	addSpriteTile(s,(0xE<<2),0x4267,-4,-13);
	addSpriteTile(s,(0xE<<2),0x4276,-20,3);
	addSpriteTile(s,(0xE<<2),0x4277,-4,3);
}
//Marching Milde, 1/2
void drawSprite_0D3(sprite_t * s) {
	int base = findSpGfxFile(0x6F);
	addSpriteTile(s,(0xF<<2)|0x41,base+0x0D,10,20);
	addSpriteTile(s,(0xF<<2)|0x41,base+0x0E,-6,20);
	addSpriteTile(s,(0xE<<2),0x426A,-24,-26);
	addSpriteTile(s,(0xE<<2),0x426B,-8,-26);
	addSpriteTile(s,(0xE<<2),0x427A,-24,-10);
	addSpriteTile(s,(0xE<<2),0x427B,-8,-10);
	addSpriteTile(s,(0xE<<2),0x426C,-24,6);
	addSpriteTile(s,(0xE<<2),0x426D,-8,6);
	addSpriteTile(s,(0xE<<2),0x427C,-24,22);
	addSpriteTile(s,(0xE<<2),0x427D,-8,22);
	addSpriteTile(s,(0xE<<2)|0x40,0x426A,24,-26);
	addSpriteTile(s,(0xE<<2)|0x40,0x426B,8,-26);
	addSpriteTile(s,(0xE<<2)|0x40,0x427A,24,-10);
	addSpriteTile(s,(0xE<<2)|0x40,0x427B,8,-10);
	addSpriteTile(s,(0xE<<2)|0x40,0x426C,24,6);
	addSpriteTile(s,(0xE<<2)|0x40,0x426D,8,6);
	addSpriteTile(s,(0xE<<2)|0x40,0x427C,24,22);
	addSpriteTile(s,(0xE<<2)|0x40,0x427D,8,22);
	addSpriteTile(s,(0xF<<2)|1,base+0x00,-17,-19);
	addSpriteTile(s,(0xF<<2)|1,base+0x04,6,-16);
	addSpriteTile(s,(0xF<<2)|1,base+0x02,-5,-19);
	addSpriteTile(s,(0xE<<2)|1,base+0x0D,-10,20);
	addSpriteTile(s,(0xE<<2)|1,base+0x0E,-2,20);
}
//Marching Milde, 1/4
void drawSprite_0D4(sprite_t * s) {
	int base = findSpGfxFile(0x6D);
	addSpriteTile(s,(0xC<<2)|1,base+0x08,-8,-8);
	addSpriteTile(s,(0xC<<2)|1,base+0x0A,8,-8);
	addSpriteTile(s,(0xC<<2)|1,base+0x2A,-8,8);
	addSpriteTile(s,(0xC<<2)|1,base+0x2C,8,8);
}
//Ambient mountains from Hookbill boss
void drawSprite_0D5(sprite_t * s) {
	int base = findSpGfxFile(0x1A);
	addSpriteTile(s,(0xF<<2)|1,base+0x0E,232,40);
	addSpriteTile(s,(0xF<<2)|1,base+0x08,200,72);
	addSpriteTile(s,(0xF<<2)|1,base+0x08,184,16);
	addSpriteTile(s,(0xF<<2)|1,base+0x0C,144,104);
	addSpriteTile(s,(0xF<<2)|1,base+0x06,144,88);
	addSpriteTile(s,(0xF<<2)|1,base+0x02,144,72);
	addSpriteTile(s,(0xF<<2)|1,base+0x0E,136,48);
	addSpriteTile(s,(0xF<<2)|1,base+0x0A,128,104);
	addSpriteTile(s,(0xF<<2)|1,base+0x04,128,88);
	addSpriteTile(s,(0xF<<2)|1,base+0x00,128,72);
	addSpriteTile(s,(0xF<<2)|1,base+0x0E,96,24);
	addSpriteTile(s,(0xF<<2)|1,base+0x08,88,80);
	addSpriteTile(s,(0xF<<2)|1,base+0x0E,48,48);
	addSpriteTile(s,(0xF<<2)|1,base+0x0E,40,8);
	addSpriteTile(s,(0xF<<2)|1,base+0x08,32,40);
	addSpriteTile(s,(0xF<<2)|1,base+0x0C,16,56);
	addSpriteTile(s,(0xF<<2)|1,base+0x06,16,40);
	addSpriteTile(s,(0xF<<2)|1,base+0x02,16,24);
	addSpriteTile(s,(0xF<<2)|1,base+0x0E,8,88);
	addSpriteTile(s,(0xF<<2)|1,base+0x0A,0,56);
	addSpriteTile(s,(0xF<<2)|1,base+0x04,0,40);
	addSpriteTile(s,(0xF<<2)|1,base+0x00,0,24);
}
//Blarggwich
void drawSprite_0D6(sprite_t * s) {
	drawSpriteHDMAPolygon(s,&romBuf[0x0368FC],0x8400,16,false,false,0,0);
	addSpriteTile(s,(0x8<<2)|1,0x0028,-24,-137);
	addSpriteTile(s,(0x8<<2)|1,0x0029,-8,-137);
	addSpriteTile(s,(0x8<<2)|1,0x002A,8,-137);
	addSpriteTile(s,(0x8<<2)|1,0x010C,-18,-127);
	addSpriteTile(s,(0x8<<2)|0xC1,0x010C,-18,-111);
	addSpriteTile(s,(0x8<<2)|1,0x010C,2,-127);
	addSpriteTile(s,(0x8<<2)|0xC1,0x010C,2,-111);
	addSpriteTile(s,(0x8<<2)|1,0x0108,0,-137);
	addSpriteTile(s,(0x8<<2)|0x41,0x0108,-16,-137);
	addSpriteTile(s,(0x8<<2)|1,0x010A,9,-147);
	addSpriteTile(s,(0x8<<2)|0x41,0x010A,-24,-146);
	addSpriteTile(s,(0x8<<2),0x0107,-14,-124);
	addSpriteTile(s,(0x8<<2),0x0107,6,-124);
}
//Sluggy the Unshaven
void drawSprite_0D7(sprite_t * s) {
	BYTE tempBuf[16*2];
	memcpy(tempBuf,&romBuf[0x015109],32);
	for(int n=0; n<32; n+=2) {
		if(tempBuf[n+1]&0x80 && tempBuf[n+1]<0xB8) tempBuf[n+1] = 0xB8;
	}
	drawSpriteHDMASpline(s,tempBuf,0x8000,16,false,false,0x08,0x48);
	addSpriteTile(s,(0x8<<2),0x4678,-61,11);
	addSpriteTile(s,(0x8<<2)|0x80,0x4678,-61,27);
	addSpriteTile(s,(0x8<<2),0x4678,-27,11);
	addSpriteTile(s,(0x8<<2)|0x80,0x4678,-27,27);
	addSpriteTile(s,(0x8<<2),0x4679,-41,55);
	addSpriteTile(s,(0xF<<2),0x466A,18,17);
	addSpriteTile(s,(0xF<<2),0x466B,34,17);
	addSpriteTile(s,(0xF<<2),0x467A,18,33);
	addSpriteTile(s,(0xF<<2),0x467B,34,33);
	addSpriteTile(s,(0xF<<2)|0xC0,0x466A,34,65);
	addSpriteTile(s,(0xF<<2)|0xC0,0x466B,18,65);
	addSpriteTile(s,(0xF<<2)|0xC0,0x467A,34,49);
	addSpriteTile(s,(0xF<<2)|0xC0,0x467B,18,49);
	addSpriteTile(s,(0xF<<2),0x4669,19,69);
	addSpriteTile(s,(0xF<<2)|0xC0,0x4669,19,85);
	addSpriteTile(s,(0xF<<2),0x4668,44,87);
	addSpriteTile(s,(0x8<<2),0x467F,-104,-1);
	addSpriteTile(s,(0x8<<2)|0x40,0x466F,-88,-17);
	addSpriteTile(s,(0x8<<2)|0x40,0x466E,-72,-17);
	addSpriteTile(s,(0x8<<2)|0x40,0x467F,-88,-1);
	addSpriteTile(s,(0x8<<2)|0x40,0x467E,-72,-1);
	addSpriteTile(s,(0x8<<2)|0x40,0x467F,59,3);
	addSpriteTile(s,(0x8<<2),0x466E,29,-13);
	addSpriteTile(s,(0x8<<2),0x466F,45,-13);
	addSpriteTile(s,(0x8<<2),0x467E,29,3);
	addSpriteTile(s,(0x8<<2),0x467F,45,3);
	addSpriteTile(s,(0x8<<2)|0x40,0x467F,49,21);
	addSpriteTile(s,(0xF<<2),0x466C,-16,54);
	addSpriteTile(s,(0xF<<2),0x466D,0,54);
	addSpriteTile(s,(0xF<<2),0x467C,-16,70);
	addSpriteTile(s,(0xF<<2),0x467D,0,70);
}
//Chomp signboard
void drawSprite_0D8(sprite_t * s) {
	addSpriteTile(s,(0x9<<2),0x448C,-8,-8);
	addSpriteTile(s,(0x9<<2),0x448D,8,-8);
	addSpriteTile(s,(0x9<<2),0x449C,-8,8);
	addSpriteTile(s,(0x9<<2),0x449D,8,8);
}
//Fishin' Lakitu
void drawSprite_0D9(sprite_t * s) {
	int base = findSpGfxFile(0x49);
	addSpriteTile(s,(0x8<<2),base+0x1D,9,-3);
	addSpriteTile(s,(0xA<<2)|0x41,base+0x08,4,4);
	addSpriteTile(s,(0xA<<2)|1,base+0x08,-4,4);
	addSpriteTile(s,(0x9<<2),base+0x0D,3,-13);
	addSpriteTile(s,(0x9<<2)|1,base+0x04,-3,-6);
	addSpriteTile(s,(0x9<<2)|1,base+0x24,-10,-5);
	addSpriteTile(s,(0x9<<2)|1,base+0x26,-18,11);
}
//Flower pot
void drawSprite_0DA(sprite_t * s) {
	int base = findSpGfxFile(0x44);
	addSpriteTile(s,(0x9<<2)|1,base+0x02,-8,-22);
	addSpriteTile(s,(0x9<<2)|1,base+0x04,8,-22);
	addSpriteTile(s,(0x9<<2)|1,base+0x00,0,-38);
	addSpriteTile(s,(0x9<<2)|1,base+0x08,-4,0);
	addSpriteTile(s,(0x9<<2)|1,base+0x09,4,0);
	addSpriteTile(s,(0x9<<2),base+0x0D,-4,-8);
	addSpriteTile(s,(0x9<<2),base+0x0E,4,-8);
	addSpriteTile(s,(0x9<<2),base+0x0F,12,-8);
}
//Squishy block
void drawSprite_0DB(sprite_t * s) {
	BYTE tempBuf[32] = {0xE0,0xEC,0xE1,0xEA,0xE2,0xE9,0xE4,0xE8,
		0x1C,0xE8,0x1E,0xE9,0x1F,0xEA,0x20,0xEC,
		0x20,0x14,0x1F,0x16,0x1E,0x17,0x1C,0x18,
		0xE4,0x18,0xE2,0x17,0xE1,0x16,0xE0,0x14};
	drawSpriteHDMAPolygon(s,tempBuf,0x8400,16,false,false,0,8);
}
//Snowball
void drawSprite_0DC(sprite_t * s) {
	addSpriteTile(s,(0xB<<2),0x40E8,-8,-8);
	addSpriteTile(s,(0xB<<2),0x40E9,8,-8);
	addSpriteTile(s,(0xB<<2),0x40F8,-8,8);
	addSpriteTile(s,(0xB<<2),0x40F9,8,8);
}
//Boss closer, Naval Piranha
void drawSprite_0DD(sprite_t * s) {
	drawSpriteText(s," Boss  Closer \nNaval  Piranha");
}
//Falling rock from background
void drawSprite_0DE(sprite_t * s) {
	int base = findSpGfxFile(0x20);
	addSpriteTile(s,(0xE<<2)|1,base+0x00,-16,0);
	addSpriteTile(s,(0xE<<2)|1,base+0x02,0,0);
	addSpriteTile(s,(0xE<<2)|1,base+0x04,16,0);
}
//Piscatory Pete
void drawSprite_0DF(sprite_t * s) {
	int base = findSpGfxFile(0x3E);
	int pal = ((s->data[2]&1)^0xF)<<2;
	addSpriteTile(s,pal|1,base+0x08,0,0);
}
//Preying Mantas
void drawSprite_0E0(sprite_t * s) {
	int base = findSpGfxFile(0x3E);
	addSpriteTile(s,(0xE<<2)|1,base+0x00,0,0);
}
//Loch Nestor
void drawSprite_0E1(sprite_t * s) {
	int base = findSpGfxFile(0x3E);
	addSpriteTile(s,(0xF<<2)|1,base+0x0E,0,0);
}
//Boo Blah
void drawSprite_0E2(sprite_t * s) {
	int base = findSpGfxFile(0x22);
	if(s->data[2]&1) {
		addSpriteTile(s,(0x9<<2)|0x81,base+0x0C,-10,32);
		addSpriteTile(s,(0x9<<2)|0x81,base+0x0C,0,32);
		addSpriteTile(s,(0x9<<2)|0x81,base+0x09,-8,16);
		addSpriteTile(s,(0x9<<2)|0x81,base+0x0A,0,16);
		addSpriteTile(s,(0x9<<2)|0x81,base+0x28,-8,0);
		addSpriteTile(s,(0x9<<2)|0x81,base+0x29,0,0);
	} else {
		addSpriteTile(s,(0x9<<2)|1,base+0x0C,-10,-32);
		addSpriteTile(s,(0x9<<2)|1,base+0x0C,0,-32);
		addSpriteTile(s,(0x9<<2)|1,base+0x09,-8,-16);
		addSpriteTile(s,(0x9<<2)|1,base+0x0A,0,-16);
		addSpriteTile(s,(0x9<<2)|1,base+0x28,-8,0);
		addSpriteTile(s,(0x9<<2)|1,base+0x29,0,0);
	}
}
//Boo Blah with Piro Dangle
void drawSprite_0E3(sprite_t * s) {
	int base = findSpGfxFile(0x22);
	if(s->data[2]&1) {
		addSpriteTile(s,(0x9<<2)|0x81,base+0x0C,-10,32);
		addSpriteTile(s,(0x9<<2)|0x81,base+0x0C,0,32);
		addSpriteTile(s,(0x9<<2)|0x81,base+0x09,-8,16);
		addSpriteTile(s,(0x9<<2)|0x81,base+0x0A,0,16);
		addSpriteTile(s,(0x9<<2)|0x81,base+0x28,-8,0);
		addSpriteTile(s,(0x9<<2)|0x81,base+0x29,0,0);
		base = findSpGfxFile(0x45);
		addSpriteTile(s,(0x9<<2),0x4063,-4,43);
		addSpriteTile(s,(0x9<<2),base+0x02,-4,47);
		addSpriteTile(s,(0x9<<2)|0x40,base+0x02,4,47);
	} else {
		addSpriteTile(s,(0x9<<2)|1,base+0x0C,-10,-32);
		addSpriteTile(s,(0x9<<2)|1,base+0x0C,0,-32);
		addSpriteTile(s,(0x9<<2)|1,base+0x09,-8,-16);
		addSpriteTile(s,(0x9<<2)|1,base+0x0A,0,-16);
		addSpriteTile(s,(0x9<<2)|1,base+0x28,-8,0);
		addSpriteTile(s,(0x9<<2)|1,base+0x29,0,0);
		base = findSpGfxFile(0x45);
		addSpriteTile(s,(0x9<<2),0x4063,-4,-43);
		addSpriteTile(s,(0x9<<2),base+0x02,-4,-39);
		addSpriteTile(s,(0x9<<2)|0x40,base+0x02,4,-39);
	}
}
//Pokey
void drawSprite_0E4(sprite_t * s) {
	int base = findSpGfxFile(0x24);
	addSpriteTile(s,(0x8<<2)|1,base+0x0A,0,-16);
	addSpriteTile(s,(0x8<<2)|1,base+0x0C,0,0);
	addSpriteTile(s,(0x8<<2)|1,0x008A,0,-32);
}
//Needlenose, green
void drawSprite_0E5(sprite_t * s) {
	addSpriteTile(s,(0x8<<2)|1,0x008A,0,0);
}
//Gusty
void drawSprite_0E6(sprite_t * s) {
	int offs = (s->data[1]&2)|(s->data[2]&1);
	int base = findSpGfxFile(0x25);
	if(offs==1 || offs==2) {
		addSpriteTile(s,(0xA<<2)|0x41,base+0x06,8,-1);
		addSpriteTile(s,(0xA<<2)|0x40,base+0x09,-8,1);
		addSpriteTile(s,(0xA<<2)|0x40,base+0x19,-8,9);
		addSpriteTile(s,(0xA<<2)|0x40,base+0x08,0,0);
		addSpriteTile(s,(0xA<<2)|0x40,base+0x18,0,8);
	} else {
		addSpriteTile(s,(0xA<<2)|1,base+0x06,-8,-1);
		addSpriteTile(s,(0xA<<2),base+0x09,16,1);
		addSpriteTile(s,(0xA<<2),base+0x19,16,9);
		addSpriteTile(s,(0xA<<2),base+0x08,8,0);
		addSpriteTile(s,(0xA<<2),base+0x18,8,8);
	}
	if(offs&2) {
		addSpriteTile(s,(0xC<<2),0x0077,16,0);
	}
}
//Burt
void drawSprite_0E7(sprite_t * s) {
	if(s->data[2]&1) {
		addSpriteTile(s,(0x8<<2),0x448E,-8,-16);
		addSpriteTile(s,(0x8<<2),0x448F,8,-16);
		addSpriteTile(s,(0x8<<2),0x449E,-8,0);
		addSpriteTile(s,(0x8<<2),0x449F,8,0);
		addSpriteTile(s,(0x8<<2)|0x40,0x009E,0,8);
		addSpriteTile(s,(0x8<<2),0x009E,8,8);
	} else {
		addSpriteTile(s,(0x8<<2),0x448E,-8,-16);
		addSpriteTile(s,(0x8<<2),0x448F,8,-16);
		addSpriteTile(s,(0x8<<2),0x449E,-8,0);
		addSpriteTile(s,(0x8<<2),0x449F,8,0);
		addSpriteTile(s,(0x8<<2)|0x40,0x009E,0,8);
		addSpriteTile(s,(0x8<<2),0x009E,8,8);
		addSpriteTile(s,(0x8<<2),0x448E,24,-16);
		addSpriteTile(s,(0x8<<2),0x448F,40,-16);
		addSpriteTile(s,(0x8<<2),0x449E,24,0);
		addSpriteTile(s,(0x8<<2),0x449F,40,0);
		addSpriteTile(s,(0x8<<2)|0x40,0x009E,32,8);
		addSpriteTile(s,(0x8<<2),0x009E,40,8);
	}
}
//Goonie
void drawSprite_0E8(sprite_t * s) {
	int base = findSpGfxFile(0x2A);
	if(s->data[1]&2) {
		if(s->data[2]&1) {
			addSpriteTile(s,(0xA<<2)|0x41,base+0x04,-8,0);
			addSpriteTile(s,(0xA<<2)|1,base+0x22,16,0);
			addSpriteTile(s,(0xA<<2),base+0x24,32,0);
			addSpriteTile(s,(0xA<<2),base+0x34,8,4);
			addSpriteTile(s,(0xA<<2)|0x40,base+0x24,-31,0);
			addSpriteTile(s,(0xA<<2)|0x41,base+0x00,8,0);
			addSpriteTile(s,(0xA<<2)|0x40,base+0x34,-7,4);
			addSpriteTile(s,(0xA<<2)|0x41,base+0x22,-23,0);
			addSpriteTile(s,(0xC<<2),0x0077,8,0);
		} else {
			addSpriteTile(s,(0xA<<2)|1,base+0x04,8,0);
			addSpriteTile(s,(0xA<<2)|0x41,base+0x22,-16,0);
			addSpriteTile(s,(0xA<<2)|0x40,base+0x24,-24,0);
			addSpriteTile(s,(0xA<<2)|0x40,base+0x34,0,4);
			addSpriteTile(s,(0xA<<2),base+0x24,39,0);
			addSpriteTile(s,(0xA<<2)|1,base+0x00,-8,0);
			addSpriteTile(s,(0xA<<2),base+0x34,15,4);
			addSpriteTile(s,(0xA<<2)|1,base+0x22,23,0);
			addSpriteTile(s,(0xC<<2),0x0077,8,0);
		}
	} else {
		addSpriteTile(s,(0xA<<2)|1,base+0x04,8,0);
		addSpriteTile(s,(0xA<<2)|0x41,base+0x22,-16,0);
		addSpriteTile(s,(0xA<<2)|0x40,base+0x24,-24,0);
		addSpriteTile(s,(0xA<<2)|0x40,base+0x34,0,4);
		addSpriteTile(s,(0xA<<2),base+0x24,39,0);
		addSpriteTile(s,(0xA<<2)|1,base+0x00,-8,0);
		addSpriteTile(s,(0xA<<2),base+0x34,15,4);
		addSpriteTile(s,(0xA<<2)|1,base+0x22,23,0);
	}
}
//Group of 3 Flightless Goonies
void drawSprite_0E9(sprite_t * s) {
	int base = findSpGfxFile(0x2A);
	addSpriteTile(s,(0xA<<2)|1,base+0x00,-8,-2);
	addSpriteTile(s,(0xA<<2),base+0x04,8,-2);
	addSpriteTile(s,(0xA<<2)|1,base+0x08,3,6);
	addSpriteTile(s,(0xA<<2)|1,base+0x00,8,-2);
	addSpriteTile(s,(0xA<<2),base+0x04,24,-2);
	addSpriteTile(s,(0xA<<2)|1,base+0x08,19,6);
	addSpriteTile(s,(0xA<<2)|1,base+0x00,24,-2);
	addSpriteTile(s,(0xA<<2),base+0x04,40,-2);
	addSpriteTile(s,(0xA<<2)|1,base+0x08,35,6);
}
//Cloud Drop, vertical
void drawSprite_0EA(sprite_t * s) {
	int base = findSpGfxFile(0x2C);
	addSpriteTile(s,(0xA<<2)|1,base+0x00,0,0);
	addSpriteTile(s,(0xA<<2),base+0x0C,4,16);
	addSpriteTile(s,(0xA<<2),base+0x1C,3,1);
}
//Cloud Drop, horizontal
void drawSprite_0EB(sprite_t * s) {
	int base = findSpGfxFile(0x2D);
	addSpriteTile(s,(0xA<<2)|1,base+0x00,-4,0);
	addSpriteTile(s,(0xA<<2),base+0x1B,12,5);
}
//Flamer Guy
void drawSprite_0EC(sprite_t * s) {
	addSpriteTile(s,(0x9<<2),0x4144,-8,-20);
	addSpriteTile(s,(0x9<<2),0x4145,8,-20);
	addSpriteTile(s,(0x9<<2),0x4154,-8,-4);
	addSpriteTile(s,(0x9<<2),0x4155,8,-4);
	addSpriteTile(s,(0x9<<2)|1,0x01AE,-1,-5);
	addSpriteTile(s,(0x8<<2),0x009E,0,8);
	addSpriteTile(s,(0x8<<2),0x009E,8,8);
}
//Eggo-Dil
void drawSprite_0EE(sprite_t * s) {
	int base = findSpGfxFile(0x2F);
	addSpriteTile(s,(0x8<<2)|1,base+0x00,-8,0);
	addSpriteTile(s,(0x8<<2)|0x41,base+0x00,8,0);
	addSpriteTile(s,(0xA<<2)|1,base+0x08,0,-34);
	addSpriteTile(s,(0xA<<2)|1,base+0x08,-15,-23);
	addSpriteTile(s,(0xA<<2)|1,base+0x08,15,-23);
	addSpriteTile(s,(0xA<<2)|1,base+0x08,-10,-5);
	addSpriteTile(s,(0xA<<2)|1,base+0x08,9,-5);
	addSpriteTile(s,(0xC<<2),0x4106,-8,-30);
	addSpriteTile(s,(0xC<<2),0x4107,8,-30);
	addSpriteTile(s,(0xC<<2),0x4116,-8,-14);
	addSpriteTile(s,(0xC<<2),0x4117,8,-14);
}
//Eggo-Dil face
void drawSprite_0EF(sprite_t * s) {
	addSpriteTile(s,(0xC<<2),0x4106,-8,-8);
	addSpriteTile(s,(0xC<<2),0x4107,8,-8);
	addSpriteTile(s,(0xC<<2),0x4116,-8,8);
	addSpriteTile(s,(0xC<<2),0x4117,8,8);
}
//Eggo-Dil petal
void drawSprite_0F0(sprite_t * s) {
	int base = findSpGfxFile(0x2F);
	addSpriteTile(s,(0xA<<2)|1,base+0x08,0,0);
}
//Bubble-Plant
void drawSprite_0F1(sprite_t * s) {
	int base = findSpGfxFile(0x1F);
	addSpriteTile(s,(0x9<<2),base+0x1A,0,8);
	addSpriteTile(s,(0x9<<2),base+0x0A,8,8);
	addSpriteTile(s,(0xC<<2)|1,base+0x06,0,-7);
}
//Stilt Guy
void drawSprite_0F2(sprite_t * s) {
	int offset = ((s->data[2]&1)|(s->data[1]&2))<<1;
	int pal = ((romBuf[0x0209B8+offset]-1)<<1)|0x20;
	int base = findSpGfxFile(0x37);
	addSpriteTile(s,pal|1,base+0x0A,1,-8);
	addSpriteTile(s,pal,base+0x17,9,8);
	addSpriteTile(s,pal,base+0x06,0,-21);
	addSpriteTile(s,pal|1,base+0x00,3,-32);
	addSpriteTile(s,pal|1,base+0x0A,1,-23);
	addSpriteTile(s,pal|1,base+0x0A,1,-38);
	addSpriteTile(s,pal,base+0x06,12,-21);
	addSpriteTile(s,pal,base+0x07,-3,8);
}
//Woozy Guy
void drawSprite_0F3(sprite_t * s) {
	int offset = ((s->data[2]&1)|(s->data[1]&2))<<1;
	int pal = ((romBuf[0x0209B8+offset]-1)<<1)|0x20;
	addSpriteTile(s,pal,0x4072,0,0);
}
//Slugger
void drawSprite_0F5(sprite_t * s) {
	addSpriteTile(s,(0xA<<2)|1,0x0140,-2,-10);
	addSpriteTile(s,(0xA<<2),0x0154,-1,4);
	addSpriteTile(s,(0xA<<2),0x0155,7,4);
	addSpriteTile(s,(0xC<<2)|1,0x0148,6,-8);
	addSpriteTile(s,(0x8<<2),0x009E,4,8);
}
//Huffin' Puffin group
void drawSprite_0F6(sprite_t * s) {
	addSpriteTile(s,(0x9<<2),0x4120,-8,-19);
	addSpriteTile(s,(0x9<<2),0x4121,8,-19);
	addSpriteTile(s,(0x9<<2),0x4130,-8,-3);
	addSpriteTile(s,(0x9<<2),0x4131,8,-3);
	addSpriteTile(s,(0x9<<2),0x009E,0,8);
	addSpriteTile(s,(0x9<<2)|1,0x0102,24,-4);
	addSpriteTile(s,(0x9<<2),0x009E,28,8);
	addSpriteTile(s,(0x9<<2)|1,0x0102,40,-4);
	addSpriteTile(s,(0x9<<2),0x009E,44,8);
	addSpriteTile(s,(0x9<<2)|1,0x0102,56,-4);
	addSpriteTile(s,(0x9<<2),0x009E,60,8);
}
//Barney Bubble
void drawSprite_0F7(sprite_t * s) {
	addSpriteTile(s,(0x8<<2),0x009E,1,9);
	addSpriteTile(s,(0xC<<2)|1,0x014E,0,0);
	addSpriteTile(s,(0x8<<2),0x009E,10,9);
}
//Blow Hard
void drawSprite_0F8(sprite_t * s) {
	int base = findSpGfxFile(0x36);
	addSpriteTile(s,(8<<2)|1,base+0x00,-8,0);
	addSpriteTile(s,(8<<2)|1,base+0x02,8,0);
	addSpriteTile(s,(8<<2),0x4122,-16,-20);
	addSpriteTile(s,(8<<2),0x4123,0,-20);
	addSpriteTile(s,(8<<2),0x4132,-16,-4);
	addSpriteTile(s,(8<<2),0x4133,0,-4);
}
//Needlenose, yellow
void drawSprite_0F9(sprite_t * s) {
	addSpriteTile(s,(0xA<<2)|1,0x008A,0,0);
}
//Flower (SuperFX texture)
void drawSprite_0FA(sprite_t * s) {
	addSpriteTile(s,(0x9<<2),0x4042,-8,-8);
	addSpriteTile(s,(0x9<<2),0x4043,8,-8);
	addSpriteTile(s,(0x9<<2),0x4052,-8,8);
	addSpriteTile(s,(0x9<<2),0x4053,8,8);
}
//Spear Guy, long spear
void drawSprite_0FB(sprite_t * s) {
	int base = findSpGfxFile(0x35);
	addSpriteTile(s,(0x9<<2),base+0x05,2,10);
	addSpriteTile(s,(0x9<<2),base+0x05,5,12);
	addSpriteTile(s,(0x9<<2),base+0x17,-4,-5);
	addSpriteTile(s,(0x9<<2)|0x80,base+0x17,-4,3);
	addSpriteTile(s,(0x9<<2)|1,base+0x00,0,-4);
	addSpriteTile(s,(0x9<<2),base+0x04,10,-12);
	addSpriteTile(s,(0x9<<2),base+0x07,5,-12);
	addSpriteTile(s,(0x9<<2),base+0x16,7,-20);
	addSpriteTile(s,(0x9<<2),base+0x06,5,-28);
}
//Spear Guy, short spear
void drawSprite_0FC(sprite_t * s) {
	int base = findSpGfxFile(0x35);
	addSpriteTile(s,(0x9<<2),base+0x05,2,10);
	addSpriteTile(s,(0x9<<2),base+0x05,5,12);
	addSpriteTile(s,(0x9<<2),base+0x17,-4,-5);
	addSpriteTile(s,(0x9<<2)|0x80,base+0x17,-4,3);
	addSpriteTile(s,(0x9<<2)|1,base+0x00,0,-4);
	addSpriteTile(s,(0x9<<2),base+0x04,10,-12);
	addSpriteTile(s,(0x9<<2),base+0x16,7,-8);
	addSpriteTile(s,(0x9<<2),base+0x06,5,-16);
}
//Zeus Guy
void drawSprite_0FD(sprite_t * s) {
	addSpriteTile(s,(0xA<<2)|1,0x0124,2,2);
	addSpriteTile(s,(0x9<<2),0x0136,0,12);
	addSpriteTile(s,(0x9<<2),0x0136,11,12);
	addSpriteTile(s,(0x8<<2)|1,0x0120,0,-7);
	addSpriteTile(s,(0xA<<2),0x0126,-5,3);
	addSpriteTile(s,(0xA<<2),0x0126,5,4);
}
//Zeus Guy projectile
void drawSprite_0FE(sprite_t * s) {
	addSpriteTile(s,(0x9<<2)|1,0x0042,-8,-4);
	addSpriteTile(s,(0x9<<2)|0x41,0x0042,8,-4);
	addSpriteTile(s,(0x9<<2)|0x81,0x0042,-8,4);
	addSpriteTile(s,(0x9<<2)|0xC1,0x0042,8,4);
}
//Poochy
void drawSprite_0FF(sprite_t * s) {
	int base = findSpGfxFile(0x3A);
	addSpriteTile(s,(0x9<<2)|1,base+0x00,-16,-7);
	addSpriteTile(s,(0x9<<2)|1,base+0x02,0,-7);
	addSpriteTile(s,(0x9<<2)|1,base+0x0A,5,4);
	addSpriteTile(s,(0x9<<2),base+0x0C,-3,12);
	addSpriteTile(s,(0x9<<2)|1,base+0x04,-6,2);
	addSpriteTile(s,(0x9<<2)|0x40,base+0x3C,15,-5);
	addSpriteTile(s,(0x9<<2),base+0x3F,-10,-6);
}
//Bubble with 1-UP
void drawSprite_100(sprite_t * s) {
	addSpriteTile(s,(0x9<<2),0x0049,-1,4);
	addSpriteTile(s,(0x9<<2),0x006E,1,4);
	addSpriteTile(s,(0x9<<2),0x006F,8,4);
	addSpriteTile(s,(0x8<<2)|1,0x009C,-7,-7);
	addSpriteTile(s,(0x8<<2)|1,0x007E,7,-7);
	addSpriteTile(s,(0x8<<2)|0xC1,0x007E,-7,7);
	addSpriteTile(s,(0x8<<2)|0x81,0x007E,7,7);
}
//Rotating spike mace, 1 end
void drawSprite_101(sprite_t * s) {
	addSpriteTile(s,(0x8<<2),0x410C,-8,-31);
	addSpriteTile(s,(0x8<<2),0x410D,8,-31);
	addSpriteTile(s,(0x8<<2),0x411C,-8,-15);
	addSpriteTile(s,(0x8<<2),0x411D,8,-15);
	addSpriteTile(s,(0x8<<2),0x410C,-8,-59);
	addSpriteTile(s,(0x8<<2),0x410D,8,-59);
	addSpriteTile(s,(0x8<<2),0x411C,-8,-43);
	addSpriteTile(s,(0x8<<2),0x411D,8,-43);
	addSpriteTile(s,(0x8<<2),0x410A,-8,-87);
	addSpriteTile(s,(0x8<<2),0x410B,8,-87);
	addSpriteTile(s,(0x8<<2),0x411A,-8,-71);
	addSpriteTile(s,(0x8<<2),0x411B,8,-71);
}
//Rotating spike mace, 2 end
void drawSprite_102(sprite_t * s) {
	addSpriteTile(s,(0x8<<2),0x410C,-8,-31);
	addSpriteTile(s,(0x8<<2),0x410D,8,-31);
	addSpriteTile(s,(0x8<<2),0x411C,-8,-15);
	addSpriteTile(s,(0x8<<2),0x411D,8,-15);
	addSpriteTile(s,(0x8<<2),0x410C,-8,-59);
	addSpriteTile(s,(0x8<<2),0x410D,8,-59);
	addSpriteTile(s,(0x8<<2),0x411C,-8,-43);
	addSpriteTile(s,(0x8<<2),0x411D,8,-43);
	addSpriteTile(s,(0x8<<2),0x410A,-8,-87);
	addSpriteTile(s,(0x8<<2),0x410B,8,-87);
	addSpriteTile(s,(0x8<<2),0x411A,-8,-71);
	addSpriteTile(s,(0x8<<2),0x411B,8,-71);
	addSpriteTile(s,(0x8<<2)|0xC0,0x410C,8,31);
	addSpriteTile(s,(0x8<<2)|0xC0,0x410D,-8,31);
	addSpriteTile(s,(0x8<<2)|0xC0,0x411C,8,15);
	addSpriteTile(s,(0x8<<2)|0xC0,0x411D,-8,15);
	addSpriteTile(s,(0x8<<2)|0xC0,0x410C,8,59);
	addSpriteTile(s,(0x8<<2)|0xC0,0x410D,-8,59);
	addSpriteTile(s,(0x8<<2)|0xC0,0x411C,8,43);
	addSpriteTile(s,(0x8<<2)|0xC0,0x411D,-8,43);
	addSpriteTile(s,(0x8<<2)|0xC0,0x410A,8,87);
	addSpriteTile(s,(0x8<<2)|0xC0,0x410B,-8,87);
	addSpriteTile(s,(0x8<<2)|0xC0,0x411A,8,71);
	addSpriteTile(s,(0x8<<2)|0xC0,0x411B,-8,71);
}
//Boo Guys controlling rotating spike mace
void drawSprite_103(sprite_t * s) {
	int base = findSpGfxFile(0x3C);
	addSpriteTile(s,(0x8<<2)|1,base+0x00,12,0);
	addSpriteTile(s,(0x8<<2)|1,base+0x08,-4,0);
}
//Jean de Fillet
void drawSprite_104(sprite_t * s) {
	addSpriteTile(s,(0x8<<2),0x454E,-8,-8);
	addSpriteTile(s,(0x8<<2),0x454F,8,-8);
	addSpriteTile(s,(0x8<<2),0x455E,-8,8);
	addSpriteTile(s,(0x8<<2),0x455F,8,8);
}
//Boo Guys carring bomb
void drawSprite_105(sprite_t * s) {
	addSpriteTile(s,(0x8<<2)|1,0x0160,0,-8);
}
//Watermelon projectile
void drawSprite_107(sprite_t * s) {
	addSpriteTile(s,(0x8<<2),0x00BC,0,0);
}
//Milde
void drawSprite_108(sprite_t * s) {
	int base = findSpGfxFile(0x29);
	addSpriteTile(s,(0xC<<2)|1,base+0x04,0,0);
}
//Tap-Tap
void drawSprite_109(sprite_t * s) {
	addSpriteTile(s,(0x8<<2)|1,0x01AB,0,0);
	addSpriteTile(s,(0x8<<2),0x009E,0,8);
	addSpriteTile(s,(0x8<<2),0x009E,8,8);
}
//Falling spike mace
void drawSprite_10C(sprite_t * s) {
	int base = findSpGfxFile(0x3F);
	addSpriteTile(s,(0x8<<2),base+0x0B,12,8);
	addSpriteTile(s,(0x8<<2),base+0x0B,12,18);
	addSpriteTile(s,(0x8<<2),base+0x0B,12,28);
	addSpriteTile(s,(0x8<<2),base+0x0B,12,38);
	addSpriteTile(s,(0x8<<2),base+0x0B,12,48);
	addSpriteTile(s,(0x8<<2),base+0x0B,12,58);
	addSpriteTile(s,(0x8<<2),base+0x0B,12,68);
	addSpriteTile(s,(0x8<<2),base+0x0B,12,78);
	addSpriteTile(s,(0x8<<2),base+0x0B,12,88);
	addSpriteTile(s,(0x8<<2),base+0x0B,12,98);
	addSpriteTile(s,(0x8<<2),base+0x0B,12,108);
	addSpriteTile(s,(0x8<<2),0x410A,0,113);
	addSpriteTile(s,(0x8<<2),0x410B,16,113);
	addSpriteTile(s,(0x8<<2),0x411A,0,129);
	addSpriteTile(s,(0x8<<2),0x411B,16,129);
	addSpriteTile(s,(0x8<<2)|1,base+0x0C,4,0);
	addSpriteTile(s,(0x8<<2)|1,base+0x00,-4,0);
	addSpriteTile(s,(0x8<<2),base+0x1B,0,8);
}
//Boo Guys controlling falling spike mace
void drawSprite_10D(sprite_t * s) {
	int base = findSpGfxFile(0x3F);
	addSpriteTile(s,(0x8<<2)|1,base+0x0C,4,0);
	addSpriteTile(s,(0x8<<2)|1,base+0x00,-4,0);
	addSpriteTile(s,(0x8<<2),base+0x1B,0,8);
}
//Boo Man Bluff
void drawSprite_10F(sprite_t * s) {
	int base = findSpGfxFile(0x30);
	addSpriteTile(s,(0x8<<2),base+0x06,-4,12);
	addSpriteTile(s,(0x8<<2),base+0x07,4,12);
	addSpriteTile(s,(0x8<<2),base+0x08,12,12);
	addSpriteTile(s,(0x8<<2)|1,base+0x00,-4,-4);
	addSpriteTile(s,(0x8<<2)|1,base+0x01,4,-4);
	addSpriteTile(s,(0x8<<2),base+0x0B,0,-2);
	addSpriteTile(s,(0x8<<2),base+0x09,3,10);
}
//Flower (character)
void drawSprite_110(sprite_t * s) {
	int base = findSpGfxFile(0x71);
	addSpriteTile(s,(0x9<<2)|1,base+0x04,-8,-8);
	addSpriteTile(s,(0x9<<2)|1,base+0x06,8,-8);
	addSpriteTile(s,(0x9<<2)|0xC1,base+0x06,-8,8);
	addSpriteTile(s,(0x9<<2)|0xC1,base+0x04,8,8);
	addSpriteTile(s,(0x9<<2)|1,base+0x02,0,0);
}
//Georgette Jelly
void drawSprite_111(sprite_t * s) {
	int base = findSpGfxFile(0x41);
	addSpriteTile(s,(0x8<<2)|1,base+0x00,0,0);
	addSpriteTile(s,(0x8<<2),base+0x04,-7,8);
	addSpriteTile(s,(0x8<<2),base+0x14,15,8);
}
//Georgette Jelly splat
void drawSprite_112(sprite_t * s) {
	int base = findSpGfxFile(0x41);
	addSpriteTile(s,(0x8<<2),base+0x0C,0,0);
}
//Snifit
void drawSprite_113(sprite_t * s) {
	int base = findSpGfxFile(0x40);
	addSpriteTile(s,(0x8<<2)|0x41,base+0x06,3,-7);
	addSpriteTile(s,(0x8<<2),base+0x1E,1,5);
	addSpriteTile(s,(0x8<<2)|0x80,base+0x0E,9,5);
	addSpriteTile(s,(0xA<<2),base+0x1C,1,12);
	addSpriteTile(s,(0xA<<2),base+0x1C,9,12);
	addSpriteTile(s,(0xA<<2)|1,base+0x00,-4,-7);
}
//Snifit projectile
void drawSprite_114(sprite_t * s) {
	int base = findSpGfxFile(0x40);
	addSpriteTile(s,(0xA<<2),base+0x1F,0,4);
	addSpriteTile(s,(0xA<<2),base+0x0F,8,4);
}
//Coin
void drawSprite_115(sprite_t * s) {
	addSpriteTile(s,(0xA<<2)|1,0x00A0,0,0);
}
//Floating grass platform
void drawSprite_116(sprite_t * s) {
	int base = findSpGfxFile(0x4C);
	addSpriteTile(s,(0x8<<2)|1,base+0x00,-8,-8);
	addSpriteTile(s,(0x8<<2)|1,base+0x02,8,-8);
	addSpriteTile(s,(0x8<<2)|1,base+0x04,-8,8);
	addSpriteTile(s,(0x8<<2)|1,base+0x06,8,8);
}
//Donut lift
void drawSprite_117(sprite_t * s) {
	int base = findSpGfxFile(0x4C);
	addSpriteTile(s,(0x8<<2)|1,base+0x08,0,0);
}
//Large donut lift
void drawSprite_118(sprite_t * s) {
	int base = findSpGfxFile(0x4C);
	addSpriteTile(s,(0x8<<2)|1,base+0x0A,-8,-8);
	addSpriteTile(s,(0x8<<2)|1,base+0x0C,8,-8);
	addSpriteTile(s,(0x8<<2)|1,base+0x0E,-8,8);
	addSpriteTile(s,(0x8<<2)|0x41,base+0x0E,8,8);
}
//Spooky
void drawSprite_119(sprite_t * s) {
	addSpriteTile(s,(0x8<<2),0x4100,-8,-8);
	addSpriteTile(s,(0x8<<2),0x4101,8,-8);
	addSpriteTile(s,(0x8<<2),0x4110,-8,8);
	addSpriteTile(s,(0x8<<2),0x4111,8,8);
}
//Green Glove
void drawSprite_11A(sprite_t * s) {
	addSpriteTile(s,(0x8<<2),0x009E,1,8);
	addSpriteTile(s,(0x8<<2),0x0154,-1,4);
	addSpriteTile(s,(0x8<<2),0x0155,7,4);
	addSpriteTile(s,(0x8<<2)|1,0x0140,-2,-9);
	addSpriteTile(s,(0xC<<2)|0xC1,0x014A,3,3);
}
//Lakitu
void drawSprite_11B(sprite_t * s) {
	int base = findSpGfxFile(0x49);
	addSpriteTile(s,(0x8<<2),base+0x1D,10,-7);
	addSpriteTile(s,(0xA<<2)|0x41,base+0x08,4,0);
	addSpriteTile(s,(0xA<<2)|1,base+0x08,-4,0);
	addSpriteTile(s,(0x9<<2),base+0x0D,4,-20);
	addSpriteTile(s,(0x9<<2)|1,base+0x00,-2,-13);
	if(s->data[2]&1) {
		addSpriteTile(s,(0x8<<2),base+0x1D,42,-7);
		addSpriteTile(s,(0xA<<2)|0x41,base+0x08,36,0);
		addSpriteTile(s,(0xA<<2)|1,base+0x08,28,0);
		addSpriteTile(s,(0x9<<2),base+0x0D,36,-20);
		addSpriteTile(s,(0x9<<2)|1,base+0x00,30,-13);
	}
}
//Lakitu cloud
void drawSprite_11C(sprite_t * s) {
	int base = findSpGfxFile(0x49);
	addSpriteTile(s,(0xA<<2)|0x41,base+0x08,4,0);
	addSpriteTile(s,(0xA<<2)|1,base+0x08,-4,0);
}
//Lakitu projectile
void drawSprite_11D(sprite_t * s) {
	int base = findSpGfxFile(0x49);
	addSpriteTile(s,(0xA<<2)|1,base+0x0E,0,0);
}
//Arrow lift
void drawSprite_11E(sprite_t * s) {
	int pal = (((s->data[0]-0x1E)<<1)+8)<<2;
	addSpriteTile(s,pal,0x4426,-8,-8);
	addSpriteTile(s,pal,0x4427,8,-8);
	addSpriteTile(s,pal,0x4436,-8,8);
	addSpriteTile(s,pal,0x4437,8,8);
}
//Arrow lift, double-ended
void drawSprite_120(sprite_t * s) {
	addSpriteTile(s,(0xC<<2),0x4140,-8,-8);
	addSpriteTile(s,(0xC<<2),0x4141,8,-8);
	addSpriteTile(s,(0xC<<2),0x4150,-8,8);
	addSpriteTile(s,(0xC<<2),0x4151,8,8);
}
//Number platform explosion
void drawSprite_121(sprite_t * s) {
	addSpriteTile(s,(0x9<<2)|1,0x0042,-8,-8);
	addSpriteTile(s,(0x9<<2)|0x41,0x0042,8,-8);
	addSpriteTile(s,(0x9<<2)|0x81,0x0042,-8,8);
	addSpriteTile(s,(0x9<<2)|0xC1,0x0042,8,8);
}
//Stretch
void drawSprite_124(sprite_t * s) {
	int offset = ((s->data[2]&1)|(s->data[1]&2))<<1;
	int pal = ((romBuf[0x0209B8+offset]-1)<<1)|0x20;
	int base = findSpGfxFile(0x27);
	addSpriteTile(s,pal|1,base+0x08,0,-16);
	addSpriteTile(s,pal|1,base+0x0C,0,0);
}
//Falling spike mace 2
void drawSprite_126(sprite_t * s) {
	addSpriteTile(s,(0xA<<2)|1,0x010C,7,7);
	addSpriteTile(s,(0xA<<2)|1,0x010C,7,23);
	addSpriteTile(s,(0xA<<2)|1,0x010C,7,39);
	addSpriteTile(s,(0xA<<2)|1,0x010C,7,55);
	addSpriteTile(s,(0xA<<2)|1,0x010C,7,71);
	addSpriteTile(s,(0xC<<2)|1,0x0100,-2,88);
	addSpriteTile(s,(0xC<<2)|1,0x0102,14,88);
	addSpriteTile(s,(0xC<<2)|1,0x0104,-2,104);
	addSpriteTile(s,(0xC<<2)|1,0x0106,14,104);
	addSpriteTile(s,(0xC<<2)|0xC1,0x0102,-2,120);
	addSpriteTile(s,(0xC<<2)|0xC1,0x0100,14,120);
	addSpriteTile(s,(0xA<<2),0x010E,10,86);
	addSpriteTile(s,(0xC<<2),0x412E,-8,-8);
	addSpriteTile(s,(0xC<<2),0x412F,8,-8);
	addSpriteTile(s,(0xC<<2),0x413E,-8,8);
	addSpriteTile(s,(0xC<<2),0x413F,8,8);
}
//Boo Guys controlling falling spike mace 2
void drawSprite_127(sprite_t * s) {
	addSpriteTile(s,(0xC<<2),0x412E,-8,-8);
	addSpriteTile(s,(0xC<<2),0x412F,8,-8);
	addSpriteTile(s,(0xC<<2),0x413E,-8,8);
	addSpriteTile(s,(0xC<<2),0x413F,8,8);
}
//Ground shake
void drawSprite_128(sprite_t * s) {
	drawSpriteText(s,"Baby  Bowser\n    Wave    ");
}
//Fuzzy
void drawSprite_129(sprite_t * s) {
	int base = findSpGfxFile(0x52);
	addSpriteTile(s,(0xA<<2)|1,base+0x00,-4,-4);
	addSpriteTile(s,(0xA<<2)|0x41,base+0x00,4,-4);
	addSpriteTile(s,(0xA<<2),base+0x12,-4,12);
	addSpriteTile(s,(0xA<<2),base+0x13,4,12);
	addSpriteTile(s,(0xA<<2)|0x40,base+0x12,12,12);
	addSpriteTile(s,(0xA<<2)|1,base+0x04,-8,-8);
	addSpriteTile(s,(0xA<<2)|0x41,base+0x04,8,-8);
	addSpriteTile(s,(0xA<<2)|1,base+0x06,-8,8);
	addSpriteTile(s,(0xA<<2)|0x41,base+0x06,8,8);
}
//Shy Guy hiding Bandit
void drawSprite_12A(sprite_t * s) {
	addSpriteTile(s,(0x8<<2)|1,0x0088,0,-3);
	addSpriteTile(s,(0x8<<2),0x009E,1,8);
	addSpriteTile(s,(0x8<<2),0x009E,7,8);
}
//Fat Guy
void drawSprite_12B(sprite_t * s) {
	int base = findSpGfxFile(0x50);
	int pal = (s->data[2]&1)^1;
	pal = (pal+0x8)<<2;
	addSpriteTile(s,pal,base+0x09,-6,8);
	addSpriteTile(s,pal|0x40,base+0x18,-6,-6);
	addSpriteTile(s,pal|1,base+0x02,-10,-2);
	addSpriteTile(s,pal|1,base+0x06,4,-2);
	addSpriteTile(s,pal,base+0x18,15,-5);
	addSpriteTile(s,pal|0x40,base+0x09,13,8);
	addSpriteTile(s,pal|1,base+0x00,0,-14);
}
//Fly Guy
void drawSprite_12C(sprite_t * s) {
	if(s->data[2]&1) {
		addSpriteTile(s,(0xB<<2),0x002C,0,-6);
		addSpriteTile(s,(0xB<<2)|0x40,0x002C,8,-6);
		addSpriteTile(s,(0xB<<2)|0x40,0x002F,6,14);
		addSpriteTile(s,(0xB<<2)|0x40,0x002F,1,14);
		addSpriteTile(s,(0xB<<2)|1,0x0088,0,0);
	} else {
		addSpriteTile(s,(0x9<<2)|1,0x00A0,0,16);
		addSpriteTile(s,(0x9<<2),0x002C,0,-6);
		addSpriteTile(s,(0x9<<2)|0x40,0x002C,8,-6);
		addSpriteTile(s,(0x9<<2)|0x40,0x002F,6,14);
		addSpriteTile(s,(0x9<<2)|0x40,0x002F,1,14);
		addSpriteTile(s,(0x9<<2)|1,0x0088,0,0);
	}
}
//Yoshi for intro
void drawSprite_12D(sprite_t * s) {
	int base = findSpGfxFile(0xAB);
	addSpriteTile(s,(0x8<<2)|1,base+0x03,0,16);
	addSpriteTile(s,(0x8<<2)|1,base+0x00,-9,2);
	addSpriteTile(s,(0x8<<2),base+0x02,9,2);
	addSpriteTile(s,(0x8<<2),base+0x12,9,10);
}
//Boss pop effect
void drawSprite_12E(sprite_t * s) {
	drawSpriteText(s,"Boss Pop\n Effect ");
}
//Lava Drop, horizontal
void drawSprite_12F(sprite_t * s) {
	int base = findSpGfxFile(0x51);
	addSpriteTile(s,(0x9<<2)|1,base+0x07,0,0);
	addSpriteTile(s,(0x9<<2),base+0x06,16,5);
	addSpriteTile(s,(0x9<<2),base+0x05,1,3);
}
//Lava Drop, vertical
void drawSprite_130(sprite_t * s) {
	int base = findSpGfxFile(0x51);
	addSpriteTile(s,(0x9<<2)|1,base+0x00,0,0);
	addSpriteTile(s,(0x9<<2),base+0x14,4,16);
	addSpriteTile(s,(0x9<<2),base+0x04,4,0);
}
//Lemon Drop
void drawSprite_132(sprite_t * s) {
	int base = findSpGfxFile(0x45);
	addSpriteTile(s,(0x9<<2)|1,base+0x06,0,0);
}
//Lantern Ghost
void drawSprite_133(sprite_t * s) {
	int offset = ((s->data[2]&1)|(s->data[1]&2))<<1;
	int pal = ((romBuf[0x0209B8+offset]-1)<<1)|0x20;
	addSpriteTile(s,pal|0x40,0x010F,-4,2);
	addSpriteTile(s,pal|1,0x0100,0,-3);
	addSpriteTile(s,pal,0x009E,7,8);
	addSpriteTile(s,pal,0x009E,1,8);
	addSpriteTile(s,(0x9<<2),0x011B,-7,7);
}
//Baby Bowser
void drawSprite_134(sprite_t * s) {
	int base = findSpGfxFile(0xAD);
	addSpriteTile(s,(0xE<<2)|0x81,base+0x1A,0,-2);
	addSpriteTile(s,(0xE<<2)|0x81,base+0x0B,8,6);
	addSpriteTile(s,(0xE<<2)|0x80,base+0x0A,0,14);
	addSpriteTile(s,(0xE<<2),base+0x04,6,-9);
	addSpriteTile(s,(0xE<<2)|1,base+0x14,-14,-3);
	addSpriteTile(s,(0xE<<2)|1,base+0x05,-6,-11);
	addSpriteTile(s,(0xE<<2),base+0x26,2,5);
	addSpriteTile(s,(0xE<<2),base+0x36,6,13);
	addSpriteTile(s,(0xE<<2),base+0x2C,-4,13);
	addSpriteTile(s,(0xE<<2),base+0x2D,10,13);
}
//Raven
void drawSprite_135(sprite_t * s) {
	addSpriteTile(s,(0xA<<2),0x4148,-8,-8);
	addSpriteTile(s,(0xA<<2),0x4149,8,-8);
	addSpriteTile(s,(0xA<<2),0x4158,-8,8);
	addSpriteTile(s,(0xA<<2),0x4159,8,8);
}
//Falling rock, 3x6
void drawSprite_137(sprite_t * s) {
	int base = findSpGfxFile(0x20);
	for(int j=0; j<6; j++) {
		for(int i=0; i<3; i++) {
			WORD tile = 0x08;
			if(i==0) tile -= 2;
			else if(i==2) tile += 2;
			if(j==0) tile -= 6;
			else if(j!=5) tile += 6;
			if(tile==0x10) tile = 0x20;
			addSpriteTile(s,(0xE<<2)|1,base+tile,(i<<4)-16,(j<<4)-48);
		}
	}
}
//Falling rock, 3x3
void drawSprite_138(sprite_t * s) {
	int base = findSpGfxFile(0x20);
	for(int j=0; j<3; j++) {
		for(int i=0; i<3; i++) {
			WORD tile = 0x08;
			if(i==0) tile -= 2;
			else if(i==2) tile += 2;
			if(j==0) tile -= 6;
			else if(j!=2) tile += 6;
			if(tile==0x10) tile = 0x20;
			addSpriteTile(s,(0xE<<2)|1,base+tile,(i<<4)-16,j<<4);
		}
	}
}
//Falling rock, 3x9
void drawSprite_139(sprite_t * s) {
	int base = findSpGfxFile(0x20);
	for(int j=0; j<9; j++) {
		for(int i=0; i<3; i++) {
			WORD tile = 0x08;
			if(i==0) tile -= 2;
			else if(i==2) tile += 2;
			if(j==0) tile -= 6;
			else if(j!=8) tile += 6;
			if(tile==0x10) tile = 0x20;
			addSpriteTile(s,(0xE<<2)|1,base+tile,(i<<4)-16,(j<<4)-72);
		}
	}
}
//Falling rock, 6x3
void drawSprite_13A(sprite_t * s) {
	int base = findSpGfxFile(0x20);
	for(int j=0; j<3; j++) {
		for(int i=0; i<6; i++) {
			WORD tile = 0x08;
			if(i==0) tile -= 2;
			else if(i==5) tile += 2;
			if(j==0) tile -= 6;
			else if(j!=2) tile += 6;
			if(tile==0x10) tile = 0x20;
			addSpriteTile(s,(0xE<<2)|1,base+tile,(i<<4)-32,j<<4);
		}
	}
}
//Stomach drop
void drawSprite_13B(sprite_t * s) {
	int base = findSpGfxFile(0x70);
	addSpriteTile(s,(0xF<<2)|1,base+0x0E,0,0);
}
//Flipper, vertical
void drawSprite_13C(sprite_t * s) {
	addSpriteTile(s,(0x8<<2),0x4146,-24,-8);
	addSpriteTile(s,(0x8<<2),0x4147,-8,-8);
	addSpriteTile(s,(0x8<<2),0x4156,-24,8);
	addSpriteTile(s,(0x8<<2),0x4157,-8,8);
	addSpriteTile(s,(0x8<<2)|0x40,0x4146,24,-8);
	addSpriteTile(s,(0x8<<2)|0x40,0x4147,8,-8);
	addSpriteTile(s,(0x8<<2)|0x40,0x4156,24,8);
	addSpriteTile(s,(0x8<<2)|0x40,0x4157,8,8);
}
//Fang
void drawSprite_13D(sprite_t * s) {
	int base = findSpGfxFile(0x55);
	addSpriteTile(s,(0x8<<2)|0x40,base+0x1F,-3,-2);
	addSpriteTile(s,(0x8<<2)|1,base+0x0D,0,1);
	addSpriteTile(s,(0x8<<2),base+0x1F,11,-2);
}
//Flopsy Fish
void drawSprite_13F(sprite_t * s) {
	int base = findSpGfxFile(0x54);
	addSpriteTile(s,(0x9<<2)|1,base+0x02,0,0);
	addSpriteTile(s,(0x9<<2),base+0x0B,4,-8);
	addSpriteTile(s,(0x9<<2),base+0x1A,16,1);
	addSpriteTile(s,(0x9<<2)|0x80,base+0x1A,16,8);
	addSpriteTile(s,(0x9<<2),base+0x0C,8,5);
}
//Spray Fish
void drawSprite_143(sprite_t * s) {
	int base = findSpGfxFile(0x58);
	addSpriteTile(s,(0xA<<2)|1,base+0x00,-8,0);
	addSpriteTile(s,(0xA<<2)|1,base+0x02,8,0);
}
//Flipper, horizontal
void drawSprite_144(sprite_t * s) {
	if(s->data[2]&1) {
		addSpriteTile(s,(0x8<<2)|0x40,0x6146,8,-24);
		addSpriteTile(s,(0x8<<2)|0x40,0x6156,-8,-24);
		addSpriteTile(s,(0x8<<2)|0x40,0x6147,8,-8);
		addSpriteTile(s,(0x8<<2)|0x40,0x6157,-8,-8);
		addSpriteTile(s,(0x8<<2)|0xC0,0x6146,8,24);
		addSpriteTile(s,(0x8<<2)|0xC0,0x6156,-8,24);
		addSpriteTile(s,(0x8<<2)|0xC0,0x6147,8,8);
		addSpriteTile(s,(0x8<<2)|0xC0,0x6157,-8,8);
	} else {
		addSpriteTile(s,(0x8<<2),0x6146,-8,-24);
		addSpriteTile(s,(0x8<<2),0x6156,8,-24);
		addSpriteTile(s,(0x8<<2),0x6147,-8,-8);
		addSpriteTile(s,(0x8<<2),0x6157,8,-8);
		addSpriteTile(s,(0x8<<2)|0x80,0x6146,-8,24);
		addSpriteTile(s,(0x8<<2)|0x80,0x6156,8,24);
		addSpriteTile(s,(0x8<<2)|0x80,0x6147,-8,8);
		addSpriteTile(s,(0x8<<2)|0x80,0x6157,8,8);
	}
}
//Sluggy
void drawSprite_145(sprite_t * s) {
	int pal = (((s->data[0]-0x45)<<2)+8)<<2;
	int base = findSpGfxFile(0x55);
	addSpriteTile(s,pal|0x80,base+0x1A,15,0);
	addSpriteTile(s,pal|0x81,base+0x02,0,0);
}
//Horizontal pipe exit left
void drawSprite_147(sprite_t * s) {
	drawSpriteText(s,"Horizontal Pipe\n   Exit Left   ");
}
//Arrow Cloud, up
void drawSprite_149(sprite_t * s) {
	int base = findSpGfxFile(0x57);
	addSpriteTile(s,(0x8<<2)|1,base+0x00,-2,-3);
	addSpriteTile(s,(0x8<<2)|1,base+0x01,6,-3);
	addSpriteTile(s,(0x8<<2),base+0x06,-2,13);
	addSpriteTile(s,(0x8<<2),base+0x07,6,13);
	addSpriteTile(s,(0x8<<2),base+0x08,14,13);
	addSpriteTile(s,(0x8<<2)|1,base+0x0A,0,0);
}
//Arrow Cloud, up right
void drawSprite_14A(sprite_t * s) {
	int base = findSpGfxFile(0x57);
	addSpriteTile(s,(0x8<<2),base+0x06,-2,13);
	addSpriteTile(s,(0x8<<2),base+0x07,6,13);
	addSpriteTile(s,(0x8<<2),base+0x08,14,13);
	addSpriteTile(s,(0x8<<2)|1,base+0x00,-2,-3);
	addSpriteTile(s,(0x8<<2)|1,base+0x01,6,-3);
	addSpriteTile(s,(0x8<<2)|1,base+0x0C,0,0);
}
//Arrow Cloud, right
void drawSprite_14B(sprite_t * s) {
	int base = findSpGfxFile(0x57);
	addSpriteTile(s,(0x8<<2)|1,base+0x00,-2,-3);
	addSpriteTile(s,(0x8<<2)|1,base+0x01,6,-3);
	addSpriteTile(s,(0x8<<2),base+0x06,-2,13);
	addSpriteTile(s,(0x8<<2),base+0x07,6,13);
	addSpriteTile(s,(0x8<<2),base+0x08,14,13);
	addSpriteTile(s,(0x8<<2)|1,base+0x0E,0,0);
}
//Arrow Cloud, down right
void drawSprite_14C(sprite_t * s) {
	int base = findSpGfxFile(0x57);
	addSpriteTile(s,(0x8<<2),base+0x06,-2,13);
	addSpriteTile(s,(0x8<<2),base+0x07,6,13);
	addSpriteTile(s,(0x8<<2),base+0x08,14,13);
	addSpriteTile(s,(0x8<<2)|1,base+0x00,-2,-3);
	addSpriteTile(s,(0x8<<2)|1,base+0x01,6,-3);
	addSpriteTile(s,(0x8<<2)|0x81,base+0x0C,0,0);
}
//Arrow Cloud, down
void drawSprite_14D(sprite_t * s) {
	int base = findSpGfxFile(0x57);
	addSpriteTile(s,(0x8<<2),base+0x06,-2,13);
	addSpriteTile(s,(0x8<<2),base+0x07,6,13);
	addSpriteTile(s,(0x8<<2),base+0x08,14,13);
	addSpriteTile(s,(0x8<<2)|1,base+0x00,-2,-3);
	addSpriteTile(s,(0x8<<2)|1,base+0x01,6,-3);
	addSpriteTile(s,(0x8<<2)|0x81,base+0x0A,0,0);
}
//Arrow Cloud, down left
void drawSprite_14E(sprite_t * s) {
	int base = findSpGfxFile(0x57);
	addSpriteTile(s,(0x8<<2),base+0x06,-2,13);
	addSpriteTile(s,(0x8<<2),base+0x07,6,13);
	addSpriteTile(s,(0x8<<2),base+0x08,14,13);
	addSpriteTile(s,(0x8<<2)|1,base+0x00,-2,-3);
	addSpriteTile(s,(0x8<<2)|1,base+0x01,6,-3);
	addSpriteTile(s,(0x8<<2)|0xC1,base+0x0C,0,0);
}
//Arrow Cloud, left
void drawSprite_14F(sprite_t * s) {
	int base = findSpGfxFile(0x57);
	addSpriteTile(s,(0x8<<2),base+0x06,-2,13);
	addSpriteTile(s,(0x8<<2),base+0x07,6,13);
	addSpriteTile(s,(0x8<<2),base+0x08,14,13);
	addSpriteTile(s,(0x8<<2)|1,base+0x00,-2,-3);
	addSpriteTile(s,(0x8<<2)|1,base+0x01,6,-3);
	addSpriteTile(s,(0x8<<2)|0x41,base+0x0E,0,0);
}
//Arrow Cloud, up left
void drawSprite_150(sprite_t * s) {
	int base = findSpGfxFile(0x57);
	addSpriteTile(s,(0x8<<2),base+0x06,-2,13);
	addSpriteTile(s,(0x8<<2),base+0x07,6,13);
	addSpriteTile(s,(0x8<<2),base+0x08,14,13);
	addSpriteTile(s,(0x8<<2)|1,base+0x00,-2,-3);
	addSpriteTile(s,(0x8<<2)|1,base+0x01,6,-3);
	addSpriteTile(s,(0x8<<2)|0x41,base+0x0C,0,0);
}
//Flutter
void drawSprite_152(sprite_t * s) {
	int base = findSpGfxFile(0x56);
	addSpriteTile(s,(0x9<<2)|1,base+0x08,9,-13);
	addSpriteTile(s,(0x9<<2),base+0x0E,13,3);
	addSpriteTile(s,(0x9<<2)|0x41,base+0x08,-11,-12);
	addSpriteTile(s,(0x9<<2)|0x40,base+0x0E,-4,4);
	addSpriteTile(s,(0xA<<2)|1,base+0x06,0,0);
	addSpriteTile(s,(0x9<<2)|1,base+0x00,-7,-9);
	addSpriteTile(s,(0x8<<2),base+0x1F,2,-16);
}
//Goonie dropping Shy Guys
void drawSprite_153(sprite_t * s) {
	int base = findSpGfxFile(0x2A);
	addSpriteTile(s,(0xA<<2),base+0x04,8,0);
	addSpriteTile(s,(0xA<<2)|0x41,base+0x22,-16,0);
	addSpriteTile(s,(0xA<<2)|0x40,base+0x24,-24,0);
	addSpriteTile(s,(0xA<<2)|0x40,base+0x34,0,4);
	addSpriteTile(s,(0xA<<2),base+0x24,39,0);
	addSpriteTile(s,(0xA<<2)|1,base+0x00,-8,0);
	addSpriteTile(s,(0xA<<2)|1,base+0x0C,3,8);
	addSpriteTile(s,(0xA<<2),base+0x34,15,4);
	addSpriteTile(s,(0xA<<2)|1,base+0x22,23,0);
	addSpriteTile(s,(0x8<<2)|1,0x0088,0,16);
	addSpriteTile(s,(0x8<<2)|0x80,0x002F,6,30);
	addSpriteTile(s,(0x8<<2)|0xC0,0x002F,1,30);
}
//Shark Chomp
void drawSprite_154(sprite_t * s) {
	int base = findSpGfxFile(0x53);
	for(int j=0; j<0x80; j++) {
		addSpriteTile(s,0,0x8480+j,-0x40,j-0x40);
	}
	addSpriteTile(s,(0xA<<2)|1,base+0x02,-10,-50);
	addSpriteTile(s,(0xA<<2)|0xC1,base+0x02,-18,-42);
	addSpriteTile(s,(0xA<<2)|0x40,base+0x03,-18,-50);
	addSpriteTile(s,(0xA<<2)|0x80,base+0x03,-2,-34);
}
//Very Goonie
void drawSprite_155(sprite_t * s) {
	int base = findSpGfxFile(0x2B);
	addSpriteTile(s,(0xA<<2)|0x40,base+0x0D,-6,-26);
	addSpriteTile(s,(0xA<<2)|0x40,base+0x07,-4,-3);
	addSpriteTile(s,(0xA<<2)|0x41,base+0x05,-12,-19);
	addSpriteTile(s,(0xA<<2),0x4428,-8,-8);
	addSpriteTile(s,(0xA<<2),0x4429,8,-8);
	addSpriteTile(s,(0xA<<2),0x4438,-8,8);
	addSpriteTile(s,(0xA<<2),0x4439,8,8);
	addSpriteTile(s,(0xA<<2),base+0x0D,16,-26);
	addSpriteTile(s,(0xA<<2),base+0x07,14,-3);
	addSpriteTile(s,(0xA<<2)|1,base+0x05,14,-19);
}
//Cactus Jack
void drawSprite_156(sprite_t * s) {
	addSpriteTile(s,(0x8<<2),0x4166,-8,-8);
	addSpriteTile(s,(0x8<<2),0x4167,8,-8);
	addSpriteTile(s,(0x8<<2),0x4176,-8,8);
	addSpriteTile(s,(0x8<<2),0x4177,8,8);
	if(s->data[2]&1) {
		addSpriteTile(s,(0x8<<2),0x4166,-8,-24);
		addSpriteTile(s,(0x8<<2),0x4167,8,-24);
		addSpriteTile(s,(0x8<<2),0x4176,-8,-8);
		addSpriteTile(s,(0x8<<2),0x4177,8,-8);
		addSpriteTile(s,(0x8<<2),0x4166,-8,-40);
		addSpriteTile(s,(0x8<<2),0x4167,8,-40);
		addSpriteTile(s,(0x8<<2),0x4176,-8,-24);
		addSpriteTile(s,(0x8<<2),0x4177,8,-24);
	}
}
//Wall Lakitu
void drawSprite_157(sprite_t * s) {
	int base = findSpGfxFile(0x49);
	addSpriteTile(s,(0x8<<2),base+0x1D,10,17);
	addSpriteTile(s,(0x9<<2),base+0x0D,4,4);
	addSpriteTile(s,(0x9<<2)|1,base+0x00,-2,11);
}
//Bowling Goonie
void drawSprite_158(sprite_t * s) {
	addSpriteTile(s,(0xA<<2),0x4428,-8,-8);
	addSpriteTile(s,(0xA<<2),0x4429,8,-8);
	addSpriteTile(s,(0xA<<2),0x4438,-8,8);
	addSpriteTile(s,(0xA<<2),0x4439,8,8);
}
//Grunt
void drawSprite_159(sprite_t * s) {
	addSpriteTile(s,(0x8<<2),0x009E,0,8);
	addSpriteTile(s,(0x9<<2),0x01B8,0,5);
	addSpriteTile(s,(0x9<<2),0x01B9,8,5);
	addSpriteTile(s,(0x9<<2)|1,0x01A0,0,-8);
	addSpriteTile(s,(0x8<<2),0x009E,11,8);
	addSpriteTile(s,(0x9<<2),0x01A8,6,5);
	addSpriteTile(s,(0x9<<2)|1,0x01A4,0,-15);
}
//Dancing Spear Guy
void drawSprite_15B(sprite_t * s) {
	int base = findSpGfxFile(0x39);
	addSpriteTile(s,(0x9<<2),base+0x1E,12,-12);
	addSpriteTile(s,(0x9<<2)|1,base+0x00,2,-4);
	addSpriteTile(s,(0x9<<2),base+0x1E,12,-20);
	addSpriteTile(s,(0x8<<2)|0x40,base+0x0F,-1,2);
	addSpriteTile(s,(0x8<<2)|0x40,base+0x1F,7,11);
	addSpriteTile(s,(0x9<<2),base+0x1E,12,-28);
	addSpriteTile(s,(0x9<<2),base+0x0E,12,-32);
}
//Green/red switch
void drawSprite_15C(sprite_t * s) {
	int pal = ((s->data[0]-0x5C)+8)<<2;
	int base = findSpGfxFile(0x2A);
	addSpriteTile(s,pal|1,base+0x0E,0,0);
}
//Flatbed Ferry pinwheel, pink with Shy Guys
void drawSprite_15E(sprite_t * s) {
	addSpriteTile(s,(0x8<<2),0x00BD,4,-4);
	addSpriteTile(s,(0x8<<2),0x00BD,8,-16);
	addSpriteTile(s,(0x8<<2),0x00BD,12,-28);
	addSpriteTile(s,(0x8<<2),0x00BD,16,0);
	addSpriteTile(s,(0x8<<2),0x00BD,28,4);
	addSpriteTile(s,(0x8<<2),0x00BD,0,8);
	addSpriteTile(s,(0x8<<2),0x00BD,-4,20);
	addSpriteTile(s,(0x8<<2),0x00BD,-8,-8);
	addSpriteTile(s,(0x8<<2),0x00BD,-20,-12);
	addSpriteTile(s,(0xC<<2)|1,0x0028,7,-45);
	addSpriteTile(s,(0xC<<2)|1,0x002A,23,-45);
	addSpriteTile(s,(0xC<<2)|1,0x0028,29,7);
	addSpriteTile(s,(0xC<<2)|1,0x002A,45,7);
	addSpriteTile(s,(0xC<<2)|1,0x0028,-23,29);
	addSpriteTile(s,(0xC<<2)|1,0x002A,-7,29);
	addSpriteTile(s,(0xC<<2)|1,0x0028,-45,-23);
	addSpriteTile(s,(0xC<<2)|1,0x002A,-29,-23);
	addSpriteTile(s,(0x8<<2)|1,0x0088,15,-64);
	addSpriteTile(s,(0x8<<2),0x009E,16,-53);
	addSpriteTile(s,(0x8<<2),0x009E,22,-53);
	addSpriteTile(s,(0x8<<2)|1,0x0088,37,-12);
	addSpriteTile(s,(0x8<<2),0x009E,38,-1);
	addSpriteTile(s,(0x8<<2),0x009E,44,-1);
	addSpriteTile(s,(0x8<<2)|1,0x0088,-15,10);
	addSpriteTile(s,(0x8<<2),0x009E,-14,21);
	addSpriteTile(s,(0x8<<2),0x009E,-7,21);
	addSpriteTile(s,(0x8<<2)|1,0x0088,-37,-42);
	addSpriteTile(s,(0x8<<2),0x009E,-36,-31);
	addSpriteTile(s,(0x8<<2),0x009E,-30,-31);
}
//Green/red spiked platform
void drawSprite_15F(sprite_t * s) {
	int pal = ((s->data[0]-0x5F)+8)<<2;
	addSpriteTile(s,pal,0x414C,-21,-8);
	addSpriteTile(s,pal,0x414D,-5,-8);
	addSpriteTile(s,pal,0x415C,-21,8);
	addSpriteTile(s,pal,0x415D,-5,8);
	addSpriteTile(s,pal|0x40,0x414C,20,-8);
	addSpriteTile(s,pal|0x40,0x414D,4,-8);
	addSpriteTile(s,pal|0x40,0x415C,20,8);
	addSpriteTile(s,pal|0x40,0x415D,4,8);
}
//Bonus item
void drawSprite_161(sprite_t * s) {
	int offsY = (s->data[1]&2);
	if(s->data[2]&1) {
		if(offsY) {
			addSpriteTile(s,(0x8<<2),0x440F,-8,-16);
			addSpriteTile(s,(0x8<<2)|0x40,0x440F,8,-16);
			addSpriteTile(s,(0x8<<2),0x441F,-8,0);
			addSpriteTile(s,(0x8<<2)|0x40,0x441F,8,0);
		} else {
			addSpriteTile(s,(0x9<<2)|1,0x00EE,0,0);
		}
	} else {
		if(offsY) {
			addSpriteTile(s,(0x9<<2),0x4042,-8,-8);
			addSpriteTile(s,(0x9<<2),0x4043,8,-8);
			addSpriteTile(s,(0x9<<2),0x4052,-8,8);
			addSpriteTile(s,(0x9<<2),0x4053,8,8);
		} else {
			addSpriteTile(s,(0x9<<2)|1,0x00A0,0,0);
		}
	}
}
//Double green spiked platform
void drawSprite_162(sprite_t * s) {
	int base = findSpGfxFile(0x2A);
	addSpriteTile(s,(0x8<<2)|1,base+0x0E,0,0);
	addSpriteTile(s,(0x8<<2)|0x40,0x414C,-14,-8);
	addSpriteTile(s,(0x8<<2)|0x40,0x414D,-30,-8);
	addSpriteTile(s,(0x8<<2)|0x40,0x415C,-14,8);
	addSpriteTile(s,(0x8<<2)|0x40,0x415D,-30,8);
	addSpriteTile(s,(0x8<<2),0x414C,-54,-8);
	addSpriteTile(s,(0x8<<2),0x414D,-38,-8);
	addSpriteTile(s,(0x8<<2),0x415C,-54,8);
	addSpriteTile(s,(0x8<<2),0x415D,-38,8);
	addSpriteTile(s,(0x8<<2)|0x40,0x414C,54,-8);
	addSpriteTile(s,(0x8<<2)|0x40,0x414D,38,-8);
	addSpriteTile(s,(0x8<<2)|0x40,0x415C,54,8);
	addSpriteTile(s,(0x8<<2)|0x40,0x415D,38,8);
	addSpriteTile(s,(0x8<<2),0x414C,14,-8);
	addSpriteTile(s,(0x8<<2),0x414D,30,-8);
	addSpriteTile(s,(0x8<<2),0x415C,14,8);
	addSpriteTile(s,(0x8<<2),0x415D,30,8);
}
//Nipper Plant
void drawSprite_164(sprite_t * s) {
	int base = findSpGfxFile(0x25);
	addSpriteTile(s,(0x9<<2)|1,base+0x02,0,0);
}
//Nipper Spore
void drawSprite_165(sprite_t * s) {
	int base = findSpGfxFile(0x25);
	addSpriteTile(s,(0x9<<2),base+0x0F,3,2);
	addSpriteTile(s,(0x9<<2),base+0x1F,1,9);
	addSpriteTile(s,(0xA<<2)|0x41,base+0x0D,0,-8);
}
//Thunder Lakitu
void drawSprite_166(sprite_t * s) {
	int base = findSpGfxFile(0x49);
	addSpriteTile(s,(0xA<<2)|0x41,base+0x25,-7,-8);
	addSpriteTile(s,(0xA<<2)|1,base+0x25,9,-8);
	addSpriteTile(s,(0x9<<2),base+0x1D,10,1);
	addSpriteTile(s,(0xA<<2)|0x41,base+0x08,4,8);
	addSpriteTile(s,(0xA<<2)|1,base+0x08,-4,8);
	addSpriteTile(s,(0xE<<2)|1,base+0x00,-2,-5);
	if(s->data[2]&1) {
		addSpriteTile(s,(0xA<<2)|0x41,base+0x25,25,-8);
		addSpriteTile(s,(0xA<<2)|1,base+0x25,41,-8);
		addSpriteTile(s,(0x9<<2),base+0x1D,42,1);
		addSpriteTile(s,(0xA<<2)|0x41,base+0x08,36,8);
		addSpriteTile(s,(0xA<<2)|1,base+0x08,28,8);
		addSpriteTile(s,(0xE<<2)|1,base+0x00,30,-5);
	}
}
//Koopa Shell
void drawSprite_167(sprite_t * s) {
	int pal = ((s->data[0]-0x67)+8)<<2;
	int base = findSpGfxFile(0x47);
	addSpriteTile(s,pal|1,base+0x0C,0,0);
}
//Beach Koopa
void drawSprite_169(sprite_t * s) {
	int pal = ((s->data[0]-0x69)+8)<<2;
	int base = findSpGfxFile(0x47);
	addSpriteTile(s,(0x9<<2),base+0x16,-1,2);
	addSpriteTile(s,pal,base+0x09,8,6);
	addSpriteTile(s,pal|1,base+0x04,2,-4);
	addSpriteTile(s,(0x9<<2)|1,base+0x00,-4,-15);
	addSpriteTile(s,pal,base+0x08,7,8);
	addSpriteTile(s,(0x9<<2),base+0x07,10,1);
}
//Koopa
void drawSprite_16B(sprite_t * s) {
	int pal = ((s->data[0]-0x6B)+8)<<2;
	int base = findSpGfxFile(0x47);
	addSpriteTile(s,(0x9<<2),base+0x16,-1,2);
	addSpriteTile(s,pal,base+0x09,8,6);
	addSpriteTile(s,pal|1,base+0x0A,2,-4);
	addSpriteTile(s,(0x9<<2)|1,base+0x00,-4,-15);
	addSpriteTile(s,pal,base+0x08,7,8);
	addSpriteTile(s,(0x9<<2),base+0x07,10,1);
}
//Parakoopa
void drawSprite_16D(sprite_t * s) {
	int pal = (s->data[0]==0x6D)?(0x8<<2):(0x9<<2);
	int base = findSpGfxFile(0x2B);
	addSpriteTile(s,(0x9<<2)|0x40,base+0x0D,-8,-13);
	addSpriteTile(s,(0x9<<2),base+0x0D,16,-13);
	addSpriteTile(s,(0x9<<2)|0x41,base+0x08,-11,-8);
	addSpriteTile(s,pal,base+0x38,-2,16);
	addSpriteTile(s,(0x9<<2),base+0x36,-3,10);
	addSpriteTile(s,pal|1,base+0x2A,0,4);
	addSpriteTile(s,pal,base+0x38,3,16);
	addSpriteTile(s,(0x9<<2)|1,base+0x20,-6,-6);
	addSpriteTile(s,(0x9<<2)|1,base+0x08,11,-8);
	addSpriteTile(s,(0x9<<2),base+0x26,4,9);
}
//Aqua Lakitu
void drawSprite_170(sprite_t * s) {
	int base = findSpGfxFile(0x4F);
	addSpriteTile(s,(0x9<<2)|1,base+0x0A,-2,-16);
	addSpriteTile(s,(0x9<<2),base+0x19,-10,-8);
	addSpriteTile(s,(0x8<<2),base+0x0C,0,0);
	addSpriteTile(s,(0x8<<2)|0x40,base+0x0C,8,0);
}
//Naval Piranha
void drawSprite_171(sprite_t * s) {
	int base = findSpGfxFile(0x5A);
	addSpriteTile(s,(0xE<<2)|0x41,base+0x08,-33,-38);
	addSpriteTile(s,(0xE<<2)|1,base+0x08,-24,-12);
	addSpriteTile(s,(0xE<<2)|0x81,base+0x08,-24,4);
	addSpriteTile(s,(0xE<<2)|0x41,base+0x26,-28,-51);
	addSpriteTile(s,(0xE<<2)|0xC1,base+0x08,-33,-22);
	addSpriteTile(s,(0xE<<2)|0x41,base+0x08,31,-38);
	addSpriteTile(s,(0xE<<2)|1,base+0x08,40,-12);
	addSpriteTile(s,(0xE<<2)|0x81,base+0x08,40,4);
	addSpriteTile(s,(0xE<<2)|0x41,base+0x26,36,-51);
	addSpriteTile(s,(0xE<<2)|0xC1,base+0x08,31,-22);
	addSpriteTile(s,(0xE<<2)|0x41,base+0x08,-48,2);
	addSpriteTile(s,(0xE<<2)|1,base+0x08,-16,2);
	addSpriteTile(s,(0xE<<2)|0x41,base+0x00,-40,-8);
	addSpriteTile(s,(0xE<<2)|1,base+0x00,-24,-8);
	addSpriteTile(s,(0xE<<2)|0x41,base+0x08,16,2);
	addSpriteTile(s,(0xE<<2)|1,base+0x08,48,2);
	addSpriteTile(s,(0xE<<2)|0x41,base+0x00,24,-8);
	addSpriteTile(s,(0xE<<2)|1,base+0x00,40,-8);
	addSpriteTile(s,(0xE<<2)|1,base+0x08,9,-38);
	addSpriteTile(s,(0xE<<2)|0x81,base+0x08,9,-22);
	addSpriteTile(s,(0xE<<2)|0x41,base+0x08,0,-12);
	addSpriteTile(s,(0xE<<2)|0xC1,base+0x08,0,4);
	addSpriteTile(s,(0xE<<2)|1,base+0x20,-10,-4);
	addSpriteTile(s,(0xE<<2),0x442A,-12,-60);
	addSpriteTile(s,(0xE<<2),0x442B,4,-60);
	addSpriteTile(s,(0xE<<2),0x443A,-12,-44);
	addSpriteTile(s,(0xE<<2),0x443B,4,-44);
}
//Naval Piranha bud
void drawSprite_172(sprite_t * s) {
	int base = findSpGfxFile(0x5A);
	addSpriteTile(s,(0xE<<2)|0x41,base+0x08,-1,-38);
	addSpriteTile(s,(0xE<<2)|1,base+0x08,8,-12);
	addSpriteTile(s,(0xE<<2)|0x81,base+0x08,8,4);
	addSpriteTile(s,(0xE<<2)|0x41,base+0x26,4,-51);
	addSpriteTile(s,(0xE<<2)|0xC1,base+0x08,-1,-22);
}
//Baron Von Zeppelin, Shy Guy
void drawSprite_173(sprite_t * s) {
	addSpriteTile(s,(0x8<<2)|1,0x00A8,0,-24);
	addSpriteTile(s,(0x8<<2),0x00BC,4,-10);
	addSpriteTile(s,(0x8<<2),0x00BC,4,-6);
	addSpriteTile(s,(0x8<<2)|0x40,0x002F,1,14);
	addSpriteTile(s,(0x8<<2)|0x40,0x002F,6,14);
	addSpriteTile(s,(0x9<<2)|1,0x0088,0,0);
}
//Baron Von Zeppelin, Needlenose
void drawSprite_174(sprite_t * s) {
	addSpriteTile(s,(0x8<<2)|1,0x00A8,0,-24);
	addSpriteTile(s,(0x8<<2),0x00BC,4,-10);
	addSpriteTile(s,(0x8<<2),0x00BC,4,-6);
	addSpriteTile(s,(0x8<<2)|1,0x008A,0,0);
}
//Baron Von Zeppelin, bomb
void drawSprite_175(sprite_t * s) {
	addSpriteTile(s,(0x8<<2)|1,0x00A8,0,-24);
	addSpriteTile(s,(0x8<<2),0x00BC,4,-10);
	addSpriteTile(s,(0x8<<2),0x00BC,4,-6);
	addSpriteTile(s,(0xC<<2)|1,0x00EC,0,0);
}
//Baron Von Zeppelin, Bandit
void drawSprite_176(sprite_t * s) {
	addSpriteTile(s,(0x8<<2)|1,0x00A8,0,-32);
	addSpriteTile(s,(0x8<<2),0x00BC,4,-18);
	addSpriteTile(s,(0x8<<2),0x00BC,4,-14);
	addSpriteTile(s,(0x8<<2)|0x40,0x017B,2,14);
	addSpriteTile(s,(0x8<<2)|0x40,0x017B,7,14);
	addSpriteTile(s,(0x9<<2)|1,0x0168,0,-8);
	addSpriteTile(s,(0x8<<2),0x016A,-1,7);
	addSpriteTile(s,(0x8<<2),0x016B,7,7);
}
//Baron Von Zeppelin, large spring ball
void drawSprite_177(sprite_t * s) {
	addSpriteTile(s,(0x8<<2)|1,0x00A8,0,-24);
	addSpriteTile(s,(0x8<<2),0x00BC,4,-10);
	addSpriteTile(s,(0x8<<2),0x00BC,4,-6);
	addSpriteTile(s,(0x8<<2),0x414E,-8,-8);
	addSpriteTile(s,(0x8<<2),0x414F,8,-8);
	addSpriteTile(s,(0x8<<2),0x415E,-8,8);
	addSpriteTile(s,(0x8<<2),0x415F,8,8);
}
//Baron Von Zeppelin, 1-UP
void drawSprite_178(sprite_t * s) {
	addSpriteTile(s,(0x8<<2)|1,0x00A8,0,-24);
	addSpriteTile(s,(0x8<<2),0x00BC,4,-10);
	addSpriteTile(s,(0x8<<2),0x00BC,4,-6);
	addSpriteTile(s,(0xB<<2)|1,0x00E3,0,0);
	addSpriteTile(s,(0x9<<2),0x006E,0,4);
	addSpriteTile(s,(0x9<<2),0x006F,8,4);
	addSpriteTile(s,(0x9<<2),0x0049,-2,4);
}
//Baron Von Zeppelin, key
void drawSprite_179(sprite_t * s) {
	addSpriteTile(s,(0x8<<2)|1,0x00A8,0,-24);
	addSpriteTile(s,(0x8<<2),0x00BC,4,-10);
	addSpriteTile(s,(0x8<<2),0x00BC,4,-6);
	addSpriteTile(s,(0x9<<2)|1,0x00EE,0,0);
}
//Baron Von Zeppelin, 5 coins
void drawSprite_17A(sprite_t * s) {
	addSpriteTile(s,(0x8<<2)|1,0x00A8,0,-24);
	addSpriteTile(s,(0x8<<2),0x00BC,4,-10);
	addSpriteTile(s,(0x8<<2),0x00BC,4,-6);
	addSpriteTile(s,(0xA<<2)|1,0x00A0,0,0);
}
//Baron Von Zeppelin, watermelon
void drawSprite_17B(sprite_t * s) {
	addSpriteTile(s,(0x8<<2)|1,0x00A8,0,-24);
	addSpriteTile(s,(0x8<<2),0x00BC,4,-10);
	addSpriteTile(s,(0x8<<2),0x00BC,4,-6);
	addSpriteTile(s,(0x8<<2)|1,0x0068,0,0);
}
//Baron Von Zeppelin, fire watermelon
void drawSprite_17C(sprite_t * s) {
	addSpriteTile(s,(0x8<<2)|1,0x00A8,0,-24);
	addSpriteTile(s,(0x8<<2),0x00BC,4,-10);
	addSpriteTile(s,(0x8<<2),0x00BC,4,-6);
	addSpriteTile(s,(0x9<<2)|1,0x0068,0,0);
}
//Baron Von Zeppelin, ice watermelon
void drawSprite_17D(sprite_t * s) {
	addSpriteTile(s,(0x8<<2)|1,0x00A8,0,-24);
	addSpriteTile(s,(0x8<<2),0x00BC,4,-10);
	addSpriteTile(s,(0x8<<2),0x00BC,4,-6);
	addSpriteTile(s,(0xB<<2)|1,0x0068,0,0);
}
//Baron Von Zeppelin, crate
void drawSprite_17E(sprite_t * s) {
	addSpriteTile(s,(0x8<<2)|1,0x00A8,-12,-24);
	addSpriteTile(s,(0x8<<2),0x00BC,-5,-11);
	addSpriteTile(s,(0x8<<2),0x00BC,-3,-7);
	addSpriteTile(s,(0x8<<2)|1,0x00A8,12,-24);
	addSpriteTile(s,(0x8<<2),0x00BC,13,-11);
	addSpriteTile(s,(0x8<<2),0x00BC,11,-8);
	addSpriteTile(s,(0x8<<2)|1,0x00A8,0,-27);
	addSpriteTile(s,(0x8<<2),0x00BC,4,-13);
	addSpriteTile(s,(0x8<<2),0x00BC,4,-9);
	addSpriteTile(s,(0xC<<2),0x4128,-8,-8);
	addSpriteTile(s,(0xC<<2),0x4129,8,-8);
	addSpriteTile(s,(0xC<<2),0x4138,-8,8);
	addSpriteTile(s,(0xC<<2),0x4139,8,8);
}
//Baron Von Zeppelin
void drawSprite_17F(sprite_t * s) {
	addSpriteTile(s,(0x8<<2)|1,0x00A8,0,0);
	addSpriteTile(s,(0x8<<2),0x00BC,4,14);
	addSpriteTile(s,(0x8<<2),0x00BC,4,18);
}
//Spinning log
void drawSprite_180(sprite_t * s) {
	addSpriteTile(s,(0x8<<2),0x4046,-8,-8);
	addSpriteTile(s,(0x8<<2),0x4047,8,-8);
	addSpriteTile(s,(0x8<<2),0x4056,-8,8);
	addSpriteTile(s,(0x8<<2),0x4057,8,8);
}
//Crazee Dayzee
void drawSprite_181(sprite_t * s) {
	int base = findSpGfxFile(0x5D);
	int pal = (s->data[2]&1)?(0xC<<2):(0x9<<2);
	addSpriteTile(s,(0xC<<2),base+0x0A,1,8);
	addSpriteTile(s,pal|1,base+0x0C,-5,-10);
	addSpriteTile(s,pal|0xC1,base+0x0C,-1,-4);
	addSpriteTile(s,(0x8<<2)|1,base+0x0E,0,-2);
	addSpriteTile(s,(0xC<<2),base+0x0A,4,8);
}
//Ambient dragonfly
void drawSprite_182(sprite_t * s) {
	int base = findSpGfxFile(0x5D);
	addSpriteTile(s,(0x8<<2),base+0x02,0,0);
}
//Ambient butterfly
void drawSprite_183(sprite_t * s) {
	int base = findSpGfxFile(0x5D);
	addSpriteTile(s,(0x8<<2),base+0x03,0,0);
}
//Bumpty
void drawSprite_184(sprite_t * s) {
	int base = findSpGfxFile(0x5E);
	addSpriteTile(s,(0x8<<2)|1,base+0x0E,0,-1);
	addSpriteTile(s,(0x8<<2),base+0x1D,2,8);
	addSpriteTile(s,(0x8<<2),base+0x1D,6,8);
}
//Flatbed Ferry, green
void drawSprite_185(sprite_t * s) {
	addSpriteTile(s,(0x8<<2)|1,0x0028,-16,0);
	addSpriteTile(s,(0x8<<2)|1,0x0029,0,0);
	addSpriteTile(s,(0x8<<2)|1,0x002A,16,0);
}
//Flatbed Ferry, yellow
void drawSprite_187(sprite_t * s) {
	addSpriteTile(s,(0xA<<2)|1,0x0028,-16,0);
	addSpriteTile(s,(0xA<<2)|1,0x0029,0,0);
	addSpriteTile(s,(0xA<<2)|1,0x002A,16,0);
}
//Whorl lift
void drawSprite_18F(sprite_t * s) {
	addSpriteTile(s,(0x8<<2),0x4500,-8,-8);
	addSpriteTile(s,(0x8<<2),0x4501,8,-8);
	addSpriteTile(s,(0x8<<2),0x4510,-8,8);
	addSpriteTile(s,(0x8<<2),0x4511,8,8);
}
//Falling icicle
void drawSprite_190(sprite_t * s) {
	int base = findSpGfxFile(0x5F);
	addSpriteTile(s,(0xC<<2)|1,base+0x04,0,16);
	addSpriteTile(s,(0xC<<2)|1,base+0x06,0,0);
}
//Ambient bird
void drawSprite_191(sprite_t * s) {
	int base = findSpGfxFile(0x5E);
	addSpriteTile(s,(0x8<<2),base+0x00,4,4);
}
//Mufti Guy
void drawSprite_192(sprite_t * s) {
	int offset = ((s->data[2]&1)|(s->data[1]&2))<<1;
	int pal = ((romBuf[0x0209B8+offset]-1)<<1)|0x20;
	int base = findSpGfxFile(0x5D);
	addSpriteTile(s,(0x9<<2)|0x41,base+0x08,4,-10);
	addSpriteTile(s,(0x9<<2)|1,base+0x06,-4,-10);
	addSpriteTile(s,pal|1,base+0x04,0,-3);
	addSpriteTile(s,(0x8<<2),base+0x0A,1,8);
	addSpriteTile(s,(0x8<<2),base+0x0A,7,8);
}
//Caged Ghost squeezed in tunnel
void drawSprite_193(sprite_t * s) {
	drawSpriteHDMAPolygon(s,&romBuf[0x035A61],0x8400,27,true,false,0,-0x30);
	int base = findSpGfxFile(0x42);
	addSpriteTile(s,(0x9<<2)|1,base+0x20,-2,5);
	addSpriteTile(s,(0x9<<2)|1,base+0x22,14,5);
	addSpriteTile(s,(0x9<<2)|1,base+0x02,14,-11);
	addSpriteTile(s,(0x9<<2)|1,base+0x00,-2,-11);
	addSpriteTile(s,(0x9<<2),base+0x3B,4,-20);
	addSpriteTile(s,(0x9<<2),base+0x3B,14,-20);
	addSpriteTile(s,(0x9<<2)|1,base+0x24,-2,-29);
	addSpriteTile(s,(0x9<<2)|1,base+0x26,11,-29);
}
//Blargg
void drawSprite_194(sprite_t * s) {
	int base = findSpGfxFile(0x62);
	addSpriteTile(s,(0x9<<2)|1,base+0x04,-8,0);
	addSpriteTile(s,(0x9<<2)|1,base+0x06,8,0);
	addSpriteTile(s,(0x9<<2)|1,base+0x00,-8,-6);
	addSpriteTile(s,(0x9<<2)|1,base+0x02,8,-6);
	addSpriteTile(s,(0x9<<2),base+0x0E,1,-8);
	addSpriteTile(s,(0x9<<2)|0x40,base+0x0F,-3,-15);
	addSpriteTile(s,(0x9<<2),base+0x0F,5,-15);
}
//Small snowy platform
void drawSprite_195(sprite_t * s) {
	addSpriteTile(s,(0xF<<2),0x4522,-16,-16);
	addSpriteTile(s,(0xF<<2),0x4523,0,-16);
	addSpriteTile(s,(0xF<<2),0x4532,-16,0);
	addSpriteTile(s,(0xF<<2),0x4533,0,0);
}
//Snowy platform
void drawSprite_196(sprite_t * s) {
	for(int j=0; j<4; j++) {
		for(int i=0; i<4; i++) {
			int offsX = -32+(i<<4);
			int offsY = -32+(j<<4);
			addSpriteTile(s,(0xF<<2),0x4540+i+(j<<4),offsX,offsY);
		}
	}
}
//Arrow sign, cardinal
void drawSprite_197(sprite_t * s) {
	int offsY = (s->data[1]&2);
	int base = findSpGfxFile(0x1C);
	if(s->data[2]&1) {
		if(offsY) {
			addSpriteTile(s,(0x9<<2)|0xC1,base+0x0B,-8,-4);
			addSpriteTile(s,(0x9<<2)|0xC0,base+0x1A,-8,-12);
			addSpriteTile(s,(0x9<<2),base+0x0A,0,-12);
		} else {
			addSpriteTile(s,(0x9<<2)|0x41,base+0x08,-4,8);
			addSpriteTile(s,(0x9<<2)|0x40,base+0x0A,-12,8);
			addSpriteTile(s,(0x9<<2)|0x40,base+0x1A,-12,16);
		}
	} else {
		if(offsY) {
			addSpriteTile(s,(0x9<<2)|0x81,base+0x08,4,-8);
			addSpriteTile(s,(0x9<<2)|0x80,base+0x0A,20,0);
			addSpriteTile(s,(0x9<<2)|0x80,base+0x1A,20,-8);
		} else {
			addSpriteTile(s,(0x9<<2)|1,base+0x0B,8,4);
			addSpriteTile(s,(0x9<<2),base+0x1A,16,20);
			addSpriteTile(s,(0x9<<2)|0xC0,base+0x0A,8,20);
		}
	}
}
//Arrow sign, diagonal
void drawSprite_198(sprite_t * s) {
	int offsY = (s->data[1]&2);
	int base = findSpGfxFile(0x1C);
	if(s->data[2]&1) {
		if(offsY) {
			addSpriteTile(s,(0x9<<2)|0xC1,base+0x0D,-6,-6);
			addSpriteTile(s,(0x9<<2)|0xC0,base+0x0F,-14,-6);
			addSpriteTile(s,(0x9<<2)|0xC0,base+0x1F,-6,-14);
		} else {
			addSpriteTile(s,(0x9<<2)|0x41,base+0x0D,-6,6);
			addSpriteTile(s,(0x9<<2)|0x40,base+0x0F,-14,14);
			addSpriteTile(s,(0x9<<2)|0x40,base+0x1F,-6,22);
		}
	} else {
		if(offsY) {
			addSpriteTile(s,(0x9<<2)|0x81,base+0x0D,6,-6);
			addSpriteTile(s,(0x9<<2)|0x80,base+0x0F,22,-6);
			addSpriteTile(s,(0x9<<2)|0x80,base+0x1F,14,-2);
		} else {
			addSpriteTile(s,(0x9<<2)|1,base+0x0D,6,6);
			addSpriteTile(s,(0x9<<2),base+0x0F,22,14);
			addSpriteTile(s,(0x9<<2),base+0x1F,14,22);
		}
	}
}
//Dizzy Dandy
void drawSprite_199(sprite_t * s) {
	addSpriteTile(s,(0x9<<2),0x4524,-8,-8);
	addSpriteTile(s,(0x9<<2),0x4525,8,-8);
	addSpriteTile(s,(0x9<<2),0x4534,-8,8);
	addSpriteTile(s,(0x9<<2),0x4535,8,8);
}
//Boo Guy
void drawSprite_19A(sprite_t * s) {
	int base = findSpGfxFile(0x60);
	addSpriteTile(s,(0x8<<2)|1,base+0x00,0,4);
	addSpriteTile(s,(0x8<<2),base+0x02,13,11);
}
//Flying Bumpty
void drawSprite_19C(sprite_t * s) {
	int base = findSpGfxFile(0x5E);
	addSpriteTile(s,(0x8<<2),base+0x0C,3,13);
	addSpriteTile(s,(0x8<<2),base+0x0C,8,13);
	addSpriteTile(s,(0x8<<2)|1,base+0x08,0,0);
	addSpriteTile(s,(0x8<<2),base+0x0A,8,2);
	addSpriteTile(s,(0x8<<2),base+0x0B,12,-2);
}
//Skeleton Goonie
void drawSprite_19D(sprite_t * s) {
	int base = findSpGfxFile(0x63);
	addSpriteTile(s,(0xA<<2)|0x41,base+0x01,-7,-19);
	addSpriteTile(s,(0xA<<2)|0x40,base+0x09,3,-4);
	addSpriteTile(s,(0xA<<2),base+0x0E,-8,4);
	addSpriteTile(s,(0xA<<2),base+0x1E,12,11);
	addSpriteTile(s,(0xA<<2),base+0x0F,14,5);
	addSpriteTile(s,(0xA<<2)|1,base+0x0C,0,0);
	addSpriteTile(s,(0xA<<2)|1,base+0x01,11,-19);
	addSpriteTile(s,(0xA<<2),base+0x09,9,-4);
}
//Flightless Skeleton Goonie
void drawSprite_19E(sprite_t * s) {
	int base = findSpGfxFile(0x63);
	addSpriteTile(s,(0xA<<2)|1,base+0x0C,0,-1);
	addSpriteTile(s,(0xA<<2),base+0x0E,-8,3);
	addSpriteTile(s,(0xA<<2),base+0x0A,4,12);
	addSpriteTile(s,(0xA<<2),base+0x0B,14,11);
}
//Skeleton Goonie with bomb
void drawSprite_19F(sprite_t * s) {
	int base = findSpGfxFile(0x63);
	addSpriteTile(s,(0xA<<2)|0x41,base+0x01,-7,-19);
	addSpriteTile(s,(0xA<<2)|0x40,base+0x09,3,-4);
	addSpriteTile(s,(0xA<<2),base+0x0E,-8,4);
	addSpriteTile(s,(0xA<<2),base+0x1A,4,13);
	addSpriteTile(s,(0xA<<2),base+0x1A,8,13);
	addSpriteTile(s,(0xA<<2)|1,base+0x0C,0,0);
	addSpriteTile(s,(0xA<<2)|1,base+0x01,11,-19);
	addSpriteTile(s,(0xA<<2),base+0x09,9,-4);
	addSpriteTile(s,(0xC<<2)|1,0x00EC,1,17);
}
//Firebar, 2 end
void drawSprite_1A0(sprite_t * s) {
	int base = findSpGfxFile(0x65);
	addSpriteTile(s,(0x9<<2)|1,base+0x00,-80,0);
	addSpriteTile(s,(0x9<<2)|1,base+0x00,-56,0);
	addSpriteTile(s,(0x9<<2)|1,base+0x00,-32,0);
	addSpriteTile(s,(0x9<<2)|1,base+0x00,-8,0);
	addSpriteTile(s,(0x9<<2)|1,base+0x00,16,0);
	addSpriteTile(s,(0x9<<2)|1,base+0x00,40,0);
	addSpriteTile(s,(0x9<<2)|1,base+0x00,64,0);
}
//Firebar, 1 end
void drawSprite_1A1(sprite_t * s) {
	int base = findSpGfxFile(0x65);
	addSpriteTile(s,(0x9<<2)|1,base+0x00,-80,0);
	addSpriteTile(s,(0x9<<2)|1,base+0x00,-56,0);
	addSpriteTile(s,(0x9<<2)|1,base+0x00,-32,0);
	addSpriteTile(s,(0x9<<2)|1,base+0x00,-8,0);
}
//Star
void drawSprite_1A2(sprite_t * s) {
	addSpriteTile(s,(0xA<<2)|1,0x00AA,0,-2);
	addSpriteTile(s,(0x8<<2),0x009E,0,8);
	addSpriteTile(s,(0x8<<2)|0x40,0x009E,7,8);
}
//Little Skull Mouser
void drawSprite_1A3(sprite_t * s) {
	int base = findSpGfxFile(0x60);
	addSpriteTile(s,(0xE<<2),base+0x03,1,8);
	addSpriteTile(s,(0xE<<2),base+0x03,9,8);
	addSpriteTile(s,(0xE<<2),base+0x14,15,4);
	addSpriteTile(s,(0xE<<2)|1,base+0x05,0,-2);
}
//Cork
void drawSprite_1A4(sprite_t * s) {
	int base = findSpGfxFile(0x48);
	addSpriteTile(s,(0xE<<2)|1,base+0x08,0,-32);
	addSpriteTile(s,(0xE<<2)|0x41,base+0x08,16,-32);
	addSpriteTile(s,(0xE<<2)|1,base+0x0A,0,-16);
	addSpriteTile(s,(0xE<<2)|0x41,base+0x0A,16,-16);
	addSpriteTile(s,(0xE<<2)|1,base+0x0C,0,0);
	addSpriteTile(s,(0xE<<2)|0x41,base+0x0C,16,0);
}
//Grinder
void drawSprite_1A5(sprite_t * s) {
	int base = findSpGfxFile(0x64);
	addSpriteTile(s,(0x8<<2),base+0x0C,0,8);
	addSpriteTile(s,(0x8<<2),base+0x1B,9,7);
	addSpriteTile(s,(0x8<<2)|1,base+0x00,0,-7);
	addSpriteTile(s,(0x8<<2),base+0x1F,9,12);
	addSpriteTile(s,(0x8<<2),base+0x0A,16,4);
	addSpriteTile(s,(0x8<<2),base+0x0C,3,8);
}
//Hot Lips
void drawSprite_1AA(sprite_t * s) {
	int base = findSpGfxFile(0x66);
	addSpriteTile(s,(0x9<<2)|1,base+0x00,-8,0);
	addSpriteTile(s,(0x9<<2)|1,base+0x06,8,0);
	addSpriteTile(s,(0x9<<2),base+0x0D,1,-4);
	addSpriteTile(s,(0x9<<2),base+0x0D,8,-4);
}
//Boo Balloon
void drawSprite_1AB(sprite_t * s) {
	int base = findSpGfxFile(0x65);
	addSpriteTile(s,(0x8<<2)|1,base+0x04,0,0);
	addSpriteTile(s,(0x8<<2),base+0x0C,4,16);
	addSpriteTile(s,(0x8<<2),base+0x1C,4,24);
}
//Ambient frog
void drawSprite_1AC(sprite_t * s) {
	int base = findSpGfxFile(0x68);
	addSpriteTile(s,(0x8<<2)|0x40,base+0x0E,4,8);
}
//Kamek magic
void drawSprite_1AE(sprite_t * s) {
	int base = findSpGfxFile(0x67);
	addSpriteTile(s,(0xA<<2),base+0x0E,-1,3);
	addSpriteTile(s,(0x9<<2),base+0x0F,5,-1);
	addSpriteTile(s,(0xB<<2),base+0x1E,6,7);
}
//Large balloon platform
void drawSprite_1B0(sprite_t * s) {
	int base = findSpGfxFile(0x65);
	addSpriteTile(s,(9<<2)|1,base+0x0E,0,0);
	BYTE tempBuf[44*2];
	for(int n=0; n<88; n++) {
		tempBuf[n] = romBuf[0x066E64+(n<<1)];
	}
	drawSpriteHDMAPolygon(s,tempBuf,0x8400,44,true,false,14,6);
}
//Minigame coin cannon
void drawSprite_1B1(sprite_t * s) {
	addSpriteTile(s,(0x8<<2),0x4108,0,0);
	addSpriteTile(s,(0x8<<2),0x4109,16,0);
	addSpriteTile(s,(0x8<<2),0x4118,0,16);
	addSpriteTile(s,(0x8<<2),0x4119,16,16);
	addSpriteTile(s,(0x8<<2)|0xC1,0x0122,16,32);
	addSpriteTile(s,(0x8<<2)|1,0x0122,0,32);
	addSpriteTile(s,(0x8<<2)|0x40,0x0130,24,24);
	addSpriteTile(s,(0x8<<2)|0x40,0x0131,16,24);
	addSpriteTile(s,(0x8<<2),0x0131,8,24);
	addSpriteTile(s,(0x8<<2),0x0130,0,24);
	addSpriteTile(s,(0x8<<2)|0x40,0x0121,16,16);
	addSpriteTile(s,(0x8<<2),0x0121,8,16);
	addSpriteTile(s,(0x8<<2)|0x40,0x0120,16,8);
	addSpriteTile(s,(0x8<<2),0x0120,8,8);
}
//Minigame checker platform
void drawSprite_1B4(sprite_t * s) {
	addSpriteTile(s,(0x8<<2)|0xC1,0x0122,16,0);
	addSpriteTile(s,(0x8<<2)|1,0x0122,0,0);
}
//Minigame balloon
void drawSprite_1B6(sprite_t * s) {
	addSpriteTile(s,(0x9<<2),0x44CE,0,0);
	addSpriteTile(s,(0x9<<2),0x44CF,16,0);
	addSpriteTile(s,(0x9<<2),0x44DE,0,16);
	addSpriteTile(s,(0x9<<2),0x44DF,16,16);
}
//Minigame melon pot
void drawSprite_1B8(sprite_t * s) {
	drawSpriteText(s," Minigame \nMelon  Pot");
}
//Graphics/palette changer command
void drawSprite_1BA(sprite_t * s) {
	char spStr[256];
	int spRef = (s->data[0]-0xBA);
	snprintf(spStr,256,"Graphics/Palette\n   Changer %02X   ",spRef);
	drawSpriteText(s,spStr);
}
//Special auto-scroll command
void drawSprite_1CA(sprite_t * s) {
	char spStr[256];
	int spRef = (s->data[0]-0xCA);
	snprintf(spStr,256,"    Special    \nAuto-Scroll  %02X",spRef);
	drawSpriteText(s,spStr);
}
//Boo Balloon end command
void drawSprite_1D5(sprite_t * s) {
	drawSpriteText(s,"Boo Balloon\n    End    ");
}
//Lock horizontal scroll
void drawSprite_1D6(sprite_t * s) {
	drawSpriteText(s,"Lock  Scroll\n Horizontal ");
}
//Gusty generator command
void drawSprite_1D7(sprite_t * s) {
	drawSpriteText(s,"  Gusty  \nGenerator");
}
//Gusty generator end command
void drawSprite_1D8(sprite_t * s) {
	drawSpriteText(s,"    Gusty    \nGenerator End");
}
//Lakitu end command
void drawSprite_1D9(sprite_t * s) {
	drawSpriteText(s,"Lakitu\n End  ");
}
//Fuzzy end command
void drawSprite_1DA(sprite_t * s) {
	drawSpriteText(s,"    Fuzzy    \nGenerator End");
}
//Poochy end command
void drawSprite_1DB(sprite_t * s) {
	drawSpriteText(s,"Poochy\n End  ");
}
//Fang generator command
void drawSprite_1DC(sprite_t * s) {
	drawSpriteText(s,"  Fang   \nGenerator");
}
//Fang end command
void drawSprite_1DD(sprite_t * s) {
	drawSpriteText(s,"     Fang     \nGenerator  End");
}
//Fang 2 generator command
void drawSprite_1DE(sprite_t * s) {
	drawSpriteText(s," Fang  2 \nGenerator");
}
//Fang 2 end command
void drawSprite_1DF(sprite_t * s) {
	drawSpriteText(s,"   Fang  2   \nGenerator End");
}
//Wall Lakitu generator command
void drawSprite_1E0(sprite_t * s) {
	drawSpriteText(s,"Wall Lakitu\n Generator ");
}
//Wall Lakitu end command
void drawSprite_1E1(sprite_t * s) {
	drawSpriteText(s," Wall Lakitu \nGenerator End");
}
//Dancing Spear Guy command
void drawSprite_1E2(sprite_t * s) {
	char spStr[256];
	int spRef = (s->data[2]&1)|(s->data[1]&2);
	snprintf(spStr,256,"Dancing Spear Guy\n     Dance %d     ",spRef);
	drawSpriteText(s,spStr);
}
//Dancing Spear Guy end command
void drawSprite_1E3(sprite_t * s) {
	drawSpriteText(s,"Dancing Spear Guy\n    End Dance    ");
}
//Thunder Lakitu end command
void drawSprite_1E4(sprite_t * s) {
	drawSpriteText(s,"Thunder  Lakitu\n      End      ");
}
//Flutter generator command
void drawSprite_1E5(sprite_t * s) {
	drawSpriteText(s," Flutter \nGenerator");
}
//Flutter end command
void drawSprite_1E6(sprite_t * s) {
	drawSpriteText(s,"   Flutter   \nGenerator End");
}
//Nipper Spore generator command
void drawSprite_1E7(sprite_t * s) {
	drawSpriteText(s,"Nipper  Spore\n  Generator  ");
}
//Nipper Spore end command
void drawSprite_1E8(sprite_t * s) {
	drawSpriteText(s,"Nipper  Spore\nGenerator End");
}
//Baron Von Zeppelin, Needlenose generator command
void drawSprite_1E9(sprite_t * s) {
	drawSpriteText(s,"Baron Von Zeppelin (Needlenose)\n           Generator           ");
}
//Baron Von Zeppelin, Needlenose end command
void drawSprite_1EA(sprite_t * s) {
	drawSpriteText(s,"Baron Von Zeppelin (Needlenose)\n         Generator End         ");
}
//Baron Von Zeppelin, bomb generator command
void drawSprite_1EB(sprite_t * s) {
	drawSpriteText(s,"Baron Von Zeppelin (Bomb)\n        Generator        ");
}
//Baron Von Zeppelin, bomb end command
void drawSprite_1EC(sprite_t * s) {
	drawSpriteText(s,"Baron Von Zeppelin (Bomb)\n      Generator End      ");
}
//Balloon platform generator command
void drawSprite_1ED(sprite_t * s) {
	drawSpriteText(s,"Balloon  Platform\n    Generator    ");
}
//Balloon platform end command
void drawSprite_1EE(sprite_t * s) {
	drawSpriteText(s,"Balloon  Platform\n  Generator End  ");
}
//Flatbed Ferry, yellow generator command
void drawSprite_1EF(sprite_t * s) {
	drawSpriteText(s,"Flatbed Ferry  (4 Yellow)\n        Generator        ");
}
//Lemon Drop generator command
void drawSprite_1F0(sprite_t * s) {
	drawSpriteText(s,"Lemon  Drop\n Generator ");
}
//Lemon Drop end command
void drawSprite_1F1(sprite_t * s) {
	drawSpriteText(s," Lemon  Drop \nGenerator End");
}
//Fuzzy effect end
void drawSprite_1F2(sprite_t * s) {
	drawSpriteText(s,"Fuzzy  Effect\n     End     ");
}
//Goonie end command
void drawSprite_1F3(sprite_t * s) {
	drawSpriteText(s,"Goonie\n End  ");
}
//Fuzzy generator command
void drawSprite_1F4(sprite_t * s) {
	drawSpriteText(s,"  Fuzzy  \nGenerator");
}

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
	drawSprite_038,drawSprite_039,drawSprite_03A,drawSprite_03A,
	drawSprite_03C,drawSprite_03D,drawSprite_03E,drawSprite_03F,
	//040
	drawSprite_040,drawSprite_041,drawSprite_042,drawSprite_043,
	drawSprite_043,drawSprite_045,drawSprite_046,drawSprite_047,
	drawSprite_048,drawSprite_049,drawSprite_049,drawSprite_049,
	drawSprite_04C,drawSprite_04D,drawSprite_04E,drawSprite_04F,
	//050
	drawSprite_050,drawSprite_051,drawSprite_052,drawSprite_048,
	drawSprite_054,drawSprite_055,drawSprite_056,drawSprite_057,
	drawSprite_058,drawSprite_059,drawSprite_05A,drawSprite_05B,
	drawSprite_058,drawSprite_unused,drawSprite_05E,drawSprite_05E,
	//060
	drawSprite_060,drawSprite_061,drawSprite_062,drawSprite_063,
	drawSprite_064,drawSprite_065,drawSprite_066,drawSprite_067,
	drawSprite_068,drawSprite_069,drawSprite_06A,drawSprite_06B,
	drawSprite_06C,drawSprite_06D,drawSprite_06D,drawSprite_06F,
	//070
	drawSprite_070,drawSprite_071,drawSprite_072,drawSprite_073,
	drawSprite_074,drawSprite_075,drawSprite_076,drawSprite_076,
	drawSprite_078,drawSprite_078,drawSprite_078,drawSprite_07B,
	drawSprite_07B,drawSprite_07B,drawSprite_07E,drawSprite_07F,
	//080
	drawSprite_080,drawSprite_080,drawSprite_082,drawSprite_083,
	drawSprite_084,drawSprite_085,drawSprite_unused,drawSprite_087,
	drawSprite_059,drawSprite_089,drawSprite_08A,drawSprite_08B,
	drawSprite_08C,drawSprite_08D,drawSprite_08E,drawSprite_08F,
	//090
	drawSprite_090,drawSprite_091,drawSprite_092,drawSprite_093,
	drawSprite_094,drawSprite_095,drawSprite_095,drawSprite_097,
	drawSprite_098,drawSprite_099,drawSprite_09A,drawSprite_09B,
	drawSprite_09C,drawSprite_09D,drawSprite_09E,drawSprite_09F,
	//0A0
	drawSprite_0A0,drawSprite_0A1,drawSprite_0A2,drawSprite_020,
	drawSprite_020,drawSprite_0A5,drawSprite_0A6,drawSprite_0A7,
	drawSprite_0A8,drawSprite_0A9,drawSprite_0AA,drawSprite_0AB,
	drawSprite_0AC,drawSprite_0AD,drawSprite_0AE,drawSprite_0AF,
	//0B0
	drawSprite_0B0,drawSprite_0B1,drawSprite_0B2,drawSprite_0B3,
	drawSprite_0B4,drawSprite_067,drawSprite_067,drawSprite_067,
	drawSprite_067,drawSprite_067,drawSprite_067,drawSprite_067,
	drawSprite_067,drawSprite_067,drawSprite_067,drawSprite_067,
	//0C0
	drawSprite_067,drawSprite_067,drawSprite_067,drawSprite_067,
	drawSprite_067,drawSprite_067,drawSprite_067,drawSprite_067,
	drawSprite_067,drawSprite_067,drawSprite_012,drawSprite_067,
	drawSprite_067,drawSprite_0CD,drawSprite_0CE,drawSprite_0CF,
	//0D0
	drawSprite_0D0,drawSprite_0D1,drawSprite_0D2,drawSprite_0D3,
	drawSprite_0D4,drawSprite_0D5,drawSprite_0D6,drawSprite_0D7,
	drawSprite_0D8,drawSprite_0D9,drawSprite_0DA,drawSprite_0DB,
	drawSprite_0DC,drawSprite_0DD,drawSprite_0DE,drawSprite_0DF,
	//0E0
	drawSprite_0E0,drawSprite_0E1,drawSprite_0E2,drawSprite_0E3,
	drawSprite_0E4,drawSprite_0E5,drawSprite_0E6,drawSprite_0E7,
	drawSprite_0E8,drawSprite_0E9,drawSprite_0EA,drawSprite_0EB,
	drawSprite_0EC,drawSprite_0EC,drawSprite_0EE,drawSprite_0EF,
	//0F0
	drawSprite_0F0,drawSprite_0F1,drawSprite_0F2,drawSprite_0F3,
	drawSprite_0F1,drawSprite_0F5,drawSprite_0F6,drawSprite_0F7,
	drawSprite_0F8,drawSprite_0F9,drawSprite_0FA,drawSprite_0FB,
	drawSprite_0FC,drawSprite_0FD,drawSprite_0FE,drawSprite_0FF,
	//100
	drawSprite_100,drawSprite_101,drawSprite_102,drawSprite_103,
	drawSprite_104,drawSprite_105,drawSprite_105,drawSprite_107,
	drawSprite_108,drawSprite_109,drawSprite_109,drawSprite_109,
	drawSprite_10C,drawSprite_10D,drawSprite_003,drawSprite_10F,
	//110
	drawSprite_110,drawSprite_111,drawSprite_112,drawSprite_113,
	drawSprite_114,drawSprite_115,drawSprite_116,drawSprite_117,
	drawSprite_118,drawSprite_119,drawSprite_11A,drawSprite_11B,
	drawSprite_11C,drawSprite_11D,drawSprite_11E,drawSprite_11E,
	//120
	drawSprite_120,drawSprite_121,drawSprite_021,drawSprite_021,
	drawSprite_124,drawSprite_048,drawSprite_126,drawSprite_127,
	drawSprite_128,drawSprite_129,drawSprite_12A,drawSprite_12B,
	drawSprite_12C,drawSprite_12D,drawSprite_12E,drawSprite_12F,
	//130
	drawSprite_130,drawSprite_04E,drawSprite_132,drawSprite_133,
	drawSprite_134,drawSprite_135,drawSprite_135,drawSprite_137,
	drawSprite_138,drawSprite_139,drawSprite_13A,drawSprite_13B,
	drawSprite_13C,drawSprite_13D,drawSprite_13D,drawSprite_13F,
	//140
	drawSprite_13F,drawSprite_13F,drawSprite_13F,drawSprite_143,
	drawSprite_144,drawSprite_145,drawSprite_145,drawSprite_147,
	drawSprite_06C,drawSprite_149,drawSprite_14A,drawSprite_14B,
	drawSprite_14C,drawSprite_14D,drawSprite_14E,drawSprite_14F,
	//150
	drawSprite_150,drawSprite_150,drawSprite_152,drawSprite_153,
	drawSprite_154,drawSprite_155,drawSprite_156,drawSprite_157,
	drawSprite_158,drawSprite_159,drawSprite_159,drawSprite_15B,
	drawSprite_15C,drawSprite_15C,drawSprite_15E,drawSprite_15F,
	//160
	drawSprite_15F,drawSprite_161,drawSprite_162,drawSprite_0E5,
	drawSprite_164,drawSprite_165,drawSprite_166,drawSprite_167,
	drawSprite_167,drawSprite_169,drawSprite_169,drawSprite_16B,
	drawSprite_16B,drawSprite_16D,drawSprite_16D,drawSprite_16D,
	//170
	drawSprite_170,drawSprite_171,drawSprite_172,drawSprite_173,
	drawSprite_174,drawSprite_175,drawSprite_176,drawSprite_177,
	drawSprite_178,drawSprite_179,drawSprite_17A,drawSprite_17B,
	drawSprite_17C,drawSprite_17D,drawSprite_17E,drawSprite_17F,
	//180
	drawSprite_180,drawSprite_181,drawSprite_182,drawSprite_183,
	drawSprite_184,drawSprite_185,drawSprite_185,drawSprite_187,
	drawSprite_187,drawSprite_185,drawSprite_185,drawSprite_187,
	drawSprite_187,drawSprite_089,drawSprite_089,drawSprite_18F,
	//190
	drawSprite_190,drawSprite_191,drawSprite_192,drawSprite_193,
	drawSprite_194,drawSprite_195,drawSprite_196,drawSprite_197,
	drawSprite_198,drawSprite_199,drawSprite_19A,drawSprite_184,
	drawSprite_19C,drawSprite_19D,drawSprite_19E,drawSprite_19F,
	//1A0
	drawSprite_1A0,drawSprite_1A1,drawSprite_1A2,drawSprite_1A3,
	drawSprite_1A4,drawSprite_1A5,drawSprite_1A5,drawSprite_1A5,
	drawSprite_1A5,drawSprite_1A5,drawSprite_1AA,drawSprite_1AB,
	drawSprite_1AC,drawSprite_08E,drawSprite_unused,drawSprite_115,
	//1B0
	drawSprite_1B0,drawSprite_1B1,drawSprite_115,drawSprite_020,
	drawSprite_1B4,drawSprite_020,drawSprite_1B6,drawSprite_020,
	drawSprite_1B8,drawSprite_020,drawSprite_1BA,drawSprite_1BA,
	drawSprite_1BA,drawSprite_1BA,drawSprite_1BA,drawSprite_1BA,
	//1C0
	drawSprite_1BA,drawSprite_1BA,drawSprite_1BA,drawSprite_1BA,
	drawSprite_1BA,drawSprite_1BA,drawSprite_1BA,drawSprite_1BA,
	drawSprite_1BA,drawSprite_1BA,drawSprite_1CA,drawSprite_1CA,
	drawSprite_1CA,drawSprite_1CA,drawSprite_1CA,drawSprite_1CA,
	//1D0
	drawSprite_1CA,drawSprite_1CA,drawSprite_1CA,drawSprite_1CA,
	drawSprite_1CA,drawSprite_1D5,drawSprite_1D6,drawSprite_1D7,
	drawSprite_1D8,drawSprite_1D9,drawSprite_1DA,drawSprite_1DB,
	drawSprite_1DC,drawSprite_1DD,drawSprite_1DE,drawSprite_1DF,
	//1E0
	drawSprite_1E0,drawSprite_1E1,drawSprite_1E2,drawSprite_1E3,
	drawSprite_1E4,drawSprite_1E5,drawSprite_1E6,drawSprite_1E7,
	drawSprite_1E8,drawSprite_1E9,drawSprite_1EA,drawSprite_1EB,
	drawSprite_1EC,drawSprite_1ED,drawSprite_1EE,drawSprite_1EF,
	//1F0
	drawSprite_1F0,drawSprite_1F1,drawSprite_1F2,drawSprite_1F3,
	drawSprite_1F4,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused};
int setSpriteContext(int ctx) {
	int prevCtx = curSpCtx;
	curSpCtx = ctx;
	return prevCtx;
}
void drawSprites() {
	//Clear buffers
	for(int i=0; i<0x8000; i++) {
		spriteContexts[curSpCtx].assocSprites[i].clear();
		spriteContexts[curSpCtx].invalidSprites[i] = false;
	}
	//Draw sprites
	for(int n = 0; n < spriteContexts[curSpCtx].sprites.size(); n++) {
		sprite_t * thisSprite = &spriteContexts[curSpCtx].sprites[n];
		thisSprite->tiles.clear();
		thisSprite->occupiedTiles.clear();
		int id = thisSprite->data[0]|(thisSprite->data[1]<<8);
		id &= 0x1FF;
		spriteDrawFunc[id](thisSprite);
		//Draw text for sprites which have no tiles
		if(thisSprite->occupiedTiles.size()==0) {
			char spStr[256];
			snprintf(spStr,256,"S%01X\n%02X",id>>16,id&0xFF);
			drawSpriteText(thisSprite,spStr);
		}
	}
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
					if(tile&0x2000) dispMap8Tile(pixelBuf,width,height,props,tile,{sptX,sptY},thisSprite->selected);
					else dispMap8Tile(pixelBuf,width,height,props,tile+0x480,{sptX,sptY},thisSprite->selected);
					break;
				}
				case 0x4000: {
					dispSuperFXTexture(pixelBuf,width,height,props,tile&0x3FFF,{sptX,sptY},thisSprite->selected);
					break;
				}
				case 0x8000: {
					dispBackgroundRow(pixelBuf,width,height,tile&0x7FF,{sptX,sptY},thisSprite->selected);
					break;
				}
				case 0xC000: {
					dispMap8Char(pixelBuf,width,height,0xFF,0xFFFFFF,tile&0x7F,{sptX,sptY},thisSprite->selected);
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
		entry.occupiedTiles.clear();
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

//Manipulation
int selectSprites(RECT rect) {
	//Required by IntersectRect (we only care about the bool result)
	RECT dummyIntersect;
	//Check each sprite
	for(int n=0; n<spriteContexts[0].sprites.size(); n++) {
		sprite_t * thisSprite = &spriteContexts[0].sprites[n];
		thisSprite->selected = false;
		int xpos = (thisSprite->data[2])<<4;
		int ypos = (thisSprite->data[1]>>1)<<4;
		//Check each sprite tile for intersection
		for(int i=0; i<thisSprite->tiles.size(); i++) {
			sprite_tile_t * thisSpriteTile = &thisSprite->tiles[i];
			int xpos2 = xpos+thisSpriteTile->offsX;
			int ypos2 = ypos+thisSpriteTile->offsY;
			switch(thisSpriteTile->tile&0xC000) {
				case 0x0000: {
					int tileSize = (thisSpriteTile->props&1)?16:8;
					RECT tileRect = {xpos2,ypos2,xpos2+tileSize,ypos2+tileSize};
					thisSprite->selected = IntersectRect(&dummyIntersect,&rect,&tileRect);
					break;
				}
				case 0x4000: {
					RECT tileRect = {xpos2,ypos2,xpos2+16,ypos2+16};
					thisSprite->selected = IntersectRect(&dummyIntersect,&rect,&tileRect);
					break;
				}
				case 0x8000: {
					RECT tileRect = {xpos2,ypos2,xpos2+256,ypos2+1};
					thisSprite->selected = IntersectRect(&dummyIntersect,&rect,&tileRect);
					break;
				}
				case 0xC000: {
					RECT tileRect = {xpos2,ypos2,xpos2+8,ypos2+8};
					thisSprite->selected = IntersectRect(&dummyIntersect,&rect,&tileRect);
					break;
				}
			}
			if(thisSprite->selected) break;
		}
	}
}
void clearSpriteSelection() {
	//Deselect all sprites
	for(int n=0; n<spriteContexts[0].sprites.size(); n++) {
		sprite_t * thisSprite = &spriteContexts[0].sprites[n];
		thisSprite->selected = false;
	}
}
void insertSprites(int x,int y) {
	x >>= 4;
	y >>= 4;
	int numSelectedSprites = 0;
	int minX = 0x8000,minY = 0x8000;
	int maxX = 0,maxY = 0;
	//Check if any sprites are to be pasted
	for(int n=0; n<spriteContexts[0].sprites.size(); n++) {
		sprite_t * thisSprite = &spriteContexts[0].sprites[n];
		if(thisSprite->selected) {
			int xpos = thisSprite->data[2];
			int ypos = thisSprite->data[1]>>1;
			minX = std::min(xpos,minX);
			minY = std::min(ypos,minY);
			maxX = std::max(xpos,maxX);
			maxY = std::max(ypos,maxY);
			numSelectedSprites++;
		}
	}
	if(numSelectedSprites) {
		//Determine if any sprites will be out of bounds after this operation,
		//and if so, terminate
		if(x<0 || y<0 || (maxX-minX+x)>=0x100 || (maxY-minY+y)>=0x80) return;
		//Paste selected sprites
		int origSize = spriteContexts[0].sprites.size();
		for(int n=0; n<origSize; n++) {
			sprite_t * thisSprite = &spriteContexts[0].sprites[n];
			if(thisSprite->selected) {
				thisSprite->selected = false;
				int xpos = thisSprite->data[2];
				int ypos = thisSprite->data[1]>>1;
				xpos = x+(xpos-minX);
				ypos = y+(ypos-minY);
				sprite_t entry;
				entry.data[0] = thisSprite->data[0];
				entry.data[1] = (thisSprite->data[1]&1)|(ypos<<1);
				entry.data[2] = xpos;
				entry.dataSize = 3;
				entry.selected = true;
				spriteContexts[0].sprites.push_back(entry);
			}
		}
	} else {
		//Determine if any sprites will be out of bounds after this operation,
		//and if so, terminate
		if(x<0 || y<0 || x>=0x100 || y>=0x80) return;
		//Insert current sprite in selection dialog
		if(spriteContexts[1].sprites.size()) {
			sprite_t * thisSprite = &spriteContexts[1].sprites[0];
			sprite_t entry;
			entry.data[0] = thisSprite->data[0];
			entry.data[1] = (thisSprite->data[1]&1)|(y<<1);
			entry.data[2] = x;
			entry.dataSize = 3;
			entry.selected = true;
			spriteContexts[0].sprites.push_back(entry);
		}
	}
}
void deleteSprites() {
	//Delete selected sprites
	remove_if(spriteContexts[0].sprites.begin(),spriteContexts[0].sprites.end(),sprite_delPred);
}
void selectTopSprite(int x,int y) {
	//Select top sprite
	for(int n=(spriteContexts[0].sprites.size()-1); n>=0; n--) {
		sprite_t * thisSprite = &spriteContexts[0].sprites[n];
		int id = thisSprite->data[0]|(thisSprite->data[1]<<8);
		id &= 0x1FF;
		int xpos = (thisSprite->data[2])<<4;
		int ypos = (thisSprite->data[1]>>1)<<4;
		//Check each sprite tile for intersection
		for(int i=0; i<thisSprite->tiles.size(); i++) {
			sprite_tile_t * thisSpriteTile = &thisSprite->tiles[i];
			int xpos2 = xpos+thisSpriteTile->offsX;
			int ypos2 = ypos+thisSpriteTile->offsY;
			switch(thisSpriteTile->tile&0xC000) {
				case 0x0000: {
					int tileSize = (thisSpriteTile->props&1)?16:8;
					RECT tileRect = {xpos2,ypos2,xpos2+tileSize,ypos2+tileSize};
					if(PtInRect(&tileRect,{x,y})) {
						if(!thisSprite->selected) {
							clearSpriteSelection();
							thisSprite->selected = true;
						}
						return;
					}
					break;
				}
				case 0x4000: {
					RECT tileRect = {xpos2,ypos2,xpos2+16,ypos2+16};
					if(PtInRect(&tileRect,{x,y})) {
						if(!thisSprite->selected) {
							clearSpriteSelection();
							thisSprite->selected = true;
						}
						return;
					}
					break;
				}
				case 0x8000: {
					RECT tileRect = {xpos2,ypos2,xpos2+256,ypos2+1};
					if(PtInRect(&tileRect,{x,y})) {
						if(!thisSprite->selected) {
							clearSpriteSelection();
							thisSprite->selected = true;
						}
						return;
					}
					break;
				}
				case 0xC000: {
					RECT tileRect = {xpos2,ypos2,xpos2+8,ypos2+8};
					if(PtInRect(&tileRect,{x,y})) {
						if(!thisSprite->selected) {
							clearSpriteSelection();
							thisSprite->selected = true;
						}
						return;
					}
					break;
				}
			}
		}
	}
}
void moveSprites(int dx,int dy) {
	dx >>= 4;
	dy >>= 4;
	int numSelectedSprites = 0;
	int minX = 0x8000,minY = 0x8000;
	int maxX = 0,maxY = 0;
	//Check if any sprites are to be moved
	for(int n=0; n<spriteContexts[0].sprites.size(); n++) {
		sprite_t * thisSprite = &spriteContexts[0].sprites[n];
		if(thisSprite->selected) {
			int xpos = thisSprite->data[2];
			int ypos = thisSprite->data[1]>>1;
			minX = std::min(xpos,minX);
			minY = std::min(ypos,minY);
			maxX = std::max(xpos,maxX);
			maxY = std::max(ypos,maxY);
			numSelectedSprites++;
		}
	}
	if(numSelectedSprites) {
		//Determine if any sprites will be out of bounds after this operation,
		//and if so, terminate
		if((minX+dx)<0 || (minY+dy)<0 || (maxX+dx)>=0x100 || (maxY+dy)>=0x80) return;
		//Move selected sprites
		for(int n=0; n<spriteContexts[0].sprites.size(); n++) {
			sprite_t * thisSprite = &spriteContexts[0].sprites[n];
			if(thisSprite->selected) {
				thisSprite->data[2] += dx;
				thisSprite->data[1] += (dy<<1);
			}
		}
	}
}

///////////////////
//WINDOW FUNCTION//
///////////////////
HDC				hdcSp;
HBITMAP			hbmpSp;
DWORD *			bmpDataSp;
HWND			hwndCbSprite,hwndLbSprite;
RECT invRect_sprite = {0,0,0x100,0x100};

BYTE spriteDlgData_t0[] = {
	//000
	0x00,0x0E,0x07,0xFF,0xFF,0xFF,
	0x01,0x0E,0x07,0xFF,0xFF,0xFF,
	0x03,0x0E,0x07,0xFF,0xFF,0xFF,
	0x05,0x0E,0x07,0xFF,0xFF,0xFF,
	0x07,0x0E,0x07,0xFF,0xFF,0xFF,
	0x09,0x0E,0x07,0xFF,0xFF,0xFF,
	0x0A,0x0E,0x07,0xFF,0xFF,0xFF,
	0x0C,0x0E,0x07,0xFF,0xFF,0xFF,
	0x0D,0x16,0x06,0xFF,0xFF,0xFF,
	//010
	0x10,0x16,0x08,0xFF,0xFF,0xFF,
	0x12,0x0E,0x07,0xFF,0xFF,0xFF,
	0x16,0x10,0x08,0xFF,0xFF,0xFF,
	0x17,0x0E,0x07,0xFF,0xFF,0xFF,
	0x1A,0x0E,0x07,0xFF,0xFF,0xFF,
	0x1B,0x0E,0x07,0xFF,0xFF,0xFF,
	0x1C,0x0E,0x07,0xFF,0xFF,0xFF,
	0x1D,0x0E,0x07,0xFF,0xFF,0xFF,
	0x1E,0x0E,0x07,0xFF,0xFF,0xFF,
	0x1F,0x0E,0x07,0xFF,0xFF,0xFF,
	//020
	0x20,0x0E,0x07,0xFF,0xFF,0xFF,
	0x21,0x0E,0x07,0xFF,0xFF,0xFF,
	0x22,0x0E,0x07,0xFF,0xFF,0xFF,
	0x23,0x0E,0x07,0xFF,0xFF,0xFF,
	0x24,0x0E,0x07,0xFF,0xFF,0xFF,
	0x25,0x0E,0x07,0xFF,0xFF,0xFF,
	0x27,0x0E,0x07,0xFF,0xFF,0xFF,
	0x28,0x0E,0x07,0xFF,0xFF,0xFF,
	0x2C,0x0E,0x07,0xFF,0xFF,0xFF,
	0x2D,0x16,0x08,0xFF,0xFF,0xFF,
	0x2F,0x0E,0x07,0xFF,0xFF,0xFF,
	//030
	0x30,0x0E,0x07,0xFF,0xFF,0xFF,
	0x31,0x0E,0x07,0xFF,0xFF,0xFF,
	0x32,0x0E,0x07,0xFF,0xFF,0xFF,
	0x33,0x0E,0x07,0xFF,0xFF,0xFF,
	0x34,0x18,0x04,0xFF,0xFF,0xFF,
	0x36,0x14,0x05,0xFF,0xFF,0xFF,
	0x37,0x0E,0x07,0xFF,0xFF,0xFF,
	0x39,0x0C,0x07,0xFF,0xFF,0xFF,
	0x3A,0x0E,0x07,0xFF,0xFF,0xFF,
	0x3B,0x0E,0x07,0xFF,0xFF,0xFF,
	0x3C,0x0E,0x07,0xFF,0xFF,0xFF,
	0x3D,0x0E,0x00,0xFF,0xFF,0xFF,
	0x3E,0x0E,0x07,0xFF,0xFF,0xFF,
	0x3F,0x14,0x08,0xFF,0xFF,0xFF,
	//040
	0x42,0x0E,0x05,0xFF,0xFF,0xFF,
	0x45,0x0E,0x08,0xFF,0xFF,0xFF,
	0x46,0x16,0x08,0xFF,0xFF,0xFF,
	0x48,0x0E,0x07,0xFF,0xFF,0xFF,
	0x4C,0x0E,0x07,0xFF,0xFF,0xFF,
	0x4E,0x0E,0x07,0xFF,0xFF,0xFF,
	0x4F,0x0E,0x07,0xFF,0xFF,0xFF,
	//050
	0x50,0x0E,0x05,0xFF,0xFF,0xFF,
	0x51,0x0E,0x05,0xFF,0xFF,0xFF,
	0x52,0x0E,0x07,0xFF,0xFF,0xFF,
	0x54,0x0E,0x07,0xFF,0xFF,0xFF,
	0x55,0x0E,0x07,0xFF,0xFF,0xFF,
	0x56,0x0E,0x07,0xFF,0xFF,0xFF,
	0x57,0x1E,0x05,0xFF,0xFF,0xFF,
	0x58,0x0E,0x07,0xFF,0xFF,0xFF,
	0x59,0x0E,0x07,0xFF,0xFF,0xFF,
	0x5B,0x0E,0x07,0xFF,0xFF,0xFF,
	0x5C,0x0E,0x07,0xFF,0xFF,0xFF,
	0x5E,0x0E,0x07,0xFF,0xFF,0xFF,
	0x5F,0x0E,0x07,0xFF,0xFF,0xFF,
	//060
	0x62,0x0E,0x07,0xFF,0xFF,0xFF,
	0x63,0x0E,0x07,0xFF,0xFF,0xFF,
	0x64,0x0E,0x07,0xFF,0xFF,0xFF,
	0x65,0x0E,0x07,0xFF,0xFF,0xFF,
	0x66,0x0E,0x07,0xFF,0xFF,0xFF,
	0x67,0x0E,0x07,0xFF,0xFF,0xFF,
	0x68,0x0E,0x07,0xFF,0xFF,0xFF,
	0x69,0x0E,0x07,0xFF,0xFF,0xFF,
	0x6A,0x0E,0x07,0xFF,0xFF,0xFF,
	0x6C,0x0E,0x07,0xFF,0xFF,0xFF,
	0x6D,0x0E,0x07,0xFF,0xFF,0xFF,
	0x6E,0x0E,0x07,0xFF,0xFF,0xFF,
	0x6F,0x0E,0x07,0xFF,0xFF,0xFF,
	//070
	0x70,0x0E,0x07,0xFF,0xFF,0xFF,
	0x71,0x0E,0x07,0xFF,0xFF,0xFF,
	0x72,0x0E,0x07,0xFF,0xFF,0xFF,
	0x73,0x0E,0x03,0xFF,0xFF,0xFF,
	0x74,0x0E,0x07,0xFF,0xFF,0xFF,
	0x76,0x0E,0x07,0xFF,0xFF,0xFF,
	0x77,0x0E,0x07,0xFF,0xFF,0xFF,
	0x78,0x0E,0x07,0xFF,0xFF,0xFF,
	0x79,0x0E,0x07,0xFF,0xFF,0xFF,
	0x7A,0x0E,0x07,0xFF,0xFF,0xFF,
	0x7F,0x0E,0x07,0xFF,0xFF,0xFF,
	//080
	0x80,0x0E,0x07,0xFF,0xFF,0xFF,
	0x81,0x0E,0x07,0xFF,0xFF,0xFF,
	0x82,0x0E,0x07,0xFF,0xFF,0xFF,
	0x84,0x0E,0x06,0xFF,0xFF,0xFF,
	0x85,0x0E,0x07,0xFF,0xFF,0xFF,
	0x87,0x0E,0x07,0xFF,0xFF,0xFF,
	0x88,0x0E,0x07,0xFF,0xFF,0xFF,
	0x89,0x0E,0x07,0xFF,0xFF,0xFF,
	0x8A,0x0E,0x07,0xFF,0xFF,0xFF,
	0x8B,0x0E,0x07,0xFF,0xFF,0xFF,
	0x8D,0x0E,0x07,0xFF,0xFF,0xFF,
	0x8E,0x0E,0x07,0xFF,0xFF,0xFF,
	0x8F,0x0E,0x07,0xFF,0xFF,0xFF,
	//090
	0x90,0x0A,0x08,0xFF,0xFF,0xFF,
	0x92,0x0E,0x07,0xFF,0xFF,0xFF,
	0x93,0x0E,0x07,0xFF,0xFF,0xFF,
	0x94,0x0E,0x07,0xFF,0xFF,0xFF,
	0x95,0x0E,0x07,0xFF,0xFF,0xFF,
	0x96,0x0E,0x07,0xFF,0xFF,0xFF,
	0x97,0x0E,0x07,0xFF,0xFF,0xFF,
	0x98,0x0E,0x07,0xFF,0xFF,0xFF,
	0x9A,0x0E,0x07,0xFF,0xFF,0xFF,
	0x9B,0x0E,0x07,0xFF,0xFF,0xFF,
	0x9D,0x0E,0x07,0xFF,0xFF,0xFF,
	0x9E,0x0E,0x07,0xFF,0xFF,0xFF,
	0x9F,0x0E,0x07,0xFF,0xFF,0xFF,
	//0A0
	0xA0,0x0E,0x07,0xFF,0xFF,0xFF,
	0xA3,0x0E,0x07,0xFF,0xFF,0xFF,
	0xA4,0x0E,0x07,0xFF,0xFF,0xFF,
	0xA5,0x1A,0x07,0xFF,0xFF,0xFF,
	0xA6,0x0E,0x07,0xFF,0xFF,0xFF,
	0xA7,0x0E,0x07,0xFF,0xFF,0xFF,
	0xAA,0x0E,0x07,0xFF,0xFF,0xFF,
	0xAD,0x0E,0x07,0xFF,0xFF,0xFF,
	0xAE,0x0E,0x07,0xFF,0xFF,0xFF,
	0xAF,0x0E,0x07,0xFF,0xFF,0xFF,
	//0B0
	0xB0,0x0E,0x07,0xFF,0xFF,0xFF,
	0xB1,0x0E,0x07,0xFF,0xFF,0xFF,
	0xB2,0x0E,0x07,0xFF,0xFF,0xFF,
	0xB4,0x0E,0x07,0xFF,0xFF,0xFF,
	0xB5,0x0E,0x07,0xFF,0xFF,0xFF,
	0xB6,0x0E,0x07,0xFF,0xFF,0xFF,
	0xB7,0x0E,0x07,0xFF,0xFF,0xFF,
	0xB8,0x0E,0x07,0xFF,0xFF,0xFF,
	0xB9,0x0E,0x07,0xFF,0xFF,0xFF,
	0xBA,0x0E,0x07,0xFF,0xFF,0xFF,
	0xBB,0x0E,0x07,0xFF,0xFF,0xFF,
	0xBC,0x0E,0x07,0xFF,0xFF,0xFF,
	0xBD,0x0E,0x07,0xFF,0xFF,0xFF,
	0xBE,0x0E,0x07,0xFF,0xFF,0xFF,
	0xBF,0x0E,0x07,0xFF,0xFF,0xFF,
	//0C0
	0xC0,0x0E,0x07,0xFF,0xFF,0xFF,
	0xC1,0x0E,0x07,0xFF,0xFF,0xFF,
	0xC2,0x0E,0x07,0xFF,0xFF,0xFF,
	0xC3,0x0E,0x07,0xFF,0xFF,0xFF,
	0xC4,0x0E,0x07,0xFF,0xFF,0xFF,
	0xC5,0x0E,0x07,0xFF,0xFF,0xFF,
	0xC6,0x0E,0x07,0xFF,0xFF,0xFF,
	0xC7,0x0E,0x07,0xFF,0xFF,0xFF,
	0xC8,0x0E,0x07,0xFF,0xFF,0xFF,
	0xCA,0x0E,0x07,0xFF,0xFF,0xFF,
	0xCB,0x0E,0x07,0xFF,0xFF,0xFF,
	0xCC,0x0E,0x07,0xFF,0xFF,0xFF,
	//0D0
	0xD0,0x0E,0x04,0xFF,0xFF,0xFF,
	0xD1,0x0E,0x03,0xFF,0xFF,0xFF,
	0xD2,0x16,0x08,0xFF,0xFF,0xFF,
	0xD3,0x0E,0x07,0xFF,0xFF,0xFF,
	0xD4,0x0E,0x07,0xFF,0xFF,0xFF,
	0xD6,0x18,0x07,0xFF,0xFF,0xFF,
	0xD7,0x08,0x07,0xFF,0xFF,0xFF,
	0xD8,0x0E,0x07,0xFF,0xFF,0xFF,
	0xD9,0x0E,0x07,0xFF,0xFF,0xFF,
	0xDA,0x0E,0x07,0xFF,0xFF,0xFF,
	0xDB,0x0E,0x08,0xFF,0xFF,0xFF,
	0xDC,0x0E,0x07,0xFF,0xFF,0xFF,
	0xDF,0x0E,0x07,0xFF,0xFF,0xFF,
	//0E0
	0xE0,0x0E,0x07,0xFF,0xFF,0xFF,
	0xE1,0x0E,0x07,0xFF,0xFF,0xFF,
	0xE2,0x0E,0x07,0xFF,0xFF,0xFF,
	0xE3,0x0E,0x07,0xFF,0xFF,0xFF,
	0xE4,0x0E,0x07,0xFF,0xFF,0xFF,
	0xE6,0x0E,0x07,0xFF,0xFF,0xFF,
	0xE7,0x0E,0x07,0xFF,0xFF,0xFF,
	0xE8,0x0E,0x07,0xFF,0xFF,0xFF,
	0xE9,0x0E,0x07,0xFF,0xFF,0xFF,
	0xEA,0x0E,0x07,0xFF,0xFF,0xFF,
	0xEB,0x0E,0x07,0xFF,0xFF,0xFF,
	0xEC,0x0E,0x07,0xFF,0xFF,0xFF,
	0xED,0x0E,0x07,0xFF,0xFF,0xFF,
	0xEE,0x0E,0x07,0xFF,0xFF,0xFF,
	//0F0
	0xF1,0x0E,0x07,0xFF,0xFF,0xFF,
	0xF2,0x0E,0x07,0xFF,0xFF,0xFF,
	0xF3,0x0E,0x07,0xFF,0xFF,0xFF,
	0xF4,0x0E,0x07,0xFF,0xFF,0xFF,
	0xF5,0x0E,0x07,0xFF,0xFF,0xFF,
	0xF6,0x0E,0x07,0xFF,0xFF,0xFF,
	0xF7,0x0E,0x07,0xFF,0xFF,0xFF,
	0xF8,0x0E,0x07,0xFF,0xFF,0xFF,
	0xFA,0x0E,0x07,0xFF,0xFF,0xFF,
	0xFB,0x0E,0x07,0xFF,0xFF,0xFF,
	0xFC,0x0E,0x07,0xFF,0xFF,0xFF,
	0xFD,0x0E,0x07,0xFF,0xFF,0xFF,
	0xFF,0x0E,0x07,0xFF,0xFF,0xFF,
	//100
	0x01,0x0F,0x07,0xFF,0xFF,0xFF,
	0x02,0x0F,0x07,0xFF,0xFF,0xFF,
	0x03,0x0F,0x07,0xFF,0xFF,0xFF,
	0x04,0x0F,0x07,0xFF,0xFF,0xFF,
	0x05,0x0F,0x07,0xFF,0xFF,0xFF,
	0x06,0x0F,0x07,0xFF,0xFF,0xFF,
	0x08,0x0F,0x07,0xFF,0xFF,0xFF,
	0x09,0x0F,0x07,0xFF,0xFF,0xFF,
	0x0A,0x0F,0x07,0xFF,0xFF,0xFF,
	0x0B,0x0F,0x07,0xFF,0xFF,0xFF,
	0x0C,0x0F,0x07,0xFF,0xFF,0xFF,
	0x0D,0x0F,0x07,0xFF,0xFF,0xFF,
	0x0E,0x0F,0x07,0xFF,0xFF,0xFF,
	0x0F,0x0F,0x07,0xFF,0xFF,0xFF,
	//110
	0x10,0x0F,0x07,0xFF,0xFF,0xFF,
	0x11,0x0F,0x07,0xFF,0xFF,0xFF,
	0x13,0x0F,0x07,0xFF,0xFF,0xFF,
	0x16,0x0F,0x07,0xFF,0xFF,0xFF,
	0x19,0x0F,0x07,0xFF,0xFF,0xFF,
	0x1A,0x0F,0x07,0xFF,0xFF,0xFF,
	0x1B,0x0F,0x07,0xFF,0xFF,0xFF,
	0x1E,0x0F,0x07,0xFF,0xFF,0xFF,
	0x1F,0x0F,0x07,0xFF,0xFF,0xFF,
	//120
	0x20,0x0F,0x07,0xFF,0xFF,0xFF,
	0x22,0x0F,0x07,0xFF,0xFF,0xFF,
	0x23,0x0F,0x07,0xFF,0xFF,0xFF,
	0x24,0x0F,0x07,0xFF,0xFF,0xFF,
	0x25,0x0F,0x07,0xFF,0xFF,0xFF,
	0x26,0x0F,0x07,0xFF,0xFF,0xFF,
	0x2A,0x0F,0x07,0xFF,0xFF,0xFF,
	0x2B,0x0F,0x07,0xFF,0xFF,0xFF,
	0x2C,0x0F,0x07,0xFF,0xFF,0xFF,
	0x2F,0x0F,0x07,0xFF,0xFF,0xFF,
	//130
	0x30,0x0F,0x07,0xFF,0xFF,0xFF,
	0x31,0x0F,0x07,0xFF,0xFF,0xFF,
	0x32,0x0F,0x07,0xFF,0xFF,0xFF,
	0x33,0x0F,0x07,0xFF,0xFF,0xFF,
	0x34,0x0F,0x07,0xFF,0xFF,0xFF,
	0x35,0x0F,0x07,0xFF,0xFF,0xFF,
	0x36,0x0F,0x07,0xFF,0xFF,0xFF,
	0x37,0x0F,0x07,0xFF,0xFF,0xFF,
	0x38,0x0F,0x07,0xFF,0xFF,0xFF,
	0x39,0x0F,0x07,0xFF,0xFF,0xFF,
	0x3A,0x0F,0x07,0xFF,0xFF,0xFF,
	0x3C,0x0F,0x07,0xFF,0xFF,0xFF,
	0x3D,0x0F,0x07,0xFF,0xFF,0xFF,
	0x3E,0x0F,0x07,0xFF,0xFF,0xFF,
	0x3F,0x0F,0x07,0xFF,0xFF,0xFF,
	//140
	0x40,0x0F,0x07,0xFF,0xFF,0xFF,
	0x41,0x0F,0x07,0xFF,0xFF,0xFF,
	0x42,0x0F,0x07,0xFF,0xFF,0xFF,
	0x43,0x0F,0x07,0xFF,0xFF,0xFF,
	0x44,0x0F,0x07,0xFF,0xFF,0xFF,
	0x45,0x0F,0x07,0xFF,0xFF,0xFF,
	0x46,0x0F,0x07,0xFF,0xFF,0xFF,
	0x47,0x0F,0x07,0xFF,0xFF,0xFF,
	0x48,0x0F,0x07,0xFF,0xFF,0xFF,
	0x49,0x0F,0x07,0xFF,0xFF,0xFF,
	0x4A,0x0F,0x07,0xFF,0xFF,0xFF,
	0x4B,0x0F,0x07,0xFF,0xFF,0xFF,
	0x4C,0x0F,0x07,0xFF,0xFF,0xFF,
	0x4D,0x0F,0x07,0xFF,0xFF,0xFF,
	0x4E,0x0F,0x07,0xFF,0xFF,0xFF,
	0x4F,0x0F,0x07,0xFF,0xFF,0xFF,
	//150
	0x50,0x0F,0x07,0xFF,0xFF,0xFF,
	0x51,0x0F,0x07,0xFF,0xFF,0xFF,
	0x52,0x0F,0x07,0xFF,0xFF,0xFF,
	0x53,0x0F,0x07,0xFF,0xFF,0xFF,
	0x54,0x0F,0x07,0xFF,0xFF,0xFF,
	0x55,0x0F,0x07,0xFF,0xFF,0xFF,
	0x56,0x0F,0x07,0xFF,0xFF,0xFF,
	0x57,0x0F,0x07,0xFF,0xFF,0xFF,
	0x58,0x0F,0x07,0xFF,0xFF,0xFF,
	0x59,0x0F,0x07,0xFF,0xFF,0xFF,
	0x5A,0x0F,0x07,0xFF,0xFF,0xFF,
	0x5B,0x0F,0x07,0xFF,0xFF,0xFF,
	0x5C,0x0F,0x07,0xFF,0xFF,0xFF,
	0x5D,0x0F,0x07,0xFF,0xFF,0xFF,
	0x5E,0x0F,0x07,0xFF,0xFF,0xFF,
	0x5F,0x0F,0x07,0xFF,0xFF,0xFF,
	//160
	0x60,0x0F,0x07,0xFF,0xFF,0xFF,
	0x61,0x0F,0x07,0xFF,0xFF,0xFF,
	0x62,0x0F,0x07,0xFF,0xFF,0xFF,
	0x64,0x0F,0x07,0xFF,0xFF,0xFF,
	0x65,0x0F,0x07,0xFF,0xFF,0xFF,
	0x66,0x0F,0x07,0xFF,0xFF,0xFF,
	0x67,0x0F,0x07,0xFF,0xFF,0xFF,
	0x68,0x0F,0x07,0xFF,0xFF,0xFF,
	0x69,0x0F,0x07,0xFF,0xFF,0xFF,
	0x6A,0x0F,0x07,0xFF,0xFF,0xFF,
	0x6B,0x0F,0x07,0xFF,0xFF,0xFF,
	0x6C,0x0F,0x07,0xFF,0xFF,0xFF,
	0x6D,0x0F,0x07,0xFF,0xFF,0xFF,
	0x6E,0x0F,0x07,0xFF,0xFF,0xFF,
	0x6F,0x0F,0x07,0xFF,0xFF,0xFF,
	//170
	0x70,0x0F,0x07,0xFF,0xFF,0xFF,
	0x71,0x0F,0x07,0xFF,0xFF,0xFF,
	0x73,0x0F,0x07,0xFF,0xFF,0xFF,
	0x74,0x0F,0x07,0xFF,0xFF,0xFF,
	0x75,0x0F,0x07,0xFF,0xFF,0xFF,
	0x76,0x0F,0x07,0xFF,0xFF,0xFF,
	0x77,0x0F,0x07,0xFF,0xFF,0xFF,
	0x78,0x0F,0x07,0xFF,0xFF,0xFF,
	0x79,0x0F,0x07,0xFF,0xFF,0xFF,
	0x7A,0x0F,0x07,0xFF,0xFF,0xFF,
	0x7B,0x0F,0x07,0xFF,0xFF,0xFF,
	0x7C,0x0F,0x07,0xFF,0xFF,0xFF,
	0x7D,0x0F,0x07,0xFF,0xFF,0xFF,
	0x7E,0x0F,0x07,0xFF,0xFF,0xFF,
	//180
	0x80,0x0F,0x07,0xFF,0xFF,0xFF,
	0x81,0x0F,0x07,0xFF,0xFF,0xFF,
	0x82,0x0F,0x07,0xFF,0xFF,0xFF,
	0x83,0x0F,0x07,0xFF,0xFF,0xFF,
	0x84,0x0F,0x07,0xFF,0xFF,0xFF,
	0x85,0x0F,0x07,0xFF,0xFF,0xFF,
	0x86,0x0F,0x07,0xFF,0xFF,0xFF,
	0x87,0x0F,0x07,0xFF,0xFF,0xFF,
	0x88,0x0F,0x07,0xFF,0xFF,0xFF,
	0x89,0x0F,0x07,0xFF,0xFF,0xFF,
	0x8A,0x0F,0x07,0xFF,0xFF,0xFF,
	0x8B,0x0F,0x07,0xFF,0xFF,0xFF,
	0x8C,0x0F,0x07,0xFF,0xFF,0xFF,
	0x8D,0x0F,0x07,0xFF,0xFF,0xFF,
	0x8E,0x0F,0x07,0xFF,0xFF,0xFF,
	0x8F,0x0F,0x07,0xFF,0xFF,0xFF,
	//190
	0x90,0x0F,0x07,0xFF,0xFF,0xFF,
	0x91,0x0F,0x07,0xFF,0xFF,0xFF,
	0x92,0x0F,0x07,0xFF,0xFF,0xFF,
	0x93,0x0F,0x00,0xFF,0xFF,0xFF,
	0x94,0x0F,0x07,0xFF,0xFF,0xFF,
	0x97,0x0F,0x07,0xFF,0xFF,0xFF,
	0x98,0x0F,0x07,0xFF,0xFF,0xFF,
	0x99,0x0F,0x07,0xFF,0xFF,0xFF,
	0x9A,0x0F,0x07,0xFF,0xFF,0xFF,
	0x9B,0x0F,0x07,0xFF,0xFF,0xFF,
	0x9C,0x0F,0x07,0xFF,0xFF,0xFF,
	0x9D,0x0F,0x07,0xFF,0xFF,0xFF,
	0x9E,0x0F,0x07,0xFF,0xFF,0xFF,
	0x9F,0x0F,0x07,0xFF,0xFF,0xFF,
	//1A0
	0xA0,0x0F,0x07,0xFF,0xFF,0xFF,
	0xA1,0x0F,0x07,0xFF,0xFF,0xFF,
	0xA3,0x0F,0x07,0xFF,0xFF,0xFF,
	0xA4,0x0F,0x07,0xFF,0xFF,0xFF,
	0xA5,0x0F,0x07,0xFF,0xFF,0xFF,
	0xA6,0x0F,0x07,0xFF,0xFF,0xFF,
	0xA7,0x0F,0x07,0xFF,0xFF,0xFF,
	0xA8,0x0F,0x07,0xFF,0xFF,0xFF,
	0xA9,0x0F,0x07,0xFF,0xFF,0xFF,
	0xAA,0x0F,0x07,0xFF,0xFF,0xFF,
	0xAB,0x0F,0x07,0xFF,0xFF,0xFF,
	0xAC,0x0F,0x07,0xFF,0xFF,0xFF,
	0xAD,0x0F,0x07,0xFF,0xFF,0xFF,
	0xAF,0x0F,0x07,0xFF,0xFF,0xFF};
LPCTSTR spriteDlgNames_t0[] = {
	//000
	"000\tLog, lava/water (X)",
	"001\tClosed door",
	"003\tCrate (Key)",
	"005\tIcy watermelon",
	"007\tWatermelon",
	"009\tFire watermelon",
	"00A\tKaboomba",
	"00C\tRaphael the Raven",
	"00D\tGoal Ring",
	//010
	"010\tCaged Ghost, round mound",
	"012\tBoss door",
	"016\tBigger Boo",
	"017\tFrog Pirate",
	"01A\tSki Lift",
	"01B\tVertical log, lava/water (X)",
	"01C\tDr. Freezegood, nothing/6 stars/1-UP/Bumpty (YX)",
	"01D\tDr. Freezegood on Ski Lift",
	"01E\tShy Guy, green/red/yellow/pink (YX)",
	"01F\tRotating doors",
	//020
	"020\tBandit",
	"021\t? Bucket, lava/water (X)",
	"022\tFlashing Egg",
	"023\tRed Egg",
	"024\tYellow Egg",
	"025\tGreen Egg",
	"027\tKey",
	"028\tHuffin' Puffin",
	"02C\tLunge Fish",
	"02D\tSalvo the Slime",
	"02F\tLittle Mouser nest",
	//030
	"030\tLittle Mouser",
	"031\tPotted Spiked Fun Guy",
	"032\tLittle Mouser, in nest/behind stuff (X)",
	"033\tLittle Mouser, from nest",
	"034\tRoger the Potted Ghost",
	"036\tFake falling wall",
	"037\tGrim Leecher",
	"039\tSpinning platform",
	"03A\t3 Mini Ravens",
	"03B\tMini Raven",
	"03C\tTap-Tap the Red Nose",
	"03D\tSeesaw",
	"03E\tSkinny platform",
	"03F\tSlime",
	//040
	"042\tVertical Pipe Exit Down",
	"045\tPrince Froggy, throat/before/throat & uvula/after (YX)",
	"046\tBurt the Bashful",
	"048\tKamek, boss cutscene",
	"04C\tUpside-down Blow Hard",
	"04E\tLocked door",
	"04F\tMiddle Ring",
	//050
	"050\tWide plank",
	"051\tOctagonal log",
	"052\tBalloon platform, randomly colored",
	"054\tUpside-down Wild Piranha",
	"055\tFlatbed Ferry pinwheel, green large",
	"056\tFlatbed Ferry pinwheel, pink small",
	"057\tRoger Lift",
	"058\tSolo Toady, green",
	"059\tSuper Star, continuation",
	"05B\tCoin Bandit",
	"05C\tSolo Toady, pink",
	"05E\tShort plank",
	"05F\tShort plank, always spinning",
	//060
	"062\tGoomba",
	"063\tMuddy Buddy",
	"064\tPink Flatbed Ferry pinwheel, direction/size (YX)",
	"065\tRed Coin",
	"066\tWild Piranha",
	"067\tHidden Winged Cloud, 5 stars/seed/flower/1-UP (YX)",
	"068\tFlashing Egg Block",
	"069\tRed Egg Block",
	"06A\tYellow Egg Block",
	"06C\tLarge spring ball",
	"06D\tHootie the Blue Fish, clockwise",
	"06E\tHootie the Blue Fish, counterclockwise",
	"06F\tSpring ball",
	//070
	"070\tClawdaddy",
	"071\tBig Boo & 3 Boos/Big Boo/Big Boo & 3 Boos/Boo (YX)",
	"072\tTrain Bandit",
	"073\tLarge balloon platform with pump",
	"074\tSpike",
	"076\tPiro Dangle, clockwise",
	"077\tPiro Dangle, counterclockwise",
	"078\tBullet Bill launcher, red",
	"079\tBullet Bill launcher, yellow",
	"07A\tBullet Bill launcher, green",
	"07F\tLog seesaw",
	//080
	"080\tLava bubble, up and down",
	"081\tLava bubble, jumps across",
	"082\tChain Chomp",
	"084\tTeleport Exit",
	"085\tHarry Hedgehog",
	"087\tRed Egg, 1-UP",
	"088\tSuper Star",
	"089\tFlatbed Ferry, red horizontal",
	"08A\tFlatbed Ferry, pink vertical",
	"08B\tMock Up, green/red (X)",
	"08D\tFly Guy, 5 stars/red coin/1-UP/1-UP (YX)",
	"08E\tKamek, in Bowser's room",
	"08F\tGrinder Swing",
	//090
	"090\tDangling Ghost",
	"092\tMelon Bug",
	"093\tDoor",
	"094\tExpansion Block",
	"095\tBlue checker platform",
	"096\tRed checker platform",
	"097\tPOW Block",
	"098\tYoshi Block",
	"09A\tFlatbed Ferry, green chained",
	"09B\tMace Guy",
	"09D\t! Switch",
	"09E\tChomp Rock",
	"09F\tWild Ptooie Piranha",
	//0A0
	"0A0\tTulip",
	"0A3\tBandit, hiding left",
	"0A4\tBandit, hiding right",
	"0A5\tNep-Enut/Gargantua Blargg (X)",
	"0A6\tIncoming Chomp in background",
	"0A7\tIncoming Chomp in background, flock",
	"0AA\tShy Guys in background",
	"0AD\tMessage Block",
	"0AE\tHookbill the Koopa",
	"0AF\tMorph Bubble, Car",
	//0B0
	"0B0\tMorph Bubble, Mole",
	"0B1\tMorph Bubble, Helicopter",
	"0B2\tMorph Bubble, Train",
	"0B4\tMorph Bubble, Submarine",
	"0B5\tHidden Winged Cloud, 1-UP/5 stars/! Switch/5 stars (YX)",
	"0B6\tWinged Cloud, 8 coins",
	"0B7\tWinged Cloud, 1-UP in bubble",
	"0B8\tWinged Cloud, flower",
	"0B9\tWinged Cloud, POW",
	"0BA\tWinged Cloud, stairs right/left (X)",
	"0BB\tWinged Cloud, platform right/left (X)",
	"0BC\tWinged Cloud, Bandit",
	"0BD\tWinged Cloud, coin",
	"0BE\tWinged Cloud, 1-UP",
	"0BF\tWinged Cloud, key",
	//0C0
	"0C0\tWinged Cloud, 3 stars",
	"0C1\tWinged Cloud, 5 stars",
	"0C2\tWinged Cloud, door",
	"0C3\tWinged Cloud, ground eater",
	"0C4\tWinged Cloud, watermelon",
	"0C5\tWinged Cloud, fire watermelon",
	"0C6\tWinged Cloud, icy watermelon",
	"0C7\tWinged Cloud, seed for 3 leaf sunflower",
	"0C8\tWinged Cloud, seed for 6 leaf sunflower",
	"0CA\tFinal boss door",
	"0CB\tWinged Cloud, random item",
	"0CC\tWinged Cloud, ! Switch",
	//0D0
	"0D0\tHorizontal Pipe Exit Right",
	"0D1\tVertical Pipe Exit Down (Hidden)",
	"0D2\tMarching Milde",
	"0D3\tMarching Milde 1/2 segment",
	"0D4\tMarching Milde 1/4 segment",
	"0D6\tBlarggwich",
	"0D7\tSluggy the Unshaven",
	"0D8\tChomp signboard",
	"0D9\tFishin' Lakitu",
	"0DA\tFlower pot, key/6 stars/6 coins/nothing (YX)",
	"0DB\tSquishy block platform",
	"0DC\tSnowball",
	"0DF\tPiscatory Pete, blue/gold (X)",
	//0E0
	"0E0\tPreying Mantas",
	"0E1\tLoch Nestor",
	"0E2\tBoo Blah, normal/upside-down (X)",
	"0E3\tBoo Blah & Piro Dangle, normal/upside-down (X)",
	"0E4\tPokey",
	"0E6\tGusty, left/right/infinite right/infinite left (YX)",
	"0E7\tBurt, 2/1 (X)",
	"0E8\tGoonie, right/left/generator right/generator left (YX)",
	"0E9\t3 Flightless Goonies",
	"0EA\tCloud Drop, vertical",
	"0EB\tCloud Drop, horizontal",
	"0EC\tFlame Guy, jumping",
	"0ED\tFlame Guy, walking",
	"0EE\tEggo-Dill",
	//0F0
	"0F1\tBubble-Plant",
	"0F2\tStilt Guy, green/red/yellow/pink (YX)",
	"0F3\tWoozy Guy, green/red/yellow/pink (YX)",
	"0F4\tEgg-Plant/Needlenose-Plant (X)",
	"0F5\tSlugger",
	"0F6\tHuffin' Puffin' with 3 children",
	"0F7\tBarney Bubble",
	"0F8\tBlow Hard",
	"0FA\tFlower (SuperFX texture)",
	"0FB\tSpear Guy, long spear",
	"0FC\tSpear Guy, short spear",
	"0FD\tZeus Guy",
	"0FF\tPoochy",
	//100
	"101\tSpike mace, 1 end",
	"102\tSpike mace, 2 ends",
	"103\tBoo Guys controlling spike mace",
	"104\tJean de Fillet, right/left (X)",
	"105\tBoo Guy bomb brigade left",
	"106\tBoo Guy bomb brigade right",
	"108\tMilde",
	"109\tTap-Tap",
	"10A\tTap-Tap, stays on ledge",
	"10B\tTap-Tap, hopping",
	"10C\tSpike ball on chain",
	"10D\tBoo Guys controlling spike ball on chain, right/left (X)",
	"10E\tCrate, 6 stars",
	"10F\tBoo Man Bluff",
	//110
	"110\tFlower, character graphics",
	"111\tGeorgette Jelly",
	"113\tSnifit",
	"116\tFloating round platform",
	"119\tSpooky",
	"11A\tGreen Glove",
	"11B\tLakitu, 1/2 (X)",
	"11E\tArrow Wheel, brown",
	"11F\tArrow Wheel, blue",
	//120
	"120\tArrow lift, double ended",
	"122\t? bucket, Bandit",
	"123\t? bucket, 5 coins",
	"124\tStretch, green/red/yellow/pink (YX)",
	"125\tKamek, ending cutscene/flying & chasing (X)",
	"126\tSpike log on chain",
	"12A\tShy Guy hiding Bandit",
	"12B\tFat Guy, red/green (X)",
	"12C\tFly Guy, Red Coin/Whirly Guy (X)",
	"12F\tLava Drop, horizontal",
	//130
	"130\tLava Drop, vertical",
	"131\tLocked door",
	"132\tLemon Drop",
	"133\tLantern Ghost",
	"134\tBaby Bowser",
	"135\tRaven circling forever, counterclockwise/clockwise (X)",
	"136\tRaven circling back & forth, counterclockwise/clockwise (X)",
	"137\tFalling rock platform, 3x6",
	"138\tFalling rock platform, 3x3",
	"139\tFalling rock platform, 3x9",
	"13A\tFalling rock platform, 6x3",
	"13C\tVertical flipper, up/down (X)",
	"13D\tFang, hanging from ceiling",
	"13E\tFang, flying",
	"13F\tFlopsy Fish, swimming",
	//140
	"140\tFlopsy Fish, swimming & jumping",
	"141\tFlopsy Fish, jumping in arc",
	"142\tFlopsy Fish, jumping across, right/left (X)",
	"143\tSpray Fish",
	"144\tHorizontal flipper, right/left (X)",
	"145\tBlue Sluggy, falling/on ceiling (X)",
	"146\tPink Sluggy, falling/on ceiling (X)",
	"147\tHorizontal Pipe Exit Left",
	"148\tLarge spring ball",
	"149\tArrow cloud, up",
	"14A\tArrow cloud, up right",
	"14B\tArrow cloud, right",
	"14C\tArrow cloud, down right",
	"14D\tArrow cloud, down",
	"14E\tArrow cloud, down left",
	"14F\tArrow cloud, left",
	//150
	"150\tArrow cloud, up left",
	"151\tArrow cloud, rotating",
	"152\tFlutter",
	"153\tGoonie with Shy Guy",
	"154\tShark Chomp",
	"155\tVery Goonie",
	"156\tCactus Jack, 1/3 (X)",
	"157\tWall Lakitu",
	"158\tBowling Goonie",
	"159\tGrunt, walking",
	"15A\tGrunt, running",
	"15B\tDancing Spear Guy",
	"15C\tSpiked platform switch, green",
	"15D\tSpiked platform switch, red",
	"15E\tPink Flatbed Ferry pinwheel with Shy Guys, clockwise/counterclockwise (X)",
	"15F\tSpiked platform, green",
	//160
	"160\tSpiked platform, red",
	"161\tBonus item, Red Coin/key/flower/door (YX)",
	"162\tDouble spiked platform with switch",
	"164\tNipper Plant",
	"165\tNipper Spore",
	"166\tThunder Lakitu, 1/2 (X)",
	"167\tGreen Koopa Shell",
	"168\tRed Koopa Shell",
	"169\tGreen Beach Koopa",
	"16A\tRed Beach Koopa",
	"16B\tGreen Koopa Troopa",
	"16C\tRed Koopa Troopa",
	"16D\tGreen Koopa Paratroopa, jumping",
	"16E\tRed Koopa Paratroopa, flying horizontally, right/left (X)",
	"16F\tRed Koopa Paratroopa, flying vertically, down/up (X)",
	//170
	"170\tAqua Lakitu",
	"171\tNaval Piranha",
	"173\tBaron Von Zeppelin, Shy Guy",
	"174\tBaron Von Zeppelin, Needlenose",
	"175\tBaron Von Zeppelin, bomb",
	"176\tBaron Von Zeppelin, Bandit",
	"177\tBaron Von Zeppelin, large spring ball",
	"178\tBaron Von Zeppelin, 1-UP",
	"179\tBaron Von Zeppelin, key",
	"17A\tBaron Von Zeppelin, 5 coins",
	"17B\tBaron Von Zeppelin, watermelon",
	"17C\tBaron Von Zeppelin, fire watermelon",
	"17D\tBaron Von Zeppelin, icy watermelon",
	"17E\tBaron Von Zeppelin, crate, 6 stars",
	//180
	"180\tSpinning log",
	"181\tCrazee Dayzee/Bubble Dayzee (X)",
	"182\tDragonfly ambience",
	"183\tButterfly ambience",
	"184\tBumpty",
	"185\tFlatbed Ferry, green active line-guided left",
	"186\tFlatbed Ferry, green active line-guided right",
	"187\tFlatbed Ferry, yellow active line-guided left",
	"188\tFlatbed Ferry, yellow active line-guided right",
	"189\tFlatbed Ferry, green line-guided left",
	"18A\tFlatbed Ferry, green line-guided right",
	"18B\tFlatbed Ferry, yellow line-guided left",
	"18C\tFlatbed Ferry, yellow line-guided right",
	"18D\tFlatbed Ferry, red line-guided left",
	"18E\tFlatbed Ferry, red line-guided right",
	"18F\tWhirly lift",
	//190
	"190\tFalling icicle",
	"191\tBird ambience",
	"192\tMufti Guy, green/red/yellow/pink (YX)",
	"193\tCaged Ghost, squeezed in sewer",
	"194\tBlargg",
	"197\tArrow sign, up/right/left/down (YX)",
	"198\tArrow sign, up left/up right/down left/down right (YX)",
	"199\tDizzy Dandy",
	"19A\tBoo Guy",
	"19B\tBumpty, sliding",
	"19C\tBumpty, flying",
	"19D\tSkeleton Goonie",
	"19E\tFlightless Skeleton Goonie",
	"19F\tSkeleton Goonie with bomb",
	//1A0
	"1A0\tFirebar, 2 ends, clockwise/counterclockwise (X)",
	"1A1\tFirebar, 1 end, clockwise/counterclockwise (X)",
	"1A3\tLittle Skull Mouser",
	"1A4\tCork sealing 3D pipe",
	"1A5\tGrinder, runs away",
	"1A6\tGrinder, spits seeds",
	"1A7\tGrinder, Short Fuse right/Short Fuse left/Seedy Sally right/Seedy Sally left (YX)",
	"1A8\tGrinder, grabs Baby Mario",
	"1A9\tGrinder, climbs & spits seeds",
	"1AA\tHot Lips",
	"1AB\tBoo Balloon, coin/! switch (X)",
	"1AC\tFrog ambience",
	"1AD\tKamek, shoots magic",
	"1AF\tCoin"};
BYTE spriteDlgData_t1[] = {
	//000
	0x02,0x0E,0x07,0xFF,0xFF,0xFF,
	0x04,0x0E,0x07,0xFF,0xFF,0xFF,
	0x06,0x0E,0x07,0xFF,0xFF,0xFF,
	0x08,0x0E,0x07,0xFF,0xFF,0xFF,
	0x0B,0x0E,0x07,0xFF,0xFF,0xFF,
	0x0E,0x0C,0x03,0xFF,0xFF,0xFF,
	0x0F,0x10,0x08,0xFF,0xFF,0xFF,
	//010
	0x11,0x0E,0x07,0xFF,0xFF,0xFF,
	0x13,0x0E,0x07,0xFF,0xFF,0xFF,
	0x14,0x0E,0x07,0xFF,0xFF,0xFF,
	0x15,0x0E,0x07,0xFF,0xFF,0xFF,
	0x18,0x0E,0x07,0xFF,0xFF,0xFF,
	0x19,0x0E,0x07,0xFF,0xFF,0xFF,
	//020
	0x26,0x0E,0x07,0xFF,0xFF,0xFF,
	0x29,0x0E,0x07,0xFF,0xFF,0xFF,
	0x2A,0x0E,0x07,0xFF,0xFF,0xFF,
	0x2B,0x0E,0x07,0xFF,0xFF,0xFF,
	0x2E,0x0E,0x07,0xFF,0xFF,0xFF,
	//030
	0x35,0x1A,0x08,0xFF,0xFF,0xFF,
	0x38,0x0E,0x07,0xFF,0xFF,0xFF,
	//040
	0x40,0x0E,0x07,0xFF,0xFF,0xFF,
	0x41,0x0E,0x08,0xFF,0xFF,0xFF,
	0x43,0x0E,0x07,0xFF,0xFF,0xFF,
	0x44,0x0E,0x07,0xFF,0xFF,0xFF,
	0x47,0x0E,0x07,0xFF,0xFF,0xFF,
	0x49,0x0E,0x07,0xFF,0xFF,0xFF,
	0x4A,0x0E,0x07,0xFF,0xFF,0xFF,
	0x4B,0x0E,0x07,0xFF,0xFF,0xFF,
	0x4D,0x0E,0x07,0xFF,0xFF,0xFF,
	//050
	0x53,0x0E,0x07,0xFF,0xFF,0xFF,
	0x5A,0x0E,0x07,0xFF,0xFF,0xFF,
	0x5D,0x0E,0x07,0xFF,0xFF,0xFF,
	//060
	0x60,0x0E,0x07,0xFF,0xFF,0xFF,
	0x61,0x0E,0x07,0xFF,0xFF,0xFF,
	0x6B,0x0E,0x07,0xFF,0xFF,0xFF,
	//070
	0x75,0x0E,0x07,0xFF,0xFF,0xFF,
	0x7B,0x0E,0x07,0xFF,0xFF,0xFF,
	0x7C,0x0E,0x07,0xFF,0xFF,0xFF,
	0x7D,0x0E,0x07,0xFF,0xFF,0xFF,
	0x7E,0x0E,0x07,0xFF,0xFF,0xFF,
	//080
	0x83,0x0E,0x07,0xFF,0xFF,0xFF,
	0x86,0x0E,0x07,0xFF,0xFF,0xFF,
	0x8C,0x0E,0x07,0xFF,0xFF,0xFF,
	//090
	0x91,0x0E,0x07,0xFF,0xFF,0xFF,
	0x99,0x0E,0x07,0xFF,0xFF,0xFF,
	0x9C,0x0E,0x07,0xFF,0xFF,0xFF,
	//0A0
	0xA1,0x0E,0x07,0xFF,0xFF,0xFF,
	0xA2,0x0E,0x07,0xFF,0xFF,0xFF,
	0xA8,0x0E,0x07,0xFF,0xFF,0xFF,
	0xA9,0x0E,0x07,0xFF,0xFF,0xFF,
	0xAB,0x0E,0x07,0xFF,0xFF,0xFF,
	0xAC,0x0E,0x07,0xFF,0xFF,0xFF,
	//0B0
	0xB3,0x0E,0x07,0xFF,0xFF,0xFF,
	//0C0
	0xC9,0x0E,0x07,0xFF,0xFF,0xFF,
	0xCD,0x0E,0x07,0xFF,0xFF,0xFF,
	0xCE,0x0E,0x07,0xFF,0xFF,0xFF,
	0xCF,0x0E,0x05,0xFF,0xFF,0xFF,
	//0D0
	0xD5,0x08,0x00,0xFF,0xFF,0xFF,
	0xDD,0x0E,0x04,0xFF,0xFF,0xFF,
	0xDE,0x0E,0x07,0xFF,0xFF,0xFF,
	//0E0
	0xE5,0x0E,0x07,0xFF,0xFF,0xFF,
	0xEF,0x0E,0x07,0xFF,0xFF,0xFF,
	//0F0
	0xF0,0x0E,0x07,0xFF,0xFF,0xFF,
	0xF9,0x0E,0x07,0xFF,0xFF,0xFF,
	0xFE,0x0E,0x07,0xFF,0xFF,0xFF,
	//100
	0x00,0x0F,0x07,0xFF,0xFF,0xFF,
	0x07,0x0F,0x07,0xFF,0xFF,0xFF,
	//110
	0x12,0x0F,0x07,0xFF,0xFF,0xFF,
	0x14,0x0F,0x07,0xFF,0xFF,0xFF,
	0x15,0x0F,0x07,0xFF,0xFF,0xFF,
	0x17,0x0F,0x07,0xFF,0xFF,0xFF,
	0x18,0x0F,0x07,0xFF,0xFF,0xFF,
	0x1C,0x0F,0x07,0xFF,0xFF,0xFF,
	0x1D,0x0F,0x07,0xFF,0xFF,0xFF,
	//120
	0x21,0x0F,0x07,0xFF,0xFF,0xFF,
	0x27,0x0F,0x07,0xFF,0xFF,0xFF,
	0x28,0x0F,0x07,0xFF,0xFF,0xFF,
	0x29,0x0F,0x07,0xFF,0xFF,0xFF,
	0x2D,0x0F,0x07,0xFF,0xFF,0xFF,
	0x2E,0x0F,0x06,0xFF,0xFF,0xFF,
	//130
	0x3B,0x0F,0x07,0xFF,0xFF,0xFF,
	//160
	0x63,0x0F,0x07,0xFF,0xFF,0xFF,
	//170
	0x72,0x0F,0x07,0xFF,0xFF,0xFF,
	0x7F,0x0F,0x07,0xFF,0xFF,0xFF,
	//190
	0x95,0x0F,0x07,0xFF,0xFF,0xFF,
	0x96,0x0F,0x07,0xFF,0xFF,0xFF,
	//1A0
	0xA2,0x0F,0x07,0xFF,0xFF,0xFF,
	0xAE,0x0F,0x07,0xFF,0xFF,0xFF,
	//1B0
	0xB0,0x0F,0x03,0xFF,0xFF,0xFF,
	0xB1,0x0F,0x07,0xFF,0xFF,0xFF,
	0xB2,0x0F,0x07,0xFF,0xFF,0xFF,
	0xB3,0x0F,0x07,0xFF,0xFF,0xFF,
	0xB4,0x0F,0x07,0xFF,0xFF,0xFF,
	0xB5,0x0F,0x07,0xFF,0xFF,0xFF,
	0xB6,0x0F,0x07,0xFF,0xFF,0xFF,
	0xB7,0x0F,0x07,0xFF,0xFF,0xFF,
	0xB8,0x0F,0x05,0xFF,0xFF,0xFF,
	0xB9,0x0F,0x07,0xFF,0xFF,0xFF};
LPCTSTR spriteDlgNames_t1[] = {
	//000
	"002\tNaval Piranha's stalk",
	"004\tItem from Star Mario block",
	"006\tIcy watermelon's ice breath",
	"008\tRubble from final boss",
	"00B\tKaboomba's cannonball",
	"00E\t\"GOAL!\" text",
	"00F\t\"BONUS CHALLENGE!\" text",
	//010
	"010\tItem from minigame",
	"013\tBoss explosion",
	"014\tKey from boss",
	"015\tTorpedo from Submarine Yoshi",
	"018\tFire watermelon's fire breath",
	"019\tBubble",
	//020
	"026\tGiant Egg from final boss",
	"029\tEgg transformation for Super Baby Mario",
	"02A\tRed Giant Egg",
	"02B\tGreen Giant Egg",
	"02E\tPart of Salvo the Slime",
	//030
	"035\tRoger the Potted Ghost's ghost part",
	"038\tRoger the Potted Ghost's fire",
	//040
	"040\tBaby Luigi from end cutscene",
	"041\tStork from end cutscene",
	"043\tRed Giant Shy Guy from Prince Froggy",
	"044\tGreen Giant Shy Guy from Prince Froggy",
	"047\tShy Guy from Roger the Potted Ghost",
	"049\tFire Lakitu's fire",
	"04A\tFire Lakitu's fire",
	"04B\tFire Lakitu's fire",
	"04D\tUnknown",
	//050
	"053\tKamek, \"OH MY!\"",
	"05A\tRaphael the Raven's sparks",
	"05D\tCrash",
	//060
	"060\tBomb",
	"061\tBaby Mario",
	"06B\tHit Green Egg Block",
	//070
	"075\tSpike's spikeball",
	"07B\tRed Bullet Bill",
	"07C\tYellow Bullet Bill",
	"07D\tGreen Bullet Bill",
	"07E\tSquishy block dent",
	//080
	"083\tCloud from final boss",
	"086\tCrash",
	"08C\tYoshi from goal cutscene",
	//090
	"091\tGroup of 4 Toadies",
	"099\tWall Lakitu's Spiny Egg",
	"09C\tMace Guy's mace",
	//0A0
	"0A1\tPotted Spiked Fun Guy's pot",
	"0A2\tThunder Lakitu's thunderbolt",
	"0A8\tIncoming Chomp, falling",
	"0A9\tIncoming Chomp, shadow",
	"0AB\tFill Eggs command",
	"0AC\tSignal arrow from final boss",
	//0B0
	"0B3\tFuzzy cloud",
	//0C0
	"0C9\tWinged Cloud, nothing (crash)",
	"0CD\tBaron Von Zeppelin, Giant Egg from final boss",
	"0CE\tBaby Bowser's flame",
	"0CF\tBaby Bowser's quake",
	//0D0
	"0D5\tMountains from Hookbill the Koopa",
	"0DD\tBoss closer from Naval Piranha",
	"0DE\tFalling rock platform",
	//0E0
	"0E5\tGreen Needlenose",
	"0EF\tEggo-Dill's face",
	//0F0
	"0F0\tEggo-Dill's petal",
	"0F9\tYellow Needlenose",
	"0FE\tZeus Guy's lightning ball",
	//100
	"100\t1-UP in bubble",
	"107\tWatermelon's seed",
	//110
	"112\tGeorgette Jelly splashed",
	"114\tSnifit's bullet",
	"115\tCoin",
	"117\tDonut Lift",
	"118\tGiant Donut Lift",
	"11C\tLakitu's cloud",
	"11D\tLakitu's Spiny Egg",
	//120
	"121\tExplosion of number platform",
	"127\tSpike log on a chain, wheel part",
	"128\tBowser's wave",
	"129\tFuzzy",
	"12D\tYoshi from intro cutscene",
	"12E\tBoss pop effect",
	//130
	"13B\tStomach juice",
	//160
	"163\tGreen Needlenose",
	//170
	"172\tNaval Piranha's stalk",
	"17F\tBaron Von Zeppelin",
	//190
	"195\tUnbalanced snowy platform, small",
	"196\tUnbalanced snowy platform, large",
	//1A0
	"1A2\tStar",
	"1AE\tKamek's magic",
	//1B0
	"1B0\tLarge balloon platform's balloon part",
	"1B1\tMinigame coin cannon",
	"1B2\tMinigame coin",
	"1B3\tMinigame Bandit (Gather Coins)",
	"1B4\tMinigame checker platform",
	"1B5\tMinigame Bandit (Pop Balloons)",
	"1B6\tMinigame balloon",
	"1B7\tMinigame Bandit (Seed Spitting)",
	"1B8\tMinigame watermelon",
	"1B9\tMinigame Bandit (Seed Spitting 2P)"};
BYTE spriteDlgData_t2[] = {
	//1B0
	0xBA,0x0F,0x04,0xFF,0xFF,0xFF,
	0xBB,0x0F,0x04,0xFF,0xFF,0xFF,
	0xBC,0x0F,0x04,0xFF,0xFF,0xFF,
	0xBD,0x0F,0x04,0xFF,0xFF,0xFF,
	0xBE,0x0F,0x04,0xFF,0xFF,0xFF,
	0xBF,0x0F,0x04,0xFF,0xFF,0xFF,
	//1C0
	0xC0,0x0F,0x04,0xFF,0xFF,0xFF,
	0xC1,0x0F,0x04,0xFF,0xFF,0xFF,
	0xC2,0x0F,0x04,0xFF,0xFF,0xFF,
	0xC3,0x0F,0x04,0xFF,0xFF,0xFF,
	0xC4,0x0F,0x04,0xFF,0xFF,0xFF,
	0xC5,0x0F,0x04,0xFF,0xFF,0xFF,
	0xC6,0x0F,0x04,0xFF,0xFF,0xFF,
	0xC7,0x0F,0x04,0xFF,0xFF,0xFF,
	0xC8,0x0F,0x04,0xFF,0xFF,0xFF,
	0xC9,0x0F,0x04,0xFF,0xFF,0xFF,
	0xCA,0x0F,0x04,0xFF,0xFF,0xFF,
	0xCB,0x0F,0x04,0xFF,0xFF,0xFF,
	0xCC,0x0F,0x04,0xFF,0xFF,0xFF,
	0xCD,0x0F,0x04,0xFF,0xFF,0xFF,
	0xCE,0x0F,0x04,0xFF,0xFF,0xFF,
	0xCF,0x0F,0x04,0xFF,0xFF,0xFF,
	//1D0
	0xD0,0x0F,0x04,0xFF,0xFF,0xFF,
	0xD1,0x0F,0x04,0xFF,0xFF,0xFF,
	0xD2,0x0F,0x04,0xFF,0xFF,0xFF,
	0xD3,0x0F,0x04,0xFF,0xFF,0xFF,
	0xD4,0x0F,0x04,0xFF,0xFF,0xFF,
	0xD5,0x0F,0x05,0xFF,0xFF,0xFF,
	0xD6,0x0F,0x05,0xFF,0xFF,0xFF,
	0xD7,0x0F,0x06,0xFF,0xFF,0xFF,
	0xD8,0x0F,0x05,0xFF,0xFF,0xFF,
	0xD9,0x0F,0x06,0xFF,0xFF,0xFF,
	0xDA,0x0F,0x05,0xFF,0xFF,0xFF,
	0xDB,0x0F,0x06,0xFF,0xFF,0xFF,
	0xDC,0x0F,0x05,0xFF,0xFF,0xFF,
	0xDD,0x0F,0x05,0xFF,0xFF,0xFF,
	0xDE,0x0F,0x05,0xFF,0xFF,0xFF,
	0xDF,0x0F,0x05,0xFF,0xFF,0xFF,
	//1E0
	0xE0,0x0F,0x05,0xFF,0xFF,0xFF,
	0xE1,0x0F,0x05,0xFF,0xFF,0xFF,
	0xE2,0x0F,0x04,0xFF,0xFF,0xFF,
	0xE3,0x0F,0x04,0xFF,0xFF,0xFF,
	0xE4,0x0F,0x04,0xFF,0xFF,0xFF,
	0xE5,0x0F,0x06,0xFF,0xFF,0xFF,
	0xE6,0x0F,0x05,0xFF,0xFF,0xFF,
	0xE7,0x0F,0x05,0xFF,0xFF,0xFF,
	0xE8,0x0F,0x05,0xFF,0xFF,0xFF,
	0xE9,0x0F,0x00,0xFF,0xFF,0xFF,
	0xEA,0x0F,0x00,0xFF,0xFF,0xFF,
	0xEB,0x0F,0x02,0xFF,0xFF,0xFF,
	0xEC,0x0F,0x02,0xFF,0xFF,0xFF,
	0xED,0x0F,0x04,0xFF,0xFF,0xFF,
	0xEE,0x0F,0x04,0xFF,0xFF,0xFF,
	0xEF,0x0F,0x02,0xFF,0xFF,0xFF,
	//1F0
	0xF0,0x0F,0x05,0xFF,0xFF,0xFF,
	0xF1,0x0F,0x05,0xFF,0xFF,0xFF,
	0xF2,0x0F,0x05,0xFF,0xFF,0xFF,
	0xF3,0x0F,0x06,0xFF,0xFF,0xFF,
	0xF4,0x0F,0x06,0xFF,0xFF,0xFF};
LPCTSTR spriteDlgNames_t2[] = {
	//1B0
	"1BA\tGraphics/Palette Changer 00",
	"1BB\tGraphics/Palette Changer 01",
	"1BC\tGraphics/Palette Changer 02",
	"1BD\tGraphics/Palette Changer 03",
	"1BE\tGraphics/Palette Changer 04",
	"1BF\tGraphics/Palette Changer 05",
	//1C0
	"1C0\tGraphics/Palette Changer 06",
	"1C1\tGraphics/Palette Changer 07",
	"1C2\tGraphics/Palette Changer 08",
	"1C3\tGraphics/Palette Changer 09",
	"1C4\tGraphics/Palette Changer 0A",
	"1C5\tGraphics/Palette Changer 0B",
	"1C6\tGraphics/Palette Changer 0C",
	"1C7\tGraphics/Palette Changer 0D",
	"1C8\tGraphics/Palette Changer 0E",
	"1C9\tGraphics/Palette Changer 0F",
	"1CA\tSpecial Auto-Scroll 00",
	"1CB\tSpecial Auto-Scroll 01",
	"1CC\tSpecial Auto-Scroll 02",
	"1CD\tSpecial Auto-Scroll 03",
	"1CE\tSpecial Auto-Scroll 04",
	"1CF\tSpecial Auto-Scroll 05",
	//1D0
	"1D0\tSpecial Auto-Scroll 06",
	"1D1\tSpecial Auto-Scroll 07",
	"1D2\tSpecial Auto-Scroll 08",
	"1D3\tSpecial Auto-Scroll 09",
	"1D4\tSpecial Auto-Scroll 0A",
	"1D5\tBoo Balloon End",
	"1D6\tLock Scroll Horizontal",
	"1D7\tGusty Generator",
	"1D8\tGusty Generator End",
	"1D9\tLakitu End",
	"1DA\tFuzzy Generator End",
	"1DB\tPoochy End",
	"1DC\tFang Generator",
	"1DD\tFang Generator End",
	"1DE\tFang 2 Generator",
	"1DF\tFang 2 Generator End",
	//1E0
	"1E0\tWall Lakitu Generator",
	"1E1\tWall Lakitu Generator End",
	"1E2\tDancing Spear Guy Dance 0/1/2/3 (X/Y&1)",
	"1E3\tDancing Spear Guy End Dance",
	"1E4\tThunder Lakitu End",
	"1E5\tFlutter Generator",
	"1E6\tFlutter Generator End",
	"1E7\tNipper Spore Generator",
	"1E8\tNipper Spore Generator End",
	"1E9\tBaron Von Zeppelin (Needlenose) Generator",
	"1EA\tBaron Von Zeppelin (Needlenose) Generator End",
	"1EB\tBaron Von Zeppelin (Bomb) Generator",
	"1EC\tBaron Von Zeppelin (Bomb) Generator End",
	"1ED\tBalloon Platform Generator",
	"1EE\tBalloon Platform Generator End",
	"1EF\tFlatbed Ferry (4 Yellow) Generator",
	//1F0
	"1F0\tLemon Drop Generator",
	"1F1\tLemon Drop Generator End",
	"1F2\tFuzzy Effect End",
	"1F3\tGoonie End",
	"1F4\tFuzzy Generator"};
LPCTSTR whatsThisSprite[0x200] = {
	//000
	"A log, either floats on lava (X:0) or water (X:1).\r\nSprite ID: 000",
	"A closed door.\r\nSprite ID: 001",
	"Naval Piranha's stalk\r\nSprite ID: 002",
	"A crate containing a key.\r\nSprite ID: 003",
	"Item from a Star Mario block.\r\nSprite ID: 004",
	"An icy watermelon.\r\nSprite ID: 005",
	"Icy watermelon's ice breath.\r\nSprite ID: 006",
	"A watermelon which.\r\nSprite ID: 007",
	"Rubble from the Giant Baby Bowser boss.\r\nSprite ID: 008",
	"A fire watermelon.\r\nSprite ID: 009",
	"A Kaboomba.\r\nSprite ID: 00A",
	"Kaboomba's cannonball.\r\nSprite ID: 00B",
	"A Raphael the Raven.\r\nSprite ID: 00C",
	"A goal ring.\r\nSprite ID: 00D",
	"\"GOAL!\" text from goal cutscene.\r\nSprite ID: 00E",
	"\"BONUS CHALLENGE!\" text from goal cutscene.\r\nSprite ID: 00F",
	//010
	"A Caged Ghost in the shape of a round mound.\r\nSprite ID: 010",
	"Item card from a minigame.\r\nSprite ID: 011",
	"A boss door. Make sure to set a screen exit!\r\nSprite ID: 012",
	"Boss explosion effect.\r\nSprite ID: 013",
	"Key from defeated boss.\r\nSprite ID: 014",
	"Submarine Yoshi's bomb.\r\nSprite ID: 015",
	"A Bigger Boo.\r\nSprite ID: 016",
	"A Frog Pirate.\r\nSprite ID: 017",
	"Fire watermelon's fire breath.\r\nSprite ID: 018",
	"Bubble.\r\nSprite ID: 019",
	"A ski lift.\r\nSprite ID: 01A",
	"A vertical log, either floats on lava (X:0) or water (X:1).\r\nSprite ID: 01B",
	"A Dr. Freezegood, containing nothing (YX:00), 6 stars (YX:01), a 1-UP (YX:10), or a Bumpty (YX:11).\r\nSprite ID: 01C",
	"A Dr. Freezegood on a ski lift.\r\nSprite ID: 01D",
	"A Shy Guy, either green (YX:00), red (YX:01), yellow (YX:10), or pink (YX:11).\r\nSprite ID: 01E",
	"A set of rotating doors. Destinations are hardcoded.\r\nSprite ID: 01F",
	//020
	"A Bandit.\r\nSprite ID: 020",
	"A ? Bucket, either floats on lava (X:0) or water (X:1).\r\nSprite ID: 021",
	"A flashing egg containing a Red Coin.\r\nSprite ID: 022",
	"A red egg which containing 2 stars.\r\nSprite ID: 023",
	"A yellow egg containing a coin.\r\nSprite ID: 024",
	"A green egg.\r\nSprite ID: 025",
	"Giant egg from the Giant Baby Bowser boss.\r\nSprite ID: 026",
	"A key which can unlock doors.\r\nSprite ID: 027",
	"A Huffin' Puffin'.\r\nSprite ID: 028",
	"Egg transformation carried by Super Baby Mario.\r\nSprite ID: 029",
	"Giant red egg created when Yoshi eats a Fat Guy.\r\nSprite ID: 02A",
	"Giant green egg created when Yoshi eats a Fat Guy.\r\nSprite ID: 02B",
	"A Lunge Fish.\r\nSprite ID: 02C",
	"A Salvo the Slime.\r\nSprite ID: 02D",
	"Part of Salvo the Slime.\r\nSprite ID: 02E",
	"A Little Mouser nest.\r\nSprite ID: 02F",
	//030
	"A Little Mouser.\r\nSprite ID: 030",
	"A Potted Spiked Fun Guy.\r\nSprite ID: 031",
	"A Little Mouser, either hides in nest (X:0) or peeks from behind stuff (X:1).\r\nSprite ID: 032",
	"A Little Mouser spawned from a nest.\r\nSprite ID: 033",
	"A Roger the Potted Ghost.\r\nSprite ID: 034",
	"Roger the Potted Ghost's ghost part.\r\nSprite ID: 035",
	"A fake falling wall.\r\nSprite ID: 036",
	"A Grim Leecher.\r\nSprite ID: 037",
	"Roger the Potted Ghost's fire.\r\nSprite ID: 038",
	"A spinning platform.\r\nSprite ID: 039",
	"A group of 3 Mini Ravens.\r\nSprite ID: 03A",
	"A Mini Raven.\r\nSprite ID: 03B",
	"A Tap-Tap the Red Nose.\r\nSprite ID: 03C",
	"A seesaw platform.\r\nSprite ID: 03D",
	"A skinny platform.\r\nSprite ID: 03E",
	"A block Slime.\r\nSprite ID: 03F",
	//040
	"Baby Luigi from the ending cutscene.\r\nSprite ID: 040",
	"Stork from the ending cutscene.\r\nSprite ID: 041",
	"A vertical pipe entrance down. Make sure to set a screen exit!\r\nSprite ID: 042",
	"Red Giant Shy Guy from the Prince Froggy boss.\r\nSprite ID: 043",
	"Green Giant Shy Guy from the Prince Froggy boss.\r\nSprite ID: 044",
	"A Prince Froggy boss, either before boss (YX:00), inside throat (YX:01), after boss (YX:10), or inside throat with uvula (YX:11).\r\nSprite ID: 045",
	"A Burt the Bashful.\r\nSprite ID: 046",
	"Shy Guy from Roger the Potted Ghost.\r\nSprite ID: 047",
	"A Kamek used for boss cutscenes.\r\nSprite ID: 048",
	"Fire Lakitu's fire.\r\nSprite ID: 049",
	"Fire Lakitu's fire.\r\nSprite ID: 04A",
	"Fire Lakitu's fire.\r\nSprite ID: 04B",
	"An upside-down Blow Hard.\r\nSprite ID: 04C",
	"Unknown, do not use.\r\nSprite ID: 04D",
	"A locked door. Make sure to set a screen exit!\r\nSprite ID: 04E",
	"A Middle Ring.\r\nSprite ID: 04F",
	//050
	"A wide board platform.\r\nSprite ID: 050",
	"An octagonal log platform.\r\nSprite ID: 051",
	"A balloon platform.\r\nSprite ID: 052",
	"Kamek says \"OH MY!\"\r\nSprite ID: 053",
	"An upside down Wild Piranha.\r\nSprite ID: 054",
	"A large green Flatbed Ferry pinwheel platform.\r\nSprite ID: 055",
	"A small pink Flatbed Ferry pinwheel platform.\r\nSprite ID: 056",
	"A Roger Lift who moves in a hardcoded path.\r\nSprite ID: 057",
	"A green Solo Toady.\r\nSprite ID: 058",
	"A Super Star continuation powerup.\r\nSprite ID: 059",
	"Raphael the Raven's sparks.\r\nSPrite ID: 05A",
	"A Coin Bandit.\r\nSprite ID: 05B",
	"A pink Solo Toady.\r\nSprite ID: 05C",
	"Crashes the game, do not use.\r\nSprite ID: 05D",
	"A short board platform.\r\nSprite ID: 05E",
	"A short board platform which spins forever.\r\nSprite ID: 05F",
	//060
	"Bomb.\r\nSprite ID: 060",
	"Baby Mario sprite, handled specially and should not be inserted directly in a level.\r\nSprite ID: 061",
	"A Goomba.\r\nSprite ID: 062",
	"A Muddy Buddy.\r\nSprite ID: 063",
	"A pink Flatbed Ferry pinwheel, either large clockwise (YX:00), large counterclockwise (YX:01), small clockwise (YX:10), or small counterclockwise (YX:11).\r\nSprite ID: 064",
	"A Red Coin.\r\nSprite ID: 065",
	"A Wild Piranha.\r\nSprite ID: 066",
	"A Hidden Winged Cloud, containing 5 stars (YX:00), a seed (YX:01), a flower (YX:10), or a 1-UP (YX:11).\r\nSprite ID: 067",
	"A Flashing Egg Block.\r\nSprite ID: 068",
	"A Red Egg Block.\r\nSprite ID: 069",
	"A Yellow Egg Block.\r\nSprite ID: 06A",
	"Green Egg Block being hit.\r\nSprite ID: 06B",
	"A large spring ball.\r\nSprite ID: 06C",
	"A Hootie the Blue Fish moving clockwise.\r\nSprite ID: 06D",
	"A Hootie the Blue Fish moving counterclockwise.\r\nSprite ID: 06E",
	"A spring ball.\r\nSprite ID: 06F",
	//070
	"A Clawdaddy.\r\nSprite ID: 070",
	"A group of Boos, either a Big Boo with 3 Boos (YX:00), a Big Boo (YX:01), a Big Boo with 3 Boos (YX:10), or a Boo (YX:11).\r\nSprite ID: 071",
	"A Train Bandit.\r\nSprite ID: 072",
	"A pump producing a large balloon platform.\r\nSprite ID: 073",
	"A Spike.\r\nSprite ID: 074",
	"Spike's spikeball.\r\nSprite ID: 075",
	"A Piro Dangle moving clockwise.\r\nSprite ID: 076",
	"A Piro Dangle moving counterclockwise.\r\nSprite ID: 077",
	"A red Bullet Bill blaster.\r\nSprite ID: 078",
	"A yellow Bullet Bill blaster.\r\nSprite ID: 079",
	"A green Bullet Bill blaster.\r\nSprite ID: 07A",
	"Red Bullet Bill.\r\nSprite ID: 07B",
	"Yellow Bullet Bill.\r\nSprite ID: 07C",
	"Green Bullet Bill.\r\nSprite ID: 07D",
	"Dent of squishy block platform.\r\nSprite ID: 07E",
	"A log seesaw platform.\r\nSprite ID: 07F",
	//080
	"A Lava Bubble jumping up and down.\r\nSprite ID: 080",
	"A Lava Bubble jumping back and forth.\r\nSprite ID: 081",
	"A Chain Chomp.\r\nSprite ID: 082",
	"Cloud from the Giant Baby Bowser boss.\r\nSprite ID: 083",
	"A teleport exit. Make sure to set a screen exit!\r\nSprite ID: 084",
	"A Harry Hedgehog.\r\nSprite ID: 085",
	"Crashes the game, do not use.\r\nSprite ID: 086",
	"A red egg containing a 1-UP.\r\nSprite ID: 087",
	"A Super Star powerup.\r\nSprite ID: 088",
	"A red Flatbed Ferry moving horizontally.\r\nSprite ID: 089",
	"A pink Flatbed Ferry moving vertically.\r\nSprite ID: 08A",
	"A Mock Up, either green (X:0) or red (X:1).\r\nSprite ID: 08B",
	"Yoshi from goal cutscene.\r\nSprite ID: 08C",
	"A Fly Guy, containing 5 stars (YX:00), a Red Coin (YX:01), a 1-UP (YX:10), or a 1-UP (YX:11).\r\nSprite ID: 08D",
	"A Kamek used in Bowser's Room.\r\nSprite ID: 08E",
	"A swing of Grinders.\r\nSprite ID: 08F",
	//090
	"A Dangling Ghost.\r\nSprite ID: 090",
	"Group of 4 Toadies from time over cutscene.\r\nSprite ID: 091",
	"A Melon Bug.\r\nSprite ID: 092",
	"A door. Make sure to set a screen exit!\r\nSprite ID: 093",
	"A pink Egg Block platform.\r\nSprite ID: 094",
	"A blue checkered block platform moving horizontally.\r\nSprite ID: 095",
	"A red checkered block platform moving vertically.\r\nSprite ID: 096",
	"A POW block.\r\nSprite ID: 097",
	"A Yoshi block.\r\nSprite ID: 098",
	"Wall Lakitu's Spiny Egg.\r\nSprite ID: 099",
	"A green Flatbed Ferry on a chain.\r\nSprite ID: 09A",
	"A Mace Guy.\r\nSprite ID: 09B",
	"Mace Guy's mace.\r\nSprite ID: 09C",
	"A ! switch.\r\nSprite ID: 09D",
	"A Chomp Rock.\r\nSprite ID: 09E",
	"A Wild Ptooie Piranha, either spits 1 Needlenose (X:0) or 3 Needlenoses (X:1).\r\nSprite ID: 09F",
	//0A0
	"A Tulip.\r\nSprite ID: 0A0",
	"Potted Spiked Fun Guy's pot\r\nSprite ID: 0A1",
	"Thunder Lakitu's thunderbolt.\r\nSprite ID: 0A2",
	"A Bandit taking cover behind the background, looking left.\r\nSprite ID: 0A3",
	"A Bandit taking cover behind the background, looking right.\r\nSprite ID: 0A4",
	"A Nep-Enut (X:0) or Gargantua Blargg (X:1).\r\nSprite ID: 0A5",
	"An Incoming Chomp.\r\nSprite ID: 0A6",
	"A flock of Incoming Chomps.\r\nSprite ID: 0A7",
	"Incoming Chomp falling, who can break through solid ground.\r\nSprite ID: 0A8",
	"Shadow for falling Incoming Chomp.\r\nSprite ID: 0A9",
	"A Shy Guy in the background.\r\nSprite ID: 0AA",
	"Fill Eggs command.\r\nSprite ID: 0AB",
	"Signal arrow from the Giant Baby Bowser boss.\r\nSprite ID: 0AC",
	"A Message Block.\r\nSprite ID: 0AD",
	"A Hookbill the Koopa.\r\nSprite ID: 0AE",
	"A Morph Bubble containing a Car Yoshi powerup.\r\nSprite ID: 0AF",
	//0B0
	"A Morph Bubble containing a Mole Yoshi powerup.\r\nSprite ID: 0B0",
	"A Morph Bubble containing a Helicopter Yoshi powerup.\r\nSprite ID: 0B1",
	"A Morph Bubble containing a Train Yoshi powerup.\r\nSprite ID: 0B2",
	"Fuzzy cloud.\r\nSprite ID: 0B3",
	"A Morph Bubble containing a Submarine Yoshi powerup.\r\nSprite ID: 0B4",
	"A Hidden Winged Cloud, containing a 1-UP (YX:00), 5 stars (YX:01), a ! Switch (YX:10), or 5 stars (YX:11).\r\nSprite ID: 0B5",
	"A Winged Cloud containing 8 coins.\r\nSprite ID: 0B6",
	"A Winged Cloud containing a 1-UP in a bubble.\r\nSprite ID: 0B7",
	"A Winged Cloud containing a flower.\r\nSprite ID: 0B8",
	"A Winged Cloud containing a POW.\r\nSprite ID: 0B9",
	"A Winged Cloud containing stairs, which go right (X:0) or left (X:1).\r\nSprite ID: 0BA",
	"A Winged Cloud containing a platform, which goes right (X:0) or left (X:1).\r\nSprite ID: 0BB",
	"A Winged Cloud containing a Bandit.\r\nSprite ID: 0BC",
	"A Winged Cloud containing a coin.\r\nSprite ID: 0BD",
	"A Winged Cloud containing a 1-UP.\r\nSprite ID: 0BE",
	"A Winged Cloud containing a key.\r\nSprite ID: 0BF",
	//0C0
	"A Winged Cloud containing 3 stars.\r\nSprite ID: 0C0",
	"A Winged Cloud containing 5 stars.\r\nSprite ID: 0C1",
	"A Winged Cloud containing a door.\r\nSprite ID: 0C2",
	"A Winged Cloud which lowers a ground ledge at a hardcoded location.\r\nSprite ID: 0C3",
	"A Winged Cloud containing a watermelon.\r\nSprite ID: 0C4",
	"A Winged Cloud containing a fire watermelon.\r\nSprite ID: 0C5",
	"A Winged Cloud containing an icy watermelon.\r\nSprite ID: 0C6",
	"A Winged Cloud containing a seed for a sunflower with 3 leaves.\r\nSprite ID: 0C7",
	"A Winged Cloud containing a seed for a sunflower with 6 leaves.\r\nSprite ID: 0C8",
	"A Winged Cloud containing nothing which crashes the game, do not use.\r\nSprite ID: 0C9",
	"A large boss door. Make sure to set a screen exit!\r\nSprite ID: 0CA",
	"A Winged Cloud containing a random item.\r\nSprite ID: 0CB",
	"A Winged Cloud containing a ! Switch.\r\nSprite ID: 0CC",
	"Baron Von Zeppelin carrying a Giant Egg from the Giant Baby Bowser boss.\r\nSprite ID: 0CD",
	"Baby Bowser's flames.\r\nSprite ID: 0CE",
	"Baby Bowser's quake.\r\nSprite ID: 0CF",
	//0D0
	"A horizontal pipe exit right. Make sure to set a screen exit!\r\nSprite ID: 0D0",
	"A vertical pipe exit down which is hidden. Make sure to set a screen exit!\r\nSprite ID: 0D1",
	"A Marching Milde.\r\nSprite ID: 0D2",
	"A Marching Milde 1/2 segment.\r\nSprite ID: 0D3",
	"A Marching Milde 1/4 segment.\r\nSprite ID: 0D4",
	"Mountains from Hookbill the Koopa.\r\nSprite ID: 0D5",
	"A Blarggwich who moves in a hardcoded path.\r\nSprite ID: 0D6",
	"A Sluggy the Unshaven.\r\nSprite ID: 0D7",
	"A Chomp signboard.\r\nSprite ID: 0D8",
	"A Fishin' Lakitu.\r\nSprite ID: 0D9",
	"A flower pot, which contains a key (YX:00), 6 stars (YX:01), 6 coins (YX:10), or nothing (YX:11).\r\nSprite ID: 0DA",
	"A squishy block platform.\r\nSprite ID: 0DB",
	"A snowball.\r\nSprite ID: 0DC",
	"Boss closer for Naval Piranha.\r\nSprite ID: 0DD",
	"Falling rock platform.\r\nSprite ID: 0DE",
	"A Piscatory Pete, either blue (X:0) or gold (X:1).\r\nSprite ID: 0DF",
	//0E0
	"A Preying Mantas.\r\nSprite ID: 0E0",
	"A Loch Nestor.\r\nSprite ID: 0E1",
	"A Boo Blah, either normal (X:0) or upside-down (X:1).\r\nSprite ID: 0E2",
	"A Boo Blah carrying a Piro Dangle, either normal (X:0) or upside-down (X:1).\r\nSprite ID: 0E3",
	"A Pokey.\r\nSprite ID: 0E4",
	"Green Needlenose.\r\nSprite ID: 0E5",
	"A Gusty, either left (YX:00), right (YX:01), infinite right (YX:10), or infinite left (YX:11).\r\nSprite ID: 0E6",
	"A Burt, either 2 (X:0) or 1 (X:1).\r\nSprite ID: 0E7",
	"A Goonie, either right (YX:00), left (YX:01), generator right (YX:10), or generator left (YX:11).\r\nSprite ID: 0E8",
	"A group of 3 Flightless Goonies.\r\nSprite ID: 0E9",
	"A Cloud Drop moving vertically.\r\nSprite ID: 0EA",
	"A Cloud Drop moving horizontally.\r\nSprite ID: 0EB",
	"A Flamer Guy jumping around.\r\nSprite ID: 0EC",
	"A Flamer Guy walking.\r\nSprite ID: 0ED",
	"An Eggo-Dill.\r\nSprite ID: 0EE",
	"Eggo-Dill's face.\r\nSprite ID: 0EF",
	//0F0
	"Eggo-Dill's petal.\r\nSprite ID: 0F0",
	"A Bubble-Plant.\r\nSprite ID: 0F1",
	"A Stilt Guy, either green (YX:00), red (YX:01), yellow (YX:10), or pink (YX:11).\r\nSprite ID: 0F2",
	"A Woozy Guy, either green (YX:00), red (YX:01), yellow (YX:10), or pink (YX:11).\r\nSprite ID: 0F3",
	"A Egg-Plant (X:0) or Needlenose-Plant (X:1).\r\nSprite ID: 0F4",
	"A Slugger who hits eggs with his bat.\r\nSprite ID: 0F5",
	"A Huffin' Puffin' parent and 3 children.\r\nSprite ID: 0F6",
	"A Barney Bubble.\r\nSprite ID: 0F7",
	"A Blow Hard.\r\nSprite ID: 0F8",
	"Yellow Needlenose.\r\nSprite ID: 0F9",
	"A flower, which uses a SuperFX texture.\r\nSprite ID: 0FA",
	"A Spear Guy with a long spear.\r\nSprite ID: 0FB",
	"A Spear Guy with a short spear.\r\nSprite ID: 0FC",
	"A Zeus Guy.\r\nSprite ID: 0FD",
	"Zeus Guy's lightning ball.\r\nSprite ID: 0FE",
	"A Poochy.\r\nSprite ID: 0FF",
	//100
	"1-UP in a bubble.\r\nSprite ID: 100",
	"A spike mace with 1 end.\r\nSprite ID: 101",
	"A spike mace with 2 ends.\r\nSprite ID: 102",
	"A Boo Guy controlling a spike mace.\r\nSprite ID: 103",
	"A Jean de Fillet, either right (X:0) or left (X:1).\r\nSprite ID: 104",
	"A Boo Guy brigade carrying bombs left.\r\nSprite ID: 105",
	"A Boo Guy brigade carrying bombs right.\r\nSprite ID: 106",
	"Watermelon's seed.\r\nSprite ID: 107",
	"A Milde.\r\nSprite ID: 108",
	"A Tap-Tap.\r\nSprite ID: 109",
	"A Tap-Tap who stays on ledges.\r\nSprite ID: 10A",
	"A Tap-Tap hopping.\r\nSprite ID: 10B",
	"A spike ball on a chain.\r\nSprite ID: 10C",
	"A Boo Guy controlling a spike ball on a chain, either right (X:0) or left (X:1).\r\nSprite ID: 10D",
	"A crate containing 6 stars.\r\nSprite ID: 10E",
	"A Boo Man Bluff.\r\nSprite ID: 10F",
	//110
	"A flower, which uses character graphics.\r\nSprite ID: 110",
	"A Georgette Jelly.\r\nSprite ID: 111",
	"Georgette Jelly splashed.\r\nSprite ID: 112",
	"A Snifit.\r\nSprite ID: 113",
	"Snifit's bullet.\r\nSprite ID: 114",
	"Coin.\r\nSprite ID: 115",
	"A round platform floating on water.\r\nSprite ID: 116",
	"Donut Lift.\r\nSprite ID: 117",
	"Giant Donut Lift.\r\nSprite ID: 118",
	"A Spooky.\r\nSprite ID: 119",
	"A Green Glove.\r\nSprite ID: 11A",
	"A Lakitu, either 1 (X:0) or 2 (X:1).\r\nSprite ID: 11B",
	"Lakitu's cloud.\r\nSprite ID: 11C",
	"Lakitu's Spiny Egg.\r\nSprite ID: 11D",
	"A brown Arrow Wheel platform.\r\nSprite ID: 11E",
	"A blue Arrow Wheel platform.\r\nSprite ID: 11F",
	//120
	"A double ended arrow lift.\r\nSprite ID: 120",
	"Explosion of Number Platform.\r\nSprite ID: 121",
	"A ? bucket containing a Bandit.\r\nSprite ID: 122",
	"A ? bucket containing 5 coins.\r\nSprite ID: 123",
	"A Stretch, either green (YX:00), red (YX:01), yellow (YX:10), or pink (YX:11).\r\nSprite ID: 124",
	"A Kamek, either from the ending cutscene (X:0) or flying and chasing (X:1).\r\nSprite ID: 125",
	"A spike log on a chain.\r\nSprite ID: 126",
	"Spike log on a chain wheel part.\r\nSprite ID: 127",
	"Bowser's wave.\r\nSprite ID: 128",
	"Fuzzy.\r\nSprite ID: 129",
	"A Shy Guy which hides a Bandit.\r\nSprite ID: 12A",
	"A Fat Guy, either red (X:0) or green (X:1).\r\nSprite ID: 12B",
	"A Fly Guy carrying a Red Coin (X:0) or a Whirly Fly Guy (X:1).\r\nSprite ID: 12C",
	"Yoshi from intro cutscene.\r\nSprite ID: 12D",
	"Boss pop effect.\r\nSprite ID: 12E",
	"A Lava Drop moving horizontally.\r\nSprite ID: 12F",
	//130
	"A Lava Drop moving vertically.\r\nSprite ID: 130",
	"A locked door. Make sure to set a screen exit!\r\nSprite ID: 131",
	"A Lemon Drop.\r\nSprite ID: 132",
	"A Lantern Ghost.\r\nSprite ID: 133",
	"A Baby Bowser.\r\nSprite ID: 134",
	"A Raven circling forever, either counterclockwise (X:0) or clockwise (X:1).\r\nSprite ID: 135",
	"A Raven circling back and forth, either counterclockwise (X:0), or clockwise (X:1).\r\nSprite ID: 136",
	"A 3x6 falling rock platform.\r\nSprite ID: 137",
	"A 3x3 falling rock platform.\r\nSprite ID: 138",
	"A 3x9 falling rock platform.\r\nSprite ID: 139",
	"A 6x3 falling rock platform.\r\nSprite ID: 13A",
	"Stomach juice from the Prince Froggy boss.\r\nSprite ID: 13B",
	"A vertical flipper, either up (X:0) or down (X:1).\r\nSprite ID: 13C",
	"A Fang hanging from the ceiling.\r\nSprite ID: 13D",
	"A Fang flying.\r\nSprite ID: 13E",
	"A Flopsy Fish swimming.\r\nSprite ID: 13F",
	//140
	"A Flopsy Fish swimming and jumping.\r\nSprite ID: 140",
	"A Flopsy Fish jumping in an arc.\r\nSprite ID: 141",
	"A Flopsy Fish jumping across, right (X:0) or left (X:1).\r\nSprite ID: 142",
	"A Spray Fish.\r\nSprite ID: 143",
	"A horizontal flipper, either right (X:0) or left (X:1).\r\nSprite ID: 144",
	"A blue Sluggy, either falling down (X:0) or on ceiling (X:1).\r\nSprite ID: 145",
	"A pink Sluggy, either falling down (X:0) or on ceiling (X:1).\r\nSprite ID: 146",
	"A horizontal pipe exit left. Make sure to set a screen exit!\r\nSprite ID: 147",
	"A large spring ball.\r\nSprite ID: 148",
	"An arrow cloud pointing up.\r\nSprite ID: 149",
	"An arrow cloud pointing up right.\r\nSprite ID: 14A",
	"An arrow cloud pointing right.\r\nSprite ID: 14B",
	"An arrow cloud pointing down right.\r\nSprite ID: 14C",
	"An arrow cloud pointing down.\r\nSprite ID: 14D",
	"An arrow cloud pointing down left.\r\nSprite ID: 14E",
	"An arrow cloud pointing left.\r\nSprite ID: 14F",
	//150
	"An arrow cloud pointing up left.\r\nSprite ID: 150",
	"An arrow cloud rotating.\r\nSprite ID: 151",
	"A Flutter.\r\nSprite ID: 152",
	"A Goonie with a Shy Guy.\r\nSprite ID: 153",
	"A Shark Chomp.\r\nSprite ID: 154",
	"A Very Goonie.\r\nSprite ID: 155",
	"A Cactus Jack, either 1 (X:0) or 3 (X:1).\r\nSprite ID: 156",
	"A Wall Lakitu.\r\nSprite ID: 157",
	"A Bowling Goonie.\r\nSprite ID: 158",
	"A Grunt walking.\r\nSprite ID: 159",
	"A Grunt running.\r\nSprite ID: 15A",
	"A Dancing Spear Guy.\r\nSprite ID: 15B",
	"A green spiked platform switch.\r\nSprite ID: 15C",
	"A red spiked platform switch.\r\nSprite ID: 15D",
	"A pink Flatbed Ferry pinwheel with Shy Guys, either clockwise (X:0) or counterclockwise (X:1).\r\nSprite ID: 15E",
	"A green spiked platform.\r\nSprite ID: 15F",
	//160
	"A red spiked platform.\r\nSprite ID: 160",
	"A bonus item, either Red Coin (YX:00), key (YX:01), flower (YX:10), or door (YX:11).\r\nSprite ID: 161",
	"A double green spiked platform with switch.\r\nSprite ID: 162",
	"Green Needlenose.\r\nSprite ID: 163",
	"A Nipper Plant.\r\nSprite ID: 164",
	"A Nipper Spore.\r\nSprite ID: 165",
	"A Thunder Lakitu, either 1 (X:0) or 2 (X:1).\r\nSprite ID: 166",
	"A Green Koopa Shell.\r\nSprite ID: 167",
	"A Red Koopa Shell.\r\nSprite ID: 168",
	"A Green Beach Koopa.\r\nSprite ID: 169",
	"A Red Beach Koopa.\r\nSprite ID: 16A",
	"A Green Koopa Troopa.\r\nSprite ID: 16B",
	"A Red Koopa Troopa.\r\nSprite ID: 16C",
	"A Green Koopa Paratroopa, jumping.\r\nSprite ID: 16D",
	"A Red Koopa Paratroopa flying horizontally, either right (X:0) or left (X:1).\r\nSprite ID: 16E",
	"A Red Koopa Paratroopa flying vertically, either down (X:0) or up (X:1).\r\nSprite ID: 16F",
	//170
	"An Aqua Lakitu.\r\nSprite ID: 170",
	"A Naval Piranha.\r\nSprite ID: 171",
	"Naval Piranha's bud.\r\nSprite ID: 172",
	"A Baron Von Zeppelin carrying a Shy Guy.\r\nSprite ID: 173",
	"A Baron Von Zeppelin carrying a Needlenose.\r\nSprite ID: 174",
	"A Baron Von Zeppelin carrying a bomb.\r\nSprite ID: 175",
	"A Baron Von Zeppelin carrying a Bandit.\r\nSprite ID: 176",
	"A Baron Von Zeppelin carrying a large spring ball.\r\nSprite ID: 177",
	"A Baron Von Zeppelin carrying a 1-UP.\r\nSprite ID: 178",
	"A Baron Von Zeppelin carrying a key.\r\nSprite ID: 179",
	"A Baron Von Zeppelin carrying 5 coins.\r\nSprite ID: 17A",
	"A Baron Von Zeppelin carrying a watermelon.\r\nSprite ID: 17B",
	"A Baron Von Zeppelin carrying a fire watermelon.\r\nSprite ID: 17C",
	"A Baron Von Zeppelin carrying an icy watermelon.\r\nSprite ID: 17D",
	"A Baron Von Zeppelin carrying a crate with 6 stars.\r\nSprite ID: 17E",
	"Baron Von Zeppelin.\r\nSprite ID: 17F",
	//180
	"A spinning log.\r\nSprite ID: 180",
	"A Crazee Dayzee (X:0) or Bubble Dayzee (X:1).\r\nSprite ID: 181",
	"A dragonfly ambience.\r\nSprite ID: 182",
	"A butterfly ambience.\r\nSprite ID: 183",
	"A Bumpty.\r\nSprite ID: 184",
	"A green active line-guided Flatbed Ferry going left.\nSprite ID: 185",
	"A green active line-guided Flatbed Ferry going right.\r\nSprite ID: 186",
	"A yellow active line-guided Flatbed Ferry going left.\nSprite ID: 187",
	"A yellow active line-guided Flatbed Ferry going right.\r\nSprite ID: 188",
	"A green line-guided Flatbed Ferry going left.\nSprite ID: 189",
	"A green line-guided Flatbed Ferry going right.\r\nSprite ID: 18A",
	"A yellow line-guided Flatbed Ferry going left.\nSprite ID: 18B",
	"A yellow line-guided Flatbed Ferry going right.\r\nSprite ID: 18C",
	"A red line-guided Flatbed Ferry going left.\nSprite ID: 18D",
	"A red line-guided Flatbed Ferry going right.\r\nSprite ID: 18E",
	"A whirly lift.\r\nSprite ID: 18F",
	//190
	"",
	"A bird ambience.\r\nSprite ID: 191",
	"A Mufti Guy, either green (YX:00), red (YX:01), yellow (YX:10), or pink (YX:11).\r\nSprite ID: 192",
	"A Caged Ghost, squeezed in a sewer pipe.\r\nSprite ID: 193",
	"A Blargg.\r\nSprite ID: 194",
	"Small unbalanced snowy platform.\r\nSprite ID: 195",
	"Large unbalanced snowy platform.\r\nSprite ID: 196",
	"An arrow sign, either up (YX:00), right (YX:01), left (YX:10), or down (YX:11).\r\nSprite ID: 197",
	"An arrow sign, either up left (YX:00), up right (YX:01), down left (YX:10), or down right (YX:11).\r\nSprite ID: 198",
	"A Dizzy Dandy.\r\nSprite ID: 199",
	"A Boo Guy.\r\nSprite ID: 19A",
	"A Bumpty sliding.\r\nSprite ID: 19B",
	"A Bumpty flying.\r\nSprite ID: 19C",
	"A Skeleton Goonie.\r\nSprite ID: 19D",
	"A Flightless Skeleton Goonie.\r\nSprite ID: 19E",
	"A Skeleton Goonie with a bomb.\r\nSprite ID: 19F",
	//1A0
	"A firebar with 2 ends, either clockwise (X:0) or counterclockwise (X:1).\r\nSprite ID: 1A0",
	"A firebar with 1 end, either clockwise (X:0) or counterclockwise (X:1).\r\nSprite ID: 1A1",
	"Star.\r\nSprite ID: 1A2",
	"A Little Skull Mouser.\r\nSprite ID: 1A3",
	"A cork sealing a 3D pipe.\r\nSprite ID: 1A4",
	"A Grinder running away.\r\nSprite ID: 1A5",
	"A Grinder spitting seeds.\r\nSprite ID: 1A6",
	"A Grinder, either Short Fuse right (YX:00), Short Fuse left (YX:01), Seedy Sally right (YX:10), or Seedy Sally left (YX:11).\r\nSprite ID: 1A7",
	"A Grinder grabbing Baby Mario.\r\nSprite ID: 1A8",
	"A Grinder climbing and spitting seeds.\r\nSprite ID: 1A9",
	"A Hot Lips.\r\nSprite ID: 1AA",
	"A Boo Balloon, either coin (X:0) or ! switch (X:1).\r\nSprite ID: 1AB",
	"A frog ambience.\r\nSprite ID: 1AC",
	"A Kamek shooting magic.\r\nSprite ID: 1AD",
	"Kamek's magic.\r\nSprite ID: 1AE",
	"A Coin.\r\nSprite ID: 1AF",
	//1B0
	"Large balloon platform's balloon part.\r\nSprite ID: 1B0",
	"Minigame coin cannon.\r\nSprite ID: 1B1",
	"Minigame coin.\r\nSprite ID: 1B2",
	"Minigame Bandit for Gather Coins.\r\nSprite ID: 1B3",
	"Minigame checker platform.\r\nSprite ID: 1B4",
	"Minigame Bandit for Pop Balloons.\r\nSprite ID: 1B5",
	"Minigame balloon.\r\nSprite ID: 1B6",
	"Minigame Bandit for Seed Spitting.\r\nSprite ID: 1B7",
	"Minigame watermelon.\r\nSprite ID: 1B8",
	"Minigame Bandit for Seed Spitting 2P.\r\nSprite ID: 1B9",
	"A Graphics/Palette Changer 00.\r\nSprite ID: 1BA",
	"A Graphics/Palette Changer 01.\r\nSprite ID: 1BB",
	"A Graphics/Palette Changer 02.\r\nSprite ID: 1BC",
	"A Graphics/Palette Changer 03.\r\nSprite ID: 1BD",
	"A Graphics/Palette Changer 04.\r\nSprite ID: 1BE",
	"A Graphics/Palette Changer 05.\r\nSprite ID: 1BF",
	//1C0
	"A Graphics/Palette Changer 06.\r\nSprite ID: 1C0",
	"A Graphics/Palette Changer 07.\r\nSprite ID: 1C1",
	"A Graphics/Palette Changer 08.\r\nSprite ID: 1C2",
	"A Graphics/Palette Changer 09.\r\nSprite ID: 1C3",
	"A Graphics/Palette Changer 0A.\r\nSprite ID: 1C4",
	"A Graphics/Palette Changer 0B.\r\nSprite ID: 1C5",
	"A Graphics/Palette Changer 0C.\r\nSprite ID: 1C6",
	"A Graphics/Palette Changer 0D.\r\nSprite ID: 1C7",
	"A Graphics/Palette Changer 0E.\r\nSprite ID: 1C8",
	"A Graphics/Palette Changer 0F.\r\nSprite ID: 1C9",
	"A Special Auto-Scroll 00.\r\nSprite ID: 1CA",
	"A Special Auto-Scroll 01.\r\nSprite ID: 1CA",
	"A Special Auto-Scroll 02.\r\nSprite ID: 1CA",
	"A Special Auto-Scroll 03.\r\nSprite ID: 1CA",
	"A Special Auto-Scroll 04.\r\nSprite ID: 1CA",
	"A Special Auto-Scroll 05.\r\nSprite ID: 1CA",
	//1D0
	"A Special Auto-Scroll 06.\r\nSprite ID: 1D0",
	"A Special Auto-Scroll 07.\r\nSprite ID: 1D1",
	"A Special Auto-Scroll 08.\r\nSprite ID: 1D2",
	"A Special Auto-Scroll 09.\r\nSprite ID: 1D3",
	"A Special Auto-Scroll 0A.\r\nSprite ID: 1D4",
	"A Boo Balloon ender.\r\nSprite ID: 1D5",
	"A Lock Horizontal Scroll.\r\nSprite ID: 1D6",
	"A Gusty generator.\r\nSprite ID: 1D7",
	"A Gusty generator ender.\r\nSprite ID: 1D8",
	"A Lakitu ender.\r\nSprite ID: 1D9",
	"A Fuzzy generator ender.\r\nSprite ID: 1DA",
	"A Poochy ender.\r\nSprite ID: 1DB",
	"A Fang generator.\r\nSprite ID: 1DC",
	"A Fang generator ender.\r\nSprite ID: 1DD",
	"A Fang 2 generator.\r\nSprite ID: 1DE",
	"A Fang 2 generator ender.\r\nSprite ID: 1DF",
	//1E0
	"A Wall Lakitu generator.\r\nSprite ID: 1E0",
	"A Wall Lakitu generator ender.\r\nSprite ID: 1E1",
	"A Dancing Spear Guy dance trigger, either dance 0 (YX:00), dance 1 (YX:01), dance 2 (YX:02), or dance 3 (YX:03).\r\nSprite ID: 1E2",
	"A Dancing Spear Guy dance ender.\r\nSprite ID: 1E3",
	"A Thunder Lakitu ender.\r\nSprite ID: 1E4",
	"A Flutter generator.\r\nSprite ID: 1E5",
	"A Flutter generator ender.\r\nSprite ID: 1E6",
	"A Nipper Spore generator.\r\nSprite ID: 1E7",
	"A Nipper Spore generator ender.\r\nSprite ID: 1E8",
	"A Baron Von Zeppelin (Needlenose) generator.\r\nSprite ID: 1E9",
	"A Baron Von Zeppelin (Needlenose) generator ender.\r\nSprite ID: 1EA",
	"A Baron Von Zeppelin (bomb) generator.\r\nSprite ID: 1EB",
	"A Baron Von Zeppelin (bomb) generator ender.\r\nSprite ID: 1EC",
	"A balloon platform generator.\r\nSprite ID: 1ED",
	"A balloon platform generator ender.\r\nSprite ID: 1EE",
	"A 4 yellow line-guided Flatbed Ferry generator.\r\nSprite ID: 1EF",
	//1F0
	"A Lemon Drop generator.\r\nSprite ID: 1F0",
	"A Lemon Drop generator ender.\r\nSprite ID: 1F1",
	"A Fuzzy effect ender.\r\nSprite ID: 1F2",
	"A Goonie generator ender.\r\nSprite ID: 1F3",
	"A Fuzzy generator.\r\nSprite ID: 1F4",
	"Index out of bounds, do not use.\r\nSprite ID: 1F5",
	"Index out of bounds, do not use.\r\nSprite ID: 1F6",
	"Index out of bounds, do not use.\r\nSprite ID: 1F7",
	"Index out of bounds, do not use.\r\nSprite ID: 1F8",
	"Index out of bounds, do not use.\r\nSprite ID: 1F9",
	"Index out of bounds, do not use.\r\nSprite ID: 1FA",
	"Index out of bounds, do not use.\r\nSprite ID: 1FB",
	"Index out of bounds, do not use.\r\nSprite ID: 1FC",
	"Index out of bounds, do not use.\r\nSprite ID: 1FD",
	"Index out of bounds, do not use.\r\nSprite ID: 1FE",
	"Index out of bounds, do not use.\r\nSprite ID: 1FF"};

int focusSprite(int x,int y,UINT * cursor,TCHAR * text) {
	//Check each sprite
	for(int n=(spriteContexts[0].sprites.size()-1); n>=0; n--) {
		sprite_t * thisSprite = &spriteContexts[0].sprites[n];
		int id = thisSprite->data[0]|(thisSprite->data[1]<<8);
		id &= 0x1FF;
		int xpos = (thisSprite->data[2])<<4;
		int ypos = (thisSprite->data[1]>>1)<<4;
		//Check each sprite tile for intersection
		for(int i=0; i<thisSprite->tiles.size(); i++) {
			sprite_tile_t * thisSpriteTile = &thisSprite->tiles[i];
			int xpos2 = xpos+thisSpriteTile->offsX;
			int ypos2 = ypos+thisSpriteTile->offsY;
			switch(thisSpriteTile->tile&0xC000) {
				case 0x0000: {
					int tileSize = (thisSpriteTile->props&1)?16:8;
					RECT tileRect = {xpos2,ypos2,xpos2+tileSize,ypos2+tileSize};
					if(PtInRect(&tileRect,{x,y})) {
						*cursor = 0x7F86; //IDC_SIZEALL
						_tcscpy(text,whatsThisSprite[id]);
						return 5;
					}
					break;
				}
				case 0x4000: {
					RECT tileRect = {xpos2,ypos2,xpos2+16,ypos2+16};
					if(PtInRect(&tileRect,{x,y})) {
						*cursor = 0x7F86; //IDC_SIZEALL
						_tcscpy(text,whatsThisSprite[id]);
						return 5;
					}
					break;
				}
				case 0x8000: {
					RECT tileRect = {xpos2,ypos2,xpos2+256,ypos2+1};
					if(PtInRect(&tileRect,{x,y})) {
						*cursor = 0x7F86; //IDC_SIZEALL
						_tcscpy(text,whatsThisSprite[id]);
						return 5;
					}
					break;
				}
				case 0xC000: {
					RECT tileRect = {xpos2,ypos2,xpos2+8,ypos2+8};
					if(PtInRect(&tileRect,{x,y})) {
						*cursor = 0x7F86; //IDC_SIZEALL
						_tcscpy(text,whatsThisSprite[id]);
						return 5;
					}
					break;
				}
			}
		}
	}
	//Return default
	return 4;
}

//Control updaters
void updateWindowSub_sprite() {
	int prevCtx = setSpriteContext(1);
	int idx = SendMessage(hwndCbSprite,CB_GETCURSEL,0,0);
	int idx2 = SendMessage(hwndLbSprite,LB_GETCURSEL,0,0);
	if(idx==0) {
		loadSprites(&spriteDlgData_t0[idx2*6]);
	} else if(idx==1) {
		loadSprites(&spriteDlgData_t1[idx2*6]);
	} else if(idx==2) {
		loadSprites(&spriteDlgData_t2[idx2*6]);
	}
	drawSprites();
	setSpriteContext(prevCtx);
}
void updateWindow_sprite() {
	//Remove previous elements
	int prevSize = SendMessage(hwndLbSprite,LB_GETCOUNT,0,0);
	for(int i=0; i<prevSize; i++) {
		SendMessage(hwndLbSprite,LB_DELETESTRING,0,0);
	}
	//Add new elements and select first
	int idx = SendMessage(hwndCbSprite,CB_GETCURSEL,0,0);
	if(idx==0) {
		int arrSize = sizeof(spriteDlgNames_t0)/sizeof(LPCTSTR);
		for(int i=0; i<arrSize; i++) {
			SendMessage(hwndLbSprite,LB_ADDSTRING,0,(LPARAM)spriteDlgNames_t0[i]);
		}
	} else if(idx==1) {
		int arrSize = sizeof(spriteDlgNames_t1)/sizeof(LPCTSTR);
		for(int i=0; i<arrSize; i++) {
			SendMessage(hwndLbSprite,LB_ADDSTRING,0,(LPARAM)spriteDlgNames_t1[i]);
		}
	} else if(idx==2) {
		int arrSize = sizeof(spriteDlgNames_t2)/sizeof(LPCTSTR);
		for(int i=0; i<arrSize; i++) {
			SendMessage(hwndLbSprite,LB_ADDSTRING,0,(LPARAM)spriteDlgNames_t2[i]);
		}
	}
	SendMessage(hwndLbSprite,LB_SETCURSEL,0,0);
	updateWindowSub_sprite();
}
//Main drawing code
void updateEntireScreen_sp() {
	memset(bmpDataSp,0x80,0x10000*sizeof(DWORD));
	updateWindowSub_sprite();
	int prevCtx = setSpriteContext(1);
	dispSprites(bmpDataSp,0x100,0x100,{0,0,0x100,0x100});
	setSpriteContext(prevCtx);
}

LRESULT CALLBACK WndProc_Sprite(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {
	switch(msg) {
		//Creation and destruction of window(s)
		case WM_CREATE: {
			//Add controls
			hwndCbSprite = CreateWindow(WC_COMBOBOX,NULL,CBS_DROPDOWNLIST|WS_CHILD|WS_VISIBLE|WS_TABSTOP|WS_VSCROLL,
				0,256,256,100,
				hwnd,(HMENU)20,hinstMain,NULL);
			hwndLbSprite = CreateWindow(WC_LISTBOX,NULL,LBS_NOTIFY|LBS_NOINTEGRALHEIGHT|LBS_USETABSTOPS|WS_CHILD|WS_VISIBLE|WS_TABSTOP|WS_VSCROLL,
				0,282,256,102,
				hwnd,(HMENU)25,hinstMain,NULL);
			//Create objects
			hdcSp = GetDC(hwnd);
			BITMAPINFO bmi;
			memset(&bmi.bmiHeader,0,sizeof(BITMAPINFOHEADER));
			bmi.bmiHeader.biSize			= sizeof(BITMAPINFOHEADER);
			bmi.bmiHeader.biPlanes			= 1;
			bmi.bmiHeader.biBitCount		= 32;
			bmi.bmiHeader.biCompression		= BI_RGB;
			bmi.bmiHeader.biWidth			= 0x100;
			bmi.bmiHeader.biHeight			= -0x100;
			hbmpSp = CreateDIBSection(hdcSp,&bmi,DIB_RGB_COLORS,(void**)&bmpDataSp,NULL,0);
			memset(bmpDataSp,0,0x10000*sizeof(DWORD));
			//Setup font
			int ppi = GetDeviceCaps(hdcSp,LOGPIXELSY);
			int height = -ppi/9;
			HFONT hfont = CreateFont(height,0,0,0,
				FW_NORMAL,FALSE,FALSE,FALSE,
				DEFAULT_CHARSET,
				OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,
				DEFAULT_PITCH|FF_SWISS,"MS Shell Dlg");
			SendMessage(hwndCbSprite,WM_SETFONT,(WPARAM)hfont,FALSE);
			SendMessage(hwndLbSprite,WM_SETFONT,(WPARAM)hfont,FALSE);
			//Init combo boxes
			SendMessage(hwndCbSprite,CB_ADDSTRING,0,(LPARAM)"Standard Sprites");
			SendMessage(hwndCbSprite,CB_ADDSTRING,0,(LPARAM)"Special & Misc. Sprites");
			SendMessage(hwndCbSprite,CB_ADDSTRING,0,(LPARAM)"Commands & Generators");
			SendMessage(hwndCbSprite,CB_SETCURSEL,0,0);
			//Init control values
			updateWindow_sprite();
			break;
		}
		case WM_DESTROY: {
			//Free objects
			DeleteDC(hdcSp);
			DeleteObject(hbmpSp);
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
			//Update
			updateEntireScreen_sp();
			//Blit screen
			PAINTSTRUCT ps;
			BeginPaint(hwnd,&ps);
			HDC hdcMem = CreateCompatibleDC(hdcSp);
			HBITMAP hbmpOld = (HBITMAP)SelectObject(hdcMem,hbmpSp);
			BitBlt(hdcSp,0,0,0x100,0x100,hdcMem,0,0,SRCCOPY);
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
						updateWindow_sprite();
						InvalidateRect(hwnd,&invRect_sprite,false);
						UpdateWindow(hwnd);
						break;
					}
				}
				case 25: {
					if(HIWORD(wParam)==LBN_SELCHANGE) {
						updateWindowSub_sprite();
						InvalidateRect(hwnd,&invRect_sprite,false);
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

