#pragma once

#include "../IScene.h"

#include <2d/Sprite.h>
#include <array>
#include <memory>
#include <string>

/// 難易度を選択し、対応する最初のステージをGameSceneへ渡す。
class DifficultySelectScene : public IScene {
public:
	void Initialize() override;

	void Update() override;

	void Draw() override;

	bool IsEnd() const override;

	std::unique_ptr<IScene> NextScene() const override;

	SceneName GetSceneName() const override;

	// 最終ステージをクリアした難易度を、難易度選択画面のクリア表示へ反映する。
	static void MarkDifficultyCleared(const std::string& clearedStagePath);

private:
	struct Difficulty {
		// 画面表示名と、決定時に読み込むCSVファイルのパス。
		const char* name = "";
		const char* stageFilePath = "";
	};

	// 配列の並び順をキーボード・マウスでの選択順として使用する。
	static constexpr std::array<Difficulty, 4> kDifficulties{
	    {
         {"TUTORIAL", "Resources\\Stages\\Tutorial\\Tutorial_01.csv"},
         {"EASY", "Resources\\Stages\\Easy\\Easy_01.csv"},
         {"NORMAL", "Resources\\Stages\\Normal\\Normal_01.csv"},
         {"HARD", "Resources\\Stages\\Hard\\Hard_01.csv"},
	     }
    };

	bool isEnd_ = false;
	int selectedIndex_ = 0; // キーボード操作とマウスホバーで共有する選択番号。
	std::unique_ptr<KamataEngine::Sprite> backgroundSprite_;
	std::unique_ptr<KamataEngine::Sprite> panelSprite_;
	std::unique_ptr<KamataEngine::Sprite> cursorSprite_;
	std::unique_ptr<KamataEngine::Sprite> clearStarSprite_;
	uint32_t decisionSoundHandle_ = 0;
};
