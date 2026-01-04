#include "MapChipField.h"
#include <cassert>
#include <fstream>
#include <map>
#include <sstream>
#include <cctype>

using namespace KamataEngine;

namespace {

// マップチップ用トークンテーブル
std::map<std::string, MapChipType> mapChipTable = {
    {"0", MapChipType::kBlank},
    {"1", MapChipType::kBlock},
};

// 敵トークンテーブル（CSV内で敵を表すトークンをここに追加）
std::map<std::string, EnemyType> enemyTable = {
    {"E1", EnemyType::kSlime},
    {"E2", EnemyType::kBat},
};

// 末尾の改行やスペース、CR を取り除く簡易トリム
inline void TrimToken(std::string& s) {
    // remove trailing whitespace
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) s.pop_back();
    // remove leading whitespace
    size_t i = 0;
    while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) ++i;
    if (i) s.erase(0, i);
}

} // namespace

void MapChipField::ResetMapChipData() {

	// マップチップデータをリセット
	mapChipData_.Data.clear();
	mapChipData_.Data.resize(kNumBlockVirtical);

	for (std::vector<MapChipType>& mapChipDataLine : mapChipData_.Data) {
		mapChipDataLine.resize(kNumBlockHorizontal);
	}

	// 敵データをリセット
	enemySpawns_.clear();
}

void MapChipField::LoadMapChipCsv(const std::string& filePath) {

	// マップチップデータをリセット
	ResetMapChipData();

	// ファイルを開く
	std::ifstream file;
	file.open(filePath);
	// ファイルが開けなかった場合の処理
	assert(file.is_open());

	// マップチップCSV
	std::stringstream mapChipCsv;
	// ファイルの内容を文字列にストリームにコピー
	mapChipCsv << file.rdbuf();
	// ファイルを閉じる
	file.close();

	// CSVからマップチップデータを読み込む
	for (uint32_t y = 0; y < kNumBlockVirtical; ++y) {
		std::string line;
		if (!std::getline(mapChipCsv, line)) {
			// 行が足りない場合は残り放置（既に kBlank 等で初期化されている）
			break;
		}

		// 1行分の文字列をストリームに変換
		std::stringstream line_Stream(line);

		for (int32_t x = 0; x < kNumBlockHorizontal; ++x) {

			std::string word;
			if (!std::getline(line_Stream, word, ',')) {
				// 列が足りない場合は続行
				break;
			}

			TrimToken(word);

			// マップチップトークンなら設定
			if (!word.empty() && mapChipTable.contains(word)) {
				mapChipData_.Data[y][x] = mapChipTable[word];
			}
			// マップチップではなく敵トークンなら敵情報を登録し、マップは空白にする
			else if (!word.empty() && enemyTable.contains(word)) {
				EnemySpawn spawn;
				spawn.type = enemyTable[word];
				spawn.index.xIndex = static_cast<uint32_t>(x);
				spawn.index.yIndex = static_cast<uint32_t>(y);
				enemySpawns_.push_back(spawn);

				// 敵のあるセルはマップ上は空白扱い（必要なら他の扱いに変更）
				mapChipData_.Data[y][x] = MapChipType::kBlank;
			}
			// それ以外は既定値（初期化済み）
		}
	}
}

MapChipType MapChipField::GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex) {

	if (xIndex < 0 || kNumBlockHorizontal - 1 < xIndex) {
		return MapChipType::kBlank;
	}

	if (yIndex < 0 || kNumBlockVirtical - 1 < yIndex) {
		return MapChipType::kBlank;
	}

	return mapChipData_.Data[yIndex][xIndex];
}

Vector3 MapChipField::GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex) { return Vector3(kBlockWidth * xIndex, kBlockHeight * (kNumBlockVirtical - 1 - yIndex), 0); }

IndexSet MapChipField::GetMapChipIndexSetByPosition(const Vector3& position) {

	IndexSet indexSet = {};

	indexSet.xIndex = static_cast<uint32_t>((position.x + kBlockWidth / 2.0f) / kBlockWidth);
	indexSet.yIndex = kNumBlockVirtical - 1 - static_cast<uint32_t>((position.y + kBlockHeight / 2.0f) / kBlockHeight);

	return indexSet;
}

RangeRect MapChipField::GetRectIndex(uint32_t xIndex, uint32_t yIndex) {
	// 指定ブロックの中心座標を取得
	Vector3 center = GetMapChipPositionByIndex(xIndex, yIndex);

	RangeRect rect;
	rect.left = center.x - kBlockWidth / 2.0f;
	rect.right = center.x + kBlockWidth / 2.0f;
	rect.bottom = center.y - kBlockHeight / 2.0f;
	rect.top = center.y + kBlockHeight / 2.0f;

	return rect;
}