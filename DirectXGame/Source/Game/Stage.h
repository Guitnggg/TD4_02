#pragma once

#include <math/Vector3.h>

#include <vector>

class Stage {
public:
	enum class GimmickType {
		None,
		ReflectSlash,
		ReflectBackSlash,
	};

	struct GridPosition {
		int x = 0;
		int z = 0;
	};

	void InitializeTutorial();

	KamataEngine::Vector3 GridToWorld(const GridPosition& grid) const;
	GridPosition WorldToGrid(const KamataEngine::Vector3& position) const;

	bool IsInsideGrid(const GridPosition& grid) const;
	bool IsWall(const GridPosition& grid) const;
	GimmickType GetGimmick(const GridPosition& grid) const;
	bool IsGoal(const GridPosition& grid) const;

	int GetWidth() const { return width_; }
	int GetHeight() const { return height_; }
	float GetCellSize() const { return cellSize_; }
	const std::vector<GridPosition>& GetWalls() const { return walls_; }
	const std::vector<GridPosition>& GetPlaceableTiles() const { return placeableTiles_; }
	const std::vector<GridPosition>& GetReflectSlashTiles() const { return reflectSlashTiles_; }
	const std::vector<GridPosition>& GetReflectBackSlashTiles() const { return reflectBackSlashTiles_; }
	GridPosition GetPlayerStartGrid() const { return playerStartGrid_; }
	GridPosition GetGoalGrid() const { return goalGrid_; }
	int GetPlayerMinX() const { return playerMinX_; }
	int GetPlayerMaxX() const { return playerMaxX_; }

private:
	bool Contains(const std::vector<GridPosition>& grids, const GridPosition& target) const;

	int width_ = 0;
	int height_ = 0;
	float cellSize_ = 2.0f;
	GridPosition playerStartGrid_{};
	GridPosition goalGrid_{};
	int playerMinX_ = 0;
	int playerMaxX_ = 0;
	std::vector<GridPosition> walls_;
	std::vector<GridPosition> placeableTiles_;
	std::vector<GridPosition> reflectSlashTiles_;
	std::vector<GridPosition> reflectBackSlashTiles_;
};

