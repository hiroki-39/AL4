#pragma once
#include "Fade.h"
#include "KamataEngine.h"
#include "Math.h"
#include "Skydome.h"

class TitleScene {
public:
	// シーンのフェーズ
	enum class Phase {
		kFadeIn,  // フェードイン
		kMain,    // メイン部
		kFadeOut, // フェードアウト
	};

	/// <summary>
	///	ゲームシーンの初期化
	/// </summary>
	void Initialize();

	/// <summary>
	///	ゲームシーンの更新
	/// </summary>
	void Update();

	/// <summary>
	/// ゲームシーンの描画
	///	</summary>
	void Draw();

	///< summary>
	///	デストラクタ
	/// </summary>
	~TitleScene();

	// デスフラグのgetter
	bool IsFinished() const { return finished_; }

private:
	static inline const float kTimeTitleMove = 2.0f;

	// ビュープロジェクション
	KamataEngine::Camera camera_;
	KamataEngine::WorldTransform worldTransformTitle_;
	KamataEngine::WorldTransform worldTransformPlayer_;

	// モデル
	KamataEngine::Model* modelPlayer_ = nullptr;
	KamataEngine::Model* modelTitle_ = nullptr;

	float counter_ = 0.0f;

	// 終了フラグ
	bool finished_ = false;

	/*-------------- シーン --------------*/

	Fade* fade_ = nullptr;

	// 現在のフェーズ
	Phase phase_ = Phase::kFadeIn;

	// タイトルテクスチャハンドル
	uint32_t titleHandle_ = 0;

	// タイトルスプライト
	KamataEngine::Sprite* titleSprite_ = nullptr;

	// ボタンテクスチャハンドル
	uint32_t buttonHandle_ = 0;

	// スタートボタンスプライト
	KamataEngine::Sprite* buttonSprite_ = nullptr;

	Skydome* skydome_ = nullptr;

	// スカイドームのモデル
	KamataEngine::Model* modelSkydome_ = nullptr;

	/*-------------- 関数 --------------*/

	Math math;
};
