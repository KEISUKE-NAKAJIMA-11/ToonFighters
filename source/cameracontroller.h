#pragma once
#include <windows.h>
#include <DirectXMath.h>

class CameraController
{
public:
	CameraController();

	void Update();

	const DirectX::XMFLOAT3& GetEye() const { return eye; }
	const DirectX::XMFLOAT3& GetFocus() const { return focus; }
	const DirectX::XMFLOAT3& GetUp() const { return up; }

private:
	DirectX::XMFLOAT3	eye = DirectX::XMFLOAT3(0, 200, 200);
	DirectX::XMFLOAT3	focus = DirectX::XMFLOAT3(0, 0, 0);
	DirectX::XMFLOAT3	up = DirectX::XMFLOAT3(0, 1, 0);
	DirectX::XMFLOAT3	right = DirectX::XMFLOAT3(1, 0, 0);
	float				distance = 0.0f;

	DirectX::XMFLOAT2	oldCursor;
	DirectX::XMFLOAT2	newCursor;

	float				rotateX = DirectX::XMConvertToRadians(45);
	float				rotateY = 0.0f;
};
