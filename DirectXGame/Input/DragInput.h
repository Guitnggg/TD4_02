#pragma once

#include <3d/Camera.h>
#include <2d/Sprite.h>
#include <input/Input.h>
#include <math/Vector2.h>
#include <math/Vector3.h>
#include <math/Vector4.h>

#include <memory>

/// <summary>
/// ドラッグ操作を管理するクラス
/// 
/// ・マウス入力の取得
/// ・ドラッグ開始・終了の判定
/// ・発射方向と発射速度の計算
/// ・ドラッグガイドや軌道予測の描画
/// 
///  発射後の移動処理は行わず、算出した初速度をPlayerクラスへ受け渡す
/// </summary>
class DragInput {
public:
    void Initialize();

    /// <summary>
    /// ドラッグ入力の状態を初期化する
    /// </summary>
    void Reset();

    /// <summary>
    /// ドラッグを更新する
    /// </summary>
    void Update(KamataEngine::Input* input, const KamataEngine::Camera& camera, const KamataEngine::Vector3& playerPosition, bool canStart);

    /// <summary>
    /// ドラッグ中のガイドを描画する
    /// </summary>
    void Draw(const KamataEngine::Camera& camera);

    /// <summary>
    /// 発射速度を取得する
    /// 発射可能な初速度がある場合は取得し、内部状態をリセットする 
    /// <returns>発射速度を取得出来たらtrue、所得できなかったらfalse </returns>
    /// </summary>
    bool ConsumeLaunchVelocity(KamataEngine::Vector3& velocity);

    bool IsDragging() const { return isDragging_; }

    float GetPowerRate() const { return powerRate_; }

private:
    /// <summary>
    /// マウス座標を指定平面上のワールド座標へ変換する
    /// </summary>
    KamataEngine::Vector3 MouseToWorldOnPlane(const KamataEngine::Camera& camera, float planeY) const;

    /// <summary>
    /// 左クリックが押されているか判定する
    /// </summary>
    bool IsPressingLeft(KamataEngine::Input* input) const;

    /// <summary>
    /// 左クリックが押された瞬間か判定する
    /// </summary>
    bool IsTriggerLeft(KamataEngine::Input* input) const;

    /// <summary>
    /// 左クリックが離された瞬間か判定する
    /// </summary>
    bool IsReleaseLeft(KamataEngine::Input* input) const;

    /// <summary>
    /// ドラッグ量から発射方向と発射速度を更新する
    ///
    /// ドラッグ距離に応じて発射速度を決定し、
    /// ドラッグ方向とは逆方向へ発射する速度ベクトルを計算する。
    /// </summary>
    void UpdateDragVector(const KamataEngine::Vector3& playerPosition, const KamataEngine::Vector3& currentWorld);

    /// <summary>
    /// 発射方向を示す矢印を描画する
    /// </summary>
    KamataEngine::Vector2 WorldToScreen(const KamataEngine::Vector3& worldPosition, const KamataEngine::Camera& camera) const;
    void DrawArrowSprite(const KamataEngine::Vector3& from, const KamataEngine::Vector3& to, const KamataEngine::Camera& camera);

private:
    bool isDragging_ = false;         // ドラッグ中か    
    bool wasPressingLeft_ = false;    // 前フレームで左クリックされていたか    
    bool hasLaunchVelocity_ = false;  // 発射可能な速度を保持しているか
       
    KamataEngine::Vector3 dragStartWorld_{};    // ドラッグ開始位置    
    KamataEngine::Vector3 dragCurrentWorld_{};  // 現在のドラッグ位置    
    KamataEngine::Vector3 launchVelocity_{};    // 発射時の初速度

    float powerRate_ = 0.0f;  // ドラッグ量を０～１で表した値
    uint32_t arrowTextureHandle_ = 0;
    std::unique_ptr<KamataEngine::Sprite> arrowSprite_;
};

