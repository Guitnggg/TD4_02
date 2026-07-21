#include "AccelerationPanel.h"

#include <algorithm>
#include <cmath>

AccelerationPanel::AccelerationPanel(int gridX, int gridZ, Direction direction, float multiplier)
	// 1.0未満では減速床になってしまうため、最低値を1.0に制限する。
	: gridX_(gridX), gridZ_(gridZ), direction_(direction), multiplier_(std::max(multiplier, 1.0f)) {}

bool AccelerationPanel::IsAt(int gridX, int gridZ) const {
	return gridX_ == gridX && gridZ_ == gridZ;
}

bool AccelerationPanel::Apply(KamataEngine::Vector3& velocity) const {
	// ゲーム中の移動はXZ平面に限定されるため、Y方向の速度は変更しない。
	const float speed = std::sqrt(velocity.x * velocity.x + velocity.z * velocity.z);
	if (speed <= 0.0001f) {
		return false;
	}

	float arrowX = 0.0f;
	float arrowZ = 0.0f;
	switch (direction_) {
	case Direction::PositiveZ: arrowZ = 1.0f; break;
	case Direction::PositiveX: arrowX = 1.0f; break;
	case Direction::NegativeZ: arrowZ = -1.0f; break;
	case Direction::NegativeX: arrowX = -1.0f; break;
	}

	constexpr float kMinimumDirectionDot = 0.70710678f;
	const float directionDot = (velocity.x * arrowX + velocity.z * arrowZ) / speed;
	if (directionDot < kMinimumDirectionDot) {
		return false;
	}

	velocity.x *= multiplier_;
	velocity.z *= multiplier_;
	return true;
}
