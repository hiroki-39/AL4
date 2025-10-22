#pragma once
#include "CameraController.h"
#include "Math.h"
#include "KamataEngine.h"
#include "MapChipField.h"
#include "Player.h"
#include "Skydome.h"
#include <vector>

class GameScene {
public:
	/// <summary>
	///	ゲームシーンの初期化
	/// </summary>
	void Initialize();

	/// <summary>
	///	ゲームシーンの更新
	/// </summary>
	void Update();

	/// <summary>
	/// ゲームシーンの描画
	///	</summary>
	void Draw();

	///< summary>
	///	デストラクタ
	/// </summary>
	~GameScene();

	///< summary>
	// 表示ブロックの生成
	///< summary>
	void GenetateBlocks();

private:
	/*---自機---*/
	Player* player_ = nullptr;

	// プレイヤーのモデル
	KamataEngine::Model* modelPlayer_ = nullptr;

	/*---ブロック---*/
	std::vector<std::vector<KamataEngine::WorldTransform*>> worldTransformBlocks_;

	// ブロックのモデル
	KamataEngine::Model* modelBlock_ = nullptr;

	/*---スカイドーム---*/

	Skydome* skydome_ = nullptr;

	// スカイドームのモデル
	KamataEngine::Model* modelSkydome_ = nullptr;

	/*---マップチップフィールド---*/

	MapChipField* mapchipField_;

	/*---デバックカメラ---*/

	// デバックカメラの有効
	bool isDebugCameraActive_ = false;

	// デバックカメラ
	KamataEngine::DebugCamera* debugCamera_ = nullptr;

	// カメラ
	KamataEngine::Camera camera_;

	// カメラコントロール
	CameraController* cameraController_ = nullptr;

	/*---関数---*/
	Math* math;
};