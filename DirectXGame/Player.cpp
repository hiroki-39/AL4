#define NOMINMAX
#include "Player.h"
#include <algorithm>
#include <cassert>
#include <numbers>

using namespace KamataEngine;

void Player::Initialize(Model* model, Camera* camera, const Vector3& position) {
	// NULLチェック
	assert(model);

	// 引数として受け取ったデータをメンバ変数に格納
	model_ = model;
	camera_ = camera;

	// 矢印スプライトの生成
	arrowHandle = TextureManager::GetInstance()->Load("UI/arrow.png");
	arrowSprite = Sprite::Create(arrowHandle, {0.0f,0.0f});

	// 弾モデルの生成
	bulletModel_ = Model::CreateFromOBJ("Player", true);

	// ワールド変換の初期化
	worldTransformPlayer_.Initialize();

	// 位置
	worldTransformPlayer_.translation_ = position;

	// 回転
	worldTransformPlayer_.rotation_.y = std::numbers::pi_v<float> / 2.0f;
}

void Player::Update() {

	// 1.移動入力
	move();

	// Eキーで通常弾とワイヤーモード切り替え
	if (Input::GetInstance()->TriggerKey(DIK_E)) {
		if (fireMode_ == FireMode::Normal) {
			fireMode_ = FireMode::Wire;
		} else {
			fireMode_ = FireMode::Normal;
		}
	}

	// ---- 発射クールタイム ----

	if (fireTimer_ > 0.0f) {
		// 毎フレーム経過
		fireTimer_ -= 1.0f / 60.0f;
	}

	// ---- ワイヤーモード処理 ----
	// ワイヤー角度の更新
	if (fireMode_ == FireMode::Wire) {

		if (wireAngleUp_) {
			wireAngle_ += wireAngleSpeed_;
			if (wireAngle_ >= 90.0f) {
				wireAngleUp_ = false;
			}

		} else {
			wireAngle_ -= wireAngleSpeed_;
			if (wireAngle_ <= 0.0f) {
				wireAngleUp_ = true;
			}
		}
	}

	// ---- 弾の発射処理とワイヤーの発射処理　----
	if (fireMode_ == FireMode::Normal) {
		// リロード中は発射不可
		if (!isReloading_ && currentBullets_ > 0 && fireTimer_ <= 0.0f) {
			if (Input::GetInstance()->PushKey(DIK_J)) {
				// 弾の生成
				auto* newBullet = new Bullet();

				Vector3 bulletDir;

				if (lrDirection_ == LRDirection::kRight) {
					bulletDir = {1.0f, 0.0f, 0.0f}; // 右
				} else {
					bulletDir = {-1.0f, 0.0f, 0.0f}; // 左
				}

				newBullet->Initialize(bulletModel_, camera_, worldTransformPlayer_.translation_, bulletDir);

				bullets_.push_back(newBullet);

				newBullet->SetMapChipField(mapChipField_);

				// 弾消費
				currentBullets_--;

				currentBullets_ = std::max(currentBullets_, 0);

				// 発射クールタイムリセット
				fireTimer_ = fireInterval_;
			}
		}

		// ---- リロード ----
		if (Input::GetInstance()->TriggerKey(DIK_R) && !isReloading_) {

			isReloading_ = true;

			reloadTimer_ = kReloadTime;
		}

	} else if (fireMode_ == FireMode::Wire) {
		if (Input::GetInstance()->TriggerKey(DIK_J)) {

			// 角度（度 → ラジアン）
			float rad = wireAngle_ * (3.14159f / 180.0f);

			// プレイヤーの向いている左右方向
			float dirX = (lrDirection_ == LRDirection::kRight) ? 1.0f : -1.0f;

			// ワイヤー方向ベクトル
			Vector3 wireDir = {dirX * cosf(rad), sinf(rad), 0.0f};

			ShootWire(wireDir); // ワイヤー射出処理
		}
	}

	// リロード中タイマー
	if (isReloading_) {

		reloadTimer_ -= 1.0f / 60.0f;

		if (reloadTimer_ <= 0.0f) {

			currentBullets_ = maxBullets_;

			isReloading_ = false;

			currentBullets_ = std::clamp(currentBullets_, 0, maxBullets_);
		}
	}

	// 弾の更新
	for (auto* bullet : bullets_) {

		bullet->Update();
	}

	// 死んだ弾の削除
	bullets_.remove_if([](Bullet* bullet) {
		if (bullet->IsDead()) {

			delete bullet;

			return true;
		}

		return false;
	});

	// ----　マップの衝突判定　----

	// 衝突情報を初期化
	CollisionMapInfo collisionMapInfo;
	collisionMapInfo.velocity = velocity_;
	collisionMapInfo.landing = false;
	collisionMapInfo.hitWall = false;

	// 2.マップ衝突チェック
	CollisionDetection(collisionMapInfo);

	// 壁キッククールタイム減少
	if (wallKickCooldown_ > 0.0f) {
		wallKickCooldown_ -= 1.0f / 60.0f;
	}

	// 壁キック入力処理
	if (!onGround_ && collisionMapInfo.hitWall && Input::GetInstance()->PushKey(DIK_SPACE) && canWallKick_ && wallKickCooldown_ <= 0.0f) {

		// 壁の反対方向へ横方向初速を与え、上方向の初速を与える
		velocity_.y = kWallKickVertical;
		velocity_.x = (wallTouchDirection_ == LRDirection::kRight) ? -kWallKickHorizontal : kWallKickHorizontal;

		// 状態更新
		canWallKick_ = false;
		wallKickCooldown_ = kWallKickCooldownTime;

		// 方向も反転
		lrDirection_ = (wallTouchDirection_ == LRDirection::kRight) ? LRDirection::kLeft : LRDirection::kRight;
	}

	// 3.移動
	worldTransformPlayer_.translation_ += collisionMapInfo.velocity;

	// 4.天井に接触している時の処理
	if (collisionMapInfo.ceilingCollision) {
		velocity_.y = 0;
	}

	// 接地判定
	UpdateOnGround(collisionMapInfo);

	// 壁接触処理（速度の減衰など）
	UpdateOnWall(collisionMapInfo);

	/*-------------- 旋回制御 --------------*/
	if (turnTimer > 0.0f) {

		// 1/60秒だけのタイマー減を減らす
		turnTimer -= 1.0f / 60.0f;

		if (turnTimer < 0.0f) {

			turnTimer = 0.0f;
		}

		// 経過割合を計算（0.0f〜1.0f）
		float t = 1.0f - (turnTimer / ktimeTurn);

		// 左右の角度テーブル
		float destinationRotationYTable[] = {std::numbers::pi_v<float> / 2.0f, std::numbers::pi_v<float> * 3.0f / 2.0f};

		// 状態に応じた角度を取得
		float destinationRotationY = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];

		// 自キャラの角度を設定
		worldTransformPlayer_.rotation_.y = math.EaseInOut(t, turnFirstRotationY_, destinationRotationY);
	}

	// 二段ジャンプ中の回転
	if (spinning_) {

		spinTimer_ -= 1.0f / 60.0f;

		// 1回転 / kSpinDuration秒で回転
		float spinSpeed = (std::numbers::pi_v<float> * 2.0f) / (60.0f * kSpinDuration);

		// 向きに応じて回転方向を変える
		if (lrDirection_ == LRDirection::kRight) {
			// 右向き
			worldTransformPlayer_.rotation_.z -= spinSpeed;
		} else {
			// 左向き
			worldTransformPlayer_.rotation_.z += spinSpeed;
		}

		// スピン終了
		if (spinTimer_ <= 0.0f) {

			spinning_ = false;

			// 最終向きを維持
			worldTransformPlayer_.rotation_.y = (lrDirection_ == LRDirection::kRight) ? std::numbers::pi_v<float> / 2.0f : std::numbers::pi_v<float> * 3.0f / 2.0f;

			// 回転はリセット
			worldTransformPlayer_.rotation_.z = 0.0f;
		}
	}

	// ----------------------
	//   ワイヤー追跡
	// ----------------------
	if (wireMode_ == WireMode::Shot) {

		// ワイヤー先端を伸ばす
		wireTipPos_ += wireDir_ * wireSpeed_;

		// プレイヤーからの距離が最大射程を超えたら終了
		float dist = math.Length(wireTipPos_ - worldTransformPlayer_.translation_);
		if (dist > wireMaxDistance_) {
			wireMode_ = WireMode::None;
			return;
		}

		// マップ衝突チェック
		IndexSet idx = mapChipField_->GetMapChipIndexSetByPosition(wireTipPos_);
		MapChipType type = mapChipField_->GetMapChipTypeByIndex(idx.xIndex, idx.yIndex);

		// ブロックに当たったら刺さる
		if (type == MapChipType::kBlock) {

			wireHitPos_ = wireTipPos_; // 刺さり位置を保存
			wireMode_ = WireMode::Pulling;

			// 慣性を一旦止める ↑飛ぶと変になるため
			velocity_ = {0, 0, 0};
		}

	} else if (wireMode_ == WireMode::Pulling) {

		// ワイヤーの刺さり位置へ向かうベクトル
		Vector3 toHook = wireHitPos_ - worldTransformPlayer_.translation_;
		float dist = math.Length(toHook);

		// 正規化
		Vector3 dir = math.Normalize(toHook);

		// プレイヤーを移動
		worldTransformPlayer_.translation_ += dir * wirePullSpeed_;

		// 近づいたらワイヤー解除
		if (dist < 0.5f) {
			wireMode_ = WireMode::None;
		}
	}



	// 行列の変換と転送
	math.worldTransformUpdate(worldTransformPlayer_);
}

void Player::Draw() {

	// モデルの描画
	model_->Draw(worldTransformPlayer_, *camera_);

	// ---- 弾の描画 ----
	for (auto* bullet : bullets_) {
		bullet->Draw();
	}

	// ---- デバッグ情報の表示 ----

	/*ImGui::Begin("window");

	ImGui::Text("Bullets: %d / %d", currentBullets_, maxBullets_);

	if (isReloading_) {

	    ImGui::Text("Reloading...");
	}

	ImGui::End();*/
}

void Player::move() {

	// 着地状態
	if (onGround_) {
		// 左右
		if (Input::GetInstance()->PushKey(DIK_D) || Input::GetInstance()->PushKey(DIK_A)) {

			// 左右の加速
			acceleration = {0.0f, 0.0f, 0.0f};

			if (Input::GetInstance()->PushKey(DIK_D)) {

				// 左移動中の右入力
				if (velocity_.x < 0.0f) {

					// 速度と逆方向に入力中は急ブレーキ
					velocity_.x *= (1.0f - kAttenuation);
				}

				acceleration.x += kAcceleration;

				// 右向きを変える
				if (lrDirection_ != LRDirection::kRight) {

					lrDirection_ = LRDirection::kRight;

					// 旋回開始時の角度を記憶
					turnFirstRotationY_ = worldTransformPlayer_.rotation_.y;

					// 旋回タイマーに時間を設定
					turnTimer = ktimeTurn;
				}

			} else if (Input::GetInstance()->PushKey(DIK_A)) {

				// 右移動中の左入力
				if (velocity_.x > 0.0f) {

					// 速度と逆方向に入力中は急ブレーキ
					velocity_.x *= (1.0f - kAttenuation);
				}

				acceleration.x -= kAcceleration;

				// 左向きを変える
				if (lrDirection_ != LRDirection::kLeft) {

					lrDirection_ = LRDirection::kLeft;

					// 旋回開始時の角度を記憶
					turnFirstRotationY_ = worldTransformPlayer_.rotation_.y;

					// 旋回タイマーに時間を設定
					turnTimer = ktimeTurn;
				}
			}

			// 加速／減速
			velocity_ = math.Add(acceleration, velocity_);

			// 最大速度制限
			velocity_.x = std::clamp(velocity_.x, -kLimitRunSpeed, kLimitRunSpeed);

		} else {

			velocity_.x *= (1.0f - kAttenuation);
		}

		/*-------------- ジャンプ --------------*/
		if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {

			// ジャンプの初速
			velocity_ = math.Add(velocity_, Vector3(0, kJumpAcceleration, 0));

			// ジャンプ回数を1に
			jumpCount_ = 1;

			// 空中状態に移行
			onGround_ = false;

			// 地上からジャンプした場合は壁キックをリセット（再び壁に接触したら可能）
			canWallKick_ = false;
		}

		// 空中
	} else {

		// 二段ジャンプ
		if (Input::GetInstance()->TriggerKey(DIK_SPACE) && jumpCount_ < 2) {

			// 上方向の初速をリセット
			velocity_.y = kJumpAcceleration * 1.2f;

			// ジャンプ回数を増やす
			jumpCount_++;

			// スピン開始
			spinning_ = true;

			spinTimer_ = kSpinDuration;
		}

		// 落下速度
		velocity_ = math.Add(velocity_, Vector3(0, -kGravityAcceleration * (1.0f / 60.0f), 0));

		// ======== 壁スライド処理追加 ========

		if (canWallKick_ && !onGround_) {
			// 壁に触れていて落下中ならスライド
			if (velocity_.y < 0.0f) {
				// 落下速度を抑える
				velocity_.y *= 0.6f;
			}
		}

		// 落下速度制限
		velocity_.y = std::max(velocity_.y, -kLimitFallSpeed);
	}
}

Vector3 Player::CornerPosition(const Vector3& center, Corner corner) {

	Vector3 offdetTable[] = {
	    {+kWidth / 2.0f, -kHeight / 2.0f, 0},
        {-kWidth / 2.0f, -kHeight / 2.0f, 0},
        {+kWidth / 2.0f, +kHeight / 2.0f, 0},
        {-kWidth / 2.0f, +kHeight / 2.0f, 0}
    };

	return center + offdetTable[static_cast<uint32_t>(corner)];
}

void Player::CollisionDetection(CollisionMapInfo& info) {
	CollisionDetectionUp(info);
	CollisionDetectionDown(info);
	CollisionDetectionRight(info);
	CollisionDetectionLeft(info);
}

// 上方向当たり判定
void Player::CollisionDetectionUp(CollisionMapInfo& info) {
	// 上昇あり？
	if (info.velocity.y < 0) {
		return;
	}

	MapChipType mapChipType;
	MapChipType mapChipTypeNext;

	// 真上の当たり判定を行う
	bool hit = false;

	// 移動後の4つの角の座標
	std::array<Vector3, kNumCorner> positionsNew;

	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(worldTransformPlayer_.translation_ + info.velocity, static_cast<Corner>(i));
	}

	// 左上点の判定
	IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex + 1);

	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}

	// 右上点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex + 1);

	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}

	// ブロックのヒット判定
	if (hit) {

		// めり込みを排除する方向に移動量を設定する
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(worldTransformPlayer_.translation_ + info.velocity + Vector3(0, +kHeight / 2.0f, 0));

		IndexSet indexSetNow;
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(worldTransformPlayer_.translation_ + Vector3(0, +kHeight / 2.0f, 0));

		if (indexSetNow.yIndex != indexSet.yIndex) {

			// めり込み先のブロックの範囲矩形
			RangeRect rect = mapChipField_->GetRectIndex(indexSet.xIndex, indexSet.yIndex);
			info.velocity.y = std::max(0.0f, rect.bottom - worldTransformPlayer_.translation_.y - (kHeight / 2.0f + kBlank));

			// 天井に当たったことを記録
			info.ceilingCollision = true;
		}
	}
}

// 下方向当たり判定
void Player::CollisionDetectionDown(CollisionMapInfo& info) {
	// 下昇あり？
	if (info.velocity.y >= 0) {
		return;
	}

	MapChipType mapChipType;
	MapChipType mapChipTypeNext;

	// 真上の当たり判定を行う
	bool hit = false;

	// 移動後の4つの角の座標
	std::array<Vector3, kNumCorner> positionsNew;

	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(worldTransformPlayer_.translation_ + info.velocity, static_cast<Corner>(i));
	}

	// 左下点の判定
	IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex - 1);

	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}

	// 右下点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex - 1);

	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}

	// ブロックのヒット判定
	if (hit) {

		// めり込みを排除する方向に移動量を設定する
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(worldTransformPlayer_.translation_ + info.velocity + Vector3(0, -kHeight / 2.0f, 0));

		IndexSet indexSetNow;
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(worldTransformPlayer_.translation_ + Vector3(0, -kHeight / 2.0f, 0));

		if (indexSetNow.yIndex != indexSet.yIndex) {

			// めり込み先のブロックの範囲矩形
			RangeRect rect = mapChipField_->GetRectIndex(indexSet.xIndex, indexSet.yIndex);
			info.velocity.y = std::min(0.0f, rect.top - worldTransformPlayer_.translation_.y + (kHeight / 2.0f + kBlank));

			// 地面に当たったことを記録
			info.landing = true;
		}
	}
}

// 右方向当たり判定
void Player::CollisionDetectionRight(CollisionMapInfo& info) {
	// 移動後の4つの角の座標
	std::array<Vector3, kNumCorner> positionsNew;

	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(worldTransformPlayer_.translation_ + info.velocity, static_cast<Corner>(i));
	}

	MapChipType mapChipType;
	MapChipType mapChipTypeNext;

	// 右側の当たり判定
	bool hit = false;

	// 右上点の判定
	IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex - 1, indexSet.yIndex);

	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}

	// 右下点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex - 1, indexSet.yIndex);

	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}

	// ブロックにヒット？
	if (hit) {

		// 壁方向の記録（右の壁に触れた）
		wallTouchDirection_ = LRDirection::kRight;

		// 空中かつ落下中のときのみ壁キック可能
		if (!onGround_ && velocity_.y < 0.0f) {
			canWallKick_ = true;
		} else {
			canWallKick_ = false;
		}

		// めり込みを排除する方向に移動量を設定する
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(worldTransformPlayer_.translation_ + info.velocity + Vector3(+kWidth / 2.0f, 0, 0));

		// 現在座標が壁の外か判定
		IndexSet indexSetNow;
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(worldTransformPlayer_.translation_ + Vector3(+kWidth / 2.0f, 0, 0));

		if (indexSetNow.xIndex != indexSet.xIndex) {

			// めり込み先のブロックの範囲矩形
			RangeRect rect = mapChipField_->GetRectIndex(indexSet.xIndex, indexSet.yIndex);
			info.velocity.x = std::max(0.0f, rect.left - worldTransformPlayer_.translation_.x - (kWidth / 2.0f + kBlank));

			// 壁に当たったことを記録
			info.hitWall = true;
		}
	}
}

// 左方向当たり判定
void Player::CollisionDetectionLeft(CollisionMapInfo& info) {
	// 移動後の4つの角の座標
	std::array<Vector3, kNumCorner> positionsNew;

	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(worldTransformPlayer_.translation_ + info.velocity, static_cast<Corner>(i));
	}

	MapChipType mapChipType;
	MapChipType mapChipTypeNext;

	// 右側の当たり判定
	bool hit = false;

	// 左上点の判定
	IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex + 1, indexSet.yIndex);

	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}

	// 左下点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex + 1, indexSet.yIndex);

	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}

	// ブロックにヒット？
	if (hit) {

		// 壁方向の記録（左の壁に触れた）
		wallTouchDirection_ = LRDirection::kLeft;

		// 空中かつ落下中のときのみ壁キック可能
		if (!onGround_ && velocity_.y < 0.0f) {
			canWallKick_ = true;
		} else {
			canWallKick_ = false;
		}

		// めり込みを排除する方向に移動量を設定する
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(worldTransformPlayer_.translation_ + info.velocity + Vector3(-kWidth / 2.0f, 0, 0));

		// 現在座標が壁の外か判定
		IndexSet indexSetNow;
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(worldTransformPlayer_.translation_ + Vector3(-kWidth / 2.0f, 0, 0));

		if (indexSetNow.xIndex != indexSet.xIndex) {

			RangeRect rect = mapChipField_->GetRectIndex(indexSet.xIndex, indexSet.yIndex);
			info.velocity.x = std::min(0.0f, rect.right + (kWidth / 2.0f + kBlank) - worldTransformPlayer_.translation_.x);
			// 壁に当たったことを記録
			info.hitWall = true;
		}
	}
}

// 着地状態のときの処理
void Player::UpdateOnGround(const CollisionMapInfo& info) {

	if (onGround_) {
		// 着地状態

		// ジャンプ開始
		if (velocity_.y > 0.0f) {
			// 空中状態に移行
			onGround_ = false;

		} else {
			// 落下判定

			MapChipType mapChipType;

			bool hit = false;

			// 移動後の4つの角の座標
			std::array<Vector3, kNumCorner> positionsNew;

			for (uint32_t i = 0; i < positionsNew.size(); ++i) {
				positionsNew[i] = CornerPosition(worldTransformPlayer_.translation_ + info.velocity, static_cast<Corner>(i));
			}

			// 左下点の判定
			IndexSet indexSet;
			indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom] + Vector3(0, -kGroundSearchHeight, 0));
			mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
			if (mapChipType == MapChipType::kBlock) {
				hit = true;
			}

			// 右下点の判定
			indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom] + Vector3(0, -kGroundSearchHeight, 0));
			mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
			if (mapChipType == MapChipType::kBlock) {
				hit = true;
			}

			// 落下なら空中状態に切り替え
			if (!hit) {
				onGround_ = false;
			}
		}

	} else {
		// 空中状態

		// 着地フラグ
		if (info.landing) {

			// 着地状態に切り替え
			onGround_ = true;

			// 着地時にX速度を減衰
			velocity_.x *= (1.0f - kattenuationLanding);

			// Y速度を0にする
			velocity_.y = 0.0f;

			// 着地で壁キックをリセット
			canWallKick_ = false;

			// ジャンプ回数リセット
			jumpCount_ = 0;
		}
	}
}

// 壁に当たったときの処理
void Player::UpdateOnWall(const CollisionMapInfo& info) {

	if (info.hitWall) {
		velocity_.x *= (1.0f - kAttenuationWall);
	}
}

void Player::ShootWire(const Vector3& dir) {
	wireMode_ = WireMode::Shot;

	// ワイヤーの初期位置はプレイヤー位置
	wireTipPos_ = worldTransformPlayer_.translation_;

	// 方向ベクトル（正規化）
	Vector3 nd = math.Normalize(dir);
	wireDir_ = nd; // 保存
}
