#pragma once
#include "Math.h"
#include "KamataEngine.h"

// 矩形
struct Rect {
	// 左端
	float left = 0.0f;
	// 右端
	float right = 1.0f;
	// 下端
	float bottom = 0.0f;
	// 上端
	float top = 1.0f;
};

class Player;

class CameraController {
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(KamataEngine::Camera* camera);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// リセット処理
	/// </summary>
	void Reset();

	// アクセッサ
	void SetTarget(Player* target) { target_ = target; }

	void SetmovaleArea(Rect& area) { movaleArea_ = area; };

private:
	/*-------------- カメラ --------------*/
	// カメラ
	KamataEngine::Camera* camera_;

	// カメラのオフセット
	KamataEngine::Vector3 targetOffset_ = {0.0f, 0.0f, -20.0f};

	// カメラ移動範囲
	Rect movaleArea_ = {0, 100, 0, 100};

	// カメラの目標座標
	KamataEngine::Vector3 targetposition_ = {0.0f, 0.0f, 0.0f};

	// 座標補間割合
	static inline const float kInterpolationRate = 0.1f;

	// 速度掛け率
	static inline const float kVelocitybias = 2.0f;

	// 追尾対象の各方向へのカメラ移動範囲
	static inline const Rect margin = {-10.0f, 10.0f, -10.0f, 10.0f}; // 左、右、下、上の順番

	/*-------------- プレイヤー --------------*/
	Player* target_ = nullptr;

	/*-------------- 関数 --------------*/

	Math math;
};