#include "mapGen.h"

// Not sure why we need to reverse byte order...
#define COLOR(num) ( ((num & 0xFF000000) >> 24) | ((num & 0x00FF0000) >> 8) | ((num & 0x0000FF00) << 8) | ((num & 0x000000FF) << 24) )
#define COLOR_RGB(num) ( ((num & 0xFF0000) >> 16) | ((num & 0x00FF00)) | ((num & 0x0000FF) << 16) | (0xFF000000) )

 void generateSimpleWorld(RayCastWorld **worldRef, int mapHeight, int mapWidth) {
	 createWorld(worldRef, mapHeight, mapWidth, 2);
	 RayCastWorld *world = *worldRef;
	 world->cellSize = 1.0;
	 
	for(int i = 0; i < mapHeight; i++) {
		for(int j = 0; j < mapWidth; j++) {
			if(i == j & i > mapHeight / 3 && i < 2 * mapHeight / 3) {
				world->map[i][j] = 1;
			} else {
				world->map[i][j] = 1;
			}
		}
	}
	 
	 for(int i = 0; i < mapHeight; i++) {
		 world->map[i][0] = 0;
		 world->map[i][mapWidth - 1] = 0;
	 }
	 for(int i = 0; i < mapHeight; i++) {
		 world->map[0][i] = 0;
		 world->map[mapHeight - 1][i] = 0;
	 }
	 
	 CellType wallType;
	 wallType.ceilColor = 1215;
	 wallType.bottomWallColor= 5325211;
	 wallType.topWallColor = 531221;
	 wallType.floorColor = 53121;
	 wallType.isWall = 1;
	 wallType.isTranslucent = 0;
	 wallType.keepCasting = 1;
	 wallType.ceilZ = 8.5;
	 wallType.floorZ = 8;
	 
	 CellType emptyType;
	 emptyType.ceilColor = 7744;
	 emptyType.bottomWallColor= 1212;
	 emptyType.topWallColor = 1221;
	 emptyType.floorColor = 215;
	 emptyType.isWall = 0;
	 emptyType.isTranslucent = 0;
	 emptyType.keepCasting = 1;
	 emptyType.ceilZ = 6;
	 emptyType.floorZ = -2;
	 
	 world->types[0] = wallType;
	 world->types[1] = emptyType;
		 
}

void generateGratedWorld(RayCastWorld **worldRef, int mapHeight, int mapWidth) {
	 createWorld(worldRef, mapHeight, mapWidth, mapHeight-1);
	 RayCastWorld *world = *worldRef;
	 world->cellSize = 2.0;
	
	CellType *borderType = &(world->types[0]);
    borderType->ceilColor = 0;
    borderType->bottomWallColor = COLOR_RGB(0x8888bb);
    borderType->topWallColor = 0;
    borderType->floorColor = 0;
    borderType->isTranslucent = 0;
    borderType->keepCasting = 1;
    borderType->ceilZ = 4;
    borderType->floorZ = 3;
    borderType->isWall = 1;
	
	for(int i = 1; i < mapHeight - 1; i++) {
		CellType *type = &(world->types[i]);
		//type->ceilColor = 121 * i;
        type->ceilColor = (i%2==0) ? COLOR_RGB(0xaaaaff) : COLOR_RGB(0x7777cc);
		type->bottomWallColor = COLOR_RGB(0x555590);
		type->topWallColor = type->bottomWallColor;
		type->floorColor = type->ceilColor;
		type->isTranslucent = 0;
		type->keepCasting = 1;
		type->ceilZ = -0.25 * i + 2.25;
		type->floorZ = -0.25 * i - 0.5;
		type->isWall = (type->floorZ > 0);
	}
	 
	 
	for(int i = 1; i < mapHeight - 1; i++) {
		for(int j = 1; j < mapWidth - 1; j++) {
			world->map[i][j] = i;//i > j ? i : j;
		}
	}
	
	for(int i = 0; i < mapHeight; i++) {
		 world->map[i][0] = 0;
		 world->map[i][mapWidth - 1] = 0;
	}
	for(int i = 0; i < mapHeight; i++) {
		 world->map[0][i] = 0;
		 world->map[mapHeight - 1][i] = 0;
	}
}