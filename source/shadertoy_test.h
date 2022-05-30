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

class Sadertoy_test :fullscreen_quad
{
public:
	Sadertoy_test(ID3D11Device* device, uint32_t width, uint32_t height,bool ending = false) : fullscreen_quad(device)
	{
		toy_buffer = std::make_unique<framebuffer>(device, width, height, false, 1, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_UNKNOWN, true, false, true/*generate_mips*/);
		if (!ending)
		{
			shadertoy_ps = std::make_unique<pixel_shader>(device, "./Shader/shadertoy.cso");
		}
		else
		{
			shadertoy_ps = std::make_unique<pixel_shader>(device, "./Shader/shadertoyending.cso");
		}
	
		constant_buffer = std::make_unique<Constant_buffer<shader_constants>>(device);
		sampler_states[LINEAR_BORDER] = std::make_unique<bloom_sampler_state>(device, D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_BORDER, D3D11_COMPARISON_ALWAYS, 0.0f, 0.0f, 0.0f, 1.0f);
		sampler_states[POINT] = std::make_unique<bloom_sampler_state>(device, D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_COMPARISON_ALWAYS, 0.0f, 0.0f, 0.0f, 1.0f);
		sampler_states[LINEAR] = std::make_unique<bloom_sampler_state>(device, D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_COMPARISON_ALWAYS, 0.0f, 0.0f, 0.0f, 1.0f);
		sampler_states[ANISOTROPIC] = std::make_unique<bloom_sampler_state>(device, D3D11_FILTER_ANISOTROPIC, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_COMPARISON_ALWAYS, 0.0f, 0.0f, 0.0f, 1.0f);

	}
	virtual ~Sadertoy_test() = default;

	enum { LINEAR_BORDER, POINT, LINEAR, ANISOTROPIC };
	std::unique_ptr<bloom_sampler_state> sampler_states[4];
	std::unique_ptr<framebuffer> toy_buffer;
	struct shader_constants
	{
		DirectX::XMFLOAT4 iResolution;
		DirectX::XMFLOAT4 iMouse;
		DirectX::XMFLOAT4 iChannelResolution[4];
		FLOAT iTime = 0;
		FLOAT iFrame = 0;
		FLOAT iPad0 = 0;
		FLOAT iPad1 = 0;

		template<class T>
		void serialize(T& archive)
		{

		}


	};

	std::unique_ptr<Constant_buffer<shader_constants>> constant_buffer;

	std::unique_ptr<pixel_shader> shadertoy_ps;


	float cumulative_time = 0;
public:

	void blit(ID3D11DeviceContext* immediate_context, ID3D11ShaderResourceView* texture, float elapsed_time)
	{
	
		sampler_states[LINEAR]->activate(immediate_context, 0, false);
		{
			
			immediate_context->PSSetShaderResources(0, 0, &texture);


			constant_buffer->activate(immediate_context, 6, false, true);
			//toy_buffer->clear(immediate_context);
			//toy_buffer->activate(immediate_context);
			shadertoy_ps->activate(immediate_context);
	        fullscreen_quad::blit(immediate_context);
			shadertoy_ps->deactivate(immediate_context);
			//toy_buffer->deactivate(immediate_context);



			ID3D11ShaderResourceView* null_shader_resource_views[2] = {};
			immediate_context->PSSetShaderResources(0, 2, null_shader_resource_views);
		}
		sampler_states[LINEAR]->deactivate(immediate_context);

	}


	void generate(float elapsed_time)
	{
		// SSL_ST
		constant_buffer->data.iResolution.x = 1280.0f;
		constant_buffer->data.iResolution.y = 720.0f;
		constant_buffer->data.iResolution.z = 0.0f;
		constant_buffer->data.iResolution.w = 0.0f;
		constant_buffer->data.iChannelResolution[0].x = 1601.0f;
		constant_buffer->data.iChannelResolution[0].y = 893.0f;
		constant_buffer->data.iChannelResolution[0].z = 0.0f;
		constant_buffer->data.iChannelResolution[0].w = 0.0f;
		constant_buffer->data.iChannelResolution[1].x = 470.0f;
		constant_buffer->data.iChannelResolution[1].y = 470.0f;
		constant_buffer->data.iChannelResolution[1].z = 0.0f;
		constant_buffer->data.iChannelResolution[1].w = 0.0f;
		constant_buffer->data.iTime += elapsed_time;
		constant_buffer->data.iFrame += 1;
	}



};