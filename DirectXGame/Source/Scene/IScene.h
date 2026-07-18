#pragma once

#include <memory>

#include "SceneName.h"

/// <summary>
/// 各シーンが共通して実装する処理を定義するインターフェース
///
/// ・初期化、更新、描画
/// ・シーン終了判定
/// ・次シーンの生成と現在シーン名の取得
/// を各シーンに実装させる
/// </summary>
class IScene {
public:
	/// <summary>
	/// 派生シーンを安全に破棄するための仮想デストラクタ
	/// </summary>
    virtual ~IScene() = default;

    /// <summary>
    /// シーン開始時の初期化を行う
    /// </summary>
    virtual void Initialize() = 0;

    /// <summary>
    /// シーンごとの入力、状態、遷移条件を更新する
    /// </summary>
    virtual void Update() = 0;

    /// <summary>
    /// シーンごとの描画処理を行う
    /// </summary>
    virtual void Draw() = 0;

    /// <summary>
    /// シーンが終了して次シーンへ遷移できるか取得する
    /// </summary>
    virtual bool IsEnd() const = 0;

    /// <summary>
    /// 次に遷移するシーンを生成する
    /// </summary>
    virtual std::unique_ptr<IScene> NextScene() const = 0;

    /// <summary>
    /// 現在のシーン名を取得する
    /// </summary>
    virtual SceneName GetSceneName() const = 0;

    // 効果音などを優先したいシーンでは、BGM音量を個別に下げられる。
    virtual float GetBgmVolumeScale() const { return 1.0f; }
};
