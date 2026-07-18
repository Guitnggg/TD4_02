#include "DragInput.h"

#include "../Core/Math/MathUtility.h"

#include <3d/PrimitiveDrawer.h>
#include <base/WinApp.h>
#include <base/TextureManager.h>

#include <cmath>

using namespace KamataEngine;

namespace {
// ドラッグ入力の調整値
constexpr float kPlayerPickRadius = 0.425f;
constexpr float kMaxDragDistance = 2.0f;
constexpr float kMinLaunchDistance = 0.06f;
constexpr float kMinLaunchSpeed = 1.25f;
constexpr float kMaxLaunchSpeed = 5.0f;

// ドラッグ中の表示調整値
constexpr float kGuideY = 0.04f;
constexpr float kArrowBaseSize = 70.0f;
constexpr float kArrowPowerSize = 70.0f;

// 軌道予測の調整値
constexpr float kTrajectoryStepTime = 1.0f / 12.0f;
constexpr int kTrajectorySegmentCount = 18;
constexpr float kPreviewFrictionPerFrame = 0.992f;

float LengthXZ(const Vector3& v) {
	return std::sqrt(v.x * v.x + v.z * v.z);
}

Vector3 NormalizeXZ(const Vector3& v) {
	const float length = LengthXZ(v);
	if (length <= 0.0f) {
		return {};
	}
	return {v.x / length, 0.0f, v.z / length};
}

Vector3 WithGuideY(Vector3 v) {
	v.y = kGuideY;
	return v;
}
} // namespace

void DragInput::Initialize() {
	arrowTextureHandle_ = TextureManager::Load("UI/Arrow.png");
	arrowSprite_.reset(Sprite::Create(arrowTextureHandle_, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 0.9f}, {0.5f, 0.5f}));
}

void DragInput::Reset() {
	isDragging_ = false;
	wasPressingLeft_ = false;
	hasLaunchVelocity_ = false;
	dragStartWorld_ = {};
	dragCurrentWorld_ = {};
	launchVelocity_ = {};
	powerRate_ = 0.0f;
}

void DragInput::Update(Input* input, const Camera& camera, const Vector3& playerPosition, bool canStart) {
	hasLaunchVelocity_ = false;

	// マウス位置をプレイヤーと同じ高さのワールド座標に変換する。
	const Vector3 mouseWorld = MouseToWorldOnPlane(camera, playerPosition.y);

	// 発射前、かつプレイヤー付近を左クリックした時だけドラッグを開始する。
	if (IsTriggerLeft(input) && canStart) {
		const Vector3 toMouse = MyMath::Subtract(mouseWorld, playerPosition);
		if (LengthXZ(toMouse) <= kPlayerPickRadius) {
			isDragging_ = true;
			dragStartWorld_ = playerPosition;
			dragCurrentWorld_ = playerPosition;
			powerRate_ = 0.0f;
		}
	}

	if (isDragging_) {
		UpdateDragVector(playerPosition, mouseWorld);

		// 左クリックを離した瞬間に、現在のドラッグ量を発射速度として確定する。
		if (IsReleaseLeft(input)) {
			isDragging_ = false;
			const float dragDistance = LengthXZ(MyMath::Subtract(dragStartWorld_, dragCurrentWorld_));
			if (dragDistance >= kMinLaunchDistance) {
				hasLaunchVelocity_ = true;
			} else {
				launchVelocity_ = {};
				powerRate_ = 0.0f;
			}
		}
	}

	wasPressingLeft_ = IsPressingLeft(input);
}

void DragInput::Draw(const Camera& camera) {
	if (!isDragging_) {
		return;
	}

	PrimitiveDrawer* primitiveDrawer = PrimitiveDrawer::GetInstance();
	primitiveDrawer->SetCamera(&camera);

	const Vector3 start = WithGuideY(dragStartWorld_);
	const Vector3 current = WithGuideY(dragCurrentWorld_);
	const Vector3 launchDirection = NormalizeXZ(MyMath::Subtract(dragStartWorld_, dragCurrentWorld_));
	const float previewSpeed = MyMath::Length(launchVelocity_);
	const Vector3 arrowEnd = WithGuideY(MyMath::Add(dragStartWorld_, MyMath::Multiply(launchDirection, 0.6f + powerRate_ * 0.8f)));

	// ドラッグした方向を示すライン。
	primitiveDrawer->DrawLine3d(start, current, {0.95f, 0.30f, 0.20f, 1.0f});

	// 現在の発射速度から簡易的な軌道予測を描画する。
	Vector3 previewPosition = dragStartWorld_;
	Vector3 previewVelocity = MyMath::Multiply(launchDirection, previewSpeed);
	for (int i = 0; i < kTrajectorySegmentCount; ++i) {
		Vector3 nextPosition = previewPosition;
		MyMath::IntegrateXZ(nextPosition, previewVelocity, kTrajectoryStepTime);
		primitiveDrawer->DrawLine3d(WithGuideY(previewPosition), WithGuideY(nextPosition), {0.30f, 1.0f, 0.45f, 0.85f});
		previewPosition = nextPosition;
		MyMath::ApplyFrictionXZ(previewVelocity, std::pow(kPreviewFrictionPerFrame, 60.0f * kTrajectoryStepTime));
	}

	// ドラッグ距離に応じた発射速度ゲージ。
	const Vector3 gaugeStart = WithGuideY(MyMath::Add(dragStartWorld_, {-1.0f, 0.0f, -0.7f}));
	const Vector3 gaugeEnd = MyMath::Add(gaugeStart, {2.0f, 0.0f, 0.0f});
	const Vector3 gaugeValueEnd = MyMath::Add(gaugeStart, {2.0f * powerRate_, 0.0f, 0.0f});
	primitiveDrawer->DrawLine3d(gaugeStart, gaugeEnd, {0.20f, 0.20f, 0.20f, 1.0f});
	primitiveDrawer->DrawLine3d(gaugeStart, gaugeValueEnd, {1.0f, 0.85f, 0.15f, 1.0f});

	DrawArrowSprite(start, arrowEnd, camera);
}

bool DragInput::ConsumeLaunchVelocity(Vector3& velocity) {
	if (!hasLaunchVelocity_) {
		return false;
	}

	velocity = launchVelocity_;
	hasLaunchVelocity_ = false;
	return true;
}

Vector3 DragInput::MouseToWorldOnPlane(const Camera& camera, float planeY) const {
	Input* input = Input::GetInstance();
	const Vector2& mousePosition = input->GetMousePosition();

	// 画面座標を NDC に変換する。画面座標は下向きが +Y なので反転する。
	const float ndcX = (mousePosition.x / static_cast<float>(WinApp::kWindowWidth)) * 2.0f - 1.0f;
	const float ndcY = -((mousePosition.y / static_cast<float>(WinApp::kWindowHeight)) * 2.0f - 1.0f);

	// near/far の2点をワールド座標に戻し、そのレイと指定高さの平面との交点を求める。
	const Matrix4x4 viewProjection = MyMath::Multiply(camera.matView, camera.matProjection);
	const Matrix4x4 inverseViewProjection = MyMath::Inverse(viewProjection);
	const Vector3 nearPoint = MyMath::Transform({ndcX, ndcY, 0.0f}, inverseViewProjection);
	const Vector3 farPoint = MyMath::Transform({ndcX, ndcY, 1.0f}, inverseViewProjection);
	const Vector3 ray = MyMath::Subtract(farPoint, nearPoint);

	if (std::abs(ray.y) <= 0.0001f) {
		return nearPoint;
	}

	const float t = (planeY - nearPoint.y) / ray.y;
	return MyMath::Add(nearPoint, MyMath::Multiply(ray, t));
}

bool DragInput::IsPressingLeft(Input* input) const {
	return input != nullptr && input->IsPressMouse(0);
}

bool DragInput::IsTriggerLeft(Input* input) const {
	return input != nullptr && input->IsTriggerMouse(0);
}

bool DragInput::IsReleaseLeft(Input* input) const {
	return wasPressingLeft_ && !IsPressingLeft(input);
}

void DragInput::UpdateDragVector(const Vector3& playerPosition, const Vector3& currentWorld) {
	dragStartWorld_ = playerPosition;

	// ドラッグ距離を最大値で制限し、発射速度が大きくなりすぎないようにする。
	Vector3 drag = MyMath::Subtract(currentWorld, dragStartWorld_);
	drag.y = 0.0f;
	const float dragDistance = LengthXZ(drag);
	if (dragDistance > kMaxDragDistance) {
		drag = MyMath::Multiply(NormalizeXZ(drag), kMaxDragDistance);
	}

	dragCurrentWorld_ = MyMath::Add(dragStartWorld_, drag);
	const float clampedDistance = LengthXZ(drag);
	powerRate_ = MyMath::Clamp(clampedDistance / kMaxDragDistance, 0.0f, 1.0f);

	// 仕様通り、ドラッグした方向とは逆向きへ発射する。
	const Vector3 direction = NormalizeXZ(MyMath::Subtract(dragStartWorld_, dragCurrentWorld_));
	const float speed = MyMath::CalcLaunchSpeed(clampedDistance, kMaxDragDistance, kMinLaunchSpeed, kMaxLaunchSpeed);
	launchVelocity_ = MyMath::Multiply(direction, speed);
}

Vector2 DragInput::WorldToScreen(const Vector3& worldPosition, const Camera& camera) const {
	const Matrix4x4 viewProjection = MyMath::Multiply(camera.matView, camera.matProjection);
	const Vector3 ndc = MyMath::Transform(worldPosition, viewProjection);
	return {
		(ndc.x + 1.0f) * 0.5f * static_cast<float>(WinApp::kWindowWidth),
		(1.0f - ndc.y) * 0.5f * static_cast<float>(WinApp::kWindowHeight),
	};
}

void DragInput::DrawArrowSprite(const Vector3& from, const Vector3& to, const Camera& camera) {
	if (!arrowSprite_) {
		return;
	}

	// Arrow.png は 2D スプライトなので、ワールド上の発射方向を画面座標へ変換して描画する。
	const Vector2 fromScreen = WorldToScreen(from, camera);
	const Vector2 toScreen = WorldToScreen(to, camera);
	const Vector2 direction = {toScreen.x - fromScreen.x, toScreen.y - fromScreen.y};
	const float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
	if (length <= 0.001f) {
		return;
	}

	const float size = kArrowBaseSize + kArrowPowerSize * powerRate_;
	const Vector2 center = {
		fromScreen.x + direction.x * 0.65f,
		fromScreen.y + direction.y * 0.65f,
	};

	// Arrow.png は上向き画像なので、画面上の方向へ合うように 90 度補正する。
	const float rotation = std::atan2(direction.y, direction.x) + 1.57079632679f;

	Sprite::PreDraw();
	arrowSprite_->SetPosition(center);
	arrowSprite_->SetSize({size, size});
	arrowSprite_->SetRotation(rotation);
	arrowSprite_->SetColor({1.0f, 1.0f, 1.0f, 0.9f});
	arrowSprite_->Draw();
	Sprite::PostDraw();
}
