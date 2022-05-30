#include "blender.h"

void setBlendMode(ID3D11DeviceContext* devicecontext, blender::BLEND_STATE blend, blender* Brender)
{
	devicecontext->OMSetBlendState(Brender->states[blend].Get(), nullptr, 0xFFFFFFFF);

}
