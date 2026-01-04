#pragma once
#include "CameraController.h"
#include "Fade.h"
#include "KamataEngine.h"
#include "MapChipField.h"
#include "Math.h"
#include "Player.h"
#include "Skydome.h"
#include "enemy.h"
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

	void CheckAllCollisions();

	void ChangePhase();

	// デスフラグのgetter
	bool IsFinished() const { return finished_; }

private:
	/*-------------- シーン --------------*/
	// ゲームのフェーズ
	enum class Phase {
		// フェードイン
		kFadeIn,
		// ゲームプレイ
		kPlay,
		// デス演出
		kDeath,
		// フェードアウト
		kFadeOut,
	};

	Phase phase_;

	// 終了フラグ
	bool finished_ = false;

	// フェード
	Fade* fade_ = nullptr;

	/*---自機---*/
	Player* player_ = nullptr;

	// プレイヤーのモデル
	KamataEngine::Model* modelPlayer_ = nullptr;

	/*-------------- 敵mob --------------*/
	std::list<Enemy*> enemies_;

	// 敵のモデル
	KamataEngine::Model* modelEnemy_ = nullptr;

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

	/*--- UI ---*/

	uint32_t numberTexHandle[10];

	KamataEngine::Sprite* numberSprite[10];

	uint32_t maxNumberTexHandle[10];

	KamataEngine::Sprite* maxNumberSprite[10];

	uint32_t slashTexHandle;

	KamataEngine::Sprite* slashSprite;

	// リロードUI
	uint32_t reloadTexHandle;

	KamataEngine::Sprite* reloadSprite;

	//操作方法UI
	uint32_t operationTexHandle;
	KamataEngine::Sprite* operationSprite;

	//目標UI
	uint32_t targetTexHandle;
	KamataEngine::Sprite* targetSprite;

	// 敵の数用UI
	uint32_t enemyCountTexHandle[10];
	KamataEngine::Sprite* enemyCountSprite[10];

	/*---関数---*/
	Math* math;
};