
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "raycast.h"

void createPlayer(Player **playerRef) {
	Player *player = malloc(sizeof(Player));
	*playerRef = player;
	
	player->position.x = 0;
	player->position.y = 0;
	player->position.z = 0;
	player->velocity.x = 0;
	player->velocity.y = 0;
	player->velocity.z = 0;
	player->camDir.x = 0;
	player->camDir.y = 1;
	player->camDir.z = 0;
	
}

void createWorld(RayCastWorld **worldRef, int mapHeight, int mapWidth, int numTypes) {
	
	RayCastWorld *world = malloc(sizeof(RayCastWorld));
	*worldRef = world;
	
	world->mapWidth = mapWidth;
	world->mapHeight = mapHeight;
	
	world->map = malloc(sizeof(int*) * mapHeight);
	int *mapData = malloc(sizeof(int) * mapHeight * mapWidth);
	for(int i = 0; i < mapHeight; i++) {
		world->map[i] = mapData + i * mapWidth;
	}
	
	world->numTypes = numTypes;
	world->types = malloc(sizeof(CellType) * numTypes);
	
	createPlayer(&(world->player));
	
}

void destroyWorld(RayCastWorld *world) {
	
	free(world->player);
	free(world->types);
	
	free(world->map[0]);
	free(world->map);
	
	free(world);
}

void updateWorld(float delta, RayCastWorld *world) {
	
	
	Player *player = world->player;
	
	float newX = player->position.x + player->velocity.x * delta;
	float newY = player->position.y + player->velocity.y * delta;
	float newZ = player->position.z + (player->velocity.z + world->gravity) * delta;
	
	CellType potType = getCellType(world, newX,newY);
	if(!potType.isWall && potType.floorZ < player->position.z - player->height + player->stepHeight) {
		player->position.x = newX;
		player->position.y = newY;
	}
	
	CellType type = getCellType(world, player->position.x, player->position.y);
	player->position.z = fmin(fmax(newZ, type.floorZ+player->height), type.ceilZ);
	
}

typedef struct Camera {
	Vec3 pos;
	// 3D camera direction
	Vec3 dir;
	// Horizontal and Vertical camera direction (contract x/y)
	Vec2 dirSide;
	// Vectors relaitve to camDirSide that define the topmost and rightMost parts of the camera view pyramid/frustum
	Vec2 top, right;
} Camera;

Camera initCamera(Player *player, RayCastSettings *settings);

#define xNegSide 1
#define yNegSide 2
#define xPosSide 3
#define yPosSide 4
//data for determining the intersection of the camera ray with the map grid
typedef struct RayCastingPlaneData {
	Vec2 rayDir;
	float origRayMag;
	Vec2 deltaDist;
	int stepX, stepY;
	int mapX, mapY;
	Vec2 initSideDist, sideDist;
	float interDist;
	int side;
	int mapWidth, mapHeight;
	int edgeReached;
} RayCastingPlaneData;

RayCastingPlaneData initRayCastingPlaneData(float normCamX, Camera *cam, RayCastWorld *world);
void findNextIntersection(RayCastingPlaneData *data);

//data for rendering a single map cell
typedef struct RayCastCell {
	int x,y;
	CellType *type;
	int side;
	float nearHorizDist, farHorizDist;
	float startTop, startBottom;
	float wallTop, wallBottom;
	float ceilTop, floorBottom;
} RayCastCell;

typedef struct RayCastProjectionData {
	
	// How much of the screen has been determined/undetermined
	float bottomDet, bottomUndet;
	float topDet, topUndet;
		
	// The number of intersections undetermined in the history
	int historyLen;
	RayCastCell *history;
	int keepCasting, renderable;
	
	float lastInterDist;
	int lastMapX, lastMapY;
	
	
} RayCastProjectionData;

RayCastProjectionData initRayCastProjectionData(Camera *cam, RayCastCell *history, RayCastWorld *world);
void projectCell(Camera *cam, RayCastingPlaneData *data, RayCastProjectionData *projData, RayCastWorld *world);

void renderHistory(RayCastProjectionData *projData, int *slice, RayCastSettings *settings);

#define TO_SCREEN_ROW(coord, h) ((int)((-fmin(fmax(coord,-1),1) + 1) * h / 2))

void renderWorld(int *data, RayCastWorld *world, RayCastSettings *settings) {
	
	// Clear data to black. Not necessary once screen is entirely redrawn each frame
	
	int *tmp = data;
	for(int i = 0; i < settings->screenHeight; i++) {
		for(int j = 0; j < settings->screenWidth; j++) {
			*tmp = 0;
			tmp++;
		}
	}
	
	Player *player = world->player;
	Camera cam = initCamera(player, settings);
	
	// Vertical slice of the image to be rendered
	int *slice;
	
	RayCastCell *history = malloc(sizeof(RayCastCell) * settings->castLimit);
	
	// Iterate over all vertical slices of the screen
	for(int screenColumn = 0; screenColumn < settings->screenWidth; screenColumn++) {
		
		float camNormX = 2 * screenColumn / (float)(settings->screenWidth) - 1;
		
		slice = data + screenColumn * settings->screenHeight;
		
		RayCastingPlaneData planeData = initRayCastingPlaneData(camNormX, &cam, world);
		RayCastProjectionData projData = initRayCastProjectionData(&cam, history, world);
		
		// The number of intersections encountered
		int intersectionNum = 0;
		int keepCasting = 1;
		
		// Cast the ray until the distance limit is reached, the edge of the map is reached, or the slice is completely filled
		while(keepCasting) {
			
			findNextIntersection(&planeData);
			intersectionNum++;
			
			projectCell(&cam, &planeData, &projData, world);
			
			
			keepCasting = projData.keepCasting && (intersectionNum < settings->castLimit);
			
			// If the pixels are now determined or we are done, render the history
			if(projData.renderable || !keepCasting) {
				//render history
				renderHistory(&projData, slice, settings);
			} else {
				
			}
		}
		
	}
	
	free(history);
	
}

Camera initCamera(Player *player, RayCastSettings *settings) {
	
	Camera cam;
	cam.pos = player->position;
	cam.dir = player->camDir;
	
	float panMult = sqrt((1 - settings->cosXFOV) / (1 + settings->cosXFOV));
	float tiltMult = sqrt((1 - settings->cosYFOV) / (1 + settings->cosYFOV));
	
	cam.dirSide.x = sqrt(cam.dir.x * cam.dir.x + cam.dir.y * cam.dir.y);
	cam.dirSide.y = cam.dir.z;
	cam.top.x = tiltMult * -cam.dirSide.y;
	cam.top.y = tiltMult * cam.dirSide.x;
	cam.right.x = panMult * cam.dir.y / cam.dirSide.x;
	cam.right.y = panMult * -cam.dir.x / cam.dirSide.x;
	
	return cam;
}

RayCastingPlaneData initRayCastingPlaneData(float normCamX, Camera *cam, RayCastWorld *world) {
	RayCastingPlaneData data;
	
	data.mapWidth = world->mapWidth;
	data.mapHeight = world->mapHeight;
	
	data.rayDir.x = cam->dir.x + cam->right.x * normCamX;
	data.rayDir.y = cam->dir.y + cam->right.y * normCamX;
	data.origRayMag = magnitudeVec2(data.rayDir);
	
	//rayDir should be normalized to calculate Euclidean distance, but not doing so corrects fisheye distortion
	normalizeVec2(&data.rayDir);
	
	if(data.rayDir.x < 0.001 && data.rayDir.x > -0.001) {
		data.deltaDist.x = 10000000.0;
	} else {
		data.deltaDist.x = world->cellSize * fabs(1.0 / data.rayDir.x);
	}
	if(data.rayDir.y < 0.001 && data.rayDir.y > -0.001) {
		data.deltaDist.y = 10000000.0;
	} else {
		data.deltaDist.y  = world->cellSize * fabs(1.0 / data.rayDir.y);
	}
	
	data.mapX = (int)(cam->pos.x / world->cellSize);
	data.mapY = (int)(cam->pos.y / world->cellSize);
	
	if (data.rayDir.x < 0) {
		data.stepX = -1;
		data.initSideDist.x = (cam->pos.x / world->cellSize - data.mapX) * data.deltaDist.x;
	} else {
		data.stepX = 1;
		data.initSideDist.x = ((data.mapX + 1) - cam->pos.x / world->cellSize) * data.deltaDist.x;
	}
	if (data.rayDir.y < 0) {
		data.stepY = -1;
		data.initSideDist.y = (cam->pos.y / world->cellSize - data.mapY) * data.deltaDist.y;
	} else {
		data.stepY = 1;
		data.initSideDist.y = ((data.mapY + 1) - cam->pos.y / world->cellSize) * data.deltaDist.y;
	}
	
	data.sideDist.x = data.initSideDist.x;
	data.sideDist.y = data.initSideDist.y;
	
	data.interDist = 0;
	data.side = 0;
	
	return data;
}

void findNextIntersection(RayCastingPlaneData *planeData) {
	
	if (planeData->sideDist.x < planeData->sideDist.y) {
		planeData->interDist = planeData->sideDist.x;
		planeData->sideDist.x += planeData->deltaDist.x;
		planeData->mapX += planeData->stepX;
		//need to check if this is correct
		if(planeData->stepX > 0) {
			planeData->side = xNegSide;
		} else {
			planeData->side = xPosSide;
		}
	} else {
		planeData->interDist = planeData->sideDist.y;
		planeData->sideDist.y += planeData->deltaDist.y;
		planeData->mapY += planeData->stepY;
		if(planeData->stepY > 0) {
			planeData->side = yNegSide;
		} else {
			planeData->side = yPosSide;
		}
	}
	planeData->edgeReached = planeData->mapX < 0 || planeData->mapX >= planeData->mapWidth || planeData->mapY < 0 || planeData->mapY >= planeData->mapHeight;
	
	
}

RayCastProjectionData initRayCastProjectionData(Camera *cam, RayCastCell *history, RayCastWorld *world) {
	RayCastProjectionData projData;
	
	projData.bottomDet = -1;
	projData.bottomUndet = -1;
	projData.topDet = 1;
	projData.topUndet = 1;
	
	projData.historyLen = 0;
	projData.history = history;
	
	projData.lastInterDist = 0;
	projData.lastMapX =  (int)(cam->pos.x / world->cellSize);
	projData.lastMapY = (int)(cam->pos.y / world->cellSize);
	
	return projData;
	
}
void projectCell(Camera *cam, RayCastingPlaneData *planeData, RayCastProjectionData *projData, RayCastWorld *world) {
	RayCastCell *intCell = &(projData->history[projData->historyLen]);
	intCell->x = projData->lastMapX;
	intCell->y = projData->lastMapY;
	intCell->nearHorizDist = projData->lastInterDist;
	intCell->farHorizDist = planeData->interDist;
	projData->lastMapX = planeData->mapX;
	projData->lastMapY = planeData->mapY;
	projData->lastInterDist = planeData->interDist;
	CellType* type = &(world->types[world->map[world->mapHeight - 1 - intCell->y][intCell->x]]);
	intCell->type = type;
	
	float wallTopRelZ = type->ceilZ - cam->pos.z;
	float ceilRelZ = type->ceilZ - cam->pos.z;
	float wallBottomRelZ = type->floorZ - cam->pos.z;
	float floorRelZ = type->floorZ - cam->pos.z;
	
	float wallTopProj, wallBottomProj;
	float ceilProj, floorProj;
	float horizMag = cam->dirSide.x * planeData->origRayMag;
	//should probably find a better solution than this tolerance
	if(intCell->nearHorizDist * horizMag + wallTopRelZ * cam->dirSide.y <= 0.1) {
		wallTopProj = 2;
	} else {
		wallTopProj = (wallTopRelZ * horizMag - intCell->nearHorizDist * cam->dirSide.y)
					/ (intCell->nearHorizDist * cam->top.y- wallTopRelZ * cam->top.x);
	}
	if(intCell->nearHorizDist * horizMag + wallBottomRelZ * cam->dirSide.y <= 0.1) {
		wallBottomProj = -2;
	} else {
		wallBottomProj = (wallBottomRelZ * horizMag - intCell->nearHorizDist * cam->dirSide.y)
					/ (intCell->nearHorizDist * cam->top.y - wallBottomRelZ * cam->top.x);
		}
	if(intCell->farHorizDist * horizMag + ceilRelZ * cam->dirSide.y <= 0.1) {
			ceilProj = 2;
	} else {
		ceilProj = (ceilRelZ * horizMag - intCell->farHorizDist * cam->dirSide.y)
					/ (intCell->farHorizDist * cam->top.y - ceilRelZ * cam->top.x);
	}
	if(intCell->farHorizDist * horizMag + floorRelZ * cam->dirSide.y <= 0.1) {
		floorProj = -2;
	} else {
		floorProj = (floorRelZ * horizMag - intCell->farHorizDist * cam->dirSide.y)
					/ (intCell->farHorizDist * cam->top.y - floorRelZ * cam->top.x);
	}
	
	
	intCell->startTop = projData->topDet;
	intCell->startBottom = projData->bottomDet;
		
	intCell->wallTop = wallTopProj;
	intCell->ceilTop = ceilProj;
	intCell->wallBottom = wallBottomProj;
	intCell->floorBottom = floorProj;
	
	projData->historyLen++;
	
	if(type->isTranslucent) {
		projData->topUndet = fmin(fmin(projData->topUndet, intCell->wallTop), intCell->ceilTop);
		projData->bottomUndet = fmax(fmax(projData->bottomUndet, intCell->wallBottom), intCell->floorBottom);
	} else {
		projData->topDet = fmin(fmin(projData->topDet, intCell->wallTop), intCell->ceilTop);
		projData->bottomDet = fmax(fmax(projData->bottomDet, intCell->wallBottom), intCell->floorBottom);
	}
	
	projData->keepCasting = !planeData->edgeReached && type->keepCasting && projData->topDet > projData->bottomDet;
	projData->renderable = projData->topDet <= projData->topUndet && projData->bottomDet >= projData->bottomUndet;

	
}


void renderHistory(RayCastProjectionData *projData, int *slice, RayCastSettings *settings) {
	int h = settings->screenHeight;
	for(int i = projData->historyLen - 1; i >= 0; i--) {
		RayCastCell inter = projData->history[i];
		//render top wall
		for(int row = TO_SCREEN_ROW(inter.startTop, h); row < TO_SCREEN_ROW(fmax(inter.wallTop, inter.startBottom), h); row++) {
			*(slice+row) = inter.type->topWallColor;
		}					
		//render ceil
		for(int row = TO_SCREEN_ROW(fmin(inter.startTop, inter.wallTop), h); row < TO_SCREEN_ROW(fmax(inter.ceilTop, inter.startBottom), h); row++) {
			*(slice+row) = inter.type->ceilColor;
		}					
		//render floor
		for(int row = TO_SCREEN_ROW(fmin(inter.startTop, inter.floorBottom), h); row < TO_SCREEN_ROW(fmax(inter.wallBottom, inter.startBottom), h); row++) {
			*(slice+row) = inter.type->floorColor;
		}	
		//render bottom wall
		for(int row = TO_SCREEN_ROW(fmin(inter.wallBottom, inter.startTop), h); row < TO_SCREEN_ROW(inter.startBottom, h); row++) {
			*(slice+row) = inter.type->bottomWallColor;
		}
	}
	projData->topUndet = projData->topDet;
	projData->bottomUndet = projData->bottomDet;
	projData->historyLen = 0;
}

CellType getCellType(RayCastWorld *world, float x, float y) {
	return world->types[world->map[world->mapHeight - 1 - (int)(y / world->cellSize)][(int)(x / world->cellSize)]];
}
	
