#pragma once

#pragma region Include
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>

#include "Singleton.h"
#include "SimpleMath.h"
#include "DwordOverloader.h"
#pragma endregion



#pragma region Define
#define _MATRIX(x)	XMLoadFloat4x4(&x)
#define _VECTOR4(x)	XMLoadFloat4(&x)
#define _VECTOR(x)	XMLoadFloat3(&x)
#define _VECTOR2(x)	XMLoadFloat2(&x)

#define TO_STRING( x ) #x				// ex) TO_STRING(myVar) ==> "myVar"
#pragma endregion



#pragma region EnumClass
class Dir : public DwordOverloader<Dir> {
	DWORD_OVERLOADER(Dir)

	static const DWORD None  = 0x00;
	static const DWORD Front = 0x01;
	static const DWORD Back  = 0x02;
	static const DWORD Left  = 0x04;
	static const DWORD Right = 0x08;
	static const DWORD Up    = 0x10;
	static const DWORD Down  = 0x20;
};

enum class BlendType : UINT8 {
	Default,
	Alpha_Blend,
	One_To_One_Blend,
};
#pragma endregion



#pragma region Using
/* DirectX */
using namespace DirectX;
using namespace DirectX::PackedVector;
using Microsoft::WRL::ComPtr;

/* DirectX Math */
using Vec2       = DirectX::SimpleMath::Vector2;
using Vec3       = DirectX::SimpleMath::Vector3;
using Vec4       = DirectX::SimpleMath::Vector4;
using Matrix     = DirectX::SimpleMath::Matrix;
using Quat	     = DirectX::SimpleMath::Quaternion;

/* Abbreviation */
template<class T>
using RComPtr = const ComPtr<T>&;
template<class T>
using sptr = std::shared_ptr<T>;
template<class T>
using rsptr = const sptr<T>&;
template<class T, class Deleter = std::default_delete<T>>
using uptr = std::unique_ptr<T, Deleter>;
template<class T>
using ruptr = const uptr<T>&;
template<class T>
using wptr = std::weak_ptr<T>;
template<class T>
using rwptr = const wptr<T>&;
#pragma endregion



#pragma region Function
/* string to hash code. can use string in switch statement. */
constexpr unsigned int Hash(const char* str) noexcept
{
	return str[0] ? static_cast<unsigned int>(str[0]) + 0xEDB8832Full * Hash(str + 1) : 8603;
}
constexpr unsigned int Hash(const wchar_t* str) noexcept
{
	return str[0] ? static_cast<unsigned int>(str[0]) + 0xEDB8832Full * Hash(str + 1) : 8603;
}
constexpr unsigned int Hash(const std::string& str) noexcept
{
	return Hash(str.data());
}
constexpr unsigned int Hash(const std::wstring& str) noexcept
{
	return Hash(str.data());
}
#pragma endregion


#pragma region Namespace
namespace Math {
	constexpr float kPI = 3.141592f;
	constexpr float kGravity = 9.8f;

	static std::default_random_engine dre;

	inline bool IsZero(float f) { return fabsf(f) < FLT_EPSILON; }
	inline bool IsEqual(float a, float b) { return Math::IsZero(a - b); }
	inline float InverseSqrt(float f) { return 1.f / sqrtf(f); }

	inline float RandF(float min = 0.f, float max = 1.f)
	{
		return min + ((float)rand() / (float)RAND_MAX) * (max - min);
	}

	constexpr float uint16ToFloat(std::uint16_t value)
	{
		return static_cast<float>(value) / 65535.0f * 255.0f;
	}

	constexpr int Sign(float x)
	{
		return (x >= FLT_EPSILON) ? 1 : -1;
	}

	// (value)를 가장 가까운 (multiple)의 배수로 반환한다.
	// ex) GetNearestMultiple(17, 5) -> 15
	constexpr int GetNearestMultiple(int value, int multiple)
	{
		return ((value + multiple / 2) / multiple) * multiple;
	}
}

#pragma region DirectXMath
namespace Vector2 {

	static const Vec2 Zero  = { +0.f, +0.f };
	static const Vec2 One   = { +1.f, +1.f };
	static const Vec2 Right = { +1.f, +0.f };
	static const Vec2 Left  = { -1.f, +0.f };
	static const Vec2 Up    = { +0.f, +1.f };
	static const Vec2 Down  = { +0.f, -1.f };

	inline bool IsZero(const Vec2& vector) noexcept
	{
		return XMVector2Equal(_VECTOR2(vector), XMVectorZero());
	}

	inline Vec2 Normalized(const Vec2& vector) noexcept
	{
		if (Vector2::IsZero(vector)) {
			return Vector2::Zero;
		}

		Vec2 result;
		XMStoreFloat2(&result, XMVector2NormalizeEst(_VECTOR2(vector)));
		return result;
	}

	inline float SignedAngle(const Vec2& from, const Vec2& to)
	{
		return XMConvertToDegrees(atan2(to.y, to.x) - atan2(from.y, from.x));
		XMVECTOR v1 = XMVector2NormalizeEst(_VECTOR2(from));
		XMVECTOR v2 = XMVector2NormalizeEst(_VECTOR2(to));

		float angle = XMVectorGetX(XMVector2AngleBetweenNormals(v1, v2));
		float det = XMVectorGetX(XMVector2Cross(v1, v2));
		if (det < 0) {
			angle = -angle;
		}

		return XMConvertToDegrees(angle);
	}
}
namespace Vector3 {

	static const Vec3 Zero  = { +0.f, +0.f, +0.f };
	static const Vec3 Up    = { +0.f, +1.f, +0.f };
	static const Vec3 Down  = { +0.f, -1.f, +0.f };
	static const Vec3 Right = { +1.f, +0.f, +0.f };
	static const Vec3 Left  = { -1.f, +0.f, +0.f };
	static const Vec3 Front	= { +0.f, +0.f, +1.f };
	static const Vec3 Back  = { +0.f, +0.f, -1.f };
	static const Vec3 One   = { +1.f, +1.f, +1.f };

	static const Vec3 RF  = { +1.f, +0.f, +1.f };
	static const Vec3 RB  = { +1.f, +0.f, -1.f };
	static const Vec3 RUF = { +1.f, +1.f, +1.f };
	static const Vec3 RUB = { +1.f, +1.f, -1.f };
	static const Vec3 RDF = { +1.f, -1.f, +1.f };
	static const Vec3 RDB = { +1.f, -1.f, -1.f };

	static const Vec3 LF  = { -1.f, +0.f, +1.f };
	static const Vec3 LB  = { -1.f, +0.f, -1.f };
	static const Vec3 LUF = { -1.f, +1.f, +1.f };
	static const Vec3 LUB = { -1.f, +1.f, -1.f };
	static const Vec3 LDF = { -1.f, -1.f, +1.f };
	static const Vec3 LDB = { -1.f, -1.f, -12.f };

	inline bool IsZero(const Vec3& vector) noexcept
	{
		return XMVector3Equal(_VECTOR(vector), XMVectorZero());
	}

	inline Vec3 Normalized(const Vec3& vector) noexcept
	{
		if (Vector3::IsZero(vector)) {
			return Vector3::Zero;
		}

		Vec3 result;
		XMStoreFloat3(&result, XMVector3NormalizeEst(_VECTOR(vector)));
		return result;
	}

	inline Vec3 Rotate(const Vec3& v, const Vec3& eulerAngles) noexcept
	{
		Vec3 result;
		XMStoreFloat3(&result, XMVector3Rotate(_VECTOR(v), XMQuaternionRotationRollPitchYawFromVector(_VECTOR(eulerAngles))));
		return result;
	}

	inline Vec3 Rotate(const Vec3& v, const Vec4& quat) noexcept
	{
		Vec3 result;
		XMStoreFloat3(&result, XMVector3Rotate(_VECTOR(v), _VECTOR4(quat)));
		return result;
	}

	inline Vec3 Resize(const Vec3& vector, float size) noexcept
	{
		Vec3 result;
		XMStoreFloat3(&result, _VECTOR(Vector3::Normalized(vector)) * size);
		return result;
	}

	inline float Angle(const Vec3& from, const Vec3& to)
	{
		float num = sqrtf(from.LengthSquared() * to.LengthSquared());
		if (num < FLT_EPSILON) {
			return 0.f;
		}

		float num2 = std::clamp(from.Dot(to) / num, -1.f, 1.f);
		return acosf(num2) * 57.29578f;
	}

	inline float SignedAngle(const Vec3& from, const Vec3& to, const Vec3& axis)
	{
		XMVECTOR v1 = XMVector3NormalizeEst(_VECTOR(from));
		XMVECTOR v2 = XMVector3NormalizeEst(_VECTOR(to));

		float angle    = XMVectorGetX(XMVector3AngleBetweenNormals(v1, v2));
		XMVECTOR cross = XMVector3Cross(v1, v2);
		float dot      = XMVectorGetX(XMVector3Dot(cross, _VECTOR(axis)));
		if (dot < 0) {
			angle = -angle;
		}

		return XMConvertToDegrees(angle);
	}

	inline int Sign(const Vec3& from, const Vec3& to, const Vec3& axis)
	{
		XMVECTOR v1 = XMVector3NormalizeEst(_VECTOR(from));
		XMVECTOR v2 = XMVector3NormalizeEst(_VECTOR(to));

		XMVECTOR cross = XMVector3Cross(v1, v2);
		float dot = XMVectorGetX(XMVector3Dot(cross, _VECTOR(axis)));
		int sign = (dot >= 0.0f) ? 1 : -1;

		return sign;
	}
}

namespace Matrix4x4 {
	inline void SetRotation(Matrix& matrix, const Quat& quaternion) noexcept
	{
		XMVECTOR quat = _VECTOR4(quaternion);

		XMVECTOR scale, rotation, translation;
		XMMatrixDecompose(&scale, &rotation, &translation, matrix);

		// 회전값을 재적용한 행렬 재구성
		XMStoreFloat4x4(&matrix, XMMatrixScalingFromVector(scale) * XMMatrixRotationQuaternion(quat) * XMMatrixTranslationFromVector(translation));
	}

	inline Matrix OrthographicOffCenterLH(float fFovAngleY, float aspectRatio, float fNearZ, float fFarZ) noexcept
	{
		Matrix result;
		XMStoreFloat4x4(&result, XMMatrixOrthographicLH(XMConvertToRadians(fFovAngleY), aspectRatio, fNearZ, fFarZ));
		return result;
	}

	inline Matrix PerspectiveFovLH(float fFovAngleY, float aspectRatio, float fNearZ, float fFarZ) noexcept
	{
		Matrix result;
		XMStoreFloat4x4(&result, XMMatrixPerspectiveFovLH(XMConvertToRadians(fFovAngleY), aspectRatio, fNearZ, fFarZ));
		return result;
	}

	inline Matrix LookAtLH(const Vec3& eyePos, const Vec3& lookAtPos, const Vec3& upDir, bool isTranspose = false)
	{
		Matrix result;
		if (isTranspose) {
			XMStoreFloat4x4(&result, XMMatrixTranspose(XMMatrixLookAtLH(_VECTOR(eyePos), _VECTOR(lookAtPos), _VECTOR(upDir))));
		}
		else {
			XMStoreFloat4x4(&result, XMMatrixLookAtLH(_VECTOR(eyePos), _VECTOR(lookAtPos), _VECTOR(upDir)));
		}
		return result;
	}

	inline Matrix LookToLH(const Vec3& eyePos, const Vec3& lookTo, const Vec3& upDir, bool isTranspose = false)
	{
		Matrix result;
		if (isTranspose) {
			XMStoreFloat4x4(&result, XMMatrixTranspose(XMMatrixLookToLH(_VECTOR(eyePos), _VECTOR(lookTo), _VECTOR(upDir))));
		}
		else {
			XMStoreFloat4x4(&result, XMMatrixLookToLH(_VECTOR(eyePos), _VECTOR(lookTo), _VECTOR(upDir)));
		}
		return result;
	}


	inline Matrix Interpolate(const Matrix& matrix1, const Matrix& matrix2, float t)
	{
		Matrix result;

		XMVECTOR S0, R0, T0, S1, R1, T1;
		XMMatrixDecompose(&S0, &R0, &T0, _MATRIX(matrix1));
		XMMatrixDecompose(&S1, &R1, &T1, _MATRIX(matrix2));

		XMVECTOR S = XMVectorLerp(S0, S1, t);
		XMVECTOR T = XMVectorLerp(T0, T1, t);
		XMVECTOR R = XMQuaternionSlerp(R0, R1, t);
		XMStoreFloat4x4(&result, XMMatrixAffineTransformation(S, XMVectorZero(), R, T));

		return result;
	}

	inline Matrix CreateRotationMatrix(const Vec3& from, const Vec3& to)
	{
		// 벡터의 길이를 정규화합니다.
		XMVECTOR v1 = XMVector3NormalizeEst(_VECTOR(from));
		XMVECTOR v2 = XMVector3NormalizeEst(_VECTOR(to));

		// 내적을 통해 두 벡터 사이의 각도를 계산합니다.
		float dotProduct = XMVectorGetX(XMVector3Dot(v1, v2));
		float angle = XMConvertToDegrees(acosf(dotProduct));

		// 벡터의 외적을 계산합니다.
		XMVECTOR crossProduct = XMVector3Cross(v1, v2);

		// 회전 축을 결정합니다.
		XMVECTOR rotationAxis = XMVector3Normalize(crossProduct);

		// 회전 행렬을 생성합니다.
		Matrix result;
		XMStoreFloat4x4(&result, XMMatrixRotationAxis(rotationAxis, XMConvertToRadians(angle)));

		return result;
	}

	inline Matrix Zero()
	{
		Matrix result;
		XMStoreFloat4x4(&result, XMMatrixSet(0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f));
		return result;
	}
}

namespace Quaternion {
	inline Vec3 ToEuler(const Quat& q)
	{
		Vec3 result{};

		// Roll (x-axis rotation)
		float sinr_cosp = 2 * (q.w * q.x + q.y * q.z);
		float cosr_cosp = 1 - 2 * (q.x * q.x + q.y * q.y);
		result.x = XMConvertToDegrees(atan2f(sinr_cosp, cosr_cosp));

		// Pitch (y-axis rotation)
		float sinp = sqrtf(1 + 2 * (q.w * q.y - q.z * q.x));
		float cosp = sqrtf(1 - 2 * (q.w * q.y - q.z * q.x));
		result.y = XMConvertToDegrees(2 * atan2f(sinp, cosp) - XM_PI / 2);

		// Yaw (z-axis rotation)
		float siny_cosp = 2 * (q.w * q.z + q.x * q.y);
		float cosy_cosp = 1 - 2 * (q.y * q.y + q.z * q.z);
		result.z = XMConvertToDegrees(atan2f(siny_cosp, cosy_cosp));

		return result;
	}

	inline Quat ToQuaternion(const Vec3& euler)
	{
		XMVECTOR eulerRad = XMVectorScale(_VECTOR(euler), XM_PI / 180.0f);

		Quat result;
		XMStoreFloat4(&result, XMQuaternionRotationRollPitchYawFromVector(eulerRad));
		return result;
	}

	inline Quat LookRotation(const Vec3& forward, const Vec3& up)
	{
		XMVECTOR zAxis = XMVector3NormalizeEst(_VECTOR(forward));
		XMVECTOR xAxis = XMVector3NormalizeEst(XMVector3Cross(_VECTOR(up), zAxis));
		XMVECTOR yAxis = XMVector3Cross(zAxis, xAxis);

		XMMATRIX rotationMatrix(
			xAxis,
			yAxis,
			zAxis,
			g_XMIdentityR3
		);

		Quat result;
		XMStoreFloat4(&result, XMQuaternionRotationMatrix(rotationMatrix));
		return result;
	}

	inline Quat Inverse(const Quat& quaternion) noexcept
	{
		Quat result;
		XMStoreFloat4(&result, XMQuaternionInverse(_VECTOR4(quaternion)));
		return result;
	}
}
#pragma endregion

#pragma endregion



#pragma region Class
class MyBoundingOrientedBox : public BoundingOrientedBox {
private:
	Vec3 mOriginCenter{};	// 모델좌표계 Center

public:
	MyBoundingOrientedBox() = default;
	virtual ~MyBoundingOrientedBox() = default;

	Vec3 GetOrigin() const { return mOriginCenter; }

	void SetOrigin(const Vec3& origin) { mOriginCenter = origin; }

public:
	// no apply scale
	void Transform(const Matrix& transform)
	{
		const XMMATRIX kMatrix = _MATRIX(transform);
		const XMVECTOR kRotation = XMQuaternionRotationMatrix(kMatrix);

		XMStoreFloat4(&Orientation, kRotation);
		XMStoreFloat3(&Center, XMVector3Transform(_VECTOR(mOriginCenter), kMatrix));
	}

	operator const BoundingOrientedBox& () const {
		return static_cast<const BoundingOrientedBox&>(*this);
	}
};

class MyBoundingSphere : public BoundingSphere {
private:
	Vec3 mOriginCenter{};	// 모델좌표계 Center

public:
	MyBoundingSphere() = default;
	virtual ~MyBoundingSphere() = default;

	Vec3 GetOrigin() const { return mOriginCenter; }

	void SetOrigin(const Vec3& origin) { mOriginCenter = origin; }

public:
	void Transform(const Matrix& transform)
	{
		Center = Vec3::Transform(mOriginCenter, transform);
	}

	bool IntersectBoxes(const std::vector<MyBoundingOrientedBox*>& boxes) const
	{
		for (auto& box : boxes) {
			if (Intersects(*box)) {
				return true;
			}
		}

		return false;
	}

	operator const BoundingSphere& () const {
		return static_cast<const BoundingSphere&>(*this);
	}
};

#pragma endregion