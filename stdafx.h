/* orders of class functions for comment */
// [ ClassName ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// [ Constructor ] /////




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// [ Getter ] /////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// [ Setter ] /////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// [ Others ] /////

//////////////////* Functions *//////////////////




#pragma once

// import libraries
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

// ignore warnings
#pragma warning(disable : 4018) // signed/unsigned mismatch
#pragma warning(disable : 4244) // 'conversion': 'type1'에서 'type2'(으)로 변환하면서 데이터가 손실될 수 있습니다.
#pragma warning(disable : 4267) // 'var' : 'size_t'에서 'type'으로 변환하면서 데이터가 손실될 수 있습니다.

// Windows
#include <windows.h>

// C runtime
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <math.h>
#include <crtdbg.h>

#include <wrl.h>
#include <shellapi.h>

// C++
#include <memory>
#include <string>
#include <fstream>
#include <random>
#include <functional>
#include <filesystem>

// STL containers
#include <span>
#include <array>
#include <vector>
#include <forward_list>
#include <deque>
#include <set>
#include <bitset>
#include <unordered_set>
#include <unordered_map>

// DirectX
#include <d3d12.h>
#include <dxgidebug.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>
#include <D3d12SDKLayers.h>

// Others
#include <Mmsystem.h>

using namespace DirectX;
using namespace DirectX::PackedVector;

using Microsoft::WRL::ComPtr;






// [ define ]
// 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.
#define WIN32_LEAN_AND_MEAN

// [ ifdef ]
// 메모리 누수 검사, new를 하고 delete를 하지 않은 경우 그 위치를 보여준다.
#ifdef _DEBUG
// CRT 라이브러를 통해 메모리 누수를 찾도록 한다.
#define _CRTDBG_MAP_ALLOC
#define new new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#else
#define new new
#endif

// [ values ]
#define FRAME_BUFFER_WIDTH		1280
#define FRAME_BUFFER_HEIGHT		960

#define RESOURCE_TEXTURE2D			0x01
#define RESOURCE_TEXTURE2D_ARRAY	0x02	//[]
#define RESOURCE_TEXTURE2DARRAY		0x03
#define RESOURCE_TEXTURE_CUBE		0x04
#define RESOURCE_BUFFER				0x05

enum class Dir : WORD { 
	Front = 0x01,
	Back  = 0x02,
	Left  = 0x04,
	Right = 0x08,
	Up    = 0x10,
	Down  = 0x20
};

constexpr float EPSILON = 1.0e-10f;
constexpr float PI      = 3.141592f;
constexpr float GRAVITY = 9.8f;


// [ functions ]
#define RANDOM_COLOR	Vec4(rand() / float(RAND_MAX), rand() / float(RAND_MAX), rand() / float(RAND_MAX), rand() / float(RAND_MAX))

#define _MATRIX(x)	XMLoadFloat4x4(&x)
#define _VECTOR(x)	XMLoadFloat3(&x)

#define TO_STRING( x ) #x				// ex) TO_STRING(myVar) ==> "myVar"





#define SINGLETON_PATTERN(Type)				\
private:									\
   static Type* mInst;						\
public:										\
   static Type* Inst() {					\
      return mInst;							\
   }										\
   static void Destroy() {					\
      if (nullptr != mInst) {				\
         delete mInst;						\
         mInst = nullptr;					\
      }										\
   }										\

#define SINGLETON_PATTERN_DEFINITION(Type)  \
   Type* Type::mInst = new Type;			\







// [ enum class ]
enum class CameraMode {
	None = -1,
	Third = 0,
};
enum class RootParam {
	GameObjectInfo = 0,
	Camera,
	Light,
	GameInfo,
	SpriteInfo,

	Instancing,

	Texture,
	Texture1,
	Texture2,
	Texture3,
	Texture4,
	RenderTarget,

	TerrainLayer0,
	TerrainLayer1,
	TerrainLayer2,
	TerrainLayer3,
	SplatMap,
};




// [ extern ]
extern UINT	gCbvSrvDescriptorIncSize;
extern UINT	gRtvDescriptorIncSize;




/* using */
// [ DirectX Math ]
using Vec2 = XMFLOAT2;
using Vec3 = XMFLOAT3;
using Vec4 = XMFLOAT4;
using Vec4x4 = XMFLOAT4X4;
using Matrix = XMMATRIX;

// [ Others ]
template<class T>
using RComPtr = const ComPtr<T>&;
template<class T>
using sptr = std::shared_ptr<T>;
template<class T>
using rsptr = const sptr<T>&;
template<class T>
using uptr = std::unique_ptr<T>;
template<class T>
using ruptr = const uptr<T>&;
template<class T>
using wptr = std::weak_ptr<T>;
template<class T>
using rwptr = const wptr<T>&;


//////////////////* Math *//////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace Math {
	inline bool IsZero(float f) { return fabsf(f) < EPSILON; }
	inline bool IsEqual(float a, float b) { return Math::IsZero(a - b); }
	inline float InverseSqrt(float f) { return 1.0f / sqrtf(f); }

	inline float RandF(float fMin, float fMax) noexcept
	{
		return fMin + ((float)rand() / (float)RAND_MAX) * (fMax - fMin);
	}

	inline int sign(float x) noexcept
	{
		return (x >= 0.0f) ? 1 : -1;
	}
	int RoundToNearestMultiple(int value, int multiple);
}

//////////////////* DirectXMath *//////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Vector3 {

	inline Vec3 ScalarProduct(const Vec3& vector, float scalar, bool normalize = true) noexcept
	{
		Vec3 result;
		if (normalize) {
			XMStoreFloat3(&result, XMVector3Normalize(_VECTOR(vector)) * scalar);
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
		XMVECTOR cross = XMVector3Cross(_VECTOR(v1), _VECTOR(v2));
		if (normalize) {
			cross = XMVector3Normalize(cross);
		}
		
		XMStoreFloat3(&result, cross);

		return result;
	}

	inline Vec3 Normalize(const Vec3& vector) noexcept
	{
		Vec3 mNormal;
		XMStoreFloat3(&mNormal, XMVector3Normalize(_VECTOR(vector)));
		return mNormal;
	}

	inline float Length(const Vec3& vector) noexcept
	{
		Vec3 result;
		XMStoreFloat3(&result, XMVector3Length(_VECTOR(vector)));
		return result.x;
	}

	inline float AngleX(const XMVECTOR& v1, const XMVECTOR& v2) noexcept
	{
		XMVECTOR xmvAngle = XMVector3AngleBetweenNormals(v1, v2);
		return XMConvertToDegrees(acosf(XMVectorGetX(xmvAngle)));
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
		if (Math::IsEqual(scalar, .0f)) {
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
		XMStoreFloat3(&result, XMVectorMax(_VECTOR(vector), XMVectorSet(minValue, minValue, minValue, 1.0f)));
		return result;
	}

	inline Vec3 Max(const Vec3& vector, float maxValue) noexcept
	{
		Vec3 result;
		XMStoreFloat3(&result, XMVectorMin(_VECTOR(vector), XMVectorSet(maxValue, maxValue, maxValue, 1.0f)));
		return result;
	}

	inline Vec3 Resize(const Vec3& vector, float size) noexcept
	{
		Vec3 result;
		XMStoreFloat3(&result, XMVector3Normalize(_VECTOR(vector)) * size);
		return result;
	}

	// (0, 0, 0)
	inline Vec3 Zero()
	{
		return Vec3(.0f, .0f, .0f);
	}

	// (0, 1, 0)
	inline Vec3 Up()
	{
		return Vec3(.0f, 1.f, .0f);
	}

	// (0, -1, 0)
	inline Vec3 Down()
	{
		return Vec3(.0f, -1.f, .0f);
	}

	// (1, 0, 0)
	inline Vec3 Right()
	{
		return Vec3(1.f, .0f, .0f);
	}

	// (-1, 0, 0)
	inline Vec3 Left()
	{
		return Vec3(-1.f, 0.f, .0f);
	}

	// (0, 0, 1)
	inline Vec3 Forrward()
	{
		return Vec3(.0f, .0f, 1.f);
	}

	// (0, 0, -1)
	inline Vec3 Back()
	{
		return Vec3(.0f, .0f, -1.f);
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

	inline Vec4 Multiply(float scalar, const Vec4& vector) noexcept
	{
		Vec4 result;
		XMStoreFloat4(&result, scalar * XMLoadFloat4(&vector));
		return result;
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

	inline Vec4x4 Multiply(const Vec4x4& matrix1, const XMMATRIX& matrix2) noexcept
	{
		Vec4x4 result;
		XMStoreFloat4x4(&result, _MATRIX(matrix1) * matrix2);
		return result;
	}

	inline Vec4x4 Multiply(const XMMATRIX& matrix1, const Vec4x4& matrix2) noexcept
	{
		Vec4x4 result;
		XMStoreFloat4x4(&result, matrix1 * _MATRIX(matrix2));
		return result;
	}

	inline Vec4x4 Multiply(const XMMATRIX& matrix1, const XMMATRIX& matrix2) noexcept
	{
		Vec4x4 result;
		XMStoreFloat4x4(&result, matrix1 * matrix2);
		return result;
	}

	inline Vec3 Multiply(const Vec4x4& matrix, const Vec3& vector) noexcept
	{
		XMVECTOR v = XMVectorSet(vector.x, vector.y, vector.z, 1.0f);
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
		XMStoreFloat4x4(&result, XMMatrixInverse(NULL, _MATRIX(matrix)));
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

	inline Vec4x4 LookAtLH(const Vec3& eyePos, const Vec3& lookAtPos, const Vec3& upDir) noexcept
	{
		Vec4x4 result;
		XMStoreFloat4x4(&result, XMMatrixLookAtLH(_VECTOR(eyePos), _VECTOR(lookAtPos), _VECTOR(upDir)));
		return result;
	}

	inline Vec4x4 LookToLH(const Vec3& eyePos, const Vec3& xmf3LookTo, const Vec3& upDir) noexcept
	{
		Vec4x4 result;
		XMStoreFloat4x4(&result, XMMatrixLookToLH(_VECTOR(eyePos), _VECTOR(xmf3LookTo), _VECTOR(upDir)));
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
		XMVECTOR p = XMVectorSet(pos.x, pos.y, pos.z, 1.f);
		::memcpy(&matrix.r[3], &p, sizeof(XMVECTOR));
	}
}

namespace Triangle
{
	inline bool Intersect(Vec3& pos, Vec3& dir, Vec3& v0, Vec3& v1, Vec3& v2, float& hitDistance) noexcept
	{
		return TriangleTests::Intersects(_VECTOR(pos), _VECTOR(dir), _VECTOR(v0), _VECTOR(v1), _VECTOR(v2), hitDistance);
	}
}

namespace Plane
{
	inline Vec4 Normalize(Vec4& plane) noexcept
	{
		Vec4 result;
		XMStoreFloat4(&result, XMPlaneNormalize(XMLoadFloat4(&plane)));
		return result;
	}
}




//////////////////* Bounds *//////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class MyBoundingOrientedBox : public BoundingOrientedBox {
public:
	Vec3 OriginCenter{};

	// no apply scale
	inline void Transform(const Vec4x4& transform) noexcept
	{
		Matrix matrix = _MATRIX(transform);
		XMVECTOR rotation = XMQuaternionRotationMatrix(matrix);
		XMStoreFloat4(&Orientation, rotation);
		XMStoreFloat3(&Center, XMVector3Transform(_VECTOR(OriginCenter), matrix));
	}

	operator const BoundingOrientedBox& () const {
		return static_cast<const BoundingOrientedBox&>(*this);
	}
};

class MyBoundingSphere : public BoundingSphere {
public:
	Vec3 OriginCenter{};

	inline void Transform(const Vec4x4& transform) noexcept
	{
		Center = Matrix4x4::Multiply(transform, OriginCenter);
	}

	inline bool IntersectBoxes(const std::vector<MyBoundingOrientedBox*>& boxes) const noexcept {
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


//////////////////* Functions *//////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////* DirectX *//////////////////
struct BufferViews {
	ComPtr<ID3D12Resource> vertexBuffer{};
	ComPtr<ID3D12Resource> normalBuffer{};
	ComPtr<ID3D12Resource> UV0Buffer{};
	ComPtr<ID3D12Resource> UV1Buffer{};
	ComPtr<ID3D12Resource> tangentBuffer{};
	ComPtr<ID3D12Resource> biTangentBuffer{};
};
namespace D3DUtil {

	void CreateBufferResource(
		const void* data,
		UINT byteSize,
		D3D12_HEAP_TYPE heapType,
		D3D12_RESOURCE_STATES resourceStates,
		ComPtr<ID3D12Resource>& uploadBuffer,
		ComPtr<ID3D12Resource>& buffer);

	inline void CreateBufferResource(
		const void* data,
		UINT byteSize,
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

	void CreateVertexBufferViews(std::vector<D3D12_VERTEX_BUFFER_VIEW>& vertexBufferViews, size_t vertexCount, const BufferViews& bufferViews);

	template<class DataType>
	inline void CreateVertexBufferView(D3D12_VERTEX_BUFFER_VIEW& vertexBufferView, size_t vertexCount, RComPtr<ID3D12Resource> vertexBuffer)
	{
		assert(vertexBuffer);
		vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
		vertexBufferView.StrideInBytes = sizeof(DataType);
		vertexBufferView.SizeInBytes = sizeof(DataType) * vertexCount;
	}


	inline void CreateIndexBufferView(D3D12_INDEX_BUFFER_VIEW& IndexBufferView, size_t indexCount, RComPtr<ID3D12Resource> indexBuffer)
	{
		assert(indexBuffer);
		IndexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
		IndexBufferView.Format = DXGI_FORMAT_R32_UINT;
		IndexBufferView.SizeInBytes = sizeof(UINT) * indexCount;
	}


	void CreateTextureResourceFromDDSFile(
		std::wstring fileName,
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
		D3D12_CLEAR_VALUE* clearValue);

	void ResourceTransition(
		RComPtr<ID3D12Resource> resource,
		D3D12_RESOURCE_STATES stateBefore,
		D3D12_RESOURCE_STATES stateAfter);

	inline UINT CalcConstantBuffSize(UINT byteSize)
	{
		// 상수 버퍼의 크기는 하드웨어의 최소 메모리 할당 크기에 배수(256배수)가 되어야 한다.
		// 따라서, 255를 더하고 256보다 작은 모든 비트를 제거한다.
		return (byteSize + 255) & ~255;
	}
}

//////////////////* Others *//////////////////
void PrintMessage(const char* message);
void PrintErrorBlob(RComPtr<ID3DBlob> errorBlob);
inline XMVECTOR GetUnitVector(float x, float y, float z)
{
	return XMVector3Normalize(XMVectorSet(x, y, z, 0.f));
}

inline XMVECTOR RandVectorOnSphere()
{
	return GetUnitVector(Math::RandF(-1.0f, 1.0f), Math::RandF(-1.0f, 1.0f), Math::RandF(-1.0f, 1.0f));
}

inline XMVECTOR RandVectorOnDom()
{
	return GetUnitVector(Math::RandF(-1.0f, 1.0f), Math::RandF(.0f, 1.0f), Math::RandF(-1.0f, 1.0f));
}

template<class T>
inline void CopyBack(const std::vector<T>& src, std::vector<T>& dst)
{
	copy(src.begin(), src.end(), std::back_inserter(dst));
}


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