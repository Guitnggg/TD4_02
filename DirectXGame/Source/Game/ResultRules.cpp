#include "ResultRules.h"

#include <algorithm>
#include <filesystem>

std::string ResultRules::FindNextStagePath(const std::string& currentPath) {
	// ファイル名末尾の「_連番.csv」だけを置き換える。
	const size_t underscore = currentPath.find_last_of('_');
	const size_t extension = currentPath.rfind(".csv");
	if (underscore == std::string::npos || extension == std::string::npos || extension <= underscore + 1) {
		return {};
	}
	int stageNumber = 0;
	try {
		stageNumber = std::stoi(currentPath.substr(underscore + 1, extension - underscore - 1));
	} catch (...) {
		return {};
	}
	const int nextNumber = stageNumber + 1;
	// 既存のステージ名に合わせ、1桁の番号はゼロ埋めする。
	const std::string number = nextNumber < 10 ? "0" + std::to_string(nextNumber) : std::to_string(nextNumber);
	const std::string candidate = currentPath.substr(0, underscore + 1) + number + ".csv";
	return std::filesystem::exists(candidate) ? candidate : std::string{};
}

int ResultRules::GetStagePar(const std::string& stagePath) {
	// 難易度名から、星3獲得の基準となるギミック数を決める。
	if (stagePath.find("Tutorial_01.csv") != std::string::npos) {
		return 0;
	}
	if (stagePath.find("Tutorial_03.csv") != std::string::npos) {
		return 2;
	}
	if (stagePath.find("Tutorial") != std::string::npos || stagePath.find("Easy") != std::string::npos) {
		return 1;
	}
	if (stagePath.find("Normal") != std::string::npos) {
		return 2;
	}
	return 3;
}

int ResultRules::CalculateStarCount(int usedGimmickCount, const std::string& stagePath) {
	// 基準を超えたギミック1個につき星を1つ減らし、最低1つは保証する。
	const int safeCount = (std::max)(0, usedGimmickCount);
	const int overPar = (std::max)(0, safeCount - GetStagePar(stagePath));
	return std::clamp(3 - overPar, 1, 3);
}
