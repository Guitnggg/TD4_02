#pragma once

#include "../Game/Stage.h"

#include <3d/Camera.h>
#include <3d/Model.h>
#include <3d/Object3d.h>
#include <math/Vector3.h>
#include <math/Vector4.h>

#include <memory>
#include <vector>

/// <summary>
/// ステージ上の床、壁、ギミック、ゴール、プレイヤーを描画するクラス
///
/// ・ステージ情報から描画用オブジェクトを生成
/// ・プレイヤー描画位置の更新
/// ・ステージ本体とグリッドガイドの描画
/// を担当する
/// </summary>
class StageRenderer {
public:
	/// <summary>
	/// ステージ描画に必要なモデルとオブジェクトを初期化する
	/// </summary>
	/// <param name="stage">描画対象のステージ情報</param>
	/// <param name="playerPosition">プレイヤーの初期ワールド座標</param>
	void Initialize(const Stage& stage, const KamataEngine::Vector3& playerPosition);

	/// <summary>
	/// ステージ上の描画オブジェクトを描画する
	/// </summary>
	/// <param name="camera">描画に使用するカメラ</param>
	void Draw(KamataEngine::Camera& camera);

	/// <summary>
	/// ステージのグリッドガイドを描画する
	/// </summary>
	/// <param name="stage">ガイド描画に使用するステージ情報</param>
	/// <param name="camera">描画に使用するカメラ</param>
	void DrawGuide(const Stage& stage, KamataEngine::Camera& camera);

	/// <summary>
	/// プレイヤー描画オブジェクトの位置を更新する
	/// </summary>
	/// <param name="playerPosition">プレイヤーの現在ワールド座標</param>
	void UpdatePlayer(const KamataEngine::Vector3& playerPosition);

	/// <summary>
	/// ステージ上に配置されたギミックの描画を作り直す
	/// </summary>
	void RebuildGimmicks(const Stage& stage);

	/// <summary>
	/// ギミック配置カーソルの表示位置と向きを更新する
	/// </summary>
	void UpdatePlacementCursor(const Stage& stage, const Stage::GridPosition& grid, Stage::GimmickType selectedType, bool isVisible,
	                           AccelerationPanel::Direction panelDirection = AccelerationPanel::Direction::PositiveZ);

private:
	/// <summary>
	/// 指定した位置と大きさでキューブオブジェクトを生成する
	/// </summary>
	/// <param name="translation">生成するワールド座標</param>
	/// <param name="scale">生成するキューブの拡大率</param>
	/// <returns>初期化済みのキューブオブジェクト</returns>
	std::unique_ptr<KamataEngine::Object3d> CreateCube(
		const KamataEngine::Vector3& translation, const KamataEngine::Vector3& scale, const KamataEngine::Vector4& color);

	/// <summary>
	/// ステージ情報から床、壁、ギミック、ゴール、プレイヤーの描画オブジェクトを構築する
	/// </summary>
	/// <param name="stage">構築元のステージ情報</param>
	/// <param name="playerPosition">プレイヤーの初期ワールド座標</param>
	void BuildStageObjects(const Stage& stage, const KamataEngine::Vector3& playerPosition);

	/// <summary>
	/// ギミック用オブジェクトだけを作り直す
	/// </summary>
	void BuildGimmickObjects(const Stage& stage);

	// ステージ描画に共通して使用するキューブモデル
	std::unique_ptr<KamataEngine::Model> cubeModel_;
	std::unique_ptr<KamataEngine::Model> floorModel_;
	std::unique_ptr<KamataEngine::Model> wallModel_;
	std::unique_ptr<KamataEngine::Model> playerModel_;
	std::unique_ptr<KamataEngine::Model> goalModel_;

	// 配置ギミック用モデル
	std::unique_ptr<KamataEngine::Model> reflectGimmickModel_;
	std::unique_ptr<KamataEngine::Model> reflectGimmickCenterModel_;
	std::unique_ptr<KamataEngine::Model> accelerationPanelModel_;
	std::unique_ptr<KamataEngine::Model> accelerationPanelBaseModel_;

	// 床マスの描画オブジェクト一覧
	std::vector<std::unique_ptr<KamataEngine::Object3d>> floorObjects_;

	// 壁の描画オブジェクト一覧
	std::vector<std::unique_ptr<KamataEngine::Object3d>> wallObjects_;

	// ギミック設置可能マスの描画オブジェクト一覧

	// 反射ギミックの描画オブジェクト一覧
	std::vector<std::unique_ptr<KamataEngine::Object3d>> gimmickObjects_;

	// 配置予定ギミックのプレビュー表示
	std::unique_ptr<KamataEngine::Object3d> placementCursorObject_;
	std::unique_ptr<KamataEngine::Object3d> placementCursorReflectCenterObject_;
	std::unique_ptr<KamataEngine::Object3d> placementCursorBaseObject_;
	std::unique_ptr<KamataEngine::Object3d> placementCursorArrowObject_;

	// 配置カーソルを表示するかどうか
	bool isPlacementCursorVisible_ = false;
	bool isPlacementCursorReflectCenterVisible_ = false;
	bool isPlacementCursorBaseVisible_ = false;
	bool isPlacementCursorArrowVisible_ = false;

	// ゴールの描画オブジェクト
	std::unique_ptr<KamataEngine::Object3d> goalObject_;

	// プレイヤーの描画オブジェクト
	std::unique_ptr<KamataEngine::Object3d> playerObject_;
};
