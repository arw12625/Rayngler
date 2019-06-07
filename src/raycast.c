
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "raycast.h"

void createPlayer(Player **playerRef) {
	Player *player = malloc(sizeof(Player));
	*playerRef = player;
	
	player->x = 0;
	player->y = 0;
	player->z = 0;
	player->vx = 0;
	player->vy = 0;
	player->vz = 0;
	player->camX = 0;
	player->camY = 1;
	player->camZ = 0.0;
	
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
	
	float newX = player->x + player->vx * delta;
	float newY = player->y + player->vy * delta;
	float newZ = player->z + (player->vz + world->gravity) * delta;
	
	if(!getCellType(world, newX,newY).isWall) {
		player->x = newX;
		player->y = newY;
	}
	
	CellType type = getCellType(world, player->x, player->y);
	player->z = fmin(fmax(newZ, type.floorZ+player->height), type.ceilZ);
	
}

#define xNegSide 1
#define yNegSide 2
#define xPosSide 3
#define yPosSide 4
typedef struct RayCastIntersection {
	int x,y;
	CellType *type;
	int side;
	float nearHorizDist, farHorizDist;
	float startTop, startBottom;
	float wallTop, wallBottom;
	float ceilTop, floorBottom;
} RayCastIntersection;

#define TO_SCREEN_ROW(coord) ((int)((-fmin(fmax(coord,-1),1) + 1) * settings->screenHeight / 2))


void renderWorld(int *data, RayCastWorld *world, RayCastSettings *settings) {
	
	// Clear data to black. Not necessary once screen is entirely redrawn each frame
	int *tmp = data;
	for(int i = 0; i < settings->screenHeight; i++) {
		for(int j = 0; j < settings->screenWidth; j++) {
			*tmp = 0;
			tmp++;
		}
	}
	
	// Vertical slice of the image to be rendered
	int *slice;
	
	// Camera direction multiplier to define FOV
	float panMult = sqrt((1 - settings->cosXFOV) / (1 + settings->cosXFOV));
	float tiltMult = sqrt((1 - settings->cosYFOV) / (1 + settings->cosYFOV));
	// Composite camera directions
		// Pan - ignoring z component
		// Tilt - combine xy components
	float horizMag = sqrt(world->player->camX * world->player->camX + world->player->camY * world->player->camY);
	float cameraPanX = panMult * world->player->camY / horizMag;
	float cameraPanY = panMult * -world->player->camX / horizMag;
	float cameraTiltHor = tiltMult * -world->player->camZ;
	float cameraTiltVer = tiltMult * horizMag;//(world->player->camZ > 0) ? sqrt(1 - horizMag * horizMag) : -sqrt(1 - horizMag * horizMag);
	
	
	// The map cell coords that the player/camera is occupying
	int playerMapX = (int)(world->player->x / world->cellWidth);
	int playerMapY = (int)(world->player->y / world->cellHeight);
	
	RayCastIntersection *history = malloc(sizeof(RayCastIntersection) * settings->castLimit);
	
	// Iterate over all vertical slices of the screen
	for(int ray = 0; ray < settings->screenWidth; ray++) {
		
		slice = data + ray * settings->screenHeight;
		
		//x-coordinate in camera space
			// -1 is far left, 1 is far right
		float cameraX = 2 * ray / (float)(settings->screenWidth) - 1;
		float rayDirX = world->player->camX + panMult * cameraPanX * cameraX;
		float rayDirY = world->player->camY + panMult * cameraPanY * cameraX;
		
		//printf("X: %f    Y: %f\n",rayDirX,rayDirY);
		
		//rayDirX/rayDirY should be normalized to calculate Euclidean distance, but not doing so corrects fisheye distortion
		float rayDirMag = sqrt(rayDirX * rayDirX + rayDirY * rayDirY);
		rayDirX /= rayDirMag;
		rayDirY /= rayDirMag;
		
	
		float deltaDistX, deltaDistY;
		if(rayDirX < 0.001 && rayDirX > -0.001) {
			deltaDistX = 10000000.0;
		} else {
			deltaDistX = fabs(1.0 / rayDirX);
		}
		if(rayDirY < 0.001 && rayDirY > -0.001) {
			deltaDistY = 10000000.0;
		} else {
			deltaDistY = fabs(1.0 / rayDirY);
		}
		
		//Whether the x/y coord increments or decrements moving along the ray
		int stepX = 0;
		int stepY = 0;
		
		//Initial distances along ray from camera to first x/y line intersection
		float initDistX, initDistY;

		//The coordinates of the map cell the ray cast is currently within
		int mapX = playerMapX;
		int mapY = playerMapY;
		int newMapX = playerMapX;
		int newMapY = playerMapY;
		
		if (rayDirX < 0) {
			stepX = -1;
			initDistX = (world->player->x - mapX * world->cellWidth) * deltaDistX;
		} else {
			stepX = 1;
			initDistX = ((mapX + 1)*world->cellWidth - world->player->x) * deltaDistX;
		}
		if (rayDirY < 0) {
			stepY = -1;
			initDistY = (world->player->y - mapY * world->cellHeight) * deltaDistY;
		} else {
			stepY = 1;
			initDistY = ((mapY + 1) * world->cellHeight - world->player->y) * deltaDistY;
		}
		
		float sideDistX = initDistX;
		float sideDistY = initDistY;
		
		float lastInterDist = 0;
		
		float interDist = 0;
		
		// How much of the screen has been determined/undetermined
		float bottomDet = -1;
		float bottomUndet = -1;
		float topDet = 1;
		float topUndet = 1;
		
		// The number of intersections encountered
		int intersectionNum = 0;
		// The number of intersections undetermined in the history
		int historyLen = 0;
		
		int keepCasting = 1;
		// Cast the ray until the distance limit is reached, the edge of the map is reached, or the slice is completely filled
		while(keepCasting) {
			
			if (sideDistX < sideDistY) {
				interDist = sideDistX;
				sideDistX += deltaDistX;
				newMapX += stepX;
			} else {
				interDist = sideDistY;
				sideDistY += deltaDistY;
				newMapY += stepY;
			}
			
			//printf("X: %d    Y: %d\n",mapX,mapY);
			
			
				RayCastIntersection *intersection = &(history[historyLen]);
				intersection->x = mapX;
				intersection->y = mapY;
				intersection->type = &(world->types[world->map[world->mapHeight - 1 - mapY][mapX]]);
				
				// the world z coordinate at the appropriate distance from the camera along the middle vertical scan
				//float viewNearMidZ = world->player->camZ * lastInterDist / horizMag;
				//float viewFarMidZ = world->player->camZ * interDist / horizMag;
				
				float wallTopRelZ = intersection->type->ceilZ - world->player->z;// - viewNearMidZ;
				float ceilRelZ = intersection->type->ceilZ - world->player->z;// - viewFarMidZ;
				float wallBottomRelZ = intersection->type->floorZ - world->player->z;// - viewNearMidZ;
				float floorRelZ = intersection->type->floorZ - world->player->z;// - viewFarMidZ;
				
				float wallTopProj, wallBottomProj;
				float ceilProj, floorProj;
				if(lastInterDist * horizMag + wallTopRelZ * world->player->camZ <= 0.0001) {
					wallTopProj = 2;
				} else {
					wallTopProj = (wallTopRelZ * horizMag - lastInterDist * world->player->camZ)
									/ (lastInterDist * cameraTiltVer - wallTopRelZ * cameraTiltHor);
				}
				if(lastInterDist * horizMag + wallBottomRelZ * world->player->camZ <= 0.0001) {
					wallBottomProj = -2;
				} else {
					wallBottomProj = (wallBottomRelZ * horizMag - lastInterDist * world->player->camZ)
									/ (lastInterDist * cameraTiltVer - wallBottomRelZ * cameraTiltHor);
				}
				if(interDist * horizMag + ceilRelZ * world->player->camZ <= 0.0001) {
					ceilProj = 2;
				} else {
					ceilProj = (ceilRelZ * horizMag - interDist * world->player->camZ)
									/ (interDist * cameraTiltVer - ceilRelZ * cameraTiltHor);
				}
				if(interDist * horizMag + floorRelZ * world->player->camZ <= 0.0001) {
					floorProj = -2;
				} else {
					floorProj = (floorRelZ * horizMag - interDist * world->player->camZ)
									/ (interDist * cameraTiltVer - floorRelZ * cameraTiltHor);
				}
				
				
				if(ray == 145) {
					//printf("RelZ: %f, Proj: %f\n", (floorRelZ * horizMag - interDist * world->player->camZ), (floorRelZ * cameraTiltHor));
				}
				
				/*
				float wallTopProj = wallTopRelZ * tiltMult * cameraTiltHor + lastInterDist * tiltMult * cameraTiltVer;
				float ceilProj = ceilRelZ * tiltMult * cameraTiltHor + interDist * tiltMult * cameraTiltVer;
				float wallBottomProj = wallBottomRelZ * tiltMult * cameraTiltHor + lastInterDist * tiltMult * cameraTiltVer;
				float floorProj = floorRelZ * tiltMult * cameraTiltHor + interDist * tiltMult * cameraTiltVer;*/
				//printf("%f\n", interDist * tiltMult * cameraTiltVer);
				
				//float viewNearDist = fmax(0.001, sqrt(lastInterDist * lastInterDist + viewNearMidZ * viewNearMidZ));
				//float viewFarDist = fmax(0.001, sqrt(interDist * interDist + viewFarMidZ * viewFarMidZ));
				
				
				intersection->nearHorizDist = lastInterDist;
				intersection->farHorizDist = interDist;
				
				intersection->startTop = topDet;
				intersection->startBottom = bottomDet;
				
				intersection->wallTop = wallTopProj;
				intersection->ceilTop = ceilProj;
				intersection->wallBottom = wallBottomProj;
				intersection->floorBottom = floorProj;
				/*
				intersection->wallTop = wallTopProj / viewNearDist;
				intersection->ceilTop = ceilProj / viewFarDist;
				intersection->wallBottom = wallBottomProj / viewNearDist;
				intersection->floorBottom = floorProj / viewFarDist;
				*/
				
				lastInterDist = interDist;
				historyLen++;
				intersectionNum++;
			
				// for translucent walls encountered, determine which pixels are possibly undetermined
				if(intersection->type->isTranslucent) {
					topUndet = fmin(fmin(topUndet, intersection->wallTop), intersection->ceilTop);
					bottomUndet = fmax(fmax(bottomUndet, intersection->wallBottom), intersection->floorBottom);
				} else {
					topDet = fmin(fmin(topDet, intersection->wallTop), intersection->ceilTop);
					bottomDet = fmax(fmax(bottomDet, intersection->wallBottom), intersection->floorBottom);
				}
				int edgeReached = newMapX < 0 || newMapX >= world->mapWidth || newMapY < 0 || newMapY >= world->mapHeight;
				keepCasting = !edgeReached && (intersectionNum < settings->castLimit) && (topDet > bottomDet) && intersection->type->keepCasting;
				mapX = newMapX;
				mapY = newMapY;
			
			
			// If the pixels are now determined or we are done, render the history
			if((topDet <= topUndet && bottomDet >= bottomUndet) || !keepCasting) {
				//render history
				for(int i = historyLen - 1; i >= 0; i--) {
					RayCastIntersection inter = history[i];
					
					//render top wall
					for(int row = TO_SCREEN_ROW(inter.startTop); row < TO_SCREEN_ROW(fmax(inter.wallTop, inter.startBottom)); row++) {
						*(slice+row) = inter.type->topWallColor;
					}					
					//render ceil
					for(int row = TO_SCREEN_ROW(fmin(inter.startTop, inter.wallTop)); row < TO_SCREEN_ROW(fmax(inter.ceilTop, inter.startBottom)); row++) {
						*(slice+row) = inter.type->ceilColor;
					}					
					//render floor
					for(int row = TO_SCREEN_ROW(fmin(inter.startTop, inter.floorBottom)); row < TO_SCREEN_ROW(fmax(inter.wallBottom, inter.startBottom)); row++) {
						*(slice+row) = inter.type->floorColor;
					}	
					//render bottom wall
					for(int row = TO_SCREEN_ROW(fmin(inter.wallBottom, inter.startTop)); row < TO_SCREEN_ROW(inter.startBottom); row++) {
						*(slice+row) = inter.type->bottomWallColor;
					}
					if(ray == settings->screenHeight / 2) {
						//printf("%f, %f\n", inter.wallBottom, fmax(inter.startTop, inter.startBottom));
					}
				}
				
				topUndet = topDet;
				bottomUndet = bottomDet;
				historyLen = 0;
			} else {
				
			}
		}
		
	}
	
	free(history);
	
	
	
}


CellType getCellType(RayCastWorld *world, float x, float y) {
	return world->types[world->map[world->mapHeight - 1 - (int)(y / world->cellHeight)][(int)(x / world->cellWidth)]];
}
	
