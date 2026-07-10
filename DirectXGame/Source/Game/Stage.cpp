#include "Stage.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <regex>
#include <sstream>

using namespace KamataEngine;

namespace {
std::string ReadTextFile(const std::string& filePath) {
	std::ifstream file(filePath);
	if (!file.is_open()) {
		return {};
	}

	std::ostringstream stream;
	stream << file.rdbuf();
	return stream.str();
}

std::string DecodeJsonString(const std::string& value) {
	std::string decoded;
	decoded.reserve(value.size());

	for (size_t i = 0; i < value.size(); ++i) {
		if (value[i] != '\\' || i + 1 >= value.size()) {
			decoded.push_back(value[i]);
			continue;
		}

		++i;
		switch (value[i]) {
		case '\\':
		case '"':
		case '/':
			decoded.push_back(value[i]);
			break;
		case 'n':
			decoded.push_back('\n');
			break;
		default:
			decoded.push_back(value[i]);
			break;
		}
	}

	return decoded;
}

bool ExtractStringValue(const std::string& json, const std::string& key, std::string& value) {
	const std::regex pattern("\"" + key + "\"\\s*:\\s*\"((?:\\\\.|[^\"])*)\"");
	std::smatch match;
	if (!std::regex_search(json, match, pattern)) {
		return false;
	}

	value = DecodeJsonString(match[1].str());
	return true;
}

bool ExtractIntValue(const std::string& json, const std::string& key, int& value) {
	const std::regex pattern("\"" + key + "\"\\s*:\\s*(-?\\d+)");
	std::smatch match;
	if (!std::regex_search(json, match, pattern)) {
		return false;
	}

	value = std::stoi(match[1].str());
	return true;
}

bool ExtractFloatValue(const std::string& json, const std::string& key, float& value) {
	const std::regex pattern("\"" + key + "\"\\s*:\\s*(-?\\d+(?:\\.\\d+)?)");
	std::smatch match;
	if (!std::regex_search(json, match, pattern)) {
		return false;
	}

	value = std::stof(match[1].str());
	return true;
}

bool ExtractStringArray(const std::string& json, const std::string& key, std::vector<std::string>& values) {
	const std::regex arrayPattern("\"" + key + "\"\\s*:\\s*\\[([\\s\\S]*?)\\]");
	std::smatch arrayMatch;
	if (!std::regex_search(json, arrayMatch, arrayPattern)) {
		return false;
	}

	values.clear();
	const std::string arrayBody = arrayMatch[1].str();
	const std::regex stringPattern("\"((?:\\\\.|[^\"])*)\"");
	for (std::sregex_iterator iterator(arrayBody.begin(), arrayBody.end(), stringPattern), end; iterator != end; ++iterator) {
		values.push_back(DecodeJsonString((*iterator)[1].str()));
	}

	return !values.empty();
}

std::string ResolveResourcePath(const std::string& stageFilePath, const std::string& resourcePath) {
	if (resourcePath.find(':') != std::string::npos || resourcePath.starts_with("./") || resourcePath.starts_with("../")) {
		return resourcePath;
	}

	std::string normalizedPath = resourcePath;
	std::replace(normalizedPath.begin(), normalizedPath.end(), '/', '\\');

	const std::string resourcesMarker = "Resources\\";
	if (normalizedPath.starts_with(resourcesMarker)) {
		return normalizedPath;
	}

	const size_t resourcesPos = stageFilePath.find(resourcesMarker);
	if (resourcesPos == std::string::npos) {
		return normalizedPath;
	}

	return stageFilePath.substr(0, resourcesPos) + "Resources\\" + normalizedPath;
}
} // namespace

void Stage::InitializeTutorial() {
	if (LoadFromJson("Resources/Stages/Tutorial/Tutorial.json") || LoadFromJson("DirectXGame/Resources/Stages/Tutorial/Tutorial.json")) {
		return;
	}

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

bool Stage::LoadFromJson(const std::string& stageFilePath) {
	const std::string stageJson = ReadTextFile(stageFilePath);
	if (stageJson.empty()) {
		return false;
	}

	std::string mapPath;
	if (!ExtractStringValue(stageJson, "map", mapPath)) {
		return false;
	}

	const std::string mapJson = ReadTextFile(ResolveResourcePath(stageFilePath, mapPath));
	if (mapJson.empty()) {
		return false;
	}

	std::vector<std::string> rows;
	if (!ExtractStringArray(mapJson, "tiles", rows)) {
		return false;
	}

	const int loadedHeight = static_cast<int>(rows.size());
	const int loadedWidth = static_cast<int>(rows.front().size());
	if (loadedWidth <= 0 || loadedHeight <= 0) {
		return false;
	}

	for (const std::string& row : rows) {
		if (static_cast<int>(row.size()) != loadedWidth) {
			return false;
		}
	}

	std::vector<GridPosition> loadedWalls;
	std::vector<GridPosition> loadedPlaceableTiles;
	std::vector<GridPosition> loadedReflectSlashTiles;
	std::vector<GridPosition> loadedReflectBackSlashTiles;
	GridPosition loadedPlayerStart{};
	GridPosition loadedGoal{};
	bool hasPlayerStart = false;
	bool hasGoal = false;

	for (int z = 0; z < loadedHeight; ++z) {
		for (int x = 0; x < loadedWidth; ++x) {
			const GridPosition grid{x, z};
			switch (rows[z][x]) {
			case '#':
				loadedWalls.push_back(grid);
				break;
			case '*':
				loadedPlaceableTiles.push_back(grid);
				break;
			case '/':
				loadedPlaceableTiles.push_back(grid);
				loadedReflectSlashTiles.push_back(grid);
				break;
			case '\\':
				loadedPlaceableTiles.push_back(grid);
				loadedReflectBackSlashTiles.push_back(grid);
				break;
			case 'P':
				loadedPlayerStart = grid;
				hasPlayerStart = true;
				break;
			case 'G':
				loadedGoal = grid;
				hasGoal = true;
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
	cellSize_ = 2.0f;
	playerStartGrid_ = loadedPlayerStart;
	goalGrid_ = loadedGoal;
	playerMinX_ = 0;
	playerMaxX_ = width_ - 1;

	ExtractFloatValue(stageJson, "cellSize", cellSize_);
	ExtractIntValue(stageJson, "playerMinX", playerMinX_);
	ExtractIntValue(stageJson, "playerMaxX", playerMaxX_);

	playerMinX_ = std::clamp(playerMinX_, 0, width_ - 1);
	playerMaxX_ = std::clamp(playerMaxX_, playerMinX_, width_ - 1);

	walls_ = std::move(loadedWalls);
	placeableTiles_ = std::move(loadedPlaceableTiles);
	reflectSlashTiles_ = std::move(loadedReflectSlashTiles);
	reflectBackSlashTiles_ = std::move(loadedReflectBackSlashTiles);

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
	return GimmickType::None;
}

bool Stage::IsGoal(const GridPosition& grid) const { return goalGrid_.x == grid.x && goalGrid_.z == grid.z; }

bool Stage::Contains(const std::vector<GridPosition>& grids, const GridPosition& target) const {
	return std::any_of(grids.begin(), grids.end(), [target](const GridPosition& grid) { return grid.x == target.x && grid.z == target.z; });
}
