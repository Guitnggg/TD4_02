#pragma once
#include"KamataEngine.h"
#include <memory>
class ReflectiveBlock {
public:
	~ReflectiveBlock();
	ReflectiveBlock();
	//初期化
	void initialize();

	//更新
	void Update(KamataEngine::Camera&camera);

	//描画
	void Draw(KamataEngine::Camera& camera);

	//位置設定
	void SetBlockPos(KamataEngine::Vector3 pos) { blockPosition_.translation_ = pos; }
	KamataEngine::Vector3 GetBlockPos() { return blockPosition_.translation_; }

	/// 左クリックが押されているか判定する
	bool IsPressingLeft(KamataEngine::Input* input) const;

	/// 左クリックが押された瞬間か判定する
    bool IsTriggerLeft(KamataEngine::Input* input) const;

	/// 左クリックが離された瞬間か判定する
	bool IsReleaseLeft(KamataEngine::Input* input) const;

private:

	KamataEngine::Vector3 MouseToWorldOnPlane(const KamataEngine::Camera& camera, float planeY) const;

	/*ブロックの位置*/
	KamataEngine::WorldTransform blockPosition_;

	/*マウスの位置*/
	KamataEngine::Vector3 mousePosition_;

	/*掴んだかどうかのフラグ*/
	bool isCatch_ = false;

	/*前フレームで左クリックされていたか*/
	bool wasPressingLeft_ = false; 

	/*モデル*/
	KamataEngine::Model*model_=nullptr;

	/*input*/
	KamataEngine::Input* input_;


	
};
