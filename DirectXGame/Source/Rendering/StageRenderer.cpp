#include "StageRenderer.h"

#include <3d/PrimitiveDrawer.h>

#include <numbers>

using namespace KamataEngine;

namespace {
// キューブモデルは中心基準のため、各スケール値は半径方向の大きさを表す。
constexpr float kFloorHeight = -0.06f;
constexpr float kTileHalfHeight = 0.03f;
constexpr float kObjectHeight = 0.375f;
constexpr float kPlayerScale = 0.225f;
constexpr float kGoalScale = 0.275f;
constexpr float kGimmickScale = 0.21f;
constexpr float kWallScale = 0.41f;
constexpr float kCursorHeight = 0.25f;
} // namespace

void StageRenderer::Initialize(const Stage& stage, const Vector3& playerPosition) {
	cubeModel_.reset(Model::CreateFromOBJ("cube"));
	BuildStageObjects(stage, playerPosition);
}

void StageRenderer::Draw(Camera& camera) {
	// 不透明なステージ部品を一定順序で描画し、その後に補助表示とキャラクターを描く。
	Object3d::PreDraw(&camera);

	for (const std::unique_ptr<Object3d>& object : floorObjects_) {
		object->Draw(camera);
	}
	for (const std::unique_ptr<Object3d>& object : placeableObjects_) {
		object->Draw(camera);
	}
	for (const std::unique_ptr<Object3d>& object : wallObjects_) {
		object->Draw(camera);
	}
	for (const std::unique_ptr<Object3d>& object : gimmickObjects_) {
		object->Draw(camera);
	}
	if (isPlacementCursorVisible_ && placementCursorObject_) {
		placementCursorObject_->Draw(camera);
	}
	if (goalObject_) {
		goalObject_->Draw(camera);
	}
	if (playerObject_) {
		playerObject_->Draw(camera);
	}

	Object3d::PostDraw();
}

void StageRenderer::DrawGuide(const Stage& stage, Camera& camera) {
	// 描画とゲーム座標を一致させるため、グリッド線にもStageのセルサイズを使用する。
	PrimitiveDrawer* primitiveDrawer = PrimitiveDrawer::GetInstance();
	primitiveDrawer->SetCamera(&camera);

	const float halfCell = stage.GetCellSize() * 0.5f;
	const float y = 0.03f;
	const Vector4 color = {0.25f, 0.35f, 0.50f, 1.0f};

	for (int z = 0; z < stage.GetHeight(); ++z) {
		for (int x = 0; x < stage.GetWidth(); ++x) {
			const Vector3 center = stage.GridToWorld({x, z});
			const float left = center.x - halfCell;
			const float right = center.x + halfCell;
			const float front = center.z - halfCell;
			const float back = center.z + halfCell;
			primitiveDrawer->DrawLine3d({left, y, front}, {right, y, front}, color);
			primitiveDrawer->DrawLine3d({right, y, front}, {right, y, back}, color);
			primitiveDrawer->DrawLine3d({right, y, back}, {left, y, back}, color);
			primitiveDrawer->DrawLine3d({left, y, back}, {left, y, front}, color);
		}
	}
}

void StageRenderer::UpdatePlayer(const Vector3& playerPosition) {
	if (!playerObject_) {
		return;
	}

	playerObject_->SetTranslation(playerPosition);
	playerObject_->Update();
}

void StageRenderer::RebuildGimmicks(const Stage& stage) { BuildGimmickObjects(stage); }

void StageRenderer::UpdatePlacementCursor(const Stage& stage, const Stage::GridPosition& grid, Stage::GimmickType selectedType, bool isVisible) {
	isPlacementCursorVisible_ = isVisible;
	if (!isVisible || !placementCursorObject_) {
		return;
	}

	Vector3 position = stage.GridToWorld(grid);
	position.y = kCursorHeight;
	placementCursorObject_->SetTranslation(position);
	placementCursorObject_->SetScale({kGimmickScale * 1.15f, 0.06f, stage.GetCellSize() * 0.62f});

	if (selectedType == Stage::GimmickType::ReflectBackSlash) {
		placementCursorObject_->SetRotation({0.0f, std::numbers::pi_v<float> * 0.25f, 0.0f});
	} else if (selectedType == Stage::GimmickType::AccelerationPanel) {
		placementCursorObject_->SetRotation({0.0f, 0.0f, 0.0f});
	} else {
		placementCursorObject_->SetRotation({0.0f, -std::numbers::pi_v<float> * 0.25f, 0.0f});
	}

	placementCursorObject_->Update();
}

std::unique_ptr<Object3d> StageRenderer::CreateCube(const Vector3& translation, const Vector3& scale) {
	std::unique_ptr<Object3d> object = std::make_unique<Object3d>();
	object->Initialize(cubeModel_.get());
	object->SetTranslation(translation);
	object->SetScale(scale);
	object->Update();
	return object;
}

void StageRenderer::BuildStageObjects(const Stage& stage, const Vector3& playerPosition) {
	// 新しいマップを構築する前に、以前のObject3dをすべて破棄する。
	floorObjects_.clear();
	wallObjects_.clear();
	placeableObjects_.clear();
	gimmickObjects_.clear();
	placementCursorObject_.reset();

	const float cellSize = stage.GetCellSize();
	const Vector3 floorScale = {cellSize * 0.47f, kTileHalfHeight, cellSize * 0.47f};

	for (int z = 0; z < stage.GetHeight(); ++z) {
		for (int x = 0; x < stage.GetWidth(); ++x) {
			Vector3 position = stage.GridToWorld({x, z});
			position.y = kFloorHeight;
			floorObjects_.push_back(CreateCube(position, floorScale));
		}
	}

	for (const Stage::GridPosition& grid : stage.GetPlaceableTiles()) {
		Vector3 position = stage.GridToWorld(grid);
		position.y = 0.01f;
		placeableObjects_.push_back(CreateCube(position, {cellSize * 0.36f, 0.04f, cellSize * 0.36f}));
	}

	for (const Stage::GridPosition& grid : stage.GetWalls()) {
		Vector3 position = stage.GridToWorld(grid);
		position.y = kObjectHeight;
		wallObjects_.push_back(CreateCube(position, {kWallScale, kObjectHeight, kWallScale}));
	}

	BuildGimmickObjects(stage);

	Vector3 goalPosition = stage.GridToWorld(stage.GetGoalGrid());
	goalPosition.y = 0.225f;
	goalObject_ = CreateCube(goalPosition, {kGoalScale, kGoalScale, kGoalScale});

	playerObject_ = CreateCube(playerPosition, {kPlayerScale, kPlayerScale, kPlayerScale});

	placementCursorObject_ = CreateCube(stage.GridToWorld({0, 0}), {kGimmickScale, 0.06f, cellSize * 0.62f});
	isPlacementCursorVisible_ = false;
}

void StageRenderer::BuildGimmickObjects(const Stage& stage) {
	gimmickObjects_.clear();

	const float cellSize = stage.GetCellSize();

	for (const Stage::GridPosition& grid : stage.GetReflectSlashTiles()) {
		Vector3 position = stage.GridToWorld(grid);
		position.y = 0.175f;
		std::unique_ptr<Object3d> object = CreateCube(position, {kGimmickScale, 0.07f, cellSize * 0.58f});
		object->SetRotation({0.0f, -std::numbers::pi_v<float> * 0.25f, 0.0f});
		object->Update();
		gimmickObjects_.push_back(std::move(object));
	}

	for (const Stage::GridPosition& grid : stage.GetReflectBackSlashTiles()) {
		Vector3 position = stage.GridToWorld(grid);
		position.y = 0.175f;
		std::unique_ptr<Object3d> object = CreateCube(position, {kGimmickScale, 0.07f, cellSize * 0.58f});
		object->SetRotation({0.0f, std::numbers::pi_v<float> * 0.25f, 0.0f});
		object->Update();
		gimmickObjects_.push_back(std::move(object));
	}

	for (const AccelerationPanel& panel : stage.GetAccelerationPanels()) {
		// 斜めの反射ギミックと見分けられるよう、低い正方形の床として描画する。
		Vector3 position = stage.GridToWorld({panel.GetGridX(), panel.GetGridZ()});
		position.y = 0.08f;
		gimmickObjects_.push_back(CreateCube(position, {cellSize * 0.40f, 0.035f, cellSize * 0.40f}));
	}
}
