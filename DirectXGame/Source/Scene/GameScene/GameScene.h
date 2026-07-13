#pragma once

#include "../../Game/Stage.h"
#include "../../Gameplay/Player/Player.h"
#include "../../Input/DragInput.h"
#include "../../Rendering/StageRenderer.h"
#include "../../UI/UI.h"
#include "../IScene.h"

#include <3d/Camera.h>

#include <memory>
#include <string>

/// <summary>
/// ゲーム本編シーンの進行、入力、描画を管理するクラス
/// </summary>
class GameScene : public IScene {
public:
	GameScene() = default;
	explicit GameScene(std::string stageFilePath);

	void Initialize() override;
	void Update() override;
	void Draw() override;
	bool IsEnd() const override;
	std::unique_ptr<IScene> NextScene() const override;
	SceneName GetSceneName() const override;

private:
	bool isEnd_ = false;
	std::string stageFilePath_ = "Resources\\Stages\\Eazy\\Eazy_01.csv";
	bool returnTitle_ = false;
	bool returnStageSelect_ = false;

	Stage stage_;
	Player player_;
	DragInput dragInput_;
	StageRenderer stageRenderer_;
	KamataEngine::Camera camera_;
	UI ui_;
};
