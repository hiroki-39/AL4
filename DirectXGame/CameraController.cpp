#include "CameraController.h"
#include "Player.h"
#include <algorithm>
#include <cassert>
#include <numbers>

using namespace KamataEngine;

// カメラコントローラー初期化
void CameraController::Initialize(KamataEngine::Camera* camera) {

	camera_ = camera;

	// 移動範囲の指定
	movaleArea_.left = -100.0f;
	movaleArea_.right = 100.0f;
	movaleArea_.bottom = -100.0f;
	movaleArea_.top = 100.0f;

	// カメラの初期化
	camera_->Initialize();
}

// カメラコントローラーの更新
void CameraController::Update() {

	// 追尾対象のワールドトランスフォームを参照
	const WorldTransform& targetWordTransform = target_->GetWorldTransform();

	const Vector3& targetVelocity = target_->getvelocity();

	// 追尾対象とオフセットと追尾対象の速度からカメラ座標を計算
	targetposition_ = targetWordTransform.translation_ + targetOffset_ + targetVelocity * kVelocitybias;

	// カメラ座標補間
	camera_->translation_ = math.Lerp(camera_->translation_, targetposition_, kInterpolationRate);

	// カメラの移動範囲制限
	camera_->translation_.x = std::clamp(camera_->translation_.x, movaleArea_.left + margin.left, movaleArea_.right + margin.right);
	camera_->translation_.y = std::clamp(camera_->translation_.y, movaleArea_.bottom + margin.bottom, movaleArea_.top + margin.top);

	// 行列を更新
	camera_->UpdateMatrix();
}

// カメラコントローラーのリセット
void CameraController::Reset() {

	// 追尾対象のワールドトランスフォームを参照
	const WorldTransform& targetWordTransform = target_->GetWorldTransform();

	// 追尾対象とオフセットからカメラ座標を計算
	camera_->translation_ = targetWordTransform.translation_ + targetOffset_;
}