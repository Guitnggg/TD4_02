#pragma once
#include "KamataEngine.h"

class UI {
public:
	~UI();

	void Initialize();
	void Update();
	void Draw();

	void Pause();
	void MoveC();
	void ExecuteSelectedItem();

	bool IsStageselect() const { return Stageselect; }
	bool IsProgress() const { return Progress; }

private:
	int GetMenuItemIndex(const KamataEngine::Vector2& mousePosition) const;
	bool IsPauseButton(const KamataEngine::Vector2& mousePosition) const;

	KamataEngine::DirectXCommon* dxCommon_ = nullptr;
	KamataEngine::Input* input_ = nullptr;
	KamataEngine::Audio* audio_ = nullptr;

	KamataEngine::WorldTransform worldTransfrom_;

	uint32_t textureHandle_[4]{};
	KamataEngine::Sprite* spritePause_[4]{};

	uint32_t Click = 0;
	uint32_t voiceHandle_ = 0u;

	int moveC = 0;
	int mouseSelectedItem_ = -1;

	bool Uiflag = false;
	bool Stageselect = false;
	bool Progress = false;
};
