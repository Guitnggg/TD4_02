#include "GameScene.h"
#include "../DifficultySelect/DifficultySelectScene.h"
#include "../Result/ResultScene.h"
#include "../Title/TitleScene.h"
#include "../../Core/Math/MathUtility.h"

#include <KamataEngine.h>
#include <algorithm>
#include <dinput.h>
#ifdef USE_IMGUI
#include <imgui.h>
#endif

#include <memory>
#include <utility>

using namespace KamataEngine;

namespace {
    constexpr float kTopDownCameraHeight = 18.0f;
    constexpr float kTopDownCameraPitch = 1.57079632679f;
	constexpr float kPaletteLeft = 0.0f;
	constexpr float kPaletteTop = 656.0f;
	constexpr float kPaletteWidth = 256.0f;
	constexpr float kPaletteHeight = 64.0f;
} // namespace

GameScene::GameScene(std::string stageFilePath) : stageFilePath_(std::move(stageFilePath)) {}

void GameScene::Initialize() {
    isEnd_ = false;
    returnTitle_ = false;
    returnStageSelect_ = false;

    if (!stage_.LoadFromCsv(stageFilePath_)) {
        stage_.LoadFromCsv("Resources\\Stages\\Eazy\\Eazy_01.csv");
    }

    player_.Initialize(stage_);
    dragInput_.Initialize();
    ui_.Initialize();
    dragInput_.Reset();

    camera_.Initialize();
    camera_.translation_ = { 0.0f, kTopDownCameraHeight, 0.0f };
    camera_.rotation_ = { kTopDownCameraPitch, 0.0f, 0.0f };
    camera_.UpdateMatrix();

	const std::vector<Stage::GridPosition>& placeableTiles = stage_.GetPlaceableTiles();
	placementCursor_ = placeableTiles.empty() ? Stage::GridPosition{0, 0} : placeableTiles.front();
	selectedGimmickType_ = Stage::GimmickType::ReflectSlash;
	isGimmickSelected_ = false;
	isPlacementCursorValid_ = false;
	interactionPhase_ = InteractionPhase::Placement;
	maxGimmickCount_ = 3;
	InitializePlacementPalette();

	stageRenderer_.Initialize(stage_, player_.GetPosition());
	stageRenderer_.UpdatePlacementCursor(stage_, placementCursor_, selectedGimmickType_, false);
}

void GameScene::Update() {
    Input* input = Input::GetInstance();

	if (input->TriggerKey(DIK_R)) {
		stage_.ClearGimmicks();
		player_.Reset(stage_);
		dragInput_.Reset();
		const std::vector<Stage::GridPosition>& placeableTiles = stage_.GetPlaceableTiles();
		placementCursor_ = placeableTiles.empty() ? Stage::GridPosition{0, 0} : placeableTiles.front();
		stageRenderer_.Initialize(stage_, player_.GetPosition());
		isGimmickSelected_ = false;
		isPlacementCursorValid_ = false;
		interactionPhase_ = InteractionPhase::Placement;
		stageRenderer_.UpdatePlacementCursor(stage_, placementCursor_, selectedGimmickType_, false);
		return;
	}

	// 発射前のみ、プレイヤーの開始位置調整とギミック配置が可能
	if (player_.GetState() == Player::State::Aiming && !ui_.IsPaused() && input->TriggerKey(DIK_TAB)) {
		interactionPhase_ = interactionPhase_ == InteractionPhase::Placement ? InteractionPhase::Launch : InteractionPhase::Placement;
		isPlacementCursorValid_ = false;
		dragInput_.Reset();
	}

	// 配置入力と発射入力はフェーズごとに完全に分離する。
	if (player_.GetState() == Player::State::Aiming) {
		if (!ui_.IsPaused() && interactionPhase_ == InteractionPhase::Placement) {
			UpdateGimmickPlacement();
		}
		if (!ui_.IsPaused() && interactionPhase_ == InteractionPhase::Launch) {
			if (input->TriggerKey(DIK_A)) {
				player_.MoveAimRight(stage_);
			}
			if (input->TriggerKey(DIK_D)) {
				player_.MoveAimLeft(stage_);
			}
			if (input->TriggerKey(DIK_SPACE)) {
				player_.Fire();
			}
		}
	}

    dragInput_.Update(input, camera_, player_.GetPosition(), player_.GetState() == Player::State::Aiming && interactionPhase_ == InteractionPhase::Launch && !ui_.IsPaused());
    Vector3 dragLaunchVelocity{};
    if (dragInput_.ConsumeLaunchVelocity(dragLaunchVelocity)) {
        player_.Fire(dragLaunchVelocity);
    }

    ui_.Update();
	stageRenderer_.UpdatePlacementCursor(stage_, placementCursor_, selectedGimmickType_, player_.GetState() == Player::State::Aiming && interactionPhase_ == InteractionPhase::Placement && isGimmickSelected_ && isPlacementCursorValid_ && !ui_.IsPaused());

    player_.Update(stage_);
    stageRenderer_.UpdatePlayer(player_.GetPosition());

    if (ui_.IsStageselect()) {
        returnStageSelect_ = true;
        isEnd_ = true;
        return;
    }

    if (ui_.IsProgress()) {
        returnTitle_ = true;
        isEnd_ = true;
        return;
    }

    if (player_.IsClear()) {
        returnTitle_ = false;
        isEnd_ = true;
    }
}

void GameScene::Draw() {
    stageRenderer_.Draw(camera_);
    stageRenderer_.DrawGuide(stage_, camera_);
    dragInput_.Draw(camera_);
	DrawPlacementPalette();
    ui_.Draw();

#ifdef USE_IMGUI
	ImGui::SetNextWindowPos(ImVec2(520.0f, 240.0f), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(360.0f, 260.0f), ImGuiCond_FirstUseEver);
	ImGui::Begin("GameScene");
	ImGui::Text("3D One-Step Puzzle");
	ImGui::Text("Stage: %s", stageFilePath_.c_str());
	ImGui::Separator();
	ImGui::Text("TAB: switch placement / launch phase");
	ImGui::Text("A/D: adjust launch position");
	ImGui::Text("Left click palette: select gimmick");
	ImGui::Text("Right click: rotate gimmick");
	ImGui::Text("Left click board: place gimmick");
	ImGui::Text("X: remove gimmick");
	ImGui::Text("C: clear gimmicks");
	ImGui::Text("Drag player or SPACE: launch");
	ImGui::Text("R: reset");
	ImGui::Separator();
	ImGui::Text("Cursor: X %d / Z %d", placementCursor_.x, placementCursor_.z);
	ImGui::Text("Selected: %s", selectedGimmickType_ == Stage::GimmickType::ReflectSlash ? "/" : "\\");
	ImGui::Text("Gimmicks: %d / %d", stage_.GetPlacedGimmickCount(), maxGimmickCount_);
	ImGui::Text("Phase: %s", interactionPhase_ == InteractionPhase::Placement ? "PLACEMENT" : "LAUNCH");
	ImGui::Text("Drag power: %.0f%%", dragInput_.GetPowerRate() * 100.0f);
	ImGui::Text("State: %s", player_.GetState() == Player::State::Aiming ? "Aiming" : player_.GetState() == Player::State::Moving ? "Moving" : "Stopped");
	if (player_.IsFailed()) {
		ImGui::Text("FAILED: press R");
	}
	ImGui::End();
#endif
}

bool GameScene::IsEnd() const { return isEnd_; }

std::unique_ptr<IScene> GameScene::NextScene() const {
    if (returnTitle_) {
        return std::make_unique<TitleScene>();
    }
    if (returnStageSelect_) {
        return std::make_unique<DifficultySelectScene>();
    }

    return std::make_unique<ResultScene>();
}

SceneName GameScene::GetSceneName() const { return SceneName::InGame; }

void GameScene::UpdateGimmickPlacement() {
	Input* input = Input::GetInstance();

	if (input->IsTriggerMouse(0) && IsMouseOverPlacementPalette()) {
		isGimmickSelected_ = true;
		isPlacementCursorValid_ = false;
		return;
	}

	if (!isGimmickSelected_) {
		return;
	}

	isPlacementCursorValid_ = UpdatePlacementCursorFromMouse();

	if (input->IsTriggerMouse(1)) {
		selectedGimmickType_ = selectedGimmickType_ == Stage::GimmickType::ReflectSlash
			? Stage::GimmickType::ReflectBackSlash
			: Stage::GimmickType::ReflectSlash;
	}

	if (input->IsTriggerMouse(0) && isPlacementCursorValid_) {
		const bool alreadyPlaced = stage_.GetGimmick(placementCursor_) != Stage::GimmickType::None;
		if (alreadyPlaced || stage_.GetPlacedGimmickCount() < maxGimmickCount_) {
			if (stage_.PlaceGimmick(placementCursor_, selectedGimmickType_)) {
				stageRenderer_.RebuildGimmicks(stage_);
			}
		}
	}

	if (input->TriggerKey(DIK_X)) {
		if (stage_.RemoveGimmick(placementCursor_)) {
			stageRenderer_.RebuildGimmicks(stage_);
		}
	}

	if (input->TriggerKey(DIK_C)) {
		ClearPlacedGimmicks();
	}
}

bool GameScene::UpdatePlacementCursorFromMouse() {
	if (IsMouseOverPlacementPalette()) {
		return false;
	}

	const Stage::GridPosition hoveredGrid = stage_.WorldToGrid(MouseToWorldOnStage());
	if (!stage_.CanPlaceGimmick(hoveredGrid)) {
		return false;
	}

	placementCursor_ = hoveredGrid;
	return true;
}

Vector3 GameScene::MouseToWorldOnStage() const {
	const Vector2& mousePosition = Input::GetInstance()->GetMousePosition();
	const float ndcX = (mousePosition.x / static_cast<float>(WinApp::kWindowWidth)) * 2.0f - 1.0f;
	const float ndcY = -((mousePosition.y / static_cast<float>(WinApp::kWindowHeight)) * 2.0f - 1.0f);
	const Matrix4x4 inverseViewProjection = MyMath::Inverse(MyMath::Multiply(camera_.matView, camera_.matProjection));
	const Vector3 nearPoint = MyMath::Transform({ndcX, ndcY, 0.0f}, inverseViewProjection);
	const Vector3 farPoint = MyMath::Transform({ndcX, ndcY, 1.0f}, inverseViewProjection);
	const Vector3 ray = MyMath::Subtract(farPoint, nearPoint);
	if (std::abs(ray.y) <= 0.0001f) {
		return nearPoint;
	}
	return MyMath::Add(nearPoint, MyMath::Multiply(ray, -nearPoint.y / ray.y));
}

bool GameScene::IsMouseOverPlacementPalette() const {
	const Vector2& mouse = Input::GetInstance()->GetMousePosition();
	return mouse.x >= kPaletteLeft && mouse.x <= kPaletteLeft + kPaletteWidth &&
	       mouse.y >= kPaletteTop && mouse.y <= kPaletteTop + kPaletteHeight;
}

void GameScene::InitializePlacementPalette() {
	const uint32_t paletteTexture = TextureManager::Load("Placement.png");
	const uint32_t iconTexture = TextureManager::Load("white1x1.png");
	placementPaletteSprite_.reset(Sprite::Create(paletteTexture, {kPaletteLeft, kPaletteTop}));
	placementIconSprite_.reset(Sprite::Create(iconTexture, {64.0f, kPaletteTop + kPaletteHeight * 0.5f}, {0.55f, 0.72f, 0.78f, 1.0f}, {0.5f, 0.5f}));
	placementIconSprite_->SetSize({18.0f, 54.0f});
}

void GameScene::DrawPlacementPalette() {
	if (!placementPaletteSprite_ || !placementIconSprite_) {
		return;
	}

	Sprite::PreDraw(DirectXCommon::GetInstance()->GetCommandList());
	placementPaletteSprite_->Draw();
	placementIconSprite_->SetRotation(selectedGimmickType_ == Stage::GimmickType::ReflectSlash ? -0.78539816339f : 0.78539816339f);
	const bool isActive = interactionPhase_ == InteractionPhase::Placement && isGimmickSelected_;
	placementIconSprite_->SetColor(isActive ? Vector4{1.0f, 0.72f, 0.2f, 1.0f} : Vector4{0.55f, 0.72f, 0.78f, 1.0f});
	placementIconSprite_->Draw();
	Sprite::PostDraw();
}

void GameScene::ClampPlacementCursor() {
	placementCursor_.x = std::clamp(placementCursor_.x, 0, stage_.GetWidth() - 1);
	placementCursor_.z = std::clamp(placementCursor_.z, 0, stage_.GetHeight() - 1);
}

void GameScene::ClearPlacedGimmicks() {
	stage_.ClearGimmicks();
	stageRenderer_.RebuildGimmicks(stage_);
}
