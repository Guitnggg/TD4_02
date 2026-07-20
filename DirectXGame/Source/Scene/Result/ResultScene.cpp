#include "ResultScene.h"

#include "../Title/TitleScene.h"
#include "../DifficultySelect/DifficultySelectScene.h"
#include "../GameScene/GameScene.h"

#include <KamataEngine.h>
#include <dinput.h>
#ifdef USE_IMGUI
#include <imgui.h>
#endif

#include <memory>
#include <algorithm>

using namespace KamataEngine;

namespace {
std::string MakeNextStagePath(const std::string& currentPath) {
	const std::vector<std::string> keys = { "Tutorial", "Eazy", "Normal", "Hard" };
	const std::string marker = "Resources\\Stages\\";
	const auto posMarker = currentPath.find(marker);
	if (posMarker == std::string::npos) return {};
	const auto posAfterMarker = posMarker + marker.size();
	const auto posNextSlash = currentPath.find('\\', posAfterMarker);
	if (posNextSlash == std::string::npos) return {};
	const std::string currentKey = currentPath.substr(posAfterMarker, posNextSlash - posAfterMarker);
	auto it = std::find(keys.begin(), keys.end(), currentKey);
	if (it == keys.end()) return {};
	const size_t idx = static_cast<size_t>(std::distance(keys.begin(), it));
	if (idx + 1 >= keys.size()) return {};
	const std::string nextKey = keys[idx + 1];
	const auto posLastSlash = currentPath.find_last_of('\\');
	if (posLastSlash == std::string::npos || posLastSlash + 1 >= currentPath.size()) return {};
	const std::string fileName = currentPath.substr(posLastSlash + 1);
	const auto posUnderscore = fileName.find('_');
	std::string suffix;
	if (posUnderscore != std::string::npos) {
		suffix = fileName.substr(posUnderscore);
	} else {
		const auto posDot = fileName.find('.');
		if (posDot != std::string::npos) suffix = fileName.substr(posDot);
		else return {};
	}
	const std::string nextFileName = nextKey + suffix;
	return marker + nextKey + "\\" + nextFileName;
}
} // namespace

ResultScene::ResultScene(std::string currentStagePath, int placedGimmickCount)
	: currentStagePath_(std::move(currentStagePath)), placedGimmickCount_(placedGimmickCount) {
	if (placedGimmickCount_ == 0) {
		evaluationLabel_ = "Stars: 3 (0 gimmicks)";
	} else if (placedGimmickCount_ <= 3) {
		evaluationLabel_ = "Stars: 2 (1-3 gimmicks)";
	} else {
		evaluationLabel_ = "Stars: 1 (4+ gimmicks)";
	}
}

ResultScene::~ResultScene() {
	// テクスチャ解放（Loaded via TextureManager）
	if (texStars3_) TextureManager::Unload(texStars3_);
	if (texStars3Sel_) TextureManager::Unload(texStars3Sel_);
	if (texStars2_) TextureManager::Unload(texStars2_);
	if (texStars2Sel_) TextureManager::Unload(texStars2Sel_);
	if (texStars1_) TextureManager::Unload(texStars1_);
	if (texStars1Sel_) TextureManager::Unload(texStars1Sel_);

	if (texNext_) TextureManager::Unload(texNext_);
	if (texNextSel_) TextureManager::Unload(texNextSel_);
	if (texStageSelect_) TextureManager::Unload(texStageSelect_);
	if (texStageSelectSel_) TextureManager::Unload(texStageSelectSel_);
}

void ResultScene::Initialize() {
	isEnd_ = false;
	selectedIndex_ = 0;
	toStageSelect_ = false;

	texStars3_ = TextureManager::Load("UI/GameClear6.png");
	texStars3Sel_ = TextureManager::Load("UI/GameClear5.png");
	texStars2_ = TextureManager::Load("UI/GameClear4.png");
	texStars2Sel_ = TextureManager::Load("UI/GameClear3.png");
	texStars1_ = TextureManager::Load("UI/GameClear2.png");
	texStars1Sel_ = TextureManager::Load("UI/GameClear1.png");

	// スプライト作成（位置は解像度に合わせて調整してください）
	spriteEvaluation_.reset(Sprite::Create(texStars3_, {0, 0})); 
	spriteNext_.reset(Sprite::Create(texNext_, {0, 0}));
	spriteStageSelect_.reset(Sprite::Create(texStageSelect_, {0, 0}));
}

void ResultScene::Update() {
	Input* input = Input::GetInstance();

	// 上下キーで選択を切り替え
	if (input->TriggerKey(DIK_UP) || input->TriggerKey(DIK_W)) {
		selectedIndex_ = max(0, selectedIndex_ - 1);
	}
	if (input->TriggerKey(DIK_DOWN) || input->TriggerKey(DIK_S)) {
		selectedIndex_ = min(1, selectedIndex_ + 1);
	}

	// 確定（SPACE または ENTER）
	if (input->TriggerKey(DIK_SPACE) || input->TriggerKey(DIK_RETURN)) {
		isEnd_ = true;
		toStageSelect_ = (selectedIndex_ == 1);
	}
}

void ResultScene::Draw() {
	// 2D スプライト描画
	ID3D12GraphicsCommandList* commandList = DirectXCommon::GetInstance()->GetCommandList();
	Sprite::PreDraw(commandList);

	// 星評価の選択（0: 3★, 1:2★, 2:1★）
	int ratingIndex = (placedGimmickCount_ == 0) ? 0 : (placedGimmickCount_ <= 3 ? 1 : 2);

	// ハイライト条件：ここでは "Next Stage" が選択中のときハイライトを表示する例
	const bool useHighlight = (selectedIndex_ == 0);

	int useTex = 0;
	switch (ratingIndex) {
	case 0: useTex = useHighlight && texStars3Sel_ ? texStars3Sel_ : texStars3_; break;
	case 1: useTex = useHighlight && texStars2Sel_ ? texStars2Sel_ : texStars2_; break;
	default: useTex = useHighlight && texStars1Sel_ ? texStars1Sel_ : texStars1_; break;
	}

	// スプライトにテクスチャを適用して描画
	if (spriteEvaluation_) {
		spriteEvaluation_->SetTextureHandle(useTex);
		spriteEvaluation_->Draw();
	}

	// Next / StageSelect スプライトも選択中ならハイライト版を使う
	if (spriteNext_) {
		spriteNext_->SetTextureHandle(selectedIndex_ == 0 && texNextSel_ ? texNextSel_ : texNext_);
		spriteNext_->Draw();
	}
	if (spriteStageSelect_) {
		spriteStageSelect_->SetTextureHandle(selectedIndex_ == 1 && texStageSelectSel_ ? texStageSelectSel_ : texStageSelect_);
		spriteStageSelect_->Draw();
	}

	Sprite::PostDraw();

	DirectXCommon::GetInstance()->ClearDepthBuffer();

#ifdef USE_IMGUI
	ImGui::SetNextWindowPos(ImVec2(520.0f, 280.0f), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(360.0f, 200.0f), ImGuiCond_FirstUseEver);
	ImGui::Begin("ResultScene");
	ImGui::Text("CLEAR!");
	ImGui::Separator();
	ImGui::Text("%s", evaluationLabel_.c_str());
	ImGui::Separator();
	const char* options[] = { "Next Stage", "Stage Select" };
	for (int i = 0; i < 2; ++i) {
		if (selectedIndex_ == i) {
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.85f, 0.2f, 1.0f));
			ImGui::Text("> %s", options[i]);
			ImGui::PopStyleColor();
		} else {
			ImGui::Text("  %s", options[i]);
		}
	}
	ImGui::Separator();
	ImGui::Text("Use UP/DOWN to change, SPACE/ENTER to confirm");
	ImGui::End();
#endif
}

bool ResultScene::IsEnd() const { return isEnd_; }

std::unique_ptr<IScene> ResultScene::NextScene() const {
	if (toStageSelect_) {
		return std::make_unique<DifficultySelectScene>();
	}
	const std::string nextPath = MakeNextStagePath(currentStagePath_);
	if (nextPath.empty()) {
		return std::make_unique<DifficultySelectScene>();
	}
	return std::make_unique<GameScene>(nextPath);
}

SceneName ResultScene::GetSceneName() const { return SceneName::Result; }
