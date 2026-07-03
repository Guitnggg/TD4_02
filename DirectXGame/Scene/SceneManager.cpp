#include "SceneManager.h"

#include <utility>

SceneManager::SceneManager(std::unique_ptr<IScene> firstScene) {
    // 最初のシーンを設定
    SetScene(std::move(firstScene));
}

void SceneManager::SetScene(std::unique_ptr<IScene> nextScene) {
    // 所有権を移動して新しいシーンへ切り替える
    scene_ = std::move(nextScene);

    // シーンが存在する場合は初期化
    if (scene_) {
        scene_->Initialize();
    }
}

void SceneManager::Update() {
    // シーンが存在しない場合は何もしない
    if (!scene_) {
        return;
    }

    // 現在のシーンを更新
    scene_->Update();

    // シーンが終了していなければそのまま続行
    if (!scene_->IsEnd()) {
        return;
    }

    // 次のシーンへ遷移
    SetScene(scene_->NextScene());
}

void SceneManager::Draw() {
    // シーンが存在する場合のみ描画
    if (scene_) {
        scene_->Draw();
    }
}

bool SceneManager::IsEnd() const {
    // シーンが存在しなければSceneManagerも終了
    return scene_ == nullptr;
}

SceneName SceneManager::GetSceneName() const {
    // シーンが存在しない場合はNoneを返す
    if (!scene_) {
        return SceneName::None;
    }

    // 現在のシーン名を返す
    return scene_->GetSceneName();
}

IScene* SceneManager::GetScene() const {
    // 現在管理しているシーンを取得
    return scene_.get();
}