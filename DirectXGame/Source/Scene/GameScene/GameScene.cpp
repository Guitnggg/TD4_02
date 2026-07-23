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
#include <array>
#include <utility>

using namespace KamataEngine;

namespace {
    // 1マス単位のステージ全体が収まるように設定した真上視点カメラの値。
    constexpr float kTopDownCameraPitch = 1.57079632679f;
	constexpr float kStageViewMargin = 1.1f;
	constexpr float kPaletteLeft = 0.0f;
	constexpr float kPaletteTop = 656.0f;
	constexpr float kPaletteWidth = 384.0f;
	constexpr float kPaletteHeight = 64.0f;
	constexpr float kPaletteItemWidth = 128.0f;
	constexpr float kPhaseChangeLeft = 896.0f;
	constexpr float kPhaseChangeTop = 616.0f;
	constexpr float kPhaseChangeWidth = 288.0f;
	constexpr float kPhaseChangeHeight = 96.0f;
	constexpr float kPhaseChangeHoverScale = 1.06f;
	constexpr float kFailedAnimationDuration = 1.5f;
	constexpr float kResetCenterX = 1080.0f;
	constexpr float kResetCenterY = 540.0f;
	constexpr float kResetWidth = 225.0f;
	constexpr float kResetHeight = 63.0f;
	constexpr float kResetHoverScale = 1.06f;
	constexpr float kFixedDeltaTime = 1.0f / 60.0f;
	constexpr uint32_t kReflectionParticleCapacity = 512;
	constexpr uint32_t kReflectionSparkCount = 48;
	constexpr uint32_t kReflectionGlowCount = 12;
	constexpr uint32_t kMovementParticleCapacity = 768;
	constexpr float kDustEmissionInterval = 0.045f;
	constexpr float kTwoPi = 6.28318530718f;
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
	phaseChangeSoundHandle_ = audio->LoadWave("SE/Dicision.mp3");
	reflectionSoundHandle_ = audio->LoadWave("SE/InGame/ReflectSE.mp3");
	rotationSoundHandle_ = audio->LoadWave("SE/InGame/rotateSE.mp3");
	placementSoundHandle_ = audio->LoadWave("SE/InGame/placeSE.mp3");
	deletionSoundHandle_ = audio->LoadWave("SE/InGame/deleteSE.mp3");
	accelerationSoundHandle_ = audio->LoadWave("SE/InGame/accelerationSE.mp3");
    dragInput_.Reset();

	camera_.Initialize();
	camera_.translation_ = { 0.0f, CalculateTopDownCameraHeight(stage_, camera_), 0.0f };
    camera_.rotation_ = { kTopDownCameraPitch, 0.0f, 0.0f };
	camera_.UpdateMatrix();
	reflectionParticles_.Initialize(kReflectionParticleCapacity);
	movementParticles_.Initialize(kMovementParticleCapacity);

	const std::vector<Stage::GridPosition>& placeableTiles = stage_.GetPlaceableTiles();
	placementCursor_ = placeableTiles.empty() ? Stage::GridPosition{0, 0} : placeableTiles.front();
	selectedGimmickType_ = Stage::GimmickType::ReflectSlash;
	isGimmickSelected_ = false;
	isPlacementCursorValid_ = false;
	interactionPhase_ = InteractionPhase::Placement;
	placementTool_ = PlacementTool::Place;
	maxGimmickCount_ = 3;
	InitializePlacementPalette();
	InitializeInstructionUI();
	backgroundSprite_.reset(Sprite::Create(TextureManager::Load("UI/GameBackground.png"), {0.0f, 0.0f}));
	backgroundSprite_->SetSize({static_cast<float>(WinApp::kWindowWidth), static_cast<float>(WinApp::kWindowHeight)});
	failedSprite_.reset(Sprite::Create(TextureManager::Load("UI/Failed.png"), {0.0f, -static_cast<float>(WinApp::kWindowHeight)}));
	failedSprite_->SetSize({static_cast<float>(WinApp::kWindowWidth), static_cast<float>(WinApp::kWindowHeight)});
	failedBackdropSprite_.reset(Sprite::Create(TextureManager::Load("white1x1.png"), {0.0f, 0.0f}, Vector4{0.0f, 0.0f, 0.0f, 0.68f}));
	failedBackdropSprite_->SetSize({static_cast<float>(WinApp::kWindowWidth), static_cast<float>(WinApp::kWindowHeight)});
	resetSprite_.reset(Sprite::Create(TextureManager::Load("UI/Retry.png"), {kResetCenterX, kResetCenterY}, Vector4{1.0f, 1.0f, 1.0f, 1.0f}, {0.5f, 0.5f}));
	resetSprite_->SetSize({kResetWidth, kResetHeight});
	failedAnimationTimer_ = 0.0f;
	isFailedSpriteVisible_ = false;
	wasPlayerFailed_ = false;

	stageRenderer_.Initialize(stage_, player_.GetPosition());
	stageRenderer_.UpdatePlacementCursor(stage_, placementCursor_, selectedGimmickType_, false);
}

void GameScene::Update() {
    Input* input = Input::GetInstance();

	if (input->TriggerKey(DIK_R)) {
		ResetGame();
		return;
	}
	if (!ui_.IsPaused() && input->IsTriggerMouse(0) && IsMouseOverResetButton()) {
		Audio::GetInstance()->PlayWave(phaseChangeSoundHandle_, false, 0.7f);
		ResetGame();
		return;
	}

	// ポーズボタンのクリックが背後の盤面操作にも使われないよう、UIを先に更新する。
	ui_.Update();

	// 発射前のみ、プレイヤーの開始位置調整とギミック配置が可能
	const bool phaseChangeClicked = !ui_.IsPaused() && input->IsTriggerMouse(0) && IsMouseOverPhaseChangeButton();
	if (player_.GetState() == Player::State::Aiming && !ui_.IsPaused() &&
	    (input->TriggerKey(DIK_TAB) || phaseChangeClicked)) {
		// 配置操作と発射操作を同時に受け付けないよう、フェーズを明確に分ける。
		interactionPhase_ = interactionPhase_ == InteractionPhase::Placement ? InteractionPhase::Launch : InteractionPhase::Placement;
		Audio::GetInstance()->PlayWave(phaseChangeSoundHandle_, false, 0.7f);
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
    dragInput_.Update(input, camera_, player_.GetPosition(), player_.GetState() == Player::State::Aiming && interactionPhase_ == InteractionPhase::Launch && !ui_.IsPaused() && !IsMouseOverPhaseChangeButton());
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
	stageRenderer_.UpdatePlacementCursor(stage_, placementCursor_, selectedGimmickType_, player_.GetState() == Player::State::Aiming && interactionPhase_ == InteractionPhase::Placement && hasActivePlacementTool && isPlacementCursorValid_ && !ui_.IsPaused(), selectedPanelDirection_);

    player_.Update(stage_);
	if (player_.GetState() == Player::State::Moving && !ui_.IsPaused()) {
		dustEmissionTimer_ += kFixedDeltaTime;
		while (dustEmissionTimer_ >= kDustEmissionInterval) {
			dustEmissionTimer_ -= kDustEmissionInterval;
			EmitDustTrail();
		}
	} else {
		dustEmissionTimer_ = 0.0f;
	}
	if (player_.ConsumeReflectionEvent()) {
		Audio::GetInstance()->PlayWave(reflectionSoundHandle_, false, 0.85f);
		EmitReflectionParticles();
	}
	reflectionParticles_.Update(kFixedDeltaTime);
	if (player_.ConsumeAccelerationEvent()) {
		Audio::GetInstance()->PlayWave(accelerationSoundHandle_, false, 0.85f);
		EmitAccelerationParticles();
	}
	movementParticles_.Update(kFixedDeltaTime);
    stageRenderer_.UpdatePlayer(player_.GetPosition());

	const bool isPlayerFailed = player_.IsFailed();
	if (isPlayerFailed && !wasPlayerFailed_) {
		failedAnimationTimer_ = 0.0f;
		isFailedSpriteVisible_ = true;
	}
	if (isFailedSpriteVisible_ && failedSprite_ && !ui_.IsPaused()) {
		failedAnimationTimer_ = (std::min)(failedAnimationTimer_ + kFixedDeltaTime, kFailedAnimationDuration);
		const float progress = failedAnimationTimer_ / kFailedAnimationDuration;
		const float easedProgress = MyMath::EaseOutBounce(progress);
		failedSprite_->SetPosition({0.0f, MyMath::Lerp(-static_cast<float>(WinApp::kWindowHeight), 0.0f, easedProgress)});
	}
	wasPlayerFailed_ = isPlayerFailed;

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
	if (backgroundSprite_) {
		Sprite::PreDraw(DirectXCommon::GetInstance()->GetCommandList());
		backgroundSprite_->Draw();
		Sprite::PostDraw();
		DirectXCommon::GetInstance()->ClearDepthBuffer();
	}
    stageRenderer_.Draw(camera_);
    stageRenderer_.DrawGuide(stage_, camera_);
	reflectionParticles_.Draw(camera_);
	movementParticles_.Draw(camera_);
    dragInput_.Draw(camera_);
	DrawPlacementPalette();
	DrawInstructionUI();
	if (isFailedSpriteVisible_ && failedSprite_) {
		Sprite::PreDraw(DirectXCommon::GetInstance()->GetCommandList());
		if (failedBackdropSprite_) {
			failedBackdropSprite_->Draw();
		}
		failedSprite_->Draw();
		Sprite::PostDraw();
	}
	if (resetSprite_) {
		const bool isResetHovered = !ui_.IsPaused() && IsMouseOverResetButton();
		resetSprite_->SetSize(isResetHovered ? Vector2{kResetWidth * kResetHoverScale, kResetHeight * kResetHoverScale} : Vector2{kResetWidth, kResetHeight});
		resetSprite_->SetColor(isResetHovered ? Vector4{1.0f, 0.9f, 0.45f, 1.0f} : Vector4{1.0f, 1.0f, 1.0f, 1.0f});
		Sprite::PreDraw(DirectXCommon::GetInstance()->GetCommandList());
		resetSprite_->Draw();
		Sprite::PostDraw();
	}
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
		const int item = static_cast<int>((input->GetMousePosition().x - kPaletteLeft) / kPaletteItemWidth);
		placementTool_ = item == 2 ? PlacementTool::Remove : PlacementTool::Place;
		if (item == 0 && selectedGimmickType_ == Stage::GimmickType::AccelerationPanel) {
			selectedGimmickType_ = Stage::GimmickType::ReflectSlash;
		} else if (item == 1) {
			selectedGimmickType_ = Stage::GimmickType::AccelerationPanel;
		}
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
			selectedGimmickType_ = Stage::GimmickType::ReflectSlash;
		} else {
			selectedPanelDirection_ = static_cast<AccelerationPanel::Direction>((static_cast<int>(selectedPanelDirection_) + 1) % 4);
		}
		Audio::GetInstance()->PlayWave(rotationSoundHandle_, false, 0.8f);
	}

	if (placementTool_ == PlacementTool::Remove && input->IsTriggerMouse(0) && isPlacementCursorValid_) {
		if (stage_.RemoveGimmick(placementCursor_)) {
			stageRenderer_.RebuildGimmicks(stage_);
			Audio::GetInstance()->PlayWave(deletionSoundHandle_, false, 0.8f);
		}
		return;
	}

	if (placementTool_ == PlacementTool::Place && input->IsTriggerMouse(0) && isPlacementCursorValid_) {
		// 共通の配置上限に達していても、配置済みマスの置き換えは許可する。
		const bool alreadyPlaced = stage_.GetGimmick(placementCursor_) != Stage::GimmickType::None;
		if (alreadyPlaced || stage_.GetPlacedGimmickCount() < maxGimmickCount_) {
			if (stage_.PlaceGimmick(placementCursor_, selectedGimmickType_, selectedPanelDirection_)) {
				stageRenderer_.RebuildGimmicks(stage_);
				Audio::GetInstance()->PlayWave(placementSoundHandle_, false, 0.8f);
			}
		}
	}

	if (input->TriggerKey(DIK_X)) {
		if (stage_.RemoveGimmick(placementCursor_)) {
			stageRenderer_.RebuildGimmicks(stage_);
			Audio::GetInstance()->PlayWave(deletionSoundHandle_, false, 0.8f);
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

bool GameScene::IsMouseOverPhaseChangeButton() const {
	const Vector2& mouse = Input::GetInstance()->GetMousePosition();
	return mouse.x >= kPhaseChangeLeft && mouse.x <= kPhaseChangeLeft + kPhaseChangeWidth &&
	       mouse.y >= kPhaseChangeTop && mouse.y <= kPhaseChangeTop + kPhaseChangeHeight;
}

int GameScene::GetHoveredPaletteItem() const {
	if (!IsMouseOverPlacementPalette()) {
		return -1;
	}
	const float localX = Input::GetInstance()->GetMousePosition().x - kPaletteLeft;
	return std::clamp(static_cast<int>(localX / kPaletteItemWidth), 0, 2);
}

void GameScene::InitializePlacementPalette() {
	const uint32_t paletteTexture = TextureManager::Load("Placement.png");
	const uint32_t iconTexture = TextureManager::Load("white1x1.png");
	const uint32_t reflectIconTexture = TextureManager::Load("UI/ReflectModelIcon.png");
	const uint32_t accelerationIconTexture = TextureManager::Load("UI/AccelerationModelIcon.png");
	placementPaletteSprite_.reset(Sprite::Create(paletteTexture, {kPaletteLeft, kPaletteTop}));
	placementPaletteSprite_->SetSize({kPaletteWidth, kPaletteHeight});
	placementIconSprite_.reset(Sprite::Create(reflectIconTexture, {64.0f, kPaletteTop + kPaletteHeight * 0.5f}, Vector4{1.0f, 1.0f, 1.0f, 1.0f}, {0.5f, 0.5f}));
	placementIconSprite_->SetSize({58.0f, 58.0f});
	accelerationIconShaftSprite_.reset(Sprite::Create(accelerationIconTexture, {192.0f, kPaletteTop + kPaletteHeight * 0.5f}, Vector4{1.0f, 1.0f, 1.0f, 1.0f}, {0.5f, 0.5f}));
	accelerationIconShaftSprite_->SetSize({58.0f, 58.0f});
	removeIconSpriteA_.reset(Sprite::Create(iconTexture, {320.0f, kPaletteTop + kPaletteHeight * 0.5f}, {0.85f, 0.18f, 0.18f, 1.0f}, {0.5f, 0.5f}));
	removeIconSpriteB_.reset(Sprite::Create(iconTexture, {320.0f, kPaletteTop + kPaletteHeight * 0.5f}, {0.85f, 0.18f, 0.18f, 1.0f}, {0.5f, 0.5f}));
	removeIconSpriteA_->SetSize({10.0f, 48.0f});
	removeIconSpriteB_->SetSize({10.0f, 48.0f});
	removeIconSpriteA_->SetRotation(0.78539816339f);
	removeIconSpriteB_->SetRotation(-0.78539816339f);

	const std::array<const char*, 3> textTextures = {
		"UI/Reflect.png", "UI/Acceleration.png", "UI/Delete.png"};
	for (size_t i = 0; i < paletteTextSprites_.size(); ++i) {
		paletteTextSprites_[i].reset(Sprite::Create(TextureManager::Load(textTextures[i]), {0.0f, 0.0f}));
		paletteTextSprites_[i]->SetSize({120.0f, 34.0f});
	}
}

void GameScene::DrawPlacementPalette() {
	if (!placementPaletteSprite_ || !placementIconSprite_ || !accelerationIconShaftSprite_ || !removeIconSpriteA_ || !removeIconSpriteB_) {
		return;
	}

	Sprite::PreDraw(DirectXCommon::GetInstance()->GetCommandList());
	placementPaletteSprite_->Draw();
	const int hoveredItem = GetHoveredPaletteItem();
	const bool reflectActive = interactionPhase_ == InteractionPhase::Placement && placementTool_ == PlacementTool::Place && isGimmickSelected_ && selectedGimmickType_ != Stage::GimmickType::AccelerationPanel;
	placementIconSprite_->SetSize(hoveredItem == 0 ? Vector2{64.0f, 64.0f} : Vector2{58.0f, 58.0f});
	placementIconSprite_->SetColor(hoveredItem == 0 ? Vector4{1.0f, 0.9f, 0.45f, 1.0f} :
		reflectActive ? Vector4{1.0f, 1.0f, 1.0f, 1.0f} : Vector4{0.65f, 0.65f, 0.65f, 1.0f});
	placementIconSprite_->Draw();
	const bool accelerationActive = interactionPhase_ == InteractionPhase::Placement && placementTool_ == PlacementTool::Place && isGimmickSelected_ && selectedGimmickType_ == Stage::GimmickType::AccelerationPanel;
	accelerationIconShaftSprite_->SetSize(hoveredItem == 1 ? Vector2{64.0f, 64.0f} : Vector2{58.0f, 58.0f});
	const Vector4 accelerationColor = hoveredItem == 1 ? Vector4{1.0f, 0.9f, 0.45f, 1.0f} :
		accelerationActive ? Vector4{1.0f, 1.0f, 1.0f, 1.0f} : Vector4{0.65f, 0.65f, 0.65f, 1.0f};
	accelerationIconShaftSprite_->SetColor(accelerationColor);
	accelerationIconShaftSprite_->Draw();
	const bool isRemoveActive = interactionPhase_ == InteractionPhase::Placement && placementTool_ == PlacementTool::Remove;
	const bool isRemoveHovered = hoveredItem == 2;
	removeIconSpriteA_->SetSize(isRemoveHovered ? Vector2{12.0f, 54.0f} : Vector2{10.0f, 48.0f});
	removeIconSpriteB_->SetSize(isRemoveHovered ? Vector2{12.0f, 54.0f} : Vector2{10.0f, 48.0f});
	const Vector4 removeColor = isRemoveHovered ? Vector4{1.0f, 0.8f, 0.25f, 1.0f} :
		isRemoveActive ? Vector4{1.0f, 0.45f, 0.15f, 1.0f} : Vector4{0.85f, 0.18f, 0.18f, 1.0f};
	removeIconSpriteA_->SetColor(removeColor);
	removeIconSpriteB_->SetColor(removeColor);
	removeIconSpriteA_->Draw();
	removeIconSpriteB_->Draw();

	if (hoveredItem >= 0) {
		Sprite* hoveredText = paletteTextSprites_[hoveredItem].get();
		hoveredText->SetPosition({
			kPaletteLeft + kPaletteItemWidth * static_cast<float>(hoveredItem) + kPaletteItemWidth * 0.5f - 60.0f,
			kPaletteTop - 38.0f});
		hoveredText->Draw();
	}
	Sprite::PostDraw();
}

void GameScene::InitializeInstructionUI() {
	const Vector2 phaseChangeCenter = {
		kPhaseChangeLeft + kPhaseChangeWidth * 0.5f,
		kPhaseChangeTop + kPhaseChangeHeight * 0.5f};
	changePlantSprite_.reset(Sprite::Create(TextureManager::Load("UI/Change_plant.png"), phaseChangeCenter, Vector4{1.0f, 1.0f, 1.0f, 1.0f}, {0.5f, 0.5f}));
	changeShootSprite_.reset(Sprite::Create(TextureManager::Load("UI/Change_shoot.png"), phaseChangeCenter, Vector4{1.0f, 1.0f, 1.0f, 1.0f}, {0.5f, 0.5f}));
	changePlantSprite_->SetSize({kPhaseChangeWidth, kPhaseChangeHeight});
	changeShootSprite_->SetSize({kPhaseChangeWidth, kPhaseChangeHeight});

	const std::array<const char*, 3> tutorialFiles = {
		"Tutorial_01.csv", "Tutorial_02.csv", "Tutorial_03.csv"};
	int tutorialIndex = -1;
	for (size_t i = 0; i < tutorialFiles.size(); ++i) {
		if (stageFilePath_.find(tutorialFiles[i]) != std::string::npos) {
			tutorialIndex = static_cast<int>(i);
			break;
		}
	}
	if (tutorialIndex < 0) {
		return;
	}

	// Tutorial boards are narrow, so the readable help panel fits beside them.
	tutorialMarkSprite_.reset(Sprite::Create(TextureManager::Load("UI/!.png"), {896.0f, 80.0f}));
	tutorialMarkSprite_->SetSize({40.0f, 40.0f});
	const std::string textPath = "UI/Tutorial_0" + std::to_string(tutorialIndex + 1) + ".png";
	const float tutorialTextHeight = tutorialIndex == 2 ? 26.4f : 33.0f;
	tutorialTextSprite_.reset(Sprite::Create(TextureManager::Load(textPath), {952.0f, 84.0f}));
	tutorialTextSprite_->SetSize({264.0f, tutorialTextHeight});
}

void GameScene::DrawInstructionUI() {
	Sprite::PreDraw(DirectXCommon::GetInstance()->GetCommandList());
	if (player_.GetState() == Player::State::Aiming) {
		Sprite* phaseChangeSprite = interactionPhase_ == InteractionPhase::Placement
			? changePlantSprite_.get() : changeShootSprite_.get();
		if (phaseChangeSprite) {
			const bool isHovered = !ui_.IsPaused() && IsMouseOverPhaseChangeButton();
			const float scale = isHovered ? kPhaseChangeHoverScale : 1.0f;
			phaseChangeSprite->SetSize({kPhaseChangeWidth * scale, kPhaseChangeHeight * scale});
			phaseChangeSprite->SetColor(isHovered
				? Vector4{1.0f, 0.9f, 0.45f, 1.0f}
				: Vector4{1.0f, 1.0f, 1.0f, 1.0f});
			phaseChangeSprite->Draw();
		}
	}
	if (tutorialMarkSprite_ && tutorialTextSprite_) {
		tutorialMarkSprite_->Draw();
		tutorialTextSprite_->Draw();
	}
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

bool GameScene::IsMouseOverResetButton() const {
	const Vector2& mouse = Input::GetInstance()->GetMousePosition();
	const float halfWidth = kResetWidth * kResetHoverScale * 0.5f;
	const float halfHeight = kResetHeight * kResetHoverScale * 0.5f;
	return mouse.x >= kResetCenterX - halfWidth && mouse.x <= kResetCenterX + halfWidth &&
	       mouse.y >= kResetCenterY - halfHeight && mouse.y <= kResetCenterY + halfHeight;
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
	failedAnimationTimer_ = 0.0f;
	isFailedSpriteVisible_ = false;
	wasPlayerFailed_ = false;
	reflectionParticles_.Clear();
	movementParticles_.Clear();
	dustEmissionTimer_ = 0.0f;
	dustEmissionPhase_ = 0;
	if (failedSprite_) {
		failedSprite_->SetPosition({0.0f, -static_cast<float>(WinApp::kWindowHeight)});
	}
	stageRenderer_.UpdatePlacementCursor(stage_, placementCursor_, selectedGimmickType_, false);
}

void GameScene::EmitReflectionParticles() {
	Vector3 impactPosition = player_.GetPosition();
	impactPosition.y = 0.42f;

	const Vector3 reflectedVelocity = player_.GetVelocity();
	const float horizontalSpeed = std::sqrt(reflectedVelocity.x * reflectedVelocity.x + reflectedVelocity.z * reflectedVelocity.z);
	Vector3 reflectedDirection{};
	if (horizontalSpeed > 0.0001f) {
		reflectedDirection = {reflectedVelocity.x / horizontalSpeed, 0.0f, reflectedVelocity.z / horizontalSpeed};
	}


	// 一瞬だけ残る大きな白い閃光。複数枚を重ねて中心を強く見せる。
	for (uint32_t index = 0; index < 4; ++index) {
		const float size = 0.65f + static_cast<float>(index) * 0.16f;
		reflectionParticles_.Emit(
			impactPosition, {}, 0.10f + static_cast<float>(index) * 0.025f, size, size * 0.25f,
			{1.0f, 1.0f, 0.9f, 0.95f}, {1.0f, 0.55f, 0.08f, 0.0f});
	}

	// ゆっくり広がる黄色い衝撃光。
	for (uint32_t index = 0; index < kReflectionGlowCount; ++index) {
		const float ratio = static_cast<float>(index) / static_cast<float>(kReflectionGlowCount);
		const float angle = ratio * kTwoPi + 0.18f;
		const float speed = 0.35f + 0.12f * static_cast<float>(index % 3);
		const Vector3 glowVelocity{std::cos(angle) * speed, 0.0f, std::sin(angle) * speed};
		reflectionParticles_.Emit(
			impactPosition, glowVelocity, 0.42f, 0.38f, 0.08f,
			{1.0f, 0.78f, 0.12f, 0.72f}, {1.0f, 0.12f, 0.01f, 0.0f});
	}

	// 長さと速度にばらつきを持たせた高速の火花。
	for (uint32_t index = 0; index < kReflectionSparkCount; ++index) {
		const float ratio = static_cast<float>(index) / static_cast<float>(kReflectionSparkCount);
		const float angleJitter = static_cast<float>((index * 17u) % 11u) * 0.025f;
		const float angle = ratio * kTwoPi + angleJitter;
		const float radialSpeed = 1.35f + 1.65f * static_cast<float>((index * 7u) % 9u) / 8.0f;
		Vector3 particleVelocity{
			std::cos(angle) * radialSpeed + reflectedDirection.x * 0.9f,
			0.0f,
			std::sin(angle) * radialSpeed + reflectedDirection.z * 0.9f};
		const float life = 0.30f + 0.28f * static_cast<float>(index % 5) / 4.0f;
		const float startScale = index % 4 == 0 ? 0.24f : 0.15f;
		const Vector4 startColor = index % 5 == 0
			? Vector4{1.0f, 1.0f, 0.9f, 1.0f}
			: Vector4{1.0f, 0.62f, 0.08f, 1.0f};
		reflectionParticles_.Emit(
			impactPosition, particleVelocity, life, startScale, 0.01f,
			startColor, {1.0f, 0.04f, 0.0f, 0.0f});
	}
}

void GameScene::EmitDustTrail() {
	const Vector3 velocity = player_.GetVelocity();
	const float speed = std::sqrt(velocity.x * velocity.x + velocity.z * velocity.z);
	if (speed <= 0.15f) {
		return;
	}

	const Vector3 direction{velocity.x / speed, 0.0f, velocity.z / speed};
	const Vector3 side{-direction.z, 0.0f, direction.x};
	Vector3 origin = player_.GetPosition();
	origin.x -= direction.x * 0.28f;
	origin.z -= direction.z * 0.28f;
	origin.y = 0.30f;

	for (uint32_t index = 0; index < 4; ++index) {
		const uint32_t pattern = dustEmissionPhase_ * 4u + index;
		const float sideAmount = (static_cast<float>(pattern % 7u) - 3.0f) * 0.055f;
		const float backwardSpeed = 0.16f + 0.08f * static_cast<float>(pattern % 3u);
		const float sideSpeed = (static_cast<float>((pattern * 5u) % 9u) - 4.0f) * 0.055f;
		Vector3 position = origin;
		position.x += side.x * sideAmount;
		position.z += side.z * sideAmount;
		const Vector3 particleVelocity{
			-direction.x * backwardSpeed + side.x * sideSpeed,
			0.0f,
			-direction.z * backwardSpeed + side.z * sideSpeed};
		const float life = 0.42f + 0.16f * static_cast<float>(pattern % 4u) / 3.0f;
		const float startScale = 0.15f + 0.05f * static_cast<float>(pattern % 3u);
		movementParticles_.Emit(
			position, particleVelocity, life, startScale, startScale * 2.8f,
			{0.52f, 0.40f, 0.25f, 0.55f}, {0.30f, 0.27f, 0.23f, 0.0f});
	}
	++dustEmissionPhase_;
}

void GameScene::EmitAccelerationParticles() {
	Vector3 origin = player_.GetPosition();
	origin.y = 0.40f;
	const Vector3 velocity = player_.GetVelocity();
	const float speed = std::sqrt(velocity.x * velocity.x + velocity.z * velocity.z);
	if (speed <= 0.0001f) {
		return;
	}
	const Vector3 direction{velocity.x / speed, 0.0f, velocity.z / speed};
	const Vector3 side{-direction.z, 0.0f, direction.x};

	// パネルを踏んだ瞬間の中心光。
	for (uint32_t index = 0; index < 5; ++index) {
		const float size = 0.45f + static_cast<float>(index) * 0.15f;
		movementParticles_.Emit(
			origin, {}, 0.14f + static_cast<float>(index) * 0.025f, size, size * 1.7f,
			{1.0f, 1.0f, 0.55f, 0.9f}, {0.1f, 0.75f, 1.0f, 0.0f});
	}

	// 加速方向へ扇状に飛び出す粒子。
	for (uint32_t index = 0; index < 40; ++index) {
		const float spread = (static_cast<float>(index) / 39.0f - 0.5f) * 2.2f;
		const float forwardSpeed = 1.4f + 2.0f * static_cast<float>((index * 7u) % 11u) / 10.0f;
		const float sideSpeed = spread * (0.55f + 0.15f * static_cast<float>(index % 3u));
		const Vector3 particleVelocity{
			direction.x * forwardSpeed + side.x * sideSpeed,
			0.0f,
			direction.z * forwardSpeed + side.z * sideSpeed};
		const float life = 0.32f + 0.24f * static_cast<float>(index % 5u) / 4.0f;
		const Vector4 startColor = index % 3u == 0
			? Vector4{1.0f, 1.0f, 0.65f, 1.0f}
			: Vector4{0.15f, 0.85f, 1.0f, 1.0f};
		movementParticles_.Emit(
			origin, particleVelocity, life, index % 4u == 0 ? 0.23f : 0.14f, 0.015f,
			startColor, {0.05f, 0.25f, 1.0f, 0.0f});
	}
}
