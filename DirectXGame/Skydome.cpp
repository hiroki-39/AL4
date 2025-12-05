#include "Skydome.h"
#include <cassert>

using namespace KamataEngine;

void Skydome::Initialize(Model* model, Camera* camera) {

	// NULLチェック
	assert(model);

	// 引数として受け取ったデータをメンバ変数に格納
	model_ = model;
	camera_ = camera;

	// ワールド変換データの初期化
	worldTransformSkydome_.Initialize();

	// スカイドームのワールド行列の初期化
	worldTransformSkydome_.translation_ = {0.0f, 0.0f, 0.0f};
}

void Skydome::Update() {

	// スカイドームのワールド行列の初期化
	worldTransformSkydome_.matWorld_ = math->MakeAffineMatrix(worldTransformSkydome_.scale_, worldTransformSkydome_.rotation_, worldTransformSkydome_.translation_);

	// ワールド変換データの行列を転送
	worldTransformSkydome_.TransferMatrix();
}

void Skydome::Draw() {

	// 3Dモデルの前処理
	Model::PreDraw();

	// モデルの描画
	model_->Draw(worldTransformSkydome_, *camera_);

	// 3Dモデルの後処理
	Model::PostDraw();
}