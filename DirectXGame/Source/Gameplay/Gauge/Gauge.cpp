#include "Gauge.h"
#include"KamataEngine.h"

//デストラクタ
Gauge::~Gauge() { 
	delete gaugeSprite1_;
	delete gaugeSprite2_;
}
//コンストラクタ
Gauge::Gauge() {}
//初期化
void Gauge::Initalize() {
	/*テクスチャ読み込み*/
	gaugeTextureHandle1_ = KamataEngine::TextureManager::Load("red.png");
	gaugeTextureHandle2_ = KamataEngine::TextureManager::Load("green.png");

	/*スプライトの作成*/
	gaugeSprite1_ = KamataEngine::Sprite::Create(gaugeTextureHandle1_, {100.0f, 100.0f});
	gaugeSprite2_ = KamataEngine::Sprite::Create(gaugeTextureHandle2_, {100.0f, 100.0f});

	gaugeSprite1_->SetSize({300.0f, 48.0f});
	gaugeSprite1_->SetColor({1.0f, 1.0f, 1.0f, 0.8f});
	gaugeSprite2_->SetSize({0.0f, 20.0f});
	gaugeSprite2_->SetColor({1.0f, 1.0f, 1.0f, 0.8f});

}
//更新
void Gauge::Update(float powerRate, KamataEngine::Vector3 playerPos) {
	powerRate_ = powerRate;
	playerPos_ = playerPos;
}
//描画
void Gauge::Draw() {
	KamataEngine::Sprite::PreDraw();

	// 背景（赤）
	gaugeSprite1_->Draw();

	// ゲージ（緑）

	// 横幅をpowerRateに応じて変更
	gaugeSprite2_->SetSize({300.0f * powerRate_, 48.0f});

	gaugeSprite2_->Draw();

	KamataEngine::Sprite::PostDraw();
}

