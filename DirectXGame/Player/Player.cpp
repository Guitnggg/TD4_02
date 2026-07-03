#include "Player.h"

#include <algorithm>
#include <cmath>

namespace {
constexpr float kDeltaTime = 1.0f / 60.0f;
constexpr float kMoveSpeed = 7.0f;
constexpr float kFriction = 0.992f;
constexpr float kStopSpeed = 0.18f;
constexpr float kGoalRadius = 0.55f;
} // namespace

void Player::Initialize(const Stage& stage) { Reset(stage); }

void Player::Update(const Stage& stage) {
	if (state_ != State::Moving) {
		return;
	}

	const Stage::GridPosition previousGrid = stage.WorldToGrid(position_);

	position_.x += velocity_.x * kDeltaTime;
	position_.z += velocity_.z * kDeltaTime;
	velocity_.x *= kFriction;
	velocity_.z *= kFriction;

	const Stage::GridPosition currentGrid = stage.WorldToGrid(position_);
	ReflectByWallOrBounds(stage, previousGrid, currentGrid);

	const Stage::GimmickType gimmick = stage.GetGimmick(currentGrid);
	if (gimmick != Stage::GimmickType::None && (lastGimmickGrid_.x != currentGrid.x || lastGimmickGrid_.z != currentGrid.z)) {
		ReflectByGimmick(gimmick);
		lastGimmickGrid_ = currentGrid;
	} else if (gimmick == Stage::GimmickType::None) {
		lastGimmickGrid_ = {-1, -1};
	}

	const KamataEngine::Vector3 goalPosition = stage.GridToWorld(stage.GetGoalGrid());
	const KamataEngine::Vector3 toGoal = {position_.x - goalPosition.x, 0.0f, position_.z - goalPosition.z};
	if (Length(toGoal) < kGoalRadius) {
		position_ = goalPosition;
		velocity_ = {};
		isClear_ = true;
		state_ = State::Stopped;
		return;
	}

	if (Length(velocity_) < kStopSpeed) {
		velocity_ = {};
		state_ = State::Stopped;
		isFailed_ = !stage.IsGoal(stage.WorldToGrid(position_));
	}
}

void Player::Reset(const Stage& stage) {
	aimGrid_ = stage.GetPlayerStartGrid();
	position_ = stage.GridToWorld(aimGrid_);
	position_.y = 0.65f;
	velocity_ = {};
	lastGimmickGrid_ = {-1, -1};
	state_ = State::Aiming;
	isClear_ = false;
	isFailed_ = false;
}

void Player::MoveAimLeft(const Stage& stage) {
	if (state_ != State::Aiming) {
		return;
	}
	aimGrid_.x = std::max(stage.GetPlayerMinX(), aimGrid_.x - 1);
	position_ = stage.GridToWorld(aimGrid_);
	position_.y = 0.65f;
}

void Player::MoveAimRight(const Stage& stage) {
	if (state_ != State::Aiming) {
		return;
	}
	aimGrid_.x = std::min(stage.GetPlayerMaxX(), aimGrid_.x + 1);
	position_ = stage.GridToWorld(aimGrid_);
	position_.y = 0.65f;
}

void Player::Fire() {
	if (state_ != State::Aiming) {
		return;
	}
	velocity_ = {0.0f, 0.0f, kMoveSpeed};
	state_ = State::Moving;
}

float Player::Length(const KamataEngine::Vector3& value) {
	return std::sqrt(value.x * value.x + value.y * value.y + value.z * value.z);
}

void Player::ReflectByGimmick(Stage::GimmickType gimmick) {
	const KamataEngine::Vector3 oldVelocity = velocity_;
	if (gimmick == Stage::GimmickType::ReflectSlash) {
		velocity_ = {-oldVelocity.z, 0.0f, -oldVelocity.x};
	} else if (gimmick == Stage::GimmickType::ReflectBackSlash) {
		velocity_ = {oldVelocity.z, 0.0f, oldVelocity.x};
	}
}

void Player::ReflectByWallOrBounds(const Stage& stage, const Stage::GridPosition& previousGrid, const Stage::GridPosition& currentGrid) {
	if (stage.IsInsideGrid(currentGrid) && !stage.IsWall(currentGrid)) {
		return;
	}

	position_ = stage.GridToWorld(previousGrid);
	position_.y = 0.65f;

	if (currentGrid.x != previousGrid.x) {
		velocity_.x *= -1.0f;
	}
	if (currentGrid.z != previousGrid.z) {
		velocity_.z *= -1.0f;
	}
}
