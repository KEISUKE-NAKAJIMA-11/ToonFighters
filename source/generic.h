#pragma once
#include <DirectXMath.h>

#define SAFE_RELEASE(p)  if(p) { (p)->Release(); (p)=nullptr; } 

static float(* const ToRadian)(float) = DirectX::XMConvertToRadians;	 // 角度をラジアンに
static float(* const ToDegree)(float) = DirectX::XMConvertToDegrees;	 // ラジアンを角度に

DirectX::XMFLOAT3 ScreenToNDC(DirectX::XMFLOAT3 screenPos, const int screenWidht, const int screenHeight);
DirectX::XMFLOAT2 ScreenToNDC(DirectX::XMFLOAT2 screenPos, const int screenWidht, const int screenHeight);