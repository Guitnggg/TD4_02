#include "Stage.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>

using namespace KamataEngine;

namespace {
// 読み込み途中でStageを書き換えないよう、CSV全体を取得してから解析する。
std::string ReadTextFile(const std::string& filePath) {
	std::ifstream file(filePath);
	if (!file.is_open()) {
		return {};
	}

	std::ostringstream stream;
	stream << file.rdbuf();
	return stream.str();
}

std::string Trim(const std::string& text) {
	const size_t first = text.find_first_not_of(" \t\r\n");
	if (first == std::string::npos) {
		return {};
	}

	const size_t last = text.find_last_not_of(" \t\r\n");
	return text.substr(first, last - first + 1);
}

bool ParseCsvTileMap(const std::string& csv, std::vector<std::vector<int>>& rows) {
	// 不正な行を含むマップを部分的に読み込まず、全体をエラーとして扱う。
	rows.clear();

	std::istringstream csvStream(csv);
	std::string line;
	while (std::getline(csvStream, line)) {
		line = Trim(line);
		if (line.empty()) {
			continue;
		}

		std::vector<int> row;
		std::istringstream lineStream(line);
		std::string cell;
		while (std::getline(lineStream, cell, ',')) {
			cell = Trim(cell);
			if (cell.empty()) {
				return false;
			}
			try {
				size_t parsedLength = 0;
				const int tile = std::stoi(cell, &parsedLength);
				if (parsedLength != cell.size() || tile < 0 || tile > 7) { return false; }
				row.push_back(tile);
			} catch (...) {
				return false;
			}
		}

		if (row.empty()) {
			return false;
		}
		rows.push_back(std::move(row));
	}

	return !rows.empty();
}

} // namespace

bool Stage::LoadFromCsv(const std::string& stageFilePath) {
	// 一時コンテナへ解析し、検証が完了してからメンバーへ反映する。
	const std::string mapText = ReadTextFile(stageFilePath);
	if (mapText.empty()) {
		return false;
	}

	std::vector<std::vector<int>> rows;
	if (!ParseCsvTileMap(mapText, rows)) {
		return false;
	}

	const int loadedHeight = static_cast<int>(rows.size());
	const int loadedWidth = static_cast<int>(rows.front().size());
	if (loadedWidth <= 0 || loadedHeight <= 0) {
		return false;
	}

	for (const std::vector<int>& row : rows) {
		if (static_cast<int>(row.size()) != loadedWidth) {
			return false;
		}
	}

	std::vector<GridPosition> loadedWalls;
	std::vector<GridPosition> loadedPlaceableTiles;
	std::vector<GridPosition> loadedReflectSlashTiles;
	std::vector<GridPosition> loadedReflectBackSlashTiles;
	std::vector<AccelerationPanel> loadedAccelerationPanels;
	GridPosition loadedPlayerStart{};
	GridPosition loadedGoal{};
	bool hasPlayerStart = false;
	bool hasGoal = false;

	for (int z = 0; z < loadedHeight; ++z) {
		// CSVの数値から、固定マップ要素と初期ギミックを判別する。
		for (int x = 0; x < loadedWidth; ++x) {
			const GridPosition grid{x, z};
			switch (rows[z][x]) {
			case 0:
				// 通常床もギミック配置可能にし、ステージ攻略の自由度を確保する。
				loadedPlaceableTiles.push_back(grid);
				break;
			case 1:
				loadedWalls.push_back(grid);
				break;
			case 2:
				loadedPlayerStart = grid;
				hasPlayerStart = true;
				break;
			case 3:
				loadedGoal = grid;
				hasGoal = true;
				break;
			case 4:
				loadedPlaceableTiles.push_back(grid);
				break;
			case 5:
				loadedPlaceableTiles.push_back(grid);
				loadedReflectSlashTiles.push_back(grid);
				break;
			case 6:
				loadedPlaceableTiles.push_back(grid);
				loadedReflectBackSlashTiles.push_back(grid);
				break;
			case 7:
				loadedPlaceableTiles.push_back(grid);
				loadedAccelerationPanels.emplace_back(grid.x, grid.z);
				break;
			default:
				break;
			}
		}
	}

	if (!hasPlayerStart || !hasGoal) {
		return false;
	}

	width_ = loadedWidth;
	height_ = loadedHeight;
	cellSize_ = 1.0f;
	playerStartGrid_ = loadedPlayerStart;
	goalGrid_ = loadedGoal;
	// 発射位置はステージで指定された開始位置を中心に、左右1マスだけ調整できる。
	playerMinX_ = (std::max)(0, playerStartGrid_.x - 1);
	playerMaxX_ = (std::min)(width_ - 1, playerStartGrid_.x + 1);

	walls_ = std::move(loadedWalls);
	placeableTiles_ = std::move(loadedPlaceableTiles);
	reflectSlashTiles_ = std::move(loadedReflectSlashTiles);
	reflectBackSlashTiles_ = std::move(loadedReflectBackSlashTiles);
	accelerationPanels_ = std::move(loadedAccelerationPanels);

	// ギミックはプレイヤーが発射前に配置するため、初期状態では空にする
	reflectSlashTiles_.clear();
	reflectBackSlashTiles_.clear();
	accelerationPanels_.clear();

	return true;
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
	if (FindAccelerationPanel(grid) != nullptr) {
		return GimmickType::AccelerationPanel;
	}
	return GimmickType::None;
}

bool Stage::IsGoal(const GridPosition& grid) const { return goalGrid_.x == grid.x && goalGrid_.z == grid.z; }

bool Stage::IsPlaceable(const GridPosition& grid) const { return Contains(placeableTiles_, grid); }

bool Stage::CanPlaceGimmick(const GridPosition& grid) const {
	return IsInsideGrid(grid) && IsPlaceable(grid) && !IsWall(grid) && !IsGoal(grid);
}

bool Stage::PlaceGimmick(const GridPosition& grid, GimmickType type, AccelerationPanel::Direction panelDirection) {
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
	if (type == GimmickType::AccelerationPanel) {
		accelerationPanels_.emplace_back(grid.x, grid.z, panelDirection);
		return true;
	}

	return false;
}

bool Stage::RemoveGimmick(const GridPosition& grid) {
	const bool removedSlash = RemoveFromList(reflectSlashTiles_, grid);
	const bool removedBackSlash = RemoveFromList(reflectBackSlashTiles_, grid);
	const auto oldPanelEnd = accelerationPanels_.end();
	const auto newPanelEnd = std::remove_if(accelerationPanels_.begin(), accelerationPanels_.end(),
		[grid](const AccelerationPanel& panel) { return panel.IsAt(grid.x, grid.z); });
	const bool removedPanel = newPanelEnd != oldPanelEnd;
	accelerationPanels_.erase(newPanelEnd, oldPanelEnd);
	return removedSlash || removedBackSlash || removedPanel;
}

void Stage::ClearGimmicks() {
	reflectSlashTiles_.clear();
	reflectBackSlashTiles_.clear();
	accelerationPanels_.clear();
}

int Stage::GetPlacedGimmickCount() const {
	return static_cast<int>(reflectSlashTiles_.size() + reflectBackSlashTiles_.size() + accelerationPanels_.size());
}

const AccelerationPanel* Stage::FindAccelerationPanel(const GridPosition& grid) const {
	// コピーを避け、Playerから床の処理を直接呼べるようポインターを返す。
	const auto it = std::find_if(accelerationPanels_.begin(), accelerationPanels_.end(),
		[grid](const AccelerationPanel& panel) { return panel.IsAt(grid.x, grid.z); });
	return it == accelerationPanels_.end() ? nullptr : &(*it);
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
