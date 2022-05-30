#pragma once
#include "model.h"
#include "model_renderer.h"
#include "model_resource.h"
#include "collision.h"

// ステージ
class Stage
{

public:
	const float STAGE_WIDTH = 111;

	DirectX::XMFLOAT3 angle = {};
	DirectX::XMFLOAT4 color = {1,1,1,1};
public:


	Stage(const char* _filename_temp);
	~Stage();

	// インスタンス取得
	static Stage& Instance();

	// 更新処理


	void Render(ID3D11DeviceContext* dc, DirectX::XMFLOAT4X4 view_projection, DirectX::XMFLOAT4X4 projection, DirectX::XMFLOAT4 light_direction, std::shared_ptr<Camera> camera, float elapsed_time);

	
	void Render(ID3D11DeviceContext* dc, Shader* shader, DirectX::XMFLOAT4X4 view_projection, DirectX::XMFLOAT4X4 projection, DirectX::XMFLOAT4 light_direction, std::shared_ptr<Camera> camera, float elapsed_time, DirectX::XMFLOAT4X4 view, DirectX::XMFLOAT4 brightColor, DirectX::XMFLOAT4 darkColor);

	void Render(ID3D11DeviceContext* dc, DirectX::XMFLOAT4X4 view_projection, DirectX::XMFLOAT4X4 projection, DirectX::XMFLOAT4 light_direction, std::shared_ptr<Camera> camera, float elapsed_time, DirectX::XMFLOAT4X4 view);

	//void Render(ID3D11DeviceContext* dc, Shader* shader, DirectX::XMFLOAT4X4 view_projection, DirectX::XMFLOAT4X4 projection, DirectX::XMFLOAT4 light_direction, std::shared_ptr<Camera> camera, float elapsed_time, DirectX::XMFLOAT4X4 view);

	bool RayCast(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end, HitResult& hit);

	// レイキャスト
	//bool RayCast(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end, HitResult& hit);

private:
	const char* filename_temp;
	std::shared_ptr<Model> model;
	std::shared_ptr<ModelResource>	model_resource;
	std::shared_ptr<ModelRenderer> model_renderer;
};



// 壁
class Wall
{

public:
	const float STAGE_WIDTH = 111;

	DirectX::XMFLOAT3 angle = {};
	DirectX::XMFLOAT4 color = { 1,1,1,1 };
public:


	Wall(const char* _filename_temp);
	~Wall();

	// インスタンス取得
	static Wall& Instance();

	// 更新処理


	//void Render(ID3D11DeviceContext* dc, DirectX::XMFLOAT4X4 view_projection, DirectX::XMFLOAT4X4 projection, DirectX::XMFLOAT4 light_direction, std::shared_ptr<Camera> camera, float elapsed_time);


	void Render(ID3D11DeviceContext* dc, DirectX::XMFLOAT4X4 view_projection, DirectX::XMFLOAT4X4 projection, DirectX::XMFLOAT4 light_direction, std::shared_ptr<Camera> camera, float elapsed_time, DirectX::XMFLOAT4X4 view);

	// レイキャスト
	bool RayCast(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end, HitResult& hit);

private:
	const char* filename_temp;
	std::shared_ptr<Model> model;
	std::shared_ptr<Model> model2;
	std::shared_ptr<ModelResource>	model_resource;
	std::shared_ptr<ModelRenderer> model_renderer;
};