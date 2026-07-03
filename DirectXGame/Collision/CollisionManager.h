#pragma once
#include "Collider.h"

#include <math/Vector3.h>

class CollisionManager {
public:
	static bool SphereCollision(const SphereCollider& sphere1, const SphereCollider& sphere2);

	static bool SphereCollision(const KamataEngine::Vector3& center1, float radius1, const KamataEngine::Vector3& center2, float radius2);
};

