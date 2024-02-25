#include "stdafx.h"
#include "Script_Billboard.h"

#include "Scene.h"
#include "Camera.h"

#include "DXGIMgr.h"
#include "FrameResource.h"

void Script_Billboard::Update()
{
	mObject->LookAt(mainCameraObject->GetPosition());
}

void Script_Billboard::UpdateSpriteVariable(const int matIndex) const
{
	ObjectConstants objectConstants;
	objectConstants.MtxWorld = XMMatrixTranspose(XMMatrixMultiply(XMMatrixScaling(mScale, mScale, 1.f), _MATRIX(mObject->GetWorldTransform())));
	objectConstants.MatIndex = matIndex;

	// ���� ���� �ִ� ������Ʈ�� ��� ���� �ε����� �����Ѵ�.
	int objCBIndex = mObject->GetObjCBIndex();

	// ��� ���ۿ� �����ϰ� objCBIdx�� ����� ��� ���� �ε����� �����Ѵ�.
	frmResMgr->CopyData(objCBIndex, objectConstants);

	// ���� ������Ʈ�� ��� ���� �ε����� �������� ���� ��쿡�� ���Ӱ� �����Ѵ�.
	if (!mObject->GetUseObjCB()) {
		mObject->SetObjCBIndex(objCBIndex);
	}

	// ��� ���� �� Set
	dxgi->SetGraphicsRootConstantBufferView(RootParam::Object, frmResMgr->GetObjCBGpuAddr(objCBIndex));
}