#pragma once

#include <math/Vector3.h>

/// <summary>
/// 球形の当たり判定に必要な情報を持つ構造体
///
/// ・中心座標
/// ・半径
/// を保持する
/// </summary>
struct SphereCollider {
	// 球の中心ワールド座標
	KamataEngine::Vector3 center{};

	// 球の半径
	float radius = 0.0f;
};
