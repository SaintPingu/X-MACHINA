#include "stdafx.h"
#include "Script_LobbyManager.h"

#include "Script_LobbyUI.h"

#include "X-Engine.h"
#include "InputMgr.h"
#include "SoundMgr.h"
#include "Timer.h"
#include "LobbyScene.h"
#include "Light.h"
#include "Object.h"
#include "LobbyScene.h"
#include "GameFramework.h"
#include "Animator.h"
#include "AnimatorController.h"
#include "AnimatorMotion.h"
#include "ResourceMgr.h"
#include "Texture.h"
#include "TextMgr.h"

#include "Component/UI.h"
#include "Component/Camera.h"
#include "Component/ParticleSystem.h"

#include "ClientNetwork/Contents/FBsPacketFactory.h"
#include "ClientNetwork/Contents/ClientNetworkManager.h"


LobbyPlayer::LobbyPlayer(const LobbyPlayerInfo& info, unsigned char idx)
	:
	mInfo(info),
	mMaxLookAroundDelay(5.f),
	mIndex(idx)
{
	// <pos, rot>
	static const std::array<std::pair<Vec3, float>, 3> kTransforms {
		std::make_pair(Vec3(3.33f, 0.02f, 20.223f), 35.823f),
		std::make_pair(Vec3(4.021297f, 0.02f, 18.21149f), 18.933f),
		std::make_pair(Vec3(1.720729f, 0.02f, 19.2602f), 24.906f),
	};

	if (idx >= kTransforms.size()) {
		return;
	}

	const auto& transform = kTransforms[idx];
	mObject = LobbyScene::I->Instantiate("EliteTrooper", transform.first);
	mObject->SetLocalRotation(Vec3(0, transform.second, 0));
	mController = mObject->GetAnimator()->GetController();
	const auto& motion = mController->FindMotionByName("IdleLookAroundUnarmed", "Body");
	motion->AddCallback(std::bind(&LobbyPlayer::LookAroundEndCallback, this), 150);

	if (Engine::I->GetCrntScene() == LobbyScene::I.get()) {
		Init();
	}
}

void LobbyPlayer::SetSkin(TrooperSkin skin)
{
	mInfo.Skin = skin;
	Transform* transform = mObject->FindFrame("SK_EliteTrooper");
	switch (skin) {
	case TrooperSkin::Army:
		transform->SetTexture(RESOURCE<Texture>("T_EliteTrooper_Army_BaseColor"));
		break;
	case TrooperSkin::Outline:
		transform->SetTexture(RESOURCE<Texture>("T_EliteTrooper_Outline_BaseColor"));
		break;
	case TrooperSkin::Red:
		transform->SetTexture(RESOURCE<Texture>("T_EliteTrooper_Red_BaseColor"));
		break;
	case TrooperSkin::Yellow:
		transform->SetTexture(RESOURCE<Texture>("T_EliteTrooper_Yellow_BaseColor"));
		break;
	case TrooperSkin::Orange:
		transform->SetTexture(RESOURCE<Texture>("T_EliteTrooper_Orange_BaseColor"));
		break;
	case TrooperSkin::Pink:
		transform->SetTexture(RESOURCE<Texture>("T_EliteTrooper_Pink_BaseColor"));
		break;
	case TrooperSkin::Skyblue:
		transform->SetTexture(RESOURCE<Texture>("T_EliteTrooper_Skyblue_BaseColor"));
		break;
	case TrooperSkin::White:
		transform->SetTexture(RESOURCE<Texture>("T_EliteTrooper_White_BaseColor"));
		break;
	case TrooperSkin::Purple:
		transform->SetTexture(RESOURCE<Texture>("T_EliteTrooper_Purple_BaseColor"));
		break;
	case TrooperSkin::Dark:
		transform->SetTexture(RESOURCE<Texture>("T_EliteTrooper_Dark_BaseColor"));
		break;
	case TrooperSkin::Desert:
		transform->SetTexture(RESOURCE<Texture>("T_EliteTrooper_DesertCamo_BaseColor"));
		break;
	case TrooperSkin::Forest:
		transform->SetTexture(RESOURCE<Texture>("T_EliteTrooper_ForestCamo_BaseColor"));
		break;
	case TrooperSkin::Winter:
		transform->SetTexture(RESOURCE<Texture>("T_EliteTrooper_WinterCamo_BaseColor"));
		break;
	default:
		assert(0);
		break;
	}
	mMatIndex = transform->GetMatIndex();
}

void LobbyPlayer::Init()
{
	static const std::array<Vec2, 3> kTextPoistions{
		Vec2(320, 98),
		Vec2(106, 85),
		Vec2(623, 86),
	};

	{
		TextOption textOption{};
		textOption.Font = "Malgun Gothic";
		textOption.FontSize = 18.f;
		textOption.FontColor = TextFontColor::Type::White;
		textOption.FontWeight = TextFontWeight::DEMI_BOLD;

		mTextBox = TextMgr::I->CreateText(mInfo.Name, kTextPoistions[mIndex], textOption);
	}
}

void LobbyPlayer::Update()
{
	mCurLookAroundDelay += DeltaTime();
	if (mCurLookAroundDelay >= mMaxLookAroundDelay) {
		mCurLookAroundDelay = 0.f;
		if (rand() % 2) {
			mController->SetValue("LookAround", true);
		}
	}
}

void LobbyPlayer::Remove()
{
	LobbyScene::I->RemoveSkinMeshObject(mObject);
	TextMgr::I->RemoveTextBox(mTextBox);
}

void LobbyPlayer::LookAroundEndCallback()
{
	mController->SetValue("LookAround", false);
}



void Script_LobbyManager::Awake()
{
	base::Awake();

	MainCamera::I->Awake();
	MainCamera::I->SetPosition(Vec3(3.960061f, 1.980184f, 23.99316f));
	MainCamera::I->SetLocalRotation(Vec3(4.813f, 173.318f, 0.f));
	MainCamera::I->MoveForward(1.f);
	MAIN_CAMERA->SetProjMtx(0.01f, 200.f, 70.f);

	mLobbyUI = mObject->AddComponent<Script_LobbyUI>().get();
	mLobbyUI->SetLobbyManager(this);

	mPlayerRotationBound = Canvas::I->CreateUI<Button>(0, "Image", Vec2(310, -220), Vec2(270, 560));
	mPlayerRotationBound->SetOpacity(0.f);

	{
		mGBR = LobbyScene::I->Instantiate("GBR", Vec3(5.96f, 3.78f, 5.34f));
		//mGBR = LobbyScene::I->Instantiate("GBR", Vec3(4.021297f, 0.02f, 18.21149f));
		//mGBR->SetLocalRotation(Vec3(0, 24.9f, 0));
		mGBRController = mGBR->GetAnimator()->GetController();
		mGBRController->FindMotionByName("RunBackwardsCombat")->AddEndCallback(std::bind(&Script_LobbyManager::GBREndCallback, this));
		mGBRController->FindMotionByName("RunBackwardsLeftCombat")->AddEndCallback(std::bind(&Script_LobbyManager::GBREndCallback, this));
		mGBRController->FindMotionByName("RunBackwardsRightCombat")->AddEndCallback(std::bind(&Script_LobbyManager::GBREndCallback, this));
		mGBRController->FindMotionByName("RunForwardCombat")->AddEndCallback(std::bind(&Script_LobbyManager::GBREndCallback, this));
		mGBRController->FindMotionByName("RunForwardRightCombat")->AddEndCallback(std::bind(&Script_LobbyManager::GBREndCallback, this));
		mGBRController->FindMotionByName("RunLeftCombat")->AddEndCallback(std::bind(&Script_LobbyManager::GBREndCallback, this));
	}

	for (const auto& [id, lobbyPlayer] : mLobbyPlayers) {
		lobbyPlayer->Init();
	}
}

void Script_LobbyManager::Start()
{
	base::Start();

	LobbyScene::I->GetLight()->SetSunlightDir(Vec3(-0.3f, -0.6f, -0.6f));
	LobbyScene::I->GetLight()->SetSunlightColor(Vector3::One * 0.52f);

	SoundMgr::I->Play("BGM", "Lobby", 1.0f, true);
}

void Script_LobbyManager::Update()
{
	base::Update();

	mGBR->Rotate(0, 20 * DeltaTime(), 0);
	for (const auto& [id, lobbyPlayer] : mLobbyPlayers) {
		lobbyPlayer->Update();
	}

	if (mLobbyUI->IsInCustom()) {
		if (mPlayerRotationBound->IsHover()) {
			if (KEY_TAP(VK_LBUTTON)) {
				mIsInPlayerRotation = true;
				mPlayerRortationMouseStartX = InputMgr::I->GetMousePos().x;
			}
		}
	}

	if (mIsInPlayerRotation) {
		if (KEY_AWAY(VK_LBUTTON)) {
			mIsInPlayerRotation = false;
		}
		else {
			constexpr float kRotationSpeed = 0.01f;
			mLobbyPlayers[GameFramework::I->GetMyPlayerID()]->GetObj()->Rotate(0, (mPlayerRortationMouseStartX - InputMgr::I->GetMousePos().x) * kRotationSpeed, 0);
		}
	}
}


void Script_LobbyManager::AddPlayer(const LobbyPlayerInfo& info)
{
	if (mLobbyPlayers.count(info.ID)) {
		return;
	}


	if (info.ID == GameFramework::I->GetMyPlayerID()) {
		mLobbyPlayers[info.ID] = std::make_shared<LobbyPlayer>(info, 0);
	}
	else {
		mLobbyPlayers[info.ID] = std::make_shared<LobbyPlayer>(info, 1 + mCurRemotePlayerSize++);
	}

	mLobbyPlayers[info.ID]->SetSkin(TrooperSkin::Army);
}

void Script_LobbyManager::RemovePlayer(UINT32 id)
{
	if (!mLobbyPlayers.count(id)) {
		return;
	}

	mLobbyPlayers[id]->Remove();
	mLobbyPlayers.erase(id);

	--mCurRemotePlayerSize;
}

void Script_LobbyManager::ChagneToNextSkin()
{
	if (++mCurSkinIdx >= static_cast<int>(TrooperSkin::_count)) {
		mCurSkinIdx = 0;
	}
	
	ChangeSkin(GameFramework::I->GetMyPlayerID(), static_cast<TrooperSkin>(mCurSkinIdx));
}

void Script_LobbyManager::ChagneToPrevSkin()
{
	if (--mCurSkinIdx < 0) {
		mCurSkinIdx = static_cast<int>(TrooperSkin::_count) - 1;
	}
	
	ChangeSkin(GameFramework::I->GetMyPlayerID(), static_cast<TrooperSkin>(mCurSkinIdx));
}

void Script_LobbyManager::ChangeSkin(UINT32 id, const std::string& skinName)
{
	TrooperSkin skin;
	switch (Hash(skinName)) {
	case Hash("Army"):
		skin = TrooperSkin::Army;
		break;
	case Hash("Outline"):
		skin = TrooperSkin::Outline;
		break;
	case Hash("Red"):
		skin = TrooperSkin::Red;
		break;
	case Hash("Yellow"):
		skin = TrooperSkin::Yellow;
		break;
	case Hash("Orange"):
		skin = TrooperSkin::Orange;
		break;
	case Hash("Pink"):
		skin = TrooperSkin::Pink;
		break;
	case Hash("Skyblue"):
		skin = TrooperSkin::Skyblue;
		break;
	case Hash("White"):
		skin = TrooperSkin::White;
		break;
	case Hash("Purple"):
		skin = TrooperSkin::Purple;
		break;
	case Hash("Dark"):
		skin = TrooperSkin::Dark;
		break;
	case Hash("Desert"):
		skin = TrooperSkin::Desert;
		break;
	case Hash("Forest"):
		skin = TrooperSkin::Forest;
		break;
	case Hash("Winter"):
		skin = TrooperSkin::Winter;
		break;
	default:
		assert(0);
		break;
	}

	ChangeSkin(id, skin);
}

void Script_LobbyManager::ChangeSkin(UINT32 id, TrooperSkin skin)
{
	if (!mLobbyPlayers.count(id)) {
		return;
	}

	mLobbyPlayers[id]->SetSkin(skin);

	if (id == GameFramework::I->GetMyPlayerID()) {
		auto cpkt = FBS_FACTORY->CPkt_Custom(GetSkinName());
		CLIENT_NETWORK->Send(cpkt);
	}
}

std::string Script_LobbyManager::GetSkinName() const
{
	switch (static_cast<TrooperSkin>(mCurSkinIdx)) {
	case TrooperSkin::Army:
		return "Army";
	case TrooperSkin::Outline:
		return "Outline";
	case TrooperSkin::Red:
		return "Red";
	case TrooperSkin::Yellow:
		return "Yellow";
	case TrooperSkin::Orange:
		return "Orange";
	case TrooperSkin::Pink:
		return "Pink";
	case TrooperSkin::Skyblue:
		return "Skyblue";
	case TrooperSkin::White:
		return "White";
	case TrooperSkin::Purple:
		return "Purple";
	case TrooperSkin::Dark:
		return "Dark";
	case TrooperSkin::Desert:
		return "Desert";
	case TrooperSkin::Forest:
		return "Forest";
	case TrooperSkin::Winter:
		return "Winter";
	default:
		assert(0);
		break;
	}

	return "";
}

void Script_LobbyManager::GBREndCallback()
{
	if (++mGBRState > 5) {
		mGBRState = 0;
	}
	mGBRController->SetValue("State", mGBRState);
}

void Script_LobbyManager::ChangeToBattleScene()
{
	Engine::I->LoadScene(SceneType::Battle);
}