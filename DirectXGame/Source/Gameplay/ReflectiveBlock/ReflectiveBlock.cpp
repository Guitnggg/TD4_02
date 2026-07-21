#include "ReflectiveBlock.h"

#include "../../Core/Math/MathUtility.h"
#include <base/WinApp.h>


namespace {
// ドラッグ入力の調整値
constexpr float kPlayerPickRadius = 0.85f;
constexpr float kMaxDragDistance = 4.0f;
constexpr float kMinLaunchDistance = 0.12f;
constexpr float kMinLaunchSpeed = 2.5f;
constexpr float kMaxLaunchSpeed = 10.0f;

// ドラッグ中の表示調整値
constexpr float kGuideY = 0.08f;
constexpr float kArrowBaseSize = 70.0f;
constexpr float kArrowPowerSize = 70.0f;

// 軌道予測の調整値
constexpr float kTrajectoryStepTime = 1.0f / 12.0f;
constexpr int kTrajectorySegmentCount = 18;
constexpr float kPreviewFrictionPerFrame = 0.992f;

float LengthXZ(const KamataEngine::Vector3& v) { return std::sqrt(v.x * v.x + v.z * v.z); }

KamataEngine::Vector3 NormalizeXZ(const KamataEngine::Vector3& v) {
	const float length = LengthXZ(v);
	if (length <= 0.0f) {
		return {};
	}
	return {v.x / length, 0.0f, v.z / length};
}

KamataEngine::Vector3 WithGuideY(KamataEngine::Vector3 v) {
	v.y = kGuideY;
	return v;
}
} // namespace


ReflectiveBlock::~ReflectiveBlock() {}

ReflectiveBlock::ReflectiveBlock() {}

void ReflectiveBlock::initialize() {
	//モデル取り込み
	model_ = KamataEngine::Model::CreateFromOBJ("cube");
	model_->StaticInitialize();
	

	
	input_ = KamataEngine::Input::GetInstance();
	blockPosition_.Initialize();
	blockPosition_.translation_ = {0.0f, 0.0f, 0.0f};
	
	isCatch_ = false;

	
}

void ReflectiveBlock::Update(KamataEngine::Camera&camera) {
	

		if (IsTriggerLeft(input_)) {
			// マウスをワールド座標へ変換
			KamataEngine::Vector3 mouseWorld = MouseToWorldOnPlane(camera, blockPosition_.translation_.y);

			// ブロックとの距離
			KamataEngine::Vector3 diff = MyMath::Subtract(mouseWorld, blockPosition_.translation_);
			

			// 半径1.0以内ならクリックしたとみなす
			if (LengthXZ(diff) <= 1.0f) {
				isCatch_ = true;
			}
		}
	
	
	if (isCatch_) {
		// マウスをワールド座標へ変換
		KamataEngine::Vector3 mouseWorld = MouseToWorldOnPlane(camera, blockPosition_.translation_.y);
		mousePosition_ = mouseWorld;
		blockPosition_.translation_ = mousePosition_;
	}

	if (IsReleaseLeft(input_)) {
		isCatch_ = false;
	}
	wasPressingLeft_ = IsPressingLeft(input_);
	blockPosition_.UpdateMatrix();
}

void ReflectiveBlock::Draw(KamataEngine::Camera& camera) {
	

	model_->Draw(blockPosition_, camera);
}

KamataEngine::Vector3 ReflectiveBlock::MouseToWorldOnPlane(const KamataEngine::Camera& camera, float planeY) const {
	KamataEngine::Input* input = KamataEngine::Input::GetInstance();
	const KamataEngine::Vector2& mousePosition = input->GetMousePosition();

	// 画面座標を NDC に変換する。画面座標は下向きが +Y なので反転する。
	const float ndcX = (mousePosition.x / static_cast<float>(KamataEngine::WinApp::kWindowWidth)) * 2.0f - 1.0f;
	const float ndcY = -((mousePosition.y / static_cast<float>(KamataEngine::WinApp::kWindowHeight)) * 2.0f - 1.0f);

	// near/far の2点をワールド座標に戻し、そのレイと指定高さの平面との交点を求める。
	const KamataEngine::Matrix4x4 viewProjection = MyMath::Multiply(camera.matView, camera.matProjection);
	const KamataEngine::Matrix4x4 inverseViewProjection = MyMath::Inverse(viewProjection);
	const KamataEngine::Vector3 nearPoint = MyMath::Transform({ndcX, ndcY, 0.0f}, inverseViewProjection);
	const KamataEngine::Vector3 farPoint = MyMath::Transform({ndcX, ndcY, 1.0f}, inverseViewProjection);
	const KamataEngine::Vector3 ray = MyMath::Subtract(farPoint, nearPoint);

	if (std::abs(ray.y) <= 0.0001f) {
		return nearPoint;
	}

	const float t = (planeY - nearPoint.y) / ray.y;
	return MyMath::Add(nearPoint, MyMath::Multiply(ray, t));
}


bool ReflectiveBlock::IsPressingLeft(KamataEngine::Input* input) const { return input != nullptr && input->IsPressMouse(0); }

bool ReflectiveBlock::IsTriggerLeft(KamataEngine::Input* input) const { return input != nullptr && input->IsTriggerMouse(0); }

bool ReflectiveBlock::IsReleaseLeft(KamataEngine::Input* input) const { return wasPressingLeft_ && !IsPressingLeft(input); }