#include "SceneManager.h"

#include "../Core/Math/MathUtility.h"

#include <KamataEngine.h>
#include <utility>

using namespace KamataEngine;

namespace {
constexpr float kFadeDuration = 0.45f;
constexpr float kDeltaTime = 1.0f / 60.0f;
constexpr float kBgmVolume = 0.3f;
}

SceneManager::SceneManager(std::unique_ptr<IScene> firstScene) {
    // 最初のシーンを設定
    SetScene(std::move(firstScene));
}

SceneManager::~SceneManager() {
    if (audio_ && isBgmPlaying_) { audio_->StopWave(currentBgmVoiceHandle_); }
}

void SceneManager::InitializeBgmResources() {
    if (bgmResourcesInitialized_) { return; }
    audio_ = Audio::GetInstance();
    menuBgmHandle_ = audio_->LoadWave("BGM/GameStart.mp3");
    gameBgmHandle_ = audio_->LoadWave("BGM/BGM.mp3");
    bgmResourcesInitialized_ = true;
}

void SceneManager::SwitchBgm(SceneName sceneName) {
    InitializeBgmResources();

    uint32_t nextBgmHandle = 0;
    bool shouldPlay = true;
    switch (sceneName) {
    case SceneName::Title:
    case SceneName::DifficultySelect:
        nextBgmHandle = menuBgmHandle_;
        break;
    case SceneName::InGame:
        nextBgmHandle = gameBgmHandle_;
        break;
    default:
        shouldPlay = false;
        break;
    }

    if (isBgmPlaying_ && shouldPlay && currentBgmHandle_ == nextBgmHandle) { return; }
    if (isBgmPlaying_) {
        audio_->StopWave(currentBgmVoiceHandle_);
        isBgmPlaying_ = false;
        currentBgmVolumeScale_ = 1.0f;
    }
    if (shouldPlay) {
        currentBgmHandle_ = nextBgmHandle;
        currentBgmVoiceHandle_ = audio_->PlayWave(currentBgmHandle_, true, kBgmVolume);
        isBgmPlaying_ = true;
    }
}

void SceneManager::UpdateBgmVolume() {
    if (!scene_ || !isBgmPlaying_) { return; }

    const float volumeScale = scene_->GetBgmVolumeScale();
    if (volumeScale == currentBgmVolumeScale_) { return; }

    audio_->SetVolume(currentBgmVoiceHandle_, kBgmVolume * volumeScale);
    currentBgmVolumeScale_ = volumeScale;
}

void SceneManager::SetScene(std::unique_ptr<IScene> nextScene) {
	ApplyScene(std::move(nextScene));
	if (scene_) {
		InitializeFadeResources();
		fadeState_ = FadeState::FadeIn;
		fadeTimer_ = 0.0f;
		isFadeWhite_ = false;
	}
}

void SceneManager::ApplyScene(std::unique_ptr<IScene> nextScene) {
    // 所有権を移動して新しいシーンへ切り替える
    scene_ = std::move(nextScene);

    // シーンが存在する場合は初期化
    if (scene_) {
        scene_->Initialize();
        SwitchBgm(scene_->GetSceneName());
        UpdateBgmVolume();
    } else {
        SwitchBgm(SceneName::None);
    }
}

void SceneManager::InitializeFadeResources() {
	if (fadeSprite_) { return; }
	const uint32_t textureHandle = TextureManager::Load("white1x1.png");
	fadeSprite_.reset(Sprite::Create(textureHandle, {0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 1.0f}));
	fadeSprite_->SetSize({static_cast<float>(WinApp::kWindowWidth), static_cast<float>(WinApp::kWindowHeight)});
}

void SceneManager::StartTransition(std::unique_ptr<IScene> nextScene) {
	pendingScene_ = std::move(nextScene);
	hasPendingScene_ = true;
	isFadeWhite_ = scene_ && scene_->GetSceneName() == SceneName::InGame && pendingScene_ && pendingScene_->GetSceneName() == SceneName::Result;
	fadeState_ = FadeState::FadeOut;
	fadeTimer_ = 0.0f;
	InitializeFadeResources();
}

void SceneManager::UpdateFade() {
	fadeTimer_ += kDeltaTime;
	if (fadeTimer_ < kFadeDuration) { return; }
	if (fadeState_ == FadeState::FadeOut) {
		ApplyScene(std::move(pendingScene_));
		hasPendingScene_ = false;
		if (scene_) {
			fadeState_ = FadeState::FadeIn;
			fadeTimer_ = 0.0f;
		} else {
			fadeState_ = FadeState::None;
		}
	} else {
		fadeState_ = FadeState::None;
		fadeTimer_ = 0.0f;
	}
}

void SceneManager::Update() {
    // シーンが存在しない場合は何もしない
    if (!scene_) { return; }
	if (fadeState_ != FadeState::None) {
		UpdateFade();
		return;
	}

    // 現在のシーンを更新
    scene_->Update();
    UpdateBgmVolume();

    // シーンが終了していなければそのまま続行
    if (!scene_->IsEnd()) { return; }

    // 次のシーンへ遷移
	if (!hasPendingScene_) { StartTransition(scene_->NextScene()); }
}

void SceneManager::Draw() {
    // シーンが存在する場合のみ描画
    if (scene_) {
        scene_->Draw();
    }
	DrawFade();
}

void SceneManager::DrawFade() {
	if (fadeState_ == FadeState::None || !fadeSprite_) { return; }
	const float progress = MyMath::Clamp(fadeTimer_ / kFadeDuration, 0.0f, 1.0f);
	const float eased = MyMath::EaseInOutSine(progress);
	const float alpha = fadeState_ == FadeState::FadeOut ? eased : 1.0f - eased;
	const float color = isFadeWhite_ ? 1.0f : 0.0f;
	fadeSprite_->SetColor({color, color, color, alpha});
	Sprite::PreDraw(DirectXCommon::GetInstance()->GetCommandList());
	fadeSprite_->Draw();
	Sprite::PostDraw();
}

bool SceneManager::IsEnd() const {
    // シーンが存在しなければSceneManagerも終了
    return scene_ == nullptr;
}

SceneName SceneManager::GetSceneName() const {
    // シーンが存在しない場合はNoneを返す
    if (!scene_) {
        return SceneName::None;
    }

    // 現在のシーン名を返す
    return scene_->GetSceneName();
}

IScene* SceneManager::GetScene() const {
    // 現在管理しているシーンを取得
    return scene_.get();
}
