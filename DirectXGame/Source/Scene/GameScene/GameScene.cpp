#include "GameScene.h"
#include "../DifficultySelect/DifficultySelectScene.h"
#include "../Result/ResultScene.h"
#include "../Title/TitleScene.h"
#include "../../Core/Math/MathUtility.h"

#include <KamataEngine.h>
#include <algorithm>
#include <cmath>
#include <dinput.h>
#ifdef USE_IMGUI
#include <imgui.h>
#endif

#include <memory>
#include <utility>

using namespace KamataEngine;

namespace {
    // 1マス単位のステージ全体が収まるように設定した真上視点カメラの値。
    constexpr float kTopDownCameraPitch = 1.57079632679f;
	constexpr float kStageViewMargin = 1.1f;
	constexpr float kPaletteLeft = 0.0f;
	constexpr float kPaletteTop = 656.0f;
	constexpr float kPaletteWidth = 256.0f;
	constexpr float kPaletteHeight = 64.0f;
	constexpr float kPaletteItemWidth = 128.0f;

	float CalculateTopDownCameraHeight(const Stage& stage, const Camera& camera) {
		const float halfFovTangent = std::tan(camera.fovAngleY * 0.5f);
		const float heightForVerticalFit = static_cast<float>(stage.GetHeight()) / (2.0f * halfFovTangent);
		const float heightForHorizontalFit =
			static_cast<float>(stage.GetWidth()) / (2.0f * halfFovTangent * camera.aspectRatio);
		return (std::max)(heightForVerticalFit, heightForHorizontalFit) * kStageViewMargin;
	}
} // namespace

GameScene::GameScene(std::string stageFilePath) : stageFilePath_(std::move(stageFilePath)) {}

void GameScene::Initialize() {
    isEnd_ = false;
    returnTitle_ = false;
    returnStageSelect_ = false;

    if (!stage_.LoadFromCsv(stageFilePath_)) {
        stage_.LoadFromCsv("Resources\\Stages\\Easy\\Easy_01.csv");
    }

    player_.Initialize(stage_);
    dragInput_.Initialize();
    ui_.Initialize();
	Audio* audio = Audio::GetInstance();
	pullSoundHandle_ = audio->LoadWave("SE/InGame/Pull.mp3");
	firingSoundHandle_ = audio->LoadWave("SE/InGame/Firing.mp3");
    dragInput_.Reset();

    camera_.Initialize();
    camera_.translation_ = { 0.0f, CalculateTopDownCameraHeight(stage_, camera_), 0.0f };
    camera_.rotation_ = { kTopDownCameraPitch, 0.0f, 0.0f };
    camera_.UpdateMatrix();

	const std::vector<Stage::GridPosition>& placeableTiles = stage_.GetPlaceableTiles();
	placementCursor_ = placeableTiles.empty() ? Stage::GridPosition{0, 0} : placeableTiles.front();
	selectedGimmickType_ = Stage::GimmickType::ReflectSlash;
	isGimmickSelected_ = false;
	isPlacementCursorValid_ = false;
	interactionPhase_ = InteractionPhase::Placement;
	placementTool_ = PlacementTool::Place;
	maxGimmickCount_ = 3;
	InitializePlacementPalette();

	stageRenderer_.Initialize(stage_, player_.GetPosition());
	stageRenderer_.UpdatePlacementCursor(stage_, placementCursor_, selectedGimmickType_, false);
}

void GameScene::Update() {
    Input* input = Input::GetInstance();

	if (input->TriggerKey(DIK_R)) {
		ResetGame();
		return;
	}

	// ポーズボタンのクリックが背後の盤面操作にも使われないよう、UIを先に更新する。
	ui_.Update();

	// 発射前のみ、プレイヤーの開始位置調整とギミック配置が可能
	if (player_.GetState() == Player::State::Aiming && !ui_.IsPaused() && input->TriggerKey(DIK_TAB)) {
		// 配置操作と発射操作を同時に受け付けないよう、フェーズを明確に分ける。
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
				player_.MoveAimLeft(stage_);
			}
			if (input->TriggerKey(DIK_D)) {
				player_.MoveAimRight(stage_);
			}
			if (input->TriggerKey(DIK_SPACE)) {
				player_.Fire();
				Audio::GetInstance()->PlayWave(firingSoundHandle_, false, 0.9f);
			}
		}
	}

	const bool wasDragging = dragInput_.IsDragging();
    dragInput_.Update(input, camera_, player_.GetPosition(), player_.GetState() == Player::State::Aiming && interactionPhase_ == InteractionPhase::Launch && !ui_.IsPaused());
	const bool isDragging = dragInput_.IsDragging();
	if (!wasDragging && isDragging) {
		// ボタンを押している間の連続再生を避け、ドラッグ開始時に一度だけ再生する。
		Audio::GetInstance()->PlayWave(pullSoundHandle_, false, 0.75f);
	}
    Vector3 dragLaunchVelocity{};
    if (dragInput_.ConsumeLaunchVelocity(dragLaunchVelocity)) {
        player_.Fire(dragLaunchVelocity);
		Audio::GetInstance()->PlayWave(firingSoundHandle_, false, 0.9f);
    }

	const bool hasActivePlacementTool = placementTool_ == PlacementTool::Remove || isGimmickSelected_;
	stageRenderer_.UpdatePlacementCursor(stage_, placementCursor_, selectedGimmickType_, player_.GetState() == Player::State::Aiming && interactionPhase_ == InteractionPhase::Placement && hasActivePlacementTool && isPlacementCursorValid_ && !ui_.IsPaused());

    player_.Update(stage_);
    stageRenderer_.UpdatePlayer(player_.GetPosition());

    if (ui_.ShouldReturnToStageSelect()) {
        returnStageSelect_ = true;
        isEnd_ = true;
        return;
    }

    if (ui_.ShouldReturnToTitle()) {
        returnTitle_ = true;
        isEnd_ = true;
        return;
    }

	if (player_.IsClear()) {
		// この終了フラグをSceneManagerが検知し、ResultSceneへ切り替える。
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
	ImGui::SetNextWindowSize(ImVec2(380.0f, 420.0f), ImGuiCond_FirstUseEver);
	ImGui::Begin("GameScene");
	ImGui::Text("3D One-Step Puzzle");
	ImGui::Text("Stage: %s", stageFilePath_.c_str());
	ImGui::Text("Performance: %.1f FPS (%.2f ms)", ImGui::GetIO().Framerate,
		ImGui::GetIO().Framerate > 0.0f ? 1000.0f / ImGui::GetIO().Framerate : 0.0f);
	ImGui::Separator();
	ImGui::Text("Debug Controls");
	int phase = static_cast<int>(interactionPhase_);
	if (ImGui::RadioButton("Placement", phase == static_cast<int>(InteractionPhase::Placement))) {
		interactionPhase_ = InteractionPhase::Placement;
		dragInput_.Reset();
	}
	ImGui::SameLine();
	if (ImGui::RadioButton("Launch", phase == static_cast<int>(InteractionPhase::Launch))) {
		interactionPhase_ = InteractionPhase::Launch;
		isPlacementCursorValid_ = false;
		dragInput_.Reset();
	}

	int gimmickType = static_cast<int>(selectedGimmickType_);
	if (ImGui::RadioButton("Reflect /", gimmickType == static_cast<int>(Stage::GimmickType::ReflectSlash))) {
		selectedGimmickType_ = Stage::GimmickType::ReflectSlash;
		placementTool_ = PlacementTool::Place;
		isGimmickSelected_ = true;
	}
	ImGui::SameLine();
	if (ImGui::RadioButton("Reflect \\", gimmickType == static_cast<int>(Stage::GimmickType::ReflectBackSlash))) {
		selectedGimmickType_ = Stage::GimmickType::ReflectBackSlash;
		placementTool_ = PlacementTool::Place;
		isGimmickSelected_ = true;
	}
	ImGui::SameLine();
	if (ImGui::RadioButton("AccelerationPanel", gimmickType == static_cast<int>(Stage::GimmickType::AccelerationPanel))) {
		selectedGimmickType_ = Stage::GimmickType::AccelerationPanel;
		placementTool_ = PlacementTool::Place;
		isGimmickSelected_ = true;
	}
	ImGui::SliderInt("Gimmick limit", &maxGimmickCount_, 0, 20);
	if (ImGui::Button("Clear gimmicks")) {
		ClearPlacedGimmicks();
	}
	ImGui::SameLine();
	if (ImGui::Button("Reset game")) {
		ResetGame();
	}
	ImGui::Separator();
	ImGui::Text("Cursor: X %d / Z %d", placementCursor_.x, placementCursor_.z);
	const char* selectedName = selectedGimmickType_ == Stage::GimmickType::ReflectSlash ? "/" :
		selectedGimmickType_ == Stage::GimmickType::ReflectBackSlash ? "\\" : "AccelerationPanel";
	ImGui::Text("Selected: %s", selectedName);
	ImGui::Text("Gimmicks: %d / %d", stage_.GetPlacedGimmickCount(), maxGimmickCount_);
	ImGui::Text("Acceleration panels: %d", stage_.GetAccelerationPanelCount());
	ImGui::Text("Phase: %s", interactionPhase_ == InteractionPhase::Placement ? "PLACEMENT" : "LAUNCH");
	ImGui::Text("Drag power: %.0f%%", dragInput_.GetPowerRate() * 100.0f);
	ImGui::Text("State: %s", player_.GetState() == Player::State::Aiming ? "Aiming" : player_.GetState() == Player::State::Moving ? "Moving" : "Stopped");
	if (player_.IsFailed()) {
		ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.2f, 1.0f), "FAILED: press R or Reset game");
	}
	ImGui::Separator();
	ImGui::TextDisabled("TAB: phase  A/D: aim  RMB: rotate  R: reset");
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

    return std::make_unique<ResultScene>(stage_.GetPlacedGimmickCount(), stageFilePath_);
}

SceneName GameScene::GetSceneName() const { return SceneName::InGame; }

void GameScene::UpdateGimmickPlacement() {
	Input* input = Input::GetInstance();

#ifdef USE_IMGUI
	// デバッグウィンドウのクリックが背後のステージ編集にも使われることを防ぐ。
	if (ImGui::GetIO().WantCaptureMouse) {
		isPlacementCursorValid_ = false;
		return;
	}
#endif

	if (input->IsTriggerMouse(0) && IsMouseOverPlacementPalette()) {
		const float mouseX = input->GetMousePosition().x;
		placementTool_ = mouseX < kPaletteLeft + kPaletteItemWidth ? PlacementTool::Place : PlacementTool::Remove;
		isGimmickSelected_ = placementTool_ == PlacementTool::Place;
		isPlacementCursorValid_ = false;
		return;
	}

	if (placementTool_ == PlacementTool::Place && !isGimmickSelected_) {
		return;
	}

	isPlacementCursorValid_ = UpdatePlacementCursorFromMouse();

	if (placementTool_ == PlacementTool::Place && input->IsTriggerMouse(1)) {
		// 右クリックするたびに、配置可能なギミックを順番に切り替える。
		if (selectedGimmickType_ == Stage::GimmickType::ReflectSlash) {
			selectedGimmickType_ = Stage::GimmickType::ReflectBackSlash;
		} else if (selectedGimmickType_ == Stage::GimmickType::ReflectBackSlash) {
			selectedGimmickType_ = Stage::GimmickType::AccelerationPanel;
		} else {
			selectedGimmickType_ = Stage::GimmickType::ReflectSlash;
		}
	}

	if (placementTool_ == PlacementTool::Remove && input->IsTriggerMouse(0) && isPlacementCursorValid_) {
		if (stage_.RemoveGimmick(placementCursor_)) {
			stageRenderer_.RebuildGimmicks(stage_);
		}
		return;
	}

	if (placementTool_ == PlacementTool::Place && input->IsTriggerMouse(0) && isPlacementCursorValid_) {
		// 共通の配置上限に達していても、配置済みマスの置き換えは許可する。
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
	if (placementTool_ == PlacementTool::Remove && stage_.GetGimmick(hoveredGrid) == Stage::GimmickType::None) {
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
	removeIconSpriteA_.reset(Sprite::Create(iconTexture, {192.0f, kPaletteTop + kPaletteHeight * 0.5f}, {0.85f, 0.18f, 0.18f, 1.0f}, {0.5f, 0.5f}));
	removeIconSpriteB_.reset(Sprite::Create(iconTexture, {192.0f, kPaletteTop + kPaletteHeight * 0.5f}, {0.85f, 0.18f, 0.18f, 1.0f}, {0.5f, 0.5f}));
	removeIconSpriteA_->SetSize({10.0f, 48.0f});
	removeIconSpriteB_->SetSize({10.0f, 48.0f});
	removeIconSpriteA_->SetRotation(0.78539816339f);
	removeIconSpriteB_->SetRotation(-0.78539816339f);
}

void GameScene::DrawPlacementPalette() {
	if (!placementPaletteSprite_ || !placementIconSprite_ || !removeIconSpriteA_ || !removeIconSpriteB_) {
		return;
	}

	Sprite::PreDraw(DirectXCommon::GetInstance()->GetCommandList());
	placementPaletteSprite_->Draw();
	placementIconSprite_->SetRotation(selectedGimmickType_ == Stage::GimmickType::ReflectSlash ? -0.78539816339f :
		selectedGimmickType_ == Stage::GimmickType::ReflectBackSlash ? 0.78539816339f : 0.0f);
	const bool isActive = interactionPhase_ == InteractionPhase::Placement && placementTool_ == PlacementTool::Place && isGimmickSelected_;
	const Vector4 selectedColor = selectedGimmickType_ == Stage::GimmickType::AccelerationPanel
		? Vector4{0.2f, 1.0f, 0.35f, 1.0f} : Vector4{1.0f, 0.72f, 0.2f, 1.0f};
	placementIconSprite_->SetColor(isActive ? selectedColor : Vector4{0.55f, 0.72f, 0.78f, 1.0f});
	placementIconSprite_->Draw();
	const bool isRemoveActive = interactionPhase_ == InteractionPhase::Placement && placementTool_ == PlacementTool::Remove;
	const Vector4 removeColor = isRemoveActive ? Vector4{1.0f, 0.45f, 0.15f, 1.0f} : Vector4{0.85f, 0.18f, 0.18f, 1.0f};
	removeIconSpriteA_->SetColor(removeColor);
	removeIconSpriteB_->SetColor(removeColor);
	removeIconSpriteA_->Draw();
	removeIconSpriteB_->Draw();
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

void GameScene::ResetGame() {
	stage_.ClearGimmicks();
	player_.Reset(stage_);
	dragInput_.Reset();
	const std::vector<Stage::GridPosition>& placeableTiles = stage_.GetPlaceableTiles();
	placementCursor_ = placeableTiles.empty() ? Stage::GridPosition{0, 0} : placeableTiles.front();
	stageRenderer_.Initialize(stage_, player_.GetPosition());
	isGimmickSelected_ = false;
	isPlacementCursorValid_ = false;
	interactionPhase_ = InteractionPhase::Placement;
	placementTool_ = PlacementTool::Place;
	stageRenderer_.UpdatePlacementCursor(stage_, placementCursor_, selectedGimmickType_, false);
}
