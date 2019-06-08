#ifndef VECMATH_H
#define VECMATH_H


typedef struct Vec2 {
	float x, y;
} Vec2;
typedef struct Vec3 {
	float x, y, z;
} Vec3;

void normalizeVec2(Vec2 *vec);
void normalizeVec3(Vec3 *vec);

float magnitudeVec2(Vec2 vec);
float magnitudeVec3(Vec3 vec);

#endif