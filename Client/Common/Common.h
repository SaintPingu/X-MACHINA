#pragma once

#pragma region Include
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>

#include "Singleton.h"
#include "SimpleMath.h"
#pragma endregion



#pragma region Define
#define _MATRIX(x)	XMLoadFloat4x4(&x)
#define _VECTOR4(x)	XMLoadFloat4(&x)
#define _VECTOR(x)	XMLoadFloat3(&x)
#define _VECTOR2(x)	XMLoadFloat2(&x)

#define TO_STRING( x ) #x				// ex) TO_STRING(myVar) ==> "myVar"
#pragma endregion



#pragma region EnumClass
enum class Dir : WORD {
	Front = 0x01,
	Back = 0x02,
	Left = 0x04,
	Right = 0x08,
	Up = 0x10,
	Down = 0x20
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
using Quaternion = DirectX::SimpleMath::Quaternion;

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

#pragma region EnumOperator_DWORD
// 아래 정의된 EnumType의 DWORD 연산을 오버로딩하여 불필요한 static_cast를 줄인다.
// is_valid_dword_type_v에 EnumType을 추가해주어야 한다.
// Engine
enum class ObjectTag : DWORD;
enum class VertexType : DWORD;
enum class MaterialMap : DWORD;
enum class HumanBone : DWORD;
enum class FilterOption : DWORD;
// Client
enum class Movement : DWORD;

template <typename T>
constexpr bool is_valid_dword_type_v = (std::is_same_v<T, Dir> || std::is_same_v<T, ObjectTag> || std::is_same_v<T, VertexType> || std::is_same_v<T, MaterialMap> || std::is_same_v<T, HumanBone> || std::is_same_v<T, FilterOption> || std::is_same_v<T, Movement>);

template <typename EnumType, typename = std::enable_if_t<is_valid_dword_type_v<EnumType>>>
constexpr DWORD operator|(EnumType lhs, EnumType rhs)
{
	return static_cast<DWORD>(lhs) | static_cast<DWORD>(rhs);
}

template <typename EnumType, typename = std::enable_if_t<is_valid_dword_type_v<EnumType>>>
constexpr DWORD operator|(DWORD lhs, EnumType rhs)
{
	return lhs | static_cast<DWORD>(rhs);
}

template <typename EnumType, typename = std::enable_if_t<is_valid_dword_type_v<EnumType>>>
constexpr DWORD operator|(EnumType lhs, DWORD rhs)
{
	return static_cast<DWORD>(lhs) | rhs;
}

template <typename EnumType, typename = std::enable_if_t<is_valid_dword_type_v<EnumType>>>
constexpr DWORD operator|=(DWORD& lhs, EnumType rhs)
{
	return lhs = lhs | static_cast<DWORD>(rhs);
}

template <typename EnumType, typename = std::enable_if_t<is_valid_dword_type_v<EnumType>>>
constexpr EnumType operator|=(EnumType& lhs, EnumType rhs)
{
	return lhs = static_cast<EnumType>(static_cast<DWORD>(lhs) | static_cast<DWORD>(rhs));
}

template <typename EnumType, typename = std::enable_if_t<is_valid_dword_type_v<EnumType>>>
constexpr DWORD operator&(EnumType lhs, EnumType rhs)
{
	return static_cast<DWORD>(lhs) & static_cast<DWORD>(rhs);
}

template <typename EnumType, typename = std::enable_if_t<is_valid_dword_type_v<EnumType>>>
constexpr DWORD operator&(DWORD lhs, EnumType rhs)
{
	return lhs & static_cast<DWORD>(rhs);
}

template <typename EnumType, typename = std::enable_if_t<is_valid_dword_type_v<EnumType>>>
constexpr DWORD operator&(EnumType lhs, DWORD rhs)
{
	return static_cast<DWORD>(lhs) & rhs;
}

template <typename EnumType, typename = std::enable_if_t<is_valid_dword_type_v<EnumType>>>
constexpr DWORD operator&=(DWORD& lhs, EnumType rhs)
{
	return lhs = lhs & static_cast<DWORD>(rhs);
}

template <typename EnumType, typename = std::enable_if_t<is_valid_dword_type_v<EnumType>>>
constexpr DWORD operator~(EnumType value)
{
	return ~static_cast<DWORD>(value);
}
#pragma endregion

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
namespace Vector3 {
	inline bool IsZero(const Vec3& vector) noexcept
	{
		return XMVector3Equal(_VECTOR(vector), XMVectorZero());
	}

	inline Vec3 Normalized(const Vec3& vector) noexcept
	{
		if (Vector3::IsZero(vector)) {
			return Vec3::Zero;
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

	// (0, 0, 0)
	inline Vec3 Zero()
	{
		return Vec3(0.f, 0.f, 0.f);
	}

	// (0, 1, 0)
	inline Vec3 Up()
	{
		return Vec3(0.f, 1.f, 0.f);
	}

	// (0, -1, 0)
	inline Vec3 Down()
	{
		return Vec3(0.f, -1.f, 0.f);
	}

	// (1, 0, 0)
	inline Vec3 Right()
	{
		return Vec3(1.f, 0.f, 0.f);
	}

	// (-1, 0, 0)
	inline Vec3 Left()
	{
		return Vec3(-1.f, 0.f, 0.f);
	}

	// (0, 0, 1)
	inline Vec3 Forward()
	{
		return Vec3(0.f, 0.f, 1.f);
	}

	// (0, 0, -1)
	inline Vec3 Back()
	{
		return Vec3(0.f, 0.f, -1.f);
	}

	// (1, 1, 1)
	inline Vec3 One()
	{
		return Vec3(1.f, 1.f, 1.f);
	}

	// (1, 1, 1)
	inline Vec3 RUF()
	{
		return Vec3(1.f, 1.f, 1.f);
	}

	// (1, 1, -1)
	inline Vec3 RUB()
	{
		return Vec3(1.f, 1.f, -1.f);
	}

	// (1, -1, 1)
	inline Vec3 RDF()
	{
		return Vec3(1.f, -1.f, 1.f);
	}

	// (1, -1, -1)
	inline Vec3 RDB()
	{
		return Vec3(1.f, -1.f, -1.f);
	}

	// (-1, 1, 1)
	inline Vec3 LUF()
	{
		return Vec3(-1.f, 1.f, 1.f);
	}

	// (-1, 1, -1)
	inline Vec3 LUB()
	{
		return Vec3(-1.f, 1.f, -1.f);
	}

	// (-1, -1, 1)
	inline Vec3 LDF()
	{
		return Vec3(-1.f, -1.f, 1.f);
	}

	// (-1, -1, -1)
	inline Vec3 LDB()
	{
		return Vec3(-1.f, -1.f, -1.f);
	}
}

namespace Matrix4x4 {
	inline Matrix SetRotation(const Matrix& matrix, const Vec4& quaternion) noexcept
	{
		XMVECTOR quat = _VECTOR4(quaternion);

		XMVECTOR scale, rotation, translation;
		XMMatrixDecompose(&scale, &rotation, &translation, matrix);

		// 회전값을 재적용한 행렬 재구성
		Matrix result;
		XMStoreFloat4x4(&result, XMMatrixScalingFromVector(scale) * XMMatrixRotationQuaternion(quat) * XMMatrixTranslationFromVector(translation));
		return result;
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

	inline Matrix Zero()
	{
		Matrix result;
		XMStoreFloat4x4(&result, XMMatrixSet(0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f));
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
		const XMVECTOR kRotation = XMQuaternionRotationMatrix(_MATRIX(transform));

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