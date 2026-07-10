#pragma once
#include "Collider.h"

#include <math/Vector3.h>

/// <summary>
/// 当たり判定の計算を行うクラス
///
/// ・コライダー同士の衝突判定
/// ・座標と半径を直接指定した衝突判定
/// を担当する
/// </summary>
class CollisionManager {
public:
	/// <summary>
	/// 2つの球コライダーが衝突しているか判定する
	/// </summary>
	/// <param name="sphere1">判定対象の球コライダー1</param>
	/// <param name="sphere2">判定対象の球コライダー2</param>
	/// <returns>衝突していれば true</returns>
	static bool SphereCollision(const SphereCollider& sphere1, const SphereCollider& sphere2);

	/// <summary>
	/// 2つの球が衝突しているか、中心座標と半径から判定する
	/// </summary>
	/// <param name="center1">球1の中心ワールド座標</param>
	/// <param name="radius1">球1の半径</param>
	/// <param name="center2">球2の中心ワールド座標</param>
	/// <param name="radius2">球2の半径</param>
	/// <returns>衝突していれば true</returns>
	static bool SphereCollision(const KamataEngine::Vector3& center1, float radius1, const KamataEngine::Vector3& center2, float radius2);
};
