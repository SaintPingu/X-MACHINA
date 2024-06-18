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
#include <ranges>

/* STL Containers */
#include <span>
#include <array>
#include <vector>
#include <forward_list>
#include <deque>
#include <map>
#include <set>
#include <bitset>
#include <unordered_set>
#include <unordered_map>
#include <queue>
#include <stack>

/* DirectX */
#include <d3d12.h>
#include <dxgidebug.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <D3d12SDKLayers.h>
#include <d3dx12.h>


#include <boost/serialization/nvp.hpp>

/* Custom */
#include "Common.h"
#include <thread>
#include <iostream>

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

// DirectX 함수 hFunctionCall 함수를 평가하여 hResult가 실패했다면 예외를 발생시킨다.
#define THROW_IF_FAILED(hFunctionCall)															\
{																								\
    HRESULT hResult = (hFunctionCall);															\
    std::wstring fileName = AnsiToWString(__FILE__);											\
    if(FAILED(hResult)) { throw DxException(hResult, L#hFunctionCall, fileName, __LINE__); }	\
}

#pragma endregion




#pragma region EnumClass


// 카메라 시점
enum class CameraMode {
	None = -1,
	Third = 0,
};

// [ root parameter index alias ]
// usage:
// index = Scene::I->GetGraphicsRootParamIndex(RootParam);
// 32BitConstant => Scene::I->SetGraphicsRoot32BitConstants(RootParam, ...);
enum class RootParam {
	// Graphics RootParam
	Object = 0,
	Pass,
	SkinMesh,
	PostPass,
	Ssao,
	SsaoBlur,
	Ability,
	Bloom,

	Instancing,
	Material,
	SkyBox,
	Texture,
	Collider,
	Particle,

	// Compute RootParam
	Weight = 0,
	Read,
	LUT0,
	LUT1,
	Write,

	// Particle
	ParticleSystem = 0,
	ParticleShared,
	ComputeParticle,
	ParticleIndex,
};

enum class TextureMap : UINT8 {
	DiffuseMap0 = 0,
	DiffuseMap1,
	DiffuseMap2,
	DiffuseMap3,

	NormalMap,
	EmissiveMap,
	MetallicMap,
	OcclusionMap,

	_count
};
enum { TextureMapCount = static_cast<UINT8>(TextureMap::_count) };

enum class GBuffer : UINT8 {
	Position = 0,
	Normal,
	Diffuse,
	Emissive,
	MetallicSmoothness,
	Occlusion,

	_count
};
enum { GBufferCount = static_cast<UINT8>(GBuffer::_count) };

enum class Lighting : UINT8 {
	Diffuse = 0,
	Specular,
	Ambient,

	_count
};
enum { LightingCount = static_cast<UINT8>(Lighting::_count) };

enum class SsaoMap : UINT8 {
	Ssao0 = 0,
	Ssao1,

	_count
};
enum { SsaoCount = static_cast<UINT8>(SsaoMap::_count) };

enum class BloomType : UINT8 {
	Luminance = 0,
	DownSampling0,
	DownSampling1,
	DownSampling2,
	Blur0_h,
	Blur0_v,
	UpSampling0,
	Blur1_h,
	Blur1_v,
	UpSampling1,
	Blur2_h,
	Blur2_v,

	_count
};
enum { BloomCount = static_cast<UINT8>(BloomType::_count) };

enum {
	DownSamplingCount = 3,
	UpSamplingCount = 2,
};

enum class GroupType : UINT8 {
	SwapChain = 0,
	Shadow,
	GBuffer,
	Lighting,
	OffScreen,
	Ssao,
	CustomDepth,
	Bloom,
	DynamicEnvironment,

	_count
};
enum { MRTGroupTypeCount = static_cast<UINT8>(GroupType::_count) };
#pragma endregion


#pragma region Variable
constexpr int gkDescHeapCbvCount = 0;
constexpr int gkDescHeapSrvCount = 1024;
constexpr int gkDescHeapUavCount = 256;
constexpr int gkDescHeapSkyBoxCount = 16;
constexpr int gkDescHeapDsvCount = 8;

constexpr int gkMaxSceneLight = 32;	// 씬에 존재할 수 있는 조명의 최대 개수. Light.hlsl과 동일해야 한다.
constexpr int gkSkinBoneSize = 128;
#pragma endregion


#pragma region Struct
struct WindowInfo {
	HWND Hwnd;
	short Width;
	short Height;
};

struct VertexBufferViews {
	ComPtr<ID3D12Resource> VertexBuffer{};
	ComPtr<ID3D12Resource> NormalBuffer{};
	ComPtr<ID3D12Resource> UV0Buffer{};
	ComPtr<ID3D12Resource> TangentBuffer{};
	ComPtr<ID3D12Resource> BiTangentBuffer{};
	ComPtr<ID3D12Resource> BoneIndexBuffer{};
	ComPtr<ID3D12Resource> BoneWeightBuffer{};
};

struct LightInfo {
	Vec3	Strength = { 0.5f, 0.5f, 0.5f };
	float	FalloffStart = 1.f;					 // point/spot light only
	Vec3	Direction = { 0.f, -1.f, 0.f };   // directional/spot light only
	float	FalloffEnd = 10.f;				 // point/spot light only
	Vec3	Position = { 0.f, 0.f, 0.f };	 // point light only
	float	SpotPower = 64.f;				 // spot light only
	int		Type = 0;
	int		IsEnable = 0;
	int		ObjCBIndex = -1;
	int 	Padding{};
};

// must be matched with Light.hlsl LightInfo
struct LightLoadInfo {
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

struct SceneLoadLight {
	std::array<LightLoadInfo, gkMaxSceneLight> Lights{};
};

struct SceneLight {
	std::array<LightInfo, gkMaxSceneLight> Lights{};
	std::array<sptr<class ModelObjectMesh>, gkMaxSceneLight> VolumeMeshes{};
};
#pragma endregion


#pragma region Function
void PrintMsgBox(const char* message);

// print error msg if blob has error
void PrintErrorBlob(RComPtr<ID3DBlob> errorBlob);

template<class T>
// copy src's data to end of dst
inline void CopyBack(const std::vector<T>& src, std::vector<T>& dst)
{
	std::copy(src.begin(), src.end(), std::back_inserter(dst));
}

// assert if hResult is failed
inline void AssertHResult(HRESULT hResult)
{
	if (!SUCCEEDED(hResult)) {
		assert(0);
	}
}

// string 형식의 ANSI 문자열을 wstring으로 변환
inline std::wstring AnsiToWString(const std::string& str)
{
	WCHAR buffer[512];
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buffer, 512);
	return std::wstring(buffer);
}

#pragma endregion


#pragma region NameSpace
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

	ComPtr<ID3DBlob> ReadCompiledShaderFile(
		const std::string& fileName);

	inline UINT CalcConstantBuffSize(UINT byteSize)
	{
		// 상수 버퍼의 크기는 하드웨어의 최소 메모리 할당 크기에 배수(256배수)가 되어야 한다.
		// 따라서, 255를 더하고 256보다 작은 모든 비트를 제거한다.
		return (byteSize + 255) & ~255;
	}
}

namespace Filter
{
	static constexpr UINT mMaxBlurRadius = 5;

	inline std::vector<float> CalcGaussWeights(float sigma)
	{
		float twoSigma2 = 2.0f * sigma * sigma;

		int blurRadius = (int)ceil(2.0f * sigma);

		assert(blurRadius <= mMaxBlurRadius);

		std::vector<float> weights;
		weights.resize(2 * blurRadius + 1);

		float weightSum = 0.0f;

		for (int i = -blurRadius; i <= blurRadius; ++i)
		{
			float x = (float)i;

			weights[i + blurRadius] = expf(-x * x / twoSigma2);

			weightSum += weights[i + blurRadius];
		}

		for (int i = 0; i < weights.size(); ++i)
		{
			weights[i] /= weightSum;
		}

		return weights;
	}
}
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


namespace boost {
	namespace serialization {
		template<class Archive>
		void serialize(Archive& ar, Vec2& s, const unsigned int version) {
			ar& BOOST_SERIALIZATION_NVP(s.x);
			ar& BOOST_SERIALIZATION_NVP(s.y);
		}

		template<class Archive>
		void serialize(Archive& ar, Vec3& s, const unsigned int version) {
			ar& BOOST_SERIALIZATION_NVP(s.x);
			ar& BOOST_SERIALIZATION_NVP(s.y);
			ar& BOOST_SERIALIZATION_NVP(s.z);
		}

		template<class Archive>
		void serialize(Archive& ar, Vec4& s, const unsigned int version) {
			ar& BOOST_SERIALIZATION_NVP(s.x);
			ar& BOOST_SERIALIZATION_NVP(s.y);
			ar& BOOST_SERIALIZATION_NVP(s.z);
			ar& BOOST_SERIALIZATION_NVP(s.w);
		}
	}
}

#pragma endregion