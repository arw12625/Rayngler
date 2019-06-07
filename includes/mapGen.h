#ifndef MAPGEN_H
#define MAPGEN_H

#include "raycast.h"

 void generateSimpleWorld(RayCastWorld **worldRef, int mapHeight, int mapWidth);
 void generateGratedWorld(RayCastWorld **worldRef, int mapHeight, int mapWidth);

#endif