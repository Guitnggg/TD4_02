#pragma once

#include "../IScene.h"

#include <string>
#include <memory>
#include <2d/Sprite.h>

class ResultScene : public IScene {
public:
	ResultScene(std::string currentStagePath, int placedGimmickCount);
	~ResultScene() override;

	void Initialize() override;
	void Update() override;
	void Draw() override;
	bool IsEnd() const override;
	std::unique_ptr<IScene> NextScene() const override;
	SceneName GetSceneName() const override;

private:
	bool isEnd_ = false;

	std::string currentStagePath_;
	int placedGimmickCount_ = 0;
	std::string evaluationLabel_;

	// 選択肢
	int selectedIndex_ = 0; // 0: Next, 1: StageSelect
	bool toStageSelect_ = false;

	// 星画像（通常 / ハイライト 各1つずつ）
	int texStars3_ = 0;
	int texStars3Sel_ = 0;
	int texStars2_ = 0;
	int texStars2Sel_ = 0;
	int texStars1_ = 0;
	int texStars1Sel_ = 0;

	// 評価を描画するスプライト
	std::unique_ptr<KamataEngine::Sprite> spriteEvaluation_;

	// 次 / ステージセレクト用スプライト（既にあれば流用可能）
	int texNext_ = 0;
	int texNextSel_ = 0;
	int texStageSelect_ = 0;
	int texStageSelectSel_ = 0;
	std::unique_ptr<KamataEngine::Sprite> spriteNext_;
	std::unique_ptr<KamataEngine::Sprite> spriteStageSelect_;
};
