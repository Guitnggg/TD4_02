#pragma once

#include <math/Vector3.h>

struct SphereCollider {
	KamataEngine::Vector3 center{};
	float radius = 0.0f;
};
