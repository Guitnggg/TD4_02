#pragma once

#include "../../Game/Stage.h"
#include "../../Gameplay/Player/Player.h"
#include "../../Input/DragInput.h"
#include "../../Rendering/StageRenderer.h"
#include "../../UI/UI.h"
#include "../IScene.h"

#include <3d/Camera.h>

#include <memory>
#include <string>

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
	GameScene() = default;
	explicit GameScene(std::string stageFilePath);

	void Initialize() override;

	/// <summary>
	/// ゲーム本編シーンを更新する
	/// キーボード入力、ドラッグ入力、ギミック配置、プレイヤー、ステージ描画、終了判定を更新する
	/// </summary>
	void Update() override;
	void Draw() override;
	bool IsEnd() const override;
	std::unique_ptr<IScene> NextScene() const override;
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

	// シーン終了済みフラグ
	bool isEnd_ = false;
	std::string stageFilePath_ = "Resources\\Stages\\Eazy\\Eazy_01.csv";
	bool returnTitle_ = false;
	bool returnStageSelect_ = false;

	Stage stage_;
	Player player_;
	DragInput dragInput_;
	StageRenderer stageRenderer_;
	KamataEngine::Camera camera_;
	UI ui_;

	// ギミック配置カーソルのグリッド座標
	Stage::GridPosition placementCursor_{};

	// 現在選択しているギミックの種類
	Stage::GimmickType selectedGimmickType_ = Stage::GimmickType::ReflectSlash;

	// 1ステージで配置できるギミック数
	int maxGimmickCount_ = 3;
};
