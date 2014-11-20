#include <math.h>
#include <list>
#include <GL/glut.h>
#include "Vector3.h"
#include "Rays.h"
#include "Objects.h"
#include "Collisions.h"

using namespace std;

Ray::Ray(const Vector3& o, const Vector3& d)
{
	origin = o;
	direction = d.normal();
	depth = 0;
	distance = 0.0f;
	color = NULL;
	mediumRefraction = 1.0f;
	insideSphere = false;
}

Ray::Ray(const Vector3& o, const Vector3& d, int n)
{
	origin = o;
	direction = d.normal();
	depth = n;
	distance = 0.0f;
	color = NULL;
	mediumRefraction = 1.0f;
	insideSphere = false;
}

Ray::~Ray()
{
}

bool compare_distance(const Collision& col1, const Collision& col2)
{
	return col1.distance < col2.distance;
}

Collision Ray::getFirstCollision(const list<Object*>& scene)
{
	Collision firstCol;
	list<Collision> collisions;

	//첫번째 충돌 계산
	list<Object*>::const_iterator itr = scene.begin();
	for( ; itr != scene.end(); itr++)
	{
		// 첫번째 빛 충돌
		Collision col = (*itr)->collideWithRay((*this));

		if(col.distance > -1.0f)
			collisions.push_back(col);
	}

	// 충돌이 없을 경우
	if (collisions.size() == 0)
	{
		firstCol.distance = -1.0f;
		return firstCol;
	}

	// 거리가 짧은 경우부터 정렬(구의 경우 두개의 점에서 만날 수 있으므로)
	collisions.sort(compare_distance);

	return *(collisions.begin());
}

Vector3 Ray::shoot(const list<Object*>& scene, const list<Light>& lights, int depth)
{
	Collision col = getFirstCollision(scene);

	// get FirstCollison에서 충돌이 발생 할 경우
	if(col.distance > 0.0f)
	{
		distance = col.distance;
		// emittance 추가( 그림자)
		color += col.material.emittance;
		list<Light>::const_iterator itr = lights.begin();
		for( ; itr != lights.end(); itr++)
		{
			// 빛의 법선 백터 구하기
			Vector3 toLight = ((*itr).position - col.point).normal();
			Ray toLightRay(col.point, toLight);
			bool far_ = toLightRay.is_far(scene, (*itr));
				
			// Ambient
			color += (col.material.ambient).scale((*itr).color);
			
			if(far_) 
			{
				continue;
			}
			else
			{
				// 빛으로 가는 벡터와 Ray와 충돌한 점 사이의 각도 세타/2
				float normAngle = toLight * col.normal;

				// 알파/2
				float specAngle = ((toLight + (direction * -1.0f)) * 0.5f).normal() * col.normal;

				// Diffuse 삽입
				color += (col.material.diffuse).scale((*itr).color) * normAngle;

				// Specular 삽입
				if(specAngle > 0.0f)
					color += col.material.specular.scale((*itr).color) * pow(specAngle, col.material.shininess);
			}
		}

		//Refraction
		if (depth > 0)
		{
			
			if (col.material.reflection > 0.0f && !insideSphere)
			{
				Vector3 dir = direction.reflect(col.normal).normal();
				Ray reflect(col.point, dir);
				reflect.insideSphere = insideSphere;
				Vector3 reflectColor = reflect.shoot(scene, lights, depth-1);
				color += reflectColor  * col.material.reflection;
			} else {}

			// 굴절률 구하기
			if (col.material.transmission > 0.0f)
			{
				//굴절률에 사용되는 변수 구하기
				float c1 = (direction * col.normal) * -1.0f;
				float n = mediumRefraction / (mediumRefraction == 1.0f ? col.material.refractiveIdx : 1.0f);
				float c2 = sqrt( 1 - pow(n,2) * (1 - pow(c1, 2)));
				//굴절 빛의 벡터 구하여 쏘기
				Vector3 norm = (insideSphere ? col.normal * -1.0f : col.normal);
				Vector3 dir = (direction * n) + (norm * (n * c1 - c2));
				Ray refract(col.point, dir);

				//구 속에서 나가는 빛
				refract.insideSphere = !insideSphere;
				refract.mediumRefraction = col.material.refractiveIdx;
				//구 바깥으로 나가는 빛을 shoot으로 재귀
				Vector3 refractColor = refract.shoot(scene, lights, depth-1);
				color += refractColor * col.material.transmission;

				color *= 0.5f;
			} else {}
		}

		return color;
	}
	
	color.set(0.0f,0.0f,0.0f);
	return color;
}


bool Ray::is_far(const list<Object*>& scene, const Light& light)
{
	Collision col = getFirstCollision(scene);
	bool hit = false;
	
	// No distance
	if(col.distance <= -1.0f)
	{
		return hit;
	}

	// Distance
	Vector3 diff = light.position - origin;
	float distToLight = diff.size();

	// 구성요소와 빛과의 거리 중
	// 빛과의 거리가 클 경우
	if(distToLight > col.distance)
	{
		hit = true;
	}
	
	return hit;
}