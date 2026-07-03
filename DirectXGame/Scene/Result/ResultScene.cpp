#include "ResultScene.h"

#include "../Title/TitleScene.h"

#include <KamataEngine.h>
#include <dinput.h>
#ifdef USE_IMGUI
#include <imgui.h>
#endif
#include <memory>

void ResultScene::Initialize() { isEnd_ = false; }

void ResultScene::Update() {
	if (KamataEngine::Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		isEnd_ = true;
	}
}

void ResultScene::Draw() {
#ifdef USE_IMGUI
	ImGui::SetNextWindowPos(ImVec2(520.0f, 280.0f), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(240.0f, 120.0f), ImGuiCond_FirstUseEver);
	ImGui::Begin("ResultScene");
	ImGui::Text("Result Scene");
	ImGui::Separator();
	ImGui::Text("Press SPACE");
	ImGui::End();
#endif
}

bool ResultScene::IsEnd() const { return isEnd_; }

std::unique_ptr<IScene> ResultScene::NextScene() const {
	return std::make_unique<TitleScene>();
}

SceneName ResultScene::GetSceneName() const { return SceneName::Result; }
