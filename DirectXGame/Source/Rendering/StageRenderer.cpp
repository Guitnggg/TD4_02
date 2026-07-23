#include "StageRenderer.h"

#include <3d/PrimitiveDrawer.h>
#include <3d/ObjectColor.h>

#include <numbers>

using namespace KamataEngine;

namespace {
// キューブモデルは中心基準のため、各スケール値は半径方向の大きさを表す。
constexpr float kFloorHeight = -0.06f;
constexpr float kTileHalfHeight = 0.03f;
constexpr float kObjectHeight = 0.375f;
constexpr float kPlayerScale = 0.30f;
constexpr float kGoalScale = 0.35f;
constexpr float kGimmickScale = 0.21f;
constexpr float kReflectGimmickHeight = 0.42f;
constexpr float kAccelerationPanelScale = 0.21f;
constexpr float kAccelerationPanelHeight = 0.084f;
constexpr float kWallScale = 0.41f;

// 視認性確認用の仮配色。正式なアートへ差し替えるときは、この定数群だけを変更する。
constexpr Vector4 kFloorColor = {0.10f, 0.14f, 0.22f, 1.0f};
constexpr Vector4 kWallColor = {0.38f, 0.43f, 0.52f, 1.0f};
constexpr Vector4 kPlayerColor = {0.15f, 0.45f, 1.0f, 1.0f};
constexpr Vector4 kReflectFrameColor = {0.015f, 0.015f, 0.02f, 1.0f};
constexpr Vector4 kReflectCenterColor = {0.95f, 0.97f, 1.0f, 1.0f};
constexpr Vector4 kAccelerationPanelBaseColor = {0.62f, 0.68f, 0.76f, 1.0f};
constexpr Vector4 kAccelerationPanelArrowColor = {0.10f, 0.28f, 1.0f, 1.0f};

class ColoredObject3d final : public Object3d {
public:
	void Initialize(Model* model, const Vector4& color) {
		Object3d::Initialize(model);
		objectColor_.Initialize();
		objectColor_.SetColor(color);
	}

	void Draw(const Camera& camera) override { model_->Draw(worldTransform_, camera, &objectColor_); }

private:
	ObjectColor objectColor_;
};
} // namespace

void StageRenderer::Initialize(const Stage& stage, const Vector3& playerPosition) {
	cubeModel_.reset(Model::CreateFromOBJ("cube"));
	floorModel_.reset(Model::CreateFromOBJ("Floar"));
	wallModel_.reset(Model::CreateFromOBJ("Wall"));
	playerModel_.reset(Model::CreateFromOBJ("Player"));
	goalModel_.reset(Model::CreateFromOBJ("Goal"));
	reflectGimmickModel_.reset(Model::CreateFromOBJ("ReflectGimmick"));
	reflectGimmickCenterModel_.reset(Model::CreateFromOBJ("ReflectGimmickCenter"));
	accelerationPanelModel_.reset(Model::CreateFromOBJ("AccelerationPanel"));
	accelerationPanelBaseModel_.reset(Model::CreateFromOBJ("AccelerationPanelBase"));
	BuildStageObjects(stage, playerPosition);
}

void StageRenderer::Draw(Camera& camera) {
	// 不透明なステージ部品を一定順序で描画し、その後に補助表示とキャラクターを描く。
	Object3d::PreDraw(&camera);

	for (const std::unique_ptr<Object3d>& object : floorObjects_) {
		object->Draw(camera);
	}
	for (const std::unique_ptr<Object3d>& object : wallObjects_) {
		object->Draw(camera);
	}
	for (const std::unique_ptr<Object3d>& object : gimmickObjects_) {
		object->Draw(camera);
	}
	if (isPlacementCursorBaseVisible_ && placementCursorBaseObject_) {
		placementCursorBaseObject_->Draw(camera);
	}
	if (isPlacementCursorArrowVisible_ && placementCursorArrowObject_) {
		placementCursorArrowObject_->Draw(camera);
	}
	if (isPlacementCursorVisible_ && placementCursorObject_) {
		placementCursorObject_->Draw(camera);
	}
	if (isPlacementCursorReflectCenterVisible_ && placementCursorReflectCenterObject_) {
		placementCursorReflectCenterObject_->Draw(camera);
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

namespace {
float AccelerationPanelRotation(AccelerationPanel::Direction direction) {
	switch (direction) {
	case AccelerationPanel::Direction::PositiveZ: return 0.0f;
	case AccelerationPanel::Direction::PositiveX: return std::numbers::pi_v<float> * 0.5f;
	case AccelerationPanel::Direction::NegativeZ: return std::numbers::pi_v<float>;
	case AccelerationPanel::Direction::NegativeX: return -std::numbers::pi_v<float> * 0.5f;
	}
	return 0.0f;
}
} // namespace

void StageRenderer::UpdatePlacementCursor(const Stage& stage, const Stage::GridPosition& grid, Stage::GimmickType selectedType, bool isVisible,
                                          AccelerationPanel::Direction panelDirection) {
	isPlacementCursorVisible_ = isVisible && selectedType != Stage::GimmickType::AccelerationPanel;
	isPlacementCursorReflectCenterVisible_ = isVisible && selectedType != Stage::GimmickType::AccelerationPanel;
	isPlacementCursorBaseVisible_ = isVisible && selectedType == Stage::GimmickType::AccelerationPanel;
	isPlacementCursorArrowVisible_ = isVisible && selectedType == Stage::GimmickType::AccelerationPanel;
	if (!isVisible || !placementCursorObject_) {
		return;
	}

	Vector3 position = stage.GridToWorld(grid);
	position.y = selectedType == Stage::GimmickType::AccelerationPanel ? kAccelerationPanelHeight : kReflectGimmickHeight;
	placementCursorObject_->SetTranslation(position);
	placementCursorObject_->SetModel(selectedType == Stage::GimmickType::AccelerationPanel ? accelerationPanelModel_.get() : reflectGimmickModel_.get());
	placementCursorObject_->SetScale(selectedType == Stage::GimmickType::AccelerationPanel
	                                    ? Vector3{kAccelerationPanelScale, kAccelerationPanelScale, kAccelerationPanelScale}
	                                    : Vector3{kGimmickScale, kGimmickScale, kGimmickScale});

	if (selectedType == Stage::GimmickType::ReflectBackSlash) {
		placementCursorObject_->SetRotation({0.0f, -std::numbers::pi_v<float> * 0.25f, 0.0f});
	} else if (selectedType == Stage::GimmickType::AccelerationPanel) {
		placementCursorObject_->SetRotation({0.0f, AccelerationPanelRotation(panelDirection), 0.0f});
	} else {
		placementCursorObject_->SetRotation({0.0f, std::numbers::pi_v<float> * 0.25f, 0.0f});
	}

	placementCursorObject_->Update();
	if (isPlacementCursorReflectCenterVisible_ && placementCursorReflectCenterObject_) {
		Vector3 centerPosition = position;
		centerPosition.y += 0.002f;
		placementCursorReflectCenterObject_->SetTranslation(centerPosition);
		placementCursorReflectCenterObject_->SetScale({kGimmickScale, kGimmickScale, kGimmickScale});
		placementCursorReflectCenterObject_->SetRotation(placementCursorObject_->GetRotation());
		placementCursorReflectCenterObject_->Update();
	}
	if (isPlacementCursorBaseVisible_ && placementCursorBaseObject_) {
		placementCursorBaseObject_->SetTranslation(position);
		placementCursorBaseObject_->SetScale({kAccelerationPanelScale, kAccelerationPanelScale, kAccelerationPanelScale});
		placementCursorBaseObject_->SetRotation({0.0f, AccelerationPanelRotation(panelDirection), 0.0f});
		placementCursorBaseObject_->Update();
	}
	if (isPlacementCursorArrowVisible_ && placementCursorArrowObject_) {
		placementCursorArrowObject_->SetTranslation(position);
		placementCursorArrowObject_->SetScale({kAccelerationPanelScale, kAccelerationPanelScale, kAccelerationPanelScale});
		placementCursorArrowObject_->SetRotation({0.0f, AccelerationPanelRotation(panelDirection), 0.0f});
		placementCursorArrowObject_->Update();
	}
}

std::unique_ptr<Object3d> StageRenderer::CreateCube(const Vector3& translation, const Vector3& scale, const Vector4& color) {
	std::unique_ptr<ColoredObject3d> object = std::make_unique<ColoredObject3d>();
	object->Initialize(cubeModel_.get(), color);
	object->SetTranslation(translation);
	object->SetScale(scale);
	object->Update();
	return object;
}

void StageRenderer::BuildStageObjects(const Stage& stage, const Vector3& playerPosition) {
	// 新しいマップを構築する前に、以前のObject3dをすべて破棄する。
	floorObjects_.clear();
	wallObjects_.clear();
	gimmickObjects_.clear();
	placementCursorObject_.reset();
	placementCursorReflectCenterObject_.reset();
	placementCursorBaseObject_.reset();
	placementCursorArrowObject_.reset();

	const float cellSize = stage.GetCellSize();
	const Vector3 floorScale = {cellSize * 0.47f, kTileHalfHeight, cellSize * 0.47f};

	for (int z = 0; z < stage.GetHeight(); ++z) {
		for (int x = 0; x < stage.GetWidth(); ++x) {
			Vector3 position = stage.GridToWorld({x, z});
			position.y = kFloorHeight;
			std::unique_ptr<Object3d> floor = std::make_unique<Object3d>();
			floor->Initialize(floorModel_.get());
			floor->SetTranslation(position);
			floor->SetScale(floorScale);
			floor->Update();
			floorObjects_.push_back(std::move(floor));
		}
	}

	for (const Stage::GridPosition& grid : stage.GetWalls()) {
		Vector3 position = stage.GridToWorld(grid);
		position.y = kObjectHeight;
		std::unique_ptr<Object3d> wall = std::make_unique<Object3d>();
		wall->Initialize(wallModel_.get());
		wall->SetTranslation(position);
		wall->SetScale({kWallScale, kObjectHeight, kWallScale});
		wall->Update();
		wallObjects_.push_back(std::move(wall));
	}

	BuildGimmickObjects(stage);

	Vector3 goalPosition = stage.GridToWorld(stage.GetGoalGrid());
	goalPosition.y = kGoalScale;
	goalObject_ = std::make_unique<Object3d>();
	goalObject_->Initialize(goalModel_.get());
	goalObject_->SetTranslation(goalPosition);
	goalObject_->SetScale({kGoalScale, kGoalScale, kGoalScale});
	goalObject_->Update();

	std::unique_ptr<ColoredObject3d> player = std::make_unique<ColoredObject3d>();
	player->Initialize(playerModel_.get(), kPlayerColor);
	playerObject_ = std::move(player);
	playerObject_->SetTranslation(playerPosition);
	playerObject_->SetScale({kPlayerScale, kPlayerScale, kPlayerScale});
	// Blender側のモデルはZ方向に厚みがあるため、寝かせて上面から見える向きにする。
	playerObject_->SetRotation({-std::numbers::pi_v<float> * 0.5f, 0.0f, 0.0f});
	playerObject_->Update();

	std::unique_ptr<ColoredObject3d> reflectCursor = std::make_unique<ColoredObject3d>();
	reflectCursor->Initialize(reflectGimmickModel_.get(), kReflectFrameColor);
	placementCursorObject_ = std::move(reflectCursor);
	placementCursorObject_->SetTranslation(stage.GridToWorld({0, 0}));
	placementCursorObject_->SetScale({kGimmickScale, kGimmickScale, kGimmickScale});
	placementCursorObject_->Update();
	std::unique_ptr<ColoredObject3d> reflectCenterCursor = std::make_unique<ColoredObject3d>();
	reflectCenterCursor->Initialize(reflectGimmickCenterModel_.get(), kReflectCenterColor);
	reflectCenterCursor->SetTranslation(stage.GridToWorld({0, 0}));
	reflectCenterCursor->SetScale({kGimmickScale, kGimmickScale, kGimmickScale});
	reflectCenterCursor->Update();
	placementCursorReflectCenterObject_ = std::move(reflectCenterCursor);
	std::unique_ptr<ColoredObject3d> baseCursor = std::make_unique<ColoredObject3d>();
	baseCursor->Initialize(accelerationPanelBaseModel_.get(), kAccelerationPanelBaseColor);
	baseCursor->SetTranslation(stage.GridToWorld({0, 0}));
	baseCursor->SetScale({kAccelerationPanelScale, kAccelerationPanelScale, kAccelerationPanelScale});
	baseCursor->Update();
	placementCursorBaseObject_ = std::move(baseCursor);
	std::unique_ptr<ColoredObject3d> arrowCursor = std::make_unique<ColoredObject3d>();
	arrowCursor->Initialize(accelerationPanelModel_.get(), kAccelerationPanelArrowColor);
	arrowCursor->SetTranslation(stage.GridToWorld({0, 0}));
	arrowCursor->SetScale({kAccelerationPanelScale, kAccelerationPanelScale, kAccelerationPanelScale});
	arrowCursor->Update();
	placementCursorArrowObject_ = std::move(arrowCursor);
	isPlacementCursorVisible_ = false;
	isPlacementCursorReflectCenterVisible_ = false;
	isPlacementCursorBaseVisible_ = false;
	isPlacementCursorArrowVisible_ = false;
}

void StageRenderer::BuildGimmickObjects(const Stage& stage) {
	gimmickObjects_.clear();
	auto addReflectGimmick = [this, &stage](const Stage::GridPosition& grid, float rotation) {
		Vector3 position = stage.GridToWorld(grid);
		position.y = kReflectGimmickHeight;
		std::unique_ptr<ColoredObject3d> frame = std::make_unique<ColoredObject3d>();
		frame->Initialize(reflectGimmickModel_.get(), kReflectFrameColor);
		std::unique_ptr<ColoredObject3d> center = std::make_unique<ColoredObject3d>();
		center->Initialize(reflectGimmickCenterModel_.get(), kReflectCenterColor);
		Object3d* objects[] = {frame.get(), center.get()};
		for (Object3d* object : objects) {
			object->SetTranslation(position);
			object->SetScale({kGimmickScale, kGimmickScale, kGimmickScale});
			object->SetRotation({0.0f, rotation, 0.0f});
			object->Update();
		}
		Vector3 centerPosition = position;
		centerPosition.y += 0.002f;
		center->SetTranslation(centerPosition);
		center->Update();
		gimmickObjects_.push_back(std::move(frame));
		gimmickObjects_.push_back(std::move(center));
	};

	for (const Stage::GridPosition& grid : stage.GetReflectSlashTiles()) {
		addReflectGimmick(grid, std::numbers::pi_v<float> * 0.25f);
	}

	for (const Stage::GridPosition& grid : stage.GetReflectBackSlashTiles()) {
		addReflectGimmick(grid, -std::numbers::pi_v<float> * 0.25f);
	}

	for (const AccelerationPanel& panel : stage.GetAccelerationPanels()) {
		// 斜めの反射ギミックと見分けられるよう、低い正方形の床として描画する。
		Vector3 position = stage.GridToWorld({panel.GetGridX(), panel.GetGridZ()});
		position.y = kAccelerationPanelHeight;
		std::unique_ptr<ColoredObject3d> baseObject = std::make_unique<ColoredObject3d>();
		baseObject->Initialize(accelerationPanelBaseModel_.get(), kAccelerationPanelBaseColor);
		std::unique_ptr<ColoredObject3d> arrowObject = std::make_unique<ColoredObject3d>();
		arrowObject->Initialize(accelerationPanelModel_.get(), kAccelerationPanelArrowColor);
		Object3d* panelObjects[] = {baseObject.get(), arrowObject.get()};
		for (Object3d* object : panelObjects) {
			object->SetTranslation(position);
			object->SetScale({kAccelerationPanelScale, kAccelerationPanelScale, kAccelerationPanelScale});
			object->SetRotation({0.0f, AccelerationPanelRotation(panel.GetDirection()), 0.0f});
			object->Update();
		}
		gimmickObjects_.push_back(std::move(baseObject));
		gimmickObjects_.push_back(std::move(arrowObject));
	}
}
