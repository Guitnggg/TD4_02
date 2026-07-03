#pragma once

#include "../../Game/Stage.h"
#include "../../Player/Player.h"
#include "../IScene.h"

#include <3d/Camera.h>
#include <3d/Model.h>
#include <3d/Object3d.h>

#include <memory>
#include <vector>

/// <summary>
/// ゲームプレイ画面を管理するクラス
///
/// プレイヤーや敵、UIなどのゲーム本編の処理を行う。
/// ゲームの進行状況を管理し、
/// クリアまたはゲームオーバー時に次のシーンへ遷移する。
/// </summary>
class GameScene : public IScene {
public:
	/// <summary>
	/// シーンの初期化
	/// </summary>
	void Initialize() override;

	/// <summary>
	/// シーンの更新
	/// </summary>
	void Update() override;

	/// <summary>
	/// シーンの描画
	/// </summary>
	void Draw() override;

	/// <summary>
	/// シーンが終了したかを判定
	/// </summary>
	/// <returns>シーン終了してればtrue、それ以外はfalse</returns>
	bool IsEnd() const override;

	/// <summary>
	/// 次に遷移するシーンを生成
	/// </summary>
	/// <returns>次のシーン</returns>
	std::unique_ptr<IScene> NextScene() const override;

	/// <summary>
	/// 現在のシーン名を取得する
	/// </summary>
	/// <returns>現在のシーン名</returns>
	SceneName GetSceneName() const override;

private:
	void BuildStageObjects();
	std::unique_ptr<KamataEngine::Object3d> CreateCube(const KamataEngine::Vector3& translation, const KamataEngine::Vector3& scale);
	void UpdatePlayerObject();
	void DrawStageGuide();

	bool isEnd_ = false;
	Stage stage_;
	Player player_;
	KamataEngine::Camera camera_;
	std::unique_ptr<KamataEngine::Model> cubeModel_;
	std::vector<std::unique_ptr<KamataEngine::Object3d>> floorObjects_;
	std::vector<std::unique_ptr<KamataEngine::Object3d>> wallObjects_;
	std::vector<std::unique_ptr<KamataEngine::Object3d>> placeableObjects_;
	std::vector<std::unique_ptr<KamataEngine::Object3d>> gimmickObjects_;
	std::unique_ptr<KamataEngine::Object3d> goalObject_;
	std::unique_ptr<KamataEngine::Object3d> playerObject_;
};
