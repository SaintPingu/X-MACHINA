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
using Vec2 = DirectX::SimpleMath::Vector2;
using Vec3 = DirectX::SimpleMath::Vector3;
using Vec4 = DirectX::SimpleMath::Vector4;
using Matrix = DirectX::SimpleMath::Matrix;
using Vec4x4 = XMFLOAT4X4;
using Vec4x3 = XMFLOAT4X3;
using Vector = XMVECTOR;

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
enum class ObjectTag : DWORD;
enum class VertexType : DWORD;
enum class MaterialMap : DWORD;
enum class HumanBone : DWORD;
enum class FilterOption : DWORD;

template <typename T>
constexpr bool is_valid_dword_type_v = (std::is_same_v<T, Dir> || std::is_same_v<T, ObjectTag> || std::is_same_v<T, VertexType> || std::is_same_v<T, MaterialMap> || std::is_same_v<T, HumanBone> || std::is_same_v<T, FilterOption>);

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
namespace Vector2 {
	inline Vec2 Add(const Vec2& v1, const Vec2& v2)
	{
		Vec2 result;
		XMStoreFloat2(&result, _VECTOR2(v1) + _VECTOR2(v2));
		return result;
	}

	inline float Length(const Vec2& vector)
	{
		Vec2 result;
		XMStoreFloat2(&result, XMVector2LengthEst(_VECTOR2(vector)));
		return result.x;
	}

	inline float Length(const Vec2& v1, const Vec2& v2)
	{
		return Vector2::Length(Vector2::Add(v1, v2));
	}

	inline float LengthExact(const Vec2& vector)
	{
		Vec2 result;
		XMStoreFloat2(&result, XMVector2Length(_VECTOR2(vector)));
		return result.x;
	}

	inline float LengthExact(const Vec2& v1, const Vec2& v2)
	{
		return Vector2::LengthExact(Vector2::Add(v1, v2));
	}

	inline Vec2 Normalize(const Vec2& vector) noexcept
	{
		Vec2 result;
		XMStoreFloat2(&result, XMVector2NormalizeEst(_VECTOR2(vector)));
		return result;
	}

	inline float AngleX(const Vec2& v1, const Vec2& v2) noexcept
	{
		Vector angle = XMVector2AngleBetweenNormalsEst(_VECTOR2(v1), _VECTOR2(v2));
		return XMConvertToDegrees(acosf(XMVectorGetX(angle)));
	}
}

namespace Vector3 {

	inline Vec3 ScalarProduct(const Vec3& vector, float scalar, bool normalize = true) noexcept
	{
		Vec3 result;
		if (normalize) {
			XMStoreFloat3(&result, XMVector3NormalizeEst(_VECTOR(vector)) * scalar);
		}
		else {
			XMStoreFloat3(&result, _VECTOR(vector) * scalar);
		}

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

	inline Vec3 Add(const Vec3& v1, const Vec3& v2) noexcept
	{
		Vec3 result;
		XMStoreFloat3(&result, _VECTOR(v1) + _VECTOR(v2));
		return result;
	}

	inline Vec3 Add(const Vec3& v1, const Vec3& v2, const Vec3& v3) noexcept
	{
		Vec3 result;
		XMStoreFloat3(&result, _VECTOR(v1) + _VECTOR(v2) + _VECTOR(v3));
		return result;
	}

	inline Vec3 Add(const Vec3& v1, const Vec3& v2, float scalar) noexcept
	{
		Vec3 result;
		XMStoreFloat3(&result, _VECTOR(v1) + (_VECTOR(v2) * scalar));
		return result;
	}

	inline Vec3 Subtract(const Vec3& v1, const Vec3& v2) noexcept
	{
		Vec3 result;
		XMStoreFloat3(&result, _VECTOR(v1) - _VECTOR(v2));
		return result;
	}

	inline float DotProduct(const Vec3& v1, const Vec3& v2) noexcept
	{
		Vec3 result;
		XMStoreFloat3(&result, XMVector3Dot(_VECTOR(v1), _VECTOR(v2)));
		return result.x;
	}

	inline Vec3 CrossProduct(const Vec3& v1, const Vec3& v2, bool normalize = true) noexcept
	{
		Vec3 result;
		Vector cross = XMVector3Cross(_VECTOR(v1), _VECTOR(v2));
		if (normalize) {
			cross = XMVector3NormalizeEst(cross);
		}

		XMStoreFloat3(&result, cross);

		return result;
	}

	inline Vec3 Normalize(const Vec3& vector) noexcept
	{
		Vec3 result;
		XMStoreFloat3(&result, XMVector3NormalizeEst(_VECTOR(vector)));
		return result;
	}


	inline float Length(const Vec3& vector) noexcept
	{
		Vec3 result;
		XMStoreFloat3(&result, XMVector3LengthEst(_VECTOR(vector)));
		return result.x;
	}

	inline float Length(const Vec3& v1, const Vec3& v2) noexcept
	{
		return Vector3::Length(Vector3::Add(v1, v2));
	}

	inline float AngleX(const Vector& v1, const Vector& v2) noexcept
	{
		Vector angle = XMVector3AngleBetweenNormalsEst(v1, v2);
		return XMConvertToDegrees(XMVectorGetX(angle));
	}

	inline float AngleX(const Vec3& v1, const Vec3& v2) noexcept
	{
		return AngleX(_VECTOR(v1), _VECTOR(v2));
	}

	inline float AngleY(const Vector& v1, const Vector& v2) noexcept
	{
		Vector angle = XMVector3AngleBetweenNormalsEst(v1, v2);
		return XMConvertToDegrees(acosf(XMVectorGetY(angle)));
	}

	inline float AngleY(const Vec3& v1, const Vec3& v2) noexcept
	{
		return AngleY(_VECTOR(v1), _VECTOR(v2));
	}

	inline float AngleZ(const Vector& v1, const Vector& v2) noexcept
	{
		Vector angle = XMVector3AngleBetweenNormalsEst(v1, v2);
		return XMConvertToDegrees(acosf(XMVectorGetZ(angle)));
	}

	inline float AngleZ(const Vec3& v1, const Vec3& v2) noexcept
	{
		return AngleZ(_VECTOR(v1), _VECTOR(v2));
	}

	inline Vec3 Angle(const Vector& v1, const Vector& v2) noexcept
	{
		Vector angle = XMVector3AngleBetweenNormalsEst(v1, v2);
		float x = XMConvertToDegrees(acosf(XMVectorGetX(angle)));
		float y = XMConvertToDegrees(acosf(XMVectorGetY(angle)));
		float z = XMConvertToDegrees(acosf(XMVectorGetZ(angle)));
		return Vec3(x, y, z);
	}

	inline Vec3 Angle(const Vec3& v1, const Vec3& v2) noexcept
	{
		return Angle(_VECTOR(v1), _VECTOR(v2));
	}

	inline Vec3 TransformNormal(const Vec3& vector, const Matrix& transform) noexcept
	{
		Vec3 result;
		XMStoreFloat3(&result, XMVector3TransformNormal(_VECTOR(vector), transform));
		return result;
	}

	inline Vec3 TransformCoord(const Vec3& vector, const Matrix& transform) noexcept
	{
		Vec3 result;
		XMStoreFloat3(&result, XMVector3TransformCoord(_VECTOR(vector), transform));
		return result;
	}

	inline Vec3 TransformCoord(const Vec3& vector, const Vec4x4& matrix) noexcept
	{
		return TransformCoord(vector, _MATRIX(matrix));
	}

	inline Vec3 Multiply(const Vec3& v1, const Vec3& v2) noexcept
	{
		Vec3 result;
		XMStoreFloat3(&result, _VECTOR(v1) * _VECTOR(v2));
		return result;
	}

	inline Vec3 Multiply(const Vec3& vector, float scalar) noexcept
	{
		Vec3 result;
		XMStoreFloat3(&result, (_VECTOR(vector) * scalar));
		return result;
	}

	inline Vec3 Divide(const Vec3& vector, float scalar) noexcept
	{
		if (Math::IsEqual(scalar, 0.f)) {
			return Vec3{};
		}

		Vec3 result;
		XMStoreFloat3(&result, (_VECTOR(vector) / scalar));
		return result;
	}

	inline bool IsZero(const Vec3& vector) noexcept
	{
		return XMVector3Equal(_VECTOR(vector), XMVectorZero());
	}

	inline Vec3 Negative(const Vec3& vector) noexcept
	{
		Vec3 result;
		XMStoreFloat3(&result, XMVectorNegate(_VECTOR(vector)));
		return result;
	}

	inline Vec3 Min(const Vec3& vector, float minValue) noexcept
	{
		Vec3 result;
		XMStoreFloat3(&result, XMVectorMax(_VECTOR(vector), XMVectorSet(minValue, minValue, minValue, 1.f)));
		return result;
	}

	inline Vec3 Max(const Vec3& vector, float maxValue) noexcept
	{
		Vec3 result;
		XMStoreFloat3(&result, XMVectorMin(_VECTOR(vector), XMVectorSet(maxValue, maxValue, maxValue, 1.f)));
		return result;
	}

	inline Vec3 Resize(const Vec3& vector, float size) noexcept
	{
		Vec3 result;
		XMStoreFloat3(&result, XMVector3NormalizeEst(_VECTOR(vector)) * size);
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

namespace Vector4 {
	inline Vec4 Add(const Vec4& v1, const Vec4& v2) noexcept
	{
		Vec4 result;
		XMStoreFloat4(&result, XMLoadFloat4(&v1) + XMLoadFloat4(&v2));
		return result;
	}

	inline Vec4 Multiply(const Vec4& v1, const Vec4& v2) noexcept
	{
		Vec4 result;
		XMStoreFloat4(&result, XMLoadFloat4(&v1) * XMLoadFloat4(&v2));
		return result;
	}

	inline Vec4 Normalize(const Vec4& v) noexcept
	{
		Vec4 result;
		XMStoreFloat4(&result, XMVector4NormalizeEst(XMLoadFloat4(&v)));
		return result;
	}

	inline Vec4 NormalizeColor(const Vec4& vector) noexcept
	{
		Vec3 v = Vec3(vector.x, vector.y, vector.z);
		Vec3 normal;
		XMStoreFloat3(&normal, XMVector3NormalizeEst(_VECTOR(v)));
		return Vec4(normal.x, normal.y, normal.z, vector.w);
	}

	inline Vec4 Multiply(float scalar, const Vec4& vector) noexcept
	{
		Vec4 result;
		XMStoreFloat4(&result, scalar * XMLoadFloat4(&vector));
		return result;
	}

	// (1, 1, 1, 1)
	inline Vec4 One()
	{
		return Vec4(1.f, 1.f, 1.f, 1.f);
	}

	// (0, 0, 0, 0)
	inline Vec4 Zero()
	{
		return Vec4(0.f, 0.f, 0.f, 0.f);
	}
}

namespace Matrix4x4 {
	inline Vec4x4 Identity() noexcept
	{
		Vec4x4 result;
		XMStoreFloat4x4(&result, XMMatrixIdentity());
		return result;
	}

	inline Vec4x4 Translate(float x, float y, float z) noexcept
	{
		Vec4x4 result;
		XMStoreFloat4x4(&result, XMMatrixTranslation(x, y, z));
		return result;
	}

	inline Vec4x4 Multiply(const Vec4x4& matrix1, const Vec4x4& matrix2) noexcept
	{
		Vec4x4 result;
		XMStoreFloat4x4(&result, _MATRIX(matrix1) * _MATRIX(matrix2));
		return result;
	}

	inline Vec4x4 Multiply(const Vec4x4& matrix1, const Matrix& matrix2) noexcept
	{
		Vec4x4 result;
		XMStoreFloat4x4(&result, (matrix1)*matrix2);
		return result;
	}

	inline Vec4x4 Multiply(const Matrix& matrix1, const Vec4x4& matrix2) noexcept
	{
		Vec4x4 result;
		XMStoreFloat4x4(&result, matrix1 * _MATRIX(matrix2));
		return result;
	}

	inline Vec4x4 Multiply(const Matrix& matrix1, const Matrix& matrix2) noexcept
	{
		Vec4x4 result;
		XMStoreFloat4x4(&result, matrix1 * matrix2);
		return result;
	}

	inline Vec3 Multiply(const Vec4x4& matrix, const Vec3& vector) noexcept
	{
		Vector v = XMVectorSet(vector.x, vector.y, vector.z, 1.f);
		Vec3 result;
		XMStoreFloat3(&result, XMVector4Transform(v, _MATRIX(matrix)));
		return result;
	}

	inline Vec4x4 RotationYawPitchRoll(float pitch, float yaw, float roll) noexcept
	{
		Vec4x4 result;
		XMStoreFloat4x4(&result, XMMatrixRotationRollPitchYaw(XMConvertToRadians(pitch), XMConvertToRadians(yaw), XMConvertToRadians(roll)));
		return result;
	}

	inline Vec4x4 RotationAxis(const Vec3& axis, float angle) noexcept
	{
		Vec4x4 result;
		XMStoreFloat4x4(&result, XMMatrixRotationAxis(_VECTOR(axis), XMConvertToRadians(angle)));
		return result;
	}

	inline Vec4x4 Inverse(const Vec4x4& matrix) noexcept
	{
		Vec4x4 result;
		XMStoreFloat4x4(&result, XMMatrixInverse(nullptr, _MATRIX(matrix)));
		return result;
	}

	inline Vec4x4 Transpose(const Vec4x4& matrix) noexcept
	{
		Vec4x4 result;
		XMStoreFloat4x4(&result, XMMatrixTranspose(_MATRIX(matrix)));
		return result;
	}

	inline Vec4x4 OrthographicOffCenterLH(float fFovAngleY, float aspectRatio, float fNearZ, float fFarZ) noexcept
	{
		Vec4x4 result;
		XMStoreFloat4x4(&result, XMMatrixOrthographicLH(XMConvertToRadians(fFovAngleY), aspectRatio, fNearZ, fFarZ));
		return result;
	}

	inline Vec4x4 PerspectiveFovLH(float fFovAngleY, float aspectRatio, float fNearZ, float fFarZ) noexcept
	{
		Vec4x4 result;
		XMStoreFloat4x4(&result, XMMatrixPerspectiveFovLH(XMConvertToRadians(fFovAngleY), aspectRatio, fNearZ, fFarZ));
		return result;
	}

	inline Vec4x4 LookAtLH(const Vec3& eyePos, const Vec3& lookAtPos, const Vec3& upDir, bool isTranspose = false)
	{
		Vec4x4 result;
		if (isTranspose) {
			XMStoreFloat4x4(&result, XMMatrixTranspose(XMMatrixLookAtLH(_VECTOR(eyePos), _VECTOR(lookAtPos), _VECTOR(upDir))));
		}
		else {
			XMStoreFloat4x4(&result, XMMatrixLookAtLH(_VECTOR(eyePos), _VECTOR(lookAtPos), _VECTOR(upDir)));
		}
		return result;
	}

	inline Vec4x4 LookToLH(const Vec3& eyePos, const Vec3& lookTo, const Vec3& upDir, bool isTranspose = false)
	{
		Vec4x4 result;
		if (isTranspose) {
			XMStoreFloat4x4(&result, XMMatrixTranspose(XMMatrixLookToLH(_VECTOR(eyePos), _VECTOR(lookTo), _VECTOR(upDir))));
		}
		else {
			XMStoreFloat4x4(&result, XMMatrixLookToLH(_VECTOR(eyePos), _VECTOR(lookTo), _VECTOR(upDir)));
		}
		return result;
	}


	inline Vec4x4 Interpolate(const Vec4x4& matrix1, const Vec4x4& matrix2, float t)
	{
		Vec4x4 result;

		XMVECTOR S0, R0, T0, S1, R1, T1;
		XMMatrixDecompose(&S0, &R0, &T0, _MATRIX(matrix1));
		XMMatrixDecompose(&S1, &R1, &T1, _MATRIX(matrix2));

		XMVECTOR S = XMVectorLerp(S0, S1, t);
		XMVECTOR T = XMVectorLerp(T0, T1, t);
		XMVECTOR R = XMQuaternionSlerp(R0, R1, t);
		XMStoreFloat4x4(&result, XMMatrixAffineTransformation(S, XMVectorZero(), R, T));

		return result;
	}

	inline Vec4x4 Add(const Vec4x4& matrix1, const Vec4x4& matrix2)
	{
		Vec4x4 result;
		XMStoreFloat4x4(&result, _MATRIX(matrix1) + _MATRIX(matrix2));
		return result;
	}

	inline Vec4x4 Scale(const Vec4x4& matrix, float scale)
	{
		Vec4x4 result;
		XMStoreFloat4x4(&result, _MATRIX(matrix) * scale);
		return result;
	}

	inline Vec4x4 Zero()
	{
		Vec4x4 result;
		XMStoreFloat4x4(&result, XMMatrixSet(0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f));
		return result;
	}
}

namespace XMMatrix
{
	inline Matrix RotationX(float degree)
	{
		return XMMatrixRotationX(XMConvertToRadians(degree));
	}

	inline Matrix RotationY(float degree)
	{
		return XMMatrixRotationY(XMConvertToRadians(degree));
	}

	inline Matrix RotationZ(float degree)
	{
		return XMMatrixRotationZ(XMConvertToRadians(degree));
	}

	inline Matrix RotationAxis(const Vec3& axis, float degree)
	{
		return XMMatrixRotationAxis(_VECTOR(axis), XMConvertToRadians(degree));
	}

	inline Matrix Transpose(const Vec4x4& matrix) noexcept
	{
		return XMMatrixTranspose(_MATRIX(matrix));
	}

	inline void SetPosition(Matrix& matrix, const Vec3& pos)
	{
		Vector p = XMVectorSet(pos.x, pos.y, pos.z, 1.f);
		::memcpy(&matrix.m[3], &p, sizeof(Vector));
	}
}

namespace Quaternion
{
	Vec4 LookRotation(const Vec3& direction, const Vec3& up);

	inline Vec3 ToEuler(const Vec4& q)
	{
		Vec3 result{};

		// Roll (x-axis rotation)
		float sinr_cosp = +2.0f * (q.w * q.x + q.y * q.z);
		float cosr_cosp = +1.0f - 2.0f * (q.x * q.x + q.y * q.y);
		result.x = XMConvertToDegrees(atan2(sinr_cosp, cosr_cosp));

		// Pitch (y-axis rotation)
		float sinp = +2.0f * (q.w * q.y - q.z * q.x);
		if (fabs(sinp) >= 1)
			result.y = copysign(XM_PI / 2, sinp); // use 90 degrees if out of range
		else
			result.y = asin(sinp);
		result.y = XMConvertToDegrees(result.y);

		// Yaw (z-axis rotation)
		float siny_cosp = +2.0f * (q.w * q.z + q.x * q.y);
		float cosy_cosp = +1.0f - 2.0f * (q.y * q.y + q.z * q.z);
		result.z = XMConvertToDegrees(atan2(siny_cosp, cosy_cosp));

		return result;
	}

	inline Vec4 ToQuaternion(const Vec3& euler)
	{
		Vec4 result{};
		XMStoreFloat4(&result, XMQuaternionRotationRollPitchYaw(XMConvertToRadians(euler.x), XMConvertToRadians(euler.y), XMConvertToRadians(euler.z)));
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
	void Transform(const Vec4x4& transform)
	{
		const Matrix kMatrix = _MATRIX(transform);
		const Vector kRotation = XMQuaternionRotationMatrix(_MATRIX(transform));

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
	void Transform(const Vec4x4& transform)
	{
		Center = Matrix4x4::Multiply(transform, mOriginCenter);
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