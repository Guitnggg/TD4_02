#pragma once

#include "../IScene.h"

#include <array>
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

	static constexpr std::array<Difficulty, 3> kDifficulties{{
	    {"EASY", "Resources\\Stages\\Eazy\\Eazy.json"},
	    {"NORMAL", "Resources\\Stages\\Normal\\Normal.json"},
	    {"HARD", "Resources\\Stages\\Hard\\Hard.json"},
	}};

	bool isEnd_ = false;
	int selectedIndex_ = 0;
};
