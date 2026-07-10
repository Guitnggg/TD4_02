#include "MathUtility.h"

#include <3d/WorldTransform.h>

using namespace KamataEngine;

void WorldTransform::UpdateMatrix() {
	matWorld_ = MyMath::MakeAffineMatrix(scale_, rotation_, translation_);

	if (parent_) {
		matWorld_ = MyMath::Multiply(matWorld_, parent_->matWorld_);
	}

	TransferMatrix();
}
