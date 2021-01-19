#include "sprite.h"

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
//Helper function for drawing text
void drawSpriteText(sprite_t * s,std::string text,int width) {
	s->text = text;
	//Create dummy sprites to draw text on top of
	//(needed to mark space as occupied)
	width -= 0x10;
	for(int i = 0; i < width; i += 0x10) {
		//TODO
	}
	if(width&8); //TODO
}

//TODO

//Sprite function pointer table and updater
void drawSprite_unused(sprite_t * s) {
	//TODO
}
//TODO
int setSpriteContext(int ctx) {
	int prevCtx = curSpCtx;
	curSpCtx = ctx;
	return prevCtx;
}
void drawSprites() {
	for(int n = 0; n < spriteContexts[curSpCtx].sprites.size(); n++) {
		//TODO
	}
}
void dispSprites(DWORD * pixelBuf,int width,int height) {
	//TODO
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
	for(int i = 0; i < 0x8000; i++) {
		spriteContexts[curSpCtx].assocSprites[i].clear();
	}
	spriteContexts[curSpCtx].sprites.clear();
	//Reload buffer with sprite data
	while(true) {
		//TODO
	}
	//Draw sprite data
	drawSprites();
}
int saveSprites(BYTE * data) {
	//Init stuff
	int curIdx = 0;
	int curSz = 0;
	//Save sprite data
	while(true) {
		//Shorten the names of these for convenience
		BYTE * thisData = spriteContexts[curSpCtx].sprites[curIdx].data;
		int thisDataSz = spriteContexts[curSpCtx].sprites[curIdx].dataSize;
		//Check if we have room to write object
		if(curSz + thisDataSz > 0x7FFE) break;
		//Copy data
		memcpy(&data[curSz],thisData,thisDataSz);
		//Increment stuff
		curIdx++;
		curSz += thisDataSz;
	}
}

//Manipulation
int selectSprites(RECT rect,bool ctrl) {
	//TODO
}
void addToSpriteSelection(int index) {
	//Select sprite
	sprite_t thisSprite = spriteContexts[0].sprites[index];
	thisSprite.selected = true;
	//Mark occupied tiles as invalid
	for(int i = 0; i < thisSprite.tiles.size(); i++) {
		sprite_tile_t thisTile = thisSprite.tiles[i];
		for(int j = 0; j < thisTile.numOccupiedTiles; j++) {
			spriteContexts[0].invalidSprites[thisTile.occupiedTiles[j]] = true;
		}
	}
}
void removeFromSpriteSelection(int index) {
	//Deselect sprite
	sprite_t thisSprite = spriteContexts[0].sprites[index];
	thisSprite.selected = false;
	//Mark occupied tiles as invalid
	for(int i = 0; i < thisSprite.tiles.size(); i++) {
		sprite_tile_t thisTile = thisSprite.tiles[i];
		for(int j = 0; j < thisTile.numOccupiedTiles; j++) {
			spriteContexts[0].invalidSprites[thisTile.occupiedTiles[j]] = true;
		}
	}
}
void clearSpriteSelection() {
	//Deselect all sprites
	for(int n = 0; n < spriteContexts[0].sprites.size(); n++) {
		sprite_t thisSprite = spriteContexts[0].sprites[n];
		thisSprite.selected = false;
		//Mark occupied tiles as invalid
		for(int i = 0; i < thisSprite.tiles.size(); i++) {
			sprite_tile_t thisTile = thisSprite.tiles[i];
			for(int j = 0; j < thisTile.numOccupiedTiles; j++) {
				spriteContexts[0].invalidSprites[thisTile.occupiedTiles[j]] = true;
			}
		}
	}
}
void insertSprites(int x,int y) {
	int numSelectedSprites = 0;
	int minX = 0x8000,minY = 0x8000;
	//Check if any sprites are to be pasted
	for(int n = 0; n < spriteContexts[0].sprites.size(); n++) {
		if(spriteContexts[0].sprites[n].selected) {
			//TODO
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
	for(int n = 0; n < spriteContexts[0].sprites.size(); n++) {
		sprite_t thisSprite = spriteContexts[0].sprites[n];
		if(thisSprite.selected) {
			//Mark occupied tiles as invalid
			for(int i = 0; i < thisSprite.tiles.size(); i++) {
				sprite_tile_t thisTile = thisSprite.tiles[i];
				for(int j = 0; j < thisTile.numOccupiedTiles; j++) {
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
	//Check if any sprites are to be pasted
	for(int n = 0; n < spriteContexts[0].sprites.size(); n++) {
		if(spriteContexts[0].sprites[n].selected) {
			//TODO
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
LRESULT CALLBACK WndProc_Sprite(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {
	switch(msg) {
		//TODO
	}
}

