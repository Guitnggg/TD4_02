#include "CollisionManager.h"

#include "../Math/MathUtility.h"

using namespace KamataEngine;

bool CollisionManager::SphereCollision(const SphereCollider& sphere1, const SphereCollider& sphere2) {
	return SphereCollision(sphere1.center, sphere1.radius, sphere2.center, sphere2.radius);
}

bool CollisionManager::SphereCollision(const Vector3& center1, float radius1, const Vector3& center2, float radius2) {
	const float radiusSum = radius1 + radius2;
	return MyMath::LengthSq(MyMath::Subtract(center1, center2)) <= radiusSum * radiusSum;
}
