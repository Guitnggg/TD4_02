#pragma once

#include "../../Game/Stage.h"
#include "../../Gameplay/Player/Player.h"
#include "../../Input/DragInput.h"
#include "../../Rendering/StageRenderer.h"
#include "../../UI/UI.h"
#include "../IScene.h"

#include <3d/Camera.h>

#include <memory>

class GameScene : public IScene {
public:
	void Initialize() override;
	void Update() override;
	void Draw() override;
	bool IsEnd() const override;
	std::unique_ptr<IScene> NextScene() const override;
	SceneName GetSceneName() const override;

private:
	bool isEnd_ = false;
	bool returnTitle_ = false;
	Stage stage_;
	Player player_;
	DragInput dragInput_;
	StageRenderer stageRenderer_;
	KamataEngine::Camera camera_;
	UI ui_;
};
