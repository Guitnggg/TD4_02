#include "ReflectiveBlock.h"

ReflectiveBlock::~ReflectiveBlock() {}

ReflectiveBlock::ReflectiveBlock() {}

void ReflectiveBlock::initialize() {
	//モデル取り込み
	model_ = KamataEngine::Model::CreateFromOBJ("cube");
	model_->StaticInitialize();

	input_->Initialize();
	isCatch_ = false;

	
}

void ReflectiveBlock::Update() {

	
}

void ReflectiveBlock::Draw() {}


bool ReflectiveBlock::IsPressingLeft(KamataEngine::Input* input) const { return input != nullptr && input->IsPressMouse(0); }

bool ReflectiveBlock::IsTriggerLeft(KamataEngine::Input* input) const { return input != nullptr && input->IsTriggerMouse(0); }

bool ReflectiveBlock::IsReleaseLeft(KamataEngine::Input* input) const { return wasPressingLeft_ && !IsPressingLeft(input); }
