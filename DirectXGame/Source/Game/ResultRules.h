#pragma once
#include <string>

namespace ResultRules {
std::string FindNextStagePath(const std::string& currentPath);
int GetStagePar(const std::string& stagePath);
int CalculateStarCount(int usedGimmickCount, const std::string& stagePath);
}
