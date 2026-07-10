#include "GameScene.h"

#include "../Result/ResultScene.h"

#include <KamataEngine.h>
#include <algorithm>
#include <dinput.h>
#ifdef USE_IMGUI
#include <imgui.h>
#endif

using namespace KamataEngine;

namespace {
constexpr float kTopDownCameraHeight = 18.0f;
constexpr float kTopDownCameraPitch = 1.57079632679f;
} // namespace

void GameScene::Initialize() {
	isEnd_ = false;

	stage_.InitializeTutorial();
	player_.Initialize(stage_);
	dragInput_.Initialize();
	dragInput_.Reset();

	camera_.Initialize();
	camera_.translation_ = {0.0f, kTopDownCameraHeight, 0.0f};
	camera_.rotation_ = {kTopDownCameraPitch, 0.0f, 0.0f};
	camera_.UpdateMatrix();

	const std::vector<Stage::GridPosition>& placeableTiles = stage_.GetPlaceableTiles();
	placementCursor_ = placeableTiles.empty() ? Stage::GridPosition{0, 0} : placeableTiles.front();
	selectedGimmickType_ = Stage::GimmickType::ReflectSlash;
	maxGimmickCount_ = 3;

	stageRenderer_.Initialize(stage_, player_.GetPosition());
	stageRenderer_.UpdatePlacementCursor(stage_, placementCursor_, selectedGimmickType_, true);
}

void GameScene::Update() {
	Input* input = Input::GetInstance();

	if (input->TriggerKey(DIK_R)) {
		stage_.InitializeTutorial();
		player_.Reset(stage_);
		dragInput_.Reset();
		const std::vector<Stage::GridPosition>& placeableTiles = stage_.GetPlaceableTiles();
		placementCursor_ = placeableTiles.empty() ? Stage::GridPosition{0, 0} : placeableTiles.front();
		stageRenderer_.Initialize(stage_, player_.GetPosition());
		stageRenderer_.UpdatePlacementCursor(stage_, placementCursor_, selectedGimmickType_, true);
		return;
	}

	// 発射前のみ、プレイヤーの開始位置調整とギミック配置が可能
	if (player_.GetState() == Player::State::Aiming) {
		if (input->TriggerKey(DIK_A)) {
			player_.MoveAimRight(stage_);
		}
		if (input->TriggerKey(DIK_D)) {
			player_.MoveAimLeft(stage_);
		}

		UpdateGimmickPlacement();

		if (input->TriggerKey(DIK_SPACE)) {
			player_.Fire();
		}
	}

	dragInput_.Update(input, camera_, player_.GetPosition(), player_.GetState() == Player::State::Aiming);
	Vector3 dragLaunchVelocity{};
	if (dragInput_.ConsumeLaunchVelocity(dragLaunchVelocity)) {
		player_.Fire(dragLaunchVelocity);
	}

	player_.Update(stage_);
	stageRenderer_.UpdatePlayer(player_.GetPosition());
	stageRenderer_.UpdatePlacementCursor(stage_, placementCursor_, selectedGimmickType_, player_.GetState() == Player::State::Aiming);

	if (player_.IsClear()) {
		isEnd_ = true;
	}
}

void GameScene::Draw() {
	stageRenderer_.Draw(camera_);
	stageRenderer_.DrawGuide(stage_, camera_);
	dragInput_.Draw(camera_);

#ifdef USE_IMGUI
	ImGui::SetNextWindowPos(ImVec2(520.0f, 240.0f), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(360.0f, 260.0f), ImGuiCond_FirstUseEver);
	ImGui::Begin("GameScene");
	ImGui::Text("3D One-Step Puzzle");
	ImGui::Separator();
	ImGui::Text("A/D: adjust launch position");
	ImGui::Text("Arrow: move gimmick cursor");
	ImGui::Text("1/2: select / or \\");
	ImGui::Text("Z: place gimmick");
	ImGui::Text("X: remove gimmick");
	ImGui::Text("C: clear gimmicks");
	ImGui::Text("Drag player or SPACE: launch");
	ImGui::Text("R: reset");
	ImGui::Separator();
	ImGui::Text("Cursor: X %d / Z %d", placementCursor_.x, placementCursor_.z);
	ImGui::Text("Selected: %s", selectedGimmickType_ == Stage::GimmickType::ReflectSlash ? "/" : "\\");
	ImGui::Text("Gimmicks: %d / %d", stage_.GetPlacedGimmickCount(), maxGimmickCount_);
	ImGui::Text("Drag power: %.0f%%", dragInput_.GetPowerRate() * 100.0f);
	ImGui::Text("State: %s", player_.GetState() == Player::State::Aiming ? "Aiming" : player_.GetState() == Player::State::Moving ? "Moving" : "Stopped");
	if (player_.IsFailed()) {
		ImGui::Text("FAILED: press R");
	}
	ImGui::End();
#endif
}

bool GameScene::IsEnd() const { return isEnd_; }

std::unique_ptr<IScene> GameScene::NextScene() const { return std::make_unique<ResultScene>(); }

SceneName GameScene::GetSceneName() const { return SceneName::InGame; }

void GameScene::UpdateGimmickPlacement() {
	Input* input = Input::GetInstance();

	if (input->TriggerKey(DIK_LEFT)) {
		++placementCursor_.x;
	}
	if (input->TriggerKey(DIK_RIGHT)) {
		--placementCursor_.x;
	}
	if (input->TriggerKey(DIK_UP)) {
		--placementCursor_.z;
	}
	if (input->TriggerKey(DIK_DOWN)) {
		++placementCursor_.z;
	}
	ClampPlacementCursor();

	if (input->TriggerKey(DIK_1)) {
		selectedGimmickType_ = Stage::GimmickType::ReflectSlash;
	}
	if (input->TriggerKey(DIK_2)) {
		selectedGimmickType_ = Stage::GimmickType::ReflectBackSlash;
	}

	if (input->TriggerKey(DIK_Z)) {
		const bool alreadyPlaced = stage_.GetGimmick(placementCursor_) != Stage::GimmickType::None;
		if (alreadyPlaced || stage_.GetPlacedGimmickCount() < maxGimmickCount_) {
			if (stage_.PlaceGimmick(placementCursor_, selectedGimmickType_)) {
				stageRenderer_.RebuildGimmicks(stage_);
			}
		}
	}

	if (input->TriggerKey(DIK_X)) {
		if (stage_.RemoveGimmick(placementCursor_)) {
			stageRenderer_.RebuildGimmicks(stage_);
		}
	}

	if (input->TriggerKey(DIK_C)) {
		ClearPlacedGimmicks();
	}
}

void GameScene::ClampPlacementCursor() {
	placementCursor_.x = std::clamp(placementCursor_.x, 0, stage_.GetWidth() - 1);
	placementCursor_.z = std::clamp(placementCursor_.z, 0, stage_.GetHeight() - 1);
}

void GameScene::ClearPlacedGimmicks() {
	stage_.ClearGimmicks();
	stageRenderer_.RebuildGimmicks(stage_);
}
