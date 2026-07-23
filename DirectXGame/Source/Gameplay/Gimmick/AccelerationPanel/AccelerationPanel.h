#pragma once

#include <math/Vector3.h>

/// プレイヤーがマスへ進入したとき、XZ方向の速度を上昇させる床ギミック。
/// ステージと位置を一致させるため、配置位置はグリッド座標で保持する。
class AccelerationPanel {
public:
	enum class Direction { PositiveZ, PositiveX, NegativeZ, NegativeX };
	/// 指定したグリッド座標に生成する。1.0未満の倍率は1.0に補正する。
	AccelerationPanel() = default;
	AccelerationPanel(int gridX, int gridZ, Direction direction = Direction::PositiveZ, float multiplier = 1.5f);

	int GetGridX() const { return gridX_; }
	int GetGridZ() const { return gridZ_; }
	float GetMultiplier() const { return multiplier_; }
	Direction GetDirection() const { return direction_; }
	bool IsAt(int gridX, int gridZ) const;
	/// 現在の進行方向を変えずに加速を適用する。
	/// 矢印と同じ方向へ進んでいる場合だけ、進行方向を変えずに加速する。
	bool Apply(KamataEngine::Vector3& velocity) const;

private:
	int gridX_ = 0;
	int gridZ_ = 0;
	Direction direction_ = Direction::PositiveZ;
	float multiplier_ = 1.5f; // プレイヤーがこの床へ進入するたびに一度だけ適用する倍率。
};
