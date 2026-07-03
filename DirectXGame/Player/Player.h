#pragma once

#include "../Game/Stage.h"

#include <math/Vector3.h>

/// <summary>
/// プレイヤーの発射前、移動中、停止後の状態を管理するクラス
///
/// ・発射前の左右移動
/// ・発射後の移動、摩擦、反射
/// ・ゴール到達と失敗判定
/// を担当する
/// </summary>
class Player {
public:
	/// <summary>
	/// プレイヤーの現在状態
	/// </summary>
	enum class State {		
		Aiming,  // 発射前。位置調整と発射が可能		
		Moving,  // 発射後。物理演算で移動中		
		Stopped,  // クリアまたは失敗で停止
	};

	/// <summary>
	/// ステージ情報に合わせてプレイヤーを初期化する
	/// </summary>
	/// <param name="stage">現在のステージ情報</param>
	void Initialize(const Stage& stage);

	/// <summary>
	/// プレイヤーを更新する
	/// 移動中のみ、移動、摩擦、反射、ゴール判定、停止判定を行う
	/// </summary>
	/// <param name="stage">現在のステージ情報</param>
	void Update(const Stage& stage);

	/// <summary>
	/// プレイヤーをステージ開始状態に戻す
	/// </summary>
	/// <param name="stage">現在のステージ情報</param>
	void Reset(const Stage& stage);

	/// <summary>
	/// 発射前のプレイヤー位置を左に移動する
	/// </summary>
	/// <param name="stage">移動範囲を持つステージ情報</param>
	void MoveAimLeft(const Stage& stage);

	/// <summary>
	/// 発射前のプレイヤー位置を右に移動する
	/// </summary>
	/// <param name="stage">移動範囲を持つステージ情報</param>
	void MoveAimRight(const Stage& stage);

	/// <summary>
	/// プレイヤーを発射して移動状態へ切り替える
	/// </summary>
	void Fire();
	void Fire(const KamataEngine::Vector3& initialVelocity);

	/// <summary>
	/// 現在のワールド座標を取得する
	/// </summary>
	const KamataEngine::Vector3& GetPosition() const { return position_; }

	/// <summary>
	/// 現在の状態を取得する
	/// </summary>
	State GetState() const { return state_; }

	/// <summary>
	/// ゴール到達済みか取得する
	/// </summary>
	bool IsClear() const { return isClear_; }

	/// <summary>
	/// 失敗済みか取得する
	/// </summary>
	bool IsFailed() const { return isFailed_; }

private:
	/// <summary>
	/// 壁またはステージ外へ進んだ場合に直前位置へ戻して反射する
	/// </summary>
	/// <param name="stage">現在のステージ情報</param>
	/// <param name="previousGrid">移動前のグリッド座標</param>
	/// <param name="currentGrid">移動後のグリッド座標</param>
	void ReflectByWallOrBounds(const Stage& stage, const Stage::GridPosition& previousGrid, const Stage::GridPosition& currentGrid);

	// 現在のワールド座標
	KamataEngine::Vector3 position_{};

	// 現在の速度
	KamataEngine::Vector3 velocity_{};

	// 発射前に調整しているグリッド座標
	Stage::GridPosition aimGrid_{};

	// 同じ反射ギミックで連続反射しないための直前接触グリッド
	Stage::GridPosition lastGimmickGrid_{-1, -1};

	// 現在のプレイヤー状態
	State state_ = State::Aiming;

	// クリア済みフラグ
	bool isClear_ = false;

	// 失敗済みフラグ
	bool isFailed_ = false;
};
