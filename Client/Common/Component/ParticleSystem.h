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

	std::array<T, N>		Vals{};
	std::array<float, N>	ValKeys{};

	PSVal(T val = {}) { Vals.fill(val), ValKeys.fill(0.f); }

	PSVal& Set(PSValOp option, std::initializer_list<T> vals, std::initializer_list<float> keys = { 1.f }) {
		assert(vals.size() <= N);
		IsOn = true;
		Option = option;
		std::memcpy(Vals.data(), vals.begin(), vals.size() * sizeof(T));

		if (keys.size() > 1)
			std::memcpy(ValKeys.data(), keys.begin(), keys.size() * sizeof(float));

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
		ar& BOOST_SERIALIZATION_NVP(ValKeys);
	}
};


#pragma region Derived_PSVal
struct PSFloat : public PSVal<float, 4> {
	PSFloat() {}
	PSFloat(float val) : PSVal(val) {}
};

struct PSVec4 : public PSVal<Vec4, 4> {
	PSVec4() {}
	PSVec4(Vec4 val) : PSVal(val) {}
};

struct PSColor : public PSVec4 {
	std::array<float, 4> Alphas{};
	std::array<float, 4> AlphaKeys{};

	PSColor() : PSVec4(Vec4{ 1.f }) {}

public:
	PSVal& SetColors(PSValOp option, std::initializer_list<Vec3> vals, std::initializer_list<float> keys = { 1.f }) {
		assert(vals.size() <= 4);
		IsOn = true;
		Option = option;

		int cnt{};
		for (const auto& val : vals)
			Vals[cnt++] = { val.x, val.y, val.z, 1.f };

		if (keys.size() > 1)
			std::memcpy(ValKeys.data(), keys.begin(), keys.size() * sizeof(float));

		return *this;
	}

	PSVal& SetAlphas(std::initializer_list<float> vals, std::initializer_list<float> keys = { 1.f }) {
		std::memcpy(Alphas.data(), vals.begin(), vals.size() * sizeof(float));

		if (keys.size() > 1)
			std::memcpy(AlphaKeys.data(), keys.begin(), keys.size() * sizeof(float));

		return *this;
	}

public:
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version) {
		PSVal<Vec4, 4>::serialize(ar, version);
		ar& BOOST_SERIALIZATION_NVP(Alphas);
		ar& BOOST_SERIALIZATION_NVP(AlphaKeys);
	}
};

#pragma endregion

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
	PSColor				StartColor{};
	float				GravityModifier = 0.f;
	PSSimulationSpace	SimulationSpace = PSSimulationSpace::World;
	float				SimulationSpeed = 1.f;
	bool				PlayOnAwake = false;
	int					MaxParticles = 1000;

	/* unity particle system module */
	Emission			Emission{};
	PSShape 			Shape{};
	PSFloat				SizeOverLifetime{};
	PSVec4				VelocityOverLifetime{};
	PSColor				ColorOverLifetime{};
	PSFloat				RotationOverLifetime{};
	PSRenderer			Renderer{};

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
	PSColor				StartColor{};


	float				GravityModifier{};
	PSSimulationSpace   SimulationSpace{};
	float				SimulationSpeed{};
	float				Duration{};

	PSVec4				VelocityOverLifetime{};
	PSColor				ColorOverLifetime{};
	PSFloat				SizeOverLifetime{};
	PSFloat				RotationOverLifetime{};
	PSShape 			Shape{};
};

class ParticleSystemGPULoadData : public ParticleSystemGPUData, public Resource {
public:
	ParticleSystemGPULoadData() : Resource(ResourceType::ParticleSystemGPUData) { }
	virtual ~ParticleSystemGPULoadData() = default;
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
class ParticleSystem {
	friend class ParticleSystemPool;

private:
	bool				mIsUse = false;
	Transform*				mTarget{};					// 파티클을 부착시킬 타겟
	ParticleSystem*		mNext{};

	int					mPSIdx = -1;				// 파티클 시스템 구조적 버퍼 인덱스
	bool				mIsStopCreation = true;		// 파티클 생성 중지 플래그
	bool				mIsRunning = false;

	float				mAccElapsed = 0.f;
	float				mStopElapsed = 0.f;
	float				mStartElapsed = 0.f;
	float				mLoopingElapsed = 0.f;
	std::vector<float>	mBurstElapseds{};
	std::vector<bool>	mBurstRunnings{};

	sptr<ParticleSystemCPUData>	mPSCD{};			// 모든 파티클에 공통적으로 적용되는 CPU 데이터
	uptr<ParticleSystemGPUData>	mPSGD{};			// 모든 파티클에 개별적으로 적용되는 GPU 데이터

	uptr<UploadBuffer<ParticleData>> mParticles;	// 개별 파티클에 특수적으로 적용되는 GPU 데이터

public:
	ParticleSystem() = default;
	virtual ~ParticleSystem() = default;

#pragma region Getter
	bool IsUse() const { return mIsUse; }
	bool IsRunning() const { return mIsRunning; }
	ParticleSystem* GetNext() const { return mNext; }
	rsptr<ParticleSystemCPUData> GetPSCD() const { return mPSCD; }
#pragma endregion

#pragma region Setter
	void SetNext(ParticleSystem* next) { mNext = next; }
	void SetTarget(Object* target) { mTarget = target; }
#pragma endregion

public:
	void Init();
	ParticleSystem* Play(rsptr<ParticleSystemCPUData> pscd, Transform* target);
	bool Update();
	void Stop();

public:
	void Compute() const;
	void Render() const;
	void ReturnIndex();

public:
	static void SavePSCD(ParticleSystemCPUData& pscd);
	static sptr<ParticleSystemCPUData> LoadPSCD(const std::string& filePath);
	static sptr<ParticleSystemGPULoadData> LoadPSGD(const std::string& pscdName);
};


class ParticleSystemPool {
private:
	static constexpr int mkPoolSize = 500;
	int mUseCount = 0;

	std::array<ParticleSystem, mkPoolSize> mPSs;
	ParticleSystem* mFirstAvailable{};

public:
	ParticleSystemPool();
	virtual ~ParticleSystemPool() = default;

public:
	ParticleSystem* Create(rsptr<ParticleSystemCPUData> pscd, Transform* target);
	void Update();
	void ComputePSs() const;
	void RenderPSs() const;

	bool IsUse() const { return mUseCount >= 1; }
};


class ParticleManager : public Singleton<ParticleManager> {
	friend Singleton;

private:
	sptr<Shader> mCompute{};
	std::array<sptr<Shader>, BlendTypeCount> mShaders;
	std::array<sptr<ParticleSystemPool>, BlendTypeCount> mPSPools;

public:
#pragma region C/Dtor
	ParticleManager() = default;
	virtual ~ParticleManager() = default;
#pragma endregion

public:
	void Init();
	ParticleSystem* Play(const std::string& pscdName, Transform* target);
	void Update();
	void Render() const;
};
#pragma endregion
