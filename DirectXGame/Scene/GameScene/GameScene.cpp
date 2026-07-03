#include "GameScene.h"

#include "../Result/ResultScene.h"

#include <KamataEngine.h>
#include <dinput.h>
#ifdef USE_IMGUI
#include <imgui.h>
#endif
#include <memory>

void GameScene::Initialize() { isEnd_ = false; }

void GameScene::Update() {
	if (KamataEngine::Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		isEnd_ = true;
	}
}

void GameScene::Draw() {

#ifdef USE_IMGUI
	ImGui::SetNextWindowPos(ImVec2(520.0f, 280.0f), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(240.0f, 120.0f), ImGuiCond_FirstUseEver);
	ImGui::Begin("GameScene");
	ImGui::Text("Game Scene");
	ImGui::Separator();
	ImGui::Text("Press SPACE");
	ImGui::End();
#endif

}

bool GameScene::IsEnd() const { return isEnd_; }

std::unique_ptr<IScene> GameScene::NextScene() const {
	return std::make_unique<ResultScene>();
}

SceneName GameScene::GetSceneName() const { return SceneName::InGame; }
