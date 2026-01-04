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

	/*-------------- スプライト用アニメーション変数 --------------*/

	// タイトルスプライトの基準位置（スクリーン座標）
	KamataEngine::Vector2 titleSpriteBasePos_ = {300.0f, 100.0f};
	// タイトル上下アニメーションの振幅（ピクセル）
	float titleSpriteAmplitude_ = 8.0f;
	// タイトル上下アニメーション周期（秒）
	float titleSpritePeriod_ = 2.0f;
	// タイトルアニメーション用カウンタ
	float titleSpriteCounter_ = 0.0f;

	// ボタン点滅（アルファ変化）周期（秒）
	float buttonBlinkPeriod_ = 1.0f;
	// ボタン点滅用カウンタ
	float buttonBlinkCounter_ = 0.0f;
	// ボタンのベースカラー（RGB）
	KamataEngine::Vector4 buttonBaseColor_ = {1.0f, 1.0f, 1.0f, 1.0f};

	/*-------------- 関数 --------------*/

	Math math;
};
