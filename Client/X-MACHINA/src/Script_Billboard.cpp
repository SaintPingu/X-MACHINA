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

void Script_Billboard::OnDestroy()
{
	// �� ���� ��� ��� ���� �ε����� ��ȯ�Ѵ�.
	frmResMgr->ReturnObjCBIdx(mObject->GetObjCBIdx());
}

void Script_Billboard::UpdateSpriteVariable() const
{
	ObjectConstants objectConstants;
	objectConstants.MtxWorld = XMMatrixTranspose(XMMatrixMultiply(XMMatrixScaling(mScale, mScale, 1.f), _MATRIX(mObject->GetWorldTransform())));

	// ���� ���� �ִ� ������Ʈ�� ��� ���� �ε����� �����Ѵ�.
	int objCBIdx = mObject->GetObjCBIdx();

	// ��� ���ۿ� �����ϰ� objCBIdx�� ����� ��� ���� �ε����� �����Ѵ�.
	frmResMgr->CopyData(objCBIdx, objectConstants);

	// ���� ������Ʈ�� ��� ���� �ε����� �������� ���� ��쿡�� ���Ӱ� �����Ѵ�.
	if (!mObject->GetUseObjCB()) {
		mObject->SetUseObjCB(true);
		mObject->SetObjCBIdx(objCBIdx);
	}

	// ��� ���� �� Set
	scene->SetGraphicsRootConstantBufferView(RootParam::Object, frmResMgr->GetObjCBGpuAddr(objCBIdx));
}