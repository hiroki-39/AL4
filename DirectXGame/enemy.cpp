#include "Enemy.h"
#include <cassert>
#include <numbers>

using namespace KamataEngine;

void Enemy::Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position) {
	// NUllチェック
	assert(model);

	// 初期化
	model_ = model;
	camera_ = camera;
	worldTransformEnemy_.Initialize();
	worldTransformEnemy_.translation_ = position;
	// 角度調整
	worldTransformEnemy_.rotation_.y = std::numbers::pi_v<float> * 3.0f / 2.0f;

	// 速度設定
	velocity_ = {-kWalkSpeed, 0.0f, 0.0f};
}

void Enemy::Update() {

	if (behaviorRequest_ != Behavior::kUnknown) {

		// 振るまいを変更する
		behavior_ = behaviorRequest_;

		// 各振るまいごとの初期化
		switch (behavior_) {
		case Behavior::kDefeated:
		default:
			counter_ = 0;
			break;
		}

		// 振るまいリクエストをリセット
		behaviorRequest_ = Behavior::kUnknown;
	}

	switch (behavior_) {
	case Enemy::Behavior::kWalk:
		/*---　歩き　---*/

		// 移動
		/*worldTransformEnemy_.translation_ += velocity_;*/

		// タイマーの加算
		walkTimer += 1.0f / 60.0f;

		worldTransformEnemy_.rotation_.x = std::sin(std::numbers::pi_v<float> * 2.0f * walkTimer / kWalkMotionTime);

		// 行列の変換と転送
		math.worldTransformUpdate(worldTransformEnemy_);

		break;
	case Enemy::Behavior::kDefeated:
		/*---　やられ状態　---*/
		// タイマー
		counter_ += 1.0f / 60.0f;

		worldTransformEnemy_.rotation_.y += 0.3f;
		worldTransformEnemy_.rotation_.x = math.EaseOut(counter_ / kDefeatedTime, kDefeatedMotionAngleStart, kDefeatedMotionAngleEnd);

		// 行列の変換と転送
		math.worldTransformUpdate(worldTransformEnemy_);

		if (counter_ >= kDefeatedTime) {
			isDead_ = true;
		}
		break;
	}
}

void Enemy::Draw() {

	if (!isDead_) {
		// モデルの描画
		model_->Draw(worldTransformEnemy_, *camera_);
	}
}

void Enemy::OnCollision(const Player* player) {

	(void)player;

	// 既にやられているなら何もしない
	if (behavior_ == Behavior::kDefeated) {
		return;
	}

	// プレイヤーの弾が当たった場合：非表示にする
	// （描画を止め、衝突を無効化する）
	isVisible_ = false;
	isCollisionDisabled_ = true;

	// 必要ならやられ演出を続けて最終的に isDead_ にするためのリクエストを出す
	behaviorRequest_ = Behavior::kDefeated;
}

KamataEngine::Vector3 Enemy::GetWorldPosition() {
	// ワールド座標を入れる変数
	Vector3 worldPos;

	// ワールド行列の平行移動成分を取得（ワールド座標）
	worldPos.x = worldTransformEnemy_.matWorld_.m[3][0];
	worldPos.y = worldTransformEnemy_.matWorld_.m[3][1];
	worldPos.z = worldTransformEnemy_.matWorld_.m[3][2];

	return worldPos;
}

AABB Enemy::GetAABB() {

	Vector3 worldPos = GetWorldPosition();

	AABB aabb;

	aabb.min = {worldPos.x - kWidth / 2.0f, worldPos.y - kHeight / 2.0f, worldPos.z - kWidth / 2.0f};
	aabb.max = {worldPos.x + kWidth / 2.0f, worldPos.y + kHeight / 2.0f, worldPos.z + kWidth / 2.0f};

	return aabb;
}
