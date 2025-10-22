#pragma once
#include "KamataEngine.h"

KamataEngine::Vector3 operator+(const KamataEngine::Vector3& v1, const KamataEngine::Vector3& v2);
KamataEngine::Vector3 operator-(const KamataEngine::Vector3& v1, const KamataEngine::Vector3& v2);
KamataEngine::Vector3 operator*(const KamataEngine::Vector3& v1, float v2);
KamataEngine::Vector3 operator/(const KamataEngine::Vector3& v1, float v2);

KamataEngine::Vector3& operator+=(KamataEngine::Vector3& lhv, const KamataEngine::Vector3& rhv);
KamataEngine::Vector3& operator-=(KamataEngine::Vector3& lhv, const KamataEngine::Vector3& rhv);
KamataEngine::Vector3& operator*=(KamataEngine::Vector3& v, float s);
KamataEngine::Vector3& operator/=(KamataEngine::Vector3& v, float s);

struct AABB {

	KamataEngine::Vector3 min;
	KamataEngine::Vector3 max;
};

class Math {
public:

	/// <summary>
	/// 行列の加算
	/// </summary>
	/// <param name="m1">変数1</param>
	/// <param name="m2">変数2</param>
	/// <returns>値</returns>
	KamataEngine::Matrix4x4 Add(const KamataEngine::Matrix4x4& m1, const KamataEngine::Matrix4x4& m2);

	/// <summary>
	/// 行列の減算
	/// </summary>
	/// <param name="m1">変数1</param>
	/// <param name="m2">変数2</param>
	/// <returns>値</returns>
	KamataEngine::Matrix4x4 Subtract(const KamataEngine::Matrix4x4& m1, const KamataEngine::Matrix4x4& m2);

	/// <summary>
	/// 行列の積
	/// </summary>
	/// <param name="m1">変数1</param>
	/// <param name="m2">変数2</param>
	/// <returns>値</returns>
	KamataEngine::Matrix4x4 Multiply(const KamataEngine::Matrix4x4& m1, const KamataEngine::Matrix4x4& m2);

	/// <summary>
	/// 逆行列
	/// </summary>
	/// <param name="m">変数</param>
	/// <returns>値</returns>
	KamataEngine::Matrix4x4 Inverse(const KamataEngine::Matrix4x4& m);

	/// <summary>
	/// 逆行列(アフィン変換用)
	/// </summary>
	/// <param name="m"></param>
	/// <returns></returns>
	KamataEngine::Matrix4x4 InverseAffine(const KamataEngine::Matrix4x4& m);

	/// <summary>
	/// 転置行列
	/// </summary>
	/// <param name="m">変数</param>
	/// <returns>値</returns>
	KamataEngine::Matrix4x4 Transpose(const KamataEngine::Matrix4x4& m);

	/// <summary>
	/// 単位行列の作成
	/// </summary>
	/// <returns></returns>
	KamataEngine::Matrix4x4 MakeIdentity();

	/// <summary>
	/// 拡大縮小行列
	/// </summary>
	/// <param name="scale">変数</param>
	/// <returns>値</returns>
	KamataEngine::Matrix4x4 MakeScaleMatrix(const KamataEngine::Vector3& scale);

	/// <summary>
	/// 平行移動行列
	/// </summary>
	/// <param name="translate">変数</param>
	/// <returns>値</returns>
	KamataEngine::Matrix4x4 MakeTranslationMatrix(const KamataEngine::Vector3& translate);

	/// <summary>
	/// X軸回転行列
	/// </summary>
	/// <param name="radian"></param>
	/// <returns></returns>
	KamataEngine::Matrix4x4 MakeRotateXMatrix(float radian);

	/// <summary>
	/// Y軸回転行列
	/// </summary>
	/// <param name="radian"></param>
	/// <returns></returns>
	KamataEngine::Matrix4x4 MakeRotateYMatrix(float radian);

	/// <summary>
	/// Z軸回転行列
	/// </summary>
	/// <param name="radion"></param>
	/// <returns></returns>
	KamataEngine::Matrix4x4 MakeRotateZMatrix(float radian);

	/// <summary>
	/// 3次元のアフィン変換行列
	/// </summary>
	/// <param name="scale">拡大縮小</param>
	/// <param name="rotate">回転</param>
	/// <param name="translate">平行移動</param>
	/// <returns>変換した値</returns>
	KamataEngine::Matrix4x4 MakeAffineMatrix(const KamataEngine::Vector3& scale, const KamataEngine::Vector3& rotate, const KamataEngine::Vector3& translate);

	/// <summary>
	/// 行列を計算・転送する
	/// </summary>
	/// <param name="worldtransfrom"></param>
	void worldTransformUpdate(KamataEngine::WorldTransform& worldtransfrom);

	/// <summary>
	/// 加算
	/// </summary>
	/// <param name="v1">変数1</param>
	/// <param name="v2">変数2</param>
	/// <returns>それぞれの合計値</returns>
	KamataEngine::Vector3 Add(const KamataEngine::Vector3& v1, const KamataEngine::Vector3& v2);

	/// <summary>
	/// 減算
	/// </summary>
	/// <param name="v1">変数1</param>
	/// <param name="v2">変数2</param>
	/// <returns>それぞれの合計値</returns>
	KamataEngine::Vector3 Subtract(const KamataEngine::Vector3& v1, const KamataEngine::Vector3& v2);

	/// <summary>
	/// 乗算
	/// </summary>
	/// <param name="v">変数1</param>
	/// <param name="scalar">変数2</param>
	/// <returns>それぞれの合計値</returns>
	KamataEngine::Vector3 Multiply(const KamataEngine::Vector3& v, float scalar);

	/// <summary>
	/// 内積
	/// </summary>
	/// <param name="v1">変数1</param>
	/// <param name="v2">変数2</param>
	/// <returns>合計値</returns>
	float Dot(const KamataEngine::Vector3& v1, const KamataEngine::Vector3& v2);

	/// <summary>
	/// 長さ(ノルム)
	/// </summary>
	/// <param name="v">変数</param>
	/// <returns>長さの値</returns>
	float Length(const KamataEngine::Vector3& v);

	/// <summary>
	/// 正規化
	/// </summary>
	/// <param name="v">変数</param>
	/// <returns>正規化した値</returns>
	KamataEngine::Vector3 Normalize(const KamataEngine::Vector3& v);

	KamataEngine::Vector3 Lerp(const KamataEngine::Vector3& v1, const KamataEngine::Vector3& v2, float t);

	/// <summary>
	/// 座標変換行列
	/// </summary>
	/// <param name="vector"></param>
	/// <param name="matrix"></param>
	/// <returns>値</returns>
	KamataEngine::Vector3 Transform(const KamataEngine::Vector3& vector, KamataEngine::Matrix4x4& matrix);

	/// <summary>
	/// イージング 加速
	/// </summary>
	/// <param name="t">時間</param>
	/// <param name="x1">イージングの始点</param>
	/// <param name="x2">イージングの終点</param>
	/// <returns>x1 と x2 の間でイージングされた値</returns>
	float EaseIn(float t, float x1, float x2);

	/// <summary>
	/// イージング 減速
	/// </summary>
	/// <param name="t">時間</param>
	/// <param name="x1">イージングの始点</param>
	/// <param name="x2">イージングの終点</param>
	/// <returns>x1 と x2 の間でイージングされた値</returns>
	float EaseOut(float t, float x1, float x2);

	/// <summary>
	/// イージング 加減速
	/// </summary>
	/// <param name="t">時間</param>
	/// <param name="x1">イージングの始点</param>
	/// <param name="x2">イージングの終点</param>
	/// <returns>x1 と x2 の間でイージングされた値</returns>
	float EaseInOut(float t, float x1, float x2);

	/// <summary>
	/// AABB同士の当たり判定
	/// </summary>
	/// <param name="aabb1"></param>
	/// <param name="aabb2"></param>
	/// <returns></returns>
	bool IsCollision(const AABB& aabb1, const AABB& aabb2);

	inline float ToRadians(float degrees) { return degrees * (3.1415f / 180.0f); }
	inline float ToDegrees(float radians) { return radians * (180.0f / 3.1415f); }

};

