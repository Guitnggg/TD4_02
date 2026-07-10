#include "Stage.h"

#include <algorithm>
#include <cmath>

using namespace KamataEngine;

void Stage::InitializeTutorial() {
	width_ = 5;
	height_ = 5;
	cellSize_ = 2.0f;
	playerStartGrid_ = {2, 4};
	goalGrid_ = {2, 0};
	playerMinX_ = 0;
	playerMaxX_ = width_ - 1;

	walls_ = {
	    {0, 2},
	    {4, 2},
	    {1, 1},
	};

	placeableTiles_ = {
	    {1, 3},
	    {3, 3},
	    {2, 2},
	    {3, 1},
	};

	// ギミックはプレイヤーが発射前に配置するため、初期状態では空にする
	reflectSlashTiles_.clear();
	reflectBackSlashTiles_.clear();
}

Vector3 Stage::GridToWorld(const GridPosition& grid) const {
	const float originX = -static_cast<float>(width_ - 1) * cellSize_ * 0.5f;
	const float originZ = -static_cast<float>(height_ - 1) * cellSize_ * 0.5f;
	return {originX + static_cast<float>(grid.x) * cellSize_, 0.0f, originZ + static_cast<float>(grid.z) * cellSize_};
}

Stage::GridPosition Stage::WorldToGrid(const Vector3& position) const {
	const float originX = -static_cast<float>(width_ - 1) * cellSize_ * 0.5f;
	const float originZ = -static_cast<float>(height_ - 1) * cellSize_ * 0.5f;
	return {
	    static_cast<int>(std::round((position.x - originX) / cellSize_)),
	    static_cast<int>(std::round((position.z - originZ) / cellSize_)),
	};
}

bool Stage::IsInsideGrid(const GridPosition& grid) const {
	return 0 <= grid.x && grid.x < width_ && 0 <= grid.z && grid.z < height_;
}

bool Stage::IsWall(const GridPosition& grid) const { return Contains(walls_, grid); }

Stage::GimmickType Stage::GetGimmick(const GridPosition& grid) const {
	if (Contains(reflectSlashTiles_, grid)) {
		return GimmickType::ReflectSlash;
	}
	if (Contains(reflectBackSlashTiles_, grid)) {
		return GimmickType::ReflectBackSlash;
	}
	return GimmickType::None;
}

bool Stage::IsGoal(const GridPosition& grid) const { return goalGrid_.x == grid.x && goalGrid_.z == grid.z; }

bool Stage::IsPlaceable(const GridPosition& grid) const { return Contains(placeableTiles_, grid); }

bool Stage::CanPlaceGimmick(const GridPosition& grid) const {
	return IsInsideGrid(grid) && IsPlaceable(grid) && !IsWall(grid) && !IsGoal(grid);
}

bool Stage::PlaceGimmick(const GridPosition& grid, GimmickType type) {
	if (type == GimmickType::None) {
		return RemoveGimmick(grid);
	}
	if (!CanPlaceGimmick(grid)) {
		return false;
	}

	// 同じマスに別向きのギミックが残らないように先に削除する
	RemoveGimmick(grid);

	if (type == GimmickType::ReflectSlash) {
		reflectSlashTiles_.push_back(grid);
		return true;
	}
	if (type == GimmickType::ReflectBackSlash) {
		reflectBackSlashTiles_.push_back(grid);
		return true;
	}

	return false;
}

bool Stage::RemoveGimmick(const GridPosition& grid) {
	const bool removedSlash = RemoveFromList(reflectSlashTiles_, grid);
	const bool removedBackSlash = RemoveFromList(reflectBackSlashTiles_, grid);
	return removedSlash || removedBackSlash;
}

void Stage::ClearGimmicks() {
	reflectSlashTiles_.clear();
	reflectBackSlashTiles_.clear();
}

int Stage::GetPlacedGimmickCount() const {
	return static_cast<int>(reflectSlashTiles_.size() + reflectBackSlashTiles_.size());
}

bool Stage::Contains(const std::vector<GridPosition>& grids, const GridPosition& target) const {
	return std::any_of(grids.begin(), grids.end(), [target](const GridPosition& grid) { return grid.x == target.x && grid.z == target.z; });
}

bool Stage::RemoveFromList(std::vector<GridPosition>& grids, const GridPosition& target) {
	const auto oldEnd = grids.end();
	const auto newEnd = std::remove_if(grids.begin(), grids.end(), [target](const GridPosition& grid) { return grid.x == target.x && grid.z == target.z; });
	if (newEnd == oldEnd) {
		return false;
	}

	grids.erase(newEnd, oldEnd);
	return true;
}
