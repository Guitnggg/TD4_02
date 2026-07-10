#pragma once

#include "IScene.h"
#include "SceneName.h"

#include <memory>

class SceneManager {
public:
    SceneManager() = default;

    explicit SceneManager(std::unique_ptr<IScene> firstScene);

    ~SceneManager() = default;

    SceneManager(const SceneManager&) = delete;

    SceneManager& operator=(const SceneManager&) = delete;

    SceneManager(SceneManager&&) = default;

    SceneManager& operator=(SceneManager&&) = default;

    void SetScene(std::unique_ptr<IScene> nextScene);

    void Update();

    void Draw();

    bool IsEnd() const;

    SceneName GetSceneName() const;

    IScene* GetScene() const;

private:
    std::unique_ptr<IScene> scene_;
};
