#ifndef RAYCAST_H
#define RAYCAST_H

#include "vecMath.h"

/* A cell consists of three regions:
		top: a rectangular prism extending in the z direction from ceilZ to +infty 
		gap: an empty rectangular prism extending in the z direction from floorZ to ceilZ
		bottom: a rectangular prism extending in the z direction from -infty to floorZ
	If the cell represents a uniform wall extending from -infty to +infty, then set isWall = true
		and the cell is treated like floorZ = +infty ( not implemented yet :)
*/
typedef struct CellType {
	int ceilColor, floorColor, bottomWallColor, topWallColor;
	int isWall, isTranslucent, keepCasting;
	float ceilZ;
	float floorZ;
} CellType;

typedef struct Player {
	Vec3 position;
	Vec3 velocity;
	Vec3 camDir;
	float height;
} Player;


typedef struct RayCastWorld {
	int mapWidth,mapHeight;
	float cellSize;
    int **map;
	int numTypes;
	CellType *types;
	float gravity;
	Player *player;
	
} RayCastWorld;

typedef struct RayCastSettings {
	int screenWidth, screenHeight;
	float cosXFOV,cosYFOV;
	
	int castLimit;
	
} RayCastSettings;

void createWorld(RayCastWorld **worldRef, int mapHeight, int mapWidth, int numTypes);
void destroyWorld(RayCastWorld *world);

void updateWorld(float delta, RayCastWorld *world);
	
void renderWorld(int *data, RayCastWorld *world, RayCastSettings *settings);

CellType getCellType(RayCastWorld *world, float x, float y);
#endif