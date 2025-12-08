#pragma once
#include "Bullet.h"
#include "MapChipField.h"
#include "Math.h"

enum class LRDirection {
	kRight,
	kLeft,
};

enum class FireMode { Normal, Wire };

enum class WireMode { None, Shot, Pulling };

// マップとの当たり判定情報
struct CollisionMapInfo {
	bool ceilingCollision = false;  // 天井衝突フラグ
	bool landing = false;           // 着地フラグ
	bool hitWall = false;           // 壁接触フラグ
	KamataEngine::Vector3 velocity; // 移動量
};

// 角
enum Corner {
	kRightBottom, // 右下
	kLeftBottom,  // 左下
	kRightTop,    // 右上
	kLeftTop,     // 左上

	kNumCorner // 要素数
};

class MapChipField;

class Player {
public:
	/// <summary>
	/// プレイヤーの初期化
	/// </summary>
	/// <param name="model">モデル</param>
	/// <param name="camera">カメラ</param>
	/// <param name="camera">位置</param>
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position);

	/// <summary>
	/// プレイヤーの更新
	/// </summary>
	void Update();

	/// <summary>
	/// プレイヤーの描画
	/// </summary>
	void Draw();

	/// <summary>
	/// プレイヤーの移動
	/// </summary>
	void move();

	/// <summary>
	/// 中心座標
	/// </summary>
	/// <param name="center">中心</param>
	/// <param name="corner">角</param>
	/// <returns></returns>
	KamataEngine::Vector3 CornerPosition(const KamataEngine::Vector3& center, Corner corner);

	/// <summary>
	/// マップの衝突判定
	/// </summary>
	/// <param name="info">当たり判定の情報</param>
	void CollisionDetection(CollisionMapInfo& info);

	/// <summary>
	/// マップの衝突判定上方向
	/// </summary>
	/// <param name="info">当たり判定の情報</param>
	void CollisionDetectionUp(CollisionMapInfo& info);

	/// <summary>
	/// マップの衝突判定下方向
	/// </summary>
	/// <param name="info">当たり判定の情報</param>
	void CollisionDetectionDown(CollisionMapInfo& info);

	/// <summary>
	/// マップの衝突判定右方向
	/// </summary>
	/// <param name="info">当たり判定の情報</param>
	void CollisionDetectionRight(CollisionMapInfo& info);

	/// <summary>
	/// マップの衝突判定左方向
	/// </summary>
	/// <param name="info">当たり判定の情報</param>
	void CollisionDetectionLeft(CollisionMapInfo& info);

	/// <summary>
	/// 接地状態の切り替えの処理
	/// </summary>
	/// <param name="info"></param>
	void UpdateOnGround(const CollisionMapInfo& info);

	/// <summary>
	/// 壁接触している場合の処理
	/// </summary>
	/// <param name="info"></param>
	void UpdateOnWall(const CollisionMapInfo& info);

	void ShootWire(const KamataEngine::Vector3& dir);

	/*-------------- アクセッサ --------------*/

	KamataEngine::WorldTransform& GetWorldTransform() { return worldTransformPlayer_; }

	const KamataEngine::Vector3& getvelocity() const { return velocity_; }

	void SetMapChipField(MapChipField* mapChipField) { this->mapChipField_ = mapChipField; };

	int GetCurrentBullets() const { return currentBullets_; }
	int GetMaxBullets() const { return maxBullets_; }
	bool IsReloading() const { return isReloading_; }
	float GetReloadTimer() const { return reloadTimer_; }
	float GetReloadTime() const { return kReloadTime; }
	// 弾リストの読み取り用アクセサ（GameScene から当たり判定に利用）
	const std::list<Bullet*>& GetBullets() const { return bullets_; }

private:

	/*---  ---*/
	uint32_t arrowHandle;

	KamataEngine::Sprite* arrowSprite;

	/*-------------- 向きに関わる系 --------------*/

	// 向き
	LRDirection lrDirection_ = LRDirection::kRight;

	// 旋回開始時の角度
	float turnFirstRotationY_ = 0.0f;

	// 旋回タイマー
	float turnTimer = 0.0f;

	// 旋回時間(秒)
	static inline const float ktimeTurn = 0.3f;

	/*-------------- プレイヤーの移動に関わる系 --------------*/

	// ワールド変換データ
	KamataEngine::WorldTransform worldTransformPlayer_;

	// 速度
	KamataEngine::Vector3 velocity_ = {};

	// 加速
	KamataEngine::Vector3 acceleration = {};

	// 加速
	static inline const float kAcceleration = 0.05f;

	// 速度減衰
	static inline const float kAttenuation = 0.15f;

	// 速度制限
	static inline const float kLimitRunSpeed = 0.3f;

	// 重力加速度(下方向)
	static inline const float kGravityAcceleration = 6.8f;

	// 最大落下速度(下方向)
	static inline const float kLimitFallSpeed = 0.8f;

	// ジャンプ初速(上方向)
	static inline const float kJumpAcceleration = 1.3f;

	// 着地時の速度減衰率
	static inline const float kattenuationLanding = 0.0f;

	// ジャンプ回数
	int jumpCount_ = 0;

	float spinTimer_ = 0.0f;

	// 回転時間
	static inline const float kSpinDuration = 0.3f;

	bool spinning_ = false;

	/*-------------- プレイヤーの描画に関わる系 --------------*/

	// モデル
	KamataEngine::Model* model_ = nullptr;

	// カメラ
	KamataEngine::Camera* camera_ = nullptr;

	/*-------------- プレイヤーの弾に関わる系 --------------*/

	// 弾のリスト
	std::list<Bullet*> bullets_;

	// 弾モデル
	KamataEngine::Model* bulletModel_ = nullptr;

	// ---- 射撃レート管理 ----

	// 発射間隔(秒) 例: 0.2秒 = 約5発/秒
	float fireInterval_ = 0.3f;

	// 発射クールタイム
	float fireTimer_ = 0.0f;

	// ---- 弾管理 ----

	// 最大所持弾数
	int maxBullets_ = 10;

	// 現在の弾数
	int currentBullets_ = 10;

	// リロード中
	bool isReloading_ = false;

	// リロードタイマー
	float reloadTimer_ = 0.0f;

	// リロード時間
	static inline const float kReloadTime = 0.8f;

	/*-------------- ワイヤーモードに関わる系 --------------*/
	FireMode fireMode_ = FireMode::Normal;
	WireMode wireMode_ = WireMode::None;

	// ワイヤー角度関連
	float wireAngle_ = 0.0f;      // 現在角度（0～90度）
	float wireAngleSpeed_ = 1.5f; // 揺れる速度
	bool wireAngleUp_ = true;     // 上昇中？

	// ワイヤーの方向
	KamataEngine::Vector3 wireDir_ = {};

	// ワイヤーの先端位置
	KamataEngine::Vector3 wireTipPos_ = {};

	// ワイヤーが刺さった位置
	KamataEngine::Vector3 wireHitPos_ = {};

	// ワイヤーの飛行速度
	float wireSpeed_ = 0.6f;

	// ワイヤーの最大射程
	float wireMaxDistance_ = 25.0f;

	// プル（引っ張り）速度
	float wirePullSpeed_ = 0.4f;
	/*-------------- プレイヤーの当たり判定に関わる系 --------------*/

	// マップチップによるフィールド
	MapChipField* mapChipField_ = nullptr;

	// 接地状態フラグ
	bool onGround_ = true;

	// 着地フラグ
	bool landing_ = false;

	// キャラクターの当たり判定サイズ
	static inline const float kWidth = 1.99f;

	static inline const float kHeight = 1.99f;

	static inline const float kBlank = 0.001f;

	// 微小な数値
	static inline const float kGroundSearchHeight = 0.06f;

	// 着地時の速度減衰率
	static inline const float kAttenuationWall = 0.1f;

	/*-------------- 壁キックに関わる系 --------------*/

	// 壁キック可能か（再度壁に接触してリセットされる）
	bool canWallKick_ = false;

	// 壁キック後のクールタイム（再キック防止）
	float wallKickCooldown_ = 0.0f;
	static inline const float kWallKickCooldownTime = 0.25f;

	// 最後に触れた壁の方向（壁キックで離れる方向の決定に使用）
	LRDirection wallTouchDirection_ = LRDirection::kRight;

	// 壁キックの横方向初速（壁から離れる向き）
	static inline const float kWallKickHorizontal = 0.6f;

	// 壁キックの縦方向初速（上方向）
	static inline const float kWallKickVertical = 1.2f;

	// ワイヤーで壁に当たった直後に壁ジャンプを許可するフラグ（短時間のみ有効）
	bool wallTouchFromWire_ = false;
	float wallTouchFromWireTimer_ = 0.0f;
	static inline const float kWallTouchFromWireWindow = 0.25f; // 有効時間（秒）

	/*-------------- 関数 --------------*/

	Math math;
};