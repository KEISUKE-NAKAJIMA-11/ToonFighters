#pragma once

#include	<DirectXMath.h>



class Camera
{
public:

	enum MODE
	{
		WATCH,				//	�u�Ď��J�����v
		DIRECTIONAL,		//	�u���Έʒu�Œ�J�����v
		CHASE,				//	�u�ǐՃJ�����v
		TPS,				//	�u3�l�̃J����(TPS)�v
		FPS,				//	�u1�l�̃J����(FPS)�v
		MAX					//	�u�J�����Ǘ��ԍ��ő�l�v	
	};

private:

	int		mode;
	int     timer = 0;

public:
	DirectX::XMFLOAT3	pos;				//	�u�ʒu(���W)�v
	DirectX::XMFLOAT3	standard_pos;				//	�u�ʒu(���W)�v
	DirectX::XMFLOAT3	target;				//	�u�����_�v
	DirectX::XMFLOAT3		at = { 0.0f,0.0f,0.0f };
	DirectX::XMMATRIX	projection;			//	�u���e�s��v
	int camera_shake_time = 0.0f;

	Camera();

	void				Update();					//	�X�V�֐�
	void Update(DirectX::XMFLOAT3 pos1, DirectX::XMFLOAT3 pos2, bool hit1p, bool hit2p);
	//void				Update(DirectX::XMFLOAT3 pos1, DirectX::XMFLOAT3 pos2);					//	�i�Q�[�p�J�����X�V�֐�

private:		//	�ǉ������o�֐�(�O������Ăяo���\�肪�����̂�private��)(�����g�ݕ����˂��c)
	void				Watch();		//	�Ď��J�����֐�
	void				Directional();	//	���Έʒu�Œ�J�����֐�
	void				Chase(DirectX::XMFLOAT3 pos1, DirectX::XMFLOAT3 pos2);		//	�ǐՃJ�����֐�

	void				WinZoom1p(DirectX::XMFLOAT3 pos1, DirectX::XMFLOAT3 pos2);		//	�ǐՃJ�����֐�
	void				WinZoom2p(DirectX::XMFLOAT3 pos1, DirectX::XMFLOAT3 pos2);		//	�ǐՃJ�����֐�
	void CameraShake(float min, float max, float total_time, float time);
	void				Tps();			//	3�l�̃J����(TPS)�֐�
	void				Fps();			//	1�l�̃J����(FPS)�֐�
public:

	DirectX::XMMATRIX	SetOrthographicMatrix(float w, float h, float znear, float zfar);			//	���s���e�s��ݒ�֐�
	DirectX::XMMATRIX	SetPerspectiveMatrix(float fov, float aspect, float znear, float zfar);	 //	�������e�s��ݒ�֐�
	DirectX::XMMATRIX	GetViewMatrix();															//	�r���[�s��擾�֐�
	DirectX::XMMATRIX	GetProjectionMatrix() { return	projection; }			//	���e�s��擾�֐�


	

};



class MapCamera: public Camera
{
public:

	enum MODE
	{
		CHASE,
		WATCH,				//	�u�Ď��J�����v
		DIRECTIONAL,		//	�u���Έʒu�Œ�J�����v
							//	�u�ǐՃJ�����v
		TPS,				//	�u3�l�̃J����(TPS)�v
		FPS,				//	�u1�l�̃J����(FPS)�v
		MAX					//	�u�J�����Ǘ��ԍ��ő�l�v	
	};

private:

	int		mode;

public:
	DirectX::XMFLOAT3	pos;				//	�u�ʒu(���W)�v
	DirectX::XMFLOAT3	target;				//	�u�����_�v
	DirectX::XMMATRIX	projection;			//	�u���e�s��v
	DirectX::XMFLOAT3	up = DirectX::XMFLOAT3(0, 1, 0);

	DirectX::XMFLOAT3	right = DirectX::XMFLOAT3(1, 0, 0);
	float				distance = 0.0f;

	DirectX::XMFLOAT2	oldCursor;
	DirectX::XMFLOAT2	newCursor;

	float				rotateX = DirectX::XMConvertToRadians(45);
	float				rotateY = 0.0f;


	MapCamera();

	void				Update();					//	�X�V�֐�

private:		//	�ǉ������o�֐�(�O������Ăяo���\�肪�����̂�private��)(�����g�ݕ����˂��c)
	void				Watch();		//	�Ď��J�����֐�
	void				Directional();	//	���Έʒu�Œ�J�����֐�
	void				Chase();		//	�ǐՃJ�����֐�
	void				Tps();			//	3�l�̃J����(TPS)�֐�
	void				Fps();			//	1�l�̃J����(FPS)�֐�
public:

	DirectX::XMMATRIX	SetOrthographicMatrix(float w, float h, float znear, float zfar);			//	���s���e�s��ݒ�֐�
	DirectX::XMMATRIX	SetPerspectiveMatrix(float fov, float aspect, float znear, float zfar);	 //	�������e�s��ݒ�֐�
	DirectX::XMMATRIX	GetViewMatrix();															//	�r���[�s��擾�֐�
	DirectX::XMMATRIX	GetProjectionMatrix() { return	projection; }								//	���e�s��擾�֐�

};



static inline float InQuad(float time, float totaltime, float max = 1, float min = 0)
{
	max -= min;
	time /= totaltime;

	return max * time * time + min;
}