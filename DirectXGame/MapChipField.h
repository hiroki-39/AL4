
#pragma once
#include "Math.h"
#include "KamataEngine.h"
#include <vector>

enum class MapChipType {
	kBlank, // 空白
	kBlock, // ブロック
};

// マップチップの構造体
struct MapChipData {

	std::vector<std::vector<MapChipType>> Data;
};

struct IndexSet {
	uint32_t xIndex;
	uint32_t yIndex;
};

// 範囲矩形
struct RangeRect {
	float left;   // 左端
	float right;  // 右端
	float bottom; // 下端
	float top;    // 上端
};

class MapChipField {
public:
	/// <summary>
	/// 読み込みデータのリセット
	/// </summary>
	void ResetMapChipData();

	/// <summary>
	/// CSVファイルパスを引数で指定して読み込む
	/// </summary>
	/// <param name="filePath"></param>
	void LoadMapChipCsv(const std::string& filePath);

	/// <summary>
	/// マップチップ種別の取得
	/// </summary>
	/// <param name="xIndex"></param>
	/// <param name="yIndex"></param>
	/// <returns></returns>
	MapChipType GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex);

	/// <summary>
	/// マップチップ座標の取得
	/// </summary>
	/// <param name="xIndex"></param>
	/// <param name="yIndex"></param>
	/// <returns></returns>
	KamataEngine::Vector3 GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex);

	IndexSet GetMapChipIndexSetByPosition(const KamataEngine::Vector3& position);

	RangeRect GetRectIndex(uint32_t xIndex, uint32_t yIndex);

	// アクセッサー
	uint32_t GetNumBlockVirtical() { return kNumBlockVirtical; };
	uint32_t GetNumBlockHorizontal() { return kNumBlockHorizontal; };

private:
	// マップチップのデータ
	MapChipData mapChipData_;

	// 1ブロックのサイズ
	static inline const float kBlockWidth = 2.0f;
	static inline const float kBlockHeight = 2.0f;

	// ブロックの個数
	static inline const uint32_t kNumBlockVirtical = 25;
	static inline const uint32_t kNumBlockHorizontal = 100;
};
