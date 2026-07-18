#include "AccelerationPanel.h"

#include <algorithm>

AccelerationPanel::AccelerationPanel(int gridX, int gridZ, float multiplier)
	// 1.0未満では減速床になってしまうため、最低値を1.0に制限する。
	: gridX_(gridX), gridZ_(gridZ), multiplier_(std::max(multiplier, 1.0f)) {}

bool AccelerationPanel::IsAt(int gridX, int gridZ) const {
	return gridX_ == gridX && gridZ_ == gridZ;
}

void AccelerationPanel::Apply(KamataEngine::Vector3& velocity) const {
	// ゲーム中の移動はXZ平面に限定されるため、Y方向の速度は変更しない。
	velocity.x *= multiplier_;
	velocity.z *= multiplier_;
}
