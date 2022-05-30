#include "sprite.h"
#include "Shadermanager.h"
#include "framework.h"

#include "misc.h"
#include <queue>
#include "texture.h"



static const float UV_ADJUST = 0.99994f;

sprite::sprite(ID3D11Device* device, const wchar_t* file_name, int nBufSize)
{
	HRESULT hr = S_OK;

	vertex vertices[] = {
			{ DirectX::XMFLOAT3(0, 0, 0), DirectX::XMFLOAT4(1, 1, 1, 1), DirectX::XMFLOAT2(0, 0) },
			{ DirectX::XMFLOAT3(0, 0, 0), DirectX::XMFLOAT4(1, 1, 1, 1), DirectX::XMFLOAT2(0, 0) },
			{ DirectX::XMFLOAT3(0, 0, 0), DirectX::XMFLOAT4(1, 1, 1, 1), DirectX::XMFLOAT2(0, 0) },
			{ DirectX::XMFLOAT3(0, 0, 0), DirectX::XMFLOAT4(1, 1, 1, 1), DirectX::XMFLOAT2(0, 0) },
	};

	D3D11_BUFFER_DESC buffer_desc;
	ZeroMemory(&buffer_desc, sizeof(buffer_desc));
	buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
	buffer_desc.ByteWidth = sizeof(vertices) * nBufSize;
	buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	buffer_desc.MiscFlags = 0;
	buffer_desc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA subresource_data;
	ZeroMemory(&subresource_data, sizeof(subresource_data));
	subresource_data.pSysMem = vertices;
	subresource_data.SysMemPitch = 0; //Not use for vertex buffers.
	subresource_data.SysMemSlicePitch = 0; //Not use for vertex buffers.
	hr = device->CreateBuffer(&buffer_desc, &subresource_data, &buffer);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	D3D11_INPUT_ELEMENT_DESC input_element_desc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	create_vs_from_cso(device, "./Shader/sprite_vs.cso", &vertex_shader, &input_layout, input_element_desc, ARRAYSIZE(input_element_desc));
	create_ps_from_cso(device, "./Shader/sprite_ps.cso", &pixel_shader);
	D3D11_RASTERIZER_DESC rasterizer_desc;
	ZeroMemory(&rasterizer_desc, sizeof(rasterizer_desc));

	rasterizer_desc.FillMode = D3D11_FILL_SOLID; //D3D11_FILL_WIREFRAME, D3D11_FILL_SOLID
	rasterizer_desc.CullMode = D3D11_CULL_NONE; //D3D11_CULL_NONE, D3D11_CULL_FRONT, D3D11_CULL_BACK   
	rasterizer_desc.FrontCounterClockwise = FALSE;
	rasterizer_desc.DepthBias = 0;
	rasterizer_desc.DepthBiasClamp = 0;
	rasterizer_desc.SlopeScaledDepthBias = 0;
	rasterizer_desc.DepthClipEnable = FALSE;
	rasterizer_desc.ScissorEnable = FALSE;
	rasterizer_desc.MultisampleEnable = FALSE;
	rasterizer_desc.AntialiasedLineEnable = FALSE;
	hr = device->CreateRasterizerState(&rasterizer_desc, &rasterizer_state);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	hr = load_texture_from_file(device, file_name, &shader_resource_view, &texture2d_desc);

	D3D11_SAMPLER_DESC sampler_desc;
	sampler_desc.Filter = D3D11_FILTER_ANISOTROPIC; //D3D11_FILTER_ANISOTROPIC
	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	sampler_desc.MipLODBias = 0;
	sampler_desc.MaxAnisotropy = 16;
	sampler_desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	memcpy(sampler_desc.BorderColor, &DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f), sizeof(DirectX::XMFLOAT4));
	sampler_desc.MinLOD = 0;
	sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = device->CreateSamplerState(&sampler_desc, &sampler_state);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	//Unit06 setting depthstencildesc;
	D3D11_DEPTH_STENCIL_DESC depth_stencil_desc;
	depth_stencil_desc.DepthEnable = FALSE;
	depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depth_stencil_desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
	depth_stencil_desc.StencilEnable = FALSE;
	depth_stencil_desc.StencilReadMask = 0xFF;
	depth_stencil_desc.StencilWriteMask = 0xFF;
	depth_stencil_desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depth_stencil_desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depth_stencil_desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depth_stencil_desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depth_stencil_desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depth_stencil_desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depth_stencil_desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depth_stencil_desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	hr = device->CreateDepthStencilState(&depth_stencil_desc, &depth_stencil_state);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));



}

//sprite::sprite(ID3D11Device* device, int nBufSize)
//{
//	HRESULT hr = S_OK;
//
//	vertex vertices[] = {
//			{ DirectX::XMFLOAT3(0, 0, 0), DirectX::XMFLOAT4(1, 1, 1, 1), DirectX::XMFLOAT2(0, 0) },
//			{ DirectX::XMFLOAT3(0, 0, 0), DirectX::XMFLOAT4(1, 1, 1, 1), DirectX::XMFLOAT2(0, 0) },
//			{ DirectX::XMFLOAT3(0, 0, 0), DirectX::XMFLOAT4(1, 1, 1, 1), DirectX::XMFLOAT2(0, 0) },
//			{ DirectX::XMFLOAT3(0, 0, 0), DirectX::XMFLOAT4(1, 1, 1, 1), DirectX::XMFLOAT2(0, 0) },
//	};
//
//	D3D11_BUFFER_DESC buffer_desc;
//	ZeroMemory(&buffer_desc, sizeof(buffer_desc));
//	buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
//	buffer_desc.ByteWidth = sizeof(vertices) * nBufSize;
//	buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
//	buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
//	buffer_desc.MiscFlags = 0;
//	buffer_desc.StructureByteStride = 0;
//
//	D3D11_SUBRESOURCE_DATA subresource_data;
//	ZeroMemory(&subresource_data, sizeof(subresource_data));
//	subresource_data.pSysMem = vertices;
//	subresource_data.SysMemPitch = 0; //Not use for vertex buffers.
//	subresource_data.SysMemSlicePitch = 0; //Not use for vertex buffers.
//	hr = device->CreateBuffer(&buffer_desc, &subresource_data, &buffer);
//	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
//
//	D3D11_INPUT_ELEMENT_DESC input_element_desc[] =
//	{
//		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
//		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
//		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
//	};
//	create_vs_from_cso(device, "./Shader/sprite_vs.cso", &vertex_shader, &input_layout, input_element_desc, ARRAYSIZE(input_element_desc));
//	create_ps_from_cso(device, "./Shader/sprite_ps.cso", &pixel_shader);
//	D3D11_RASTERIZER_DESC rasterizer_desc;
//	ZeroMemory(&rasterizer_desc, sizeof(rasterizer_desc));
//
//	rasterizer_desc.FillMode = D3D11_FILL_SOLID; //D3D11_FILL_WIREFRAME, D3D11_FILL_SOLID
//	rasterizer_desc.CullMode = D3D11_CULL_NONE; //D3D11_CULL_NONE, D3D11_CULL_FRONT, D3D11_CULL_BACK   
//	rasterizer_desc.FrontCounterClockwise = FALSE;
//	rasterizer_desc.DepthBias = 0;
//	rasterizer_desc.DepthBiasClamp = 0;
//	rasterizer_desc.SlopeScaledDepthBias = 0;
//	rasterizer_desc.DepthClipEnable = FALSE;
//	rasterizer_desc.ScissorEnable = FALSE;
//	rasterizer_desc.MultisampleEnable = FALSE;
//	rasterizer_desc.AntialiasedLineEnable = FALSE;
//	hr = device->CreateRasterizerState(&rasterizer_desc, &rasterizer_state);
//	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
//
//
//	D3D11_SAMPLER_DESC sampler_desc;
//	sampler_desc.Filter = D3D11_FILTER_ANISOTROPIC; //D3D11_FILTER_ANISOTROPIC
//	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
//	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
//	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
//	sampler_desc.MipLODBias = 0;
//	sampler_desc.MaxAnisotropy = 16;
//	sampler_desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
//	memcpy(sampler_desc.BorderColor, &DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f), sizeof(DirectX::XMFLOAT4));
//	sampler_desc.MinLOD = 0;
//	sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;
//	hr = device->CreateSamplerState(&sampler_desc, &sampler_state);
//	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
//
//	//Unit06 setting depthstencildesc;
//	D3D11_DEPTH_STENCIL_DESC depth_stencil_desc;
//	depth_stencil_desc.DepthEnable = FALSE;
//	depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
//	depth_stencil_desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
//	depth_stencil_desc.StencilEnable = FALSE;
//	depth_stencil_desc.StencilReadMask = 0xFF;
//	depth_stencil_desc.StencilWriteMask = 0xFF;
//	depth_stencil_desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
//	depth_stencil_desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
//	depth_stencil_desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
//	depth_stencil_desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
//	depth_stencil_desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
//	depth_stencil_desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
//	depth_stencil_desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
//	depth_stencil_desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
//	hr = device->CreateDepthStencilState(&depth_stencil_desc, &depth_stencil_state);
//	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
//
//
//}


void sprite::render(ID3D11DeviceContext* context,
	const VECTOR2& position, const VECTOR2& scale,
	const VECTOR2& texPos, const VECTOR2& texSize,
	const VECTOR2& center, float angle,
	const VECTOR4& color) const
{
	if (scale.x == 0.0f || scale.y == 0.0f) return;

	D3D11_VIEWPORT viewport;
	UINT numViewports = 1;
	context->RSGetViewports(&numViewports, &viewport);

	float tw = texSize.x;
	float th = texSize.y;
	if (tw <= 0.0f || th <= 0.0f)
	{
		tw = (float)texture2d_desc.Width;
		th = (float)texture2d_desc.Height;
	}

	vertex vertices[4] = {};
#ifdef GAMELIB_PLUS_UP
	vertices[0] = { VECTOR3(0.0f, 1.0f, 0), color, VECTOR2(0, 0) };
	vertices[1] = { VECTOR3(1.0f, 1.0f, 0), color, VECTOR2(1, 0) };
	vertices[2] = { VECTOR3(0.0f, 0.0f, 0), color, VECTOR2(0, 1) };
	vertices[3] = { VECTOR3(1.0f, 0.0f, 0), color, VECTOR2(1, 1) };
#else
	vertices[0] = { VECTOR3(0.0f, 1.0f, 0), color, VECTOR2(0, 1) };
	vertices[1] = { VECTOR3(1.0f, 1.0f, 0), color, VECTOR2(1, 1) };
	vertices[2] = { VECTOR3(0.0f, 0.0f, 0), color, VECTOR2(0, 0) };
	vertices[3] = { VECTOR3(1.0f, 0.0f, 0), color, VECTOR2(1, 0) };
#endif

	float sinValue = sinf(angle);
	float cosValue = cosf(angle);
	float mx = (tw * scale.x) / tw * center.x;
	float my = (th * scale.y) / th * center.y;
	for (int i = 0; i < 4; i++)
	{
		vertices[i].position.x *= (tw * scale.x);
		vertices[i].position.y *= (th * scale.y);

		vertices[i].position.x -= mx;
		vertices[i].position.y -= my;

		float rx = vertices[i].position.x;
		float ry = vertices[i].position.y;
		vertices[i].position.x = rx * cosValue - ry * sinValue;
		vertices[i].position.y = rx * sinValue + ry * cosValue;

		vertices[i].position.x += mx;
		vertices[i].position.y += my;

		vertices[i].position.x += (position.x - scale.x * center.x);
		vertices[i].position.y += (position.y - scale.y * center.y);

		vertices[i].position.x = -1.0f + vertices[i].position.x * 2 / viewport.Width;
		vertices[i].position.y = 1.0f - vertices[i].position.y * 2 / viewport.Height;
#ifdef GAMELIB_PLUS_UP
		vertices[i].position.y = -vertices[i].position.y;
#endif

		// UV座標の調整
		vertices[i].texcoord.x = (std::min)(vertices[i].texcoord.x, UV_ADJUST);
		vertices[i].texcoord.y = (std::min)(vertices[i].texcoord.y, UV_ADJUST);

		vertices[i].texcoord.x = (texPos.x + vertices[i].texcoord.x * tw) / texture2d_desc.Width;
		vertices[i].texcoord.y = (texPos.y + vertices[i].texcoord.y * th) / texture2d_desc.Height;
	}

	D3D11_MAPPED_SUBRESOURCE msr;
	context->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
	memcpy(msr.pData, vertices, sizeof(vertices));
	context->Unmap(buffer, 0);

	UINT stride = sizeof(vertex);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &buffer, &stride, &offset);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	context->IASetInputLayout(input_layout);
	context->RSSetState(rasterizer_state);
	context->VSSetShader(vertex_shader, nullptr, 0);
	context->PSSetShader(pixel_shader, nullptr, 0);

	context->PSSetShaderResources(0, 1, &shader_resource_view);
	context->PSSetSamplers(0, 1, &sampler_state);

	context->OMSetDepthStencilState(depth_stencil_state, 1);

	context->Draw(4, 0);
}

//void sprite::render(ID3D11DeviceContext* context, TextureShader* tex, float dx, float dy, float dw, float dh, float sx, float sy, float sw, float sh, float angle, float alpha)
//{
//	ID3D11DeviceContext* device_context = context;
//	D3D11_VIEWPORT vp;
//	UINT num = 1;
//
//	device_context->RSGetViewports(&num, &vp);
//	float screen_width = vp.Width;
//	float screen_height = vp.Height;
//
//
//
//
//	context->VSSetShader(vertex_shader, nullptr, 0);
//	context->PSSetShader(pixel_shader, nullptr, 0);
//	//頂点データ設定
//	vertex data[4];
//
//	data[0].position.x = dx;
//	data[0].position.y = dy;
//	data[0].position.z = 0.0f;
//			
//	data[1].position.x = dx + dw;
//	data[1].position.y = dy;
//	data[1].position.z = 0.0f;
//			
//	data[2].position.x = dx;
//	data[2].position.y = dy + dh;
//	data[2].position.z = 0.0f;
//			
//	data[3].position.x = dx + dw;
//	data[3].position.y = dy + dh;
//	data[3].position.z = 0.0f;
//
//	//回転の中心
//	float workPosX = dx + dw * 0.5f;
//	float workPosY = dy + dh * 0.5f;
//
//	//回転処理
//	for (int i = 0; i < 4; i++) {
//
//		float workX = data[i].position.x - workPosX;
//		float workY = data[i].position.y - workPosY;
//		data[i].position.x = workX * cosf(angle) - workY * sinf(angle) + workPosX;
//		data[i].position.y = workX * sinf(angle) + workY * cosf(angle) + workPosY;
//		data[i].position.z = 0.0f;
//	}
//
//
//
//	// 正規化デバイス座標系
//
//	for (int i = 0; i < 4; i++) {
//		data[i].position.x = 2.0f * data[i].position.x / screen_width - 1.0f;
//		data[i].position.y = 1.0f - 2.0f * data[i].position.y / screen_height;
//		data[i].position.z = 0.0f;
//	}
//
//
//	//テクスチャ座標設定
//	data[0].texcoord.x = sx;
//	data[0].texcoord.y = sy;
//	data[1].texcoord.x = sx + sw;
//	data[1].texcoord.y = sy;
//	data[2].texcoord.x = sx;
//	data[2].texcoord.y = sy + sh;
//	data[3].texcoord.x = sx + sw;
//	data[3].texcoord.y = sy + sh;
//
//	//UV座標
//	if (tex) {
//		for (int i = 0; i < 4; i++) {
//			data[i].texcoord.x = data[i].texcoord.x / tex->GetWidth();
//			data[i].texcoord.y = data[i].texcoord.y / tex->GetHeight();
//		}
//	}
//	//頂点カラー
//	data[0].color = XMFLOAT4(1, 1, 1, alpha);
//	data[1].color = XMFLOAT4(1, 1, 1, alpha);
//	data[2].color = XMFLOAT4(1, 1, 1, alpha);
//	data[3].color = XMFLOAT4(1, 1, 1, alpha);
//	//法線
//	/*data[0].no = XMFLOAT3(0, 0, 1);
//	data[1].Normal = XMFLOAT3(0, 0, 1);
//	data[2].Normal = XMFLOAT3(0, 0, 1);
//	data[3].Normal = XMFLOAT3(0, 0, 1);*/
//
//	UINT stride = sizeof(vertex);
//	UINT offset = 0;
//	context->IASetVertexBuffers(0, 1, &buffer, &stride, &offset);
//	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
//	context->IASetInputLayout(input_layout);
//	context->RSSetState(rasterizer_state);
//	context->VSSetShader(vertex_shader, nullptr, 0);
//	context->PSSetShader(pixel_shader, nullptr, 0);
//
//	context->PSSetShaderResources(0, 1, &shader_resource_view);
//	context->PSSetSamplers(0, 1, &sampler_state);
//
//	context->OMSetDepthStencilState(depth_stencil_state, 1);
//
//	//テクスチャの設定
//	if (tex) {
//		tex->Set(0);
//	}
//	else {
//		if (texture)texture->Set(0);
//	}
//
//	device_context->Draw(4, 0);
//	//シェーダ無効か
//	
//
//}

//--------------------------------------------------------------
//  スプライト描画
//--------------------------------------------------------------




sprite_batch::sprite_batch(ID3D11Device* device, const wchar_t* file_name, size_t max_instance) : MAX_INSTANCES(max_instance)
{
	HRESULT hr = S_OK;

	vertex vertices[] =
	{
		{ DirectX::XMFLOAT3(0, 0, 0), DirectX::XMFLOAT2(0, 0) },
		{ DirectX::XMFLOAT3(1, 0, 0), DirectX::XMFLOAT2(1, 0) },
		{ DirectX::XMFLOAT3(0, 1, 0), DirectX::XMFLOAT2(0, 1) },
		{ DirectX::XMFLOAT3(1, 1, 0), DirectX::XMFLOAT2(1, 1) },
	};

	D3D11_BUFFER_DESC buffer_desc = {};
	buffer_desc.ByteWidth = sizeof(vertices);
	buffer_desc.Usage = D3D11_USAGE_IMMUTABLE;
	buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	buffer_desc.CPUAccessFlags = 0;
	buffer_desc.MiscFlags = 0;
	buffer_desc.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA subresource_data = {};
	subresource_data.pSysMem = vertices;
	subresource_data.SysMemPitch = 0; //Not use for vertex buffers.
	subresource_data.SysMemSlicePitch = 0; //Not use for vertex buffers.
	hr = device->CreateBuffer(&buffer_desc, &subresource_data, &vertex_buffer);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	D3D11_INPUT_ELEMENT_DESC input_element_desc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NDC_TRANSFORM", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "NDC_TRANSFORM", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "NDC_TRANSFORM", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "NDC_TRANSFORM", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "TEXCOORD_TRANSFORM", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	};
	create_vs_from_cso(device, "./Shader/sprite_batch_vs.cso", &vertex_shader, &input_layout, input_element_desc, ARRAYSIZE(input_element_desc));
	create_ps_from_cso(device, "./Shader/sprite_batch_ps.cso", &pixel_shader);

	instance* instances = new instance[MAX_INSTANCES];
	{
		D3D11_BUFFER_DESC buffer_desc = {};
		D3D11_SUBRESOURCE_DATA subresource_data = {};

		buffer_desc.ByteWidth = sizeof(instance) * (UINT)MAX_INSTANCES;
		buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
		buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		buffer_desc.MiscFlags = 0;
		buffer_desc.StructureByteStride = 0;
		subresource_data.pSysMem = instances;
		subresource_data.SysMemPitch = 0; //Not use for vertex buffers.mm 
		subresource_data.SysMemSlicePitch = 0; //Not use for vertex buffers.
		hr = device->CreateBuffer(&buffer_desc, &subresource_data, &instance_buffer);
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	}
	delete[] instances;

	D3D11_RASTERIZER_DESC rasterizer_desc = {};
	rasterizer_desc.FillMode = D3D11_FILL_SOLID; //D3D11_FILL_WIREFRAME, D3D11_FILL_SOLID
	rasterizer_desc.CullMode = D3D11_CULL_NONE; //D3D11_CULL_NONE, D3D11_CULL_FRONT, D3D11_CULL_BACK   
	rasterizer_desc.FrontCounterClockwise = FALSE;
	rasterizer_desc.DepthBias = 0;
	rasterizer_desc.DepthBiasClamp = 0;
	rasterizer_desc.SlopeScaledDepthBias = 0;
	rasterizer_desc.DepthClipEnable = FALSE;
	rasterizer_desc.ScissorEnable = FALSE;
	rasterizer_desc.MultisampleEnable = FALSE;
	rasterizer_desc.AntialiasedLineEnable = FALSE;
	hr = device->CreateRasterizerState(&rasterizer_desc, &rasterizer_state);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	hr = load_texture_from_file(device, file_name, &shader_resource_view, &texture2d_desc);

	D3D11_SAMPLER_DESC sampler_desc;
	sampler_desc.Filter = D3D11_FILTER_ANISOTROPIC; //UNIT.06
	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	sampler_desc.MipLODBias = 0;
	sampler_desc.MaxAnisotropy = 16;
	sampler_desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	memcpy(sampler_desc.BorderColor, &DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f), sizeof(DirectX::XMFLOAT4));
	sampler_desc.MinLOD = 0;
	sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = device->CreateSamplerState(&sampler_desc, &sampler_state);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	D3D11_DEPTH_STENCIL_DESC depth_stencil_desc;
	depth_stencil_desc.DepthEnable = FALSE;
	depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depth_stencil_desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
	depth_stencil_desc.StencilEnable = FALSE;
	depth_stencil_desc.StencilReadMask = 0xFF;
	depth_stencil_desc.StencilWriteMask = 0xFF;
	depth_stencil_desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depth_stencil_desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depth_stencil_desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depth_stencil_desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depth_stencil_desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depth_stencil_desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depth_stencil_desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depth_stencil_desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	hr = device->CreateDepthStencilState(&depth_stencil_desc, &depth_stencil_state);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}

sprite_batch::~sprite_batch()
{
	vertex_shader->Release();
	pixel_shader->Release();
	input_layout->Release();
	vertex_buffer->Release();
	rasterizer_state->Release();

	shader_resource_view->Release();

	depth_stencil_state->Release();
	sampler_state->Release();
	instance_buffer->Release();
}


void sprite_batch::begin(ID3D11DeviceContext* immediate_context)
{

	HRESULT hr = S_OK;

	UINT strides[2] = { sizeof(vertex), sizeof(instance) };
	UINT offsets[2] = { 0, 0 };
	ID3D11Buffer* vbs[2] = { vertex_buffer, instance_buffer };
	immediate_context->IASetVertexBuffers(0, 2, vbs, strides, offsets);
	immediate_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	immediate_context->IASetInputLayout(input_layout);
	immediate_context->OMSetDepthStencilState(depth_stencil_state, 1);
	immediate_context->RSSetState(rasterizer_state);
	immediate_context->VSSetShader(vertex_shader, nullptr, 0);
	immediate_context->PSSetShader(pixel_shader, nullptr, 0);
	immediate_context->PSSetShaderResources(0, 1, &shader_resource_view);
	immediate_context->PSSetSamplers(0, 1, &sampler_state);

	UINT num_viewports = 1;
	immediate_context->RSGetViewports(&num_viewports, &viewport);

	D3D11_MAP map = D3D11_MAP_WRITE_DISCARD;
	D3D11_MAPPED_SUBRESOURCE mapped_buffer;
	hr = immediate_context->Map(instance_buffer, 0, map, 0, &mapped_buffer);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	instances = static_cast<instance*>(mapped_buffer.pData);

	count_instance = 0;
}

void sprite_batch::render(
	const VECTOR2& position, const VECTOR2& scale,
	const VECTOR2& texPos, const VECTOR2& texSize,
	const VECTOR2& center, float angle/*radian*/,
	const VECTOR4& color)
{
	if (count_instance >= MAX_INSTANCES)
	{
		assert(!"Number of instances must be less than MAX_INSTANCES.(SpriteBatch)");
		return;
	}

	if (scale.x == 0.0f || scale.y == 0.0f) return;
	float tw = texSize.x;
	float th = texSize.y;
	if (texSize.x <= 0.0f || texSize.y <= 0.0f)
	{
		tw = (float)texture2d_desc.Width;
		th = (float)texture2d_desc.Height;
	}

	//	float cx = dw*0.5f, cy = dh*0.5f; /*Center of Rotation*/
	float cx = center.x;
	float cy = center.y;
	float sx = scale.x;
	float sy = scale.y;

#ifdef GAMELIB_PLUS_UP
	sy = -sy; // Y軸上が正ならスケール反転
	cy = texSize.y - cy;
#endif
	cx *= sx;
	cy *= sy;

#if 0
	DirectX::XMVECTOR scaling = DirectX::XMVectorSet(dw, dh, 1.0f, 0.0f);
	DirectX::XMVECTOR origin = DirectX::XMVectorSet(cx, cy, 0.0f, 0.0f);
	DirectX::XMVECTOR translation = DirectX::XMVectorSet(dx, dy, 0.0f, 0.0f);
	DirectX::XMMATRIX M = DirectX::XMMatrixAffineTransformation2D(scaling, origin, angle * 0.01745f, translation);
	DirectX::XMMATRIX N(
		2.0f / viewport.Width, 0.0f, 0.0f, 0.0f,
		0.0f, -2.0f / viewport.Height, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f, 1.0f);
	XMStoreFloat4x4(&instances[count_instance].ndc_transform, DirectX::XMMatrixTranspose(M * N)); //column_major
#else
	FLOAT c = cosf(angle);
	FLOAT s = sinf(angle);
	FLOAT w = 2.0f / viewport.Width;
	FLOAT h = -2.0f / viewport.Height;

	instances[count_instance].ndc_transform._11 = w * sx * tw * c;
	instances[count_instance].ndc_transform._21 = h * sx * tw * s;
	instances[count_instance].ndc_transform._31 = 0.0f;
	instances[count_instance].ndc_transform._41 = 0.0f;
	instances[count_instance].ndc_transform._12 = w * sy * th * -s;
	instances[count_instance].ndc_transform._22 = h * sy * th * c;
	instances[count_instance].ndc_transform._32 = 0.0f;
	instances[count_instance].ndc_transform._42 = 0.0f;
	instances[count_instance].ndc_transform._13 = 0.0f;
	instances[count_instance].ndc_transform._23 = 0.0f;
	instances[count_instance].ndc_transform._33 = 1.0f;
	instances[count_instance].ndc_transform._43 = 0.0f;
	instances[count_instance].ndc_transform._14 = w * (-cx * c + -cy * -s + cx * 0 + position.x) - 1.0f;
	instances[count_instance].ndc_transform._24 = h * (-cx * s + -cy * c + cy * 0 + position.y) + 1.0f;
	instances[count_instance].ndc_transform._34 = 0.0f;
	instances[count_instance].ndc_transform._44 = 1.0f;
#ifdef GAMELIB_PLUS_UP
	instances[instanceCount].ndcTransform._21 *= -1;
	instances[instanceCount].ndcTransform._22 *= -1;
	instances[instanceCount].ndcTransform._24 *= -1;
#endif
#endif
	float tex_width = static_cast<float>(texture2d_desc.Width);
	float tex_height = static_cast<float>(texture2d_desc.Height);

	// UV座標の調整
	float u = tw * UV_ADJUST / tex_width;
	float v = th * UV_ADJUST / tex_height;

	instances[count_instance].texcoord_transform = VECTOR4(texPos.x / tex_width, texPos.y / tex_height, u, v);
	instances[count_instance].color = color;

	count_instance++;
}

void sprite_batch::end(ID3D11DeviceContext* immediate_context)
{
	immediate_context->Unmap(instance_buffer, 0);

	immediate_context->DrawInstanced(4, count_instance, 0, 0);
}