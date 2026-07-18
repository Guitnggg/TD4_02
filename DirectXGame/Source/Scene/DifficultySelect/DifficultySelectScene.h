#pragma once

#include "../IScene.h"

#include <array>
#include <2d/Sprite.h>
#include <memory>

class DifficultySelectScene : public IScene {
public:
	void Initialize() override;

	void Update() override;

	void Draw() override;

	bool IsEnd() const override;

	std::unique_ptr<IScene> NextScene() const override;

	SceneName GetSceneName() const override;

private:
	struct Difficulty {
		const char* name = "";
		const char* stageFilePath = "";
	};

	static constexpr std::array<Difficulty, 4> kDifficulties{{
	    {"TUTORIAL", "Resources\\Stages\\Tutorial\\Tutorial_01.csv"},
	    {"EAZY", "Resources\\Stages\\Eazy\\Eazy_01.csv"},
	    {"NORMAL", "Resources\\Stages\\Normal\\Normal_01.csv"},
	    {"HARD", "Resources\\Stages\\Hard\\Hard_01.csv"},
	}};

	bool isEnd_ = false;
	int selectedIndex_ = 0;
	std::unique_ptr<KamataEngine::Sprite> backgroundSprite_;
	std::unique_ptr<KamataEngine::Sprite> panelSprite_;
	std::unique_ptr<KamataEngine::Sprite> cursorSprite_;
	uint32_t decisionSoundHandle_ = 0;
};
