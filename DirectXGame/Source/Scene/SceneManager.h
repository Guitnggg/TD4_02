#pragma once

#include "IScene.h"
#include "SceneName.h"

#include <memory>

namespace KamataEngine {
class Audio;
class Sprite;
}

/// <summary>
/// 現在のシーンを所有し、更新、描画、シーン遷移を管理するクラス
///
/// ・シーンの生成済みインスタンスの保持
/// ・現在シーンの更新と描画呼び出し
/// ・終了したシーンから次シーンへの切り替え
/// を担当する
/// </summary>
class SceneManager {
public:
    /// <summary>
    /// 空のシーン管理状態で生成する
    /// </summary>
    SceneManager() = default;

    /// <summary>
    /// 最初のシーンを設定して生成する
    /// </summary>
    /// <param name="firstScene">最初に実行するシーン</param>
    explicit SceneManager(std::unique_ptr<IScene> firstScene);

    /// <summary>
    /// 管理中のシーンを破棄する
    /// </summary>
    ~SceneManager();

    /// <summary>
    /// シーンの二重所有を防ぐためコピーを禁止する
    /// </summary>
    SceneManager(const SceneManager&) = delete;

    /// <summary>
    /// シーンの二重所有を防ぐためコピー代入を禁止する
    /// </summary>
    SceneManager& operator=(const SceneManager&) = delete;

    /// <summary>
    /// シーンの所有権を移動できるようにする
    /// </summary>
    SceneManager(SceneManager&&) = default;

    /// <summary>
    /// シーンの所有権をムーブ代入できるようにする
    /// </summary>
    SceneManager& operator=(SceneManager&&) = default;

    /// <summary>
    /// 管理するシーンを切り替えて初期化する
    /// </summary>
    /// <param name="nextScene">次に管理するシーン</param>
    void SetScene(std::unique_ptr<IScene> nextScene);

    /// <summary>
    /// 現在のシーンを更新し、終了していれば次シーンへ遷移する
    /// </summary>
    void Update();

    /// <summary>
    /// 現在のシーンを描画する
    /// </summary>
    void Draw();

    /// <summary>
    /// 管理中のシーンが存在しないか取得する
    /// </summary>
    bool IsEnd() const;

    /// <summary>
    /// 現在管理しているシーン名を取得する
    /// </summary>
    SceneName GetSceneName() const;

    /// <summary>
    /// 現在管理しているシーンのポインタを取得する
    /// </summary>
    IScene* GetScene() const;

private:
	enum class FadeState { None, FadeOut, FadeIn };

	std::unique_ptr<IScene> pendingScene_;
	std::unique_ptr<KamataEngine::Sprite> fadeSprite_;
	FadeState fadeState_ = FadeState::None;
	float fadeTimer_ = 0.0f;
	bool hasPendingScene_ = false;
	bool isFadeWhite_ = false;

    // 現在管理しているシーン
    std::unique_ptr<IScene> scene_;
    KamataEngine::Audio* audio_ = nullptr;
    uint32_t menuBgmHandle_ = 0;
    uint32_t gameBgmHandle_ = 0;
    uint32_t currentBgmHandle_ = 0;
    uint32_t currentBgmVoiceHandle_ = 0;
    bool bgmResourcesInitialized_ = false;
    bool isBgmPlaying_ = false;
    float currentBgmVolumeScale_ = 1.0f;

    void InitializeBgmResources();
	void InitializeFadeResources();
	void ApplyScene(std::unique_ptr<IScene> nextScene);
	void StartTransition(std::unique_ptr<IScene> nextScene);
	void UpdateFade();
	void DrawFade();
    void SwitchBgm(SceneName sceneName);
    void UpdateBgmVolume();
};
