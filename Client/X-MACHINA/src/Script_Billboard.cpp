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
	// 다 사용된 경우 상수 버퍼 인덱스를 반환한다.
	frmResMgr->ReturnObjCBIdx(mObject->GetObjCBIdx());
}

void Script_Billboard::UpdateSpriteVariable() const
{
	ObjectConstants objectConstants;
	objectConstants.MtxWorld = XMMatrixTranspose(XMMatrixMultiply(XMMatrixScaling(mScale, mScale, 1.f), _MATRIX(mObject->GetWorldTransform())));

	// 현재 물고 있는 오브젝트의 상수 버퍼 인덱스를 복사한다.
	int objCBIdx = mObject->GetObjCBIdx();

	// 상수 버퍼에 매핑하고 objCBIdx에 사용할 상수 버퍼 인덱스를 저장한다.
	frmResMgr->CopyData(objCBIdx, objectConstants);

	// 현재 오브젝트의 상수 버퍼 인덱스가 설정되지 않은 경우에만 새롭게 설정한다.
	if (!mObject->GetUseObjCB()) {
		mObject->SetUseObjCB(true);
		mObject->SetObjCBIdx(objCBIdx);
	}

	// 상수 버퍼 뷰 Set
	scene->SetGraphicsRootConstantBufferView(RootParam::Object, frmResMgr->GetObjCBGpuAddr(objCBIdx));
}