#pragma once
#include <cmath>
#include <math/Matrix4x4.h>
#include <math/Vector3.h>

struct Line {
	KamataEngine::Vector3 origin_;
	KamataEngine::Vector3 diff_;
};

struct Ray {
	KamataEngine::Vector3 origin_;
	KamataEngine::Vector3 diff_;
};

struct Segment {
	KamataEngine::Vector3 origin_;
	KamataEngine::Vector3 diff_;
};

struct Plane {
	KamataEngine::Vector3 normal_;
	float distance_;
};

/// <summary>
/// ベクトル、行列、物理計算で使う共通数学関数をまとめたクラス
///
/// ・Vector3 の基本演算
/// ・行列生成と座標変換
/// ・発射、摩擦、反射などの物理補助
/// を担当する
/// </summary>
class MyMath {
public:
	// =========================
	// Vector3 四則演算
	// =========================

	// 加算 v1 + v2
	static KamataEngine::Vector3 Add(const KamataEngine::Vector3& v1, const KamataEngine::Vector3& v2);

	// 減算 v1 - v2
	static KamataEngine::Vector3 Subtract(const KamataEngine::Vector3& v1, const KamataEngine::Vector3& v2);

	// += 相当
	static void AddAssign(KamataEngine::Vector3& v1, const KamataEngine::Vector3& v2);

	// -= 相当
	static void SubtractAssign(KamataEngine::Vector3& v1, const KamataEngine::Vector3& v2);

	// 乗算 Vector3 * scalar
	static KamataEngine::Vector3 Multiply(const KamataEngine::Vector3& v, float scalar);

	// 除算 Vector3 / scalar
	static KamataEngine::Vector3 Divide(const KamataEngine::Vector3& v, float scalar);

	// =========================
	// Vector3 補助
	// =========================

	// 内積を返す
	static float Dot(const KamataEngine::Vector3& v1, const KamataEngine::Vector3& v2);

	// 外積を返す
	static KamataEngine::Vector3 Cross(const KamataEngine::Vector3& v1, const KamataEngine::Vector3& v2);

	// ベクトルの長さを返す
	static float Length(const KamataEngine::Vector3& v);

	// ベクトルの長さの2乗を返す
	static float LengthSq(const KamataEngine::Vector3& v);

	// 正規化したベクトルを返す
	static KamataEngine::Vector3 Normalize(const KamataEngine::Vector3& v);

	// 2点間の距離を返す
	static float Distance(const KamataEngine::Vector3& v1, const KamataEngine::Vector3& v2);

	// 法線 normal を基準に vector を反射する
	static KamataEngine::Vector3 Reflect(const KamataEngine::Vector3& vector, const KamataEngine::Vector3& normal);

	// start から end へ t の割合で線形補間する
	static float Lerp(float start, float end, float t);

	// start から end へ t の割合で線形補間する
	static KamataEngine::Vector3 Lerp(const KamataEngine::Vector3& start, const KamataEngine::Vector3& end, float t);

	// 度数法からラジアンへ変換する
	static float ToRadian(float degree);

	// ラジアンから度数法へ変換する
	static float ToDegree(float radian);

	// =========================
	// 値の範囲制限
	// =========================

	// value を min から max の範囲に収める
	static float Clamp(float value, float min, float max);

	// =========================
	// 行列処理
	// =========================

	// 行列同士を乗算する
	static KamataEngine::Matrix4x4 Multiply(const KamataEngine::Matrix4x4& m1, const KamataEngine::Matrix4x4& m2);

	// 平行移動行列を作成する
	static KamataEngine::Matrix4x4 MakeTranslateMatrix(const KamataEngine::Vector3& translate);

	// 拡大縮小行列を作成する
	static KamataEngine::Matrix4x4 MakeScaleMatrix(const KamataEngine::Vector3& scale);

	// X軸回転行列を作成する
	static KamataEngine::Matrix4x4 MakeRotateXMatrix(float radius);

	// Y軸回転行列を作成する
	static KamataEngine::Matrix4x4 MakeRotateYMatrix(float radius);

	// Z軸回転行列を作成する
	static KamataEngine::Matrix4x4 MakeRotateZMatrix(float radius);

	// スケール、回転、平行移動を合成したアフィン行列を作成する
	static KamataEngine::Matrix4x4 MakeAffineMatrix(const KamataEngine::Vector3& scale, const KamataEngine::Vector3& rotate, const KamataEngine::Vector3& translate);

	// 透視投影行列を作成する
	static KamataEngine::Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip);

	// 正射影行列を作成する
	static KamataEngine::Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip);

	// ビューポート変換行列を作成する
	static KamataEngine::Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth);

	// 逆行列を作成する
	static KamataEngine::Matrix4x4 Inverse(const KamataEngine::Matrix4x4& m);

	// 座標を行列で変換する
	static KamataEngine::Vector3 Transform(const KamataEngine::Vector3& vector, const KamataEngine::Matrix4x4& matrix);

	// 法線を行列で変換する
	static KamataEngine::Vector3 TransformNormal(const KamataEngine::Vector3& vector, const KamataEngine::Matrix4x4& matrix);

	// =========================
	// 物理計算の補助処理
	// =========================

	// ドラッグ開始位置と終了位置から発射方向を計算する
	static KamataEngine::Vector3 CalcLaunchDirection(const KamataEngine::Vector3& dragStart, const KamataEngine::Vector3& dragEnd);

	// ドラッグ距離から発射速度を計算する
	static float CalcLaunchSpeed(float dragDistance, float maxDragDistance, float minSpeed, float maxSpeed);

	// XZ平面上で position += velocity * deltaTime を行う
	static void IntegrateXZ(KamataEngine::Vector3& position, const KamataEngine::Vector3& velocity, float deltaTime);

	// 速度全体に摩擦係数を掛ける
	static void ApplyFriction(KamataEngine::Vector3& velocity, float friction);

	// XZ平面の速度に摩擦係数を掛ける
	static void ApplyFrictionXZ(KamataEngine::Vector3& velocity, float friction);

	// 速度が最大速度を超えないように制限する
	static void ClampVelocity(KamataEngine::Vector3& velocity, float maxSpeed);

	// 速度が停止閾値以下か判定する
	static bool IsStopped(const KamataEngine::Vector3& velocity, float stopSpeed);

	// 指定方向へ加速する
	static void Accelerate(KamataEngine::Vector3& velocity, const KamataEngine::Vector3& direction, float acceleration, float deltaTime);

	// 速度方向を保ったまま減速する
	static void Decelerate(KamataEngine::Vector3& velocity, float deceleration, float deltaTime);

	// 質量と重力加速度から重力を計算する
	static KamataEngine::Vector3 CalcGravityForce(float mass, float gravityAcceleration = 9.8f);

	// XZ平面上で「／」反射板に当たったときの速度を返す
	static KamataEngine::Vector3 ReflectSlashXZ(const KamataEngine::Vector3& velocity);

	// XZ平面上で「＼」反射板に当たったときの速度を返す
	static KamataEngine::Vector3 ReflectBackSlashXZ(const KamataEngine::Vector3& velocity);

	// グリッドの壁または外周に当たったときの速度反転を行う
	static void ReflectGridBounceXZ(KamataEngine::Vector3& velocity, bool hitX, bool hitZ);
};
