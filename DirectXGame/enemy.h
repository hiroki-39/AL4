#pragma once
#include "math.h"
#include "KamataEngine.h"
#include "Player.h"

class Player;

class Enemy {
public:
	enum class Behavior {
		kUnknown = -1,
		// 歩行
		kWalk,
		// やられ状態
		kDefeated
	};

public:
	/// <summary>
	/// 敵の初期化
	/// </summary>
	/// <param name="model">モデル</param>
	/// <param name="camera">カメラ</param>
	/// <param name="camera">位置</param>
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position);

	/// <summary>
	/// 敵の更新
	/// </summary>
	void Update();

	/// <summary>
	/// 敵の描画
	/// </summary>
	void Draw();

	// 衝突応答
	void OnCollision(const Player* player);

	KamataEngine::Vector3 GetWorldPosition();

	// AABBの取得
	AABB GetAABB();

	bool IsDead() const { return isDead_; }

	bool IsCollisionDisabled() const { return isCollisionDisabled_; }

private:
	// ワールド変換データ
	KamataEngine::WorldTransform worldTransformEnemy_;

	// モデル
	KamataEngine::Model* model_ = nullptr;

	// カメラ
	KamataEngine::Camera* camera_ = nullptr;

	// 歩行速度
	static inline const float kWalkSpeed = 0.02f;

	// 速度
	KamataEngine::Vector3 velocity_ = {};

	// キャラクターの当たり判定サイズ
	static inline const float kWidth = 1.6f;

	static inline const float kHeight = 1.5f;

	// デスフラグ
	bool isDead_ = false;

	// 振る舞い
	Behavior behavior_ = Behavior::kWalk;

	Behavior behaviorRequest_ = Behavior::kUnknown;

	/*-------------- やられ演出のあれやこれや --------------*/

	// 演出時間
	static inline const float kDefeatedTime = 0.6f;
	static inline const float kDefeatedMotionAngleStart = 0.0f;
	static inline const float kDefeatedMotionAngleEnd = -60.0f;

	// タイマー
	float counter_ = 0.0f;

	// 無効化フラグ
	bool isCollisionDisabled_ = false;

	// 表示フラグ（弾に当たったら false にして描画を止める）
	bool isVisible_ = true;

	/*-------------- アニメーションの設定 --------------*/

	// 最初の角度
	static inline const float kWalkMotionAngleStart = 0.0f;

	// 最後の角度
	static inline const float kWalkMotionAngleEnd = 30.0f;

	// アニメーションの周期となる時間(秒)
	static inline const float kWalkMotionTime = 1.0f;

	// 時間
	float walkTimer = 0.0f;

	/*-------------- 関数 --------------*/

	Math math;
};
