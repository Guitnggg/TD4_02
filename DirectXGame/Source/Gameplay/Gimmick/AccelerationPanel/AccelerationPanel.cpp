#include "AccelerationPanel.h"

#include <algorithm>

AccelerationPanel::AccelerationPanel(int gridX, int gridZ, float multiplier)
	: gridX_(gridX), gridZ_(gridZ), multiplier_(std::max(multiplier, 1.0f)) {}

bool AccelerationPanel::IsAt(int gridX, int gridZ) const {
	return gridX_ == gridX && gridZ_ == gridZ;
}

void AccelerationPanel::Apply(KamataEngine::Vector3& velocity) const {
	velocity.x *= multiplier_;
	velocity.z *= multiplier_;
}
