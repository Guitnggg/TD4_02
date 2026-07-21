#include "Player.h"

#include "../../Core/Math/MathUtility.h"

#include <algorithm>
#include <limits>

using namespace KamataEngine;

namespace {
constexpr float kDeltaTime = 1.0f / 60.0f;
constexpr float kMoveSpeed = 3.5f;
constexpr float kFriction = 0.992f;
constexpr float kStopSpeed = 0.09f;
// 反射板の当たり判定用。
// 以前は「プレイヤーの中心が反射板と同じマスに入ったか」で判定していたため、
// 見た目の斜め板から離れていても反射してしまうことがあった。
// ここでは、プレイヤーの移動線分と反射板の斜め線分の距離で判定する。
constexpr float kGimmickDiagonalHalfRate = 0.48f;
constexpr float kGimmickCollisionRadius = 0.26f;
constexpr float kCollisionEpsilon = 0.0001f;

float Clamp01(float value) {
	return std::clamp(value, 0.0f, 1.0f);
}

bool IsSameGrid(const Stage::GridPosition& a, const Stage::GridPosition& b) {
	return a.x == b.x && a.z == b.z;
}

Vector3 MakeXZ(float x, float z) {
	return {x, 0.0f, z};
}

float CrossXZ(const Vector3& a, const Vector3& b, const Vector3& c) {
	const float abx = b.x - a.x;
	const float abz = b.z - a.z;
	const float acx = c.x - a.x;
	const float acz = c.z - a.z;
	return abx * acz - abz * acx;
}

bool IsRangeOverlap(float aMin, float aMax, float bMin, float bMax) {
	if (aMin > aMax) {
		std::swap(aMin, aMax);
	}
	if (bMin > bMax) {
		std::swap(bMin, bMax);
	}
	return aMin <= bMax + kCollisionEpsilon && bMin <= aMax + kCollisionEpsilon;
}

bool IsSegmentIntersectXZ(const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& d) {
	if (!IsRangeOverlap(a.x, b.x, c.x, d.x) || !IsRangeOverlap(a.z, b.z, c.z, d.z)) {
		return false;
	}

	const float c1 = CrossXZ(a, b, c);
	const float c2 = CrossXZ(a, b, d);
	const float c3 = CrossXZ(c, d, a);
	const float c4 = CrossXZ(c, d, b);

	return c1 * c2 <= kCollisionEpsilon && c3 * c4 <= kCollisionEpsilon;
}

float SegmentPointDistanceSqXZ(const Vector3& point, const Vector3& segmentStart, const Vector3& segmentEnd) {
	const float abx = segmentEnd.x - segmentStart.x;
	const float abz = segmentEnd.z - segmentStart.z;
	const float apx = point.x - segmentStart.x;
	const float apz = point.z - segmentStart.z;
	const float abLengthSq = abx * abx + abz * abz;

	float t = 0.0f;
	if (abLengthSq > kCollisionEpsilon) {
		t = Clamp01((apx * abx + apz * abz) / abLengthSq);
	}

	const float closestX = segmentStart.x + abx * t;
	const float closestZ = segmentStart.z + abz * t;
	const float dx = point.x - closestX;
	const float dz = point.z - closestZ;
	return dx * dx + dz * dz;
}

float SegmentSegmentDistanceSqXZ(const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& d) {
	if (IsSegmentIntersectXZ(a, b, c, d)) {
		return 0.0f;
	}

	float result = SegmentPointDistanceSqXZ(a, c, d);
	result = std::min(result, SegmentPointDistanceSqXZ(b, c, d));
	result = std::min(result, SegmentPointDistanceSqXZ(c, a, b));
	result = std::min(result, SegmentPointDistanceSqXZ(d, a, b));
	return result;
}

void MakeGimmickSegment(const Stage& stage, const Stage::GridPosition& grid, Stage::GimmickType type, Vector3& start, Vector3& end) {
	const Vector3 center = stage.GridToWorld(grid);
	const float halfLength = stage.GetCellSize() * kGimmickDiagonalHalfRate;
	const float invSqrt2 = 0.70710678118f;

	Vector3 direction{};
	if (type == Stage::GimmickType::ReflectBackSlash) {
		direction = MakeXZ(invSqrt2, invSqrt2);
	} else {
		direction = MakeXZ(invSqrt2, -invSqrt2);
	}

	start = {center.x - direction.x * halfLength, 0.0f, center.z - direction.z * halfLength};
	end = {center.x + direction.x * halfLength, 0.0f, center.z + direction.z * halfLength};
}
} // namespace

void Player::Initialize(const Stage& stage) { Reset(stage); }

void Player::Update(const Stage& stage) {
	// 移動中以外は物理更新を行わない
	if (state_ != State::Moving) {
		return;
	}

	// 反射判定用に移動前のグリッド座標を保持
	const Vector3 previousPosition = position_;
	const Stage::GridPosition previousGrid = stage.WorldToGrid(previousPosition);

	// 速度で移動し、摩擦で徐々に減速する
	MyMath::IntegrateXZ(position_, velocity_, kDeltaTime);
	MyMath::ApplyFrictionXZ(velocity_, kFriction);

	// 移動後のグリッドで壁またはステージ外との反射を確認
	Stage::GridPosition currentGrid = stage.WorldToGrid(position_);
	ReflectByWallOrBounds(stage, previousPosition, previousGrid, currentGrid);
	currentGrid = stage.WorldToGrid(position_);

	// ギミックは「同じマスに入ったか」ではなく、見た目の斜め板に近いかで判定する
	ReflectByGimmick(stage, previousPosition);
	AccelerateOnPanel(stage);

	// ゴールタイルに入った瞬間にクリアする
	if (stage.IsGoal(currentGrid)) {
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
	position_.y = 0.325f;

	// 発射前の初期状態へ戻す
	velocity_ = {};
	lastGimmickGrid_ = {-1, -1};
	lastAccelerationPanelGrid_ = {-1, -1};
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
	position_.y = 0.325f;
}

void Player::MoveAimRight(const Stage& stage) {
	// 発射前のみ位置調整できる
	if (state_ != State::Aiming) {
		return;
	}

	// ステージごとの移動範囲内で右へ移動する
	aimGrid_.x = std::min(stage.GetPlayerMaxX(), aimGrid_.x + 1);
	position_ = stage.GridToWorld(aimGrid_);
	position_.y = 0.325f;
}

void Player::Fire() {
	Fire({0.0f, 0.0f, -kMoveSpeed});
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

void Player::ReflectByWallOrBounds(const Stage& stage, const Vector3& previousPosition, const Stage::GridPosition& previousGrid, const Stage::GridPosition& currentGrid) {
	// グリッド内かつ壁でなければ反射不要
	if (stage.IsInsideGrid(currentGrid) && !stage.IsWall(currentGrid)) {
		return;
	}

	// めり込みを避けるため、反射前の安全な座標へ戻す
	position_ = previousPosition;
	position_.y = 0.325f;

	// X/Z のどちらへ進んで衝突したかに応じて速度を反転する
	MyMath::ReflectGridBounceXZ(velocity_, currentGrid.x != previousGrid.x, currentGrid.z != previousGrid.z);
}

bool Player::ReflectByGimmick(const Stage& stage, const Vector3& previousPosition) {
	const Vector3 moveStart = {previousPosition.x, 0.0f, previousPosition.z};
	const Vector3 moveEnd = {position_.x, 0.0f, position_.z};
	const float hitRadiusSq = kGimmickCollisionRadius * kGimmickCollisionRadius;

	Stage::GridPosition hitGrid{-1, -1};
	Stage::GimmickType hitType = Stage::GimmickType::None;
	float nearestDistanceSq = std::numeric_limits<float>::max();

	auto checkGimmickList = [&](const std::vector<Stage::GridPosition>& grids, Stage::GimmickType type) {
		for (const Stage::GridPosition& grid : grids) {
			Vector3 boardStart{};
			Vector3 boardEnd{};
			MakeGimmickSegment(stage, grid, type, boardStart, boardEnd);

			const float distanceSq = SegmentSegmentDistanceSqXZ(moveStart, moveEnd, boardStart, boardEnd);
			if (distanceSq <= hitRadiusSq && distanceSq < nearestDistanceSq) {
				nearestDistanceSq = distanceSq;
				hitGrid = grid;
				hitType = type;
			}
		}
	};

	checkGimmickList(stage.GetReflectSlashTiles(), Stage::GimmickType::ReflectSlash);
	checkGimmickList(stage.GetReflectBackSlashTiles(), Stage::GimmickType::ReflectBackSlash);

	if (hitType == Stage::GimmickType::None) {
		lastGimmickGrid_ = {-1, -1};
		return false;
	}

	// 同じ反射板に触れ続けている間は、1回だけ反射させる
	if (IsSameGrid(lastGimmickGrid_, hitGrid)) {
		return true;
	}

	if (hitType == Stage::GimmickType::ReflectSlash) {
		velocity_ = MyMath::ReflectSlashXZ(velocity_);
	} else if (hitType == Stage::GimmickType::ReflectBackSlash) {
		velocity_ = MyMath::ReflectBackSlashXZ(velocity_);
	}

	lastGimmickGrid_ = hitGrid;
	return true;
}

void Player::AccelerateOnPanel(const Stage& stage) {
	const Stage::GridPosition currentGrid = stage.WorldToGrid(position_);
	const AccelerationPanel* panel = stage.FindAccelerationPanel(currentGrid);
	if (panel == nullptr) {
		lastAccelerationPanelGrid_ = {-1, -1};
		return;
	}

	if (!IsSameGrid(lastAccelerationPanelGrid_, currentGrid)) {
		if (panel->Apply(velocity_)) {
			lastAccelerationPanelGrid_ = currentGrid;
		}
	}
}
