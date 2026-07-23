#include "CollisionManager.h"

#include "../Math/MathUtility.h"

using namespace KamataEngine;

bool CollisionManager::SphereCollision(const SphereCollider& sphere1, const SphereCollider& sphere2) {
	// コライダー版も、中心座標と半径を受け取る版と同じ判定処理を使用する。
	return SphereCollision(sphere1.center, sphere1.radius, sphere2.center, sphere2.radius);
}

bool CollisionManager::SphereCollision(const Vector3& center1, float radius1, const Vector3& center2, float radius2) {
	// 不要な平方根計算を避けるため、距離の二乗値で比較する。
	const float radiusSum = radius1 + radius2;
	return MyMath::LengthSq(MyMath::Subtract(center1, center2)) <= radiusSum * radiusSum;
}
