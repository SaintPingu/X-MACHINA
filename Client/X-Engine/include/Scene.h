#pragma once

class Scene abstract {
public:
	virtual void RenderBegin() abstract;

	virtual void RenderShadow() abstract;
	virtual void RenderDeferred() abstract;
	virtual void RenderLights() abstract;
	virtual void RenderCustomDepth() abstract;
	virtual void RenderForward() abstract;
	virtual void RenderUI() abstract;
	virtual void RenderText(RComPtr<struct ID2D1DeviceContext2> device) abstract;

	virtual void ApplyDynamicContext() abstract;
};