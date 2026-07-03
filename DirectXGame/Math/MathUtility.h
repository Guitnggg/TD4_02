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
	static float Dot(const KamataEngine::Vector3& v1, const KamataEngine::Vector3& v2);

	static float Length(const KamataEngine::Vector3& v);

	static KamataEngine::Vector3 Normalize(const KamataEngine::Vector3& v);

	// =========================
	// Clamp
	// =========================
	static float Clamp(float value, float min, float max);

	// =========================
	// Matrix
	// =========================
	static KamataEngine::Matrix4x4 Multiply(const KamataEngine::Matrix4x4& m1, const KamataEngine::Matrix4x4& m2);

	static KamataEngine::Matrix4x4 MakeTranslateMatrix(const KamataEngine::Vector3& translate);

	static KamataEngine::Matrix4x4 MakeScaleMatrix(const KamataEngine::Vector3& scale);

	static KamataEngine::Matrix4x4 MakeRotateXMatrix(float radius);
	static KamataEngine::Matrix4x4 MakeRotateYMatrix(float radius);
	static KamataEngine::Matrix4x4 MakeRotateZMatrix(float radius);

	static KamataEngine::Matrix4x4 MakeAffineMatrix(const KamataEngine::Vector3& scale, const KamataEngine::Vector3& rotate, const KamataEngine::Vector3& translate);

	static KamataEngine::Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip);

	static KamataEngine::Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip);

	static KamataEngine::Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth);

	static KamataEngine::Matrix4x4 Inverse(const KamataEngine::Matrix4x4& m);

	static KamataEngine::Vector3 Transform(const KamataEngine::Vector3& vector, const KamataEngine::Matrix4x4& matrix);

	static KamataEngine::Vector3 TransformNormal(const KamataEngine::Vector3& vector, const KamataEngine::Matrix4x4& matrix);

	// =========================
	// Physics helper
	// =========================
	static void IntegrateXZ(KamataEngine::Vector3& position, const KamataEngine::Vector3& velocity, float deltaTime);

	static void ApplyFrictionXZ(KamataEngine::Vector3& velocity, float friction);

	static KamataEngine::Vector3 ReflectSlashXZ(const KamataEngine::Vector3& velocity);

	static KamataEngine::Vector3 ReflectBackSlashXZ(const KamataEngine::Vector3& velocity);

	static void ReflectGridBounceXZ(KamataEngine::Vector3& velocity, bool hitX, bool hitZ);
};
