#ifndef _RAYS_H_
#define _RAYS_H_
#define MAX_RECURSE_DEPTH 5

#include <list>

#include "Objects.h"
#include "Collisions.h"

class Object;
class Sphere;
struct Light;
struct Collision;

using namespace std;

class Ray
{
public:
	Vector3 origin;
	Vector3 direction;
	Vector3 color;
	float distance;
	float mediumRefraction;
	float attenuation;
	bool insideSphere;

	Ray(const Vector3& o, const Vector3& d);
	Ray(const Vector3& o, const Vector3& d, int n);
	~Ray();

	Collision getFirstCollision(const list<Object*>& scene);
	Vector3 shoot(const list<Object*>& scene, const list<Light>& lights, int depth);
	bool is_far(const list<Object*>& scene, const Light& light);

private:
	int depth;
};
#endif