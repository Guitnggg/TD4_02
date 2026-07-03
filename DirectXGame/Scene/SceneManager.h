#pragma once

#include "IScene.h"
#include "SceneName.h"

#include <memory>

class SceneManager {
public:
    SceneManager() = default;

    /// <summary>
    /// 最初のシーンを設定してSceneManagerを生成
    /// </summary>
    /// <param name="firstScene">開始時に表示するシーン</param>
    explicit SceneManager(std::unique_ptr<IScene> firstScene);

    /// <summary>
    /// デストラクタ
    /// </summary>
    ~SceneManager() = default;

    /// <summary>
    /// コピーは禁止（シーンを一意に管理するため）
    /// </summary>
    SceneManager(const SceneManager&) = delete;

    /// <summary>
    /// コピー代入は禁止
    /// </summary>
    SceneManager& operator=(const SceneManager&) = delete;

    /// <summary>
    /// ムーブは許可
    /// </summary>
    SceneManager(SceneManager&&) = default;

    /// <summary>
    /// ムーブ代入は許可
    /// </summary>
    SceneManager& operator=(SceneManager&&) = default;

    /// <summary>
    /// シーンを切り替える
    /// 切り替え後、自動でInitialize()を呼び出す
    /// </summary>
    /// <param name="nextScene">次に表示するシーン</param>
    void SetScene(std::unique_ptr<IScene> nextScene);

    /// <summary>
    /// 現在のシーンを更新する
    /// シーン終了時は次のシーンへ遷移する
    /// </summary>
    void Update();

    /// <summary>
    /// 現在のシーンを描画する
    /// </summary>
    void Draw();

    /// <summary>
    /// SceneManagerが終了しているか判定
    /// （管理しているシーンが存在しない場合true）
    /// </summary>
    bool IsEnd() const;

    /// <summary>
    /// 現在のシーン名を取得
    /// </summary>
    SceneName GetSceneName() const;

    /// <summary>
    /// 現在のシーンを取得
    /// </summary>
    IScene* GetScene() const;

private:
    // 現在管理しているシーン
    std::unique_ptr<IScene> scene_;
};