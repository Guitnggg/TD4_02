#pragma once

#include "../../Game/Stage.h"
#include "../../Gameplay/Player/Player.h"
#include "../../Input/DragInput.h"
#include "../../Rendering/StageRenderer.h"
#include "../IScene.h"

#include <3d/Camera.h>

#include <memory>

/// <summary>
/// ゲーム本編シーンの進行、入力、描画を管理するクラス
///
/// ・ステージ、プレイヤー、ドラッグ入力の初期化と更新
/// ・発射前のギミック配置
/// ・ゲーム用カメラとステージ描画の管理
/// ・クリア時のリザルトシーンへの遷移判定
/// を担当する
/// </summary>
class GameScene : public IScene {
public:
	/// <summary>
	/// ゲーム本編シーンを初期化する
	/// ステージ、プレイヤー、入力、カメラ、描画オブジェクトを開始状態にする
	/// </summary>
	void Initialize() override;

	/// <summary>
	/// ゲーム本編シーンを更新する
	/// キーボード入力、ドラッグ入力、ギミック配置、プレイヤー、ステージ描画、終了判定を更新する
	/// </summary>
	void Update() override;

	/// <summary>
	/// ゲーム本編シーンを描画する
	/// ステージ、ガイド、ドラッグ入力、デバッグ表示を描画する
	/// </summary>
	void Draw() override;

	/// <summary>
	/// シーン終了済みか取得する
	/// </summary>
	bool IsEnd() const override;

	/// <summary>
	/// 次に遷移するシーンを生成する
	/// </summary>
	std::unique_ptr<IScene> NextScene() const override;

	/// <summary>
	/// 現在のシーン名を取得する
	/// </summary>
	SceneName GetSceneName() const override;

private:
	/// <summary>
	/// 発射前のギミック配置入力を更新する
	/// </summary>
	void UpdateGimmickPlacement();

	/// <summary>
	/// ギミック配置カーソルをステージ範囲内に収める
	/// </summary>
	void ClampPlacementCursor();

	/// <summary>
	/// 配置されたギミックを削除して描画を更新する
	/// </summary>
	void ClearPlacedGimmicks();

private:
	// シーン終了済みフラグ
	bool isEnd_ = false;

	// 現在プレイ中のステージ情報
	Stage stage_;

	// ステージ上で操作するプレイヤー
	Player player_;

	// マウスドラッグによる発射入力
	DragInput dragInput_;

	// ステージとプレイヤーの描画管理
	StageRenderer stageRenderer_;

	// ゲーム本編を上から見下ろすカメラ
	KamataEngine::Camera camera_;

	// ギミック配置カーソルのグリッド座標
	Stage::GridPosition placementCursor_{};

	// 現在選択しているギミックの種類
	Stage::GimmickType selectedGimmickType_ = Stage::GimmickType::ReflectSlash;

	// 1ステージで配置できるギミック数
	int maxGimmickCount_ = 3;
};
