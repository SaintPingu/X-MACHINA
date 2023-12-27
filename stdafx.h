/* orders of class functions for comment */
// [ ClassName ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//===== (ClassName) =====//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// [ Constructor ] /////




//===== (ClassName) =====//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// [ Getter ] /////



//===== (ClassName) =====//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// [ Setter ] /////



//===== (ClassName) =====//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// [ Others ] /////

//////////////////* Functions *//////////////////




#pragma once

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma warning(disable : 4018) // signed/unsigned mismatch
#pragma warning(disable : 4244) // 'conversion': 'type1'에서 'type2'(으)로 변환하면서 데이터가 손실될 수 있습니다.
#pragma warning(disable : 4267) // 'var' : 'size_t'에서 'type'으로 변환하면서 데이터가 손실될 수 있습니다.

// Windows 헤더 파일:
#include <windows.h>

// C의 런타임 헤더 파일입니다.
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
using namespace std;

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

#include <Mmsystem.h>

using namespace DirectX;
using namespace DirectX::PackedVector;

using Microsoft::WRL::ComPtr;

// [ define ]
#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.
#define _CRTDBG_MAP_ALLOC

// [ ifdef ]
// 메모리 누수 검사, new를 하고 delete를 하지 않은 경우 그 위치를 보여준다.
#ifdef _DEBUG
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

#define DIR_FORWARD				0x01
#define DIR_BACKWARD			0x02
#define DIR_LEFT				0x04
#define DIR_RIGHT				0x08
#define DIR_UP					0x10
#define DIR_DOWN				0x20

#define EPSILON 1.0e-10f
#define PI		3.141592f
#define GRAVITY 9.8f


// [ functions ]
#define RANDOM_COLOR	Vec4(rand() / float(RAND_MAX), rand() / float(RAND_MAX), rand() / float(RAND_MAX), rand() / float(RAND_MAX))

#define _MATRIX(x)	XMLoadFloat4x4(&x)
#define _VECTOR(x)	XMLoadFloat3(&x)

#define TO_STRING( x ) #x				// ex) TO_STRING(myVar) ==> "myVar"



#define SINGLETON_PATTERN(TYPE)				\
private:									\
   static TYPE* mInst;						\
public:										\
   static TYPE* Inst()					\
   {										\
      if (!mInst) mInst = new TYPE;			\
      return mInst;							\
   }										\
   static void Destroy() {					\
      if (nullptr != mInst) {				\
         delete mInst;						\
         mInst = nullptr;					\
      }										\
   }                  

#define SINGLETON_PATTERN_DEFINITION(TYPE)  \
   TYPE* TYPE::mInst = nullptr;



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
// TODO: 프로그램에 필요한 추가 헤더는 여기에서 참조합니다.
extern UINT	gnCbvSrvDescriptorIncrementSize;
extern UINT	gnRtvDescriptorIncrementSize;



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




//////////////////* DirectXMath *//////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Vector3 {
	inline Vec3 XMVectorToFloat3(const XMVECTOR& vector) noexcept
	{
		Vec3 result;
		XMStoreFloat3(&result, vector);
		return result;
	}

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
		XMStoreFloat3(&result, XMVectorAdd(_VECTOR(v1), _VECTOR(v2)));
		return result;
	}

	inline Vec3 Add(const Vec3& v1, const Vec3& v2, const Vec3& v3) noexcept
	{
		Vec3 result;
		XMStoreFloat3(&result, XMVectorAdd(XMVectorAdd(_VECTOR(v1), _VECTOR(v2)), _VECTOR(v3)));
		return result;
	}

	inline Vec3 Add(const Vec3& v1, const Vec3& v2, float scalar) noexcept
	{
		Vec3 result;
		XMStoreFloat3(&result, XMVectorAdd(_VECTOR(v1), (_VECTOR(v2) * scalar)));
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
		if (normalize) {
			XMStoreFloat3(&result, XMVector3Normalize(XMVector3Cross(_VECTOR(v1), _VECTOR(v2))));
		}
		else {
			XMStoreFloat3(&result, XMVector3Cross(_VECTOR(v1), _VECTOR(v2)));
		}

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
		//return XMConvertToDegrees(acosf(XMVectorGetY(xmvAngle)));
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
		XMStoreFloat3(&result, XMVectorMultiply(_VECTOR(v1), _VECTOR(v2)));
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

	inline Vec3 Zero()noexcept
	{
		return Vec3(.0f, .0f, .0f);
	}

	inline Vec3 Up()noexcept
	{
		return Vec3(.0f, 1.f, .0f);
	}

	inline Vec3 Down()noexcept
	{
		return Vec3(.0f, -1.f, .0f);
	}

	inline Vec3 Right()noexcept
	{
		return Vec3(1.f, .0f, .0f);
	}

	inline Vec3 Left()noexcept
	{
		return Vec3(-1.f, 0.f, .0f);
	}

	inline Vec3 Forrward()noexcept
	{
		return Vec3(.0f, .0f, 1.f);
	}

	inline Vec3 Back()noexcept
	{
		return Vec3(.0f, .0f, -1.f);
	}

	inline Vec3 One()noexcept
	{
		return Vec3(1.f, 1.f, 1.f);
	}
}

namespace Vector4 {
	inline Vec4 Add(const Vec4& v1, const Vec4& v2) noexcept
	{
		Vec4 xmf4Result;
		XMStoreFloat4(&xmf4Result, XMLoadFloat4(&v1) + XMLoadFloat4(&v2));
		return xmf4Result;
	}

	inline Vec4 Multiply(const Vec4& v1, const Vec4& v2) noexcept
	{
		Vec4 xmf4Result;
		XMStoreFloat4(&xmf4Result, XMLoadFloat4(&v1) * XMLoadFloat4(&v2));
		return xmf4Result;
	}

	inline Vec4 Normalize(const Vec4& v) noexcept
	{
		Vec4 xmf4Result;
		XMStoreFloat4(&xmf4Result, XMVector4NormalizeEst(XMLoadFloat4(&v)));
		return xmf4Result;
	}

	inline Vec4 Multiply(float scalar, const Vec4& vector) noexcept
	{
		Vec4 xmf4Result;
		XMStoreFloat4(&xmf4Result, scalar * XMLoadFloat4(&vector));
		return xmf4Result;
	}
}

namespace Matrix4x4 {
	inline Vec4x4 Identity() noexcept
	{
		Vec4x4 xmmtx4x4Result;
		XMStoreFloat4x4(&xmmtx4x4Result, XMMatrixIdentity());
		return(xmmtx4x4Result);
	}

	inline Vec4x4 Translate(float x, float y, float z) noexcept
	{
		Vec4x4 xmmtx4x4Result;
		XMStoreFloat4x4(&xmmtx4x4Result, XMMatrixTranslation(x, y, z));
		return(xmmtx4x4Result);
	}

	inline Vec4x4 Multiply(const Vec4x4& matrix1, const Vec4x4& matrix2) noexcept
	{
		Vec4x4 xmmtx4x4Result;
		XMStoreFloat4x4(&xmmtx4x4Result, XMMatrixMultiply(_MATRIX(matrix1), _MATRIX(matrix2)));
		return(xmmtx4x4Result);
	}

	inline Vec4x4 Multiply(const Vec4x4& matrix1, const XMMATRIX& matrix2) noexcept
	{
		Vec4x4 xmmtx4x4Result;
		XMStoreFloat4x4(&xmmtx4x4Result, _MATRIX(matrix1) * matrix2);
		return(xmmtx4x4Result);
	}

	inline Vec4x4 Multiply(const XMMATRIX& matrix1, const Vec4x4& matrix2) noexcept
	{
		Vec4x4 xmmtx4x4Result;
		XMStoreFloat4x4(&xmmtx4x4Result, matrix1 * _MATRIX(matrix2));
		return(xmmtx4x4Result);
	}

	inline Vec4x4 Multiply(const XMMATRIX& matrix1, const XMMATRIX& matrix2) noexcept
	{
		Vec4x4 xmmtx4x4Result;
		XMStoreFloat4x4(&xmmtx4x4Result, matrix1 * matrix2);
		return(xmmtx4x4Result);
	}

	inline Vec3 Multiply(const Vec4x4& matrix, const Vec3& vector) noexcept
	{
		Vec4 v = Vec4(vector.x, vector.y, vector.z, 1.0f);
		Vec3 result;
		XMStoreFloat3(&result, XMVector4Transform(XMLoadFloat4(&v), _MATRIX(matrix)));
		return(result);
	}

	inline Vec4x4 RotationYawPitchRoll(float pitch, float yaw, float roll) noexcept
	{
		Vec4x4 xmmtx4x4Result;
		XMStoreFloat4x4(&xmmtx4x4Result, XMMatrixRotationRollPitchYaw(XMConvertToRadians(pitch), XMConvertToRadians(yaw), XMConvertToRadians(roll)));
		return(xmmtx4x4Result);
	}

	inline Vec4x4 RotationAxis(const Vec3& axis, float angle) noexcept
	{
		Vec4x4 xmmtx4x4Result;
		XMStoreFloat4x4(&xmmtx4x4Result, XMMatrixRotationAxis(XMLoadFloat3(&axis), XMConvertToRadians(angle)));
		return(xmmtx4x4Result);
	}

	inline Vec4x4 Inverse(const Vec4x4& matrix) noexcept
	{
		Vec4x4 xmmtx4x4Result;
		XMStoreFloat4x4(&xmmtx4x4Result, XMMatrixInverse(NULL, _MATRIX(matrix)));
		return(xmmtx4x4Result);
	}

	inline Vec4x4 Transpose(const Vec4x4& matrix) noexcept
	{
		Vec4x4 xmmtx4x4Result;
		XMStoreFloat4x4(&xmmtx4x4Result, XMMatrixTranspose(_MATRIX(matrix)));
		return(xmmtx4x4Result);
	}

	inline Vec4x4 PerspectiveFovLH(float fFovAngleY, float aspectRatio, float fNearZ, float fFarZ) noexcept
	{
		Vec4x4 xmmtx4x4Result;
		XMStoreFloat4x4(&xmmtx4x4Result, XMMatrixPerspectiveFovLH(XMConvertToRadians(fFovAngleY), aspectRatio, fNearZ, fFarZ));
		return(xmmtx4x4Result);
	}

	inline Vec4x4 LookAtLH(const Vec3& eyePos, const Vec3& lookAtPos, const Vec3& upDir) noexcept
	{
		Vec4x4 xmmtx4x4Result;
		XMStoreFloat4x4(&xmmtx4x4Result, XMMatrixLookAtLH(XMLoadFloat3(&eyePos), XMLoadFloat3(&lookAtPos), XMLoadFloat3(&upDir)));
		return(xmmtx4x4Result);
	}

	inline Vec4x4 LookToLH(const Vec3& eyePos, const Vec3& xmf3LookTo, const Vec3& upDir) noexcept
	{
		Vec4x4 xmmtx4x4Result;
		XMStoreFloat4x4(&xmmtx4x4Result, XMMatrixLookToLH(XMLoadFloat3(&eyePos), XMLoadFloat3(&xmf3LookTo), XMLoadFloat3(&upDir)));
		return(xmmtx4x4Result);
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
		return XMMatrixRotationAxis(XMLoadFloat3(&axis), XMConvertToRadians(degree));
	}

	inline Matrix Transpose(const Vec4x4& matrix) noexcept
	{
		return XMMatrixTranspose(_MATRIX(matrix));
	}

	inline void SetPosition(Matrix& matrix, const Vec3& pos)
	{
		::memcpy(&matrix.r[3], &XMVectorSet(pos.x, pos.y, pos.z, 1.f), sizeof(XMVECTOR));
	}
}

namespace Triangle
{
	inline bool Intersect(Vec3& pos, Vec3& dir, Vec3& v0, Vec3& v1, Vec3& v2, float& hitDistance) noexcept
	{
		return(TriangleTests::Intersects(XMLoadFloat3(&pos), XMLoadFloat3(&dir), XMLoadFloat3(&v0), XMLoadFloat3(&v1), XMLoadFloat3(&v2), hitDistance));
	}
}

namespace Plane
{
	inline Vec4 Normalize(Vec4& plane) noexcept
	{
		Vec4 xmf4Result;
		XMStoreFloat4(&xmf4Result, XMPlaneNormalize(XMLoadFloat4(&plane)));
		return(xmf4Result);
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
		XMStoreFloat4(&Orientation, XMQuaternionMultiply(XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), rotation));
		XMStoreFloat3(&Center, XMVector3Transform(XMLoadFloat3(&OriginCenter), matrix));
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
	CreateBufferResource(data, byteSize, heapType, resourceStates, tempUploadBuffer, buffer);
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

	::CreateBufferResource(data.data(), sizeof(DataType) * data.size(), D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, uploadBuffer, buffer);
}


inline void CreateIndexBufferResource(
	
	
	const std::vector<UINT>& indices, 
	ComPtr<ID3D12Resource>& uploadBuffer, 
	ComPtr<ID3D12Resource>& buffer)
{
	::CreateBufferResource(indices.data(), sizeof(UINT) * indices.size(), D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, uploadBuffer, buffer);
}

struct BufferViews {
	ComPtr<ID3D12Resource> vertexBuffer{};
	ComPtr<ID3D12Resource> normalBuffer{};
	ComPtr<ID3D12Resource> UV0Buffer{};
	ComPtr<ID3D12Resource> UV1Buffer{};
	ComPtr<ID3D12Resource> tangentBuffer{};
	ComPtr<ID3D12Resource> biTangentBuffer{};
};

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

void SynchronizeResourceTransition(
	RComPtr<ID3D12Resource> resource,
	D3D12_RESOURCE_STATES stateBefore,
	D3D12_RESOURCE_STATES stateAfter);

//////////////////* Math *//////////////////
inline bool IsZero(float f) { return fabsf(f) < EPSILON; }
inline bool IsEqual(float a, float b) { return ::IsZero(a - b); }
inline float InverseSqrt(float f) { return 1.0f / sqrtf(f); }

inline float RandF(float fMin, float fMax) noexcept
{
	return(fMin + ((float)rand() / (float)RAND_MAX) * (fMax - fMin));
}

inline int sign(float x) noexcept
{
	return (x >= 0.0f) ? 1 : -1;
}
int RoundToNearestMultiple(int value, int multiple);

//////////////////* I/O *//////////////////
int ReadUnityBinaryString(FILE* file, std::string& token);
int ReadIntegerFromFile(FILE* file);
float ReadFloatFromFile(FILE* file);
UINT ReadStringFromFile(FILE* file, std::string& token);

//////////////////* Others *//////////////////
void PrintMessage(const char* message);
void PrintErrorBlob(RComPtr<ID3DBlob> errorBlob);
XMVECTOR RandomUnitVectorOnSphere();
XMVECTOR RandomUnitVectorOnDom();

template<class T>
inline void CopyBack(const std::vector<T>& src, std::vector<T>& dst)
{
	copy(src.begin(), src.end(), std::back_inserter(dst));
}


void LoadTextureNames(std::vector<std::string>& out, const std::string& folder);