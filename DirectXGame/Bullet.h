#pragma once
#include "KamataEngine.h"
#include "MapChipField.h"
#include "Math.h"

class Bullet {
public:
	/// <summary>
	///	初期化
	/// </summary>
	/// <param name="model">モデル</param>
	/// <param name="camera">カメラ</param>
	/// <param name="position">位置</param>
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position, const KamataEngine::Vector3& direction);
	
	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw();

	/// <summary>
	/// ワールド変換の更新
	/// </summary>
	/// <param name="worldtransfrom"></param>
	void worldTransformUpdate(KamataEngine::WorldTransform& worldtransfrom);

	void SetMapChipField(MapChipField* m) { mapChipField_ = m; }

	KamataEngine::Vector3 GetPosition() const { return worldTransformBullet_.translation_; }

	KamataEngine::Vector3 GetSpeed() { return velocity_; };

	bool GetIsShot() { return isShot_; };

	bool IsDead() const { return isDead_; }

	void Kill() { isDead_ = true; }

private:
	// 弾のワールドトランスフォーム
	KamataEngine::WorldTransform worldTransformBullet_;

	// モデル
	KamataEngine::Model* modelBullet_ = nullptr;

	// カメラ
	KamataEngine::Camera* camera_ = nullptr;

	// 移動量
	KamataEngine::Vector3 velocity_ = {};

	// 速度
	static inline const float kBulletSpeed = 0.6f;

	// 撃ったかどうか
	bool isShot_ = false;

	// 5秒寿命
	float lifeTime_ = 5.0f;

	// 消滅フラグ
	bool isDead_ = false;

	// マップチップによるフィールド
	MapChipField* mapChipField_ = nullptr;

	/*--- 関数 ---*/
	Math math;
};
