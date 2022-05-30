#pragma once

#include "ui_element.h"

#include <d3d11.h>
#include <wrl.h>
#include <DirectXMath.h>

class UIChargeGauge :public UIElement
{
private:
	static const int surfaceCount = 64;
	static constexpr float outlineRadius = gaugeRadius - 32.0f - 16.0f;
	static constexpr float inlineRadius = gaugeRadius - 32.0f * 2;

	unsigned int verticesCount = 0;
	int count[2] = {};

	ID3D11Buffer* vertexBuffer = nullptr;
	ID3D11Buffer* constantBuffer = nullptr;
	ID3D11VertexShader* vertexShader = nullptr;
	ID3D11InputLayout* inputLayout = nullptr;
	ID3D11PixelShader* pixelShader = nullptr;
	ID3D11RasterizerState* rasterizer = nullptr;
	ID3D11DepthStencilState* depthStencil = nullptr;

	struct Vertex
	{
		DirectX::XMFLOAT3 pos = {};
		float dummy = {};
	};

	void CreateBuffers(ID3D11Device* device, Vertex* vertices, unsigned int vNum);
	void CreateVertexShader(ID3D11Device* device);
	void CreatePixelShader(ID3D11Device* device);
	void CreateRasterrizer(ID3D11Device* device);
	void CreateDepthStencil(ID3D11Device* device);

public:
	~UIChargeGauge();

	void Initialize(const Entity* entity);
	void Update(const float deltaTime, Entity* entity);
	void Render(const Entity* entity);
};
