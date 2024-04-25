#pragma once

#pragma region Include
#include "Component/Component.h"
#include "UploadBuffer.h"

#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/array.hpp>

#include "Resources.h"
#pragma endregion


#pragma region ClassForwardDecl
class Shader;
#pragma endregion


#pragma region EnumClass
enum class PSSimulationSpace : UINT32 {
	Local = 0,
	World,
};

enum class PSValOp : UINT32 {
	Constant = 0,
	Curve,
	RandomBetweenTwoConstants,
	RandomBetweenTwoCurve,
};

enum class PSRenderMode : UINT32 {
	None = 0,
	Billboard,
	StretchedBillboard,
	HorizontalBillboard,
	VerticalBillboard,
};

enum class PSShapeType : UINT32 {
	None = 0,
	Sphere,
	HemiSphere,
	Cone,
	Box,
};
#pragma endregion


#pragma region Struct
template <typename T, UINT8 N>
struct PSVal {
	static_assert(std::is_same<T, Vec4>::value || std::is_same<T, float>::value || N % 4 == 0);

	PSValOp					Option = PSValOp::Constant;
	UINT					IsOn = 0;
	float					Param1 = 0.f;
	float					Param2 = 0.f;

	std::array<T, N>		Vals;
	std::array<float, N>	Curves;	

	PSVal(T val = {}) { Vals.fill(val), Curves.fill(0.f); }

	PSVal& Set(PSValOp option, std::initializer_list<T> vals, std::initializer_list<float> curves = { 1.f }) {
		assert(vals.size() <= N);
		IsOn = true;
		Option = option;
		std::memcpy(Vals.data(), vals.begin(), vals.size() * sizeof(T));

		if (curves.size() > 1) {
			std::memcpy(Curves.data(), curves.begin(), curves.size() * sizeof(float));
		}

		return *this;
	}

	void SetParam(float param1 = 0.f, float param2 = 0.f) { Param1 = param1, Param2 = param2; }

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version) {
		ar& BOOST_SERIALIZATION_NVP(Option);
		ar& BOOST_SERIALIZATION_NVP(IsOn);
		ar& BOOST_SERIALIZATION_NVP(Param1);
		ar& BOOST_SERIALIZATION_NVP(Param2);
		ar& BOOST_SERIALIZATION_NVP(Vals);
		ar& BOOST_SERIALIZATION_NVP(Curves);
	}
};


struct Emission {
	struct Burst {
		int		Count = 0;
		float	Time = 0.f;
		float	BurstElapsed = 0.f;
		int		IsRunning = false;

		template<class Archive>
		void serialize(Archive& ar, const unsigned int version) {
			ar& BOOST_SERIALIZATION_NVP(Count);
			ar& BOOST_SERIALIZATION_NVP(Time);
			ar& BOOST_SERIALIZATION_NVP(BurstElapsed);
		}
	};

	bool				IsOn = true;
	int					RateOverTime = 0;
	std::vector<Burst>	Bursts;

public:
	void UpdateDeltaTime(float deltaTime) {
		if (!IsOn)
			return;

		for (auto& burst : Bursts) {
			burst.BurstElapsed += deltaTime;
		}
	}

	void SetBurst(int count, float time = 0.f) {
		Bursts.emplace_back(count, time);
	}

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version) {
		ar& BOOST_SERIALIZATION_NVP(IsOn);
		ar& BOOST_SERIALIZATION_NVP(RateOverTime);
		ar& BOOST_SERIALIZATION_NVP(Bursts);
	}
};

struct PSShape {
	PSShapeType ShapeType = PSShapeType::None;
	Vec3		Padding;
	float		Angle = 25.f;
	float		Radius = 1.f;
	float		RadiusThickness = 1.f;
	float		Arc = 360.f;

	void SetSphere(float radius, float radiusThickness = 1.f, float arc = 360.f, bool isHemiSphere = false) {
		ShapeType = isHemiSphere ? PSShapeType::HemiSphere : PSShapeType::Sphere;
		Radius = radius;
		RadiusThickness = radiusThickness;
		Arc = arc;
	}
	void SetCone(float angle, float radius, float radiusThickness = 1.f, float arc = 360.f) {
		ShapeType = PSShapeType::Cone;
		Angle = angle;
		Radius = radius;
		RadiusThickness = radiusThickness;
		Arc = arc;
	}
	void SetBox() {
		ShapeType = PSShapeType::Box;
	}

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version) {
		ar& BOOST_SERIALIZATION_NVP(ShapeType);
		ar& BOOST_SERIALIZATION_NVP(Angle);
		ar& BOOST_SERIALIZATION_NVP(Radius);
		ar& BOOST_SERIALIZATION_NVP(RadiusThickness);
		ar& BOOST_SERIALIZATION_NVP(Arc);
	}
};


//struct VelocityOverLifetime {
//	UINT	IsOn = false;
//	Vec3	Linear{};
//	float	SpeedModifier{};
//	Vec3	Padding;
//
//public:
//	void SetLinear(const Vec3 linear, float speedModifier = 1.f) {
//		IsOn = true;
//		Linear = linear;
//		SpeedModifier = speedModifier;
//	}
//
//	template<class Archive>
//	void serialize(Archive& ar, const unsigned int version) {
//		ar& BOOST_SERIALIZATION_NVP(IsOn);
//		ar& BOOST_SERIALIZATION_NVP(Linear);
//		ar& BOOST_SERIALIZATION_NVP(SpeedModifier);
//	}
//};

struct PSRenderer {
	std::string		TextureName{};
	PSRenderMode	RenderMode = PSRenderMode::Billboard;
	BlendType		BlendType = BlendType::Alpha_Blend;
	float			LengthScale = 1.f;

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version) {
		ar& BOOST_SERIALIZATION_NVP(TextureName);
		ar& BOOST_SERIALIZATION_NVP(RenderMode);
		ar& BOOST_SERIALIZATION_NVP(BlendType);
		ar& BOOST_SERIALIZATION_NVP(LengthScale);
	}
};

// 같은 파티클 시스템에 공통으로 사용되는 CPU 데이터
class ParticleSystemCPUData : public Resource {
public:
	/* my value */
	std::string			mName{};
	int					MaxAddCount = 0;		// 한번에 생성되는 파티클 개수
	float				PlayMaxTime = 5.f;

	/* Transform */		
	Vec3				Position{};
	int					Padding;

	/* unity particle system */
	float				Duration = 0.05f;
	bool				Looping = false;
	bool				Prewarm = false;
	float				StartDelay = 0.f;
	Vec2				StartLifeTime = Vec2{ 1.f };
	Vec2				StartSpeed = Vec2{ 1.f };
	Vec4				StartSize3D = Vec4{ 1.f, 1.f, 1.f, 0.f};	// w값이 0이면 StartSize3D를 사용하지 않음
	Vec2				StartSize = Vec2{ 0.1f };
	Vec4				StartRotation3D = Vec4{ 0.f, 0.f, 0.f, 0.f };	// w값이 0이면 StartRotation3D를 사용하지 않음
	Vec2				StartRotation = Vec2{ 0.f };
	PSVal<Vec4, 4>		StartColor = Vec4{ 1.f };
	float				GravityModifier = 0.f;
	PSSimulationSpace	SimulationSpace = PSSimulationSpace::World;
	float				SimulationSpeed = 1.f;
	bool				PlayOnAwake = false;
	int					MaxParticles = 1000;

	/* unity particle system module */
	Emission				Emission{};
	PSShape 				Shape{};
	int						SizeOverLifetime{};
	PSVal<Vec4, 4>			VelocityOverLifetime = Vec4{ 0.f };
	PSVal<Vec4, 4>			ColorOverLifetime = Vec4{ 1.f };
	PSVal<float, 4>			RotationOverLifetime{};
	PSRenderer				Renderer{};

public:
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version) {
		ar& BOOST_SERIALIZATION_NVP(mName);
		ar& BOOST_SERIALIZATION_NVP(MaxAddCount);
		ar& BOOST_SERIALIZATION_NVP(Position);
		ar& BOOST_SERIALIZATION_NVP(PlayMaxTime);
		ar& BOOST_SERIALIZATION_NVP(Duration);
		ar& BOOST_SERIALIZATION_NVP(Looping);
		ar& BOOST_SERIALIZATION_NVP(Prewarm);
		ar& BOOST_SERIALIZATION_NVP(StartDelay);
		ar& BOOST_SERIALIZATION_NVP(StartLifeTime);
		ar& BOOST_SERIALIZATION_NVP(StartSpeed);
		ar& BOOST_SERIALIZATION_NVP(StartSize3D);
		ar& BOOST_SERIALIZATION_NVP(StartSize);
		ar& BOOST_SERIALIZATION_NVP(StartRotation3D);
		ar& BOOST_SERIALIZATION_NVP(StartRotation);
		ar& BOOST_SERIALIZATION_NVP(StartColor);
		ar& BOOST_SERIALIZATION_NVP(GravityModifier);
		ar& BOOST_SERIALIZATION_NVP(SimulationSpace);
		ar& BOOST_SERIALIZATION_NVP(SimulationSpeed);
		ar& BOOST_SERIALIZATION_NVP(PlayOnAwake);
		ar& BOOST_SERIALIZATION_NVP(MaxParticles);
		ar& BOOST_SERIALIZATION_NVP(Emission);
		ar& BOOST_SERIALIZATION_NVP(Shape);
		ar& BOOST_SERIALIZATION_NVP(VelocityOverLifetime);
		ar& BOOST_SERIALIZATION_NVP(ColorOverLifetime);
		ar& BOOST_SERIALIZATION_NVP(SizeOverLifetime);
		ar& BOOST_SERIALIZATION_NVP(RotationOverLifetime);
		ar& BOOST_SERIALIZATION_NVP(Renderer);
	}

public:
	ParticleSystemCPUData() : Resource(ResourceType::ParticleSystemCPUData) { }
	virtual ~ParticleSystemCPUData() = default;
};

struct ParticleSystemGPUData {
	Vec3				WorldPos{};
	int					TextureIndex{};
	Vec4				Color{};
						
	int					AddCount{};
	int					MaxParticles{};
	float				DeltaTime{};
	float				TotalTime{};
						
	Vec2				StartLifeTime{};
	Vec2				StartSpeed{};
	Vec4				StartSize3D{};
	Vec4				StartRotation3D{};
	Vec2				StartSize{};
	Vec2				StartRotation{};
	PSVal<Vec4, 4>		StartColor{};


	float				GravityModifier{};
	PSSimulationSpace   SimulationSpace{};
	float				SimulationSpeed{};
	int					SizeOverLifetime{};

	PSVal<Vec4, 4>		VelocityOverLifetime{};
	PSVal<Vec4, 4>		ColorOverLifetime{};
	PSVal<float, 4>		RotationOverLifetime{};
	PSShape 			Shape{};
	float				Duration{};
	Vec3				Padding{};
};

struct ParticleData {
	Vec3	StartPos{};
	float	CurTime{};
	Vec3	LocalPos{};
	float	LifeTime{};
	Vec3	WorldPos{};
	int		Alive{};
	Vec3	WorldDir{};
	int		TextureIndex{};
	Vec3	MoveDir{};
	float	StartSpeed{};
	Vec3	StartRotation{};
	float	Padding1{};
	Vec3	FinalRotation{};
	float   AngularVelocity{};
	Vec2	StartSize{};
	Vec2	FinalSize{};
	Vec4	StartColor{};
	Vec4	FinalColor{};
	Vec4	VelocityOverLifetime{};
};

struct ParticleSharedData {
	int		AddCount{};
	Vec3	Padding{};
};
#pragma endregion


#pragma region Class
/* particle system component */
class ParticleSystem : public Component, public std::enable_shared_from_this<ParticleSystem> {
	COMPONENT(ParticleSystem, Component)

private:
	Transform*			mTarget = mObject;			// 파티클을 부착시킬 타겟
	int					mPSIdx = -1;				// 파티클 시스템 구조적 버퍼 인덱스

	bool				mIsRunning = false;			// 파티클 시스템 동작 플래그
	bool				mIsStopCreation = true;		// 파티클 생성 중지 플래그

	float				mAccElapsed = 0.f;
	float				mStopElapsed = 0.f;
	float				mStartElapsed = 0.f;
	float				mLoopingElapsed = 0.f;
	std::vector<float>	mBurstElapseds{};
	std::vector<bool>	mBurstRunnings{};

	sptr<ParticleSystemCPUData>	mPSCD{};			// 모든 파티클에 공통적으로 적용되는 CPU 데이터
	ParticleSystemGPUData		mPSGD{};			// 모든 파티클에 개별적으로 적용되는 GPU 데이터

	uptr<UploadBuffer<ParticleData>> mParticles;	// 개별 파티클에 특수적으로 적용되는 GPU 데이터

public:
#pragma region Getter
	sptr<ParticleSystemCPUData> GetPSCD() { return mPSCD; }
	int	GetPSIdx() const { return mPSIdx; }
	bool IsRunning() const { return mIsRunning; }
#pragma endregion

#pragma region Setter
	void SetPSCD(sptr<ParticleSystemCPUData> pscd) { mPSCD = pscd; }
	sptr<ParticleSystem> SetTarget(const std::string& frameName);
	void SetSizeByRenderMode(PSRenderMode renderMode);
	void SetColorByBlendType(BlendType blendType);
#pragma endregion

public:
	virtual void Awake() override;

public:
	void UpdateParticleSystem();

	void Play();
	void Stop();
	void Reset();

	void ComputeParticleSystem() const;
	void RenderParticleSystem() const;

	void ReturnIndex();

public:
	sptr<ParticleSystem> Load(const std::string& fileName);

	static void SavePSCD(ParticleSystemCPUData& pscd);
	static sptr<ParticleSystemCPUData> LoadPSCD(const std::string& filePath);
};





class ParticleRenderer : public Singleton<ParticleRenderer> {
	friend Singleton;

private:
	using KeyPSMap = std::unordered_map<int, sptr<ParticleSystem>>;

	std::array<KeyPSMap, BlendTypeCount> mPSs;

	sptr<Shader> mCompute{};
	std::array<sptr<Shader>, BlendTypeCount> mShaders;

	std::array<std::queue<int>, BlendTypeCount> mRemovals;

public:
#pragma region C/Dtor
	ParticleRenderer() = default;
	virtual ~ParticleRenderer() = default;
#pragma endregion

public:
	void Init();
	void AddParticleSystem(BlendType type, sptr<ParticleSystem> particleSystem);
	void RemoveParticleSystem(BlendType type, int particleSystemIdx);

	void Update();
	void Render() const;
};
#pragma endregion
