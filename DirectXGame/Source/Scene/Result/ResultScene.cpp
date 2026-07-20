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
	if (posMarker == std::string::npos) {
		return {};
	}

	const auto posAfterMarker = posMarker + marker.size();
	const auto posNextSlash = currentPath.find('\\', posAfterMarker);
	if (posNextSlash == std::string::npos) {
		return {};
	}

	const std::string currentKey = currentPath.substr(posAfterMarker, posNextSlash - posAfterMarker);

	auto it = std::find(keys.begin(), keys.end(), currentKey);
	if (it == keys.end()) {
		return {};
	}

	const size_t idx = static_cast<size_t>(std::distance(keys.begin(), it));
	if (idx + 1 >= keys.size()) {
		// 最後の難易度だった場合は次が無いので空を返してステージセレクトへ戻す
		return {};
	}

	const std::string nextKey = keys[idx + 1];

	// ファイル名を取得（最後の '\' の次から末尾まで）
	const auto posLastSlash = currentPath.find_last_of('\\');
	if (posLastSlash == std::string::npos || posLastSlash + 1 >= currentPath.size()) {
		return {};
	}
	const std::string fileName = currentPath.substr(posLastSlash + 1);

	// ファイル名の接尾辞（'_' 以降）を保持して、先頭のキー部分を差し替える
	const auto posUnderscore = fileName.find('_');
	std::string suffix;
	if (posUnderscore != std::string::npos) {
		suffix = fileName.substr(posUnderscore); // 包括 '_' 以降
	} else {
		// '_' が無ければ拡張子を含めた ".csv" を残す
		const auto posDot = fileName.find('.');
		if (posDot != std::string::npos) {
			suffix = fileName.substr(posDot);
		} else {
			// ファイル名形式不明
			return {};
		}
	}

	const std::string nextFileName = nextKey + suffix;
	// 組み立て: Resources\Stages\{nextKey}\{nextFileName}
	std::string nextPath = marker + nextKey + "\\" + nextFileName;
	return nextPath;
}
} // namespace

ResultScene::ResultScene(std::string currentStagePath, int placedGimmickCount)
	: isEnd_(false),
	  currentStagePath_(std::move(currentStagePath)),
	  placedGimmickCount_(placedGimmickCount),
	  selectedIndex_(0),
	  toStageSelect_(false) {
	// ギミック数に応じて評価ラベルを決定
	if (placedGimmickCount_ == 0) {
		evaluationLabel_ = "Stars: ★★★ (0 gimmicks)";
	} else if (placedGimmickCount_ >= 1 && placedGimmickCount_ <= 3) {
		evaluationLabel_ = "Stars: ★★ (1-3 gimmicks)";
	} else { // 4 個以上
		evaluationLabel_ = "Stars: ★ (4+ gimmicks)";
	}
}

void ResultScene::Initialize() { isEnd_ = false; }

void ResultScene::Update() {
	Input* input = Input::GetInstance();

	// 上下キーで選択を切り替え（0..1 を循環させる）
	if (input->TriggerKey(DIK_UP)) {
		selectedIndex_ = (selectedIndex_ == 0) ? 1 : selectedIndex_ - 1;
	}
	if (input->TriggerKey(DIK_DOWN)) {
		selectedIndex_ = (selectedIndex_ == 1) ? 0 : selectedIndex_ + 1;
	}

	// 確定（SPACE または ENTER）
	if (input->TriggerKey(DIK_SPACE) || input->TriggerKey(DIK_RETURN)) {
		isEnd_ = true;
		toStageSelect_ = (selectedIndex_ == 1);
	}
}

void ResultScene::Draw() {
#ifdef USE_IMGUI
	ImGui::SetNextWindowPos(ImVec2(520.0f, 280.0f), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(360.0f, 200.0f), ImGuiCond_FirstUseEver);
	ImGui::Begin("ResultScene");
	ImGui::Text("CLEAR!");
	ImGui::Separator();

	// 評価（ギミック数に基づく星評価）
	ImGui::Text("%s", evaluationLabel_.c_str());
	ImGui::Separator();

	// 選択肢表示（選択中は色を変えて強調）
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
	// ステージセレクトを選んだ場合は難易度選択へ戻す
	if (toStageSelect_) {
		return std::make_unique<DifficultySelectScene>();
	}

	// Next Stage を選んだ場合は、現在のパスから次のステージパスを生成して遷移
	const std::string nextPath = MakeNextStagePath(currentStagePath_);
	if (nextPath.empty()) {
		// 生成できなければステージセレクトへ戻す
		return std::make_unique<DifficultySelectScene>();
	}

	// 次ステージへ進む（GameScene のコンストラクタはステージファイルパスを受け取る）
	return std::make_unique<GameScene>(nextPath);
}

SceneName ResultScene::GetSceneName() const { return SceneName::Result; }
