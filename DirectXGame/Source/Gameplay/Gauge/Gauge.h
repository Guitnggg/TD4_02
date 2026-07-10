#pragma once
#include<3d/Camera.h>
#include<math/Vector3.h>
#include<2d/Sprite.h>
class Gauge {
public:
	~Gauge();
	Gauge();
	void Initalize();
	void Update(float powerRate,KamataEngine::Vector3 playerPos);
	void Draw();

private:
	float powerRate_;
	KamataEngine::Vector3 playerPos_;
	uint32_t gaugeTextureHandle1_ = 0;
	uint32_t gaugeTextureHandle2_ = 0;

	KamataEngine::Sprite* gaugeSprite1_ = nullptr;
	KamataEngine::Sprite* gaugeSprite2_ = nullptr;
};
