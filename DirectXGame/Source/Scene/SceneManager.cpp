#include "SceneManager.h"

#include <KamataEngine.h>
#include <utility>

using namespace KamataEngine;

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
        currentBgmVoiceHandle_ = audio_->PlayWave(currentBgmHandle_, true, 0.4f);
        isBgmPlaying_ = true;
    }
}

void SceneManager::UpdateBgmVolume() {
    if (!scene_ || !isBgmPlaying_) { return; }

    const float volumeScale = scene_->GetBgmVolumeScale();
    if (volumeScale == currentBgmVolumeScale_) { return; }

    constexpr float kBaseBgmVolume = 0.4f;
    audio_->SetVolume(currentBgmVoiceHandle_, kBaseBgmVolume * volumeScale);
    currentBgmVolumeScale_ = volumeScale;
}

void SceneManager::SetScene(std::unique_ptr<IScene> nextScene) {
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

void SceneManager::Update() {
    // シーンが存在しない場合は何もしない
    if (!scene_) { return; }

    // 現在のシーンを更新
    scene_->Update();
    UpdateBgmVolume();

    // シーンが終了していなければそのまま続行
    if (!scene_->IsEnd()) { return; }

    // 次のシーンへ遷移
    SetScene(scene_->NextScene());
}

void SceneManager::Draw() {
    // シーンが存在する場合のみ描画
    if (scene_) {
        scene_->Draw();
    }
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
