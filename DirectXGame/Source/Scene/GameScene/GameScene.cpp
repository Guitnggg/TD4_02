#include "GameScene.h"
#include "../DifficultySelect/DifficultySelectScene.h"
#include "../Result/ResultScene.h"
#include "../Title/TitleScene.h"

#include <KamataEngine.h>
#include <dinput.h>
#ifdef USE_IMGUI
#include <imgui.h>
#endif

#include <memory>
#include <utility>

using namespace KamataEngine;

namespace {
    constexpr float kTopDownCameraHeight = 18.0f;
    constexpr float kTopDownCameraPitch = 1.57079632679f;
} // namespace

GameScene::GameScene(std::string stageFilePath) : stageFilePath_(std::move(stageFilePath)) {}

void GameScene::Initialize() {
    isEnd_ = false;
    returnTitle_ = false;
    returnStageSelect_ = false;

    if (!stage_.LoadFromCsv(stageFilePath_)) {
        stage_.LoadFromCsv("Resources\\Stages\\Eazy\\Eazy_01.csv");
    }

    player_.Initialize(stage_);
    dragInput_.Initialize();
    ui_.Initialize();
    dragInput_.Reset();

    camera_.Initialize();
    camera_.translation_ = { 0.0f, kTopDownCameraHeight, 0.0f };
    camera_.rotation_ = { kTopDownCameraPitch, 0.0f, 0.0f };
    camera_.UpdateMatrix();

    stageRenderer_.Initialize(stage_, player_.GetPosition());
}

void GameScene::Update() {
    Input* input = Input::GetInstance();

    if (input->TriggerKey(DIK_D)) {
        player_.MoveAimLeft(stage_);
    }
    if (input->TriggerKey(DIK_A)) {
        player_.MoveAimRight(stage_);
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

    ui_.Update();

    player_.Update(stage_);
    stageRenderer_.UpdatePlayer(player_.GetPosition());

    if (ui_.IsStageselect()) {
        returnStageSelect_ = true;
        isEnd_ = true;
        return;
    }

    if (ui_.IsProgress()) {
        returnTitle_ = true;
        isEnd_ = true;
        return;
    }

    if (player_.IsClear()) {
        returnTitle_ = false;
        isEnd_ = true;
    }
}

void GameScene::Draw() {
    stageRenderer_.Draw(camera_);
    stageRenderer_.DrawGuide(stage_, camera_);
    dragInput_.Draw(camera_);
    ui_.Draw();

#ifdef USE_IMGUI
    ImGui::SetNextWindowPos(ImVec2(520.0f, 280.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(280.0f, 140.0f), ImGuiCond_FirstUseEver);
    ImGui::Begin("GameScene");
    ImGui::Text("3D One-Step Puzzle");
    ImGui::Text("Stage: %s", stageFilePath_.c_str());
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
    if (returnTitle_) {
        return std::make_unique<TitleScene>();
    }
    if (returnStageSelect_) {
        return std::make_unique<DifficultySelectScene>();
    }

    const int gimmickCount = static_cast<int>(stage_.GetReflectSlashTiles().size() + stage_.GetReflectBackSlashTiles().size());
    return std::make_unique<ResultScene>(gimmickCount);
}

SceneName GameScene::GetSceneName() const { return SceneName::InGame; }
