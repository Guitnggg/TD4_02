#include "ResultScene.h"

#include "../DifficultySelect/DifficultySelectScene.h"
#include "../GameScene/GameScene.h"

#include <KamataEngine.h>
#include <algorithm>
#include <dinput.h>
#include <filesystem>
#ifdef USE_IMGUI
#include <imgui.h>
#endif
#include <memory>
#include <string>
#include <utility>

using namespace KamataEngine;

namespace {
std::string FindNextStagePath(const std::string& currentPath) {
	const size_t underscore = currentPath.find_last_of('_');
	const size_t extension = currentPath.rfind(".csv");
	if (underscore == std::string::npos || extension == std::string::npos || extension <= underscore + 1) { return {}; }

	int stageNumber = 0;
	try {
		stageNumber = std::stoi(currentPath.substr(underscore + 1, extension - underscore - 1));
	} catch (...) {
		return {};
	}

	const int nextNumber = stageNumber + 1;
	const std::string number = nextNumber < 10 ? "0" + std::to_string(nextNumber) : std::to_string(nextNumber);
	const std::string candidate = currentPath.substr(0, underscore + 1) + number + ".csv";
	return std::filesystem::exists(candidate) ? candidate : std::string{};
}
} // namespace

ResultScene::ResultScene(int usedGimmickCount, std::string clearedStagePath)
    : usedGimmickCount_(usedGimmickCount < 0 ? 0 : usedGimmickCount), starCount_(std::clamp(3 - usedGimmickCount_, 0, 3)),
      clearedStagePath_(std::move(clearedStagePath)), nextStagePath_(FindNextStagePath(clearedStagePath_)) {}

void ResultScene::Initialize() {
	isEnd_ = false;
	selectedIndex_ = nextStagePath_.empty() ? 1 : 0;
	// Each star rating has two images for the two menu selections.
	const int firstImageNumber = starCount_ * 2 + 1;
	for (size_t i = 0; i < kSelectionFrameCount; ++i) {
		resultTextureHandles_[i] = TextureManager::Load("Result/GameClear" + std::to_string(firstImageNumber + static_cast<int>(i)) + ".png");
	}
	resultSprite_.reset(Sprite::Create(resultTextureHandles_[selectedIndex_], {0.0f, 0.0f}));

	Audio* audio = Audio::GetInstance();
	const uint32_t fanfareHandle = audio->LoadWave("SE/GameClear.mp3");
	decisionSoundHandle_ = audio->LoadWave("SE/Dicision.mp3");
	audio->PlayWave(fanfareHandle, false, 0.7f);
}

void ResultScene::Update() {
	Input* input = Input::GetInstance();
	if (input->TriggerKey(DIK_W) || input->TriggerKey(DIK_UP) || input->TriggerKey(DIK_S) || input->TriggerKey(DIK_DOWN)) {
		selectedIndex_ = 1 - selectedIndex_;
		if (resultSprite_) { resultSprite_->SetTextureHandle(resultTextureHandles_[selectedIndex_]); }
	}

	if (input->TriggerKey(DIK_SPACE) || input->TriggerKey(DIK_RETURN)) {
		Audio::GetInstance()->PlayWave(decisionSoundHandle_, false, 0.8f);
		isEnd_ = true;
	}
}

void ResultScene::Draw() {
	Sprite::PreDraw(DirectXCommon::GetInstance()->GetCommandList());
	if (resultSprite_) { resultSprite_->Draw(); }
	Sprite::PostDraw();
#ifdef USE_IMGUI
	ImGui::SetNextWindowPos(ImVec2(520.0f, 280.0f), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(240.0f, 120.0f), ImGuiCond_FirstUseEver);
	ImGui::Begin("ResultScene");
	ImGui::Text("CLEAR!");
	ImGui::Text("Used gimmicks: %d", usedGimmickCount_);
	ImGui::Text("Stars: %d", starCount_);
	ImGui::Text("Selected: %s", selectedIndex_ == 0 ? "NEXT STAGE" : "STAGE SELECT");
	ImGui::Separator();
	ImGui::Text("UP/DOWN: select, SPACE/ENTER: decide");
	ImGui::End();
#endif
}

bool ResultScene::IsEnd() const { return isEnd_; }

std::unique_ptr<IScene> ResultScene::NextScene() const {
	if (selectedIndex_ == 0 && !nextStagePath_.empty()) {
		return std::make_unique<GameScene>(nextStagePath_);
	}
	return std::make_unique<DifficultySelectScene>();
}

SceneName ResultScene::GetSceneName() const { return SceneName::Result; }
