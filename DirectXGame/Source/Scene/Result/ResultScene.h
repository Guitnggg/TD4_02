#pragma once

#include "../IScene.h"

#include <string>

class ResultScene : public IScene {
public:
	ResultScene(std::string currentStagePath, int placedGimmickCount);

	void Initialize() override;

	void Update() override;

	void Draw() override;

	bool IsEnd() const override;

	std::unique_ptr<IScene> NextScene() const override;

	SceneName GetSceneName() const override;

private:
	bool isEnd_ = false;

	std::string currentStagePath_;

	int placedGimmickCount_ = 0;

	std::string evaluationLabel_;

	int selectedIndex_ = 0;

	bool toStageSelect_ = false;
};
