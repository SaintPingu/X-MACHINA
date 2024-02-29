#pragma once

/* orders of class functions */
/*---------------------------------------*/
class SampleClass {
public:
	#pragma region C/Dtor
	#pragma endregion

	#pragma region Getter
	#pragma endregion

	#pragma region Setter
	#pragma endregion
};
/*---------------------------------------*/

/* orders of header */
/*---------------------------------------*/
#pragma region Pragma
#pragma endregion


#pragma region Include
#pragma endregion


#pragma region Define
#pragma endregion


#pragma region Using
#pragma endregion


#pragma region ClassForwardDecl
#pragma endregion


#pragma region EnumClass
#pragma endregion


#pragma region Variable
#pragma endregion


#pragma region Struct
#pragma endregion


#pragma region Function
#pragma endregion


#pragma region NameSpace
#pragma endregion


#pragma region Class
#pragma endregion
/*---------------------------------------*/

#pragma region Pragma
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma endregion


#pragma region Include
/* Windows */
#include <windows.h>
#include <Mmsystem.h>

/* C */
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <math.h>
#include <crtdbg.h>
#include <comdef.h>
#include <wrl.h>
#include <shellapi.h>

/* C++ */
#include <memory>
#include <string>
#include <fstream>
#include <random>
#include <functional>
#include <filesystem>
#include <type_traits>

/* STL Containers */
#include <span>
#include <array>
#include <vector>
#include <forward_list>
#include <deque>
#include <set>
#include <bitset>
#include <unordered_set>
#include <unordered_map>
#include <queue>

/* DirectX */
#include <d3d12.h>
#include <dxgidebug.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>
#include <D3d12SDKLayers.h>
#include <d3dx12.h>

/* Custom */
#include "Singleton.h"
#pragma endregion


#pragma region Define
#define WIN32_LEAN_AND_MEAN		// 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.

/* ifdef */
#ifdef _DEBUG

// 메모리 누수 검사, new를 하고 delete를 하지 않은 경우 그 위치를 보여준다.
#define _CRTDBG_MAP_ALLOC

#ifdef _DEBUG
#ifdef UNICODE                                                                                      
#pragma comment(linker, "/entry:wWinMainCRTStartup /subsystem:console") 
#else                                                                                                    
#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")   
#endif                                                                                                   
#endif        

#pragma region Imgui - 장재문 -
//#define new new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#pragma endregion


#else

#define new new

#endif

/* Macro */
#define RANDOM_COLOR	Vec4(rand() / float(RAND_MAX), rand() / float(RAND_MAX), rand() / float(RAND_MAX), rand() / float(RAND_MAX))	// return random Vec4

#define _MATRIX(x)	XMLoadFloat4x4(&x)
#define _VECTOR(x)	XMLoadFloat3(&x)

#define TO_STRING( x ) #x				// ex) TO_STRING(myVar) ==> "myVar"

// DirectX 함수 hFunctionCall 함수를 평가하여 hResult가 실패했다면 예외를 발생시킨다.
#define THROW_IF_FAILED(hFunctionCall)															\
{																								\
    HRESULT hResult = (hFunctionCall);															\
    std::wstring fileName = AnsiToWString(__FILE__);											\
    if(FAILED(hResult)) { throw DxException(hResult, L#hFunctionCall, fileName, __LINE__); }	\
}

#pragma endregion


#pragma region Using
/* DirectX */
using namespace DirectX;
using namespace DirectX::PackedVector;
using Microsoft::WRL::ComPtr;

/* DirectX Math */
using Vec2   = XMFLOAT2;
using Vec3   = XMFLOAT3;
using Vec4   = XMFLOAT4;
using Vec4x4 = XMFLOAT4X4;
using Vec4x3 = XMFLOAT4X3;
using Vector = XMVECTOR;
using Matrix = XMMATRIX;

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


#pragma region EnumClass
enum class D3DResource {
	Texture2D = 0,
	Texture2D_Array,	// []
	Texture2DArray,
	TextureCube,
	Buffer
};

enum class Dir : WORD {
	Front = 0x01,
	Back  = 0x02,
	Left  = 0x04,
	Right = 0x08,
	Up    = 0x10,
	Down  = 0x20
};

// 카메라 시점
enum class CameraMode {
	None  = -1,
	Third = 0,
};

// [ root parameter index alias ]
// usage:
// index = scene->GetGraphicsRootParamIndex(RootParam);
// 32BitConstant => scene->SetGraphicsRoot32BitConstants(RootParam, ...);
enum class RootParam {
	// Compute RootParam
	Object = 0,
	Pass,
	SkinMesh,
	Instancing,
	Material,
	SkyBox,
	Texture,
	Collider,

	// Compute RootParam
	Weight = 0,
	Read,
	LUT0,
	LUT1,
	Write,
};

enum class TextureMap : UINT8 {
	DiffuseMap0 = 0,
	DiffuseMap1,
	DiffuseMap2,
	DiffuseMap3,

	NormalMap,
	HeightMap,
	ShadowMap,
	RoughnessMap,

	_count
};
enum { TextureMapCount = static_cast<UINT8>(TextureMap::_count) };

enum class GBuffer : UINT8 {
	Texture = 0,
	UI,
	Normal,
	Depth,
	Distance,

	_count
};
enum { GBufferCount = static_cast<UINT8>(GBuffer::_count) };

enum class GroupType : UINT8 {
	SwapChain = 0,
	GBuffer,

	_count
};
enum { MRTGroupTypeCount = static_cast<UINT8>(GroupType::_count) };
#pragma endregion


#pragma region Variable
constexpr short gkFrameBufferWidth  = 1280;
constexpr short gkFrameBufferHeight = 960;

constexpr int	gkMaxTexture		= 200;	// 씬에 존재할 수 있는 텍스처의 최대 개수. Common.hlsl과 동일해야 한다.
constexpr int	gkMaxSceneLight		= 32;	// 씬에 존재할 수 있는 조명의 최대 개수. Light.hlsl과 동일해야 한다.

constexpr int	gkSkinBoneSize = 128;
#pragma endregion


#pragma region Struct
struct VertexBufferViews {
	ComPtr<ID3D12Resource> VertexBuffer{};
	ComPtr<ID3D12Resource> NormalBuffer{};
	ComPtr<ID3D12Resource> UV0Buffer{};
	ComPtr<ID3D12Resource> UV1Buffer{};
	ComPtr<ID3D12Resource> TangentBuffer{};
	ComPtr<ID3D12Resource> BiTangentBuffer{};
	ComPtr<ID3D12Resource> BoneIndexBuffer{};
	ComPtr<ID3D12Resource> BoneWeightBuffer{};
};

// must be matched with Light.hlsl LightInfo
struct LightInfo {
	Vec4	Ambient{};
	Vec4	Diffuse{};
	Vec4	Specular{};

	Vec3	Position{};
	float	Falloff{};

	Vec3	Direction{};
	float	Theta{};

	Vec3	Attenuation{};
	float	Phi{};

	float	Range{};
	float	Padding{};
	int		Type{};
	bool	IsEnable{};
};

// must be matched with Light.hlsl cbLights
struct SceneLight {
	std::array<LightInfo, gkMaxSceneLight> Lights{};

	Vec4	GlobalAmbient{};

	Vec4	FogColor{};
	float	FogStart = 100.f;
	float	FogRange = 300.f;
};
#pragma endregion


#pragma region Function
void PrintMsgBox(const char* message);

// print error msg if blob has error
void PrintErrorBlob(RComPtr<ID3DBlob> errorBlob);

inline Vector GetUnitVector(float x, float y, float z)
{
	return XMVector3NormalizeEst(XMVectorSet(x, y, z, 0.f));
}
Vector RandVectorOnSphere();
Vector RandVectorOnDom();

template<class T>
// copy src's data to end of dst
inline void CopyBack(const std::vector<T>& src, std::vector<T>& dst)
{
	std::copy(src.begin(), src.end(), std::back_inserter(dst));
}

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

// assert if hResult is failed
inline void AssertHResult(HRESULT hResult)
{
	assert(SUCCEEDED(hResult));
}

// string 형식의 ANSI 문자열을 wstring으로 변환
inline std::wstring AnsiToWString(const std::string& str)
{
	WCHAR buffer[512];
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buffer, 512);
	return std::wstring(buffer);
}

#pragma region EnumOperator_DWORD
// 아래 정의된 EnumType의 DWORD 연산을 오버로딩하여 불필요한 static_cast를 줄인다.
// is_valid_dword_type_v에 EnumType을 추가해주어야 한다.
enum class ObjectTag : DWORD;
enum class VertexType : DWORD;
enum class MaterialMap : DWORD;
enum class HumanBone : DWORD;

template <typename T>
constexpr bool is_valid_dword_type_v = (std::is_same_v<T, Dir> || std::is_same_v<T, ObjectTag> || std::is_same_v<T, VertexType> || std::is_same_v<T, MaterialMap> || std::is_same_v<T, HumanBone>);

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


#pragma region NameSpace

namespace Math {
	constexpr float kPI      = 3.141592f;
	constexpr float kGravity = 9.8f;

	static std::default_random_engine dre;

	inline bool IsZero(float f) { return fabsf(f) < FLT_EPSILON; }
	inline bool IsEqual(float a, float b) { return Math::IsZero(a - b); }
	inline float InverseSqrt(float f) { return 1.f / sqrtf(f); }

	inline float RandF(float min, float max)
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

namespace D3DUtil {

	// create buffer resource from data
	void CreateBufferResource(
		const void* data,
		size_t byteSize,
		D3D12_HEAP_TYPE heapType,
		D3D12_RESOURCE_STATES resourceStates,
		ComPtr<ID3D12Resource>& uploadBuffer,
		ComPtr<ID3D12Resource>& buffer);

	// upload buffer can be nullptr
	inline void CreateBufferResource(
		const void* data,
		size_t byteSize,
		D3D12_HEAP_TYPE heapType,
		D3D12_RESOURCE_STATES resourceStates,
		ID3D12Resource** uploadBuffer,
		ComPtr<ID3D12Resource>& buffer)
	{
		assert(!uploadBuffer);
		ComPtr<ID3D12Resource> tempUploadBuffer{};
		D3DUtil::CreateBufferResource(data, byteSize, heapType, resourceStates, tempUploadBuffer, buffer);
	}

	template<class DataType>
	inline void CreateVertexBufferResource(
		const std::vector<DataType>& data,
		ComPtr<ID3D12Resource>& uploadBuffer,
		ComPtr<ID3D12Resource>& buffer)
	{
		if (data.empty()) {
			return;
		}

		D3DUtil::CreateBufferResource(data.data(), sizeof(DataType) * data.size(), D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, uploadBuffer, buffer);
	}

	inline void CreateIndexBufferResource(
		const std::vector<UINT>& indices,
		ComPtr<ID3D12Resource>& uploadBuffer,
		ComPtr<ID3D12Resource>& buffer)
	{
		D3DUtil::CreateBufferResource(indices.data(), sizeof(UINT) * indices.size(), D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, uploadBuffer, buffer);
	}

	void CreateVertexBufferViews(std::vector<D3D12_VERTEX_BUFFER_VIEW>& out, size_t vertexCount, const VertexBufferViews& bufferViews);

	template<class DataType>
	inline void CreateVertexBufferView(D3D12_VERTEX_BUFFER_VIEW& vertexBufferView, size_t vertexCount, RComPtr<ID3D12Resource> vertexBuffer)
	{
		assert(vertexBuffer);
		vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
		vertexBufferView.StrideInBytes = sizeof(DataType);
		vertexBufferView.SizeInBytes = (UINT)(sizeof(DataType) * vertexCount);
	}


	inline void CreateIndexBufferView(D3D12_INDEX_BUFFER_VIEW& IndexBufferView, size_t indexCount, RComPtr<ID3D12Resource> indexBuffer)
	{
		assert(indexBuffer);
		IndexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
		IndexBufferView.Format = DXGI_FORMAT_R32_UINT;
		IndexBufferView.SizeInBytes = (UINT)(sizeof(UINT) * indexCount);
	}


	void CreateTextureResourceFromDDSFile(
		const std::wstring& fileName,
		ComPtr<ID3D12Resource>& uploadBuffer,
		ComPtr<ID3D12Resource>& texture,
		D3D12_RESOURCE_STATES resourceStates = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	ComPtr<ID3D12Resource> CreateTexture2DResource(
		UINT width,
		UINT height,
		UINT elements,
		UINT mipLevels,
		DXGI_FORMAT dxgiFormat,
		D3D12_RESOURCE_FLAGS resourceFlags,
		D3D12_RESOURCE_STATES resourceStates,
		Vec4 clearValue);

	D3D12_RESOURCE_BARRIER ResourceTransition(
		RComPtr<ID3D12Resource> resource,
		D3D12_RESOURCE_STATES stateBefore,
		D3D12_RESOURCE_STATES stateAfter);

	D3D12_SHADER_BYTECODE CompileShaderFile(
		const std::wstring& fileName,
		LPCSTR shaderName,
		LPCSTR shaderProfile,
		ComPtr<ID3DBlob>& shaderBlob);

	D3D12_SHADER_BYTECODE ReadCompiledShaderFile(
		const std::wstring& fileName,
		ComPtr<ID3DBlob>& shaderBlob);

	inline UINT CalcConstantBuffSize(UINT byteSize)
	{
		// 상수 버퍼의 크기는 하드웨어의 최소 메모리 할당 크기에 배수(256배수)가 되어야 한다.
		// 따라서, 255를 더하고 256보다 작은 모든 비트를 제거한다.
		return (byteSize + 255) & ~255;
	}
}

#pragma region DirectXMath
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
		Vec3 mNormal;
		XMStoreFloat3(&mNormal, XMVector3NormalizeEst(_VECTOR(vector)));
		return mNormal;
	}

	inline float Length(const Vec3& vector) noexcept
	{
		Vec3 result;
		XMStoreFloat3(&result, XMVector3LengthEst(_VECTOR(vector)));
		return result.x;
	}

	inline float AngleX(const Vector& v1, const Vector& v2) noexcept
	{
		Vector angle = XMVector3AngleBetweenNormalsEst(v1, v2);
		return XMConvertToDegrees(acosf(XMVectorGetX(angle)));
	}

	inline float AngleX(const Vec3& v1, const Vec3& v2) noexcept
	{
		return AngleX(_VECTOR(v1), _VECTOR(v2));
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
	inline Vec3 Forrward()
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
		XMStoreFloat4x4(&result, _MATRIX(matrix1) * matrix2);
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
		::memcpy(&matrix.r[3], &p, sizeof(Vector));
	}
}
	#pragma endregion
#pragma endregion


#pragma region Class
class DxException {
public:
	HRESULT mErrorCode = S_OK;
	std::wstring mFunctionName{};
	std::wstring mFilename{};
	int mLineNumber = -1;

public:
	DxException() = default;
	DxException(HRESULT hr, const std::wstring& functionName, const std::wstring& fileName, int lineNumber);

	std::wstring ToString()const;
};

// 복사를 방지하기 위한 기본 클래스로 컴파일 시간에 복사 방지가 가능하다.
// 복사하고 싶지 않은 객체가 있다면 이 클래스를 상속받는다.
class UnCopyable {
protected:
	// 생성과 소멸은 허용한다.
	UnCopyable() = default;
	~UnCopyable() = default;

private:
	// 정의를 두지 않았으며 복사는 허용하지 않는다.
	UnCopyable(const UnCopyable&);
	UnCopyable& operator=(const UnCopyable&);
};

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
	void Transform(const Vec4x4& transform);

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

	Vec3 GetOrigin() const	{ return mOriginCenter; }

	void SetOrigin(const Vec3& origin) { mOriginCenter = origin; }

public:
	void Transform(const Vec4x4& transform);

	bool IntersectBoxes(const std::vector<MyBoundingOrientedBox*>& boxes) const;

	operator const BoundingSphere& () const {
		return static_cast<const BoundingSphere&>(*this);
	}
};
#pragma endregion