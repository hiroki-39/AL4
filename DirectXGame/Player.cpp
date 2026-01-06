#define NOMINMAX
#include "Player.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <numbers>
#include <vector>

using namespace KamataEngine;

void Player::Initialize(Model* model, Camera* camera, const Vector3& position) {
	// NULLチェック
	assert(model);

	// 引数として受け取ったデータをメンバ変数に格納
	model_ = model;
	camera_ = camera;

	// 矢印スプライトの生成
	arrowHandle = TextureManager::GetInstance()->Load("UI/arrow.png");
	arrowSprite = Sprite::Create(arrowHandle, {0.0f, 0.0f});
	// 矢印サイズ（適宜調整）
	arrowSprite->SetSize({48.0f, 48.0f});

	// 重要: 回転・位置ズレを防ぐため中心を基準にする
	// (スプライトの回転や拡大時に左上基準だと見た目がずれる)
	arrowSprite->SetAnchorPoint({0.5f, 0.5f});

	// 弾モデルの生成
	bulletModel_ = Model::CreateFromOBJ("Block", true);

	// ワールド変換の初期化
	worldTransformPlayer_.Initialize();

	// 位置
	worldTransformPlayer_.translation_ = position;

	// 回転
	worldTransformPlayer_.rotation_.y = std::numbers::pi_v<float> / 2.0f;

	// wireProjectileSpeed_ を既存の wireSpeed_ で初期化（互換性）
	wireProjectileSpeed_ = wireSpeed_;
}

void Player::Update() {

	// 1.移動入力
	move();

	// Eキーで通常弾とワイヤーモード切替
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

		// 狙いの上下選択（ワイヤーモードかつ発射前のみ）
		// w: 上向き、s: 下向き（TriggerKey を使って押した瞬間に切り替え）
		if (wireMode_ == WireMode::None) {
			if (Input::GetInstance()->TriggerKey(DIK_W)) {
				wireAimDown_ = false;
			} else if (Input::GetInstance()->TriggerKey(DIK_S)) {
				wireAimDown_ = true;
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
			// Y 成分の符号は wireAimDown_ で選択（上向きなら正、下向きなら負）
			float vy = (wireAimDown_) ? -fabsf(sinf(rad)) : fabsf(sinf(rad));
			Vector3 wireDir = {dirX * cosf(rad), vy, 0.0f};

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

	// Note:
	// 死んだ弾の削除は Update の最後にまとめて行うようにしました。
	// これにより同一ポインタを複数箇所で delete してしまう事態（double delete）を避け、
	// wire 処理での削除は「IsDead() を立てる」だけに統一します。

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

	// ワイヤーで壁に当たった直後の許可時間を減らす
	if (wallTouchFromWireTimer_ > 0.0f) {
		wallTouchFromWireTimer_ -= 1.0f / 60.0f;
		if (wallTouchFromWireTimer_ <= 0.0f) {
			wallTouchFromWire_ = false;
			wallTouchFromWireTimer_ = 0.0f;
		}
	}

	// Glide タイマー更新（滑空が有効な場合は時間経過で解除）
	if (gliding_) {
		glideTimer_ -= 1.0f / 60.0f;
		if (glideTimer_ <= 0.0f) {
			gliding_ = false;
			glideTimer_ = 0.0f;
		}
	}

	// 壁キック入力処理
	// 変更点: 通常のマップ当たり判定に加え、ワイヤーで当たった直後でも壁ジャンプ可能にする
	if (!onGround_ && (collisionMapInfo.hitWall || wallTouchFromWire_) && Input::GetInstance()->PushKey(DIK_SPACE) && canWallKick_ && wallKickCooldown_ <= 0.0f) {

		// 壁の反対方向へ横方向初速を与え、上方向の初速を与える
		velocity_.y = kWallKickVertical;
		velocity_.x = (wallTouchDirection_ == LRDirection::kRight) ? -kWallKickHorizontal : kWallKickHorizontal;

		// 状態更新
		canWallKick_ = false;
		wallKickCooldown_ = kWallKickCooldownTime;

		// 方向も反転
		lrDirection_ = (wallTouchDirection_ == LRDirection::kRight) ? LRDirection::kLeft : LRDirection::kRight;

		// ワイヤー直当たりフラグは消しておく
		wallTouchFromWire_ = false;
		wallTouchFromWireTimer_ = 0.0f;

		// ジャンプで滑空を解除
		gliding_ = false;
		glideTimer_ = 0.0f;
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
		float destinationRotationY = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)] ;

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
		// 射出用弾が存在するかをチェック。弾自身の当たり判定で刺さったら hooked_ が立つ。
		if (wireProjectile_) {
			// プレイヤーからの距離チェック（最大射程）
			float distFromPlayer = math.Length(wireProjectile_->GetPosition() - worldTransformPlayer_.translation_);
			if (distFromPlayer > wireMaxDistance_) {
				// 射程オーバー: ワイヤーをキャンセルして弾を削除
				for (auto* b : wireBullets_) {
					// 所有は bullets_ にあるため、ここでは削除フラグを立てるだけにする
					if (b)
						b->Kill();
				}
				wireBullets_.clear();
				wireProjectile_ = nullptr;

				// 空中でワイヤーが外れたら滑空開始
				if (!onGround_) {
					gliding_ = true;
					glideTimer_ = kGlideDuration;
				}

				wireMode_ = WireMode::None;

			} else if (wireProjectile_->IsHooked()) {

				// フック弾がブロックに刺さった -> 引っ張りに移行
				wireHitPos_ = wireProjectile_->GetPosition();
				wireMode_ = WireMode::Pulling;

				// 慣性を一旦止める
				velocity_ = {0, 0, 0};

				// ワイヤーの可視化：プレイヤーから hook まで等間隔で弾モデルを配置する
				// 既に bullets_ に wireProjectile_ が入っている想定なので、
				// 以前の中間セグメントがあれば削除してから新規作成する
				// まず既存のセグメント（hook 以外）を削除（ここでは削除フラグのみ）
				for (auto* b : wireBullets_) {
					if (b != wireProjectile_) {
						if (b)
							b->Kill();
					}
				}
				wireBullets_.clear();

				Vector3 start = worldTransformPlayer_.translation_;
				Vector3 end = wireHitPos_;
				Vector3 to = end - start;
				float totalDist = math.Length(to);
				if (totalDist > 0.001f) {
					Vector3 dirSeg = math.Normalize(to);
					// セグメント数は start から end まで wireSegmentSpacing_ 毎
					int segCount = static_cast<int>(std::floor(totalDist / wireSegmentSpacing_));
					// segCount が 0 の場合は hook のみ
					// 我々は配列を「プレイヤー寄り → フック」の順に保持する
					for (int i = 1; i < segCount; ++i) {
						Vector3 segPos = start + dirSeg * (i * wireSegmentSpacing_);
						auto* segBullet = new Bullet();
						// 使用モデルは wireSegmentModel_ が優先（nullptr の場合は通常弾モデル）
						KamataEngine::Model* segModel = (wireSegmentModel_) ? wireSegmentModel_ : bulletModel_;
						// 移動しないので direction はゼロ
						segBullet->Initialize(segModel, camera_, segPos, Vector3{0.0f, 0.0f, 0.0f});
						segBullet->SetMapChipField(mapChipField_);
						segBullet->SetPersistent(true);
						segBullet->SetPosition(segPos);
						// 管理リストへ（先に push すると順序はプレイヤー側から並ぶ）
						bullets_.push_back(segBullet);
						wireBullets_.push_back(segBullet);
					}
				}
				// 最後に hook を配列末尾に置く（wireBullets_ の最後が hook）
				wireBullets_.push_back(wireProjectile_);

				// アキュムレータをリセット
				wirePullAccumulatedDistance_ = 0.0f;
			}

		} else {
			// 予期せぬケース: 射出弾が nullptr になっている -> キャンセル
			// 空中でワイヤーが外れたら滑空開始
			if (!onGround_) {
				gliding_ = true;
				glideTimer_ = kGlideDuration;
			}
			wireMode_ = WireMode::None;
		}

	} else if (wireMode_ == WireMode::Pulling) {

		// ワイヤーの刺さり位置へ向かうベクトル
		Vector3 toHook = wireHitPos_ - worldTransformPlayer_.translation_;
		float dist = math.Length(toHook);

		// 近づいたらワイヤー解除（閾値を事前チェック）
		const float pullReleaseDistance = 0.5f;
		if (dist < pullReleaseDistance) {
			// 目的地に到達したので解除
			wireMode_ = WireMode::None;

			// 空中でワイヤーが外れたら滑空開始（到達しても空中なら）
			if (!onGround_) {
				gliding_ = true;
				glideTimer_ = kGlideDuration;
			}

			// ワイヤーが終わったのでワイヤー用の弾を消す（プレイヤー移動終了時に消す要件）
			for (auto* b : wireBullets_) {
				// hook は bullets_ にも入っているためまとめて削除フラグを立てる
				if (b)
					b->Kill();
			}

			wireBullets_.clear();
			wireProjectile_ = nullptr;

		} else {
			// 正規化（距離が非常に小さい場合はゼロベクトルを使う）
			Vector3 dir = (dist > 1e-6f) ? math.Normalize(toHook) : Vector3{0.0f, 0.0f, 0.0f};

			// ワイヤーで引っ張る移動を「衝突判定あり」で行う
			CollisionMapInfo pullInfo;
			pullInfo.velocity = dir * wirePullSpeed_;
			pullInfo.landing = false;
			pullInfo.hitWall = false;
			pullInfo.ceilingCollision = false;

			// 衝突判定（マップとの干渉を検査）
			CollisionDetection(pullInfo);

			// 衝突によって移動できない・接触した場合の処理
			if (pullInfo.hitWall || pullInfo.landing || pullInfo.ceilingCollision) {
				// ワイヤー移動を解除
				wireMode_ = WireMode::None;

				// 空中でワイヤーが外れたら滑空開始（接触解除でも空中なら）
				if (!onGround_) {
					gliding_ = true;
					glideTimer_ = kGlideDuration;
				}

				// 衝突時は慣性を止める（安全のため）
				velocity_ = {0.0f, 0.0f, 0.0f};

				// 壁触れ情報（CollisionDetection 内で wallTouchDirection_ は更新される）
				wallTouchFromWire_ = true;
				wallTouchFromWireTimer_ = kWallTouchFromWireWindow;

				// 壁ジャンプ許可（通常の canWallKick_ と同様）
				canWallKick_ = true;

				// onGround_ は false のままにする（空中扱い）
				onGround_ = false;

				// 引っ張り中に衝突で解除された場合もワイヤー用の弾を消す（削除フラグのみ）
				for (auto* b : wireBullets_) {
					if (b) {
						// hook は残す or 既に persistent なら Kill() で統一
						b->Kill();
					}
				}

				wireBullets_.clear();
				wireProjectile_ = nullptr;

			} else {

				// 移動量を適用（衝突処理で調整済み）
				worldTransformPlayer_.translation_ += pullInfo.velocity;

				// 物理速度としても保存（次フレームの衝突処理などに利用）
				velocity_ = pullInfo.velocity;

				// 累積距離を増やし、規定間隔ごとにプレイヤー側のセグメントを削除
				float moved = math.Length(pullInfo.velocity);
				wirePullAccumulatedDistance_ += moved;

				while (wirePullAccumulatedDistance_ >= wireSegmentSpacing_ && wireBullets_.size() > 1) {
					// wireBullets_ の先頭はプレイヤー寄りのセグメント、末尾が hook として作成している
					Bullet* removeSeg = wireBullets_.front();
					// 念のため hook は残す（末尾）
					if (removeSeg == wireProjectile_) {
						// もし先頭が hook だったら終了
						break;
					}
					// リストから即座に取り除かず、削除フラグを立てる
					if (removeSeg)
						removeSeg->Kill();
					// vector の先頭を消す
					wireBullets_.erase(wireBullets_.begin());
					wirePullAccumulatedDistance_ -= wireSegmentSpacing_;
					// 続けて削除するループ
				}

				// 接地・壁の状態を更新
				UpdateOnGround(pullInfo);
				UpdateOnWall(pullInfo);
			}
		}
	}

	// 死んだ弾の削除（Update の最後にまとめて行う）
	bullets_.remove_if([](Bullet* bullet) {
		if (bullet->IsDead()) {
			delete bullet;
			return true;
		}
		return false;
	});

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

	// ---- ワイヤー狙い用の矢印表示 ----
	// ワイヤーモードで、まだワイヤーを射出していない（狙い中）の場合に表示
	if (fireMode_ == FireMode::Wire && wireMode_ == WireMode::None) {
		// 矢印をプレイヤーの中心に表示する（変更点）
		Vector3 worldPos = worldTransformPlayer_.translation_;
		// （以前は頭上にオフセットしていた: worldPos.y += (kHeight / 2.0f + 0.5f);）

		// world -> view -> proj の順で変換し、NDC を得る
		Vector3 viewPos = math.Transform(worldPos, camera_->matView);
		Vector3 projPos = math.Transform(viewPos, camera_->matProjection);

		// NDC (-1..+1) をスクリーン座標に変換
		float screenX = (projPos.x + 1.0f) * 0.5f * static_cast<float>(WinApp::kWindowWidth);
		float screenY = (1.0f - (projPos.y + 1.0f) * 0.5f) * static_cast<float>(WinApp::kWindowHeight);

		// スプライトの設定
		// anchor を中央にしているため、SetPosition にそのままスクリーン座標を渡せば
		// 回転しても描画位置がずれない
		arrowSprite->SetPosition({screenX, screenY});

		// 回転：ワイヤー実際の発射ベクトル（Draw時点の狙い方向）から角度を算出して適用
		// 実際の射出時と同じ計算を使う（Yは上向きに固定 / 下向き指定があれば負にする）
		float rad = wireAngle_ * (3.14159f / 180.0f);
		float dirX = (lrDirection_ == LRDirection::kRight) ? 1.0f : -1.0f;
		// 目標方向ベクトル（ShootWire と同じ）
		float vx = dirX * cosf(rad);
		float vy = (wireAimDown_) ? -fabsf(sinf(rad)) : fabsf(sinf(rad));
		// atan2f( y, x ) で角度を取得（+x を基準に反時計回り）
		float angleRad = atan2f(vy, vx);

		// 環境によってスプライト回転の正負が逆なので、ここでは -angleRad をセットしている。
		// 必要なら +angleRad に切り替えてください。
		arrowSprite->SetRotation(-angleRad);

		// 描画（スプライト描画状態にする）
		Sprite::PreDraw();
		arrowSprite->Draw();
		Sprite::PostDraw();
	}
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

			// ジャンプで滑空を解除
			gliding_ = false;
			glideTimer_ = 0.0f;
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

			// 二段ジャンプでは滑空を解除
			gliding_ = false;
			glideTimer_ = 0.0f;
		}

		// 空中での左右「簡易制御」を追加（ワイヤー衝突後も左右移動できるようにする）
		if (Input::GetInstance()->PushKey(DIK_D) || Input::GetInstance()->PushKey(DIK_A)) {
			if (Input::GetInstance()->PushKey(DIK_D)) {
				// 反対向きの減速（ブレーキ）
				if (velocity_.x < 0.0f)
					velocity_.x *= (1.0f - kAttenuation);
				// 空中加速（地上より小さめ） -- 滑空中も操作は同じ（必要ならここで差分付け可）
				velocity_.x += kAirAcceleration;

				// 向きの更新（見た目の回転用）
				if (lrDirection_ != LRDirection::kRight) {
					lrDirection_ = LRDirection::kRight;
					turnFirstRotationY_ = worldTransformPlayer_.rotation_.y;
					turnTimer = ktimeTurn;
				}
			} else if (Input::GetInstance()->PushKey(DIK_A)) {
				if (velocity_.x > 0.0f)
					velocity_.x *= (1.0f - kAttenuation);
				velocity_.x -= kAirAcceleration;

				if (lrDirection_ != LRDirection::kLeft) {
					lrDirection_ = LRDirection::kLeft;
					turnFirstRotationY_ = worldTransformPlayer_.rotation_.y;
					turnTimer = ktimeTurn;
				}
			}

			// 水平速度上限
			velocity_.x = std::clamp(velocity_.x, -kLimitRunSpeed, kLimitRunSpeed);
		} else {
			// 空中慣性で徐々に減衰
			velocity_.x *= (1.0f - kAirAttenuation);
		}

		// 落下速度（滑空中は重力を軽減）
		float gravityScale = gliding_ ? kGlideGravityScale : 1.0f;
		velocity_ = math.Add(velocity_, Vector3(0, -kGravityAcceleration * gravityScale * (1.0f / 60.0f), 0));

		// ======== 壁スライド処理追加（ワイヤー衝突由来の接触ではスライドしない） ========
		// 変更: canWallKick_ を満たしていても、wallTouchFromWire_ のときは壁スライドを適用しない
		if (canWallKick_ && !onGround_ && !wallTouchFromWire_) {
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

			// 着地で滑空を解除
			gliding_ = false;
			glideTimer_ = 0.0f;
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

	// 方向ベクトル（正規化）
	Vector3 nd = math.Normalize(dir);
	wireDir_ = nd; // 保存

	// 弾の生成（フック弾）
	auto* newBullet = new Bullet();

	// 使用するモデルは優先順位: wireProjectileModel_ -> bulletModel_
	KamataEngine::Model* projModel = (wireProjectileModel_) ? wireProjectileModel_ : bulletModel_;

	newBullet->Initialize(projModel, camera_, worldTransformPlayer_.translation_, nd);
	newBullet->SetMapChipField(mapChipField_);
	newBullet->SetPersistent(true); // ブロックに刺さったら残す

	// 発射速度を上書き（Bullet の速度フィールドを直接設定）
	newBullet->SetVelocity(nd * wireProjectileSpeed_);

	// フックを発射方向に傾ける
	newBullet->SetRotationFromDirection(nd);

	// フック弾のサイズ（既にあるなら上書きしないでください）
	newBullet->SetScale(Vector3{0.5f, 0.5f, 0.5f});

	// 保持しておく
	bullets_.push_back(newBullet);
	wireBullets_.push_back(newBullet);
	wireProjectile_ = newBullet;
}

/* ---------- ワイヤー設定 API の実装 ---------- */
void Player::SetWireModels(KamataEngine::Model* projectileModel, KamataEngine::Model* segmentModel) {
	wireProjectileModel_ = projectileModel;
	wireSegmentModel_ = segmentModel;
}

void Player::SetWireProjectileSpeed(float speed) {
	wireProjectileSpeed_ = speed;
	// 互換のため既存 wireSpeed_ も更新しておく
	wireSpeed_ = speed;
}

void Player::SetWireSegmentSpacing(float spacing) {
	// 安全値チェック
	if (spacing > 0.01f) {
		wireSegmentSpacing_ = spacing;
	}
}

void Player::SetWirePullSpeed(float speed) { wirePullSpeed_ = speed; }
