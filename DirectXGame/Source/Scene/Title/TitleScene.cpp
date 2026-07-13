#include "TitleScene.h"

#include "../DifficultySelect/DifficultySelectScene.h"

#include <KamataEngine.h>
#include <dinput.h>
#ifdef USE_IMGUI
#include <imgui.h>
#endif
#include <memory>

using namespace KamataEngine;

void TitleScene::Initialize() { isEnd_ = false; }

void TitleScene::Update() {
    if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
        isEnd_ = true;
    }
}

void TitleScene::Draw() {
#ifdef USE_IMGUI
    ImGui::SetNextWindowPos(ImVec2(520.0f, 280.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(240.0f, 120.0f), ImGuiCond_FirstUseEver);
    ImGui::Begin("TitleScene");
    ImGui::Text("Title Scene");
    ImGui::Separator();
    ImGui::Text("Press SPACE");
    ImGui::End();
#endif
}

bool TitleScene::IsEnd() const { return isEnd_; }

std::unique_ptr<IScene> TitleScene::NextScene() const {
    return std::make_unique<DifficultySelectScene>();
}

SceneName TitleScene::GetSceneName() const { return SceneName::Title; }
