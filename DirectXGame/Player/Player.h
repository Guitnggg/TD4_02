#pragma once

#include "../Game/Stage.h"

#include <math/Vector3.h>

class Player {
public:
	enum class State {
		Aiming,
		Moving,
		Stopped,
	};

	void Initialize(const Stage& stage);
	void Update(const Stage& stage);
	void Reset(const Stage& stage);

	void MoveAimLeft(const Stage& stage);
	void MoveAimRight(const Stage& stage);
	void Fire();

	const KamataEngine::Vector3& GetPosition() const { return position_; }
	State GetState() const { return state_; }
	bool IsClear() const { return isClear_; }
	bool IsFailed() const { return isFailed_; }

private:
	void ReflectByWallOrBounds(const Stage& stage, const Stage::GridPosition& previousGrid, const Stage::GridPosition& currentGrid);

	KamataEngine::Vector3 position_{};
	KamataEngine::Vector3 velocity_{};
	Stage::GridPosition aimGrid_{};
	Stage::GridPosition lastGimmickGrid_{-1, -1};
	State state_ = State::Aiming;
	bool isClear_ = false;
	bool isFailed_ = false;
};

