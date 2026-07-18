#pragma once

#include "../IScene.h"

#include <2d/Sprite.h>
#include <array>
#include <memory>
#include <string>

class ResultScene : public IScene {
public:
	ResultScene(int usedGimmickCount, std::string clearedStagePath);

	void Initialize() override;

	void Update() override;

	void Draw() override;

	bool IsEnd() const override;

	std::unique_ptr<IScene> NextScene() const override;

	SceneName GetSceneName() const override;

private:
	static constexpr size_t kSelectionFrameCount = 2;
	int usedGimmickCount_ = 0;
	int starCount_ = 3;
	int selectedIndex_ = 0;
	bool isEnd_ = false;
	std::string clearedStagePath_;
	std::string nextStagePath_;
	std::array<uint32_t, kSelectionFrameCount> resultTextureHandles_{};
	std::unique_ptr<KamataEngine::Sprite> resultSprite_;
	uint32_t decisionSoundHandle_ = 0;
};
