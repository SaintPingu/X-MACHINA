#include "stdafx.h"
#include "Script_Billboard.h"

#include "Component/Camera.h"

void Script_Billboard::Update()
{
	//mObject->LookAt(MainCamera::I->GetPosition());
}

void Script_Billboard::UpdateSpriteVariable(const int matIndex) const
{
	//ObjectConstants objectConstants;
	//objectConstants.MtxWorld = XMMatrixTranspose(XMMatrixMultiply(XMMatrixScaling(mScale, mScale, 1.f), _MATRIX(mObject->GetWorldTransform())));
	//objectConstants.MatIndex = matIndex;

	//// ���� ���� �ִ� ������Ʈ�� ��� ���� �ε����� �����Ѵ�.
	//int objCBIndex = mObject->GetObjCBIndex();

	//// ��� ���ۿ� �����ϰ� objCBIdx�� ����� ��� ���� �ε����� �����Ѵ�.
	//FRAME_RESOURCE_MGR->CopyData(objCBIndex, objectConstants);

	//// ���� ������Ʈ�� ��� ���� �ε����� �������� ���� ��쿡�� ���Ӱ� �����Ѵ�.
	//if (!mObject->GetUseObjCB()) {
	//	mObject->SetObjCBIndex(objCBIndex);
	//}

	//// ��� ���� �� Set
	//DXGIMgr::I->SetGraphicsRootConstantBufferView(RootParam::Object, FRAME_RESOURCE_MGR->GetObjCBGpuAddr(objCBIndex));
}