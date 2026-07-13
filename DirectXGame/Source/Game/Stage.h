#pragma once

#include <math/Vector3.h>

#include <string>
#include <vector>

/// <summary>
/// ステージのグリッド情報、配置物、座標変換を管理するクラス
///
/// ・ステージサイズとセルサイズ
/// ・壁、設置可能マス、反射ギミック、ゴール位置
/// ・グリッド座標とワールド座標の変換
/// を担当する
/// </summary>
class Stage {
public:
	/// <summary>
	/// グリッド上に配置されているギミックの種類
	/// </summary>
	enum class GimmickType {
		None,              // ギミックなし
		ReflectSlash,      // 右上がり方向へ反射するギミック
		ReflectBackSlash,  // 右下方向へ反射するギミック
	};

	/// <summary>
	/// ステージ上のマスを表すグリッド座標
	/// </summary>
	struct GridPosition {
		int x = 0;  // 横方向のマス位置
		int z = 0;  // 縦方向のマス位置
	};

	/// <summary>
	/// チュートリアル用ステージの配置で初期化する
	/// </summary>
	bool LoadFromCsv(const std::string& stageFilePath);

	/// <summary>
	/// グリッド座標をワールド座標へ変換する
	/// </summary>
	/// <param name="grid">変換するグリッド座標</param>
	/// <returns>グリッド中央のワールド座標</returns>
	KamataEngine::Vector3 GridToWorld(const GridPosition& grid) const;

	/// <summary>
	/// ワールド座標を最も近いグリッド座標へ変換する
	/// </summary>
	/// <param name="position">変換するワールド座標</param>
	/// <returns>ワールド座標に対応するグリッド座標</returns>
	GridPosition WorldToGrid(const KamataEngine::Vector3& position) const;

	/// <summary>
	/// 指定したグリッド座標がステージ範囲内か判定する
	/// </summary>
	/// <param name="grid">判定するグリッド座標</param>
	/// <returns>範囲内なら true</returns>
	bool IsInsideGrid(const GridPosition& grid) const;

	/// <summary>
	/// 指定したグリッド座標が壁か判定する
	/// </summary>
	/// <param name="grid">判定するグリッド座標</param>
	/// <returns>壁なら true</returns>
	bool IsWall(const GridPosition& grid) const;

	/// <summary>
	/// 指定したグリッド座標にあるギミックの種類を取得する
	/// </summary>
	/// <param name="grid">確認するグリッド座標</param>
	/// <returns>配置されているギミックの種類</returns>
	GimmickType GetGimmick(const GridPosition& grid) const;

	/// <summary>
	/// 指定したグリッド座標がゴールか判定する
	/// </summary>
	/// <param name="grid">判定するグリッド座標</param>
	/// <returns>ゴールなら true</returns>
	bool IsGoal(const GridPosition& grid) const;

	/// <summary>
	/// ステージの横幅を取得する
	/// </summary>
	int GetWidth() const { return width_; }

	/// <summary>
	/// ステージの奥行き幅を取得する
	/// </summary>
	int GetHeight() const { return height_; }

	/// <summary>
	/// 1マスあたりのワールド座標上の大きさを取得する
	/// </summary>
	float GetCellSize() const { return cellSize_; }

	/// <summary>
	/// 壁が配置されているグリッド座標一覧を取得する
	/// </summary>
	const std::vector<GridPosition>& GetWalls() const { return walls_; }

	/// <summary>
	/// ギミックを設置できるグリッド座標一覧を取得する
	/// </summary>
	const std::vector<GridPosition>& GetPlaceableTiles() const { return placeableTiles_; }

	/// <summary>
	/// ReflectSlash ギミックのグリッド座標一覧を取得する
	/// </summary>
	const std::vector<GridPosition>& GetReflectSlashTiles() const { return reflectSlashTiles_; }

	/// <summary>
	/// ReflectBackSlash ギミックのグリッド座標一覧を取得する
	/// </summary>
	const std::vector<GridPosition>& GetReflectBackSlashTiles() const { return reflectBackSlashTiles_; }

	/// <summary>
	/// プレイヤー開始位置のグリッド座標を取得する
	/// </summary>
	GridPosition GetPlayerStartGrid() const { return playerStartGrid_; }

	/// <summary>
	/// ゴール位置のグリッド座標を取得する
	/// </summary>
	GridPosition GetGoalGrid() const { return goalGrid_; }

	/// <summary>
	/// 発射前にプレイヤーが移動できる最小 X 座標を取得する
	/// </summary>
	int GetPlayerMinX() const { return playerMinX_; }

	/// <summary>
	/// 発射前にプレイヤーが移動できる最大 X 座標を取得する
	/// </summary>
	int GetPlayerMaxX() const { return playerMaxX_; }

private:
	/// <summary>
	/// 指定したグリッド座標一覧に対象座標が含まれているか判定する
	/// </summary>
	/// <param name="grids">検索対象のグリッド座標一覧</param>
	/// <param name="target">検索するグリッド座標</param>
	/// <returns>含まれていれば true</returns>
	bool Contains(const std::vector<GridPosition>& grids, const GridPosition& target) const;

	// ステージの横幅
	int width_ = 0;

	// ステージの奥行き幅
	int height_ = 0;

	// 1マスあたりのワールド座標上の大きさ
	float cellSize_ = 2.0f;

	// プレイヤー開始位置のグリッド座標
	GridPosition playerStartGrid_{};

	// ゴール位置のグリッド座標
	GridPosition goalGrid_{};

	// 発射前にプレイヤーが移動できる最小 X 座標
	int playerMinX_ = 0;

	// 発射前にプレイヤーが移動できる最大 X 座標
	int playerMaxX_ = 0;

	// 壁が配置されているグリッド座標一覧
	std::vector<GridPosition> walls_;

	// ギミックを設置できるグリッド座標一覧
	std::vector<GridPosition> placeableTiles_;

	// ReflectSlash ギミックのグリッド座標一覧
	std::vector<GridPosition> reflectSlashTiles_;

	// ReflectBackSlash ギミックのグリッド座標一覧
	std::vector<GridPosition> reflectBackSlashTiles_;
};
