#pragma once

#include <memory>

#include "SceneName.h"

class IScene {
public:
    virtual ~IScene() = default;

	virtual void Initialize() = 0;

	virtual void Update() = 0;

	virtual void Draw() = 0;

	virtual bool IsEnd() const = 0;

	virtual std::unique_ptr<IScene> NextScene() const = 0;

	virtual SceneName GetSceneName() const = 0;
};
