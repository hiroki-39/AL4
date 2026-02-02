#pragma once
#include "math.h"
#include "KamataEngine.h"
#include "Player.h"
#include "MapChipField.h"

class Player;

class Enemy {
public:
	enum class Behavior {
		kUnknown = -1,
		// 歩行（汎用）
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

	// 種類アクセス
	void SetType(EnemyType t) { type_ = t; }
	EnemyType GetType() const { return type_; }

	// プレイヤー参照をセット（逃走挙動で使う）
	void SetPlayer( Player* p) { player_ = p; }

	// マップ情報をセット（逃走時の境界チェックで使用）
	void SetMapChipField(MapChipField* m) { mapField_ = m; }

private:
	// 敵タイプ
	EnemyType type_ = EnemyType::kNone;

	// ワールド変換データ
	KamataEngine::WorldTransform worldTransformEnemy_;

	// 初期位置（振る舞い用）
	KamataEngine::Vector3 initialPosition_ = {};

	// モデル
	KamataEngine::Model* model_ = nullptr;

	// カメラ
	KamataEngine::Camera* camera_ = nullptr;

	// 歩行速度（汎用）
	static inline const float kWalkSpeed = 0.02f;

	// LR/UD 特有パラメータ
	static inline const float kLRAmplitude = 2.0f;
	static inline const float kLRFrequency = 0.8f;
	static inline const float kUDAmplitude = 1.2f;
	static inline const float kUDFrequency = 0.9f;

	// 逃走速度、閾値
	static inline const float kFleeSpeed = 0.2f;
	static inline const float kFleeTriggerDist = 10.0f;

	// 速度
	KamataEngine::Vector3 velocity_ = {};

	// キャラクターの当たり判定サイズ
	static inline const float kWidth = 1.6f;

	static inline const float kHeight = 1.5f;

	// 微小オフセット（プレイヤー側と同値を用意）
	static inline const float kBlank = 0.001f;

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

	/*-------------- プレイヤー参照（逃走用） --------------*/
	Player* player_ = nullptr;

	/*-------------- マップ参照（境界チェック用） --------------*/
	MapChipField* mapField_ = nullptr;

	/*-------------- 関数 --------------*/

	Math math;
};
