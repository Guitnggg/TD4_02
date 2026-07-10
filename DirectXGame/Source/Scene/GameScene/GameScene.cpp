#include "GameScene.h"

#include "../Result/ResultScene.h"

#include <KamataEngine.h>
#include <dinput.h>
#ifdef USE_IMGUI
#include <imgui.h>
#endif

#include <memory>

using namespace KamataEngine;

void GameScene::Initialize() {
	isEnd_ = false;

	stage_.InitializeTutorial();
	player_.Initialize(stage_);
	dragInput_.Initialize();
	dragInput_.Reset();

	camera_.Initialize();
	camera_.translation_ = {0.0f, 12.5f, -13.0f};
	camera_.rotation_ = {0.72f, 0.0f, 0.0f};
	camera_.UpdateMatrix();

	stageRenderer_.Initialize(stage_, player_.GetPosition());
}

void GameScene::Update() {
	Input* input = Input::GetInstance();

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
	Vector3 dragLaunchVelocity{};
	if (dragInput_.ConsumeLaunchVelocity(dragLaunchVelocity)) {
		player_.Fire(dragLaunchVelocity);
	}

	player_.Update(stage_);
	stageRenderer_.UpdatePlayer(player_.GetPosition());

	if (player_.IsClear()) {
		isEnd_ = true;
	}
}

void GameScene::Draw() {
	stageRenderer_.Draw(camera_);
	stageRenderer_.DrawGuide(stage_, camera_);
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
