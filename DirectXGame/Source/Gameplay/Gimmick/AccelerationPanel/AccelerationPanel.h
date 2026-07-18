#pragma once

#include <math/Vector3.h>

class AccelerationPanel {
public:
	AccelerationPanel() = default;
	AccelerationPanel(int gridX, int gridZ, float multiplier = 1.5f);

	int GetGridX() const { return gridX_; }
	int GetGridZ() const { return gridZ_; }
	float GetMultiplier() const { return multiplier_; }
	bool IsAt(int gridX, int gridZ) const;
	void Apply(KamataEngine::Vector3& velocity) const;

private:
	int gridX_ = 0;
	int gridZ_ = 0;
	float multiplier_ = 1.5f;
};
