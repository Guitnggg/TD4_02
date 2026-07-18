#include "MathUtility.h"

using namespace KamataEngine;

// =========================
// Vector3 四則演算
// =========================
Vector3 MyMath::Add(const Vector3& v1, const Vector3& v2) { return { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z }; }

Vector3 MyMath::Subtract(const Vector3& v1, const Vector3& v2) { return { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z }; }

void MyMath::AddAssign(Vector3& v1, const Vector3& v2) {

	v1.x += v2.x;
	v1.y += v2.y;
	v1.z += v2.z;
}

void MyMath::SubtractAssign(Vector3& v1, const Vector3& v2) {
	v1.x -= v2.x;
	v1.y -= v2.y;
	v1.z -= v2.z;
}

Vector3 MyMath::Multiply(const Vector3& v, float scalar) { return { v.x * scalar, v.y * scalar, v.z * scalar }; }

Vector3 MyMath::Divide(const Vector3& v, float scalar) {
	// 0 除算防止（最低限）
	if (scalar == 0.0f) {
		return { 0.0f, 0.0f, 0.0f };
	}
	return { v.x / scalar, v.y / scalar, v.z / scalar };
}

// =========================
// Vector3 補助
// =========================
float MyMath::Dot(const Vector3& v1, const Vector3& v2) { return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z; }

Vector3 MyMath::Cross(const Vector3& v1, const Vector3& v2) {
	return {v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x};
}

float MyMath::Length(const Vector3& v) { return std::sqrt(LengthSq(v)); }

float MyMath::LengthSq(const Vector3& v) { return Dot(v, v); }

Vector3 MyMath::Normalize(const Vector3& v) {

	float len = Length(v);
	if (len == 0.0f) {
		return { 0.0f, 0.0f, 0.0f };
	}
	return Divide(v, len);
}

float MyMath::Distance(const Vector3& v1, const Vector3& v2) { return Length(Subtract(v1, v2)); }

Vector3 MyMath::Reflect(const Vector3& vector, const Vector3& normal) {
	const Vector3 normalizedNormal = Normalize(normal);
	if (LengthSq(normalizedNormal) == 0.0f) {
		return vector;
	}

	return Subtract(vector, Multiply(normalizedNormal, 2.0f * Dot(vector, normalizedNormal)));
}

float MyMath::Lerp(float start, float end, float t) { return start + (end - start) * t; }

Vector3 MyMath::Lerp(const Vector3& start, const Vector3& end, float t) {
	return {Lerp(start.x, end.x, t), Lerp(start.y, end.y, t), Lerp(start.z, end.z, t)};
}

float MyMath::ToRadian(float degree) {
	constexpr float kPi = 3.14159265358979323846f;
	return degree * kPi / 180.0f;
}

float MyMath::ToDegree(float radian) {
	constexpr float kPi = 3.14159265358979323846f;
	return radian * 180.0f / kPi;
}

// =========================
// 値の範囲制限
// =========================
float MyMath::Clamp(float value, float min, float max) {
	if (value < min) {
		return min;
	}
	if (value > max) {
		return max;
	}
	return value;
}

// =========================
// 行列処理
// =========================
Matrix4x4 MyMath::Multiply(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 result{};
	for (int32_t i = 0; i < 4; ++i) {
		for (int32_t j = 0; j < 4; ++j) {
			result.m[i][j] = 0.0f;
			for (int32_t k = 0; k < 4; ++k) {
				result.m[i][j] += m1.m[i][k] * m2.m[k][j];
			}
		}
	}
	return result;
}

Matrix4x4 MyMath::MakeTranslateMatrix(const Vector3& translate) {
	Matrix4x4 result{};
	for (int32_t i = 0; i < 4; i++) {
		for (int32_t j = 0; j < 4; j++) {
			result.m[i][j] = (i == j) ? 1.0f : 0.0f;
		}
	}
	result.m[3][0] = translate.x;
	result.m[3][1] = translate.y;
	result.m[3][2] = translate.z;
	return result;
}

Matrix4x4 MyMath::MakeScaleMatrix(const Vector3& scale) {
	Matrix4x4 result{};
	for (int32_t i = 0; i < 4; i++) {
		for (int32_t j = 0; j < 4; j++) {
			result.m[i][j] = (i == j) ? 1.0f : 0.0f;
		}
	}
	result.m[0][0] = scale.x;
	result.m[1][1] = scale.y;
	result.m[2][2] = scale.z;
	return result;
}

Matrix4x4 MyMath::MakeRotateXMatrix(float radius) {
	Matrix4x4 result{};
	for (int32_t i = 0; i < 4; i++) {
		for (int32_t j = 0; j < 4; j++) {
			result.m[i][j] = (i == j) ? 1.0f : 0.0f;
		}
	}
	result.m[1][1] = float(std::cos(radius));
	result.m[1][2] = float(std::sin(radius));
	result.m[2][1] = float(-std::sin(radius));
	result.m[2][2] = float(std::cos(radius));
	return result;
}

Matrix4x4 MyMath::MakeRotateYMatrix(float radius) {
	Matrix4x4 result{};
	for (int32_t i = 0; i < 4; i++) {
		for (int32_t j = 0; j < 4; j++) {
			result.m[i][j] = (i == j) ? 1.0f : 0.0f;
		}
	}
	result.m[0][0] = float(std::cos(radius));
	result.m[0][2] = float(-std::sin(radius));
	result.m[2][0] = float(std::sin(radius));
	result.m[2][2] = float(std::cos(radius));
	return result;
}

Matrix4x4 MyMath::MakeRotateZMatrix(float radius) {
	Matrix4x4 result{};
	for (int32_t i = 0; i < 4; i++) {
		for (int32_t j = 0; j < 4; j++) {
			result.m[i][j] = (i == j) ? 1.0f : 0.0f;
		}
	}
	result.m[0][0] = float(std::cos(radius));
	result.m[0][1] = float(std::sin(radius));
	result.m[1][0] = float(-std::sin(radius));
	result.m[1][1] = float(std::cos(radius));
	return result;
}

Matrix4x4 MyMath::MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate) {
	Matrix4x4 translateMatrix = MakeTranslateMatrix(translate);
	Matrix4x4 scaleMatrix = MakeScaleMatrix(scale);

	Matrix4x4 rotateMatrix = Multiply(MakeRotateXMatrix(rotate.x), Multiply(MakeRotateYMatrix(rotate.y), MakeRotateZMatrix(rotate.z)));

	Matrix4x4 sr = Multiply(scaleMatrix, rotateMatrix);
	Matrix4x4 srt = Multiply(sr, translateMatrix);
	return srt;
}

Matrix4x4 MyMath::MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip) {
	Matrix4x4 result{ 0 };

	float cot = 1.0f / std::tan(fovY / 2.0f);
	result.m[0][0] = (1.0f / aspectRatio) * cot;
	result.m[1][1] = cot;
	result.m[2][2] = farClip / (farClip - nearClip);
	result.m[2][3] = 1.0f;
	result.m[3][2] = (-nearClip * farClip) / (farClip - nearClip);
	return result;
}

Matrix4x4 MyMath::MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip) {
	Matrix4x4 result{ 0 };
	result.m[0][0] = 2.0f / (right - left);
	result.m[1][1] = 2.0f / (top - bottom);
	result.m[2][2] = 1.0f / (farClip - nearClip);
	result.m[3][0] = (left + right) / (left - right);
	result.m[3][1] = (top + bottom) / (bottom - top);
	result.m[3][2] = nearClip / (nearClip - farClip);
	result.m[3][3] = 1.0f;
	return result;
}

Matrix4x4 MyMath::MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth) {
	Matrix4x4 result{ 0 };
	result.m[0][0] = width / 2.0f;
	result.m[1][1] = -height / 2.0f;
	result.m[2][2] = maxDepth - minDepth;
	result.m[3][0] = left + width / 2.0f;
	result.m[3][1] = top + height / 2.0f;
	result.m[3][2] = minDepth;
	result.m[3][3] = 1.0f;
	return result;
}

Matrix4x4 MyMath::Inverse(const Matrix4x4& m) {
	float determinant = +m.m[0][0] * m.m[1][1] * m.m[2][2] * m.m[3][3] + m.m[0][0] * m.m[1][2] * m.m[2][3] * m.m[3][1] + m.m[0][0] * m.m[1][3] * m.m[2][1] * m.m[3][2] -
		m.m[0][0] * m.m[1][3] * m.m[2][2] * m.m[3][1] - m.m[0][0] * m.m[1][2] * m.m[2][1] * m.m[3][3] - m.m[0][0] * m.m[1][1] * m.m[2][3] * m.m[3][2]

		- m.m[0][1] * m.m[1][0] * m.m[2][2] * m.m[3][3] - m.m[0][2] * m.m[1][0] * m.m[2][3] * m.m[3][1] - m.m[0][3] * m.m[1][0] * m.m[2][1] * m.m[3][2]

		+ m.m[0][3] * m.m[1][0] * m.m[2][2] * m.m[3][1] + m.m[0][2] * m.m[1][0] * m.m[2][1] * m.m[3][3] + m.m[0][1] * m.m[1][0] * m.m[2][3] * m.m[3][2]

		+ m.m[0][1] * m.m[1][2] * m.m[2][0] * m.m[3][3] + m.m[0][2] * m.m[1][3] * m.m[2][0] * m.m[3][1] + m.m[0][3] * m.m[1][1] * m.m[2][0] * m.m[3][2]

		- m.m[0][3] * m.m[1][2] * m.m[2][0] * m.m[3][1] - m.m[0][2] * m.m[1][1] * m.m[2][0] * m.m[3][3] - m.m[0][1] * m.m[1][3] * m.m[2][0] * m.m[3][2]

		- m.m[0][1] * m.m[1][2] * m.m[2][3] * m.m[3][0] - m.m[0][2] * m.m[1][3] * m.m[2][1] * m.m[3][0] - m.m[0][3] * m.m[1][1] * m.m[2][2] * m.m[3][0]

		+ m.m[0][3] * m.m[1][2] * m.m[2][1] * m.m[3][0] + m.m[0][2] * m.m[1][1] * m.m[2][3] * m.m[3][0] + m.m[0][1] * m.m[1][3] * m.m[2][2] * m.m[3][0];

	Matrix4x4 result = {};
	float recpDeterminant = 1.0f / determinant;
	result.m[0][0] = (m.m[1][1] * m.m[2][2] * m.m[3][3] + m.m[1][2] * m.m[2][3] * m.m[3][1] + m.m[1][3] * m.m[2][1] * m.m[3][2] - m.m[1][3] * m.m[2][2] * m.m[3][1] -
		m.m[1][2] * m.m[2][1] * m.m[3][3] - m.m[1][1] * m.m[2][3] * m.m[3][2]) *
		recpDeterminant;
	result.m[0][1] = (-m.m[0][1] * m.m[2][2] * m.m[3][3] - m.m[0][2] * m.m[2][3] * m.m[3][1] - m.m[0][3] * m.m[2][1] * m.m[3][2] + m.m[0][3] * m.m[2][2] * m.m[3][1] +
		m.m[0][2] * m.m[2][1] * m.m[3][3] + m.m[0][1] * m.m[2][3] * m.m[3][2]) *
		recpDeterminant;
	result.m[0][2] = (m.m[0][1] * m.m[1][2] * m.m[3][3] + m.m[0][2] * m.m[1][3] * m.m[3][1] + m.m[0][3] * m.m[1][1] * m.m[3][2] - m.m[0][3] * m.m[1][2] * m.m[3][1] -
		m.m[0][2] * m.m[1][1] * m.m[3][3] - m.m[0][1] * m.m[1][3] * m.m[3][2]) *
		recpDeterminant;
	result.m[0][3] = (-m.m[0][1] * m.m[1][2] * m.m[2][3] - m.m[0][2] * m.m[1][3] * m.m[2][1] - m.m[0][3] * m.m[1][1] * m.m[2][2] + m.m[0][3] * m.m[1][2] * m.m[2][1] +
		m.m[0][2] * m.m[1][1] * m.m[2][3] + m.m[0][1] * m.m[1][3] * m.m[2][2]) *
		recpDeterminant;

	result.m[1][0] = (-m.m[1][0] * m.m[2][2] * m.m[3][3] - m.m[1][2] * m.m[2][3] * m.m[3][0] - m.m[1][3] * m.m[2][0] * m.m[3][2] + m.m[1][3] * m.m[2][2] * m.m[3][0] +
		m.m[1][2] * m.m[2][0] * m.m[3][3] + m.m[1][0] * m.m[2][3] * m.m[3][2]) *
		recpDeterminant;
	result.m[1][1] = (m.m[0][0] * m.m[2][2] * m.m[3][3] + m.m[0][2] * m.m[2][3] * m.m[3][0] + m.m[0][3] * m.m[2][0] * m.m[3][2] - m.m[0][3] * m.m[2][2] * m.m[3][0] -
		m.m[0][2] * m.m[2][0] * m.m[3][3] - m.m[0][0] * m.m[2][3] * m.m[3][2]) *
		recpDeterminant;
	result.m[1][2] = (-m.m[0][0] * m.m[1][2] * m.m[3][3] - m.m[0][2] * m.m[1][3] * m.m[3][0] - m.m[0][3] * m.m[1][0] * m.m[3][2] + m.m[0][3] * m.m[1][2] * m.m[3][0] +
		m.m[0][2] * m.m[1][0] * m.m[3][3] + m.m[0][0] * m.m[1][3] * m.m[3][2]) *
		recpDeterminant;
	result.m[1][3] = (m.m[0][0] * m.m[1][2] * m.m[2][3] + m.m[0][2] * m.m[1][3] * m.m[2][0] + m.m[0][3] * m.m[1][0] * m.m[2][2] - m.m[0][3] * m.m[1][2] * m.m[2][0] -
		m.m[0][2] * m.m[1][0] * m.m[2][3] - m.m[0][0] * m.m[1][3] * m.m[2][2]) *
		recpDeterminant;

	result.m[2][0] = (m.m[1][0] * m.m[2][1] * m.m[3][3] + m.m[1][1] * m.m[2][3] * m.m[3][0] + m.m[1][3] * m.m[2][0] * m.m[3][1] - m.m[1][3] * m.m[2][1] * m.m[3][0] -
		m.m[1][1] * m.m[2][0] * m.m[3][3] - m.m[1][0] * m.m[2][3] * m.m[3][1]) *
		recpDeterminant;
	result.m[2][1] = (-m.m[0][0] * m.m[2][1] * m.m[3][3] - m.m[0][1] * m.m[2][3] * m.m[3][0] - m.m[0][3] * m.m[2][0] * m.m[3][1] + m.m[0][3] * m.m[2][1] * m.m[3][0] +
		m.m[0][1] * m.m[2][0] * m.m[3][3] + m.m[0][0] * m.m[2][3] * m.m[3][1]) *
		recpDeterminant;
	result.m[2][2] = (m.m[0][0] * m.m[1][1] * m.m[3][3] + m.m[0][1] * m.m[1][3] * m.m[3][0] + m.m[0][3] * m.m[1][0] * m.m[3][1] - m.m[0][3] * m.m[1][1] * m.m[3][0] -
		m.m[0][1] * m.m[1][0] * m.m[3][3] - m.m[0][0] * m.m[1][3] * m.m[3][1]) *
		recpDeterminant;
	result.m[2][3] = (-m.m[0][0] * m.m[1][1] * m.m[2][3] - m.m[0][1] * m.m[1][3] * m.m[2][0] - m.m[0][3] * m.m[1][0] * m.m[2][1] + m.m[0][3] * m.m[1][1] * m.m[2][0] +
		m.m[0][1] * m.m[1][0] * m.m[2][3] + m.m[0][0] * m.m[1][3] * m.m[2][1]) *
		recpDeterminant;

	result.m[3][0] = (-m.m[1][0] * m.m[2][1] * m.m[3][2] - m.m[1][1] * m.m[2][2] * m.m[3][0] - m.m[1][2] * m.m[2][0] * m.m[3][1] + m.m[1][2] * m.m[2][1] * m.m[3][0] +
		m.m[1][1] * m.m[2][0] * m.m[3][2] + m.m[1][0] * m.m[2][2] * m.m[3][1]) *
		recpDeterminant;
	result.m[3][1] = (m.m[0][0] * m.m[2][1] * m.m[3][2] + m.m[0][1] * m.m[2][2] * m.m[3][0] + m.m[0][2] * m.m[2][0] * m.m[3][1] - m.m[0][2] * m.m[2][1] * m.m[3][0] -
		m.m[0][1] * m.m[2][0] * m.m[3][2] - m.m[0][0] * m.m[2][2] * m.m[3][1]) *
		recpDeterminant;
	result.m[3][2] = (-m.m[0][0] * m.m[1][1] * m.m[3][2] - m.m[0][1] * m.m[1][2] * m.m[3][0] - m.m[0][2] * m.m[1][0] * m.m[3][1] + m.m[0][2] * m.m[1][1] * m.m[3][0] +
		m.m[0][1] * m.m[1][0] * m.m[3][2] + m.m[0][0] * m.m[1][2] * m.m[3][1]) *
		recpDeterminant;
	result.m[3][3] = (m.m[0][0] * m.m[1][1] * m.m[2][2] + m.m[0][1] * m.m[1][2] * m.m[2][0] + m.m[0][2] * m.m[1][0] * m.m[2][1] - m.m[0][2] * m.m[1][1] * m.m[2][0] -
		m.m[0][1] * m.m[1][0] * m.m[2][2] - m.m[0][0] * m.m[1][2] * m.m[2][1]) *
		recpDeterminant;

	return result;
}

Vector3 MyMath::Transform(const Vector3& vector, const Matrix4x4& matrix) {
	Vector3 result{};
	result.x = matrix.m[0][0] * vector.x + matrix.m[1][0] * vector.y + matrix.m[2][0] * vector.z + matrix.m[3][0];
	result.y = matrix.m[0][1] * vector.x + matrix.m[1][1] * vector.y + matrix.m[2][1] * vector.z + matrix.m[3][1];
	result.z = matrix.m[0][2] * vector.x + matrix.m[1][2] * vector.y + matrix.m[2][2] * vector.z + matrix.m[3][2];
	float w = matrix.m[0][3] * vector.x + matrix.m[1][3] * vector.y + matrix.m[2][3] * vector.z + matrix.m[3][3];
	if (w != 0.0f) {
		result.x /= w;
		result.y /= w;
		result.z /= w;
	}
	return result;
}

Vector3 MyMath::TransformNormal(const Vector3& vector, const Matrix4x4& matrix) {
	Vector3 result{};
	result.x = matrix.m[0][0] * vector.x + matrix.m[1][0] * vector.y + matrix.m[2][0] * vector.z;
	result.y = matrix.m[0][1] * vector.x + matrix.m[1][1] * vector.y + matrix.m[2][1] * vector.z;
	result.z = matrix.m[0][2] * vector.x + matrix.m[1][2] * vector.y + matrix.m[2][2] * vector.z;
	return result;
}

Vector3 MyMath::CalcLaunchDirection(const Vector3& dragStart, const Vector3& dragEnd) {
	Vector3 direction = Subtract(dragStart, dragEnd);
	direction.y = 0.0f;
	return Normalize(direction);
}

float MyMath::CalcLaunchSpeed(float dragDistance, float maxDragDistance, float minSpeed, float maxSpeed) {
	if (maxDragDistance <= 0.0f) {
		return minSpeed;
	}

	const float rate = Clamp(dragDistance / maxDragDistance, 0.0f, 1.0f);
	return Lerp(minSpeed, maxSpeed, rate);
}

void MyMath::IntegrateXZ(Vector3& position, const Vector3& velocity, float deltaTime) {
	position.x += velocity.x * deltaTime;
	position.z += velocity.z * deltaTime;
}

void MyMath::ApplyFriction(Vector3& velocity, float friction) { velocity = Multiply(velocity, friction); }

void MyMath::ApplyFrictionXZ(Vector3& velocity, float friction) {
	velocity.x *= friction;
	velocity.z *= friction;
}

void MyMath::ClampVelocity(Vector3& velocity, float maxSpeed) {
	if (maxSpeed < 0.0f) {
		maxSpeed = 0.0f;
	}

	const float speedSq = LengthSq(velocity);
	const float maxSpeedSq = maxSpeed * maxSpeed;
	if (speedSq > maxSpeedSq) {
		velocity = Multiply(Normalize(velocity), maxSpeed);
	}
}

bool MyMath::IsStopped(const Vector3& velocity, float stopSpeed) { return LengthSq(velocity) <= stopSpeed * stopSpeed; }

void MyMath::Accelerate(Vector3& velocity, const Vector3& direction, float acceleration, float deltaTime) {
	velocity = Add(velocity, Multiply(Normalize(direction), acceleration * deltaTime));
}

void MyMath::Decelerate(Vector3& velocity, float deceleration, float deltaTime) {
	const float speed = Length(velocity);
	if (speed == 0.0f) {
		return;
	}

	const float nextSpeed = Clamp(speed - deceleration * deltaTime, 0.0f, speed);
	velocity = Multiply(Normalize(velocity), nextSpeed);
}

Vector3 MyMath::CalcGravityForce(float mass, float gravityAcceleration) {
	return {0.0f, -mass * gravityAcceleration, 0.0f};
}

Vector3 MyMath::ReflectSlashXZ(const Vector3& velocity) { return {-velocity.z, 0.0f, -velocity.x}; }

Vector3 MyMath::ReflectBackSlashXZ(const Vector3& velocity) { return {velocity.z, 0.0f, velocity.x}; }

void MyMath::ReflectGridBounceXZ(Vector3& velocity, bool hitX, bool hitZ) {
	if (hitX) {
		velocity.x *= -1.0f;
	}
	if (hitZ) {
		velocity.z *= -1.0f;
	}
}
