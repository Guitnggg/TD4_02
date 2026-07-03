#include "DragInput.h"

#include "../Math/MathUtility.h"

#include <3d/PrimitiveDrawer.h>
#include <base/WinApp.h>
#include <base/TextureManager.h>

#include <algorithm>
#include <cmath>

using KamataEngine::Vector3;
using KamataEngine::Vector4;

namespace {
constexpr float kPlayerPickRadius = 0.85f;
constexpr float kMaxDragDistance = 4.0f;
constexpr float kMinLaunchDistance = 0.12f;
constexpr float kMinLaunchSpeed = 2.5f;
constexpr float kMaxLaunchSpeed = 10.0f;
constexpr float kGuideY = 0.08f;
constexpr float kArrowBaseSize = 70.0f;
constexpr float kArrowPowerSize = 70.0f;
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
	arrowTextureHandle_ = KamataEngine::TextureManager::Load("Arrow.png");
	arrowSprite_.reset(KamataEngine::Sprite::Create(arrowTextureHandle_, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 0.9f}, {0.5f, 0.5f}));
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

void DragInput::Update(KamataEngine::Input* input, const KamataEngine::Camera& camera, const Vector3& playerPosition, bool canStart) {
	hasLaunchVelocity_ = false;

	const Vector3 mouseWorld = MouseToWorldOnPlane(camera, playerPosition.y);

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

void DragInput::Draw(const KamataEngine::Camera& camera) {
	if (!isDragging_) {
		return;
	}

	KamataEngine::PrimitiveDrawer* primitiveDrawer = KamataEngine::PrimitiveDrawer::GetInstance();
	primitiveDrawer->SetCamera(&camera);

	const Vector3 start = WithGuideY(dragStartWorld_);
	const Vector3 current = WithGuideY(dragCurrentWorld_);
	const Vector3 launchDirection = NormalizeXZ(MyMath::Subtract(dragStartWorld_, dragCurrentWorld_));
	const float previewSpeed = MyMath::Length(launchVelocity_);
	const Vector3 arrowEnd = WithGuideY(MyMath::Add(dragStartWorld_, MyMath::Multiply(launchDirection, 1.2f + powerRate_ * 1.6f)));

	primitiveDrawer->DrawLine3d(start, current, {0.95f, 0.30f, 0.20f, 1.0f});

	Vector3 previewPosition = dragStartWorld_;
	Vector3 previewVelocity = MyMath::Multiply(launchDirection, previewSpeed);
	for (int i = 0; i < kTrajectorySegmentCount; ++i) {
		Vector3 nextPosition = previewPosition;
		MyMath::IntegrateXZ(nextPosition, previewVelocity, kTrajectoryStepTime);
		primitiveDrawer->DrawLine3d(WithGuideY(previewPosition), WithGuideY(nextPosition), {0.30f, 1.0f, 0.45f, 0.85f});
		previewPosition = nextPosition;
		MyMath::ApplyFrictionXZ(previewVelocity, std::pow(kPreviewFrictionPerFrame, 60.0f * kTrajectoryStepTime));
	}

	const Vector3 gaugeStart = WithGuideY(MyMath::Add(dragStartWorld_, {-2.0f, 0.0f, -1.4f}));
	const Vector3 gaugeEnd = MyMath::Add(gaugeStart, {4.0f, 0.0f, 0.0f});
	const Vector3 gaugeValueEnd = MyMath::Add(gaugeStart, {4.0f * powerRate_, 0.0f, 0.0f});
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

Vector3 DragInput::MouseToWorldOnPlane(const KamataEngine::Camera& camera, float planeY) const {
	KamataEngine::Input* input = KamataEngine::Input::GetInstance();
	const KamataEngine::Vector2& mousePosition = input->GetMousePosition();
	const float ndcX = (mousePosition.x / static_cast<float>(KamataEngine::WinApp::kWindowWidth)) * 2.0f - 1.0f;
	const float ndcY = -((mousePosition.y / static_cast<float>(KamataEngine::WinApp::kWindowHeight)) * 2.0f - 1.0f);

	const KamataEngine::Matrix4x4 viewProjection = MyMath::Multiply(camera.matView, camera.matProjection);
	const KamataEngine::Matrix4x4 inverseViewProjection = MyMath::Inverse(viewProjection);
	const Vector3 nearPoint = MyMath::Transform({ndcX, ndcY, 0.0f}, inverseViewProjection);
	const Vector3 farPoint = MyMath::Transform({ndcX, ndcY, 1.0f}, inverseViewProjection);
	const Vector3 ray = MyMath::Subtract(farPoint, nearPoint);

	if (std::abs(ray.y) <= 0.0001f) {
		return nearPoint;
	}

	const float t = (planeY - nearPoint.y) / ray.y;
	return MyMath::Add(nearPoint, MyMath::Multiply(ray, t));
}

bool DragInput::IsPressingLeft(KamataEngine::Input* input) const {
	return input != nullptr && input->IsPressMouse(0);
}

bool DragInput::IsTriggerLeft(KamataEngine::Input* input) const {
	return input != nullptr && input->IsTriggerMouse(0);
}

bool DragInput::IsReleaseLeft(KamataEngine::Input* input) const {
	return wasPressingLeft_ && !IsPressingLeft(input);
}

void DragInput::UpdateDragVector(const Vector3& playerPosition, const Vector3& currentWorld) {
	dragStartWorld_ = playerPosition;

	Vector3 drag = MyMath::Subtract(currentWorld, dragStartWorld_);
	drag.y = 0.0f;
	const float dragDistance = LengthXZ(drag);
	if (dragDistance > kMaxDragDistance) {
		drag = MyMath::Multiply(NormalizeXZ(drag), kMaxDragDistance);
	}

	dragCurrentWorld_ = MyMath::Add(dragStartWorld_, drag);
	const float clampedDistance = LengthXZ(drag);
	powerRate_ = MyMath::Clamp(clampedDistance / kMaxDragDistance, 0.0f, 1.0f);

	const Vector3 direction = NormalizeXZ(MyMath::Subtract(dragStartWorld_, dragCurrentWorld_));
	const float speed = MyMath::CalcLaunchSpeed(clampedDistance, kMaxDragDistance, kMinLaunchSpeed, kMaxLaunchSpeed);
	launchVelocity_ = MyMath::Multiply(direction, speed);
}

KamataEngine::Vector2 DragInput::WorldToScreen(const Vector3& worldPosition, const KamataEngine::Camera& camera) const {
	const KamataEngine::Matrix4x4 viewProjection = MyMath::Multiply(camera.matView, camera.matProjection);
	const Vector3 ndc = MyMath::Transform(worldPosition, viewProjection);
	return {
		(ndc.x + 1.0f) * 0.5f * static_cast<float>(KamataEngine::WinApp::kWindowWidth),
		(1.0f - ndc.y) * 0.5f * static_cast<float>(KamataEngine::WinApp::kWindowHeight),
	};
}

void DragInput::DrawArrowSprite(const Vector3& from, const Vector3& to, const KamataEngine::Camera& camera) {
	if (!arrowSprite_) {
		return;
	}

	const KamataEngine::Vector2 fromScreen = WorldToScreen(from, camera);
	const KamataEngine::Vector2 toScreen = WorldToScreen(to, camera);
	const KamataEngine::Vector2 direction = {toScreen.x - fromScreen.x, toScreen.y - fromScreen.y};
	const float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
	if (length <= 0.001f) {
		return;
	}

	const float size = kArrowBaseSize + kArrowPowerSize * powerRate_;
	const KamataEngine::Vector2 center = {
		fromScreen.x + direction.x * 0.65f,
		fromScreen.y + direction.y * 0.65f,
	};
	const float rotation = std::atan2(direction.y, direction.x) + 1.57079632679f;

	KamataEngine::Sprite::PreDraw();
	arrowSprite_->SetPosition(center);
	arrowSprite_->SetSize({size, size});
	arrowSprite_->SetRotation(rotation);
	arrowSprite_->SetColor({1.0f, 1.0f, 1.0f, 0.9f});
	arrowSprite_->Draw();
	KamataEngine::Sprite::PostDraw();
}
