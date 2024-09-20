#include "EnginePch.h"
#include "LoginScene.h"
#include "DXGIMgr.h"

#include "Component/UI.h"


LoginScene::LoginScene()
{
	mName = "Login";
}

void LoginScene::Build()
{
	base::Build();

	mManager->SetActive(true);
}

void LoginScene::RenderDeferred()
{
	base::RenderDeferred();

	CMD_LIST->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
