#include "Bullet.h"
#include <cassert>

void Bullet::Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position, const KamataEngine::Vector3& direction) {

	// NULLチェック
	assert(model);

	// 引数として受け取ったデータをメンバ変数に格納
	modelBullet_ = model;
	camera_ = camera;

	// ワールド変換の初期化
	worldTransformBullet_.Initialize();

	// 位置
	worldTransformBullet_.translation_ = position;

	// スケール
	worldTransformBullet_.scale_ = {0.2f, 0.2f, 0.2f};

	// 速度
	velocity_ = direction * kBulletSpeed;

	isShot_ = true;
}

void Bullet::Update() {

	if (!isShot_) {
		return;
	}

	// 移動
	worldTransformBullet_.translation_ += velocity_;

	// 寿命減少
	if (!persistent_) {
		lifeTime_ -= 1.0f / 60.0f;
		if (lifeTime_ <= 0.0f) {
			isDead_ = true;
		}
	}

	// マップ壁に当たったら消滅
	if (mapChipField_) {

		auto index = mapChipField_->GetMapChipIndexSetByPosition(worldTransformBullet_.translation_);

		auto type = mapChipField_->GetMapChipTypeByIndex(index.xIndex, index.yIndex);

		if (type == MapChipType::kBlock) {
			if (persistent_) {
				// ワイヤー弾：ブロックに刺さって停止する（以後描画は残す）
				velocity_ = {0.0f, 0.0f, 0.0f};
				hooked_ = true;
			} else {
				// 通常弾は消える
				isDead_ = true;
			}
		}
	}

	// 行列の変換と転送
	worldTransformUpdate(worldTransformBullet_);
}

void Bullet::Draw() {

	// モデルの描画
	if (isShot_ && !isDead_) {

		modelBullet_->Draw(worldTransformBullet_, *camera_);
	}
}

void Bullet::worldTransformUpdate(KamataEngine::WorldTransform& worldtransfrom) {

	// スケール、回転、平行移動を合成して変換
	worldtransfrom.matWorld_ = math.MakeAffineMatrix(worldtransfrom.scale_, worldtransfrom.rotation_, worldtransfrom.translation_);

	// 定数バッファに転送
	worldtransfrom.TransferMatrix();
}
