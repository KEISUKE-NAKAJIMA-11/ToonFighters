#pragma once

#include <d3d11.h>
#include <directxmath.h>
#include <wrl.h>

#include "Vector.h"
#include "util.h"

class sprite
{
private:
	ID3D11VertexShader* vertex_shader;
	ID3D11PixelShader* pixel_shader;
	ID3D11InputLayout* input_layout;
	ID3D11Buffer* buffer;
	ID3D11RasterizerState* rasterizer_state;

	//std::shared_ptr<TextureShader> texture = nullptr;

	ID3D11ShaderResourceView* shader_resource_view;
	D3D11_TEXTURE2D_DESC       texture2d_desc;
	ID3D11DepthStencilState* depth_stencil_state;
	ID3D11SamplerState* sampler_state;



public:

	struct vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT4 color;
		DirectX::XMFLOAT2 texcoord;
	};



	sprite(ID3D11Device* device, const wchar_t* file_name, int nBufSize = (4));
	//sprite(ID3D11Device* device, int nBufSize = (4));

	void render(ID3D11DeviceContext*,
		const VECTOR2&, const VECTOR2&,
		const VECTOR2&, const VECTOR2&,
		const VECTOR2&, float,
		const VECTOR4&
	) const;

	//void render(ID3D11DeviceContext*,
	//	TextureShader* tex,
	//	float dx, float dy, float dw, float dh,
	//	float sx, float sy, float sw, float sh,
	//	float angle = 0.0f,
	//	float alpha = 1.0f
	//);




	//void render(ID3D11DeviceContext* immediate_context, VECTOR2& pos, VECTOR2& size, VECTOR2& s, VECTOR2& s_size, VECTOR2& center, VECTOR2& scale, float angle, VECTOR4& color) const;
 //   
	virtual ~sprite()
	{
		vertex_shader->Release();
		pixel_shader->Release();
		input_layout->Release();
		buffer->Release();
		rasterizer_state->Release();

		shader_resource_view->Release();

		depth_stencil_state->Release();
		sampler_state->Release();
	}




	//void render(ID3D11DeviceContext*, DirectX::XMFLOAT2 pos,// dx, dy : Coordinate of sprite's left-top corner in screen space 
	//	DirectX::XMFLOAT2 size,
	//	DirectX::XMFLOAT2 s,
	//	DirectX::XMFLOAT2 s_size,
	//	DirectX::XMFLOAT2 center,
	//	DirectX::XMFLOAT2 scale,// dw, dh : Size of sprite in screen space   
	//	float angle,// angle : Raotation angle (Rotation centre is sprite's centre), Unit is degree 
	//	DirectX::XMFLOAT4 color) const;// r, g, b, a : Color of sprite's each vertices ) const;

};






class sprite_batch
{
private:

	ID3D11VertexShader* vertex_shader;
	ID3D11PixelShader* pixel_shader;
	ID3D11InputLayout* input_layout;
	ID3D11Buffer* vertex_buffer;
	ID3D11RasterizerState* rasterizer_state;
	ID3D11SamplerState* sampler_state;
	ID3D11DepthStencilState* depth_stencil_state;
	ID3D11ShaderResourceView* shader_resource_view;

	D3D11_TEXTURE2D_DESC       texture2d_desc;
	const size_t MAX_INSTANCES = 256;

	struct instance
	{

		DirectX::XMFLOAT4X4 ndc_transform;

		DirectX::XMFLOAT4 texcoord_transform;

		DirectX::XMFLOAT4 color;

	};
	instance instanceresource;
	ID3D11Buffer* instance_buffer;
public:
	struct vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 texcoord;
	};


	sprite_batch(ID3D11Device* device, const wchar_t* file_name, size_t max_instance = 256);
	~sprite_batch();

	void begin(ID3D11DeviceContext* immediate_context);
	void render(const VECTOR2& position, const VECTOR2& scale, const VECTOR2& texPos, const VECTOR2& texSize, const VECTOR2& center, float angle, const VECTOR4& color);
	//void render(ID3D11DeviceContext*, VECTOR2& pos, VECTOR2& size, VECTOR2& s, VECTOR2& s_size, VECTOR2& center, VECTOR2& scale, float angle, VECTOR4& color);
	//
	//void render(ID3D11DeviceContext*, DirectX::XMFLOAT2 pos,// dx, dy : Coordinate of sprite's left-top corner in screen space 
	//	DirectX::XMFLOAT2 size,
	//	DirectX::XMFLOAT2 s,
	//	DirectX::XMFLOAT2 s_size,
	//	DirectX::XMFLOAT2 center,
	//	DirectX::XMFLOAT2 scale, // dw, dh : Size of sprite in screen space   
	//	float angle,// angle : Raotation angle (Rotation centre is sprite's centre), Unit is degree 
	//	DirectX::XMFLOAT4 color);// r, g, b, a : Color of sprite's each vertices ) const;
	void end(ID3D11DeviceContext* immediate_context);

private:
	D3D11_VIEWPORT viewport;

	UINT count_instance = 0;
	instance* instances = nullptr;
};