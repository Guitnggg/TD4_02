#include "GameScene.h"

#include "../Result/ResultScene.h"

#include <KamataEngine.h>
#include <dinput.h>
#ifdef USE_IMGUI
#include <imgui.h>
#endif
#include <numbers>
#include <memory>

namespace {
constexpr float kFloorHeight = -0.12f;
constexpr float kTileHalfHeight = 0.06f;
constexpr float kObjectHeight = 0.75f;
constexpr float kPlayerScale = 0.45f;
constexpr float kGoalScale = 0.55f;
constexpr float kGimmickScale = 0.42f;
constexpr float kWallScale = 0.82f;
} // namespace

void GameScene::Initialize() {
	isEnd_ = false;

	stage_.InitializeTutorial();
	player_.Initialize(stage_);
	dragInput_.Reset();

	camera_.Initialize();
	camera_.translation_ = {0.0f, 12.5f, -13.0f};
	camera_.rotation_ = {0.72f, 0.0f, 0.0f};
	camera_.UpdateMatrix();

	cubeModel_.reset(KamataEngine::Model::CreateFromOBJ("cube"));
	BuildStageObjects();
	UpdatePlayerObject();
}

void GameScene::Update() {
	KamataEngine::Input* input = KamataEngine::Input::GetInstance();

	if (input->TriggerKey(DIK_A)) {
		player_.MoveAimLeft(stage_);
	}
	if (input->TriggerKey(DIK_D)) {
		player_.MoveAimRight(stage_);
	}
	if (input->TriggerKey(DIK_SPACE)) {
		player_.Fire();
	}
	if (input->TriggerKey(DIK_R)) {
		player_.Reset(stage_);
		dragInput_.Reset();
	}

	dragInput_.Update(input, camera_, player_.GetPosition(), player_.GetState() == Player::State::Aiming);
	KamataEngine::Vector3 dragLaunchVelocity{};
	if (dragInput_.ConsumeLaunchVelocity(dragLaunchVelocity)) {
		player_.Fire(dragLaunchVelocity);
	}

	player_.Update(stage_);
	UpdatePlayerObject();

	if (player_.IsClear()) {
		isEnd_ = true;
	}
}

void GameScene::Draw() {
	KamataEngine::Object3d::PreDraw(&camera_);

	for (const std::unique_ptr<KamataEngine::Object3d>& object : floorObjects_) {
		object->Draw(camera_);
	}
	for (const std::unique_ptr<KamataEngine::Object3d>& object : placeableObjects_) {
		object->Draw(camera_);
	}
	for (const std::unique_ptr<KamataEngine::Object3d>& object : wallObjects_) {
		object->Draw(camera_);
	}
	for (const std::unique_ptr<KamataEngine::Object3d>& object : gimmickObjects_) {
		object->Draw(camera_);
	}
	goalObject_->Draw(camera_);
	playerObject_->Draw(camera_);

	KamataEngine::Object3d::PostDraw();

	DrawStageGuide();
	dragInput_.Draw(camera_);

#ifdef USE_IMGUI
	ImGui::SetNextWindowPos(ImVec2(520.0f, 280.0f), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(280.0f, 140.0f), ImGuiCond_FirstUseEver);
	ImGui::Begin("GameScene");
	ImGui::Text("3D One-Step Puzzle");
	ImGui::Separator();
	ImGui::Text("A/D: adjust launch position");
	ImGui::Text("Drag player: launch");
	ImGui::Text("SPACE: launch forward");
	ImGui::Text("R: reset");
	ImGui::Text("Drag power: %.0f%%", dragInput_.GetPowerRate() * 100.0f);
	ImGui::Text("State: %s", player_.GetState() == Player::State::Aiming ? "Aiming" : player_.GetState() == Player::State::Moving ? "Moving" : "Stopped");
	if (player_.IsFailed()) {
		ImGui::Text("FAILED: press R");
	}
	ImGui::End();
#endif

}

bool GameScene::IsEnd() const { return isEnd_; }

std::unique_ptr<IScene> GameScene::NextScene() const {
	return std::make_unique<ResultScene>();
}

SceneName GameScene::GetSceneName() const { return SceneName::InGame; }

void GameScene::BuildStageObjects() {
	floorObjects_.clear();
	wallObjects_.clear();
	placeableObjects_.clear();
	gimmickObjects_.clear();

	const float cellSize = stage_.GetCellSize();
	const KamataEngine::Vector3 floorScale = {cellSize * 0.47f, kTileHalfHeight, cellSize * 0.47f};

	for (int z = 0; z < stage_.GetHeight(); ++z) {
		for (int x = 0; x < stage_.GetWidth(); ++x) {
			KamataEngine::Vector3 position = stage_.GridToWorld({x, z});
			position.y = kFloorHeight;
			floorObjects_.push_back(CreateCube(position, floorScale));
		}
	}

	for (const Stage::GridPosition& grid : stage_.GetPlaceableTiles()) {
		KamataEngine::Vector3 position = stage_.GridToWorld(grid);
		position.y = 0.02f;
		placeableObjects_.push_back(CreateCube(position, {cellSize * 0.36f, 0.08f, cellSize * 0.36f}));
	}

	for (const Stage::GridPosition& grid : stage_.GetWalls()) {
		KamataEngine::Vector3 position = stage_.GridToWorld(grid);
		position.y = kObjectHeight;
		wallObjects_.push_back(CreateCube(position, {kWallScale, kObjectHeight, kWallScale}));
	}

	for (const Stage::GridPosition& grid : stage_.GetReflectSlashTiles()) {
		KamataEngine::Vector3 position = stage_.GridToWorld(grid);
		position.y = 0.35f;
		std::unique_ptr<KamataEngine::Object3d> object = CreateCube(position, {kGimmickScale, 0.14f, cellSize * 0.58f});
		object->SetRotation({0.0f, -std::numbers::pi_v<float> * 0.25f, 0.0f});
		object->Update();
		gimmickObjects_.push_back(std::move(object));
	}

	for (const Stage::GridPosition& grid : stage_.GetReflectBackSlashTiles()) {
		KamataEngine::Vector3 position = stage_.GridToWorld(grid);
		position.y = 0.35f;
		std::unique_ptr<KamataEngine::Object3d> object = CreateCube(position, {kGimmickScale, 0.14f, cellSize * 0.58f});
		object->SetRotation({0.0f, std::numbers::pi_v<float> * 0.25f, 0.0f});
		object->Update();
		gimmickObjects_.push_back(std::move(object));
	}

	KamataEngine::Vector3 goalPosition = stage_.GridToWorld(stage_.GetGoalGrid());
	goalPosition.y = 0.45f;
	goalObject_ = CreateCube(goalPosition, {kGoalScale, kGoalScale, kGoalScale});

	playerObject_ = CreateCube(player_.GetPosition(), {kPlayerScale, kPlayerScale, kPlayerScale});
}

std::unique_ptr<KamataEngine::Object3d> GameScene::CreateCube(const KamataEngine::Vector3& translation, const KamataEngine::Vector3& scale) {
	std::unique_ptr<KamataEngine::Object3d> object = std::make_unique<KamataEngine::Object3d>();
	object->Initialize(cubeModel_.get());
	object->SetTranslation(translation);
	object->SetScale(scale);
	object->Update();
	return object;
}

void GameScene::UpdatePlayerObject() {
	if (!playerObject_) {
		return;
	}
	playerObject_->SetTranslation(player_.GetPosition());
	playerObject_->Update();
}

void GameScene::DrawStageGuide() {
	KamataEngine::PrimitiveDrawer* primitiveDrawer = KamataEngine::PrimitiveDrawer::GetInstance();
	primitiveDrawer->SetCamera(&camera_);

	const float halfCell = stage_.GetCellSize() * 0.5f;
	const float y = 0.03f;
	const KamataEngine::Vector4 color = {0.25f, 0.35f, 0.50f, 1.0f};

	for (int z = 0; z < stage_.GetHeight(); ++z) {
		for (int x = 0; x < stage_.GetWidth(); ++x) {
			KamataEngine::Vector3 center = stage_.GridToWorld({x, z});
			const float left = center.x - halfCell;
			const float right = center.x + halfCell;
			const float front = center.z - halfCell;
			const float back = center.z + halfCell;
			primitiveDrawer->DrawLine3d({left, y, front}, {right, y, front}, color);
			primitiveDrawer->DrawLine3d({right, y, front}, {right, y, back}, color);
			primitiveDrawer->DrawLine3d({right, y, back}, {left, y, back}, color);
			primitiveDrawer->DrawLine3d({left, y, back}, {left, y, front}, color);
		}
	}
}
