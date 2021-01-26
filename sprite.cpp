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
	//Quick way to store 1,2,2,or 4 depending on the offset alignment
	entry.numOccupiedTiles = 1;
	if((offsX & 0xF) > 8) entry.numOccupiedTiles <<= 1;
	if((offsY & 0xF) > 8) entry.numOccupiedTiles <<= 1;
	//Calculate occupied tile data
	if(props&1) {
		//16x16
		//TODO
	} else {
		//8x8
		//TODO
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
	//TODO
}

//HDMA displayer functions
//TODO

//Floating log
void drawSprite_000(sprite_t * s) {
	//TODO
}
//Closed door
void drawSprite_001(sprite_t * s) {
	//TODO
}
//Naval Piranha stalk
void drawSprite_002(sprite_t * s) {
	//TODO
}
//Crate (key)
void drawSprite_003(sprite_t * s) {
	//TODO
}
//TODO

//Sprite function pointer table and updater
void drawSprite_unused(sprite_t * s) {}
void (*spriteDrawFunc[0x200])(sprite_t * s) = {
	//000
	drawSprite_000,drawSprite_001,drawSprite_002,drawSprite_003,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	//010
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	//020
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
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
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
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
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	//110
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
	//120
	drawSprite_unused,drawSprite_unused,drawSprite_unused,drawSprite_unused,
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
		sprite_t thisSprite = spriteContexts[curSpCtx].sprites[n];
		int id = thisSprite.data[0]|(thisSprite.data[1]<<8);
		spriteDrawFunc[id&0x1FF](&thisSprite);
	}
}
void drawSingleSprite(int n) {
	sprite_t thisSprite = spriteContexts[curSpCtx].sprites[n];
	int id = thisSprite.data[0]|(thisSprite.data[1]<<8);
	spriteDrawFunc[id&0x1FF](&thisSprite);
}
void dispSprites(DWORD * pixelBuf,int width,int height,RECT rect) {
	for(int n = 0; n < spriteContexts[curSpCtx].sprites.size(); n++) {
		sprite_t thisSprite = spriteContexts[curSpCtx].sprites[n];
		int spX = 0;
		int spY = 0;
		for(int i=0; i<thisSprite.tiles.size(); i++) {
			sprite_tile_t thisSpriteTile = thisSprite.tiles[i];
			BYTE props = thisSpriteTile.props;
			WORD tile = thisSpriteTile.tile;
			int sptX = spX+thisSpriteTile.offsX;
			int sptY = spY+thisSpriteTile.offsY;
			switch(tile&0xC000) {
				case 0x0000: {
					dispMap8Tile(pixelBuf,width,height,props,tile+0x480,{sptX,sptY});
					break;
				}
				case 0x4000: {
					//TODO
					break;
				}
				case 0x8000: {
					//TODO
					break;
				}
				case 0xC000: {
					//TODO
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

