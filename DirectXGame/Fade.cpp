#include "Fade.h"
#include <algorithm>
#include <cassert>
#include <numbers>

using namespace KamataEngine;

void Fade::Initialize() {

	// スプライトの生成
	sprite_ = Sprite::Create(0, Vector2{});

	sprite_->SetSize(Vector2(WinApp::kWindowWidth, WinApp::kWindowHeight));
	sprite_->SetColor(Vector4(0.0f, 0.0f, 0.0f, 1.0f));
}

void Fade::Update() {

	// フェード状態による分岐
	switch (status_) {
	case Fade::Status::None:
		// 何もなし

		break;
	case Fade::Status::FadeIn:
		/*--- フェードイン中の更新処理 ---*/

		// 1フレーム分の秒数をカウントアップ
		counter_ += 1.0f / 60.0f;

		// フェード持続時間に到達したら止める
		if (counter_ >= duration_) {
			counter_ = duration_;
		}

		// スプライトのアルファ値を更新
		sprite_->SetColor(Vector4(0, 0, 0, std::clamp(1.0f - counter_ / duration_, 0.0f, 1.0f)));

		break;
	case Fade::Status::FadeOut:
		/*--- フェードアウト中の更新処理 ---*/

		// 1フレーム分の秒数をカウントアップ
		counter_ += 1.0f / 60.0f;

		// フェード持続時間に到達したら止める
		if (counter_ >= duration_) {
			counter_ = duration_;
		}

		// スプライトのアルファ値を更新
		sprite_->SetColor(Vector4(0.0f, 0.0f, 0.0f, std::clamp(counter_ / duration_, 0.0f, 1.0f)));

		break;
	}
}

void Fade::Draw() {

	if (status_ == Status::None) {
		return;
	}

	// directXcommonのインスタンスを取得
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// 3Dモデルの前処理
	Sprite::PreDraw(dxCommon->GetCommandList());

	// モデルの描画
	sprite_->Draw();

	// 3Dモデルの後処理
	Sprite::PostDraw();
}

void Fade::Start(Status status, float duration) {
	status_ = status;
	duration_ = duration;
	counter_ = 0.0f;
}

void Fade::Stop() { status_ = Status::None; }

bool Fade::IsFinished() const {

	// フェード状態による分岐
	switch (status_) {
	case Fade::Status::FadeIn:
	case Fade::Status::FadeOut:
		if (counter_ >= duration_) {
			return true;
		} else {
			return false;
		}
	}

	return true;
}
