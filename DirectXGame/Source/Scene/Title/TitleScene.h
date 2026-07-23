#pragma once

#include "../IScene.h"

#include <2d/Sprite.h>
#include <memory>

/// タイトル画面を表示し、キーボードまたはマウスによる決定入力を待つ。
class TitleScene : public IScene {
public:
	void Initialize() override;

	void Update() override;

	void Draw() override;

	bool IsEnd() const override;

	std::unique_ptr<IScene> NextScene() const override;

	SceneName GetSceneName() const override;

private:
	// trueになるとSceneManagerが次のシーンへ切り替える。
	bool isEnd_ = false;
	std::unique_ptr<KamataEngine::Sprite> backgroundSprite_;
};
