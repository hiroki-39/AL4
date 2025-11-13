#include "TitleScene.h"
#include <numbers>

using namespace KamataEngine;

void TitleScene::Initialize() {

#pragma region "スカイドーム"
	/*-------------- スカイドームの初期化 --------------*/

	// スカイドームのモデルの生成
	modelSkydome_ = Model::CreateFromOBJ("SkyDome", true);

	// スカイドームの生成
	skydome_ = new Skydome();

	// スカイドームの初期化
	skydome_->Initialize(modelSkydome_, &camera_);

#pragma endregion

	// タイトルの読み込み
	titleHandle_ = TextureManager::Load("font/title.png");

	titleSprite_ = Sprite::Create(titleHandle_, {300, 100});

	// スタートボタンの読み込み
	buttonHandle_ = TextureManager::Load("font/start.png");

	buttonSprite_ = Sprite::Create(buttonHandle_, {340, 500});
	buttonSprite_->SetSize({600, 66});

	// カメラ初期化
	camera_.Initialize();

	const float kPlayerTitle = 2.0f;

	// タイトル(モデル)の追加初期化
	worldTransformTitle_.Initialize();

	worldTransformTitle_.scale_ = {kPlayerTitle, kPlayerTitle, kPlayerTitle};

	const float kPlayerScale = 10.0f;

	// プレイヤー(モデル)の初期化
	worldTransformPlayer_.Initialize();

	worldTransformPlayer_.scale_ = {kPlayerScale, kPlayerScale, kPlayerScale};

	worldTransformPlayer_.rotation_.y = 0.95f * std::numbers::pi_v<float>;

	worldTransformPlayer_.translation_.x = -2.0f;

	worldTransformPlayer_.translation_.y = -10.0f;

	// フェードの初期化
	fade_ = new Fade();
	fade_->Initialize();
	fade_->Start(Fade::Status::FadeIn, 1.0f);
}

void TitleScene::Update() {

	// スカイドームの描画
	skydome_->Update();

	// フェードの更新
	switch (phase_) {
	case Phase::kFadeIn:

		fade_->Update();

		if (fade_->IsFinished()) {
			phase_ = Phase::kMain;
		}

		break;
	case Phase::kMain:

		if (Input::GetInstance()->PushKey(DIK_SPACE)) {
			phase_ = Phase::kFadeOut;
			fade_->Start(Fade::Status::FadeOut, 1.0f);
		}

		break;
	case Phase::kFadeOut:

		fade_->Update();

		if (fade_->IsFinished()) {
			finished_ = true;
		}

		break;
	}

	counter_ += 1.0f / 60.0f;
	counter_ = std::fmod(counter_, kTimeTitleMove);

	float angle = counter_ / kTimeTitleMove * 2.0f * std::numbers::pi_v<float>;

	worldTransformTitle_.translation_.y = std::sin(angle) + 10.0f;

	// カメラの行列を転送
	camera_.TransferMatrix();

	// 行列の変換と転送
	math.worldTransformUpdate(worldTransformTitle_);

	// 行列の変換と転送
	math.worldTransformUpdate(worldTransformPlayer_);
}

void TitleScene::Draw() {

	// 3Dモデルの前処理
	Model::PreDraw();

	// スカイドームの描画
	skydome_->Draw();

	// スプライトの描画前処理
	Sprite::PreDraw();

	titleSprite_->Draw();

	buttonSprite_->Draw();

	// スプライトの描画後処理
	Sprite::PostDraw();

	fade_->Draw();

	Model::PostDraw();
}

TitleScene::~TitleScene() {
	// モデルの解放
	delete modelPlayer_;
	delete modelTitle_;

	// フェードの解放
	delete fade_;
}
