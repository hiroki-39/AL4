#define NOMINMAX 
#include "Enemy.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <numbers>
#include <vector>

using namespace KamataEngine;

void Enemy::Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position) {
	// NUllチェック
	assert(model);

	// 初期化
	model_ = model;
	camera_ = camera;
	worldTransformEnemy_.Initialize();
	worldTransformEnemy_.translation_ = position;

	// 初期位置保存（挙動で参照）
	initialPosition_ = position;

	// 速度設定（デフォルトは左へ）
	velocity_ = {-kWalkSpeed, 0.0f, 0.0f};
}

void Enemy::Update() {

	if (behaviorRequest_ != Behavior::kUnknown) {

		// 振るまいを変更する
		behavior_ = behaviorRequest_;

		// 各振るまいごとの初期化
		switch (behavior_) {
		case Behavior::kDefeated:
		default:
			counter_ = 0;
			break;
		}

		// 振るまいリクエストをリセット
		behaviorRequest_ = Behavior::kUnknown;
	}

	// 敵の AABB 四隅位置を取得
	auto CornerPositionEnemy = [this](const Vector3& center, int corner) -> Vector3 {
		Vector3 offdetTable[] = {
		    {+kWidth / 2.0f, -kHeight / 2.0f, 0},
            {-kWidth / 2.0f, -kHeight / 2.0f, 0},
            {+kWidth / 2.0f, +kHeight / 2.0f, 0},
            {-kWidth / 2.0f, +kHeight / 2.0f, 0}
        };
		return center + offdetTable[corner];
	};

	// 指定位置の AABB 四隅でブロック判定（プレイヤーの方式）
	auto IsBlockedAt = [&](const Vector3& pos)->bool {
		if (!mapField_) return false;
		for (int i = 0; i < 4; ++i) {
			Vector3 cornerPos = CornerPositionEnemy(pos, i);
			IndexSet idx = mapField_->GetMapChipIndexSetByPosition(cornerPos);
			if (mapField_->GetMapChipTypeByIndex(idx.xIndex, idx.yIndex) == MapChipType::kBlock) {
				return true;
			}
		}
		return false;
	};

	// 指定の移動量 delta をマップとの衝突に基づき補正する
	// pos は現在位置、delta は移動ベクトル（参照で補正される）。戻り値は補正後に少しでも移動したか。
	auto ResolveMapCollision = [&](const Vector3& pos, Vector3& delta) -> bool {
		if (!mapField_) {
			// マップ情報が無ければそのまま移動
			return (std::fabs(delta.x) > 1e-6f) || (std::fabs(delta.y) > 1e-6f);
		}

		// Up 判定（delta.y > 0） - Player::CollisionDetectionUp 互換ロジック
		if (delta.y > 0.0f) {
			// 移動後の4点
			std::array<Vector3, 4> positionsNew;
			for (uint32_t i = 0; i < positionsNew.size(); ++i) {
				positionsNew[i] = CornerPositionEnemy(pos + delta, i);
			}

			bool hit = false;
			IndexSet idx;
			MapChipType mapChipType;
			MapChipType mapChipTypeNext;

			// 左上
			idx = mapField_->GetMapChipIndexSetByPosition(positionsNew[3]); // kLeftTop
			mapChipType = mapField_->GetMapChipTypeByIndex(idx.xIndex, idx.yIndex);
			mapChipTypeNext = mapField_->GetMapChipTypeByIndex(idx.xIndex, idx.yIndex + 1);
			if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) hit = true;

			// 右上
			idx = mapField_->GetMapChipIndexSetByPosition(positionsNew[2]); // kRightTop
			mapChipType = mapField_->GetMapChipTypeByIndex(idx.xIndex, idx.yIndex);
			mapChipTypeNext = mapField_->GetMapChipTypeByIndex(idx.xIndex, idx.yIndex + 1);
			if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) hit = true;

			if (hit) {
				IndexSet idx2 = mapField_->GetMapChipIndexSetByPosition(pos + delta + Vector3(0, +kHeight / 2.0f, 0));
				IndexSet idxNow = mapField_->GetMapChipIndexSetByPosition(pos + Vector3(0, +kHeight / 2.0f, 0));
				if (idxNow.yIndex != idx2.yIndex) {
					RangeRect rect = mapField_->GetRectIndex(idx2.xIndex, idx2.yIndex);
					// y をめり込み外に補正
					delta.y = std::max<float>(0.0f, rect.bottom - pos.y - (kHeight / 2.0f + kBlank));
				}
			}
		}

		// Down 判定（delta.y < 0）
		if (delta.y < 0.0f) {
			std::array<Vector3, 4> positionsNew;
			for (uint32_t i = 0; i < positionsNew.size(); ++i) {
				positionsNew[i] = CornerPositionEnemy(pos + delta, i);
			}

			bool hit = false;
			IndexSet idx;
			MapChipType mapChipType;
			MapChipType mapChipTypeNext;

			// 左下
			idx = mapField_->GetMapChipIndexSetByPosition(positionsNew[1]); // kLeftBottom
			mapChipType = mapField_->GetMapChipTypeByIndex(idx.xIndex, idx.yIndex);
			mapChipTypeNext = mapField_->GetMapChipTypeByIndex(idx.xIndex, idx.yIndex - 1);
			if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) hit = true;

			// 右下
			idx = mapField_->GetMapChipIndexSetByPosition(positionsNew[0]); // kRightBottom
			mapChipType = mapField_->GetMapChipTypeByIndex(idx.xIndex, idx.yIndex);
			mapChipTypeNext = mapField_->GetMapChipTypeByIndex(idx.xIndex, idx.yIndex - 1);
			if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) hit = true;

			if (hit) {
				IndexSet idx2 = mapField_->GetMapChipIndexSetByPosition(pos + delta + Vector3(0, -kHeight / 2.0f, 0));
				IndexSet idxNow = mapField_->GetMapChipIndexSetByPosition(pos + Vector3(0, -kHeight / 2.0f, 0));
				if (idxNow.yIndex != idx2.yIndex) {
					RangeRect rect = mapField_->GetRectIndex(idx2.xIndex, idx2.yIndex);
					delta.y = std::min<float>(0.0f, rect.top - pos.y + (kHeight / 2.0f + kBlank));
				}
			}
		}

		// Right 判定（delta.x > 0）
		if (delta.x > 0.0f) {
			std::array<Vector3, 4> positionsNew;
			for (uint32_t i = 0; i < positionsNew.size(); ++i) {
				positionsNew[i] = CornerPositionEnemy(pos + delta, i);
			}

			bool hit = false;
			IndexSet idx;
			MapChipType mapChipType;
			MapChipType mapChipTypeNext;

			// 右上
			idx = mapField_->GetMapChipIndexSetByPosition(positionsNew[2]); // kRightTop
			mapChipType = mapField_->GetMapChipTypeByIndex(idx.xIndex, idx.yIndex);
			mapChipTypeNext = mapField_->GetMapChipTypeByIndex(idx.xIndex - 1, idx.yIndex);
			if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) hit = true;

			// 右下
			idx = mapField_->GetMapChipIndexSetByPosition(positionsNew[0]); // kRightBottom
			mapChipType = mapField_->GetMapChipTypeByIndex(idx.xIndex, idx.yIndex);
			mapChipTypeNext = mapField_->GetMapChipTypeByIndex(idx.xIndex - 1, idx.yIndex);
			if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) hit = true;

			if (hit) {
				// 壁側の記録は不要だが、めり込みを排除
				IndexSet idx2 = mapField_->GetMapChipIndexSetByPosition(pos + delta + Vector3(+kWidth / 2.0f, 0, 0));
				IndexSet idxNow = mapField_->GetMapChipIndexSetByPosition(pos + Vector3(+kWidth / 2.0f, 0, 0));
				if (idxNow.xIndex != idx2.xIndex) {
					RangeRect rect = mapField_->GetRectIndex(idx2.xIndex, idx2.yIndex);
					delta.x = std::max<float>(0.0f, rect.left - pos.x - (kWidth / 2.0f + kBlank));
				}
			}
		}

		// Left 判定（delta.x < 0）
		if (delta.x < 0.0f) {
			std::array<Vector3, 4> positionsNew;
			for (uint32_t i = 0; i < positionsNew.size(); ++i) {
				positionsNew[i] = CornerPositionEnemy(pos + delta, i);
			}

			bool hit = false;
			IndexSet idx;
			MapChipType mapChipType;
			MapChipType mapChipTypeNext;

			// 左上
			idx = mapField_->GetMapChipIndexSetByPosition(positionsNew[3]); // kLeftTop
			mapChipType = mapField_->GetMapChipTypeByIndex(idx.xIndex, idx.yIndex);
			mapChipTypeNext = mapField_->GetMapChipTypeByIndex(idx.xIndex + 1, idx.yIndex);
			if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) hit = true;

			// 左下
			idx = mapField_->GetMapChipIndexSetByPosition(positionsNew[1]); // kLeftBottom
			mapChipType = mapField_->GetMapChipTypeByIndex(idx.xIndex, idx.yIndex);
			mapChipTypeNext = mapField_->GetMapChipTypeByIndex(idx.xIndex + 1, idx.yIndex);
			if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) hit = true;

			if (hit) {
				IndexSet idx2 = mapField_->GetMapChipIndexSetByPosition(pos + delta + Vector3(-kWidth / 2.0f, 0, 0));
				IndexSet idxNow = mapField_->GetMapChipIndexSetByPosition(pos + Vector3(-kWidth / 2.0f, 0, 0));
				if (idxNow.xIndex != idx2.xIndex) {
					RangeRect rect = mapField_->GetRectIndex(idx2.xIndex, idx2.yIndex);
					delta.x = std::min<float>(0.0f, rect.right + (kWidth / 2.0f + kBlank) - pos.x);
				}
			}
		}

		// 移動できる量が残っているかを返す
		return (std::fabs(delta.x) > 1e-6f) || (std::fabs(delta.y) > 1e-6f);
	};

	// 種類別挙動
	switch (type_) {
	case EnemyType::kLR: {
		// 左右往復（初期位置を基準に sin 波で動かす）
		walkTimer += 1.0f / 60.0f;
		float dx = std::sinf(walkTimer * kLRFrequency * 2.0f * std::numbers::pi_v<float>) * kLRAmplitude;
		worldTransformEnemy_.translation_.x = initialPosition_.x + dx;

		// 少し見た目を揺らす
		worldTransformEnemy_.rotation_.y = math.EaseInOut(0.5f + 0.5f * std::sinf(walkTimer * 2.0f), kWalkMotionAngleStart, kWalkMotionAngleEnd);

		// 行列の変換と転送
		math.worldTransformUpdate(worldTransformEnemy_);
		break;
	}
	case EnemyType::kUD: {
		// 上下往復
		walkTimer += 1.0f / 60.0f;
		float dy = std::sinf(walkTimer * kUDFrequency * 2.0f * std::numbers::pi_v<float>) * kUDAmplitude;
		worldTransformEnemy_.translation_.y = initialPosition_.y + dy;

		worldTransformEnemy_.rotation_.y = math.EaseInOut(0.5f + 0.5f * std::sinf(walkTimer * 2.0f), kWalkMotionAngleStart, kWalkMotionAngleEnd);

		math.worldTransformUpdate(worldTransformEnemy_);
		break;
	}
	case EnemyType::kFlee: {
		// 近づいたらプレイヤーから逃げる（プレイヤーと同等のマップ判定で移動）
		if (player_) {
			Vector3 ppos = player_->GetWorldTransform().translation_;
			Vector3 dir = worldTransformEnemy_.translation_ - ppos;
			float dist = math.Length(dir);
			if (dist < kFleeTriggerDist && dist > 1e-6f) {
				Vector3 nd = math.Normalize(dir);

				// 希望移動ベクトル
				Vector3 current = worldTransformEnemy_.translation_;
				Vector3 desiredDelta = nd * kFleeSpeed;

				// まずそのまま移動できるか試す（マップ衝突補正を通す）
				Vector3 testDelta = desiredDelta;
				bool moved = ResolveMapCollision(current, testDelta);
				if (moved) {
					worldTransformEnemy_.translation_ += testDelta;
				} else {
					// 壁に阻まれるなら X/Y 成分ごとに試す（壁に沿う）
					Vector3 testDeltaX = Vector3(desiredDelta.x, 0.0f, 0.0f);
					Vector3 testDeltaY = Vector3(0.0f, desiredDelta.y, 0.0f);
					bool okX = ResolveMapCollision(current, testDeltaX);
					bool okY = ResolveMapCollision(current, testDeltaY);

					if (okX && !okY) {
						worldTransformEnemy_.translation_.x += testDeltaX.x;
					} else if (!okX && okY) {
						worldTransformEnemy_.translation_.y += testDeltaY.y;
					} else if (okX && okY) {
						// 両方行けるなら大きい成分を優先
						if (std::fabs(desiredDelta.x) > std::fabs(desiredDelta.y)) {
							worldTransformEnemy_.translation_.x += testDeltaX.x;
						} else {
							worldTransformEnemy_.translation_.y += testDeltaY.y;
						}
					} else {
						// さらに壁に沿う方向（垂直成分）を試す
						Vector3 perp1 = Vector3(-nd.y, nd.x, 0.0f);
						Vector3 perp2 = Vector3(nd.y, -nd.x, 0.0f);
						if (math.Length(perp1) > 1e-6f) {
							Vector3 td = math.Normalize(perp1) * kFleeSpeed;
							if (ResolveMapCollision(current, td)) {
								worldTransformEnemy_.translation_ += td;
							}
						}
						if (!IsBlockedAt(worldTransformEnemy_.translation_)) {
							// moved by perp1 above
						} else if (math.Length(perp2) > 1e-6f) {
							Vector3 td2 = math.Normalize(perp2) * kFleeSpeed;
							if (ResolveMapCollision(current, td2)) {
								worldTransformEnemy_.translation_ += td2;
							}
						}
						// それでも動けなければ停止
					}
				}
			} else {
				// 閾値外はアイドル
				walkTimer += 1.0f / 60.0f;
				worldTransformEnemy_.rotation_.y = 0.2f * std::sinf(walkTimer * 2.0f);
			}
		} else {
			walkTimer += 1.0f / 60.0f;
			worldTransformEnemy_.rotation_.y = 0.2f * std::sinf(walkTimer * 2.0f);
		}

		// マップ外に出ないようにクランプ（境界は MapChipField に基づく）
		if (mapField_) {
			const float minX = 0.0f;
			const float maxX = (static_cast<float>(mapField_->GetNumBlockHorizontal() - 1)) * mapField_->GetBlockWidth();
			const float minY = 0.0f;
			const float maxY = (static_cast<float>(mapField_->GetNumBlockVirtical() - 1)) * mapField_->GetBlockHeight();

			worldTransformEnemy_.translation_.x = std::clamp(worldTransformEnemy_.translation_.x, minX, maxX);
			worldTransformEnemy_.translation_.y = std::clamp(worldTransformEnemy_.translation_.y, minY, maxY);
		}

		math.worldTransformUpdate(worldTransformEnemy_);
		break;
	}
	case EnemyType::kSplit:
		// 分裂型は当たり判定で分裂処理を GameScene が行うため、ここでは Idle 見た目アニメ
		walkTimer += 1.0f / 60.0f;
		worldTransformEnemy_.rotation_.y = 0.3f * std::sinf(walkTimer * 2.0f);
		math.worldTransformUpdate(worldTransformEnemy_);
		break;
	default:
		// デフォルト歩行（既存互換）
		/*---　歩き　---*/

		// タイマーの加算
		walkTimer += 1.0f / 60.0f;

		worldTransformEnemy_.rotation_.y += 0.1f;

		// 行列の変換と転送
		math.worldTransformUpdate(worldTransformEnemy_);
		break;
	}

	// やられ演出処理
	if (behavior_ == Behavior::kDefeated) {
		// タイマー
		counter_ += 1.0f / 60.0f;

		worldTransformEnemy_.rotation_.y += 0.3f;
		worldTransformEnemy_.rotation_.x = math.EaseOut(counter_ / kDefeatedTime, kDefeatedMotionAngleStart, kDefeatedMotionAngleEnd);

		// スケールを徐々に小さくして消す
		{
			float t = counter_ / kDefeatedTime;
			if (t > 1.0f) t = 1.0f;
			// イージングで自然に縮む
			float s = math.EaseOut(t, 1.0f, 0.0f);
			worldTransformEnemy_.scale_ = {s, s, s};
		}

		// 行列の変換と転送（演出も適用）
		math.worldTransformUpdate(worldTransformEnemy_);

		if (counter_ >= kDefeatedTime) {
			isDead_ = true;
		}
	}
}

void Enemy::Draw() {

	if (!isDead_) {
		// モデルの描画
		model_->Draw(worldTransformEnemy_, *camera_);
	}
}

void Enemy::OnCollision(const Player* player) {

	(void)player;

	// 既にやられているなら何もしない
	if (behavior_ == Behavior::kDefeated) {
		return;
	}

	// プレイヤーの弾が当たった場合：非表示にしてやられ演出に遷移
	isVisible_ = false;
	isCollisionDisabled_ = true;

	// 必要ならやられ演出を続けて最終的に isDead_ にするためのリクエストを出す
	behaviorRequest_ = Behavior::kDefeated;
}

KamataEngine::Vector3 Enemy::GetWorldPosition() {
	// ワールド座標を入れる変数
	Vector3 worldPos;

	// ワールド行列の平行移動成分を取得（ワールド座標）
	worldPos.x = worldTransformEnemy_.matWorld_.m[3][0];
	worldPos.y = worldTransformEnemy_.matWorld_.m[3][1];
	worldPos.z = worldTransformEnemy_.matWorld_.m[3][2];

	return worldPos;
}

AABB Enemy::GetAABB() {

	Vector3 worldPos = GetWorldPosition();

	AABB aabb;

	aabb.min = {worldPos.x - kWidth / 2.0f, worldPos.y - kHeight / 2.0f, worldPos.z - kWidth / 2.0f};
	aabb.max = {worldPos.x + kWidth / 2.0f, worldPos.y + kHeight / 2.0f, worldPos.z + kWidth / 2.0f};

	return aabb;
}
