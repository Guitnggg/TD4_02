#include "DragInput.h"

#include "../Math/MathUtility.h"

#include <3d/PrimitiveDrawer.h>
#include <base/WinApp.h>

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

void DragInput::Draw(const KamataEngine::Camera& camera) const {
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
	DrawArrow(start, arrowEnd, {0.20f, 0.85f, 1.0f, 1.0f});

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

void DragInput::DrawArrow(const Vector3& from, const Vector3& to, const Vector4& color) const {
	KamataEngine::PrimitiveDrawer* primitiveDrawer = KamataEngine::PrimitiveDrawer::GetInstance();
	primitiveDrawer->DrawLine3d(from, to, color);

	const Vector3 direction = NormalizeXZ(MyMath::Subtract(to, from));
	if (LengthXZ(direction) <= 0.0f) {
		return;
	}

	const Vector3 side = {-direction.z, 0.0f, direction.x};
	const Vector3 back = MyMath::Multiply(direction, -0.35f);
	const Vector3 leftHead = MyMath::Add(to, MyMath::Add(back, MyMath::Multiply(side, 0.18f)));
	const Vector3 rightHead = MyMath::Add(to, MyMath::Add(back, MyMath::Multiply(side, -0.18f)));
	primitiveDrawer->DrawLine3d(to, leftHead, color);
	primitiveDrawer->DrawLine3d(to, rightHead, color);
}
