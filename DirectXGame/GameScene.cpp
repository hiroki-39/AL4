#include "GameScene.h"

using namespace KamataEngine;

// ゲームシーンの初期化
void GameScene::Initialize() {

#pragma region "マップチップ"
	/*-------------- マップチップの初期化 --------------*/
	mapchipField_ = new MapChipField;
	mapchipField_->LoadMapChipCsv("Resources/maps/maps.csv");

#pragma endregion

#pragma region "プレイヤー"
	/*-------------- プレイヤーの初期化 --------------*/

	// 3Dモデルの生成
	modelPlayer_ = Model::CreateFromOBJ("Player", true);

	// プレイヤーの生成
	player_ = new Player();

	// 座標をマップチップ番号で取得
	Vector3 playerPosition = mapchipField_->GetMapChipPositionByIndex(5, 18);

	// プレイヤーの初期化
	player_->Initialize(modelPlayer_, &camera_, playerPosition);

	// マップチップデータのセット
	player_->SetMapChipField(mapchipField_);

#pragma endregion

#pragma region "ブロック"
	/*-------------- ブロックの初期化 --------------*/

	// 3Dモデルの生成
	modelBlock_ = Model::CreateFromOBJ("Block", true);

	// ブロックの生成
	GenetateBlocks();

#pragma endregion

#pragma region "スカイドーム"
	/*-------------- スカイドームの初期化 --------------*/

	// スカイドームのモデルの生成
	modelSkydome_ = Model::CreateFromOBJ("SkyDome", true);

	// スカイドームの生成
	skydome_ = new Skydome();

	// スカイドームの初期化
	skydome_->Initialize(modelSkydome_, &camera_);

#pragma endregion

#pragma region "カメラ"

	/*-------------- カメラの初期化 --------------*/
	// デバックカメラの生成
	debugCamera_ = new DebugCamera(1280, 720);

	camera_.farZ = 1000.0f; // カメラの奥行き

	// カメラの初期化
	camera_.Initialize();

	// カメラコントローラーの生成
	cameraController_ = new CameraController();

	// カメラコントローラーの初期化
	cameraController_->Initialize(&camera_);

	// 追尾対象を設定
	cameraController_->SetTarget(player_);

	// リセット
	cameraController_->Reset();

#pragma endregion
}

// ゲームシーンの更新
void GameScene::Update() {

	// プレイヤーの更新
	player_->Update();

	// ブロックの更新
	for (std::vector<WorldTransform*>& worldTransBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransBlockLine) {
			if (!worldTransformBlock) {
				continue;
			}

			math->worldTransformUpdate(*worldTransformBlock);
		}
	}

	// スカイドームの更新
	skydome_->Update();

#ifdef _DEBUG

	if (Input::GetInstance()->TriggerKey(DIK_TAB)) {
		// デバックカメラの有効
		isDebugCameraActive_ = true;
	}

#endif

	// カメラの更新
	if (isDebugCameraActive_) {
		// デバックカメラの更新
		debugCamera_->Update();

		// カメラのワールド行列を取得
		camera_.matView = debugCamera_->GetCamera().matView;

		// カメラのプロジェクション行列を取得
		camera_.matProjection = debugCamera_->GetCamera().matProjection;

		// カメラのビュー行列を転送
		camera_.TransferMatrix();

	} else {
		// ビュープロジェクション行列の更新と転送
		camera_.UpdateMatrix();
	}

	// カメラコントローラーの更新
	cameraController_->Update();
}

// ゲームシーンの描画
void GameScene::Draw() {

	// 3Dモデル描画前処理
	Model::PreDraw(Model::CullingMode::kBack, Model::BlendMode::kNone, Model::DepthTestMode::kOn);

	// プレイヤーの描画
	player_->Draw();


	// ブロックの描画
	for (std::vector<WorldTransform*>& worldTransBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransBlockLine) {

			if (!worldTransformBlock) {
				continue;
			}

			modelBlock_->Draw(*worldTransformBlock, camera_);
		}
	}

	// スカイドームの描画
	skydome_->Draw();

	// 3Dモデルの後処理
	Model::PostDraw();
}

// デストラクタ
GameScene::~GameScene() {

	// カメラの解放
	delete debugCamera_;

	// プレイヤーの解放
	delete player_;
	delete modelPlayer_;

	// ブロックの解放
	for (std::vector<WorldTransform*>& worldTransBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransBlockLine) {

			delete worldTransformBlock;
		}
	}
	worldTransformBlocks_.clear();

	delete modelBlock_;

	// スカイドームの解放
	delete skydome_;

	delete modelSkydome_;

	// マップチップフィールドの解放
	delete mapchipField_;
}

void GameScene::GenetateBlocks() {

	// 要素数
	uint32_t numBlockVertical = mapchipField_->GetNumBlockVirtical();
	uint32_t numBlockHorizontal = mapchipField_->GetNumBlockHorizontal();

	/*---要素数の変更---*/
	// 列数の設定
	worldTransformBlocks_.resize(numBlockVertical);
	// 行数の設定
	for (uint32_t x = 0; x < numBlockVertical; ++x) {
		worldTransformBlocks_[x].resize(numBlockHorizontal);
	}

	// ブロックの生成
	for (uint32_t y = 0; y < numBlockVertical; ++y) {
		for (uint32_t x = 0; x < numBlockHorizontal; ++x) {

			// マップチップデータに沿って、ワールド変換データの生成、初期化、配置
			if (mapchipField_->GetMapChipTypeByIndex(x, y) == MapChipType::kBlock) {
				// ワールド変換データの生成
				WorldTransform* worldTransformBlock = new WorldTransform();

				// ワールド変換データの初期化
				worldTransformBlock->Initialize();

				// ワールド変換データの位置を設定
				worldTransformBlocks_[y][x] = worldTransformBlock;
				worldTransformBlocks_[y][x]->translation_ = mapchipField_->GetMapChipPositionByIndex(x, y);
			}
		}
	}
}