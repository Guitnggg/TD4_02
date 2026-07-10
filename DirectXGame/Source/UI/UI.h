#pragma once
#include "KamataEngine.h"

// UIクラスを処理するクラス
class UI {
public:
	~UI();

	void Initialize();

	void Update();

	void Draw();

	// 一時停止の判定
	void Pause();

	// カーソル移動
	void MoveC();

	// UI起動のgetter
	bool IsStageselect() const { return Stageselect; }

	// ゲーム進行のgetter
	bool IsProgress() const { return Progress; }
	
private:
	KamataEngine::DirectXCommon* dxCommon_ = nullptr;
	KamataEngine::Input* input_ = nullptr;
	KamataEngine::Audio* audio_ = nullptr;

	// ワールドトランスフォーム
	KamataEngine::WorldTransform worldTransfrom_;

	// テクスチャハンドル
	uint32_t textureHandle_[4];

	// 一時停止ボタン用スプライト
	KamataEngine::Sprite* spritePause_[4];

	// サウンドデータハンドル
	uint32_t Click = 0;

	// 音声再生ハンドル
	uint32_t voiceHandle_ = 0u;

	// カーソル
	int moveC = 0;

	// UI起動フラグ
	bool Uiflag = false;

	// ステージセレクトフラグ
	bool Stageselect = false;

	// ゲーム進行フラグ
	bool Progress = false;
};