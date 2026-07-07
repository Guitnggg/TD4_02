#include "Player.h"

#include "../../Core/Math/MathUtility.h"

#include <algorithm>

using namespace KamataEngine;

namespace {
constexpr float kDeltaTime = 1.0f / 60.0f;
constexpr float kMoveSpeed = 7.0f;
constexpr float kFriction = 0.992f;
constexpr float kStopSpeed = 0.18f;
constexpr float kGoalRadius = 0.55f;
} // namespace

void Player::Initialize(const Stage& stage) { Reset(stage); }

void Player::Update(const Stage& stage) {
	// 移動中以外は物理更新を行わない
	if (state_ != State::Moving) {
		return;
	}

	// 反射判定用に移動前のグリッド座標を保持
	const Stage::GridPosition previousGrid = stage.WorldToGrid(position_);

	// 速度で移動し、摩擦で徐々に減速する
	MyMath::IntegrateXZ(position_, velocity_, kDeltaTime);
	MyMath::ApplyFrictionXZ(velocity_, kFriction);

	// 移動後のグリッドで壁またはステージ外との反射を確認
	const Stage::GridPosition currentGrid = stage.WorldToGrid(position_);
	ReflectByWallOrBounds(stage, previousGrid, currentGrid);

	// ギミックに接触した場合、同じマスで連続反射しないように一度だけ反射する
	const Stage::GimmickType gimmick = stage.GetGimmick(currentGrid);
	if (gimmick != Stage::GimmickType::None && (lastGimmickGrid_.x != currentGrid.x || lastGimmickGrid_.z != currentGrid.z)) {
		if (gimmick == Stage::GimmickType::ReflectSlash) {
			velocity_ = MyMath::ReflectSlashXZ(velocity_);
		} else if (gimmick == Stage::GimmickType::ReflectBackSlash) {
			velocity_ = MyMath::ReflectBackSlashXZ(velocity_);
		}
		lastGimmickGrid_ = currentGrid;
	} else if (gimmick == Stage::GimmickType::None) {
		lastGimmickGrid_ = {-1, -1};
	}

	// ゴール半径内に入ったらクリアとして停止する
	const Vector3 goalPosition = stage.GridToWorld(stage.GetGoalGrid());
	const Vector3 toGoal = MyMath::Subtract(position_, goalPosition);
	if (MyMath::Length(toGoal) < kGoalRadius) {
		position_ = goalPosition;
		velocity_ = {};
		isClear_ = true;
		state_ = State::Stopped;
		return;
	}

	// 十分に遅くなったら停止し、ゴール未到達なら失敗にする
	if (MyMath::Length(velocity_) < kStopSpeed) {
		velocity_ = {};
		state_ = State::Stopped;
		isFailed_ = !stage.IsGoal(stage.WorldToGrid(position_));
	}
}

void Player::Reset(const Stage& stage) {
	// ステージ設定の開始グリッドへ戻す
	aimGrid_ = stage.GetPlayerStartGrid();
	position_ = stage.GridToWorld(aimGrid_);
	position_.y = 0.65f;

	// 発射前の初期状態へ戻す
	velocity_ = {};
	lastGimmickGrid_ = {-1, -1};
	state_ = State::Aiming;
	isClear_ = false;
	isFailed_ = false;
}

void Player::MoveAimLeft(const Stage& stage) {
	// 発射前のみ位置調整できる
	if (state_ != State::Aiming) {
		return;
	}

	// ステージごとの移動範囲内で左へ移動する
	aimGrid_.x = std::max(stage.GetPlayerMinX(), aimGrid_.x - 1);
	position_ = stage.GridToWorld(aimGrid_);
	position_.y = 0.65f;
}

void Player::MoveAimRight(const Stage& stage) {
	// 発射前のみ位置調整できる
	if (state_ != State::Aiming) {
		return;
	}

	// ステージごとの移動範囲内で右へ移動する
	aimGrid_.x = std::min(stage.GetPlayerMaxX(), aimGrid_.x + 1);
	position_ = stage.GridToWorld(aimGrid_);
	position_.y = 0.65f;
}

void Player::Fire() {
	Fire({0.0f, 0.0f, kMoveSpeed});
}

void Player::Fire(const Vector3& initialVelocity) {
	// 発射済みの場合は再発射しない
	if (state_ != State::Aiming) {
		return;
	}

	velocity_ = initialVelocity;
	velocity_.y = 0.0f;
	state_ = State::Moving;
}

void Player::ReflectByWallOrBounds(const Stage& stage, const Stage::GridPosition& previousGrid, const Stage::GridPosition& currentGrid) {
	// グリッド内かつ壁でなければ反射不要
	if (stage.IsInsideGrid(currentGrid) && !stage.IsWall(currentGrid)) {
		return;
	}

	// めり込みを避けるため、反射前の安全な座標へ戻す
	position_ = stage.GridToWorld(previousGrid);
	position_.y = 0.65f;

	// X/Z のどちらへ進んで衝突したかに応じて速度を反転する
	MyMath::ReflectGridBounceXZ(velocity_, currentGrid.x != previousGrid.x, currentGrid.z != previousGrid.z);
}
