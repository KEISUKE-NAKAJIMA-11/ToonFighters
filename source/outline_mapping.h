#pragma once
#include <iostream>
#include <fstream>
#include <shlwapi.h>
#undef max
#undef min


#include "framebuffer.h"

#include "rasterizer.h"
#include "bloom.h"
#include "Shadermanager.h"

class OutLine :fullscreen_quad
{
public:
	OutLine(ID3D11Device* device) : fullscreen_quad(device)
	{
		average_luminance = std::make_unique<framebuffer>(device, 512, 512, false, 1, DXGI_FORMAT_R16_FLOAT, DXGI_FORMAT_UNKNOWN, true, false, true/*generate_mips*/);
		outline_ps = std::make_unique<pixel_shader>(device, "./Shader/average_luminance_ps.cso");
		constant_buffer = std::make_unique<Constant_buffer<shader_constants>>(device);
		sampler_states[LINEAR_BORDER] = std::make_unique<bloom_sampler_state>(device, D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_BORDER, D3D11_COMPARISON_ALWAYS, 0.0f, 0.0f, 0.0f, 1.0f);
		sampler_states[POINT] = std::make_unique<bloom_sampler_state>(device, D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_COMPARISON_ALWAYS, 0.0f, 0.0f, 0.0f, 1.0f);
		sampler_states[LINEAR] = std::make_unique<bloom_sampler_state>(device, D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_COMPARISON_ALWAYS, 0.0f, 0.0f, 0.0f, 1.0f);
		sampler_states[ANISOTROPIC] = std::make_unique<bloom_sampler_state>(device, D3D11_FILTER_ANISOTROPIC, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_COMPARISON_ALWAYS, 0.0f, 0.0f, 0.0f, 1.0f);

	}
	virtual ~OutLine() = default;

	enum { LINEAR_BORDER, POINT, LINEAR, ANISOTROPIC };
	std::unique_ptr<bloom_sampler_state> sampler_states[4];
	std::unique_ptr<framebuffer> average_luminance;
	struct shader_constants
	{
		float bias = 0.00f;//アウトラインの太さ
		DirectX::XMFLOAT3 line_coulour{};//アウトラインカラー


	};

	std::unique_ptr<Constant_buffer<shader_constants>> constant_buffer;

	std::unique_ptr<pixel_shader> outline_ps;


	float cumulative_time = 0;
public:

	void blit(ID3D11DeviceContext* immediate_context, ID3D11ShaderResourceView* hdr_texture, float elapsed_time)
	{
		sampler_states[LINEAR_BORDER]->activate(immediate_context, 2, false);
		{
			immediate_context->PSSetShaderResources(0, 1, &hdr_texture);

			constant_buffer->activate(immediate_context, 0, false, true);

			outline_ps->activate(immediate_context);
			fullscreen_quad::blit(immediate_context);
			outline_ps->deactivate(immediate_context);

			ID3D11ShaderResourceView* null_shader_resource_view = nullptr;
			immediate_context->PSSetShaderResources(0, 1, &null_shader_resource_view);
		}
		sampler_states[LINEAR_BORDER]->deactivate(immediate_context);

	}


};