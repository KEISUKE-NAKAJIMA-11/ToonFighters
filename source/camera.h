#pragma once

#include	<DirectXMath.h>



class Camera
{
public:

	enum MODE
	{
		WATCH,				//	「監視カメラ」
		DIRECTIONAL,		//	「相対位置固定カメラ」
		CHASE,				//	「追跡カメラ」
		TPS,				//	「3人称カメラ(TPS)」
		FPS,				//	「1人称カメラ(FPS)」
		MAX					//	「カメラ管理番号最大値」	
	};

private:

	int		mode;
	int     timer = 0;

public:
	DirectX::XMFLOAT3	pos;				//	「位置(座標)」
	DirectX::XMFLOAT3	standard_pos;				//	「位置(座標)」
	DirectX::XMFLOAT3	target;				//	「注視点」
	DirectX::XMFLOAT3		at = { 0.0f,0.0f,0.0f };
	DirectX::XMMATRIX	projection;			//	「投影行列」
	int camera_shake_time = 0.0f;

	Camera();

	void				Update();					//	更新関数
	void Update(DirectX::XMFLOAT3 pos1, DirectX::XMFLOAT3 pos2, bool hit1p, bool hit2p);
	//void				Update(DirectX::XMFLOAT3 pos1, DirectX::XMFLOAT3 pos2);					//	格ゲー用カメラ更新関数

private:		//	追加メンバ関数(外部から呼び出す予定が無いのでprivate化)(汚い組み方だねぇ…)
	void				Watch();		//	監視カメラ関数
	void				Directional();	//	相対位置固定カメラ関数
	void				Chase(DirectX::XMFLOAT3 pos1, DirectX::XMFLOAT3 pos2);		//	追跡カメラ関数

	void				WinZoom1p(DirectX::XMFLOAT3 pos1, DirectX::XMFLOAT3 pos2);		//	追跡カメラ関数
	void				WinZoom2p(DirectX::XMFLOAT3 pos1, DirectX::XMFLOAT3 pos2);		//	追跡カメラ関数
	void CameraShake(float min, float max, float total_time, float time);
	void				Tps();			//	3人称カメラ(TPS)関数
	void				Fps();			//	1人称カメラ(FPS)関数
public:

	DirectX::XMMATRIX	SetOrthographicMatrix(float w, float h, float znear, float zfar);			//	平行投影行列設定関数
	DirectX::XMMATRIX	SetPerspectiveMatrix(float fov, float aspect, float znear, float zfar);	 //	透視投影行列設定関数
	DirectX::XMMATRIX	GetViewMatrix();															//	ビュー行列取得関数
	DirectX::XMMATRIX	GetProjectionMatrix() { return	projection; }			//	投影行列取得関数


	

};



class MapCamera: public Camera
{
public:

	enum MODE
	{
		CHASE,
		WATCH,				//	「監視カメラ」
		DIRECTIONAL,		//	「相対位置固定カメラ」
							//	「追跡カメラ」
		TPS,				//	「3人称カメラ(TPS)」
		FPS,				//	「1人称カメラ(FPS)」
		MAX					//	「カメラ管理番号最大値」	
	};

private:

	int		mode;

public:
	DirectX::XMFLOAT3	pos;				//	「位置(座標)」
	DirectX::XMFLOAT3	target;				//	「注視点」
	DirectX::XMMATRIX	projection;			//	「投影行列」
	DirectX::XMFLOAT3	up = DirectX::XMFLOAT3(0, 1, 0);

	DirectX::XMFLOAT3	right = DirectX::XMFLOAT3(1, 0, 0);
	float				distance = 0.0f;

	DirectX::XMFLOAT2	oldCursor;
	DirectX::XMFLOAT2	newCursor;

	float				rotateX = DirectX::XMConvertToRadians(45);
	float				rotateY = 0.0f;


	MapCamera();

	void				Update();					//	更新関数

private:		//	追加メンバ関数(外部から呼び出す予定が無いのでprivate化)(汚い組み方だねぇ…)
	void				Watch();		//	監視カメラ関数
	void				Directional();	//	相対位置固定カメラ関数
	void				Chase();		//	追跡カメラ関数
	void				Tps();			//	3人称カメラ(TPS)関数
	void				Fps();			//	1人称カメラ(FPS)関数
public:

	DirectX::XMMATRIX	SetOrthographicMatrix(float w, float h, float znear, float zfar);			//	平行投影行列設定関数
	DirectX::XMMATRIX	SetPerspectiveMatrix(float fov, float aspect, float znear, float zfar);	 //	透視投影行列設定関数
	DirectX::XMMATRIX	GetViewMatrix();															//	ビュー行列取得関数
	DirectX::XMMATRIX	GetProjectionMatrix() { return	projection; }								//	投影行列取得関数

};



static inline float InQuad(float time, float totaltime, float max = 1, float min = 0)
{
	max -= min;
	time /= totaltime;

	return max * time * time + min;
}