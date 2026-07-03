#pragma once

#include <memory>

#include "SceneName.h"

class IScene {
public:
    virtual ~IScene() = default;

	/// <summary>
	/// 初期化処理
	/// </summary>
	virtual void Initialize() = 0;

	/// <summary>
	/// 更新処理（毎フレーム呼ばれる）
	/// </summary>
	virtual void Update() = 0;

	/// <summary>
	/// 描画処理（毎フレーム呼ばれる）
	/// </summary>
	virtual void Draw() = 0;

	/// <summary>
	/// シーンが終了したかどうか
	/// </summary>
	virtual bool IsEnd() const = 0;

	/// <summary>
	/// 次のシーンを返す（nullptr を返すとゲーム終了）
	/// </summary>
	virtual std::unique_ptr<IScene> NextScene() const = 0;

	/// <summary>
	/// 現在のシーン名を返す（デバッグや切替の判定に使える）
	/// </summary>
	virtual SceneName GetSceneName() const = 0;
};