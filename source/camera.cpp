#include	"camera.h"
#include	"game_scene.h"
#include "judge_manager.h"


#define ToRadian(x)     DirectX::XMConvertToRadians(x)  // 角度をラジアンに

/*******************************************************************************
	「カメラ」クラスのコンストラクタ
*******************************************************************************/
Camera::Camera()
{
	pos = DirectX::XMFLOAT3(.0f, 130.0f, -530.f);
	target = DirectX::XMFLOAT3(.0f, 140.0f, .0f);

	float	fov = DirectX::XMConvertToRadians(400.0f);
	float	aspect = (float)framework::SCREEN_WIDTH / (float)framework::SCREEN_HEIGHT;
	SetPerspectiveMatrix(fov, aspect,  5.0f, 5000.0f);

	mode = MODE::WATCH;
	timer = 0;
}


/*******************************************************************************
	平行投影行列を算出する関数
*******************************************************************************/
DirectX::XMMATRIX	Camera::SetOrthographicMatrix(float w, float h, float znear, float zfar)
{
	projection = DirectX::XMMatrixOrthographicLH(w, h, znear, zfar);
	return	projection;
}


/*******************************************************************************
	透視投影行列を算出する関数
*******************************************************************************/
DirectX::XMMATRIX	Camera::SetPerspectiveMatrix(float fov, float aspect, float znear, float zfar)
{
	projection = DirectX::XMMatrixPerspectiveFovLH(fov, aspect, znear, zfar);
	return	projection;
}


/*******************************************************************************
	ビュー行列を取得する関数
*******************************************************************************/
DirectX::XMMATRIX	Camera::GetViewMatrix()
{

	DirectX::XMVECTOR	p = DirectX::XMVectorSet(pos.x, pos.y, pos.z, 1.0f);
	DirectX::XMVECTOR	t = DirectX::XMVectorSet(target.x, target.y, -target.z, 1.0f);
	DirectX::XMVECTOR	up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	return		DirectX::XMMatrixLookAtLH(p, t, up);
}


/*******************************************************************************
	カメラ用更新関数の実装
*******************************************************************************/
void	Camera::Update()
{
	
	//	カメラの更新
	//float tgy = 0.0f;
	//float posy = 0.0f;

	//tgy = ;
	


#ifdef USE_IMGUI
	if (framework::Instance().useimgui == false) return;
	ImGui::Begin("camera");
	ImGui::Text("camera.x:%f", pos.x);
	ImGui::Text("camera.y:%f", pos.y);
	ImGui::Text("camera.z:%f", pos.z);
	ImGui::SliderFloat("camera.y", &pos.y, .0f, 2000.0f);
	ImGui::SliderFloat("camera.x", &pos.x, .0f, 1000.0f);
	ImGui::SliderFloat("camera.z", &pos.z, -2000.0f, 0.0f);
	ImGui::SliderFloat("target.x", &target.x, .0f, 1000.0f);
	ImGui::SliderFloat("target.y", &target.y, .0f, 1000.0f);
	ImGui::SliderFloat("target.z", &target.z, .0f, 1000.0f);

	ImGui::End();
#endif;
}

void Camera::Update(DirectX::XMFLOAT3 pos1, DirectX::XMFLOAT3 pos2,bool hit1p, bool hit2p)
{

    VECTOR3 at = VECTOR3(target.x, target.y, target.z) - VECTOR3(pos.x, pos.y, pos.z);

	vec3Normalize(at, &at);
	this->at = DirectX::XMFLOAT3(at.x,at.y,at.z);


	//if (UI_Manager::Instance().Get_1pwinflag())
	//{
	//	timer++;
	//	if (timer >= (60 * 2))
	//	{
	//		WinZoom1p(pos1, pos2);
	//	}
	//	
	//}
	//else if (UI_Manager::Instance().Get_2pwinflag())
	//{
	//	timer++;
	//	if (timer >= (60 * 2))
	//	{
	//		WinZoom2p(pos1, pos2);
	//	}
	//}
	//else
	{
		

		if (hit1p||hit2p)
		{
			CameraShake(0, 720, 15, camera_shake_time);
			if (camera_shake_time <= 15) camera_shake_time++;
			else
			{
				
				camera_shake_time = 0;
			}
		}
		
			Chase(pos1, pos2);
		
	}
#ifdef USE_IMGUI
	if (framework::Instance().useimgui == false) return ;
	ImGui::Begin("camera");
	ImGui::Text("camera.x:%f", pos.x);
	ImGui::Text("camera.y:%f", pos.y);
	ImGui::Text("camera.z:%f", pos.z);
	ImGui::SliderFloat("camera.y", &pos.y, .0f, 2000.0f);
	ImGui::SliderFloat("camera.x", &pos.x, .0f, 1000.0f);
	ImGui::SliderFloat("camera.z", &pos.z, -2000.0f, 0.0f);
	ImGui::SliderFloat("target.x", &target.x, .0f, 1000.0f);
	ImGui::SliderFloat("target.y", &target.y, .0f, 1000.0f);
	ImGui::SliderFloat("target.z", &target.z, .0f, 1000.0f);

	ImGui::End();
#endif;
}



/*******************************************************************************
	「監視カメラ」の実装
*******************************************************************************/
void	Camera::Watch()
{

}
/*******************************************************************************
	「相対位置固定カメラ」の実装
*******************************************************************************/
void	Camera::Directional()
{

}

/*******************************************************************************
	「3人称カメラ」の実装
*******************************************************************************/
void	Camera::Tps()
{

}
/*******************************************************************************
	「1人称カメラ」の実装
*******************************************************************************/
void	Camera::Fps()
{

}
/*******************************************************************************
	「追跡カメラ」の実装
*******************************************************************************/

void Camera::Chase(DirectX::XMFLOAT3 pos1, DirectX::XMFLOAT3 pos2)
{
	const float	Distance = 130.0f;
	DirectX::XMFLOAT3 p = {};

	if (pos1.y >=pos2.y)
	{
		p.y = pos1.y;
	}

	else
	{
		p.y = pos2.y;
	}

	pos.z = -530;

	//pos.x = pos1.x;
	pos.y = (p.y + Distance) * 0.7f;
	//pos.y *= -1;

	standard_pos = pos;

	target = pos;
}

void Camera::WinZoom1p(DirectX::XMFLOAT3 pos1, DirectX::XMFLOAT3 pos2)
{
	const float	Distance = 200.0f;
	float d = judge_manager::Instance().get_1pdistance();
	DirectX::XMFLOAT3 p = {};
	
	
	if (d < 0)
	{
		if (target.x <= 160.0f)
		{
			target.x += 10.0f;
		}

		pos.x = pos1.x - 300.0f;
		
	}

	else
	{
		if (target.x >= -160.0f)
		{
			target.x -= 10.0f;
		}

		pos.x = pos1.x + 300.0f;
	}
	
	if (pos.z<=0)
	{
		pos.z += 10;
	}


	//pos.x = pos1.x;
	pos.y = (Distance) * 0.7f;
	//pos.y *= -1;

	target.z = pos.z;
	target.y = pos.y;
}

void Camera::WinZoom2p(DirectX::XMFLOAT3 pos1, DirectX::XMFLOAT3 pos2)
{
	const float	Distance = 200.0f;
	float d = judge_manager::Instance().get_2pdistance();
	DirectX::XMFLOAT3 p = {};


	if (d < 0)
	{
		if (target.x <= 160.0f)
		{
			target.x += 10.0f;
		}

		pos.x = pos2.x - 300.0f;

	}

	else
	{
		if (target.x >= -160.0f)
		{
			target.x -= 10.0f;
		}

		pos.x = pos2.x + 300.0f;
	}

	if (pos.z <= 0)
	{
		pos.z += 10;
	}


	//pos.x = pos1.x;
	pos.y = (Distance) * 0.7f;
	//pos.y *= -1;

	standard_pos = pos;

	target.z = pos.z;
	target.y = pos.y;
}


void Camera::CameraShake(float min, float max, float total_time, float time)
{
	
	float l_angle = InQuad(time, total_time, max, min);
	float l_speed = InQuad(time, total_time, 0, 1.5f);
	pos.x = standard_pos.x + sinf(ToRadian(l_angle)) * l_speed;
	target.x = standard_pos.x + sinf(ToRadian(l_angle)) * l_speed;
}

/*******************************************************************************
	「カメラ」クラスのコンストラクタ
*******************************************************************************/
MapCamera::MapCamera()
{
	pos = DirectX::XMFLOAT3(.0f, 130.0f, -530.f);
	target = DirectX::XMFLOAT3(.0f, 140.0f, .0f);

	float	fov = DirectX::XMConvertToRadians(400.0f);
	float	aspect = (float)framework::SCREEN_WIDTH / (float)framework::SCREEN_HEIGHT;
	SetPerspectiveMatrix(fov, aspect, 5.0f, 5000.0f);

	mode = MODE::WATCH;
	
}


/*******************************************************************************
	平行投影行列を算出する関数
*******************************************************************************/
DirectX::XMMATRIX	MapCamera::SetOrthographicMatrix(float w, float h, float znear, float zfar)
{
	projection = DirectX::XMMatrixOrthographicLH(w, h, znear, zfar);
	return	projection;
}


/*******************************************************************************
	透視投影行列を算出する関数
*******************************************************************************/
DirectX::XMMATRIX	MapCamera::SetPerspectiveMatrix(float fov, float aspect, float znear, float zfar)
{
	projection = DirectX::XMMatrixPerspectiveFovLH(fov, aspect, znear, zfar);
	return	projection;
}


/*******************************************************************************
	ビュー行列を取得する関数
*******************************************************************************/
DirectX::XMMATRIX	MapCamera::GetViewMatrix()
{
	//target = player.getPL()->position;
	DirectX::XMVECTOR	p = DirectX::XMVectorSet(pos.x, pos.y, pos.z, 1.0f);
	DirectX::XMVECTOR	t = DirectX::XMVectorSet(target.x, target.y, -target.z, 1.0f);
	DirectX::XMVECTOR	up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);


	float x = target.x - pos.x;
	float y = target.y - pos.y;
	float z = target.z - pos.z;

	distance = ::sqrtf(x * x + y * y + z * z);

	return		DirectX::XMMatrixLookAtLH(p, t, up);
}


/*******************************************************************************
	カメラ用更新関数の実装
*******************************************************************************/
void	MapCamera::Update()
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

			target.x += right.x * x;
			target.y += right.y * x;
			target.z += right.z * x;

			target.x += up.x * y;
			target.y += up.y * y;
			target.z += up.z * y;
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

		DirectX::XMVECTOR focus = DirectX::XMLoadFloat3(&this->target);
		DirectX::XMVECTOR distance = DirectX::XMVectorSet(this->distance, this->distance, this->distance, 0.0f);
		DirectX::XMVECTOR eye = DirectX::XMVectorSubtract(focus, DirectX::XMVectorMultiply(front, distance));
		//focus = DirectX::XMVectorSubtract(focus, DirectX::XMVectorMultiply(front, distance));

		DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(eye, focus, up);
		view = DirectX::XMMatrixTranspose(view);

		right = DirectX::XMVector3TransformNormal(DirectX::XMVectorSet(1, 0, 0, 0), view);
		up = DirectX::XMVector3TransformNormal(DirectX::XMVectorSet(0, 1, 0, 0), view);
		front = DirectX::XMVector3TransformNormal(DirectX::XMVectorSet(0, 0, 1, 0), view);

		DirectX::XMStoreFloat3(&this->pos, eye);
		//DirectX::XMStoreFloat3(&m_focus, focus);
		DirectX::XMStoreFloat3(&this->up, up);
		DirectX::XMStoreFloat3(&this->right, right);

	}
}


/*******************************************************************************
	「監視カメラ」の実装
*******************************************************************************/
void	MapCamera::Watch()
{

}
/*******************************************************************************
	「相対位置固定カメラ」の実装
*******************************************************************************/
void	MapCamera::Directional()
{

}
/*******************************************************************************
	「3人称カメラ」の実装
*******************************************************************************/
void	MapCamera::Tps()
{

}
/*******************************************************************************
	「1人称カメラ」の実装
*******************************************************************************/
void	MapCamera::Fps()
{

}
/*******************************************************************************
	「追跡カメラ」の実装
*******************************************************************************/
void	MapCamera::Chase()
{

	const float	Distance = 1200.0f;

	pos.y = Distance;
}