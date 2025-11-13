#pragma once
#include "KamataEngine.h"

class Fade {
public:
	// フェードの状態
	enum class Status {
		// フェードなし
		None,
		// 　フェードイン中
		FadeIn,
		// フェードアウト中
		FadeOut,
	};

	/// <summary>
	/// フェードの初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// フェードの更新
	/// </summary>
	void Update();

	/// <summary>
	/// フェードの描画
	/// </summary>
	void Draw();

	// フェード開始
	void Start(Status status, float duration);

	// フェード停止
	void Stop();

	// フェード終了判定
	bool IsFinished() const;

private:
	// スプライト
	KamataEngine::Sprite* sprite_ = nullptr;

	// 現在のフェードの状態
	Status status_ = Status::None;

	// 　フェードの持続時間
	float duration_ = 0.0f;

	// 経過時間カウンター
	float counter_ = 0.0f;
};
