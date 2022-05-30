#include "ui_charge_gauge.h"

#include "input_manager.h"
#include "scene.h"
#include "player_component.h"
#include <stdio.h>
#include <vector>
#include <assert.h>

UIChargeGauge::~UIChargeGauge()
{
	if (vertexBuffer)vertexBuffer->Release();
	if (constantBuffer)constantBuffer->Release();
	if (vertexShader)vertexShader->Release();
	if (inputLayout) inputLayout->Release();
	if (pixelShader) pixelShader->Release();
	if (rasterizer)rasterizer->Release();
	if (depthStencil)depthStencil->Release();
}

void UIChargeGauge::Initialize(const Entity* entity)
{
	ID3D11Device* device = entity->GetScene()->GetFramework()->GetDevice();

	std::vector<Vertex> vertices;
	vertices.resize((surfaceCount + 1) * 2);
	vertices.clear();

	float radian = XMConvertToRadians(-90.0f);
	constexpr float radianSub = XMConvertToRadians(-360.0f / surfaceCount);
	for (int i = 0; i < surfaceCount + 1; i++)
	{
		float sn = sinf(radian);
		float cs = cosf(radian);

		Vertex outlineVertex = {
			XMFLOAT3(cs * outlineRadius,sn * outlineRadius,0.0f),
			0.0f
		};
		vertices.push_back(outlineVertex);

		Vertex inlineVertex = {
			XMFLOAT3(cs * inlineRadius,sn * inlineRadius,0.0f),
			0.0f
		};
		vertices.push_back(inlineVertex);

		radian += radianSub;
	}

	for (auto& vertex : vertices)
	{
		vertex.pos.x += GaugePivot.x;
		vertex.pos.x /= gScreenWidth;
		vertex.pos.x *= 2.0f;
		vertex.pos.x -= 1.0f;

		vertex.pos.y += GaugePivot.y;
		vertex.pos.y /= gScreenHeight;
		vertex.pos.y *= 2.0f;
		vertex.pos.y -= 1.0f;
		vertex.pos.y *= -1.0f;
	}
	verticesCount = vertices.size();

	CreateBuffers(device, vertices.data(), verticesCount);
	CreateVertexShader(device);
	CreatePixelShader(device);
	CreateRasterrizer(device);
	CreateDepthStencil(device);
}

void UIChargeGauge::CreateBuffers(ID3D11Device* device, Vertex* vertices, unsigned int vNum)
{
	HRESULT hr = S_OK;

	//create vertex buffer
	{
		D3D11_BUFFER_DESC bufdesk = {};
		bufdesk.Usage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
		bufdesk.ByteWidth = sizeof(Vertex) * vNum;
		bufdesk.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufdesk.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufdesk.MiscFlags = 0;
		bufdesk.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA subresouce = {};
		subresouce.pSysMem = vertices;

		hr = device->CreateBuffer(&bufdesk, &subresouce, &vertexBuffer);
		assert(hr == S_OK && "CreateBuffer Vertex failed");
	}

	//create constant buffer
	{
		D3D11_BUFFER_DESC constantBuffer_desc = {};
		constantBuffer_desc.ByteWidth = sizeof(XMFLOAT4);
		constantBuffer_desc.Usage = D3D11_USAGE_DEFAULT;
		constantBuffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constantBuffer_desc.CPUAccessFlags = 0;
		constantBuffer_desc.MiscFlags = 0;
		constantBuffer_desc.StructureByteStride = 0;
		hr = device->CreateBuffer(&constantBuffer_desc, nullptr, &constantBuffer);
		assert(SUCCEEDED(hr) && "CreateBuffer constant failed");
	}
}

void UIChargeGauge::CreateVertexShader(ID3D11Device* device)
{
	HRESULT hr = S_OK;

	D3D11_INPUT_ELEMENT_DESC inputElement[] = {
		{"POSITION",	0,	DXGI_FORMAT_R32G32B32_FLOAT,	0,	D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA,	0},
		{"DUMMY",	0,	DXGI_FORMAT_R32_FLOAT,	0,	D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA,	0},
	};
	const char* filename = "Shader/cso/ui_gauge_vs.cso";

	std::vector<char> vsdata;
	{
		FILE* fp = nullptr;
		fopen_s(&fp, filename, "rb");
		assert(fp && "file open failed");
		fseek(fp, 0, SEEK_END);
		vsdata.resize(ftell(fp));
		fseek(fp, 0, SEEK_SET);
		fread(vsdata.data(), sizeof(vsdata[0]), vsdata.size(), fp);
		fclose(fp);
	}

	hr = device->CreateVertexShader(vsdata.data(), vsdata.size(), nullptr, &vertexShader);
	assert(hr == S_OK && "CreateVertexShader failed");

	hr = device->CreateInputLayout(inputElement, ARRAYSIZE(inputElement), vsdata.data(), vsdata.size(), &inputLayout);
	assert(hr == S_OK && "CreateInputLayout failed");
}

void UIChargeGauge::CreatePixelShader(ID3D11Device* device)
{
	HRESULT hr = S_OK;

	const char* filename = "Shader/cso/ui_gauge_ps.cso";

	std::vector<char> psdata;
	{
		FILE* fp = nullptr;
		fopen_s(&fp, filename, "rb");
		assert(fp && "file open failed");
		fseek(fp, 0, SEEK_END);
		psdata.resize(ftell(fp));
		fseek(fp, 0, SEEK_SET);
		fread(psdata.data(), sizeof(psdata[0]), psdata.size(), fp);
		fclose(fp);
	}

	hr = device->CreatePixelShader(psdata.data(), psdata.size(), nullptr, &pixelShader);
	assert(hr == S_OK && "create pixel shader failed");
}

void UIChargeGauge::CreateRasterrizer(ID3D11Device* device)
{
	HRESULT hr = S_OK;

	D3D11_RASTERIZER_DESC rasterizer_desc = {};
	rasterizer_desc.FillMode = D3D11_FILL_SOLID;
	rasterizer_desc.CullMode = D3D11_CULL_NONE;
	rasterizer_desc.FrontCounterClockwise = false;
	rasterizer_desc.DepthBias = 0;
	rasterizer_desc.DepthBiasClamp = 0;
	rasterizer_desc.SlopeScaledDepthBias = 0;
	rasterizer_desc.DepthClipEnable = false;
	rasterizer_desc.ScissorEnable = false;
	rasterizer_desc.MultisampleEnable = false;
	rasterizer_desc.AntialiasedLineEnable = false;
	hr = device->CreateRasterizerState(&rasterizer_desc, &rasterizer);
	assert(hr == S_OK && "CreateRasterizerState failed");
}

void UIChargeGauge::CreateDepthStencil(ID3D11Device* device)
{
	HRESULT hr = S_OK;

	D3D11_DEPTH_STENCIL_DESC depth_stencil_desk = {};
	depth_stencil_desk.DepthEnable = FALSE;
	depth_stencil_desk.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depth_stencil_desk.DepthFunc = D3D11_COMPARISON_ALWAYS;
	depth_stencil_desk.StencilEnable = FALSE;
	depth_stencil_desk.StencilReadMask = 0xFF;
	depth_stencil_desk.StencilWriteMask = 0xFF;
	depth_stencil_desk.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depth_stencil_desk.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depth_stencil_desk.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depth_stencil_desk.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depth_stencil_desk.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depth_stencil_desk.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depth_stencil_desk.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depth_stencil_desk.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	hr = device->CreateDepthStencilState(&depth_stencil_desk, &depthStencil);
	assert(hr == S_OK && "CreateDepthStencilState failed");
}

void UIChargeGauge::Update(const float deltaTime, Entity* entity)
{
	auto player = entity->GetScene()->GetOtherEntity(EntityTag::Player);
	if (player)
	{
		PlayerComponent* pCon = player->GetComponent<PlayerComponent>();
		count[0] = (int)(pCon->GetCharge() / pCon->chargeTime * (surfaceCount + 1)) * 2;
		if (count[0] <= 2)count[0] = 0;
		if (count[0] > verticesCount)count[0] = verticesCount;

		if (pCon->GetCharge() > pCon->chargeTime)
		{
			count[1] = (int)((pCon->GetCharge() - pCon->chargeTime) / pCon->chargeTime * (surfaceCount + 1)) * 2;
			if (count[1] <= 2)count[1] = 0;
			if (count[1] > verticesCount)
				count[1] = verticesCount;
		}
		else
		{
			count[1] = 0;
		}
	}
}

#include "font.h"

void UIChargeGauge::Render(const Entity* entity)
{
	ID3D11DeviceContext* context = entity->GetScene()->GetFramework()->GetDeviceContext();

	unsigned int stride = sizeof(Vertex);
	unsigned int offset = 0;
	context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	//context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	//context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
	context->IASetInputLayout(inputLayout);

	context->RSSetState(rasterizer);

	context->VSSetShader(vertexShader, nullptr, 0);
	context->PSSetShader(pixelShader, nullptr, 0);
	context->OMSetDepthStencilState(depthStencil, 1);

	{
		XMFLOAT4 backColor = { 1.0f,1.0f,1.0f,0.5f };
		context->UpdateSubresource(constantBuffer, 0, nullptr, &backColor, 0, 0);
		context->VSSetConstantBuffers(0, 1, &constantBuffer);
		context->Draw(verticesCount, 0);
	}
	{
		XMFLOAT4 gaugeColor = { 0.85f,0.85f,1.0f,1.0f };
		context->UpdateSubresource(constantBuffer, 0, nullptr, &gaugeColor, 0, 0);
		context->VSSetConstantBuffers(0, 1, &constantBuffer);
		context->Draw(count[0], 0);
	}
	{
		XMFLOAT4 gaugeColor = { 1.0f,1.0f,0.25f,1.0f };
		context->UpdateSubresource(constantBuffer, 0, nullptr, &gaugeColor, 0, 0);
		context->VSSetConstantBuffers(0, 1, &constantBuffer);
		context->Draw(count[1], 0);
	}
}
