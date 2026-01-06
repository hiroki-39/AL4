#define NOMINMAX
#include "GameScene.h"
#include <algorithm>
#include <cmath>

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

	auto* hookModel = Model::CreateFromOBJ("anchor", true);
	auto* segmentModel = Model::CreateFromOBJ("chain", true);
	player_->SetWireModels(hookModel, segmentModel);
	player_->SetWireProjectileSpeed(1.2f);
	player_->SetWireSegmentSpacing(0.6f);
	player_->SetWirePullSpeed(0.5f);

#pragma endregion

#pragma region "敵"
	/*-------------- 敵の初期化 --------------*/

	// 3Dモデルの生成
	modelEnemy_ = Model::CreateFromOBJ("target", true);

	// 敵の生成（CSVのスポーン情報を使用）
	if (mapchipField_) {
		const auto& spawns = mapchipField_->GetEnemySpawns();
		for (const auto& spawn : spawns) {
			Enemy* newEnemy = new Enemy();

			// マップチップ座標をワールド座標に変換して初期位置とする
			Vector3 enemyPosition = mapchipField_->GetMapChipPositionByIndex(spawn.index.xIndex, spawn.index.yIndex);

			// 敵の初期化
			newEnemy->Initialize(modelEnemy_, &camera_, enemyPosition);

			// 将来的に spawn.type に応じた初期設定を行う場合はここで switch する
			// switch (spawn.type) { ... }

			enemies_.push_back(newEnemy);
		}
	}

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

	// ここでマップの範囲を元にカメラの可動範囲を設定する（タイル単位パディング：4）
	if (mapchipField_) {
		cameraController_->SetMapField(mapchipField_, 4); // これで「マップ端から4タイル手前」で追従が止まる
	}

	// 追尾対象を設定
	cameraController_->SetTarget(player_);

	// リセット
	cameraController_->Reset();

#pragma endregion

#pragma region "UI"

	numberTexHandle[0] = TextureManager::Load("UI/Numbers/0.png");
	numberTexHandle[1] = TextureManager::Load("UI/Numbers/1.png");
	numberTexHandle[2] = TextureManager::Load("UI/Numbers/2.png");
	numberTexHandle[3] = TextureManager::Load("UI/Numbers/3.png");
	numberTexHandle[4] = TextureManager::Load("UI/Numbers/4.png");
	numberTexHandle[5] = TextureManager::Load("UI/Numbers/5.png");
	numberTexHandle[6] = TextureManager::Load("UI/Numbers/6.png");
	numberTexHandle[7] = TextureManager::Load("UI/Numbers/7.png");
	numberTexHandle[8] = TextureManager::Load("UI/Numbers/8.png");
	numberTexHandle[9] = TextureManager::Load("UI/Numbers/9.png");

	for (int i = 0; i < 10; i++) {

		numberSprite[i] = Sprite::Create(numberTexHandle[i], {0.0f, 0.0f});
		numberSprite[i]->SetSize({48.0f, 48.0f});
	}

	maxNumberTexHandle[0] = TextureManager::Load("UI/Numbers/0.png");
	maxNumberTexHandle[1] = TextureManager::Load("UI/Numbers/1.png");
	maxNumberTexHandle[2] = TextureManager::Load("UI/Numbers/2.png");
	maxNumberTexHandle[3] = TextureManager::Load("UI/Numbers/3.png");
	maxNumberTexHandle[4] = TextureManager::Load("UI/Numbers/4.png");
	maxNumberTexHandle[5] = TextureManager::Load("UI/Numbers/5.png");
	maxNumberTexHandle[6] = TextureManager::Load("UI/Numbers/6.png");
	maxNumberTexHandle[7] = TextureManager::Load("UI/Numbers/7.png");
	maxNumberTexHandle[8] = TextureManager::Load("UI/Numbers/8.png");
	maxNumberTexHandle[9] = TextureManager::Load("UI/Numbers/9.png");

	for (int i = 0; i < 10; i++) {
		maxNumberSprite[i] = Sprite::Create(maxNumberTexHandle[i], {0.0f, 0.0f});
		maxNumberSprite[i]->SetSize({48.0f, 48.0f});
	}

	slashTexHandle = TextureManager::Load("UI/Numbers/slash.png");

	slashSprite = Sprite::Create(slashTexHandle, {0.0f, 0.0f});
	slashSprite->SetSize({48.0f, 48.0f});

	reloadTexHandle = TextureManager::Load("UI/Numbers/reload.png");

	reloadSprite = Sprite::Create(reloadTexHandle, {0.0f, 0.0f});
	reloadSprite->SetSize({40.0f, 40.0f});

	operationTexHandle = TextureManager::Load("font/manual.png");
	operationSprite = Sprite::Create(operationTexHandle, {760.0f, 620.0f});
	operationSprite->SetSize({500.0f, 100.0f});

	targetTexHandle = TextureManager::Load("font/target.png");
	targetSprite = Sprite::Create(targetTexHandle, {400.0f, 20.0f});
	targetSprite->SetSize({125.0f, 50.0f});

	enemyCountTexHandle[0] = TextureManager::Load("UI/Numbers/0.png");
	enemyCountTexHandle[1] = TextureManager::Load("UI/Numbers/1.png");
	enemyCountTexHandle[2] = TextureManager::Load("UI/Numbers/2.png");
	enemyCountTexHandle[3] = TextureManager::Load("UI/Numbers/3.png");
	enemyCountTexHandle[4] = TextureManager::Load("UI/Numbers/4.png");
	enemyCountTexHandle[5] = TextureManager::Load("UI/Numbers/5.png");
	enemyCountTexHandle[6] = TextureManager::Load("UI/Numbers/6.png");
	enemyCountTexHandle[7] = TextureManager::Load("UI/Numbers/7.png");
	enemyCountTexHandle[8] = TextureManager::Load("UI/Numbers/8.png");
	enemyCountTexHandle[9] = TextureManager::Load("UI/Numbers/9.png");

	// --- 追加: 敵数表示用スプライトを生成 ---
	for (int i = 0; i < 10; i++) {
		enemyCountSprite[i] = Sprite::Create(enemyCountTexHandle[i], {0.0f, 0.0f});
		// 画面右上に表示する想定なのでやや小さめに
		enemyCountSprite[i]->SetSize({36.0f, 36.0f});
	}

#pragma endregion

	// BGMの読み込み
	soundHandle_ = Audio::GetInstance()->LoadWave("Sounds/BGM_game.wav");

	// BGM再生
	bgmHandle_ = Audio::GetInstance()->PlayWave(soundHandle_, true);

	// ゲームの現在フェーズ
	phase_ = Phase::kFadeIn;

	// フェードの更新
	fade_ = new Fade();
	fade_->Initialize();
	fade_->Start(Fade::Status::FadeIn, 1.0f);
}

// ゲームシーンの更新
void GameScene::Update() {

	ChangePhase();

	switch (phase_) {
	case Phase::kFadeIn:
		// フェードインの更新

		fade_->Update();

		if (fade_->IsFinished()) {
			fade_->Start(Fade::Status::FadeOut, 1.0f);
			phase_ = Phase::kPlay;
		}

		// スカイドームの更新
		skydome_->Update();

		// カメラコントローラーの更新
		cameraController_->Update();

		// プレイヤーの更新
		player_->Update();

		// 敵の更新
		for (Enemy* enemy : enemies_) {
			enemy->Update();
		}

		// ブロックの更新
		for (std::vector<WorldTransform*>& worldTransBlockLine : worldTransformBlocks_) {
			for (WorldTransform* worldTransformBlock : worldTransBlockLine) {
				if (!worldTransformBlock) {
					continue;
				}

				math->worldTransformUpdate(*worldTransformBlock);
			}
		}

		break;
	case Phase::kPlay:
		// ゲームプレイフェーズの処理

		// スカイドームの更新
		skydome_->Update();

		// プレイヤーの更新
		player_->Update();

		// 敵の更新
		for (Enemy* enemy : enemies_) {
			enemy->Update();
		}

		// カメラコントローラーの更新
		cameraController_->Update();

		// ブロックの更新
		for (std::vector<WorldTransform*>& worldTransBlockLine : worldTransformBlocks_) {
			for (WorldTransform* worldTransformBlock : worldTransBlockLine) {
				if (!worldTransformBlock) {
					continue;
				}

				math->worldTransformUpdate(*worldTransformBlock);
			}
		}

#ifdef _DEBUG

		if (Input::GetInstance()->TriggerKey(DIK_TAB)) {
			// デバックカメラの有効
			isDebugCameraActive_ = !isDebugCameraActive_;
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

		// 全ての当たり判定を行う
		CheckAllCollisions();

		// --- 全敵が死亡しているかチェック ---
		{
			bool allDead = true;
			for (Enemy* enemy : enemies_) {
				if (enemy && !enemy->IsDead()) {
					allDead = false;
					break;
				}
			}

			if (allDead) {
				// フェードアウトしてタイトルへ戻る
				if (fade_) {
					fade_->Start(Fade::Status::FadeOut, 1.0f);
				}
				phase_ = Phase::kFadeOut;
				// BGM停止を追加
				Audio::GetInstance()->StopWave(bgmHandle_);
			}
		}
		break;
	case Phase::kDeath:
		// デス演出フェーズの処理

		// スカイドームの更新
		skydome_->Update();

		// カメラコントローラーの更新
		cameraController_->Update();

		// 敵の更新
		for (Enemy* enemy : enemies_) {
			enemy->Update();
		}

		break;
	case Phase::kFadeOut:
		// フェードアウトの更新

		// フェードの更新
		fade_->Update();

		if (fade_->IsFinished()) {
			finished_ = true;
		}

		// スカイドームの更新
		skydome_->Update();

		// 敵の更新
		for (Enemy* enemy : enemies_) {
			enemy->Update();
		}

		// カメラコントローラーの更新
		cameraController_->Update();

		break;
	}
}

// ゲームシーンの描画
void GameScene::Draw() {
	// 3Dモデル描画前処理
	Model::PreDraw(Model::CullingMode::kBack, Model::BlendMode::kNone, Model::DepthTestMode::kOn);

	// スカイドームの描画
	skydome_->Draw();

	// 3Dモデルの後処理
	Model::PostDraw();

	// 3Dモデル描画前処理
	Model::PreDraw(Model::CullingMode::kBack, Model::BlendMode::kNone, Model::DepthTestMode::kOn);

	// ブロックの描画
	for (std::vector<WorldTransform*>& worldTransBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransBlockLine) {

			if (!worldTransformBlock) {
				continue;
			}

			modelBlock_->Draw(*worldTransformBlock, camera_);
		}
	}

	// 3Dモデルの後処理
	Model::PostDraw();

	// 3Dモデル描画前処理
	Model::PreDraw(Model::CullingMode::kBack, Model::BlendMode::kNone, Model::DepthTestMode::kOn);

	// プレイヤーの描画
	player_->Draw();

	// 3Dモデルの後処理
	Model::PostDraw();

	Model::PreDraw(Model::CullingMode::kBack, Model::BlendMode::kNone, Model::DepthTestMode::kOn);

	// 敵の描画
	for (Enemy* enemy : enemies_) {
		enemy->Draw();
	}

	fade_->Draw();

	// 3Dモデルの後処理
	Model::PostDraw();

	Sprite::PreDraw();

	int current = player_->GetCurrentBullets();
	int max = player_->GetMaxBullets();

	// 位置（左から順に配置）
	Vector2 basePos = {40.0f, 670.0f};
	// float size = 48.0f;
	float spacing = 40.0f;

	// ---- 現在弾数（左） ----
	int curTens = current / 10;
	int curOnes = current % 10;

	if (current >= 10) {
		numberSprite[curTens]->SetPosition(basePos);
		numberSprite[curTens]->Draw();
		numberSprite[curOnes]->SetPosition({basePos.x + spacing, basePos.y});
		numberSprite[curOnes]->Draw();
	} else {
		numberSprite[curOnes]->SetPosition(basePos);
		numberSprite[curOnes]->Draw();
	}

	// ---- スラッシュ ----
	slashSprite->SetPosition({basePos.x + spacing * 2, basePos.y});
	slashSprite->Draw();

	// ---- 最大弾数（右） ----
	int maxTens = max / 10;
	int maxOnes = max % 10;

	if (max >= 10) {
		maxNumberSprite[maxTens]->SetPosition({basePos.x + spacing * 3, basePos.y});
		maxNumberSprite[maxTens]->Draw();
		maxNumberSprite[maxOnes]->SetPosition({basePos.x + spacing * 4, basePos.y});
		maxNumberSprite[maxOnes]->Draw();
	} else {
		maxNumberSprite[maxOnes]->SetPosition({basePos.x + spacing * 3, basePos.y});
		maxNumberSprite[maxOnes]->Draw();
	}

	// 操作方法UIの描画
	operationSprite->Draw();

	// 目標UIの描画
	targetSprite->Draw();

	// --- 追加: 敵数表示 (右上) ---
	{
		int alive = 0;
		for (Enemy* e : enemies_) {
			if (e && !e->IsDead()) {
				++alive;
			}
		}

		// 右上配置
		Vector2 enemyBasePos = {540.0f, 25.0f};
		float enemySpacing = 40.0f;

		// 現在 (生存数)
		int aliveTens = alive / 10;
		int aliveOnes = alive % 10;

		if (alive >= 10) {
			enemyCountSprite[aliveTens]->SetPosition(enemyBasePos);
			enemyCountSprite[aliveTens]->Draw();
			enemyCountSprite[aliveOnes]->SetPosition({enemyBasePos.x + enemySpacing, enemyBasePos.y});
			enemyCountSprite[aliveOnes]->Draw();
		} else {
			enemyCountSprite[aliveOnes]->SetPosition(enemyBasePos);
			enemyCountSprite[aliveOnes]->Draw();
		}
	}

	Sprite::PostDraw();
}

// デストラクタ
GameScene::~GameScene() {

	// カメラの解放
	delete debugCamera_;
	// カメラコントローラーの解放（Initialize で new している）
	delete cameraController_;

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

	// 敵の解放
	for (Enemy* enemy : enemies_) {
		delete enemy;
	}
	// 敵モデルの解放（CreateFromOBJ で取得しているため解放する）
	delete modelEnemy_;

	// フェードの解放（new しているため解放する）
	delete fade_;

	// マップチップフィールドの解放
	delete mapchipField_;

	for (uint32_t i = 0; i < 10; i++) {
		delete numberSprite[i];
	}

	for (uint32_t i = 0; i < 10; i++) {
		delete maxNumberSprite[i];
	}
	delete slashSprite;
	delete reloadSprite;
	delete operationSprite;

	// --- 追加: 敵数スプライトの解放 ---
	for (uint32_t i = 0; i < 10; i++) {
		delete enemyCountSprite[i];
	}
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

// そう当たり判定
void GameScene::CheckAllCollisions() {

	// 判定対象1と2の座標
	/*AABB aabb1, aabb2;*/

#pragma region 自キャラと敵キャラAの当たり判定
	//{
	//	// 自キャラの座標
	//	aabb1 = player_->GetAABB();

	//	// 自キャラと敵弾全ての当たり判定
	//	for (Enemy* enemy : enemies_) {
	//		// 敵弾の座標
	//		aabb2 = enemy->GetAABB();

	//		// 敵の衝突無効化フラグが立っているなら、衝突判定を無視
	//		if (enemy->IsCollisionDisabled()) {
	//			// 衝突無効化フラグの敵はスキップ
	//			continue;
	//		}

	//		// AABB同士の交差判定
	//		if (function.IsCollision(aabb1, aabb2)) {
	//			// 自キャラの衝突時コールバックを呼び出す
	//			player_->OnCollision(enemy);
	//			// 敵弾の衝突時コールバックを呼び出す
	//			enemy->OnCollision(player_);
	//		}
	//	}
	//}
#pragma endregion

#pragma region 自弾(通常弾)と敵キャラの当たり判定
	{
		// プレイヤーの弾リストを取得
		const std::list<Bullet*>& bullets = player_->GetBullets();

		// 各敵に対して当たり判定
		for (Enemy* enemy : enemies_) {
			if (!enemy)
				continue;
			if (enemy->IsDead())
				continue;
			if (enemy->IsCollisionDisabled())
				continue;

			AABB enemyAabb = enemy->GetAABB();

			for (Bullet* bullet : bullets) {
				if (!bullet)
					continue;
				if (bullet->IsDead())
					continue;
				if (!bullet->GetIsShot())
					continue;

				KamataEngine::Vector3 pos = bullet->GetPosition();

				// 弾（点）と敵のAABBの当たり判定（点がAABB内にあるか）
				if (pos.x >= enemyAabb.min.x && pos.x <= enemyAabb.max.x && pos.y >= enemyAabb.min.y && pos.y <= enemyAabb.max.y && pos.z >= enemyAabb.min.z && pos.z <= enemyAabb.max.z) {

					// 衝突時処理
					enemy->OnCollision(player_);

					// 弾を消す（削除は Player::Update 内の remove_if に任せる）
					bullet->Kill();

					// １つの弾で複数敵に当たらない想定 -> 内側ループを抜ける
					break;
				}
			}
		}
	}
#pragma endregion
}

void GameScene::ChangePhase() {

	switch (phase_) {
	case Phase::kPlay:
		// Initialize関数のいきなりパーティクル発生処理は消す
		// if (player_->IsDead()) {
		//	// 死亡演出
		//	phase_ = Phase::kDeath;

		//	const Vector3& deathParticlesPosition = player_->GetWorldPosition();

		//	deathParticles_ = new DeathParticles;
		//	deathParticles_->Initialize(modelDeathParticles_, &camera_, deathParticlesPosition);
		//}
		break;
	case Phase::kDeath:
		break;
	}
}