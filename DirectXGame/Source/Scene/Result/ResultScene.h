#pragma once

#include "../IScene.h"
#include <string>

class ResultScene : public IScene {
public:
	explicit ResultScene(int placedGimmickCount);

	void Initialize() override;

	void Update() override;

	void Draw() override;

	bool IsEnd() const override;

	std::unique_ptr<IScene> NextScene() const override;

	SceneName GetSceneName() const override;

private:
	bool isEnd_ = false;

	int placedGimmickCount_ = 0;

	std::string evaluationLabel_;
};
