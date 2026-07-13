#include <cassert>
#include "UI.h"

using namespace KamataEngine;

UI::~UI() {
	// 一時停止ボタン削除
	for ( int i = 0; i < 4; i++ ) {
		delete spritePause_[i];
	}
}

void UI::Initialize() {
	dxCommon_ = DirectXCommon::GetInstance();
	input_ = Input::GetInstance();
	audio_ = Audio::GetInstance();

	// ワールド変数の初期化
	worldTransfrom_.Initialize();
	worldTransfrom_.scale_ = { 2, 2, 2 };

	// ファイル名を指定してテクスチャを読み込む
	textureHandle_[0] = TextureManager::Load("UI/Pause.png");
	textureHandle_[1] = TextureManager::Load("UI/Pose1.png");
	textureHandle_[2] = TextureManager::Load("UI/Pose2.png");
	textureHandle_[3] = TextureManager::Load("UI/Pose3.png");

	// BGM・SE読み込み
	Click = audio_->LoadWave("SE/Dicision.mp3");

	// 一時停止ボタン用スプライト	
	spritePause_[0] = Sprite::Create(textureHandle_[0], { 1216, 0 });
	spritePause_[1] = Sprite::Create(textureHandle_[1], { 320, 180 });
	spritePause_[2] = Sprite::Create(textureHandle_[2], { 320, 180 });
	spritePause_[3] = Sprite::Create(textureHandle_[3], { 320, 180 });
}

void UI::Update() {
	// 一時停止の判定
	Pause();

	// カーソル移動
	MoveC();

	// 行列を更新
	worldTransfrom_.UpdateMatrix();
}

void UI::Draw() {
	// コマンドリストの取得
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();
	// 背景スプライト描画前処理
	Sprite::PreDraw(commandList);
	// 一時停止ボタン用描画
	spritePause_[0]->Draw();
	// ポーズ描画
	if (Uiflag && moveC == 0) {
		spritePause_[1]->Draw();
	}
	if (Uiflag && moveC == 1) {
		spritePause_[2]->Draw();
	}
	if (Uiflag && moveC == 2) {
		spritePause_[3]->Draw();
	}
	// プライト描画後処理
	Sprite::PostDraw();
	// 深度バッファクリア
	dxCommon_->ClearDepthBuffer();
}

// 一時停止の判定
void UI::Pause() {
	// 設定を開く
	if (Input::GetInstance()->IsTriggerMouse(0)) {
		// マウスの位置取得
		Vector2 v = Input::GetInstance()->GetMousePosition();

		if ( v.x >= 1216 && v.y <= 64 ) {
			audio_->PlayWave(Click, false);
			Uiflag = true;
		}
	}
}

// カーソル移動
void UI::MoveC() {
	// 移動
	if (input_->TriggerKey(DIK_UP)) {
		moveC -= 1;
	}
	if (input_->TriggerKey(DIK_DOWN)) {
		moveC += 1;
	}

	// 決定
	if (input_->PushKey(DIK_RETURN) && moveC == 0) {
		Uiflag = false;
	}
	if (input_->PushKey(DIK_RETURN) && moveC == 1) {
		Stageselect = true;
	}
	if (input_->PushKey(DIK_RETURN) && moveC == 2) {
		Progress = true;
	}

	// 上限
	if (moveC < 0) {
		moveC = 0;
	}
	if (moveC > 2) {
		moveC = 2;
	}
}
