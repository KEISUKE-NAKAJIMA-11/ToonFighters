#include "cameracontroller.h"

#include "CameraController.h"

CameraController::CameraController()
{
	float x = focus.x - eye.x;
	float y = focus.y - eye.y;
	float z = focus.z - eye.z;

	distance = ::sqrtf(x * x + y * y + z * z);
}

// 更新処理
void CameraController::Update()
{
	POINT cursor;
	::GetCursorPos(&cursor);

	oldCursor = newCursor;
	newCursor = DirectX::XMFLOAT2(static_cast<float>(cursor.x), static_cast<float>(cursor.y));

	float move_x = (newCursor.x - oldCursor.x) * 0.02f;
	float move_y = (newCursor.y - oldCursor.y) * 0.02f;

	// Altキー
	if (::GetAsyncKeyState(VK_MENU) & 0x8000)
	{
		if (::GetAsyncKeyState(VK_LBUTTON) & 0x8000)
		{
			// Y軸回転
			rotateY += move_x * 0.5f;
			if (rotateY > DirectX::XM_PI)
			{
				rotateY -= DirectX::XM_2PI;
			}
			else if (rotateY < -DirectX::XM_PI)
			{
				rotateY += DirectX::XM_2PI;
			}
			// X軸回転
			rotateX += move_y * 0.5f;
			if (rotateX > DirectX::XM_PI)
			{
				rotateX -= DirectX::XM_2PI;
			}
			else if (rotateX < -DirectX::XM_PI)
			{
				rotateX += DirectX::XM_2PI;
			}
		}
		else if (::GetAsyncKeyState(VK_MBUTTON) & 0x8000)
		{
			// 平行移動
			float s = distance * 0.035f;
			float x = -move_x * s;
			float y = move_y * s;

			focus.x += right.x * x;
			focus.y += right.y * x;
			focus.z += right.z * x;

			focus.x += up.x * y;
			focus.y += up.y * y;
			focus.z += up.z * y;
		}
		else if (::GetAsyncKeyState(VK_RBUTTON) & 0x8000)
		{
			// ズーム
			distance += (-move_y - move_x) * distance * 0.1f;
		}


		float sx = ::sinf(rotateX);
		float cx = ::cosf(rotateX);
		float sy = ::sinf(rotateY);
		float cy = ::cosf(rotateY);


		DirectX::XMVECTOR front = DirectX::XMVectorSet(-cx * sy, -sx, -cx * cy, 0.0f);
		DirectX::XMVECTOR right = DirectX::XMVectorSet(cy, 0, -sy, 0.0f);
		DirectX::XMVECTOR up = DirectX::XMVector3Cross(right, front);

		DirectX::XMVECTOR focus = DirectX::XMLoadFloat3(&this->focus);
		DirectX::XMVECTOR distance = DirectX::XMVectorSet(this->distance, this->distance, this->distance, 0.0f);
		DirectX::XMVECTOR eye = DirectX::XMVectorSubtract(focus, DirectX::XMVectorMultiply(front, distance));
		//focus = DirectX::XMVectorSubtract(focus, DirectX::XMVectorMultiply(front, distance));

		DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(eye, focus, up);
		view = DirectX::XMMatrixTranspose(view);

		right = DirectX::XMVector3TransformNormal(DirectX::XMVectorSet(1, 0, 0, 0), view);
		up = DirectX::XMVector3TransformNormal(DirectX::XMVectorSet(0, 1, 0, 0), view);
		front = DirectX::XMVector3TransformNormal(DirectX::XMVectorSet(0, 0, 1, 0), view);

		DirectX::XMStoreFloat3(&this->eye, eye);
		//DirectX::XMStoreFloat3(&m_focus, focus);
		DirectX::XMStoreFloat3(&this->up, up);
		DirectX::XMStoreFloat3(&this->right, right);

	}

}
