#pragma once

#include "../IScene.h"

#include <2d/Sprite.h>
#include <array>
#include <memory>
#include <string>

/// クリア評価を表示し、次のステージまたはステージ選択への遷移を受け付ける。
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
	// 各星評価には、2種類のメニュー選択状態に対応する画像がある。
	static constexpr size_t kSelectionFrameCount = 2;
	int usedGimmickCount_ = 0;
	int starCount_ = 3;
	int selectedIndex_ = 0;
	bool isEnd_ = false;
	std::string clearedStagePath_;
	std::string nextStagePath_; // 最終ステージをクリアした場合は空文字列になる。
	std::array<uint32_t, kSelectionFrameCount> resultTextureHandles_{};
	std::unique_ptr<KamataEngine::Sprite> backgroundSprite_;
	std::unique_ptr<KamataEngine::Sprite> resultSprite_;
	uint32_t decisionSoundHandle_ = 0;
};
