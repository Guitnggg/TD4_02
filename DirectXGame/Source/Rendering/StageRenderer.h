#pragma once

#include "../Game/Stage.h"

#include <3d/Camera.h>
#include <3d/Model.h>
#include <3d/Object3d.h>
#include <math/Vector3.h>

#include <memory>
#include <vector>

class StageRenderer {
public:
	void Initialize(const Stage& stage, const KamataEngine::Vector3& playerPosition);
	void Draw(KamataEngine::Camera& camera);
	void DrawGuide(const Stage& stage, KamataEngine::Camera& camera);
	void UpdatePlayer(const KamataEngine::Vector3& playerPosition);

private:
	std::unique_ptr<KamataEngine::Object3d> CreateCube(const KamataEngine::Vector3& translation, const KamataEngine::Vector3& scale);
	void BuildStageObjects(const Stage& stage, const KamataEngine::Vector3& playerPosition);

	std::unique_ptr<KamataEngine::Model> cubeModel_;
	std::vector<std::unique_ptr<KamataEngine::Object3d>> floorObjects_;
	std::vector<std::unique_ptr<KamataEngine::Object3d>> wallObjects_;
	std::vector<std::unique_ptr<KamataEngine::Object3d>> placeableObjects_;
	std::vector<std::unique_ptr<KamataEngine::Object3d>> gimmickObjects_;
	std::unique_ptr<KamataEngine::Object3d> goalObject_;
	std::unique_ptr<KamataEngine::Object3d> playerObject_;
};
