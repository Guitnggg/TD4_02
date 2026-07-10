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

	reflectSlashTiles_ = {
	    {3, 3},
	};
	reflectBackSlashTiles_ = {
	    {2, 2},
	};
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

bool Stage::Contains(const std::vector<GridPosition>& grids, const GridPosition& target) const {
	return std::any_of(grids.begin(), grids.end(), [target](const GridPosition& grid) { return grid.x == target.x && grid.z == target.z; });
}
