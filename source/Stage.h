#pragma once
#include "model.h"
#include "model_renderer.h"
#include "model_resource.h"
#include "collision.h"

// �X�e�[�W
class Stage
{

public:
	const float STAGE_WIDTH = 111;

	DirectX::XMFLOAT3 angle = {};
	DirectX::XMFLOAT4 color = {1,1,1,1};
public:


	Stage(const char* _filename_temp);
	~Stage();

	// �C���X�^���X�擾
	static Stage& Instance();

	// �X�V����


	void Render(ID3D11DeviceContext* dc, DirectX::XMFLOAT4X4 view_projection, DirectX::XMFLOAT4X4 projection, DirectX::XMFLOAT4 light_direction, std::shared_ptr<Camera> camera, float elapsed_time);

	
	void Render(ID3D11DeviceContext* dc, Shader* shader, DirectX::XMFLOAT4X4 view_projection, DirectX::XMFLOAT4X4 projection, DirectX::XMFLOAT4 light_direction, std::shared_ptr<Camera> camera, float elapsed_time, DirectX::XMFLOAT4X4 view, DirectX::XMFLOAT4 brightColor, DirectX::XMFLOAT4 darkColor);

	void Render(ID3D11DeviceContext* dc, DirectX::XMFLOAT4X4 view_projection, DirectX::XMFLOAT4X4 projection, DirectX::XMFLOAT4 light_direction, std::shared_ptr<Camera> camera, float elapsed_time, DirectX::XMFLOAT4X4 view);

	//void Render(ID3D11DeviceContext* dc, Shader* shader, DirectX::XMFLOAT4X4 view_projection, DirectX::XMFLOAT4X4 projection, DirectX::XMFLOAT4 light_direction, std::shared_ptr<Camera> camera, float elapsed_time, DirectX::XMFLOAT4X4 view);

	bool RayCast(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end, HitResult& hit);

	// ���C�L���X�g
	//bool RayCast(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end, HitResult& hit);

private:
	const char* filename_temp;
	std::shared_ptr<Model> model;
	std::shared_ptr<ModelResource>	model_resource;
	std::shared_ptr<ModelRenderer> model_renderer;
};



// ��
class Wall
{

public:
	const float STAGE_WIDTH = 111;

	DirectX::XMFLOAT3 angle = {};
	DirectX::XMFLOAT4 color = { 1,1,1,1 };
public:


	Wall(const char* _filename_temp);
	~Wall();

	// �C���X�^���X�擾
	static Wall& Instance();

	// �X�V����


	//void Render(ID3D11DeviceContext* dc, DirectX::XMFLOAT4X4 view_projection, DirectX::XMFLOAT4X4 projection, DirectX::XMFLOAT4 light_direction, std::shared_ptr<Camera> camera, float elapsed_time);


	void Render(ID3D11DeviceContext* dc, DirectX::XMFLOAT4X4 view_projection, DirectX::XMFLOAT4X4 projection, DirectX::XMFLOAT4 light_direction, std::shared_ptr<Camera> camera, float elapsed_time, DirectX::XMFLOAT4X4 view);

	// ���C�L���X�g
	bool RayCast(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end, HitResult& hit);

private:
	const char* filename_temp;
	std::shared_ptr<Model> model;
	std::shared_ptr<Model> model2;
	std::shared_ptr<ModelResource>	model_resource;
	std::shared_ptr<ModelRenderer> model_renderer;
};