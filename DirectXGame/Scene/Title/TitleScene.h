#pragma once

#include "../IScene.h"

/// <summary>
/// タイトル画面を管理するクラス
///
/// ゲーム開始時に表示されるシーン。
/// タイトル画面の更新・描画を行い、
/// ゲーム開始時は次のシーンへ遷移する。
/// </summary>
class TitleScene : public IScene {
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
	bool isEnd_ = false;
};
