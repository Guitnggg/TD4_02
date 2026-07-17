#include "ResultScene.h"

#include "../Title/TitleScene.h"

#include <KamataEngine.h>
#include <dinput.h>
#ifdef USE_IMGUI
#include <imgui.h>
#endif
#include <memory>

using namespace KamataEngine;

ResultScene::ResultScene(int placedGimmickCount) : isEnd_(false), placedGimmickCount_(placedGimmickCount) {
	if (placedGimmickCount_ == 0) {
		evaluationLabel_ = "Stars: 3 (0 gimmicks)";
	} else if (placedGimmickCount_ >= 1 && placedGimmickCount_ <= 3) {
		evaluationLabel_ = "Stars: 2 (1-3 gimmicks)";
	} else { 
		evaluationLabel_ = "Stars: 1 (4+ gimmicks)";
	}
}

void ResultScene::Initialize() { isEnd_ = false; }

void ResultScene::Update() {
	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		isEnd_ = true;
	}
}

void ResultScene::Draw() {
#ifdef USE_IMGUI
	ImGui::SetNextWindowPos(ImVec2(520.0f, 280.0f), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(240.0f, 120.0f), ImGuiCond_FirstUseEver);//
	ImGui::Begin("ResultScene");
	ImGui::Text("CLEAR!");
	ImGui::Separator();

	ImGui::Text("%s", evaluationLabel_.c_str());

	ImGui::Separator();
	ImGui::Text("Press SPACE to title");
	ImGui::End();
#endif
}

bool ResultScene::IsEnd() const { return isEnd_; }

std::unique_ptr<IScene> ResultScene::NextScene() const {
	return std::make_unique<TitleScene>();
}

SceneName ResultScene::GetSceneName() const { return SceneName::Result; }
