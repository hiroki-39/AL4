#pragma once
#include "Math.h"
#include "KamataEngine.h"

class Skydome {
public:
	/// <summary>
	///	スカイドームの初期化
	/// </summary>
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera);

	/// <summary>
	/// スカイドームの更新
	/// </summary>
	void Update();

	/// <summary>
	/// スカイドームの描画
	/// </summary>
	void Draw();

private:
	// ワールド変換データ
	KamataEngine::WorldTransform worldTransformSkydome_;

	// スカイドームのモデル
	KamataEngine::Model* model_ = nullptr;

	// カメラ
	KamataEngine::Camera* camera_ = nullptr;

	// テクスチャハンドル
	uint32_t textureHandle_ = 0u;

	/*---関数---*/
	Math* math = new Math();
};