#define NOMINMAX 
#include "CameraController.h"
#include "Player.h"
#include "MapChipField.h"
#include <algorithm>
#include <cassert>
#include <numbers>
#include <cmath>

using namespace KamataEngine;

// カメラコントローラー初期化
void CameraController::Initialize(KamataEngine::Camera* camera) {

	camera_ = camera;

	// 移動範囲の指定（デフォルト）
	movaleArea_.left = -100.0f;
	movaleArea_.right = 100.0f;
	movaleArea_.bottom = -100.0f;
	movaleArea_.top = 100.0f;

	// カメラの初期化
	camera_->Initialize();
}

// カメラコントローラーの更新
void CameraController::Update() {

	// 必要な参照を取得
	const WorldTransform& targetWordTransform = target_->GetWorldTransform();
	const Vector3& targetVelocity = target_->getvelocity();

	// 追尾対象とオフセットと追尾対象の速度からカメラ「目標座標」を計算（補正前）
	targetposition_ = targetWordTransform.translation_ + targetOffset_ + targetVelocity * kVelocitybias;

	// 可動領域にマージンを加えた最終クランプ境界
	const float minX = movaleArea_.left + margin.left;
	const float maxX = movaleArea_.right + margin.right;
	const float minY = movaleArea_.bottom + margin.bottom;
	const float maxY = movaleArea_.top + margin.top;

	// 目標座標をクランプして補間する。
	// 重要: movaleArea_ は SetMapField でビューポート+パディング分を縮めて設定しているため、
	// プレイヤーがマップ端から paddingTiles_ 分手前に達したらここで追従が止まる。
	Vector3 clampedTarget = targetposition_;
	clampedTarget.x = std::clamp(clampedTarget.x, minX, maxX);
	clampedTarget.y = std::clamp(clampedTarget.y, minY, maxY);

	// カメラ座標補間（現在座標 -> 「クランプ済み目標」へ）
	camera_->translation_ = math.Lerp(camera_->translation_, clampedTarget, kInterpolationRate);

	// 念のため最終的にもクランプ（補間誤差対策）
	camera_->translation_.x = std::clamp(camera_->translation_.x, minX, maxX);
	camera_->translation_.y = std::clamp(camera_->translation_.y, minY, maxY);

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

// マップ矩形を受け取り、ビューポートサイズ分だけ内側に縮めて可動範囲を設定する
void CameraController::SetmovaleArea(const Rect& area) {
	// カメラとターゲット間のZオフセット（正の距離）
	float camDist = std::abs(targetOffset_.z);

	// 垂直半分のワールドサイズ： tan(fov/2) * distance
	float halfVert = std::tan(camera_->fovAngleY * 0.5f) * camDist;
	// 水平方向はアスペクト比から計算
	float halfHorz = halfVert * camera_->aspectRatio;

	// マップ矩形をビューポート分だけ内側に縮める
	Rect shrunk = area;
	shrunk.left += halfHorz;
	shrunk.right -= halfHorz;
	shrunk.bottom += halfVert;
	shrunk.top -= halfVert;

	// マップがビューポートより小さい場合は中央に固定（左右・上下それぞれ）
	if (shrunk.left > shrunk.right) {
		float centerX = (area.left + area.right) * 0.5f;
		shrunk.left = shrunk.right = centerX;
	}
	if (shrunk.bottom > shrunk.top) {
		float centerY = (area.bottom + area.top) * 0.5f;
		shrunk.bottom = shrunk.top = centerY;
	}

	movaleArea_ = shrunk;
}

// マップ情報を渡して「タイル単位のパディング」で可動範囲を設定する
void CameraController::SetMapField(MapChipField* mapField, int paddingTiles) {
	// 保存
	mapField_ = mapField;
	paddingTiles_ = paddingTiles < 0 ? 0 : paddingTiles;

	if (!mapField_) {
		return;
	}

	// マップ四隅のワールド座標を取得（安全チェック付き）
	uint32_t numH = mapField_->GetNumBlockHorizontal();
	uint32_t numV = mapField_->GetNumBlockVirtical();

	Vector3 p00 = mapField_->GetMapChipPositionByIndex(0, 0);
	Vector3 pX0 = mapField_->GetMapChipPositionByIndex((numH > 0) ? numH - 1 : 0, 0);
	Vector3 p0Y = mapField_->GetMapChipPositionByIndex(0, (numV > 0) ? numV - 1 : 0);
	Vector3 pXY = mapField_->GetMapChipPositionByIndex((numH > 0) ? numH - 1 : 0, (numV > 0) ? numV - 1 : 0);

	float minX = std::min({p00.x, pX0.x, p0Y.x, pXY.x});
	float maxX = std::max({p00.x, pX0.x, p0Y.x, pXY.x});
	float minY = std::min({p00.y, pX0.y, p0Y.y, pXY.y});
	float maxY = std::max({p00.y, pX0.y, p0Y.y, pXY.y});

	Rect area{};
	area.left = minX;
	area.right = maxX;
	area.bottom = minY;
	area.top = maxY;

	// タイル数からワールド単位のパディング量を計算
	float padX = static_cast<float>(paddingTiles_) * mapField_->GetBlockWidth();
	float padY = static_cast<float>(paddingTiles_) * mapField_->GetBlockHeight();

	// さらにビューポート半幅/半高さを考慮して内側に縮める（SetmovaleArea と同様だがパディングを追加）
	float camDist = std::abs(targetOffset_.z);
	float halfVert = std::tan(camera_->fovAngleY * 0.5f) * camDist;
	float halfHorz = halfVert * camera_->aspectRatio;

	Rect shrunk = area;
	shrunk.left += halfHorz + padX;
	shrunk.right -= halfHorz + padX;
	shrunk.bottom += halfVert + padY;
	shrunk.top -= halfVert + padY;

	// マップがビューポート+パディングより小さい場合は中央に固定
	if (shrunk.left > shrunk.right) {
		float centerX = (area.left + area.right) * 0.5f;
		shrunk.left = shrunk.right = centerX;
	}
	if (shrunk.bottom > shrunk.top) {
		float centerY = (area.bottom + area.top) * 0.5f;
		shrunk.bottom = shrunk.top = centerY;
	}

	movaleArea_ = shrunk;
}