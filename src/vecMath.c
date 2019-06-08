#include <math.h>

#include "vecMath.h"


void normalizeVec2(Vec2 *vec) {
	float mag = magnitudeVec2(*vec);
	vec->x /= mag;
	vec->y /= mag;
}
void normalizeVec3(Vec3 *vec) {
	float mag = magnitudeVec3(*vec);
	vec->x /= mag;
	vec->y /= mag;
	vec->z /= mag;
}

float magnitudeVec2(Vec2 vec) {
	return vec.x * vec.x + vec.y * vec.y;
}

float magnitudeVec3(Vec3 vec) {
	return sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
}